// main.cpp — CppLogicMake driver CLI
//
// Single project:
//   logicmake --input project.lm --output CMakeLists.txt
//
// Multiple independent projects, resolved in parallel (one worker per
// input, capped at hardware_concurrency — see resolveAll() below):
//   logicmake --input a.lm --input b.lm --input c.lm --output-dir generated/
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "cmake_emitter.hpp"
#include "git_integration.hpp"
#include "prolog_engine.hpp"
#include "resolver.hpp"

namespace {

struct Args {
    std::vector<std::filesystem::path> inputs;
    std::filesystem::path output = "CMakeLists.txt";  // single-input mode
    std::filesystem::path outputDir;                  // multi-input mode
    std::filesystem::path schema = "prolog/targets.pl";
};

std::optional<Args> parse(int argc, char** argv) {
    Args args;

    for (int i = 1; i < argc; ++i) {
        const std::string flag = argv[i];
        auto next = [&]() -> std::string {
            if (i + 1 >= argc) {
                throw std::runtime_error("missing value for " + flag);
            }
            return argv[++i];
        };

        if (flag == "--input" || flag == "-i") {
            args.inputs.push_back(next());
        } else if (flag == "--output" || flag == "-o") {
            args.output = next();
        } else if (flag == "--output-dir") {
            args.outputDir = next();
        } else if (flag == "--schema" || flag == "-s") {
            args.schema = next();
        } else if (flag == "--help" || flag == "-h") {
            return std::nullopt;
        } else {
            throw std::runtime_error("unrecognised argument: " + flag);
        }
    }

    if (args.inputs.empty()) {
        throw std::runtime_error("at least one --input <project.lm> is required");
    }
    if (args.inputs.size() > 1 && args.outputDir.empty()) {
        throw std::runtime_error(
            "multiple --input files require --output-dir "
            "(one CMakeLists.txt per input isn't meaningful — each "
            "input gets <stem>.cmake under the output directory)");
    }
    return args;
}

void printUsage() {
    std::cerr
        << "logicmake --input <project.lm> --output <CMakeLists.txt>\n"
        << "          [--schema <prolog/targets.pl>]\n"
        << "logicmake --input <a.lm> --input <b.lm> ... --output-dir <dir>\n"
        << "          [--schema <prolog/targets.pl>]\n";
}

struct Job {
    std::filesystem::path input;
    std::filesystem::path output;
};

// Returns true only if the job produced a CMakeLists.txt. A false
// return must propagate to a non-zero process exit code (see main):
// the logimake wrapper keys off that exit code to halt before running
// cmake, so a swallowed resolver failure here would leave it building a
// stale CMakeLists.txt from a previous run.
bool runJob(const Args& args, const Job& job,
            const std::optional<std::string>& gitStamp) {
    const auto start = std::chrono::steady_clock::now();
    try {
        logicmake::Resolver resolver(args.schema, job.input);
        const auto targets = resolver.resolve();

        if (targets.empty()) {
            std::cerr << "warning: no targets resolved from "
                      << job.input.string() << "\n";
        }

        const auto cmake =
            logicmake::emitCMakeLists(targets, gitStamp, job.output.parent_path());

        std::ofstream out(job.output);
        if (!out) {
            std::cerr << "error: could not write " << job.output.string()
                      << "\n";
            return false;
        }
        out << cmake;

        const auto elapsedMs =
            std::chrono::duration<double, std::milli>(
                std::chrono::steady_clock::now() - start)
                .count();

        std::cerr << "wrote " << job.output.string() << " ("
                   << targets.size() << " targets, " << elapsedMs << " ms)\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "error resolving " << job.input.string() << ": "
                   << e.what() << "\n";
        return false;
    }
}

// Resolves every job. Each job gets its own PrologEngine/Interpreter —
// there is no shared mutable Prolog state across jobs by design, which
// is what makes this safe to parallelize at all (see
// prolog_engine.cpp's warmUpPrologRuntime for the one piece of shared
// state — CppProlog's builtin-predicate table — that does need
// warming up first). Work is handed out via a shared atomic index
// rather than a fixed static split, so a slow project file (a large
// dependency graph) doesn't leave a worker idle while others still
// have jobs queued.
// Returns true only if every job succeeded. Any failure must surface as
// a non-zero exit code so downstream tooling stops rather than building
// stale output.
bool resolveAll(const Args& args, const std::vector<Job>& jobs) {
    logicmake::warmUpPrologRuntime();
    const auto gitStamp = logicmake::gitProvenanceStamp();

    const auto workerCount = std::min<std::size_t>(
        jobs.size(),
        std::max(1u, std::thread::hardware_concurrency()));

    std::atomic<std::size_t> next{0};
    std::atomic<bool> anyFailed{false};
    std::vector<std::jthread> pool;
    pool.reserve(workerCount);

    for (std::size_t w = 0; w < workerCount; ++w) {
        pool.emplace_back([&] {
            for (;;) {
                const auto i = next.fetch_add(1, std::memory_order_relaxed);
                if (i >= jobs.size()) return;
                if (!runJob(args, jobs[i], gitStamp)) {
                    anyFailed.store(true, std::memory_order_relaxed);
                }
            }
        });
    }
    // Join explicitly before reading anyFailed: std::jthread would join
    // on `pool`'s destruction, but that happens after this return
    // evaluates, so we must wait for every worker here first.
    for (auto& t : pool) {
        t.join();
    }
    return !anyFailed.load(std::memory_order_relaxed);
}

}  // namespace

int main(int argc, char** argv) {
    std::optional<Args> args;
    try {
        args = parse(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        printUsage();
        return 1;
    }

    if (!args) {
        printUsage();
        return 0;
    }

    std::vector<Job> jobs;
    if (args->inputs.size() == 1) {
        jobs.push_back({args->inputs.front(), args->output});
    } else {
        std::filesystem::create_directories(args->outputDir);
        for (const auto& input : args->inputs) {
            jobs.push_back({input, args->outputDir / (input.stem().string() +
                                                        ".cmake")});
        }
    }

    return resolveAll(*args, jobs) ? 0 : 1;
}

// git_integration.cpp
#include "git_integration.hpp"

#include <array>
#include <algorithm>
#include <cstdio>
#include <memory>
#include <sstream>
#include <stdexcept>

#if !defined(_WIN32)
#include <sys/wait.h>
#endif

namespace logicmake {

namespace {

std::string shellQuote(const std::string& s) {
#if defined(_WIN32)
    std::string out = "\"";
    for (char c : s) {
        if (c == '"') {
            out += "\\\"";
        } else if (c == '\\') {
            out += "\\\\";
        } else {
            out += c;
        }
    }
    out += "\"";
    return out;
#else
    std::string out = "'";
    for (char c : s) {
        if (c == '\'') {
            out += "'\\''";
        } else {
            out += c;
        }
    }
    out += "'";
    return out;
#endif
}

int exitCodeOf(int pcloseStatus) {
#if defined(_WIN32)
    return pcloseStatus;
#else
    if (WIFEXITED(pcloseStatus)) {
        return WEXITSTATUS(pcloseStatus);
    }
    return -1;
#endif
}

struct ShellResult {
    int exitCode;
    std::string output;
};

ShellResult runShell(const std::string& command) {
    std::array<char, 4096> buffer{};
    std::string output;

#if defined(_WIN32)
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(
        _popen(command.c_str(), "r"), _pclose);
#else
    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen(command.c_str(), "r"), pclose);
#endif

    if (!pipe) {
        throw std::runtime_error("failed to launch: " + command);
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) !=
           nullptr) {
        output += buffer.data();
    }

#if defined(_WIN32)
    const int rawStatus = _pclose(pipe.release());
#else
    const int rawStatus = pclose(pipe.release());
#endif

    return {exitCodeOf(rawStatus), output};
}

std::string rtrim(std::string s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ')) {
        s.pop_back();
    }
    return s;
}

}  // namespace

std::vector<std::string> resolveGitSources(const std::string& pathspec) {
    // --recurse-submodules: without it, a pathspec pointing inside a real
    // git submodule (e.g. "Ext/glfw/src/*.c") resolves to zero files —
    // plain `git ls-files` only sees a submodule's own gitlink entry, not
    // the tracked files inside it, since submodule content lives in its
    // own separate index. With the flag, git ls-files transparently
    // descends into any submodule the pathspec reaches (as long as it has
    // been initialized/checked out via `git submodule update --init`) and
    // returns paths relative to the superproject, same as any other
    // tracked file. Requires git 2.11+ (2016), safe to assume.
    const auto result = runShell("git ls-files --recurse-submodules -- " +
                                  shellQuote(pathspec) + " 2>&1");

    if (result.exitCode != 0) {
        throw std::runtime_error(
            "git ls-files failed for pathspec '" + pathspec +
            "': " + rtrim(result.output) +
            " (is the current directory inside a git repository?)");
    }

    std::vector<std::string> files;
    std::istringstream stream(result.output);
    std::string line;
    while (std::getline(stream, line)) {
        line = rtrim(line);
        if (!line.empty()) {
            files.push_back(line);
        }
    }

    if (files.empty()) {
        throw std::runtime_error(
            "pathspec '" + pathspec +
            "' matched no tracked files (check the path, and that the "
            "files are committed or staged — git ls-files only sees "
            "what's in the index)");
    }

    std::sort(files.begin(), files.end());
    return files;
}

std::optional<std::string> gitProvenanceStamp() {
    const auto hashResult = runShell("git rev-parse --short HEAD 2>&1");
    if (hashResult.exitCode != 0) {
        return std::nullopt;
    }
    const std::string hash = rtrim(hashResult.output);
    if (hash.empty()) {
        return std::nullopt;
    }

    const auto statusResult = runShell("git status --porcelain 2>&1");
    const bool dirty =
        statusResult.exitCode == 0 && !rtrim(statusResult.output).empty();

    return dirty ? hash + "-dirty" : hash;
}

}  // namespace logicmake

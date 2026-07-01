// test_threading.cpp — verifies that concurrent Resolver use, each
// with its own PrologEngine, produces results identical to sequential
// resolution, once warmUpPrologRuntime() has run first. This is the
// functional-correctness half of the threading story; the data race
// warmUpPrologRuntime() avoids (in CppProlog's builtin-predicate
// table) was separately confirmed and fixed under ThreadSanitizer —
// see prolog_engine.cpp's comment for that verification. This test
// checks results, not races: run it under `ctest -T memcheck` or a
// TSan-instrumented build (LOGICMAKE_SANITIZE=thread) for the latter.
#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include "prolog_engine.hpp"
#include "resolver.hpp"

namespace {

std::size_t resolveTargetCount() {
    logicmake::Resolver resolver("prolog/targets.pl", "examples/kai_workspace.pl");
    return resolver.resolve().size();
}

}  // namespace

TEST(Threading, ConcurrentResolutionsAgreeWithSequential) {
    const std::size_t expected = resolveTargetCount();
    ASSERT_EQ(expected, 5u);

    logicmake::warmUpPrologRuntime();

    constexpr int kThreads = 8;
    std::vector<std::size_t> results(kThreads, 0);
    {
        std::vector<std::jthread> threads;
        threads.reserve(kThreads);
        for (int i = 0; i < kThreads; ++i) {
            threads.emplace_back([&results, i] {
                results[static_cast<std::size_t>(i)] = resolveTargetCount();
            });
        }
        // jthreads join here, at end of scope.
    }

    for (int i = 0; i < kThreads; ++i) {
        EXPECT_EQ(results[static_cast<std::size_t>(i)], expected) << "thread " << i;
    }
}

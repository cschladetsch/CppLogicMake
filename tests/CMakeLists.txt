add_executable(logicmake_tests
    test_emitter.cpp
    test_resolver.cpp
    test_threading.cpp
)
target_link_libraries(logicmake_tests PRIVATE logicmake_core GTest::gtest_main)

include(GoogleTest)
# WORKING_DIRECTORY matters: test_resolver.cpp and test_threading.cpp
# load prolog/targets.pl and examples/kai_workspace.lm by their
# repo-relative paths.
gtest_discover_tests(logicmake_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Whole-scope integration test: drives the logimake CLI to build the
# examples/fib project (a C++23 and a C++17 target), then runs those plus
# the Python and Rust siblings and asserts every available implementation
# agrees on the sum of the first ten Fibonacci numbers (88). Python and
# Rust are skipped, not failed, when their toolchains are absent. Needs
# PowerShell; when it is unavailable the test is simply not registered.
find_program(PWSH_EXECUTABLE NAMES pwsh powershell)
if(PWSH_EXECUTABLE)
    add_test(
        NAME Fib.CrossLanguageEndToEnd
        COMMAND ${PWSH_EXECUTABLE} -NoProfile -File
                ${CMAKE_CURRENT_SOURCE_DIR}/fib_end_to_end.ps1
                -BuildDir ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
else()
    message(WARNING
        "pwsh/powershell not found; skipping Fib.CrossLanguageEndToEnd test")
endif()

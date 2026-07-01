add_executable(logicmake_tests
    test_emitter.cpp
    test_resolver.cpp
    test_threading.cpp
)
target_link_libraries(logicmake_tests PRIVATE logicmake_core GTest::gtest_main)

include(GoogleTest)
# WORKING_DIRECTORY matters: test_resolver.cpp and test_threading.cpp
# load prolog/targets.pl and examples/kai_workspace.pl by their
# repo-relative paths.
gtest_discover_tests(logicmake_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

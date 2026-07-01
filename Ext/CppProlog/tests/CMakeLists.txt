include_directories(../src)

add_executable(prolog_tests
    test_main.cpp
    test_term.cpp
    test_unification.cpp
    test_parser.cpp
    test_database.cpp
    test_resolver.cpp
    test_interpreter.cpp
    test_builtin_predicates.cpp
    test_utils.cpp
    test_deductive_reasoning.cpp
    test_new_features.cpp
    # test_comprehensive_features.cpp
    test_more_builtins.cpp
    test_cut.cpp
)

target_link_libraries(prolog_tests 
    prolog_core
    gtest
    gtest_main
)
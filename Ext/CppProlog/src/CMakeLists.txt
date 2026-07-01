add_library(prolog_core
    prolog/term.cpp
    prolog/clause.cpp
    prolog/database.cpp
    prolog/parser.cpp
    prolog/unification.cpp
    prolog/resolver.cpp
    prolog/interpreter.cpp
    prolog/builtin_predicates.cpp
    utils/string_utils.cpp
    utils/memory_pool.cpp
)

target_include_directories(prolog_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/External/rang/include
    ${CMAKE_BINARY_DIR}/_deps/linenoise-src/include
)

target_compile_features(prolog_core PUBLIC cxx_std_23)

target_link_libraries(prolog_core PUBLIC linenoise rang)

target_compile_options(linenoise PRIVATE -fpermissive)

add_executable(prolog_interpreter
    main.cpp
)

target_link_libraries(prolog_interpreter prolog_core)
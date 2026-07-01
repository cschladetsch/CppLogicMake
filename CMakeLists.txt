cmake_minimum_required(VERSION 3.25)
project(CppLogicMake LANGUAGES CXX)

# Yes, CMake builds the tool that generates CMakeLists.txt files.
# This one is handwritten — CppLogicMake replaces authoring layers for
# resolved dependency graphs, not for building itself.

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Development option: build everything (driver, tests, and the
# prolog_core dependency) instrumented with a sanitizer. Used to
# verify the threading story in src/prolog_engine.cpp — see that file
# for what was actually found and fixed under -fsanitize=thread.
#   cmake -B build -DLOGICMAKE_SANITIZE=thread
set(LOGICMAKE_SANITIZE "" CACHE STRING
    "Build with a sanitizer: thread, address, or empty for none")
if(LOGICMAKE_SANITIZE)
    add_compile_options(-fsanitize=${LOGICMAKE_SANITIZE} -g -O1)
    add_link_options(-fsanitize=${LOGICMAKE_SANITIZE})
endif()

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/Ext/CppProlog/src/prolog/interpreter.h")
    message(FATAL_ERROR
        "Ext/CppProlog is empty. Run: git submodule update --init --recursive")
endif()

# --- CppProlog (Ext/CppProlog), embedded directly ---
#
# Deliberately NOT add_subdirectory(Ext/CppProlog): its own
# CMakeLists.txt pulls in googletest, google/benchmark, and
# linenoise-ng via FetchContent to build its own tests, benchmarks,
# and examples — none of which CppLogicMake needs, and it would
# collide with our own googletest submodule below. Instead the
# interpreter core is built as our own library target directly from
# the submodule's sources. Same principle as the CMake substrate
# itself: keep the dependency, replace only the parts of its build
# we don't want.
set(PROLOG_CORE_DIR ${CMAKE_SOURCE_DIR}/Ext/CppProlog)

add_library(prolog_core
    ${PROLOG_CORE_DIR}/src/prolog/term.cpp
    ${PROLOG_CORE_DIR}/src/prolog/clause.cpp
    ${PROLOG_CORE_DIR}/src/prolog/database.cpp
    ${PROLOG_CORE_DIR}/src/prolog/parser.cpp
    ${PROLOG_CORE_DIR}/src/prolog/unification.cpp
    ${PROLOG_CORE_DIR}/src/prolog/resolver.cpp
    ${PROLOG_CORE_DIR}/src/prolog/interpreter.cpp
    ${PROLOG_CORE_DIR}/src/prolog/builtin_predicates.cpp
    ${PROLOG_CORE_DIR}/src/utils/string_utils.cpp
    ${PROLOG_CORE_DIR}/src/utils/memory_pool.cpp
)
target_include_directories(prolog_core PUBLIC
    ${PROLOG_CORE_DIR}/src
    ${PROLOG_CORE_DIR}/External/rang/include
)
target_compile_features(prolog_core PUBLIC cxx_std_23)

# --- googletest (Ext/googletest) ---
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(Ext/googletest)

# --- driver, split so the resolution/emission logic is testable
#     without linking the CLI's main() ---
find_package(Threads REQUIRED)

add_library(logicmake_core
    src/prolog_engine.cpp
    src/resolver.cpp
    src/cmake_emitter.cpp
    src/git_integration.cpp
)
target_include_directories(logicmake_core PUBLIC src)
target_link_libraries(logicmake_core PUBLIC prolog_core Threads::Threads)

add_executable(logicmake src/main.cpp)
target_link_libraries(logicmake PRIVATE logicmake_core)

enable_testing()
add_subdirectory(tests)

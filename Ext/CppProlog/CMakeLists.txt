cmake_minimum_required(VERSION 3.20)

function(display_build_info)
    message("
   █████████ ███████████ ███████████               ████                 
  ███░░░░░██░░███░░░░░██░░███░░░░░███             ░░███                 
 ███     ░░░ ░███    ░███░███    ░██████████ ██████░███  ██████  ███████
░███         ░██████████ ░█████████░░███░░█████░░██░███ ███░░██████░░███
░███         ░███░░░░░░  ░███░░░░░░ ░███ ░░░███ ░██░███░███ ░██░███ ░███
░░███     ███░███        ░███       ░███   ░███ ░██░███░███ ░██░███ ░███
 ░░█████████ █████       █████      █████  ░░██████████░░██████░░███████
  ░░░░░░░░░ ░░░░░       ░░░░░      ░░░░░    ░░░░░░░░░░░ ░░░░░░  ░░░░░███
                                                                ███ ░███
                                                               ░░██████ 
                                                                ░░░░░░  ")
    message("")
    message(STATUS "=== Build Configuration ===")
    message(STATUS "Project: ${PROJECT_NAME}")
    message(STATUS "Version: ${PROJECT_VERSION}")
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
    message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
    message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
    message(STATUS "Source Directory: ${CMAKE_SOURCE_DIR}")
    message(STATUS "Build Directory: ${CMAKE_BINARY_DIR}")
    message(STATUS "===========================")
    message("")
endfunction()


project(CppLProlog VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

display_build_info()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)
FetchContent_MakeAvailable(benchmark)

FetchContent_Declare(
    linenoise
    GIT_REPOSITORY https://github.com/arangodb/linenoise-ng.git
    GIT_TAG master
)
FetchContent_MakeAvailable(linenoise)

# Add rang header-only library
add_subdirectory(External/rang)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(examples)
add_subdirectory(benchmarks)

enable_testing()

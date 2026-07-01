cmake_minimum_required(VERSION 3.15)
project(c++_project VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
add_executable(c++_project src/main.cpp)

# Set properties
set_target_properties(c++_project PROPERTIES
    OUTPUT_NAME c++_project
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Enable testing
enable_testing()

# Add a simple test
add_test(NAME basic_test COMMAND c++_project --version)

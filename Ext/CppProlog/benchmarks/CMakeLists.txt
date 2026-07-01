include_directories(../src)

add_executable(prolog_benchmarks
    benchmark_main.cpp
    benchmark_unification.cpp
    benchmark_parsing.cpp
    benchmark_resolution.cpp
)

target_link_libraries(prolog_benchmarks 
    prolog_core
    benchmark::benchmark
)
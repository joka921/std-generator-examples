cmake_minimum_required(VERSION 3.20)
project(std-generator-examples)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Enable testing for Google Benchmark to avoid building the benchmarks themselves
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark's tests")
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable GTest dependency in benchmark")

FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3 # You can change this to the version you need
)

FetchContent_MakeAvailable(benchmark)

# Now you can link `benchmark` or `benchmark_main` to your targets
# Example:

add_executable(generator_benchmark generator_benchmark.cpp IndirectIota.cpp)
target_link_libraries(generator_benchmark PRIVATE benchmark::benchmark_main)






add_executable(fibonacci_generator fibonacci_generator.cpp)
add_executable(expressions_main ExpressionsMain.cpp)
# Include FetchContent
include(FetchContent)

# CMake Standard
cmake_minimum_required(VERSION 4.3)

# Project
project(skip_list)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include Directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Core library
add_library(skip_list_lib src/skip_list.cpp)

# Tests Executable
add_executable(skip_list_test tests/test.cpp)
target_link_libraries(skip_list_test PRIVATE skip_list_lib)

# Fetch Google Benchmark
find_package(benchmark QUIET)
if(NOT benchmark_FOUND)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.10.0
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)
endif()

# Benchmark Executable
add_executable(skip_list_benchmark benchmarks/benchmark.cpp)
target_link_libraries(skip_list_benchmark PRIVATE skip_list_lib benchmark::benchmark)
cmake_minimum_required(VERSION 3.15)
project(SquirtleFilter CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")

# Threads required for gtest and project
find_package(Threads REQUIRED)

# Add include path
include_directories(include)

# === GoogleTest ===
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
# Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# === Google Benchmark ===
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG main
)
FetchContent_MakeAvailable(benchmark)

# Main executable
add_executable(SquirtleMain main.cpp src/SquirtleFilter.cpp)
target_link_libraries(SquirtleMain PRIVATE Threads::Threads)

# === Tests ===
enable_testing()
add_executable(SquirtleTests tests/tests.cpp src/SquirtleFilter.cpp)
target_link_libraries(SquirtleTests gtest gtest_main Threads::Threads)
add_test(NAME SquirtleUnitTests COMMAND SquirtleTests)

# === Benchmark ===
add_executable(SquirtleBenchmark benchmarks/benchmark.cpp src/SquirtleFilter.cpp)
target_link_libraries(SquirtleBenchmark benchmark::benchmark Threads::Threads)

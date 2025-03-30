// ------------------------
// File: CMakeLists.txt
// ------------------------
cmake_minimum_required(VERSION 3.15)
project(SquirtleFilter CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")

include_directories(include)
add_subdirectory(tests)
add_subdirectory(benchmarks)

add_executable(SquirtleMain main.cpp src/SquirtleFilter.cpp)

find_package(Threads REQUIRED)
target_link_libraries(SquirtleMain PRIVATE Threads::Threads)

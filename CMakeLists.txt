cmake_minimum_required(VERSION 3.14)
project(mst_benchmark CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Release build for accurate timing (-O2 optimisations, no debug overhead)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(mst_benchmark
    src/main.cpp
    src/graph_utils.cpp
    src/kruskal.cpp
    src/prim.cpp
    src/boruvka.cpp
)

target_include_directories(mst_benchmark PRIVATE include)

target_compile_options(mst_benchmark PRIVATE
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Debug>:-g -O0>
    -Wall -Wextra
)

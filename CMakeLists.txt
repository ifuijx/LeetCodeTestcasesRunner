cmake_minimum_required(VERSION 3.10)

project(LeetCode-Testcases-Runner VERSION 1.0
                                  DESCRIPTION "A C++ project to run testcases for algorithm problems conveniently."
                                  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

configure_file(
    "${PROJECT_SOURCE_DIR}/source_dir_locator.hpp.in"
    "${PROJECT_BINARY_DIR}/source_dir_locator.hpp"
)

add_library(libsourcelocator INTERFACE)
target_include_directories(libsourcelocator INTERFACE "${PROJECT_BINARY_DIR}")

add_subdirectory(utils)

add_executable(Main main.cpp)

target_link_libraries(Main PUBLIC libunpacker libsourcelocator)

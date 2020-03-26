cmake_minimum_required(VERSION 3.14)
project(test)

set(CMAKE_CXX_STANDARD 98)
## add_executable(test main.cpp)

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")
set(LIBRARY_OUTPUT_PATH  "${PROJECT_SOURCE_DIR}/lib")
add_subdirectory(algorithm)

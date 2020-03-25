cmake_minimum_required(VERSION 3.14)
project(test)

set(CMAKE_CXX_STANDARD 98)
## add_executable(test main.cpp)

aux_source_directory(. SRC_LIST) # 搜索当前目录下的所有.cpp文件
add_executable(test ${SRC_LIST})

add_library(knn SHARED KNN.cpp KNN.h)
add_library(kmeans SHARED KMEANS.cpp KMEANS.h)
add_library(string SHARED  String.cpp String.h)
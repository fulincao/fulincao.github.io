
set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wno-unused-result -Wno-unused-but-set-variable -Wno-unused-variable -O3 -DUSE_GLOG=1 -std=c++11")
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wno-unused-result -Wno-unused-but-set-variable -Wno-unused-variable -g -O0 -std=c++11")

SET(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}")

add_library(knn SHARED KNN.cpp)
add_library(kmeans SHARED KMEANS.cpp)
add_library(string SHARED String.cpp)

aux_source_directory(. SRC_LIST)

# add_executable(test_algorithm ${SRC_LIST})

include_directories(
    "${PROJECT_ROOT_PATH}/include"
    "/usr/include"
    "/usr/local/include"
)


add_executable(kalman kalman.cpp)




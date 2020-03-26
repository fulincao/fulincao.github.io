
add_library(knn SHARED KNN.cpp)
add_library(kmeans SHARED KMEANS.cpp)
add_library(string SHARED String.cpp)

aux_source_directory(. SRC_LIST)
add_executable(test_algorithm ${SRC_LIST})


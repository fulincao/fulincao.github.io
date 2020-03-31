

aux_source_directory(. SRC_LIST)
add_executable(test_net utils.cpp test_net.cpp)
add_executable(test_listen test_listen.cpp)

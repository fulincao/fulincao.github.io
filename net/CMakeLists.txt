

aux_source_directory(. SRC_LIST)
add_executable(test_net utils.cpp test_net.cpp)
add_executable(test_listen test_listen.cpp)
add_executable(test_oob_recv testoobrecv.cpp)
add_executable(test_oob_send testoobsend.cpp)

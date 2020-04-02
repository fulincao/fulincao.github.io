

aux_source_directory(. SRC_LIST)
add_executable(test_net utils.cpp test_net.cpp)
add_executable(test_listen test_listen.cpp)
add_executable(test_oob_recv testoobrecv.cpp)
add_executable(test_oob_send testoobsend.cpp)
add_executable(test_send_buffer test_send_buffer.cpp)
add_executable(test_recv_buffer test_recv_buffer.cpp)

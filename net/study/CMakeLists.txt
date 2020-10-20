

aux_source_directory(. SRC_LIST)
add_executable(test_net utils.cpp test_net.cpp)
add_executable(test_listen test_listen.cpp)
add_executable(test_oob_recv testoobrecv.cpp)
add_executable(test_oob_send testoobsend.cpp)
add_executable(test_send_buffer test_send_buffer.cpp)
add_executable(test_recv_buffer test_recv_buffer.cpp)
add_executable(test_splice test_splice.cpp)
add_executable(test_tee test_tee.cpp)
add_executable(test_select test_select.cpp)
add_executable(poll_chatroom poll_chatroom.cpp)

FIND_LIBRARY(EVENT_LIB event /usr/local/lib NO_DEFAULT_PATH)
link_libraries(${EVENT_LIB})

add_executable(libevent_test libevent_test.cpp)
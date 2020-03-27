//
// Created by cao on 2020/3/27.
//  backlog listen监听队列的最大长度
//

#include <sys/socket.h>
#include <signal.h>

static bool stop = false;

static void handle_term(int sig){
    stop = true;
}

int main(int argc, char* argv[]) {
    signal(SIGTERM, handle_term);

    if(argc <= 3) {
        printf("usage")
    }


    return 0;
}

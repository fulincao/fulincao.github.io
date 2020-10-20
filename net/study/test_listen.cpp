//
// Created by cao on 2020/3/27.
//  backlog listen监听队列的最大长度
//

#include <sys/socket.h>
#include <signal.h>
#include <stdio.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>


static bool stop = false;

static void handle_term(int sig){
    stop = true;
}

int main(int argc, char* argv[]) {

    // 设置signal 处理函数
    signal(SIGTERM, handle_term);

    if(argc <= 3) {
        printf("usage： %s ip, port backlog", basename(argv[0]));
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int backlog = atoi(argv[3]);

    // 创建socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);
    sockaddr_in ski;
    //初始化socket地址 与memset等价
    bzero(&ski, sizeof(ski));

    ski.sin_family = AF_INET;

    // host to network seq
    ski.sin_port = htons(port);
    // 字符串ip地址转网络字节序
    inet_pton(AF_INET, ip, &ski.sin_addr);

    // 绑定
    int ret = bind(sock, (sockaddr* )&ski, sizeof(ski));
    assert(ret != -1);

    // 监听
    ret = listen(sock, backlog);
    assert(ret != -1);

    while(!stop ){
        sleep(1);
    }
    close(sock);
    return 0;
}

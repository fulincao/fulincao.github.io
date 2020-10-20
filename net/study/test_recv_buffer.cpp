//
// Created by cao on 2020/4/2.
//

#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <assert.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]){

    if(argc <= 3) {
        printf("usage: %s ip_address port_number recv buffer size\n", basename(argv[0]));
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);

    sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));

    server_address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_address.sin_addr);
    server_address.sin_family = AF_INET;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock > 0);

    int rcvbuf = atoi(argv[3]);
    int len = sizeof(rcvbuf);
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
    getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf, (socklen_t*)&len);
    printf("the tcp receive buffer size after setting is %d\n", rcvbuf);

    int ret = bind(sock, (sockaddr*)&server_address, sizeof(server_address));
    assert(ret != -1);
    ret = listen(sock, 5);
    assert(ret != -1);

    sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int connfd = accept(sock, (sockaddr*)&client, &client_len);

    if(connfd < 0) {
        printf("errno is: %d\n", connfd);
    }else{
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));
        while(recv(connfd, buffer, BUFFER_SIZE-1, 0) > 0 ){}
        close(connfd);
    }
    close(sock);
    return 0;
}
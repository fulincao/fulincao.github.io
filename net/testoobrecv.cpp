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

    if(argc <= 2) {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
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

    int ret = bind(sock, (sockaddr*) &server_address, sizeof(server_address));
    assert(ret != -1);
    ret = listen(sock, 5);
    assert(ret != -1);

    sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    int connfd = accept(sock, (sockaddr*)&client, &client_addrlength);
    if(connfd < 0 ) {
        printf("errno is %d\n", connfd);
    }else {
        char buffer[BUFFER_SIZE];

        memset(buffer, '\0', BUFFER_SIZE);
        ret = recv(connfd, buffer, BUFFER_SIZE-1, 0);
        printf("got %d bytes of normal data '%s'\n", ret, buffer);

        memset(buffer, '\0', BUFFER_SIZE);
        ret = recv(connfd, buffer, BUFFER_SIZE-1, MSG_OOB);
        printf("got %d bytes of oob data '%s'\n", ret, buffer);

        memset(buffer, '\0', BUFFER_SIZE);
        ret = recv(connfd, buffer, BUFFER_SIZE-1, 0);
        printf("got %d bytes of normal data '%s'\n", ret, buffer);
        close(connfd);
    }
    close(sock);
    return 0;
}
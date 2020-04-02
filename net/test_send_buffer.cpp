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
        printf("usage: %s ip_address port_number send_buffer_size\n", basename(argv[0]));
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

    int sendbuf = atoi(argv[3]);
    int len = sizeof(sendbuf);

    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuf, sizeof(sendbuf));
    getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuf, (socklen_t*)&len);
    printf("the tcp send buffer size after setting is %d\n", len);



    if(connect(sock, (sockaddr*) &server_address, sizeof(server_address)) < 0 ) {
        printf("connection failed\n");
    }else {
        char buffer[BUFFER_SIZE];
        memset(buffer, 'a', sizeof(buffer));
        send(sock, buffer, sizeof(buffer), 0);
    }
    close(sock);
    return 0;
}
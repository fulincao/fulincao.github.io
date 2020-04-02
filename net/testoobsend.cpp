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

    if(connect(sock, (sockaddr*) &server_address, sizeof(server_address)) < 0 ) {
        printf("connection failed\n");
    }else {
        const char* oob_data = "abc";
        const char* normal_data = "123";
        send(sock, normal_data, strlen(normal_data), 0);
        send(sock, oob_data, strlen(oob_data), MSG_OOB);
        send(sock, normal_data, strlen(normal_data), 0);
    }
    close(sock);
    return 0;
}
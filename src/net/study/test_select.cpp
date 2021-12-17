//
// Created by cao on 2020/4/29.
//

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <errno.h>


int main(int argc, char* argv[]){

    int server = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr;
    bzero(&server_addr, 0);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    server_addr.sin_port = htons(12345);
    server_addr.sin_family = AF_INET;
    int ret = bind(server, (sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0){
        printf("bind fail\n");
        return -1;
    }

    ret = listen(server, 10);
    if(ret < 0) {
        printf("listen fail\n");
        return -1;
    }

    sockaddr_in client_addr;
    int client_length = sizeof(client_addr);
    int client = accept(server, (sockaddr*)&client_addr, (socklen_t*)&client_length);
    if(client < 0){
        printf("accept fail, errno is %d", errno);
        return -1;
    }

    fd_set readSets, exceptionSets;
    FD_ZERO(&readSets);
    FD_ZERO(&exceptionSets);

    char buffer[1024];

    while(1) {
        memset(buffer, 0, sizeof(buffer));
        // 每次select都要重置
        FD_SET(client, &readSets);
        FD_SET(client, &exceptionSets);
        ret = select(client+1, &readSets, NULL, &exceptionSets, NULL);
        if(ret < 0 ) {
            printf("select fail\n");
            break;
        }
        if(FD_ISSET(client, &readSets)){
            ret = recv(client, buffer, sizeof(buffer), 0);
            if(ret <= 0) break;
            printf("recv data is %s\n", buffer);
        } else if (FD_ISSET(client, &exceptionSets)) {
            ret = recv(client, buffer, sizeof(buffer), MSG_OOB);
            if(ret <= 0) break;
            printf("recv oob data is %s\n", buffer);
        }
    }
    close(client);
    close(server);



    return 0;
}
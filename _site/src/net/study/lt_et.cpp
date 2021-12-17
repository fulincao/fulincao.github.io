//
// Created by cao on 2020/4/30.
// lt/et 模式。 et是epoll的高效模式
//
#include <sys/socket.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <errno.h>

#define BUFFER_SIZE 10
#define MAX_EVENT_NUMBER 1024


int set_no_block(int fd) {
    int old_potion = fcntl(fd, F_GETFL);
    int new_potion = old_potion | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_potion);
    return new_potion;
}

void add_fd(int epfd, int fd, bool use_et) {
    epoll_event event;
    event.events = EPOLLIN;
    if(use_et) event.events |= EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
    set_no_block(fd);
}

void lt(epoll_event* events, int numbers, int epfd, int listened){
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < numbers; ++i) {
        int sock_fd = events[i].fd;
        if (sock_fd == listened) {
            sockaddr_in client_addr;
            __socklen_t addr_length = sizeof(client_addr);
            bzero(&client_addr, addr_length);
            int client_fd = accept(sock_fd, (sockaddr*)&client_addr, &addr_length);
            add_fd(epfd, client_fd, false);
        } else if (events[i].events & EPOLLIN){
            printf("have data read\n");
            memset(buffer, 0, BUFFER_SIZE);
            int ret = recv(sock_fd, buffer, BUFFER_SIZE, 0);
            if (ret < 0) {
                printf("recv error\n");
                close(sock_fd);
                continue;
            }
            printf("recv data is %s\n", buffer);
        }else{
            printf("something happened\n");
        }
    }

}

void et(epoll_event* events, int numbers, int epfd, int listened){
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < numbers; ++i) {
        int sock_fd = events[i].fd;
        if (sock_fd == listened) {
            sockaddr_in client_addr;
            __socklen_t addr_length = sizeof(client_addr);
            bzero(&client_addr, addr_length);
            int client_fd = accept(sock_fd, (sockaddr*)&client_addr, &addr_length);
            add_fd(epfd, client_fd, true);
        } else if (events[i].events & EPOLLIN){
            printf("event trigger once\n");

            while(1) {
                memset(buffer, 0, BUFFER_SIZE);
                int ret = recv(sock_fd, buffer, BUFFER_SIZE, 0);
                if (ret < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        printf("read later\n");
                        break;
                    }
                    close(sock_fd);
                    break;
                } else if (ret == 0) {
                    close(sock_fd);
                } else {
                    printf("recv data is %s\n", buffer);
                }

            }
        }else{
            printf("something happened\n");
        }
    }

}


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
    epoll_event events[MAX_EVENT_NUMBER];
    int epoll_fd = epoll_create(5);
    assert(epoll_fd != -1);
    add_fd(epoll_fd, server, true)

    while(1){
        int ret = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, -1);
        if(ret < 0){
            printf("epoll fail\n");
            break;
        }
        lt(events, ret, epoll_fd, server);
        // et(events, ret, epoll_fd, server);
    }


    return 0;
}
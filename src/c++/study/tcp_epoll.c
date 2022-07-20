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
#include <assert.h>

#define BUFFER_SIZE 1024
#define MAX_EVENT_NUMBER 1024


int set_no_block(int fd) {
    int old_potion = fcntl(fd, F_GETFL);
    int new_potion = old_potion | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_potion);
    return new_potion;
}

void add_fd(int epfd, int fd, int use_et) {
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    if(use_et) event.events |= EPOLLET;
    set_no_block(fd);
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
    
}

void lt(struct epoll_event* events, int numbers, int epfd, int listened){
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < numbers; ++i) {
        int sock_fd = events[i].data.fd;
        if (sock_fd == listened) {
            struct sockaddr_in client_addr;
            __socklen_t addr_length = sizeof(client_addr);
            bzero(&client_addr, addr_length);
            int client_fd = accept(sock_fd, (struct sockaddr*)&client_addr, &addr_length);
            add_fd(epfd, client_fd, 0);
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

static void et(struct epoll_event* events, int numbers, int epfd, int listened){
    char buffer[BUFFER_SIZE];

    for (int i = 0; i < numbers; ++i) {
        int sock_fd = events[i].data.fd;
        if (sock_fd == listened) {
            struct sockaddr_in client_addr;
            __socklen_t addr_length = sizeof(client_addr);
            bzero(&client_addr, addr_length);
            int client_fd = accept(sock_fd, (struct sockaddr*)&client_addr, &addr_length);
            printf("new client: %d\n", client_fd);
            add_fd(epfd, client_fd, 1);
        } else if (events[i].events & EPOLLIN){
            // printf("event trigger once\n");

            while(1) {
                memset(buffer, 0, BUFFER_SIZE);
                int ret = recv(sock_fd, buffer, BUFFER_SIZE, 0);
                // printf("%d %d %d %d\n", errno, ret, sock_fd, listened);
                if (ret < 0) {
                   
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // printf("read later\n");
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
        }else if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                !(events[i].events & EPOLLIN))
            {
              printf("epoll error\n");
              close (events[i].data.fd);
              return -1;
        }else{
            printf("something happened\n");
        }
    }

}



int main(int argc, char* argv[]){

    int server = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, 0);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    server_addr.sin_port = htons(12345);
    server_addr.sin_family = AF_INET;
    int ret = bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0){
        printf("bind fail\n");
        return -1;
    }

    ret = listen(server, 10);
    if(ret < 0) {
        printf("listen fail\n");
        return -1;
    }
    struct epoll_event events[MAX_EVENT_NUMBER];
    int epoll_fd = epoll_create(5);
    assert(epoll_fd != -1);
    printf("server id: %d\n", server);
    add_fd(epoll_fd, server, 1);

    while(1){
        int ret = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, -1);
        if(ret < 0){
            printf("epoll fail\n");
            break;
        }
        // lt(events, ret, epoll_fd, server);
        et(events, ret, epoll_fd, server);
    }


    return 0;
}
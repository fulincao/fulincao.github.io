//
// Created by cao on 2020/5/27.
//  chatting room server
//


#include <arpa/inet.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>


#define USER_LIMIT 5 /* user limit */
#define BUFFER_SIZE 64 // buffer size
#define FD_LIMIT 65535  // fd limit


struct client_data {
    sockaddr_in address;
    char* write_buf = NULL;
    char buf[BUFFER_SIZE];
};

int set_no_blocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return new_option;
}


int main(int argc, char* argv[]){

    const char* ip = "127.0.0.1";
    int port = 1234;

    int ret = 0;

    sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &address.sin_addr);

    int listened = socket(AF_INET, SOCK_STREAM, 0);
    assert(listened >=0 );

    ret = bind(listened, (sockaddr*)&address, sizeof(address));
    assert(ret != -1 );

    ret = listen(listened, 5);
    assert(ret != -1);

    // 直接用 fd 做下标映射数据
    client_data* users = new client_data[FD_LIMIT];

    // 记录socket
    pollfd fds[USER_LIMIT+1];

    for(int i = 1; i < USER_LIMIT+1; i++) {
        fds[i].fd = -1;
        fds[i].events = 0;
    }

    fds[0].fd = listened;
    fds[0].events = POLLIN | POLLERR;
    fds[0].revents = 0;

    int user_counter = 0;
    while(1) {
        ret = poll(fds, user_counter+1, -1);
        if (ret < 0){
            printf("poll failure\n");
            break;
        }
        for(int i = 0; i < user_counter+1; i++) {

            if( fds[i].fd == listened && fds[i].revents & POLLIN) {
                sockaddr_in client_address;
                socklen_t  addr_length = sizeof(client_address);
                int conn_fd = accept(listened, (sockaddr*)&client_address, &addr_length);
                if( conn_fd < 0 ) {
                    printf("errno is %d\n", errno);
                    continue;
                }

                if(user_counter + 1 > USER_LIMIT){
                    const char* info = "too many user\n";
                    printf("%s", info);
                    send(conn_fd, info, strlen(info), 0);
                    close(conn_fd);
                    continue;
                }

                user_counter += 1;
                users[conn_fd].address = client_address;
                set_no_blocking(conn_fd);

                fds[user_counter].fd = conn_fd;
                fds[user_counter].events = POLLIN | POLLERR | POLLRDHUP;
                fds[user_counter].revents = 0;
                printf("new client, now has %d user\n", user_counter);

            } else if( fds[i].revents & POLLRDHUP) {
                // 客户端关闭连接，服务器也关闭
                users[fds[i].fd] = users[fds[user_counter].fd];
                close(fds[i].fd);
                user_counter -= 1;
                i -= 1;
                printf("one client exit\n");

            } else if( fds[i].revents & POLLERR) {
                // 错误发生
                printf("get and error from %d\n", fds[i].fd);
                continue;
            } else if( fds[i].revents & POLLIN ) {
                int conn_fd = fds[i].fd;
                memset(users[conn_fd].buf, 0, BUFFER_SIZE);
                ret = recv(conn_fd, users[conn_fd].buf, BUFFER_SIZE-1, 0);
                printf("got client data: %s\n", users[conn_fd].buf);
                if( ret < 0 ) {
                    if(errno != EAGAIN) {
                        close(conn_fd);
                        users[conn_fd] = users[fds[user_counter].fd];
                        user_counter -= 1;
                        i -= 1;
                    }
                }else if ( ret > 0 ){
                    // 注册其他客户端的 写事件
                    for( int j = 1; j <= user_counter; j++) {
                        if( fds[j].fd == conn_fd) continue;
                        fds[j].events |= POLLOUT;
                        fds[j].events |= ~POLLIN;
                        users[fds[j].fd].write_buf = users[conn_fd].buf;
                    }
                }
            } else if (fds[i].revents & POLLOUT) {
                int conn_fd = fds[i].fd;
                if( ! users[conn_fd].write_buf) continue;
                ret = send(conn_fd, users[conn_fd].write_buf, strlen(users[conn_fd].write_buf), 0);
                users[conn_fd].write_buf = NULL;
                fds[i].events |= ~POLLOUT;
                fds[1].events |= POLLIN;
            }

        }

    }
    delete [] users;
    close(listened);
    return 0;
}
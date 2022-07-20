//
// Created by cao on 2020/4/9.
//    高效的数据回传，没有read/recv操作
//

#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    int ret = bind(sock, (sockaddr*)&addr, sizeof(addr));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);
    sockaddr_in client;
    socklen_t sz= sizeof(client);
    int conn_fd = accept(sock, (sockaddr*)&client, &sz);
    if(conn_fd < 0) {
        printf("connect error\n");
    }else{
        int pp[2];
        ret = pipe(pp);
        ret = splice(conn_fd, NULL, pp[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
        assert(ret != -1);
        ret = splice(pp[0], NULL, conn_fd, NULL, 32768, SPLICE_F_MOVE | SPLICE_F_MORE);
        assert(ret != -1);
        close(conn_fd);
    }
    close(sock);
    return 0;
}

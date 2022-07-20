//
// Created by cao on 2020/4/9.
// 测试 tee函数， 并实现系统的tee命令
//
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>


int main(int argc, char* argv[]) {

    if(argc != 2) {
        printf("param is not correct\n");
        return 1;
    }
    char* file_name = argv[1];
    int ffd = open(file_name, O_CREAT | O_WRONLY, 0666);
    assert(ffd > 0);

    int first_pipes[2];
    int second_pipes[2];

    int ret = pipe(first_pipes);
    assert(ret != -1);
    ret = pipe(second_pipes);
    assert(ret != -1);

//    printf("pipes fd %d %d %d %d\n", first_pipes[0], first_pipes[1], second_pipes[0], second_pipes[1]);

    ret = splice(STDIN_FILENO, NULL, first_pipes[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
    assert(ret != -1);
    ret = tee(first_pipes[0], second_pipes[1], 32768, SPLICE_F_NONBLOCK);
    assert(ret != -1);
    ret = splice(second_pipes[0], NULL, ffd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
//    printf("error: %d\n", errno);
//    perror("error");
    assert(ret != -1);
//    ret = splice(first_pipes[0], NULL, STDOUT_FILENO, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
//    assert(ret != -1);
    close(ffd);
    close(first_pipes[0]);
    close(first_pipes[1]);
    close(second_pipes[0]);
    close(second_pipes[1]);
    return 0;
}

//
// Created by cao on 2020/3/27.
//

#include <stdio.h>
#include "utils.h"

void byteorder() {
    // 共用地址
    union {
        short value;
        char union_bytes[sizeof(short)];
    }test;
    test.value = 0x0102;
    // 高位字节低地址 大端
    if( test.union_bytes[0] == 1 && test.union_bytes[1] == 2){
        printf("big endian\n");
    }else if (test.union_bytes[1] == 1 && test.union_bytes[0] == 2) {
        printf("little endian\n");
    }else{
        printf("unknown\n");
    }

}





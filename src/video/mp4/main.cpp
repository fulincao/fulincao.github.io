/// @file main.cpp
/// @brief Convert images to mp4 video file.
/// @author Zhang Qi (zhangqi@minieye.cc)
/// @date 2018-06-01
/// Copyright (C) 2018 - MiniEye INC.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "./MP4Writer.h"

static const char *usage = "Usage: %s [options] 1.jpg 2.jpg ...\n"
"\n"
"Options:\n"
"   -h, --help             Print this help message and exit\n"
"   -o, --output STRING    Output file name, eg: xxx.avi\n"
"   -F, --fps INT          Frames per second\n"
"   -W, --width INT        Width of output video\n"
"   -H, --height INT       Height of output video\n"
"\n";

int main(int argc, char **argv) {
    static struct option longopts[] = {
        {"help",   0, NULL, 'h'},
        {"output", 1, NULL, 'o'},
        {"fps",    1, NULL, 'F'},
        {"width",  1, NULL, 'W'},
        {"height", 1, NULL, 'H'},
    };

    int height = 0;
    int width = 0;
    int fps = 25;
    char* output = nullptr;
    int opt;
    while ((opt = getopt_long(argc, argv, "ho:W:H:F:", longopts, NULL)) != -1) {
        switch (opt) {
            case 'h':
                fprintf(stderr, usage, argv[0]);
                exit(EXIT_SUCCESS);
            case 'o':
                output = optarg;
                break;
            case 'F':
                fps = atoi(optarg);
                break;
            case 'W':
                width = atoi(optarg);
                break;
            case 'H':
                height = atoi(optarg);
                break;
            default:
                fprintf(stderr, usage, argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (output == nullptr) {
        fprintf(stderr, "Error: output file name unspecified\n");
        exit(EXIT_FAILURE);
    }

    std::vector<std::string> jpegs;        // 保存图片名称
    for (; optind < argc; optind++) {
        jpegs.push_back(argv[optind]);
    }

    if (jpegs.size() == 0) {
        fprintf(stderr, "Error: no input files\n");
        exit(EXIT_FAILURE);
    }

    if (width == 0) {
        fprintf(stderr, "Error: invalid value of width\n");
        exit(EXIT_FAILURE);
    }

    if (width == 0 || height == 0) {
        fprintf(stderr, "Error: invalid value of height\n");
        exit(EXIT_FAILURE);
    }

    std::ofstream ofs(output, std::ofstream::out | std::ofstream::binary);
    MP4Writer mp4(ofs, fps, width, height);

    const int MaxJPEGSize = 1024 * 1024;
    std::vector<char> buf(MaxJPEGSize);

    for (auto filename : jpegs) {
        FILE* fp = fopen(filename.c_str(), "rb");
        if (fp == nullptr) {
            fprintf(stderr, "Error: cannot open %s: %s\n",
                filename.c_str(), strerror(errno));
            exit(1);
        }

        ssize_t nb = fread(buf.data(), 1, buf.size(), fp);
        fclose(fp);
        mp4.Write(buf.data(), nb);
    }

    mp4.End();
    ofs.close();
    printf("Successfully written to %s\n", output);

    return 0;
}

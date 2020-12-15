/// @file MjpegWriter.cpp
/// @brief A Motion-JPEG Writer
/// @author pengquanhua (pengquanhua@minieye.cc)
/// @date 2018-1-15
/// Copyright (C) 2018 - MiniEye INC.

#include <stdio.h>
#include <string.h>
#include <iostream>
#include "./MjpegWriter.h"

#define fourCC(a, b, c, d) ( static_cast<int>(((unsigned char)(d) << 24) | \
                ((unsigned char)(c) << 16) | ((unsigned char)(b) << 8) |\
                 (unsigned char)(a)))

static const float NUM_MICROSEC_PER_SEC = 1000000.0f;  // 每秒有1000000纳秒
static const int STREAMS = 1;  // stream 个数
static const int AVIH_STRH_SIZE = 56;
static const int STRF_SIZE = 40;
static const int AVI_DWFLAG = 0x00000910;
static const int AVI_DWSCALE = 1;
static const int AVI_DWQUALITY = -1;
static const int AVI_BITS_PER_PIXEL = 24;
static const int AVI_BIPLANES = 1;
static const int JUNK_SEEK = 4096;
static const int AVIIF_KEYFRAME = 0x10;
static const int MAX_BYTES_PER_SEC = 15552000;
static const int SUG_BUFFER_SIZE = 1048576;

MjpegWriter::MjpegWriter(std::ostream& _os, int fps, int _width, int _height) :
    os(_os), mFPS(fps), width(_width), height(_height), mFrameNum(0) {
    StartWriteAVI();
    WriteStreamHeader();
}

int MjpegWriter::End() {
    EndWriteChunk();  // end LIST 'movi'
    WriteIndex();
    FinishWriteAVI();
    return 0;
}

int MjpegWriter::Write(void *pBuf, int pBufSize) {
    if (!WriteFrame(pBuf, pBufSize))
        return -2;
    return 1;
}

void MjpegWriter::StartWriteAVI() {
    StartWriteChunk(fourCC('R', 'I', 'F', 'F'));
    PutInt(fourCC('A', 'V', 'I', ' '));

    // hdrl
    StartWriteChunk(fourCC('L', 'I', 'S', 'T'));
    PutInt(fourCC('h', 'd', 'r', 'l'));
    PutInt(fourCC('a', 'v', 'i', 'h'));
    PutInt(AVIH_STRH_SIZE);
    PutInt(static_cast<int>(NUM_MICROSEC_PER_SEC / mFPS));
    PutInt(MAX_BYTES_PER_SEC);
    PutInt(0);
    PutInt(AVI_DWFLAG);
    FrameNumIndexes.push_back(os.tellp());
    PutInt(0);
    PutInt(0);
    PutInt(STREAMS);
    PutInt(SUG_BUFFER_SIZE);
    PutInt(width);
    PutInt(height);
    PutInt(0);
    PutInt(0);
    PutInt(0);
    PutInt(0);
}

void MjpegWriter::WriteStreamHeader() {
    // strh
    StartWriteChunk(fourCC('L', 'I', 'S', 'T'));
    PutInt(fourCC('s', 't', 'r', 'l'));
    PutInt(fourCC('s', 't', 'r', 'h'));
    PutInt(AVIH_STRH_SIZE);
    PutInt(fourCC('v', 'i', 'd', 's'));
    PutInt(fourCC('M', 'J', 'P', 'G'));
    PutInt(0);
    PutInt(0);
    PutInt(0);
    PutInt(AVI_DWSCALE);
    PutInt(mFPS);
    PutInt(0);
    FrameNumIndexes.push_back(os.tellp());
    PutInt(0);
    PutInt(SUG_BUFFER_SIZE);
    PutInt(AVI_DWQUALITY);
    PutInt(0);
    PutShort(0);
    PutShort(0);
    PutShort(width);
    PutShort(height);

    // strf
    StartWriteChunk(fourCC('s', 't', 'r', 'f'));
    PutInt(STRF_SIZE);                   // biSize
    PutInt(width);                       // biWidth
    PutInt(height);                      // biHeight
    PutShort(AVI_BIPLANES);              // biPlanes
    PutShort(AVI_BITS_PER_PIXEL);        // biBitCount
    PutInt(fourCC('M', 'J', 'P', 'G'));  // biCompression
    PutInt(width * height * 3);          // biSizeImage
    PutInt(0);
    PutInt(0);
    PutInt(0);
    PutInt(0);
    EndWriteChunk();  // end strf
    EndWriteChunk();  // end strl

    EndWriteChunk();  // end hdrl

    // JUNK
    StartWriteChunk(fourCC('J', 'U', 'N', 'K'));
    std::vector<char> junk(JUNK_SEEK - os.tellp(), '\0');
    os.write(junk.data(), junk.size());
    EndWriteChunk();  // end JUNK

    // movi
    StartWriteChunk(fourCC('L', 'I', 'S', 'T'));
    moviPointer = os.tellp();
    PutInt(fourCC('m', 'o', 'v', 'i'));
}

bool MjpegWriter::WriteFrame(void *pBuf, int pBufSize) {
    chunkPointer = os.tellp();
    StartWriteChunk(fourCC('0', '0', 'd', 'c'));
    // Frame data
    os.write((const char*)pBuf, pBufSize);
    WritePadding(16);
    FrameOffset.push_back(chunkPointer - moviPointer);
    FrameSize.push_back((int)os.tellp() - chunkPointer - 8);
    mFrameNum++;
    EndWriteChunk();  // end '00dc'
    return true;
}

void MjpegWriter::WriteIndex() {
    StartWriteChunk(fourCC('i', 'd', 'x', '1'));
    for (int i = 0; i < mFrameNum; i++) {
        PutInt(fourCC('0', '0', 'd', 'c'));
        PutInt(AVIIF_KEYFRAME);
        PutInt(FrameOffset[i]);
        PutInt(FrameSize[i]);
    }
    EndWriteChunk();  // End idx1
}

void MjpegWriter::WritePadding(int alignment) {
    int mod = os.tellp() % alignment;
    size_t pad_size = alignment - mod;
    if (pad_size) {
        PutChars(0, pad_size);
    }
}

void MjpegWriter::FinishWriteAVI() {
    // Record frames numbers to AVI Header
    uint32_t curPointer = os.tellp();
    while (!FrameNumIndexes.empty()) {
        os.seekp(FrameNumIndexes.back(), std::ios_base::beg);
        PutInt(mFrameNum);
        FrameNumIndexes.pop_back();
    }
    os.seekp(curPointer, std::ios_base::beg);
    EndWriteChunk();  // end RIFF
}

void MjpegWriter::PutInt(int elem) {
    os.write((const char*)&elem, sizeof elem);
}

void MjpegWriter::PutShort(int16_t elem) {
    os.write((const char*)&elem, sizeof elem);
}

void MjpegWriter::PutChars(char c, size_t n) {
    char buf[n];
    memset(buf, c, n);
    os.write(buf, n);
}

void MjpegWriter::StartWriteChunk(int fourcc) {
    uint32_t fpos;
    if (fourcc != 0) {
        PutInt(fourcc);
        fpos = os.tellp();
        PutInt(0);
    } else {
        fpos = os.tellp();
        PutInt(0);
    }
    AVIChunkSizeIndex.push_back(static_cast<int>(fpos));
}

void MjpegWriter::EndWriteChunk() {
    if (!AVIChunkSizeIndex.empty()) {
        uint32_t curPointer = os.tellp();
        os.seekp(AVIChunkSizeIndex.back(), std::ios_base::beg);
        int size = static_cast<int>(curPointer -\
                   (AVIChunkSizeIndex.back() + 4));
        PutInt(size);
        os.seekp(curPointer, std::ios_base::beg);
        AVIChunkSizeIndex.pop_back();
    }
}

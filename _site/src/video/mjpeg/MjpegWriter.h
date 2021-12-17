/// @file MjpegWriter.h
/// @brief A Motion-JPEG Writer
/// @author pengquanhua (pengquanhua@minieye.cc)
/// @date 2018-1-15
/// Copyright (C) 2018 - MiniEye INC.

#ifndef COMMON_MJPEG_MJPEGWRITER_H_
#define COMMON_MJPEG_MJPEGWRITER_H_

#include <stdint.h>
#include <vector>
#include <string>
#include <ostream>

class MjpegWriter {
 public:
    MjpegWriter(std::ostream& os, int fps, int width, int height);
    int Write(void *pBuf, int pBufSize);  // 写入一帧
    int End();

 private:
    std::ostream& os;
    int mFPS;
    int width, height, mFrameNum;
    int chunkPointer, moviPointer;
    std::vector<int> FrameOffset;
    std::vector<int> FrameSize;
    std::vector<int> AVIChunkSizeIndex;
    std::vector<int> FrameNumIndexes;

    void StartWriteAVI();
    void WriteStreamHeader();
    void WriteIndex();
    bool WriteFrame(void *pBuf, int pBufSize);
    void WritePadding(int alignment);
    void FinishWriteAVI();
    void PutInt(int elem);
    void PutShort(int16_t elem);
    void PutChars(char c, size_t n);
    void StartWriteChunk(int fourcc);
    void EndWriteChunk();
};

#endif  // COMMON_MJPEG_MJPEGWRITER_H_

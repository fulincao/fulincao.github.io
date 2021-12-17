/// @file MP4Writer.h
/// @brief A MP4 Writer
/// @author Zhang Qi (zhangqi@minieye.cc)
/// @date 2018-06-01
/// Copyright (C) 2018 - MiniEye INC.

#ifndef COMMON_MP4_MP4WRITER_H_
#define COMMON_MP4_MP4WRITER_H_

#include <stdint.h>
#include <vector>
#include <string>
#include <ostream>

class MP4Writer {
 public:
  MP4Writer(std::ostream& os, int fps, int width, int height);
  int Write(void* buf, size_t size);  // write a frame
  int End();

 private:
  std::ostream& os;
  int mFPS;
  int mWidth;
  int mHeight;
  int mFrameNum;
  std::vector<int> mFrameSizes;

  int GetDurationMs();
  int GetMdatSize();
  int GetMoovSize();
  int GetTrakSize();
  int GetMdiaSize();
  int GetMinfSize();
  int GetStblSize();
  int GetStszSize();
  void WriteBox_ftyp();
  void WriteBox_free();
  void WriteBox_moov();
  void WriteBox_mvhd();
  void WriteBox_trak();
  void WriteBox_tkhd();
  void WriteBox_edts();
  void WriteBox_mdia();
  void WriteBox_mdhd();
  void WriteBox_hdlr();
  void WriteBox_minf();
  void WriteBox_vmhd();
  void WriteBox_dinf();
  void WriteBox_dref();
  void WriteBox_stbl();
  void WriteBox_stsd();
  void WriteBox_stts();
  void WriteBox_stsc();
  void WriteBox_stsz();
  void WriteBox_stco();
  void WriteBox_udta();
  void WriteFrame(void* buf, size_t size);
  void WriteData(const char* buf, size_t size);
  void WriteStr(const char* str);
  void WriteInt8(int8_t num);
  void WriteInt16(int16_t num);
  void WriteInt24(int32_t num);
  void WriteInt32(int32_t num);
};

#endif  // COMMON_MP4_MP4WRITER_H_

/// @file MP4Writer.cpp
/// @brief A MP4 Writer
/// @author Zhang Qi (zhangqi@minieye.cc)
/// @date 2018-06-01
/// Copyright (C) 2018 - MiniEye INC.

#include <stdio.h>
#include <string.h>
#include "./MP4Writer.h"

static const int BoxHeadSize = 8;
static const int BoxSize_ftyp = 28;
static const int BoxSize_free = 8;
static const int BoxSize_mvhd = 108;
static const int BoxSize_tkhd = 92;
static const int BoxSize_edts = 36;
static const int BoxSize_mdhd = 32;
static const int BoxSize_hdlr = 45;
static const int BoxSize_vmhd = 20;
static const int BoxSize_dinf = 36;
static const int BoxSize_dref = 28;
static const int BoxSize_stsd = 146;
static const int BoxSize_stts = 24;
static const int BoxSize_stsc = 28;
static const int BoxSize_stco = 20;
static const int BoxSize_udta = 98;

MP4Writer::MP4Writer(std::ostream& _os, int fps, int width, int height) :
  os(_os), mFPS(fps), mWidth(width), mHeight(height), mFrameNum(0) {
}

int MP4Writer::Write(void* buf, size_t size) {
  if (mFrameNum == 0) {
    WriteBox_ftyp();
    WriteBox_free();

    // mdat header
    WriteInt32(0);  // place holder
    WriteStr("mdat");
  }

  WriteFrame(buf, size);
  mFrameNum += 1;
  mFrameSizes.push_back(size);
  return 0;
}

int MP4Writer::End() {
  // fix box mdat's size
  uint32_t curPointer = os.tellp();
  os.seekp(BoxSize_ftyp + BoxSize_free, std::ios_base::beg);
  WriteInt32(GetMdatSize());
  os.seekp(curPointer, std::ios_base::beg);

  WriteBox_moov();
  return 0;
}

int MP4Writer::GetDurationMs() {
  return mFrameNum * 1000 / mFPS;
}

int MP4Writer::GetMdatSize() {
  int size = BoxHeadSize;
  for (auto frame_size : mFrameSizes) {
    size += frame_size;
  }

  return size;
}

int MP4Writer::GetMoovSize() {
  return BoxHeadSize + BoxSize_mvhd + GetTrakSize() + BoxSize_udta;
}

int MP4Writer::GetTrakSize() {
  return BoxHeadSize + BoxSize_tkhd + BoxSize_edts + GetMdiaSize();
}

int MP4Writer::GetMdiaSize() {
  return BoxHeadSize + BoxSize_mdhd + BoxSize_hdlr + GetMinfSize();
}

int MP4Writer::GetMinfSize() {
  return BoxHeadSize + BoxSize_vmhd + BoxSize_dinf + GetStblSize();
}

int MP4Writer::GetStblSize() {
  return BoxHeadSize + BoxSize_stsd + BoxSize_stts + BoxSize_stsc +
    GetStszSize() + BoxSize_stco;
}

int MP4Writer::GetStszSize() {
  return BoxHeadSize + 1 + 3 + 4 + 4 + mFrameNum * 4;
}

void MP4Writer::WriteBox_ftyp() {
  WriteInt32(BoxSize_ftyp);
  WriteStr("ftyp");

  WriteStr("isom");
  WriteInt32(0x00000200);
  WriteStr("isom");
  WriteStr("iso2");
  WriteStr("mp41");
}

void MP4Writer::WriteBox_free() {
  WriteInt32(BoxSize_free);
  WriteStr("free");
}

void MP4Writer::WriteBox_moov() {
  WriteInt32(GetMoovSize());
  WriteStr("moov");

  WriteBox_mvhd();
  WriteBox_trak();
  WriteBox_udta();
}

void MP4Writer::WriteBox_mvhd() {
  WriteInt32(BoxSize_mvhd);
  WriteStr("mvhd");

  WriteInt8(0);   // version
  WriteInt24(0);  // flags
  WriteInt32(0);  // creation_time
  WriteInt32(0);  // modification_time

  int time_scale = 1000;
  int duration_ms = GetDurationMs();
  int duration_scale = time_scale * duration_ms / 1000;
  WriteInt32(time_scale);  // time_scale
  WriteInt32(duration_scale);  // duration

  WriteInt32(0x00010000);  // playback_speed = 65536
  WriteInt16(0x0100);  // volume = 65536
  WriteData("\0\0\0\0\0\0\0\0\0\0", 10);  // reserved, 10 bytes

  char matrix_structure[36] = {
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00,
  };

  WriteData(matrix_structure, 36);  // matrix_structure, 36 bytes

  WriteInt32(0);  // preview_start_time
  WriteInt32(0);  // preview_duration
  WriteInt32(0);  // poster_time
  WriteInt32(0);  // selection_time
  WriteInt32(0);  // selection_duration
  WriteInt32(0);  // current_time
  WriteInt32(2);  // track_id
}

void MP4Writer::WriteBox_trak() {
  WriteInt32(GetTrakSize());
  WriteStr("trak");

  WriteBox_tkhd();
  WriteBox_edts();
  WriteBox_mdia();
}

void MP4Writer::WriteBox_tkhd() {
  WriteInt32(BoxSize_tkhd);
  WriteStr("tkhd");

  WriteInt8(0);   // version
  WriteInt24(0x00003);  // flags
  WriteInt32(0);  // creation_time
  WriteInt32(0);  // modification_time
  WriteInt32(1);  // track_id
  WriteInt32(0);  // reserved, 4 bytes

  int time_scale = 1000;
  int duration_ms = GetDurationMs();
  int duration_scale = time_scale * duration_ms / 1000;
  WriteInt32(duration_scale);  // duration

  WriteInt32(0);  // reserved, 8 bytes
  WriteInt32(0);  // (cont.)
  WriteInt16(0);  // layer
  WriteInt16(0);  // alternate_group
  WriteInt16(0);  // volume
  WriteInt16(0);  // reserved, 2 bytes

  char matrix_structure[36] = {
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00,
  };

  WriteData(matrix_structure, 36);  // matrix_structure, 36 bytes

  WriteInt16(mWidth);  // track_width, 32bit fixed point decimal
  WriteInt16(0);       // (cont.)
  WriteInt16(mHeight);  // track_width, 32bit fixed point decimal
  WriteInt16(0);       // (cont.)
}

void MP4Writer::WriteBox_edts() {
  WriteInt32(BoxSize_edts);
  WriteStr("edts");

  unsigned char data[28] = {
    0x00, 0x00, 0x00, 0x1c, 0x65, 0x6c, 0x73, 0x74,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x13, 0x88, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00,
  };

  WriteData((const char*)data, sizeof data);
}

void MP4Writer::WriteBox_mdia() {
  WriteInt32(GetMdiaSize());
  WriteStr("mdia");

  WriteBox_mdhd();
  WriteBox_hdlr();
  WriteBox_minf();
}

void MP4Writer::WriteBox_mdhd() {
  WriteInt32(BoxSize_mdhd);
  WriteStr("mdhd");

  WriteInt8(0);   // version
  WriteInt24(0);  // flags
  WriteInt32(0);  // creation_time
  WriteInt32(0);  // modification_time

  int time_scale = 12288;
  int duration_ms = GetDurationMs();
  int duration_scale = time_scale * duration_ms / 1000;
  WriteInt32(time_scale);  // time_scale
  WriteInt32(duration_scale);  // duration

  WriteInt16(0x55c4);  // language
  WriteInt16(0);  // quality
}

void MP4Writer::WriteBox_hdlr() {
  WriteInt32(BoxSize_hdlr);
  WriteStr("hdlr");

  WriteInt8(0);   // version
  WriteInt24(0);  // flags
  WriteInt32(0x00000000);  // component_type
  WriteStr("vide");  // component_subtype
  WriteData("\x00\x00\x00\x00", 4);  // component_manufacturer
  WriteData("\x00\x00\x00\x00", 4);  // component_flags
  WriteData("\x00\x00\x00\x00", 4);  // component_flags_mask
  WriteData("VideoHandler\x00", 13);  // component_name
}

void MP4Writer::WriteBox_minf() {
  WriteInt32(GetMinfSize());
  WriteStr("minf");

  WriteBox_vmhd();
  WriteBox_dinf();
  WriteBox_stbl();
}

void MP4Writer::WriteBox_vmhd() {
  WriteInt32(BoxSize_vmhd);
  WriteStr("vmhd");

  WriteInt8(0);   // version
  WriteInt24(0x000001);  // flags
  WriteInt16(0);  // graphic_mode
  WriteInt16(0);  // mode_color_r
  WriteInt16(0);  // mode_color_g
  WriteInt16(0);  // mode_color_b
}

void MP4Writer::WriteBox_dinf() {
  WriteInt32(BoxSize_dinf);
  WriteStr("dinf");

  WriteBox_dref();
}

void MP4Writer::WriteBox_dref() {
  WriteInt32(BoxSize_dref);
  WriteStr("dref");

  WriteInt8(0);   // version
  WriteInt24(0);  // flags
  WriteInt32(1);  // entry_count

  // entries[0]
  WriteInt32(12);    // size
  WriteStr("url ");  // type
  WriteInt8(0);   // version
  WriteInt24(1);  // flags
}

void MP4Writer::WriteBox_stbl() {
  WriteInt32(GetStblSize());
  WriteStr("stbl");

  WriteBox_stsd();
  WriteBox_stts();
  WriteBox_stsc();
  WriteBox_stsz();
  WriteBox_stco();
}

void MP4Writer::WriteBox_stsd() {
  WriteInt32(BoxSize_stsd);
  WriteStr("stsd");

  WriteInt8(0);   // version
  WriteInt24(0);  // flags
  WriteInt32(1);  // entry_count

  // entries[1]
  WriteInt32(130);  // sample_description_size
  WriteStr("mp4v");  // data_format
  WriteData("\x00\x00\x00\x00\x00\x00", 6);  // reserved
  WriteInt16(1);  // data_reference_index

  unsigned char data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x80, 0x01, 0x68, 0x00, 0x48, 0x00, 0x00,
    0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x18, 0xFF, 0xFF, 0x00, 0x00,
    0x00, 0x2c, 0x65, 0x73, 0x64, 0x73, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x80, 0x80, 0x80, 0x1b, 0x00,
    0x01, 0x00, 0x04, 0x80, 0x80, 0x80, 0x0d, 0x6c,
    0x11, 0x00, 0x00, 0x00, 0x00, 0x14, 0x3b, 0xEC,
    0x00, 0x14, 0x3b, 0xec, 0x06, 0x80, 0x80, 0x80,
    0x01, 0x02,
  };

  WriteData((const char*)data, 114);
}

void MP4Writer::WriteBox_stts() {
  WriteInt32(BoxSize_stts);
  WriteStr("stts");

  WriteInt8(0);   // version
  WriteInt24(0);  // flags
  WriteInt32(1);  // entry_count

  // entries[1]
  WriteInt32(mFrameNum);  // sample_count
  WriteInt32(12288 / mFPS);  // sample_duration
}

void MP4Writer::WriteBox_stsc() {
  WriteInt32(BoxSize_stsc);
  WriteStr("stsc");

  WriteInt8(0);   // version
  WriteInt24(0);  // flags
  WriteInt32(1);  // entry_count

  // entries[1]
  WriteInt32(1);  // first_chunk
  WriteInt32(mFrameNum);  // samples_per_chunk
  WriteInt32(1);  // sample_description
}

void MP4Writer::WriteBox_stsz() {
  WriteInt32(GetStszSize());
  WriteStr("stsz");

  WriteInt8(0);   // version
  WriteInt24(0);  // flags
  WriteInt32(0);  // sample_size
  WriteInt32(mFrameNum);  // sample_count

  for (int i = 0; i < mFrameNum; i += 1) {
    WriteInt32(mFrameSizes[i]);  // sample_size
  }
}

void MP4Writer::WriteBox_stco() {
  WriteInt32(BoxSize_stco);
  WriteStr("stco");

  WriteInt8(0);   // version
  WriteInt24(0);  // flags
  WriteInt32(1);  // chunk_offset_count

  // entries[0]
  int chunk_offset = BoxSize_ftyp + BoxSize_free + BoxHeadSize;
  WriteInt32(chunk_offset);
}

void MP4Writer::WriteBox_udta() {
  WriteInt32(BoxSize_udta);
  WriteStr("udta");

  unsigned char data[] = {
    0x00, 0x00, 0x00, 0x5a, 0x6d, 0x65, 0x74, 0x61,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21,
    0x68, 0x64, 0x6c, 0x72, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x6d, 0x64, 0x69, 0x72,
    0x61, 0x70, 0x70, 0x6c, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x2d, 0x69, 0x6c, 0x73, 0x74, 0x00, 0x00, 0x00,
    0x25, 0xa9, 0x74, 0x6f, 0x6f, 0x00, 0x00, 0x00,
    0x1d, 0x64, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x4c, 0x61, 0x76,
    0x66, 0x35, 0x37, 0x2e, 0x34, 0x31, 0x2e, 0x31,
    0x30, 0x30,
  };

  WriteData((const char*)data, 90);
}

void MP4Writer::WriteFrame(void* buf, size_t size) {
  WriteData(reinterpret_cast<const char*>(buf), size);
}

void MP4Writer::WriteData(const char* buf, size_t size) {
  os.write(buf, size);
}

void MP4Writer::WriteStr(const char* str) {
  WriteData(str, strlen(str));
}

void MP4Writer::WriteInt8(int8_t num) {
  const char* ptr = reinterpret_cast<const char*>(&num);
  char buf[1] = {ptr[0]};
  WriteData(buf, 1);
}

void MP4Writer::WriteInt16(int16_t num) {
  const char* ptr = reinterpret_cast<const char*>(&num);
  char buf[2] = {ptr[1], ptr[0]};
  WriteData(buf, 2);
}

void MP4Writer::WriteInt24(int32_t num) {
  const char* ptr = reinterpret_cast<const char*>(&num);
  char buf[3] = {ptr[2], ptr[1], ptr[0]};
  WriteData(buf, 3);
}

void MP4Writer::WriteInt32(int32_t num) {
  const char* ptr = reinterpret_cast<const char*>(&num);
  char buf[4] = {ptr[3], ptr[2], ptr[1], ptr[0]};
  WriteData(buf, 4);
}

"""
https://blog.csdn.net/houxiaoni01/article/details/84341885
https://www.cnblogs.com/ManMonth/archive/2010/02/01/1661113.html
"""

AVIH_STRH_SIZE = 56
MAX_BYTES_PER_SEC = 15552000
AVI_DWFLAG = 2320
NSTREAMS = 1
SUG_BUFFER_SIZE = 1048576
AVI_DWSCALE = 1
AVI_DWQUALITY = -1
STRF_SIZE = 40
AVI_BIPLANES = 1
AVI_BITS_PER_PIXEL = 24
AVIF_KEYFRAME = 16

class MJPEGWriter(object):
    video_path = None
    fp = None
    fsize = 0
    width = 0
    height = 0
    fps = 0
    wcnt = 0
    # riff_size_idx = 0
    frame_num_idxes = []
    movie_idx = 0
    frame_num = 0
    frame_offset = []
    frame_size = []
    chunk_idxes = []

    def __init__(self, video_path, width, height, fps):
        self.init_video(video_path, width, height, fps)

    def init_video(self, video_path, width, height, fps):
        self.video_path = video_path
        self.fp = open(video_path, 'wb')
        self.width = width
        self.height = height
        self.fps = fps
        # self.riff_size_idx = 0
        self.frame_num_idxes = []
        self.movie_idx = 0
        self.frame_num = 0
        self.frame_offset = []
        self.frame_size = []
        self.chunk_idxes = []

    def _put(self, b):
        if self.fp is not None:
            self.fp.write(b)
            self.wcnt += len(b)

    def _put_at(self, p, b):
        if self.fp is not None:
            currentfp = self.fp.tell()
            self.fp.seek(p)
            self.fp.write(b)
            self.fp.seek(currentfp)

    def _padding(self, ali):
        mod = self.fp.tell() % ali
        if mod == 0:
            return
        pad_size = ali - mod
        for i in range(pad_size):
            self._put(b'\x00')

    def _chunk(self, b):
        self._put(b)
        self.chunk_idxes.append(self.fp.tell())
        self._put(b'\x00\x00\x00\x00')

    def _end_chunk(self):
        cpos = self.fp.tell()
        fpos = self.chunk_idxes.pop()
        self.fp.seek(fpos)
        self._put((cpos-fpos-4).to_bytes(4, 'little'))
        self.fp.seek(cpos)

    # def _start_chunk(self):
    def write_header(self):
        # RIFF
        self._chunk(b'RIFF') ## 块类型avi

        self._put(b'AVI\x20')
        self._put(b'LIST')
        self._put(b'\xdc\x00\x00\x00')

        self._put(b'hdrl') # 嵌套一个avih 和 一个或多个strl
        # avih
        self._put(b'avih')
        self._put(AVIH_STRH_SIZE.to_bytes(4, 'little'))  ## avih size = 56
        self._put(int(1000000/self.fps).to_bytes(4, 'little')) # 帧间间隔 us
        self._put(MAX_BYTES_PER_SEC.to_bytes(4, 'little')) # 最大输出速率
        self._put(b'\x00\x00\x00\x00')  # 记录快长度为此值的倍数,通常2048
        self._put(AVI_DWFLAG.to_bytes(4, 'little')) # avi文件的特殊属性,包含标志字,比如:有无索引快,是否包含版权信息
        self.frame_num_idxes.append(self.fp.tell())
        self._put(b'\x00\x00\x00\x00') # 数据帧的总数, (后面会更改)
        self._put(b'\x00\x00\x00\x00') # 播放前需要的帧数
        self._put(NSTREAMS.to_bytes(4, 'little')) # 流的种类 = 1 
        self._put(SUG_BUFFER_SIZE.to_bytes(4, 'little')) # 建议的缓冲区大小,通常为一帧的大小
        self._put(self.width.to_bytes(4, 'little')) # 图像宽
        self._put(self.height.to_bytes(4, 'little')) # 图像高
        self._put(b'\x00\x00\x00\x00')  # 保留4字节 * 4 ( dwscale, dwrate, dwstart, dwlength)
        self._put(b'\x00\x00\x00\x00')
        self._put(b'\x00\x00\x00\x00')
        self._put(b'\x00\x00\x00\x00')

        # strl 至少包含一个strh 和一个strf
        # strh
        self._put(b'LIST')
        self._put(b'\x74\x00\x00\x00')
        self._put(b'strl')
        self._put(b'strh')
        self._put(AVIH_STRH_SIZE.to_bytes(4, 'little')) # strh size = 56
        self._put(b'vids') # vids视频流 auds 音视频流
        self._put(b'MJPG') # 编码解码算法
        self._put(b'\x00\x00\x00\x00') # 数据流属性 
        self._put(b'\x00\x00\x00\x00') # 优先级,音频代号 (各两字节)
        self._put(b'\x00\x00\x00\x00') # 用于interlaced文件，开始播放前所需帧数
        self._put(AVI_DWSCALE.to_bytes(4, 'little')) # dwscale视频每帧大小或者音频采样大小
        self._put(self.fps.to_bytes(4, 'little')) # dwrate每秒采样数
        self._put(b'\x00\x00\x00\x00') # dwstart 数据流开始播放位置
        self.frame_num_idxes.append(self.fp.tell())
        self._put(b'\x00\x00\x00\x00') # 数据流的数据量 dwscale为单位(写入更改)
        self._put(SUG_BUFFER_SIZE.to_bytes(4, 'little')) # 建议缓冲区大小
        self._put(AVI_DWQUALITY.to_bytes(4, 'little', signed=True)) # 解压缩质量参数,越大越好
        self._put(b'\x00\x00\x00\x00') # 音频采样大小

        self._put(b'\x00\x00\x00\x00') # rcFrame ( short int left, top, right bottom) 
        self._put(self.width.to_bytes(2, 'little'))
        self._put(self.height.to_bytes(2, 'little')) # 即视频数据所占的矩形
        # strf
        self._put(b'strf')
        self._put(STRF_SIZE.to_bytes(4, 'little')) # strf size = 40
        self._put(STRF_SIZE.to_bytes(4, 'little')) # dwsize
        self._put(self.width.to_bytes(4, 'little')) # dwwidth
        self._put(self.height.to_bytes(4, 'little')) # dwheight
        self._put(AVI_BIPLANES.to_bytes(2, 'little')) # wplanes
        self._put(AVI_BITS_PER_PIXEL.to_bytes(2, 'little')) # wbitcount
        self._put(b'MJPG') # dwcompression
        self._put((self.width*self.height*3).to_bytes(4, 'little')) # dwSizeImage
        self._put(b'\x00\x00\x00\x00') # dwXPelsPerMeter
        self._put(b'\x00\x00\x00\x00') # dwYPelsPerMeter
        self._put(b'\x00\x00\x00\x00') # dwCLrUsed
        self._put(b'\x00\x00\x00\x00') # dwClrImportant

        # info
        self._put(b'LIST')
        self._put(b'\x14\x00\x00\x00')
        self._put(b'odml')
        self._put(b'dmlh')
        self._put(b'\x08\x00\x00\x00')
        self.frame_num_idxes.append(self.fp.tell())
        self._put(b'\x00\x00\x00\x00')
        self._put(b'\x00\x00\x00\x00')
        # JUNK
        self._put(b'JUNK')
        self._put(b'\x08\x0f\x00\x00')
        junk = bytes(0x0f08)
        self._put(junk)

        # movi
        self._chunk(b'LIST')
        # self._put(b'LIST')
        # self._put(b'\x00\x00\x00\x00') # size modify later
        self.movie_idx = self.fp.tell()
        self._put(b'movi')
        # 接frame

        # index 块 结束是写入 offest 基于movi的index

    # 写入jpg数据流
    def write_frame(self, jpg):
        if len(jpg) == 0:
           print('write frame error: jpg len is 0.')
        cptr = self.fp.tell()
        self._chunk(b'00dc') # ##dc,其中##表示所属流的序号, dc压缩视频帧,db未压缩(rgb),wb(音频未压缩wave),wc(音频压缩wave)
                            # pc：改用新的调色板。（新的调色板使用一个数据结构AVIPALCHANGE来定义。如果一个流的调色板中途可能改变，
                            # 则应在这个流格式的描述中，也就是AVISTREAMHEADER结构的dwFlags中包含一个AVISF_VIDEO_PALCHANGES标记。）
        # _chunk函数后面跟了一个四字节表示接下来数据的大小 
        self._put(jpg)
        self._padding(4) # 4字节对齐
        self.frame_offset.append(cptr-self.movie_idx) # 计算index与movi的offest
        self.frame_size.append(self.fp.tell()-cptr-8) # 计算帧数据大小
        self.frame_num += 1
        self._end_chunk() # 计算_chunk 到此时的数据大小

    def write_index(self):
        self._chunk(b'idx1') # fourcc 必须为 idx1
        for i in range(self.frame_num): 
            self._put(b'00dc') # 同上
            self._put(AVIF_KEYFRAME.to_bytes(4, 'little')) # dwflags 表明是否关键帧
            self._put(self.frame_offset[i].to_bytes(4, 'little')) # 偏移量
            self._put(self.frame_size[i].to_bytes(4, 'little')) # 大小
        self._end_chunk()

    def flush(self):
        if self.fp:
            self.fp.flush()

    def finish_video(self):
        if not self.fp:
            return
        self._end_chunk()  # movi 块大小
        self.write_index()
        cpos = self.fp.tell()
        for idx in self.frame_num_idxes: # 更新总帧数
            self.fp.seek(idx)
            self._put(self.frame_num.to_bytes(4, 'little'))
        self.fp.seek(cpos)

        self._end_chunk()  # RIFF 块大小
        self.fp.flush()
        self.fp.close()

    def release(self):
        self.finish_video()
# - * - coding: utf-8 - * -
import sys
import numpy as np
import matplotlib.pyplot as plt
import wave
import math
import librosa
import scipy

plt.figure(dpi=100)  # 将显示的所有图分辨率调高

class MyWave:

    def __init__(self, wavpath: str) -> None:

        self.wavpath = wavpath
        self.wav_fp = open(wavpath, "rb")
        self.readheader()

    def readheader(self):
        """https://docs.fileformat.com/audio/wav/"""
        self.wav_fp.seek(0)
        self.file_kind_marker = str(self.wav_fp.read(4), encoding="utf-8")
        self.file_size = int.from_bytes(self.wav_fp.read(4), byteorder="little")
        self.file_type_header = str(self.wav_fp.read(4), encoding="utf-8")
        self.format_chunk_marker = str(self.wav_fp.read(4), encoding="utf-8")
        self.length_above_format_data = int.from_bytes(
            self.wav_fp.read(4), byteorder="little")
        self.format = int.from_bytes(self.wav_fp.read(2), byteorder="little")
        self.channels = int.from_bytes(self.wav_fp.read(2), byteorder="little")
        self.sample_rate = int.from_bytes(
            self.wav_fp.read(4), byteorder="little")
        self.byte_rate = int.from_bytes(self.wav_fp.read(4), byteorder="little")
        self.block_align = int.from_bytes(
            self.wav_fp.read(2), byteorder="little")
        self.sample_width = int.from_bytes(
            self.wav_fp.read(2), byteorder="little") // 8
        self.data_chunk_marker = str(self.wav_fp.read(4), encoding="utf-8")
        self.data_size = int.from_bytes(self.wav_fp.read(4), byteorder="little")

    def getparams(self):
        return self.channels, self.sample_width, self.sample_rate, self.data_size // self.sample_width

    def readframes(self):
        self.wav_fp.seek(44)
        data = self.wav_fp.read()
        fmt = "<i%d" % (self.sample_width)
        return np.frombuffer(data, dtype=fmt)

    def normalize(self, data):
        scale = 1.0 / (1 << (8 * self.sample_width - 1))
        return data * scale

    def cal_spl(self, data, seg=1024):
        '''
        计算声压 db 
        https://zh.m.wikipedia.org/zh-hans/%E5%88%86%E8%B2%9D
        https://en.wikipedia.org/wiki/Sound_pressure
        '''
        data_len = len(data)
        frame_num = int(math.ceil(data_len / seg))
        volume = np.zeros((frame_num, 1))
        for i in range(frame_num):
            curFrame = data[i*seg: i*seg+seg]
            curFrame = curFrame - np.mean(curFrame)  # zero-justified
            # volume[i] = 10*np.log10(np.sum(curFrame*curFrame))
            prms = np.sqrt(np.mean(curFrame * curFrame))
            pref = 20e-6  # 空气中 20微pa，水中 1微pa
            volume[i] = 20 * np.log10(prms / pref)
        return volume

    def dft(self, x):
        """
        离散傅立叶变换
        https://zh.wikipedia.org/wiki/%E7%A6%BB%E6%95%A3%E5%82%85%E9%87%8C%E5%8F%B6%E5%8F%98%E6%8D%A2
        https://zh.wikipedia.org/wiki/%E5%BF%AB%E9%80%9F%E5%82%85%E9%87%8C%E5%8F%B6%E5%8F%98%E6%8D%A2
        """
        N = len(x)
        print("dft data size: ", N)
        # 循环方式
        y = np.zeros(N, dtype="complex_")
        for k in range(N):
            t0 = x * np.exp(-2j * np.pi * k * np.arange(N)/N)
            # t1 = x * np.power(-1, -2 * k * np.arange(n)/n)  e^(i * pi) = -1 的替换，没有虚数
            y[k] = np.sum(t0)

        # 矩阵方式 虽然优雅，存消耗极大O(n*n)
        # n = np.arange(N)
        # k = n.reshape((N, 1))
        # e = np.exp(-2j * np.pi * k * n / N)
        # y = e @ x
        return y

    def fft(self, x: np.ndarray):
        """
        https://pythonnumericalmethods.berkeley.edu/notebooks/chapter24.03-Fast-Fourier-Transform.html
        https://jakevdp.github.io/blog/2013/08/28/understanding-the-fft/
        
        A recursive implementation of 
        the 1D Cooley-Tukey FFT, the 
        input should have a length of 
        power of 2. 
        """
        N = len(x)
        if N == 1:
            return x
        else:
            X_even = self.fft(x[::2])
            X_odd = self.fft(x[1::2])
            factor = \
                np.exp(-2j*np.pi*np.arange(N) / N)

            X = np.concatenate(
                [X_even+factor[:int(N/2)]*X_odd,
                 X_even+factor[int(N/2):]*X_odd])
            return X

    def cal_spectrum(self, data, seg=1024):
        """
        计算频谱
        """
        # print("dft begin ")
        # fft_y = self.fft(data)
        # fft_y = self.dft(data)
        fft_y = scipy.fft.fft(data)
        fft_y = self.cal_spl(fft_y, seg=seg)
        return fft_y

    def __del__(self):
        self.wav_fp.close()

    def __str__(self) -> str:
        vars_dict = vars(self)
        res = []
        for key in vars_dict:
            res.append(str(key) + ":" + str(vars_dict[key]))
        return "\n".join(res)

def test_wave():
    sys.argv.append(
        "/xurban/temp_data/test_microphone_structure/1/inner_channel_0.wav")
    mw = MyWave(sys.argv[1])
    print(mw)

    wav_data = mw.readframes()
    amplitude = mw.normalize(wav_data)

    nchannels, sampwidth, framerate, nframes = mw.getparams()
    time1 = np.arange(0, len(amplitude)) * (1.0 / framerate)

    seg = int(20e-3 * framerate)  # 20ms 50ms 100ms 常用计算区间
    volume = mw.cal_spl(amplitude, seg=seg)
    time2 = np.arange(0, len(volume)) * seg * 1.0 / framerate

    test_sz = 1024
    fft_y = mw.cal_spectrum(amplitude[:test_sz], seg)
    fft_2y = mw.fft(amplitude[:test_sz])
    fft_3y = mw.dft(amplitude[:test_sz])

    print(np.allclose(fft_2y, fft_3y))

    fft_x = np.arange(0, len(fft_y)) * seg * framerate / len(amplitude)

    volume2 = librosa.amplitude_to_db(amplitude, ref=20e-6, top_db=200)
    for i in range(int(math.ceil(len(volume2) / seg))):
        volume2[i] = np.mean(volume2[i*seg: i*seg+seg])
    volume2 = volume2[:int(math.ceil(len(volume2) / seg))]

    plt.subplot(311)
    plt.plot(time1, amplitude)
    plt.subplot(312)
    plt.plot(time2, volume, label="user")
    plt.plot(time2, volume2, label="librosa")
    plt.legend()

    plt.subplot(313)
    plt.plot(fft_x, fft_y)
    plt.plot(fft_x, fft_2y, label="fft")
    plt.plot(fft_x, fft_3y, label="dft")
    plt.legend()
    plt.show()

if __name__ == '__main__':
    test_wave()

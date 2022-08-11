# - * - coding: utf-8 - * -
import sys
import matplotlib 
import numpy as np
import matplotlib.pyplot as plt
import scipy
from scipy.fft import fft, fftfreq
import wave
import math
import librosa
import scipy.signal 

wavepath = "test.wav"


def read_data():
    # 打开WAV文档
    f = wave.open(wavepath, "r")
    # 读取格式信息
    # (nchannels, sampwidth, framerate, nframes, comptype, compname)
    params = f.getparams()
    nchannels, sampwidth, framerate, nframes = params[:4]
    # 读取波形数据
    str_data = f.readframes(nframes)
    print(params)
    f.close()
    fmt = "<i%d" % (sampwidth)
    # 将波形数据转换为数组
    x = np.frombuffer(str_data, dtype=fmt) 
    
    # normailize 
    x = 1.0 * x / (1 << (8 * sampwidth - 1))
    f.close()
    return framerate, sampwidth, x

def save_data(frames_arr, filepath, sample_width, sample_rate, n_channels=1):

    frames = frames_arr * ( 1 << (8 * sample_width - 1))
    fmt = "<i%d" % (sample_width)
    frames = frames.astype(dtype=fmt)
    frames = frames.tobytes()

    with wave.open(filepath, 'wb') as wf:
        wf.setnchannels(n_channels)
        wf.setsampwidth(sample_width)
        wf.setframerate(sample_rate)
        wf.writeframes(frames)

def cal_spl(data, seg=1024):
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
        prms = np.sqrt(np.mean(curFrame * curFrame))
        pref = 20e-6  # 空气中 20微pa，水中 1微pa
        volume[i] = 20 * np.log10(prms / pref)
    return volume

def cal_zcr(data, seg=1024):
    '''
    https://zh.wikipedia.org/zh-mo/%E8%BF%87%E9%9B%B6%E7%8E%87
    '''
    data_len = len(data)
    frame_num = int(math.ceil(data_len / seg))
    zcr = np.zeros((frame_num, 1))
    for i in range(frame_num):
        curFrame = data[i*seg: i*seg+seg]
        zcr[i] = sum( curFrame[0:-1] * curFrame[1:] < 0 ) / len(curFrame)
    return zcr



def test_spl_zcr():
    sr, sw, data = read_data()
    spl = cal_spl(data)
    zcr = cal_zcr(data)

    time = np.arange(0, len(data)) * (1.0 / sr)

    ax = plt.subplot(411)
    plt.title("amplitude")
    plt.plot(time, data)

    time_spl = np.arange(0, len(spl)) * ( len(data) / sr / len(spl))
    plt.subplot(412, sharex=ax)
    plt.title("spl")
    plt.plot(time_spl, spl)

    time_zcr = np.arange(0, len(zcr)) * ( len(data) / sr / len(zcr))
    plt.subplot(413, sharex=ax)
    plt.title("zcr")
    plt.plot(time_zcr, zcr)
    
    plt.subplot(414)
    plt.title("specgram")
    plt.specgram(data, Fs=sr, scale_by_freq=True)
    plt.ylabel("frquency")
    plt.xlabel("time_s)")

    plt.tight_layout()
    plt.show()

def frequence_analysis():
    sr, sw, data = read_data()
    time = np.arange(0, len(data)) * (1.0 / sr)

    fft_y = np.fft.fft(data)
    fft_freq = np.linspace(0, 1, len(data)) * sr
    

    b, a = scipy.signal.butter(4, [82/sr*2, 1000/sr*2], 'bandpass')
    fix_x = scipy.signal.filtfilt(b, a, data)
    
    fix_fft_y = np.fft.fft(fix_x)

    mel = librosa.feature.mfcc(data, sr=sr, n_mfcc=1,)
    print(mel.shape)
    print(mel[:10])

    ax = plt.subplot(221)
    plt.title("amplitude")
    plt.plot(time, data)

    plt.subplot(222)
    plt.plot(fft_freq, fft_y)

    plt.subplot(223)
    plt.plot(time, fix_x)

    plt.subplot(224)
    plt.plot(fft_freq, fix_fft_y)
    plt.show()
    np.hamming
    save_data(fix_x, "/home/fulincao/WXWork/output.wav", sw, sr)


if __name__ == "__main__":
    # test_spl_zcr()
    frequence_analysis()

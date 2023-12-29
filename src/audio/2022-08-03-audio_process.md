---
title: 音频处理
date: 2022-08-03
categories: [音频处理]
tags: [fft, 音频处理]     # TAG names should always be lowercase
---

### 音频处理
这里对音频进行简单的振幅，声压以及频谱分析  
**振幅**:是指振动的物理量可能达到的最大值，通常以A表示。它是表示振动的范围和强度的物理量。  
**声压级**: 声学中，声音的强度定义为声压。计算分贝值时采用20微帕斯卡为参考值[5]。这一参考值是人类对声音能够感知的阈值下限。声压是场量，因此使用声压计算分贝时使用下述版本的公式:

$$
\begin{aligned}
L_p = 20\log_{10}(\dfrac{p_{rms}}{p_{ref}}) dBspl
\end{aligned}
$$

全尺寸的计算方式
$$
\begin{aligned}
L = 20\log_{10}(p_{rms}) dBfs \approx dBspl - 94
\end{aligned}
$$

其中的$p_{ref}$是标准参考声压值20微帕。
在空气中1帕斯卡等于94分贝声压级。在其他介质，如水下，1微帕斯卡更为普遍。这些标准被ANSIS1.1-1994.所收录.  

**频率**

- 模拟频率$f$：单位时间内信号变化的次数，即信号的周期，单位为赫兹`Hz`。
- 模拟角频率$\Omega$：单位时间内信号变化的角度，即信号的角频率，单位为弧度$\Omega=2\pi * f$。
- 采样频率$f_s$：单位时间内采集的样本数，即信号的采样率，单位为赫兹`Hz`,根据奈奎斯特采样定理，采样频率必须高于信号的最高频率二倍才不会发生信号混叠，因此$f_s$能采样到的最高频率为$f_s / 2$。

- 数字频率$\omega$: 数字频率是对模拟频率的采样，数字频率代表的是两个采样点之间的间隔$\omega = 2\pi * f / f_s = \Omega / f_s$


**频谱**: 是指一个时域的信号在频域下的表示方式，可以针对信号进行傅里叶变换而得，所得的结果会是分别以幅度及相位为纵轴，频率为横轴的两张图，不过有时也会省略相位的信息，只有不同频率下对应幅度的资料。有时也以“幅度频谱”表示幅度随频率变化的情形，“相位频谱”表示相位随频率变化的情形。
一个声音信号（左图）及其对应的频谱.简单来说，频谱可以表示一个信号是由哪些频率的弦波所组成，也可以看出各频率弦波的大小及相位等信息。

详情见下方的代码
```python
    def normalize(self, data):
        '''归一化'''
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

    def cal_spectrum(self, data, seg=1024):
        """
        https://zh.m.wikipedia.org/wiki/%E9%A2%91%E8%B0%B1
        计算频谱，傅立叶变换之后计算强度
        """
        # print("dft begin ")
        # fft_y = self.fft(data)
        # fft_y = self.dft(data)
        fft_y = scipy.fft.fft(data)
        fft_y = self.cal_spl(fft_y, seg=seg)
        return fft_y

def test_wave():
    sys.argv.append("0.wav")
    mw = MyWave(sys.argv[1])
    print(mw)

    # 读取数据
    wav_data = mw.readframes()
    amplitude = mw.normalize(wav_data)

    nchannels, sampwidth, framerate, nframes = mw.getparams()
    time1 = np.arange(0, len(amplitude)) * (1.0 / framerate)

    seg = int(20e-3 * framerate)  # 20ms 50ms 100ms 常用计算区间
    volume = mw.cal_spl(amplitude, seg=seg)
    time2 = np.arange(0, len(volume)) * seg * 1.0 / framerate

    test_sz = 1024
    fft_y = mw.cal_spectrum(amplitude[:test_sz], seg)
    fft_x = np.arange(0, len(fft_y)) * seg * framerate / len(amplitude)

    plt.subplot(311)
    plt.plot(time1, amplitude)
    plt.subplot(312)
    plt.plot(time2, volume, label="db")
    plt.legend()

    plt.subplot(313)
    plt.plot(fft_x, fft_y)
    plt.show()

## 完整代码见test_wav.py
```

***生成固定大小频率的音频***

```python
def generate_fixed_decibel_signal(db, f=1000, fs=48000, duration=10):
    """
    生成固定分贝信号 
        dbfs + 20*log10(V_ref) = dbspl ref always equal 20e-6 in air
        so dbfs ~= dbspl - 93
    """
    t = np.arange(1, duration * fs + 1, 1).flatten()
    # t = np.ones(duration * fs) * -1000
    amp = math.pow(10, db / 20)  # db -> amplify
    incr = 2 * math.pi * f / fs # declare the increment between two neighbor sample
    signal = amp * np.sin(t * incr) # generate the signal
    # print(signal[:10])
    signal = np.clip(signal, -1, 1) # clip the signal to [-1, 1]
    # print(signal[:10])
    return signal
```
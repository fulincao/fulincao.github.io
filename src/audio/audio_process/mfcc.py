"""
    mfcc implement
    https://en.wikipedia.org/wiki/Mel-frequency_cepstrum
    http://www.practicalcryptography.com/miscellaneous/machine-learning/guide-mel-frequency-cepstral-coefficients-mfccs/
    https://www.cnblogs.com/BaroC/p/4283380.html
"""

import numpy as np
import math
import scipy.fftpack
import scipy


def pre_add_weight(signal, alpha=0.97):
    """
    预加重
    https://zhuanlan.zhihu.com/p/34798429
    """
    signal = np.insert(signal, 0, 0)
    return signal[1:] - alpha * signal[:-1]


def hamming(N):
    """
    生成汉明窗(hamming windows)
    https://zh.wikipedia.org/wiki/%E7%AA%97%E5%87%BD%E6%95%B0#Hamming%E7%AA%97
    http://t.zoukankan.com/dylancao-p-14212421.html
    """
    return 0.54 - 0.46 * np.cos(np.arange(0, N) * np.pi * 2 / (N - 1))


def mel2hz(mel):
    """
    梅尔频率(mel frequency)转普通频率
    https://www.mathworks.com/help/audio/ref/mel2hz.html
    """
    return 700 * (10**(mel / 2595.0) - 1)


def hz2mel(hz):
    """
    频率转梅尔频率
    """
    return 2595.0 * np.log10(1 + hz / 700)


def split_frame(signal, win_len, win_step, win_func=hamming):
    """
    分帧加窗
    """
    hamming_win = win_func(win_len)
    sig_len = len(signal)
    pad_len = 0
    num_frames = 0
    if sig_len < win_len:
        pad_len = win_len - sig_len
        num_frames = 1
    else:
        pad_n = math.ceil((sig_len - win_len) / win_step)
        pad_len = pad_n * win_step + win_len - sig_len
        num_frames = pad_n + 1
    pad_signal = np.zeros(sig_len + pad_len)
    pad_signal[:sig_len] = signal
   
    frames = []
    for i in range(num_frames):
        r = pad_signal[i*win_step:i*win_step+win_len] * hamming_win     
        frames.append(pad_signal[i*win_step:i*win_step+win_len] * hamming_win)
    return np.array(frames)


def power_spectral(frames, n_fft=512):
    """
    计算功率图谱
    """
    n_fft = n_fft or frames.shape[1]
    pow_spec = np.fft.rfft(frames, n_fft)  # n_fft / 2 + １
    pow_spec = np.abs(pow_spec)
    return 1.0 / n_fft * np.square(pow_spec)


def get_mel_filter_banks(low_freq, high_freq, sample_rate, n_filt=26, n_fft=512):
    low_mel = hz2mel(low_freq)
    high_mel = hz2mel(high_freq)
    mel_points = np.linspace(low_mel, high_mel, n_filt + 2)
    # f(i) = floor((nfft+1)*h(i)/samplerate)
    bin = np.floor((n_fft+1)*mel2hz(mel_points) / sample_rate)
    fbank = np.zeros([n_filt, n_fft//2+1])
    for j in range(0, n_filt):
        for i in range(int(bin[j]), int(bin[j+1])):
            fbank[j, i] = (i - bin[j]) / (bin[j+1]-bin[j])
        for i in range(int(bin[j+1]), int(bin[j+2])):
            fbank[j, i] = (bin[j+2]-i) / (bin[j+2]-bin[j+1])
    return fbank


def matlab_dct2(x):
    """
    https://ww2.mathworks.cn/help/signal/ref/dct.html#responsive_offcanvas
    https://zh.wikipedia.org/wiki/%E7%A6%BB%E6%95%A3%E4%BD%99%E5%BC%A6%E5%8F%98%E6%8D%A2
    系数为 sqrt(2/n) 同时增加克罗内克函数(Kronecker) delta(k,0)
    see also scipy.fftpack.dct(x, type=2, norm='ortho')
    """
    N = len(x)
    y = np.zeros(N)
    for k in range(N):
        t0 = x * np.cos(np.pi * (np.arange(N) + 1/2) * k / N)
        # for Kronecker
        kronecker_delta = k == 0
        t0 = t0 * 1 / np.sqrt(kronecker_delta + 1)
        y[k] = np.sqrt(2 / N) * np.sum(t0)
    return y


def lifter(cepstra, L=22):
    nframes, ncoeff = np.shape(cepstra)
    n = np.arange(ncoeff)
    lift = 1 + (L/2.) * np.sin(np.pi*n/L)
    return lift*cepstra


def mfcc(signal, sample_rate, win_durtation=0.025, win_step=0.010, num_cep=13, n_filter=26, win_func=hamming, pre_weight=0.97):
    
    win_len = round(sample_rate * win_durtation)
    win_step = round(sample_rate * win_step)

    signal = pre_add_weight(signal, pre_weight)
    frames = split_frame(signal, win_len, win_step, win_func=hamming)
    
    n_fft = win_len
    pow_spec = power_spectral(frames, n_fft=win_len)
    energy = np.sum(pow_spec, 1)
    energy = np.where(energy == 0,np.finfo(float).eps,energy) # if energy is zero, we get problems with log

    mel_filter_banks = get_mel_filter_banks(0, sample_rate // 2, sample_rate, n_filt=n_filter, n_fft=n_fft)
    
    feat = np.dot(pow_spec, mel_filter_banks.T)
    feat = np.where(feat == 0,np.finfo(float).eps,feat) # if feat is zero, we get problems with log
    feat = np.log(feat)

    feat = scipy.fftpack.dct(feat, type=2, norm='ortho')
    feat = feat[:, :num_cep]
    feat = lifter(feat)
    feat[:,0] = np.log(energy) # replace first cepstral coefficient with log of frame energy

    return feat

if __name__ == "__main__":
    
    import python_speech_features
    import python_speech_features.sigproc
    import scipy.signal
    import matplotlib.pyplot as plt

    samplerate = 1000
    sig = np.linspace(0, 1, samplerate)
    sig = np.sin(sig)

    frames = split_frame(sig, 20, 10)
    
    feat1 = python_speech_features.mfcc(sig,samplerate=samplerate, nfft=25, appendEnergy=False, winfunc=hamming, nfilt=26)
    feat2 = mfcc(sig, samplerate, n_filter=26)


    # feat1= python_speech_features.get_filterbanks(nfilt=26, nfft=250, samplerate=samplerate, highfreq=samplerate//2)
    # feat2 = get_mel_filter_banks(0, samplerate//2, samplerate, n_filt=26, n_fft=250)

    # feat1,_ = python_speech_features.fbank(sig,samplerate=samplerate, nfft=25, winfunc=hamming, nfilt=2)
    # feat2 = mfcc(sig, samplerate, n_filter=2)

    # feat1 = python_speech_features.sigproc.powspec(frames, 20)
    # feat2 = power_spectral(frames, n_fft=20)

    # feat1 = python_speech_features.sigproc.framesig(sig, 5, 3)
    # feat2 = split_frame(sig, 5, 3, win_func=lambda x: np.ones((x,)))

    # print(feat1[:5])
    # print(feat2[:5])
    print(np.allclose(feat1, feat2))

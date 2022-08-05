from cProfile import label
import numpy as np
import math
import matplotlib.pyplot as plt

def running_mean(x, windowSize):
    # from http://stackoverflow.com/questions/13728392/moving-average-or-running-mean
    cumsum = np.cumsum(np.insert(x, 0, np.zeros(windowSize))) 
    return (cumsum[windowSize:] - cumsum[:-windowSize]) / windowSize

def low_pass_filter_by_moving_average(x, cutoff, fs=48000):
    
    # get window size
    # from http://dsp.stackexchange.com/questions/9966/what-is-the-cut-off-frequency-of-a-moving-average-filter
    freqRatio = (cutoff/fs)
    N = int(math.sqrt(0.196202 + freqRatio**2)/freqRatio)
    filtered = running_mean(x, N)
    # filtered = np.hstack([filtered, np.zeros(N-1)])
    return filtered

def low_pass_filter_by_butter(x, butter_n, cutoff, fs=48000):
    fft_x = np.fft.rfft(x)
    freqs = np.linspace(0, fs//2, len(fft_x))

    xb = []
    for i in range(len(fft_x)):
        g = 1 / math.sqrt(1 + (freqs[i] / cutoff)**(2*butter_n) ) * fft_x[i]
        xb.append(g)

    filter_x = np.fft.irfft(xb, n=len(x))
    return filter_x

def run():
    framerate = 1000
    cutoff = 50
    base_x = np.linspace(0, 1, 1000)
    x = np.sin(2*math.pi * 100*base_x) + np.cos(2*math.pi *10*base_x)
    x_10 = np.cos(2*math.pi *10*base_x)

    hz = np.linspace(0, framerate, len(x))
    time = np.arange(0, len(x)) * (1 / framerate)

    origin_fft_x = np.fft.fft(x)
    
    filter_x = low_pass_filter_by_butter(x, 4, cutoff, fs=1000)
    fft_x = np.fft.fft(filter_x)

    plt.subplot(311)
    plt.plot(time, x)
    plt.title("origin_signal")

    plt.subplot(312)
    plt.plot(time, x_10, label="origin_filted")
    plt.plot(time, filter_x, label="butter_filted")
    
    plt.title("filted_signal")
    plt.legend()

    plt.subplot(313)
    plt.plot(hz, origin_fft_x, label="origin")
    plt.plot(hz, fft_x, label="butter_filted")
    plt.legend()
    plt.title("fft")

    plt.show()



    filter_x = low_pass_filter_by_moving_average(x, cutoff, fs=1000)
    fft_x = np.fft.fft(filter_x)

    plt.subplot(311)
    plt.plot(time, x)
    plt.title("origin_signal")

    plt.subplot(312)
    plt.plot(time, x_10, label="origin_filted")
    plt.plot(time, filter_x, label="moving_filted")
    
    plt.title("filted_signal")
    plt.legend()

    plt.subplot(313)
    plt.plot(hz, origin_fft_x, label="origin")
    plt.plot(hz, fft_x, label="moving_filted")
    plt.legend()
    plt.title("fft")

    plt.show()









if __name__ == "__main__":
    run()

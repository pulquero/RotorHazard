import numpy as np
from scipy.fft import rfft, rfftfreq
import matplotlib.pyplot as plt
import sys
import csv
import rh.util.persistent_homology as ph
from scipy.ndimage import median_filter


fs = 1000  # sample freq
median_window_size = 5  # should be odd
ph_history_size = 12


rssi_list = []
with open(sys.argv[1]) as f:
    reader = csv.reader(f)
    for r in reader:
        rssi_list.append(float(r[0]))

ts = np.arange(len(rssi_list))/fs
rssis = np.array(rssi_list)

rssis = median_filter(rssis, median_window_size, origin=(median_window_size-1)//2)


def plot_signal(rssis):
    F = rfft(rssis, norm='forward')
    freqs = rfftfreq(len(rssis), 1/fs)

    fig, axs = plt.subplots(1, 2, figsize=(12,6))
    fig.canvas.manager.set_window_title('Signal')
    axs[0].set_title('Signal')
    axs[0].set_ylabel('RSSI')
    axs[0].set_xlabel('Time / s')
    axs[0].plot(ts, rssis)
    axs[1].set_title('Spectrum')
    axs[1].set_xlabel('Frequency / Hz')
    axs[1].plot(freqs, np.abs(F))
    fig.tight_layout()
    plt.show(block=False)


plot_signal(rssis)


def plot_ph(title, ccs):
    fig, axs = plt.subplots(1, 3, figsize=(12,4))
    fig.canvas.manager.set_window_title(title)
    axs[0].set_title('Sample lifetimes')
    ph.plotPeakLifetimes(axs[0], ts, ccs)
    axs[1].set_title('Persistence diagram')
    ph.plotPersistenceDiagram(axs[1], ccs)
    axs[2].set_title('Persistence lifetimes')
    ph.plotLifetimes(axs[2], ccs)
    min_bound, max_bound = ph.findBreak(ccs)
    threshold = (min_bound + max_bound)/2
    xlim = axs[1].get_xlim()
    axs[2].plot(xlim, [threshold,threshold], '--', c='tomato')
    axs[2].annotate('threshold', (xlim[0], threshold), xytext=(3,3), textcoords='offset points')
    fig.tight_layout()
    plt.show(block=False)


ccs = ph.calculatePeakPersistentHomology(rssis)
ccs = ph.sortByLifetime(ccs)
plot_ph('Persistent Homology', ccs)

rt_ccs = [ph.calculateRealtimePeakPersistentHomology(rssis[:i+1], ph_history_size) for i in range(len(rssis))]
rt_ccs = ph.sortByLifetime([cc for cc in rt_ccs if cc is not None])
plot_ph('Realtime Persistent Homology', rt_ccs)

plt.show()

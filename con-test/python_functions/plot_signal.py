import sys
import os
import matplotlib.pyplot as plt
from matplotlib.ticker import (MultipleLocator, FormatStrFormatter,
                               AutoMinorLocator)
import numpy as np

def gen_weight_vector(N):
    '''
    generate the weight vector for weighted moving avarage
    the vector will neither amplify nor dampen

    Arguments:
        @param N: Size of vector

    Return:
        w = vector with weights with center weight of 0.5
    '''
    factor = (N / 2.0) - 1.0
    h = list()
    first = np.linspace(0, 0.25, factor) / (factor / 2.0)
    h = np.ndarray.tolist(first)
    h.append(0.5)
    second = np.ndarray.tolist(np.flip(np.linspace(0, 0.25, (N / 2))) / ((N / 2) / 2))
    return h + second


def running_mean(x, N):
    return np.convolve(x, gen_weight_vector(N), 'same')


def tcpdump_data(filename, path_name):
    '''
    read data from tcpdump and create plots from it

    Arguments:
        @param filename: tcpdump file to work on
        @param path_name: path to python plot directory

    Return:
        0 on success, 1 else
    '''
    try:
        t, seq, rate, mcs, noise, snr0, snr1 = np.loadtxt(filename,
                                                          delimiter=' ',
                                                          unpack=True,
                                                          skiprows=1)
    except ValueError:
        print("ValueError encountered reading {}\n".format(os.path.basename(filename)))
        return 1

    mean_signal = running_mean(noise, 100)

    fig1 = plt.figure(1)
    plt.plot(t, mcs, 'C3*', linewidth=0.1, markersize=2, alpha=0.1)
    plt.xlabel('Time [s]')
    plt.ylabel('Selected MCS rate')
    plot_file = path_name + \
                '/mcs_' + \
                os.path.splitext(os.path.basename(filename))[0] + \
                '.png'
    plt.savefig(plot_file)



    fig2 = plt.figure(2)
    plt.plot(t, rate, 'C3*', linewidth=0.1, markersize=3, alpha=0.1)
    plt.xlabel('Time [s]')
    plt.ylabel('Selected throughput rate')
    plt.grid(color='k', which='both', linestyle='-', linewidth=0.2)
    plot_file = path_name + \
                '/throughputrate_' + \
                os.path.splitext(os.path.basename(filename))[0] + \
                '.png'
    plt.savefig(plot_file)

    fig3 = plt.figure(3)
    l1 = plt.plot(t, snr0, 'C3*', linewidth=0.1, markersize=2, alpha=0.1)
    l2 = plt.plot(t, snr1, 'C0*', linewidth=0.1, markersize=2, alpha=0.1)
    l3 = plt.plot(t, mean_signal, 'k-', linewidth=0.1)

    line_labels = ['Signal strength antenna 0', 'Signal strength antenna 1', 'Signal strength']
    plot_file = path_name + \
                '/rssi_' + \
                os.path.splitext(os.path.basename(filename))[0] + \
                '.png'

    plt.xlabel('Time [s]')
    plt.ylabel('Signal strength [dBm]')
    plt.legend([l1, l2, l3], labels=line_labels)
    plt.savefig(plot_file)
    plt.show()

    return 0

# get filename and plot dir from commandline
F_NAME = sys.argv[1]
P_NAME = sys.argv[2]
tcpdump_data(F_NAME, P_NAME)

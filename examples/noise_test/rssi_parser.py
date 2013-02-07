#! /usr/bin/python

import csv as csv
import matplotlib.pyplot as pyplot
import numpy as numpy
import os as os
import math as math
import scipy as scipy

def compute_rssi(results):
    for key in results.keys():
        result = results[key]
        time, rssi = zip(*result)
        avg_rssi = numpy.average(rssi)
        std_rssi = numpy.std(rssi)
        print "Channel " + str(key) + ": " + str(avg_rssi) + " dBm +/- " + str(std_rssi) + " dB"

def parse(data):
    header = data.next()
    results = {}
    for d in data:
        time = d[0]
        channel = int(d[1])
        rssi = int(d[2])
        if results.has_key(channel):
           results[channel].append((time,rssi))     
        else:
            results[channel] = []
            results[channel].append((time, rssi))
    return  header, results

def load(name):
    data_file = open(name, 'rt')
    data = csv.reader(data_file, delimiter=',')
    return data

def files(path):
    csv = []
    extension = "csv"
    files = os.listdir(path)
    for f in files:
        if f.endswith(extension):
            csv.append(f)
    return csv

def plot_rssi(channel, data):
    time, rssi = zip(*data)
    fig = pyplot.figure()
    ax1 = fig.add_subplot(111)
    ax1.plot(rssi, marker='None', color='r', ls='-')
    ax1.set_title("Channel " + channel)
    ax1.set_xlabel("Time (s)")
    ax1.set_ylabel("RSSI (dBm)")
    ax1.grid()
    ax1.axis('tight')
    ax1.tick_params(axis='x', labelsize=8)
    ax1.tick_params(axis='y', labelsize=8)
    
    #plot.set_xticks()
    #plot.set_xticklabels()
    pyplot.savefig("RSSI Channel " + channel + ".png", format='png', dpi=150, bbox_inches='tight')

# Main function
def main():
    path = "./"
    data_files = files(path)
    for data_file in data_files:
        data = load(data_file)
        header, results = parse(data)
        compute_rssi(results)
        for key in results.keys():
            plot_rssi(str(key), results[key])

# Main function
if __name__ == "__main__":
    main()
    

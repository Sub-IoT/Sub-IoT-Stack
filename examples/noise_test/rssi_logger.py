#! /usr/local/bin/python

import sys as system
import os as os
import serial as serial
import shutil as shutil
import time as time

import ctypes

FILE_NAME = "noise_test"
FILE_EXTENSION = ".csv"

SERIAL_PORT = 'COM7'
#SERIAL_PORT = '/dev/ttyUSB0'
SERIAL_RATE_9600 = 9600
SERIAL_RATE_115200 = 115200

def parse_rssi(rssi):
        if (rssi >= 128):
                rssi = (rssi - 256)/2 - 74
        else:
                rssi = rssi/2 - 74
        return rssi

def main():
        serial_port = serial.Serial(SERIAL_PORT, SERIAL_RATE_9600)
        f = open(FILE_NAME + FILE_EXTENSION, 'w')
        f.write("Timestamp, Channel number (0-14), RSSI (dBm)" + "\n")
        while(True):
                data = serial_port.read(size=2)
                timestamp = int(time.time())
                channel = int(data[0].encode("hex"), 16)
                rssi = int(data[1].encode("hex"), 16)
                rssi = parse_rssi(rssi)
                f.write(str(timestamp) + ", ")
                f.write(str(channel) + ", ")
                f.write(str(rssi) + "\n")
                f.flush()
                print "RSSI (dBm) = " + str(rssi)
                
if __name__ == "__main__":
	main()

#! /usr/bin/python

import sys as system
import os as os
import serial as serial
import shutil as shutil
import time as time
import struct

import ctypes

FILE_NAME = "noise_test"
FILE_EXTENSION = ".csv"


def main():
	if (len(system.argv) != 3):
		print "Usage: <serialport (eg COM7)> <baudrate (eg 9600)>"
		system.exit(2)

	serial_port = serial.Serial(system.argv[1], system.argv[2])
	f = open(FILE_NAME + FILE_EXTENSION, 'w')
	f.write("Timestamp, Channel number (0-14), RSSI (dBm)" + "\n")
	while(True):
		data = serial_port.read(size=2)
		timestamp = int(time.time())
		channel = int(data[0].encode("hex"), 16)
		rssi = struct.unpack("b", data[1])[0]
		
		f.write(str(timestamp) + ", ")
		f.write(str(channel) + ", ")
		f.write(str(rssi) + "\n")
		f.flush()
		print "RSSI (dBm) = " + str(rssi)
            
if __name__ == "__main__":
	main()

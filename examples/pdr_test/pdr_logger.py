#! /usr/bin/python

import sys as system
import os as os
import serial as serial
import shutil as shutil
import time as time
import struct as struct

import ctypes

def main():
	if (len(system.argv) != 3):
		print "Usage: <serialport (eg COM7)> <baudrate (eg 9600)>"
		system.exit(2)

	serial_port = serial.Serial(system.argv[1], system.argv[2])
	counter = 0
	while(True):
		data = serial_port.read(size=2)
		new_counter = struct.unpack("H", data)[0]
		if(new_counter > counter + 1):
			print "!!! missing packet"

		counter = new_counter
		print counter

if __name__ == "__main__":
	main()
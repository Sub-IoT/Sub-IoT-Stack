#! /usr/bin/python
from __future__ import division, absolute_import, print_function, unicode_literals


import sys as system
import os as os
import serial as serial
import shutil as shutil
import time as time
import struct as struct
import ctypes
import datetime
import Queue
#import wx
from collections import namedtuple
import threading

import logging
import argparse

# The recommended way to use wx with mpl is with the WXAgg
# backend. 
#


datestr = None

serial_port = None


def read_value_from_serial():
	data = serial_port.read()	

	length = struct.unpack("b", data)[0]
	print("length: %d" % length);
		
	data = serial_port.read(size=length)
	print("data: "  + data)
	
	
	#print(">D7: " + data.encode("hex").upper() + " " + data + "\n")
	#print(">D7: " + self.message + "\n")
	#print(">D7: " + data.encode("hex").upper() + "\n")
	
	
class parse_d7(threading.Thread):
	def __init__ (self):
		self.keep_running = True
		threading.Thread.__init__ (self)
		
	def stop(self):
		self.keep_running = False
	
	def run(self):	
		while self.keep_running:
			try:
				read_value_from_serial()				
			except Exception as inst:
				print (inst) 
	
def empty_serial_buffer():
	while serial_port.inWaiting() > 0:
		serial_port.read(1)

def main():

	global serial_port, settings
	parser = argparse.ArgumentParser(description = "DASH7 to UART test. You can exit the logger using Ctrl-c, it takes some time.")
	parser.add_argument('serial', default="COM6", metavar="serial port", help="serial port (eg COM7 or /dev/ttyUSB0)", nargs='?')
	parser.add_argument('-b', '--baud' , default=115200, metavar="baudrate", type=int, help="set the baud rate (default: 9600)")
	settings = vars(parser.parse_args())

	serial_port = serial.Serial(settings['serial'], settings['baud'])
	
	empty_serial_buffer()
	keep_running = True
	
	try:
		parseThread = parse_d7()
		parseThread.start()
	except Exception as inst:
		printError("Error unable to start thread")
		printError(inst)	
	
	while keep_running:
		try:						
			input = raw_input("Prompt> ")
			serial_port.write(chr(len(input)))
			serial_port.write(input)
			print("<SER: " + input.encode("hex").upper() + " length: %i" % len(input)) 

		
		except (KeyboardInterrupt, SystemExit):	
			keep_running = False
			parseThread.stop()	
		except Exception as inst:
			print (inst)
			
	print("This program is stopping")
	system.exit(0)
		
			
if __name__ == "__main__":
	main()

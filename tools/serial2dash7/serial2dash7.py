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
import wx
from collections import namedtuple
import threading

import logging

# The recommended way to use wx with mpl is with the WXAgg
# backend. 
#


datestr = None

serial_port = None


def read_value_from_serial():
	data = serial_port.read()
	#read_length()
	#message = serial_port.read(self.length)
	print(data.encode("hex").upper())
	#print(data.encode("hex").upper())
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
	if (len(system.argv) != 2):
		print("Usage: <serialport (eg COM7)>")
		system.exit(2)

	global serial_port 
	serial_port = serial.Serial(system.argv[1], 9600)
	
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
			serial_port.write(input)
			print("<SER: " + input.encode("hex").upper()) 

		
		except (KeyboardInterrupt, SystemExit):	
			keep_running = False
			parseThread.stop()	
		except Exception as inst:
			print (inst)
			
	print("This program is stopping")
	system.exit(0)
		
			
if __name__ == "__main__":
	main()

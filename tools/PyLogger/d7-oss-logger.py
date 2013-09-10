#! /usr/bin/python

# Created on: Sep 09, 2013
# Authors:
# 	jan.stevens@ieee.org
# 	
# Discription:
# This is a python logger for D7 when logging is active.
# It's an alternative for the Qt logger currently used, which is not that great for installing and quick debugging
# Before using the logger make sure you have installed python 2.7 or higher and the serial library
# 
# Install:
# Before usage, install following python packages using PyPI (pip python)
# pip install pyserial
# pip install colorama
# 
# TODO's
# - General interface for most of the options.
# - better error controlling
# - Fix some ugly code (TODO's)

from __future__ import division, absolute_import, print_function, unicode_literals
from colorama import init, Fore, Back, Style
from collections import defaultdict
import sys as system
import os as os
import serial as serial
import struct as struct
import datetime
import threading
import Queue
import time
import logging
import argparse

DEBUG = 0

SYNC_WORD = "DD"

### Global variables we need, do not change! ###
serial_port = None
dataQueue = Queue.Queue()
displayQueue = Queue.Queue()

#TODO fix this ugly code... but it works
data = [("PHY", "GREEN"), ("DLL", "RED"), ("MAC", "YELLOW"), ("NWL", "BLUE"), ("TRANS", "MAGENTA"), ("FWK", "CYAN")]
stackColors = defaultdict(list)
for layer, color in data:
	stackColors[layer].append(color)

stackLayers = {'01' : "PHY", '02': "DLL", '03': "MAC", '04': "NWL", '05': "TRANS", '10': "FWK"}


get_input = input if system.version_info[0] >= 3 else raw_input
logging.Formatter(fmt='%(asctime)s.%(msecs)d', datefmt='%Y-%m-%d,%H:%M:%S')

# Small helper function that will format our colors
def formatHeader(header, color):
	bgColor = getattr(Back, color)
	fgColor = getattr(Fore, color)
	return bgColor + Style.BRIGHT + Fore.WHITE + header + fgColor + Back.RESET + Style.NORMAL + "  "

###
# The different logs classes, every class has its own read, write and print function for costumization
###
class Logs(object):
	def __init__(self, logType):
		self.logType = logType
		self.length = 0

	def read(self):
		pass

	def write(self):
		pass

	def __str__(self):
		pass

	def read_length(self):
		length = serial_port.read(size = 1)
		self.length = int(struct.unpack("b", length)[0])


class LogString(Logs):
	def __init__(self):
		Logs.__init__(self, "string")

	def read(self):
		# Read the length of the message
		self.read_length()
		self.message = serial_port.read(size=self.length)
		return self

	def write(self):
		if hasattr(self, 'message'):
			return "STR: " + self.message + "\n"
		return ""

	def __str__(self):
		if hasattr(self, 'message'):
			string = formatHeader("STRING", "GREEN") + self.message + Style.RESET_ALL
			return string
		return ""


class LogData(Logs):
	def __init__(self):
		Logs.__init__(self, "data")

	def read(self):
		self.read_length()
		data = serial_port.read(size=self.length)
		self.data = struct.unpack('b', data)[0]
		return self
	
	def write(self):
		if hasattr(self, 'data'):
			return "DAT: " + str(self.data) + "\n"
		return ""

	def __str__(self):
		if hasattr(self, 'data'):
			string = formatHeader("DATA", "BLUE") + str(self.data) + Style.RESET_ALL
			return string
		return ""

class LogStack(Logs):
	def __init__(self):
		Logs.__init__(self, "stack")

	def read(self):
		layer = serial_port.read(size=1).encode('hex').upper()
		self.layer = stackLayers.get(layer, "STACK")
		#print("Got layer: %s with stack: %s" % (layer, self.layer))
		self.color = stackColors[self.layer][0]
		self.read_length()
		self.message = serial_port.read(size=self.length)
		return self

	def write(self):
		if hasattr(self, 'message'):
			return self.layer + ": " + self.message + "\n"
		return ""

	def __str__(self):
		if hasattr(self, 'message'):
			string = formatHeader("STK: " + self.layer, self.color) + self.message + Style.RESET_ALL
			return string
		return ""

class LogTrace(Logs):
	def __init__(self):
		Logs.__init__(self, "trace")

	def read(self):
		self.read_length()
		self.message = serial_port.read(size=self.length)
		return self

	def write(self):
		if hasattr(self, 'message'):
			return "TRACE: " + self.message + "\n"
		return ""

	def __str__(self):
		if hasattr(self, 'message'):
			string = formatHeader("TRACE", "CYAN") + self.message + Style.RESET_ALL
			return string
		return ""



class LogDllRes(Logs):
	def __init__(self):
		Logs.__init__(self, "dllrx")

	def read(self):
		self.read_length()
		self.frame_type = str(struct.unpack('b', serial_port.read(size=1))[0])
		self.spectrum_id = "0x" + str(serial_port.read(size=1).encode("hex").upper())
		return self

	def write(self):
		if hasattr(self, 'frame_type'):
			return "DLL RES: frame_type " + self.frame_type + " spectrum_id " + self.spectrum_id
		return ""

	def __str__(self):
		if hasattr(self, 'frame_type'):
			string = formatHeader("DLL RES", "CYAN") + "frame_type " + self.frame_type + " spectrum_id " + self.spectrum_id
			return string
		return ""


##
# Different threads we use
##
class parse_d7(threading.Thread):
	def __init__(self, datQueue, disQueue):
		self.keep_running = True
		self.datQueue = datQueue
		self.disQueue = disQueue
		threading.Thread.__init__(self)

	def stop(self):
		self.keep_running = False

	def run(self):
		while self.keep_running:
			try:
				serialData = read_value_from_serial()
				if serialData is not None:
					self.datQueue.put(serialData)
					self.disQueue.put(serialData)
			except Exception as inst:
				print(inst)

class display_d7(threading.Thread):
	def __init__(self, disQueue):
		self.keep_running = True
		self.queue = disQueue
		threading.Thread.__init__(self)

	def stop(self):
		self.keep_running = False

	def run(self):
		while self.keep_running:
			try:
				while not self.queue.empty():
					data = self.queue.get()
					print(data)
			except Exception as inst:
				print (inst)

class write_d7(threading.Thread):
	def __init__(self, fileStream, queue):
		self.keep_running = True
		self.file = fileStream
		self.queue = queue
		threading.Thread.__init__(self)

	def stop(self):
		self.keep_running = False

	def run(self):
		while self.keep_running:
			try:
				while not self.queue.empty():
					data = self.queue.get()
					encoded = (data.write()).encode('utf-8')
					self.file.write(encoded)
				time.sleep(10)
			except Exception as inst:
				print(inst)

def dummy():
	pass

def read_value_from_serial():
	result = {}

	data = serial_port.read(size=1)
	while not data.encode("hex").upper() == SYNC_WORD:
		if DEBUG:
			print("received unexpected data (%s), waiting for sync word " % data.encode("hex").upper())
		data = serial_port.read(size=1)

	# Now we can read the type of the string
	logtype = serial_port.read(size=1).encode("hex").upper()

	log_string = LogString()
	log_data = LogData()
	log_stack = LogStack()
	log_trace = LogTrace()
	log_dll_res = LogDllRes()

	processedread = {"01" : log_string.read, 
			 "02" : log_data.read,
			 "03" : log_stack.read,
			 "FD" : log_dll_res.read,
			 "FF" : log_trace.read, }
		 
	# See if we have found our type in the LOG_TYPES
	#print("We got logtype: %s" % logtype)
	result = processedread[logtype]()

	return result

def empty_serial_buffer():
	while serial_port.inWaiting() > 0:
		serial_port.read(1)

def main():
	global serial_port

	# Setup the console parser
	parser = argparse.ArgumentParser(description = "DASH7 logger.")
	parser.add_argument('serial', metavar="serial port", help="serial port (eg COM7 or /dev/ttyUSB0)")
	parser.add_argument('-b', '--baud' , default=115200, metavar="baudrate", type=int, help="set the baud rate (default: 115200)")
	parser.add_argument('-v', '--version', action='version', version='DASH7 Logger 0.5', help="show the current version")
	parser.add_argument('-f', '--file', metavar="file", help="write to a specific file")
	args = vars(parser.parse_args())

	init()
	keep_running = True
	# TODO make baud rate configurable
	serial_port = serial.Serial(args['serial'], args['baud'])
	datestr = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
	# TODO should open a file for every log type
	f = open(datestr + '.log', 'w')
	f.write("Start D7 logger (%s)" % datestr)
	f.flush()
	empty_serial_buffer()

	parseThread = parse_d7(dataQueue, displayQueue)
	displayThread = display_d7(displayQueue)
	writeThread = write_d7(f, dataQueue)

	try:
		parseThread.start()
		displayThread.start()
		writeThread.start()

	except (KeyboardInterrupt, SystemExit):
		keep_running = False
		system.exit()
	except Exception as inst:
		print ("Error unable to start thread")
		print (inst)
	
	while keep_running:
		try:
			input = raw_input()
			if input == "x":
				keep_running = False

		except Exception as inst:
			print (inst)

	print("The logger is stopping!")
	parseThread.stop()
	displayThread.stop()
	writeThread.stop()

if __name__ == "__main__":
	main()



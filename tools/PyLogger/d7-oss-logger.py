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
# - Include more debugging options
# - General interface for most of the options.
# - better error controlling

from __future__ import division, absolute_import, print_function, unicode_literals
from colorama import init, Fore, Back, Style
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
serial_port = None
SYNC_WORD = "DD"

dataQueue = Queue.Queue()
displayQueue = Queue.Queue()

get_input = input if system.version_info[0] >= 3 else raw_input
logging.Formatter(fmt='%(asctime)s.%(msecs)d', datefmt='%Y-%m-%d,%H:%M:%S')

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
		self.length = struct.unpack("b", length)[0]


class LogString(Logs):
	prefix = Back.GREEN + Style.BRIGHT + Fore.WHITE + "STRING" + Fore.GREEN + Back.RESET + Style.NORMAL + "  "
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
			string = self.prefix + self.message + Style.RESET_ALL
			return string
		return ""


class LogData(Logs):
	prefix = Back.BLUE + Style.BRIGHT + Fore.WHITE + "DATA" + Fore.BLUE + Back.RESET + Style.NORMAL + "  "
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
			string = self.prefix + str(self.data) + Style.RESET_ALL
			return string
		return ""

class LogTrace(Logs):
	prefix = Back.CYAN + Style.BRIGHT + Fore.WHITE + "TRACE" + Fore.CYAN + Back.RESET + Style.NORMAL + "  "
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
			string = self.prefix + self.message + Style.RESET_ALL
			return string
		return ""

class LogPhyRxRes(Logs):
	prefix = Back.MAGENTA + Style.BRIGHT + Fore.WHITE + "PHY" + Fore.MAGENTA + Back.RESET + Style.NORMAL + "  "
	def __init__(self):
		Logs.__init__(self, "phyrx")

	def read(self):
		self.read_length()
		self.rssi = struct.unpack('B', serial_port.read(size=1))[0]
		self.lqi = struct.unpack('B', serial_port.read(size=1))[0]
		print("We got rssi: %s" % int(self.rssi))
		print("We got lqi: %s" % int(self.lqi))

class LogDllRxRes(Logs):
	def __init__(self):
		Logs.__init__(self, "dllrx")

	def read(self):
		pass


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
					self.file.write(data.write())
				time.sleep(10)
			except Exception as inst:
				print(inst)

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
	log_phy = LogPhy()
	log_trace = LogTrace()

	processedread = {"01" : log_string.read, 
			 "02" : log_data.read,
			 "03" : log_phy.read,
			 "10" : log_trace.read }
		 
	# See if we have found our type in the LOG_TYPES
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
	#writeThread = write_d7(f, dataQueue)

	try:
		parseThread.start()
		displayThread.start()
		#writeThread.start()

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
	#writeThread.stop()

if __name__ == "__main__":
	main()



#! /usr/bin/python
from __future__ import division

import sys as system
import os as os
import serial as serial
import shutil as shutil
import time as time
import struct as struct
import ctypes
import datetime

TEST_MESSAGE_COUNT = 25
FILE_NAME_PREFIX = "pdr_test_"
FILE_EXTENSION = ".csv"

serial_port = None

def read_value_from_serial():
	data = serial_port.read(size=2)
	return struct.unpack("H", data)[0] 

def empty_serial_buffer():
	while serial_port.inWaiting() > 0:
		serial_port.read(1)

def main():
	if (len(system.argv) != 3):
		print "Usage: <serialport (eg COM7)> <baudrate (eg 9600)>"
		system.exit(2)

	print str(datetime.datetime.now())
	global serial_port 
	serial_port = serial.Serial(system.argv[1], system.argv[2])
	test_name = raw_input("Enter test name: ")
	f = open(FILE_NAME_PREFIX + test_name + FILE_EXTENSION, 'w')
	f.write("# distance (m), PDR (%), # packets, timestamp started" + "\n")
	f.write("distance, PDR, packet_count, started_timestamp\n")
	raw_input("Press any key to start testing...")
	stop_testing = False
	while not stop_testing:
		empty_serial_buffer()
		initial_counter = read_value_from_serial()
		counter = initial_counter
		succes_msgs_count = 0
		error_count = 0
		total_msgs_count = 0
		while(total_msgs_count < TEST_MESSAGE_COUNT):
			data = serial_port.read(size=2)
			succes_msgs_count += 1
			new_counter = struct.unpack("H", data)[0]
			if(new_counter > counter + 1):
				error_count += new_counter - (counter + 1)
				print "!!! packet missed"

			counter = new_counter
			total_msgs_count = counter - initial_counter
			print "%(pct)0.2f%% missed of %(total)i messages (%(counter)i)" % \
				{
					'pct': (error_count*100)/total_msgs_count, 
					'total': total_msgs_count,
					'counter': counter
				}

		dist = float(raw_input("Distance between sender and receiver (in m): "))
		pdr = (succes_msgs_count*100)/total_msgs_count
		print "test %(test_name)s with distance %(dist)0.2fm => PDR=%(pdr)0.2f%%" % \
			{
				'pdr': pdr,
				'dist': dist,
				'test_name': test_name
			}

		f.write("%(dist)0.2f, %(pdr)0.2f, %(total)i, %(timestamp)s\n" %
			{
				'dist': dist,
				'pdr': pdr,
				'total': total_msgs_count,
				'timestamp': str(datetime.datetime.now())
			})
		
		f.flush()

		input = raw_input("Press any key to start next test, 'q' to stop testing ... ")
		if input == "q":
			stop_testing = True

if __name__ == "__main__":
	main()
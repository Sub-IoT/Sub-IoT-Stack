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
import numpy as np
from collections import namedtuple

TEST_MESSAGE_COUNT = 100
RESULTS_DIR = "testresults"
FILE_EXTENSION = ".csv"
PDR_RESULT_FILENAME = "pdr"
RUNNING_TEST_FILENAME = "_in_progress"
SYNC_WORD = 'CE'

serial_port = None
SerialData = namedtuple('SerialData', 'mac counter rssi')
test_name = ""

get_input = input if system.version_info[0] >= 3 else raw_input # TODO compatibility beween python 2 and 3, can be removed if we switch to python 3 (waiting on matplotlib)

def read_value_from_serial():
	data = serial_port.read(size=1)
	while not data.encode("hex").upper() == SYNC_WORD:
		print("received unexpected data (%s), waiting for sync word " % data.encode("hex").upper())
		data = serial_port.read(size=1)
		

	mac = serial_port.read(size=8).encode("hex")	
	data = serial_port.read(size=2)
	counter = struct.unpack("H", data)[0] 
	data = serial_port.read(size=1)
	rssi = struct.unpack("b", data)[0] 
	serialData = SerialData(mac, counter, rssi)
	return serialData

def empty_serial_buffer():
	while serial_port.inWaiting() > 0:
		serial_port.read(1)

def wait_for_slave_tag():
	print("Waiting for slave tag ...")
	key = ''
	while key != "y":
		empty_serial_buffer()
		data = read_value_from_serial()
		print("Received slave tag with MAC: %s" % data.mac)
		key = get_input("use this as slave node? (y/n) ")

	return data.mac

def	get_result_dir():
	return RESULTS_DIR + "/" + test_name + "/"

def main():
	if (len(system.argv) != 3):
		print("Usage: <serialport (eg COM7)> <baudrate (eg 9600)>")
		system.exit(2)

	global serial_port 
	serial_port = serial.Serial(system.argv[1], system.argv[2])
	global test_name
	test_name = get_input("Enter test name: ")
	os.mkdir(get_result_dir())
	
	slave_mac = wait_for_slave_tag()
	pdr_csv_file = open(get_result_dir() + PDR_RESULT_FILENAME + "_" + slave_mac + FILE_EXTENSION, 'w')
	pdr_csv_file.write("# distance (m), PDR (%), # packets, timestamp started, avg RSSI value, std dev RSSI" + "\n")
	pdr_csv_file.write("distance, PDR, packet_count, started_timestamp, rssi_avg, rssi_std\n")
	get_input("Press any key to start testing...")
	stop_testing = False
	while not stop_testing:
		empty_serial_buffer()
		initial_counter = read_value_from_serial().counter
		counter = initial_counter
		succes_msgs_count = 0
		error_count = 0
		total_msgs_count = 0
		rssi_values = []
		current_test_file = open(get_result_dir() + RUNNING_TEST_FILENAME, 'w')
		current_test_file.write("timestamp, counter, RSSI, error_pct\n")
		while(total_msgs_count < TEST_MESSAGE_COUNT):
			serialData = read_value_from_serial()
			if(serialData.mac == slave_mac):
				succes_msgs_count += 1
				new_counter = serialData.counter
				if(new_counter > counter + 1):
					error_count += new_counter - (counter + 1)
					print("!!! packet missed")

				counter = new_counter
				total_msgs_count = counter - initial_counter
				rssi_values.append(serialData.rssi)
				print("received counter value %(counter)i @ %(rssi)s dBm ==> %(pct)0.2f%% missed of %(total)i messages" % \
					{
						'pct':(error_count*100)/total_msgs_count, 
						'total': total_msgs_count,
						'counter': counter,
						'rssi': serialData.rssi
					})
				current_test_file.write("%(timestamp)s, %(counter)i, %(rssi)s, %(pct)0.2f\n" % \
					{
						'pct': (error_count*100)/total_msgs_count, 
						'timestamp': str(datetime.datetime.now()),
						'counter': counter,
						'rssi': serialData.rssi
					})
				current_test_file.flush()
			else:
				print("Received data from another mac: %s" % serialData.mac)

		dist = float(get_input("Distance between sender and receiver (in m): "))
		current_test_file.close()
		os.rename(get_result_dir() + RUNNING_TEST_FILENAME, get_result_dir() + time.strftime("%Y%m%d%H%M%S") + "-" + str(dist) + "m")
		pdr = (succes_msgs_count*100)/total_msgs_count
		rssi_avg = np.average(rssi_values)
		rssi_std = np.std(rssi_values)
		print("test %(test_name)s with distance %(dist)0.2fm => PDR=%(pdr)0.2f%%, avg RSSI=%(rssi_avg)0.2f, std RSSI=%(rssi_std)0.2f" % \
			{
				'pdr': pdr,
				'dist': dist,
				'test_name': test_name,
				'rssi_avg': rssi_avg,
				'rssi_std': rssi_std,
			})

		pdr_csv_file.write("%(dist)0.2f, %(pdr)0.2f, %(total)i, %(timestamp)s, %(rssi_avg)0.2f, %(rssi_std)0.2f\n" %
			{
				'dist': dist,
				'pdr': pdr,
				'total': total_msgs_count,
				'timestamp': str(datetime.datetime.now()),
				'rssi_avg': rssi_avg,
				'rssi_std': rssi_std,
			})
		
		pdr_csv_file.flush()

		keypressed = get_input("Press any key to start next test, 'q' to stop testing ... ")
		if keypressed == "q":
			stop_testing = True

if __name__ == "__main__":
	main()

#!/usr/bin/python

import serial
import struct
import time

ser = serial.Serial('/dev/ttyUSB0', 115200)

while 1:
	data = ser.read(1)
	while data != '\xfe':
		print 'unexpected data: ' + str(struct.unpack("B", data)[0])
		data = ser.read(1)

	# got sync byte ...
	channr = struct.unpack("b", ser.read(1))[0]
	rssi = struct.unpack("b", ser.read(1))[0]
	#print datetime.now().strftime("%H:%M:%S.%f") + '\t' + str(channr) + '\t' + str(rssi)
	print str(time.time()) + '\t' + str(channr) + '\t' + str(rssi)

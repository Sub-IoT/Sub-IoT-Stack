#! /usr/bin/python -S

# Create on: Sep 10, 2013
# Authors:
#	jan.stevens@ieee.org
#
# This is helpfull for debuggin the d7-oss-logger and show a basic example of the output
# To use this python script one must install the socat package on linux (windows should use google to find a alternative)
# sudo apt-get install socat
# 
# Then run the following command:
# socat -d -d pty,raw,echo=0 pty,raw,echo=0


#define LOG_PHY 0x01
#define LOG_DLL 0x02
#define LOG_MAC 0x03
#define LOG_NWL 0x04
#define LOG_TRANS 0x05
#define LOG_FWK 0x10


from __future__ import division, absolute_import, unicode_literals, print_function
import sys
import time
import struct as struct
reload(sys) 
sys.setdefaultencoding('utf-8') 
import serial as serial

SYNC_WORD = chr(221)

class LogGenerator(object):
	def __init__(self, serial_port):
		self.port = serial.Serial(serial_port, 115200)

	# Format: 0xDD 0x01 length message
	def string(self, msg):
		self.port.write(SYNC_WORD) # 0xDD
		self.port.write(chr(1))
		self.port.write(chr(len(msg)))
		self.port.write(msg)

	# Format: 0xDD 0x02 length data
	def data(self, data):
		self.port.write(SYNC_WORD)
		self.port.write(chr(2))
		self.port.write(chr(1))
		self.port.write(chr(data))

	# Format: 0xDD 0x03 layer length message
	def stack(self, layer, msg):
		self.port.write(SYNC_WORD)
		self.port.write(chr(3))
		self.port.write(chr(layer))
		self.port.write(chr(len(msg)))
		self.port.write(msg)

	# Format: 0xDD 0xFF length message
	def trace(self, msg):
		self.port.write(SYNC_WORD)
		self.port.write(chr(255))
		self.port.write(chr(len(msg)))
		self.port.write(msg)

	# Format: 0xDD 0XFD length frame_type spectrum_id
	def dll_res(self, frame_type, spectrum_id):
		self.port.write(SYNC_WORD)
		self.port.write(chr(253))
		self.port.write(chr(2))
		self.port.write(frame_type)
		self.port.write(spectrum_id)

	# Format: 0xDD 0xFE length rssi lqi data_length data
	def phy_res(self):
		self.port.write(SYNC_WORD)
		self.port.write(chr(254))
		self.port.write(chr(10))
		self.port.write(chr(211))
		self.port.write(chr(40))
		self.port.write(chr(10))


def main():
	logGen = LogGenerator("/dev/pts/12")
	keep_running = True

	while keep_running:
		try:
			logGen.string("Example of possible output that can be expected from the D7 stack")
			logGen.data(42)

			logGen.stack(1, "We did something in the phy!")
			logGen.stack(2, "A great debug message for the dll layer")
			logGen.stack(3, "We represent the MAC layer")
			logGen.stack(4, "And ofcourse we have the nwl layer")
			logGen.stack(5, "Finally, for now, the trans layer!")
			logGen.stack(10, "And ofcourse we have our framework!")

			logGen.trace("> We go into a function")
			logGen.trace("< We leave a function")

			logGen.dll_res('\x01', '\x12')
			logGen.phy_res()

			print("Send some example data!")
			time.sleep(2)
		except Exception as inst:
			print (inst)

	print("Data emulator stopping!");
if __name__ == "__main__":
	main()
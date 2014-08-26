#! /usr/bin/python

# Created on: Sep 09, 2013
# Authors:
# 	jan.stevens@ieee.org
# 	
# Discription:
# This is a python logger for D7 when logging is active.
# It's an alternative for the Qt logger currently used, which is not that great for installing and quick debugging
# Before using the logger make sure you have installed python 2.7 and the serial library
# There is no current support for Python 3+!
# 
# Install:
# Before usage, install following python packages using PyPI (pip python)
# pip install pyserial
# pip install colorama
# 
# TODO's
# - better error controlling
# - Fix some ugly code (TODO's)
# - should convert all upcase string to lower case and in the display change them to upcase [code convenctions]

from __future__ import division, absolute_import, print_function, unicode_literals
from colorama import init, Fore, Back, Style
from collections import defaultdict
import glob
import sys
import imp
import pcap
from pcap import PCAPFormatter

imp.reload(sys)
sys.setdefaultencoding('utf-8') 
import os as os
import errno
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
PIPE_FILENAME = 'pipe'

### Global variables we need, do not change! ###
serial_port = None
settings = None
dataQueue = Queue.Queue()
displayQueue = Queue.Queue()
trace_pos = 0

#TODO fix this ugly code... but it works
data = [("PHY", "GREEN"), ("DLL", "RED"), ("MAC", "YELLOW"), ("NWL", "BLUE"), ("TRANS", "MAGENTA"), ("FWK", "CYAN")]
stackColors = defaultdict(list)
for layer, color in data:
    stackColors[layer].append(color)

stackLayers = {'01' : "PHY", '02': "DLL", '03': "MAC", '04': "NWL", '05': "TRANS", '10': "FWK"}


get_input = input if sys.version_info[0] >= 3 else raw_input
logging.Formatter(fmt='%(asctime)s.%(msecs)d', datefmt='%Y-%m-%d,%H:%M:%S')

# Small helper function that will format our colors
def formatHeader(header, color, datetime):
    bgColor = getattr(Back, color)
    fgColor = getattr(Fore, color)
    msg = Style.DIM + datetime.strftime("%H:%M:%S.%f") + "  " + bgColor  + Style.BRIGHT + Fore.WHITE + header + fgColor + Back.RESET + Style.NORMAL
    # Make sure we outline everything
    msg += " " * (55 - (len(msg)))
    return msg

# Shortcut for printing errors
def printError(error):
    print(formatHeader("ERROR", "RED", datetime.datetime.now()) + Back.RED  + Style.BRIGHT + Fore.WHITE + str(error) + Style.RESET_ALL)

###
# The different logs classes, every class has its own read, write and print function for costumization
###
class Logs(object):
    def __init__(self, logType):
        self.logType = logType
        self.length = 0
        self.datetime = datetime.datetime.now()

    def read(self):
        pass

    def write(self):
        pass

    def __str__(self):
        pass

    def read_length(self):
        length = serial_port.read(size = 1)
        self.length = int(struct.unpack('B', length)[0])


class LogString(Logs):
    def __init__(self):
        Logs.__init__(self, "string")

    def read(self):
        # Read the length of the message
        self.read_length()
        self.message = serial_port.read(size=self.length)
        return self

    def write(self):
        if settings["string"]:
            return "STRING: " + self.message + "\n"
        return ""

    def __str__(self):
        if settings["string"]:
            string = formatHeader("STRING", "GREEN", self.datetime) + self.message + Style.RESET_ALL
            return string + "\n"
        return ""


class LogData(Logs):
    def __init__(self):
        Logs.__init__(self, "data")

    def read(self):
        self.read_length()
        data = serial_port.read(size=self.length)
        self.data = data.encode("hex").upper()
        return self

    def write(self):
        if settings["data"]:
            return "DATA: " + str(self.data) + "\n"
        return ""

    def __str__(self):
        if settings["data"]:
            string = formatHeader("DATA", "BLUE", self.datetime) + str(self.data) + Style.RESET_ALL
            return string + "\n"
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
        #self.message = struct.unpack("I", self.message)
        return self

    def write(self):
        if settings["stack"] and settings[self.layer.lower()]:
            return self.layer + ": " + self.message + "\n"
        return ""

    def __str__(self):
        if settings["stack"] and settings[self.layer.lower()]:
            string = formatHeader("STK: " + self.layer, self.color, self.datetime) + self.message + Style.RESET_ALL
            return string + "\n"
        return ""

class LogTrace(Logs):
    def __init__(self):
        Logs.__init__(self, "trace")

    def read(self):
        self.read_length()
        self.message = serial_port.read(size=self.length)
        return self

    def write(self):
        if settings["trace"]:
            return "TRACE: " + self.message + "\n"
        return ""

    def __str__(self):
        global trace_pos
        if settings["trace"]:
            string = formatHeader("TRACE", "YELLOW", self.datetime) + self.message + Style.RESET_ALL

            return string + "\n"
        return ""


class LogDllRes(Logs):
    def __init__(self):
        Logs.__init__(self, "dllres")

    def read(self):
        self.read_length()
        self.frame_type = str(struct.unpack('B', serial_port.read(size=1))[0])
        self.spectrum_id = "0x" + str(serial_port.read(size=1).encode("hex").upper())
        return self

    def write(self):
        if settings["dllres"]:
            return "DLL RES: frame_type " + self.frame_type + " spectrum_id " + self.spectrum_id + "\n"
        return ""

    def __str__(self):
        if settings["dllres"]:
            string = formatHeader("DLL RES", "RED", self.datetime) + "frame_type " + self.frame_type + " spectrum_id " + self.spectrum_id + Style.RESET_ALL
            return string + "\n"
        return ""

class LogPhyRes(Logs):
    def __init__(self):
        Logs.__init__(self, "phyres")

    def read(self):
        self.raw_data = bytearray() # TODO refactor
        self.read_length()
        raw_rssi = serial_port.read(size=1)
        self.rssi = struct.unpack('b', raw_rssi)[0]
        self.raw_data.append(raw_rssi)
        raw_lqi = serial_port.read(size=1)
        self.lqi = struct.unpack('B', raw_lqi)[0]
        self.raw_data.append(raw_lqi)
        raw_packet_length = serial_port.read(size=1)
        self.packet_length = struct.unpack('B', raw_packet_length)[0]
        self.raw_data.append(raw_packet_length)
        self.data = serial_port.read(size=self.packet_length)
        self.raw_data.extend(self.data)
        self.subnet = "0x" + str(self.data[2].encode("hex").upper())
        self.tx_eirp = int(struct.unpack('B', self.data[1])[0]) * 0.5 - 40
        self.frame_ctl = "0x" + str(self.data[3].encode("hex").upper())
        self.source_id = str(self.data[6:14].encode("hex").upper())
        self.data_length = self.packet_length - 16
        self.data = self.dataEncoding(self.data[14:14+self.data_length])
        self.crc = self.dataEncoding(self.data[14+self.data_length:16+self.data_length])

        #self.data = [data[i:i+2].decode("utf-8", errors='replace') for i in range(0, len(data), 2)]
        #self.data = [data[i].encode("hex").upper() for i in range(0, len(data))]
        #self.data = [str(struct.unpack('b', data[i])[0]) for i in range(0, len(data))]
        #print("packet_length %s" % self.packet_length)
        #print("dump_data: %s" % self.data)
        #print("rssi: %s" % self.rssi)
        #print("lqi: %s" % self.lqi)
        #print("data_length: %s" % self.data_length)
        return self

    def dataEncoding(self, byte_str):
        if settings["display"] == 'hex':
            return " ".join(["0x" + byte_str[i].encode("hex").upper() for i in range(0, len(byte_str))])
        elif settings["display"] == 'bin':
            return " ".join([bin(int(byte_str[i].encode("hex").upper(), 16))[2:].zfill(8) for i in range(0, len(byte_str))])
        elif settings["display"] == 'dec':
            return " ".join([str(struct.unpack('b', byte_str[i])[0]) for i in range(0, len(byte_str))])
        else:
            return " ".join([byte_str[i].decode("utf-8", errors='replace') for i in range(0, len(byte_str))])

    def write(self):
        if settings["phyres"]:
            string = "PHY RES: "
            string += " rssi: " + str(self.rssi) + " lqi: " + str(self.lqi) + " subnet: " + str(self.subnet)
            string += " tx_eirp: " + str(self.tx_eirp) + " frame_ctl: " + str(self.frame_ctl) + " source: " + self.source_id
            string += " data: " + str(self.data) + "\n"
            return string
        return ""

    def get_raw_data(self):
        return self.raw_data

    def __str__(self):
        if settings["phyres"]:
            #TODO format as table, this is quite ugly, there is a easier way, should look into it
            string = formatHeader("PHY RES", "GREEN", self.datetime) + "Received packet from: " + self.source_id + "\n"
            string += " " * 22 + "rssi: " + str(self.rssi) + " dBm"+ " " * 7 + "lqi: " + str(self.lqi) + " "*14 + "subnet: " + str(self.subnet) + "\n"
            string += " " * 22 + "tx_eirp: " + str(self.tx_eirp) + " dBm   " + "frame_ctl: " + str(self.frame_ctl) +  " "*7+ "CRC: " + str(self.crc) + "\n"
            string += " " * 22 + "data: " + str(self.data)
            string += Style.RESET_ALL
            return string + "\n"
        return ""



##
# Different threads we use
##
class parse_d7(threading.Thread):
    def __init__(self, pcap_file, is_pipe_enabled, disQueue):
        self.keep_running = True
        self.pcap_file = pcap_file
        self.is_pipe_enabled = is_pipe_enabled
        self.is_pipe_connected = False
        self.pipe = None
        self.disQueue = disQueue
        threading.Thread.__init__(self)

    def run(self):
        while self.keep_running:
            try:
                serialData = read_value_from_serial()
                if serialData is not None:
                    self.disQueue.put(serialData)
                    if isinstance(serialData, LogPhyRes):
                        if self.pcap_file != None:
                            self.pcap_file.write(PCAPFormatter.build_record_data(serialData.get_raw_data()))
                            self.pcap_file.flush()
                        if self.is_pipe_enabled:
                            if self.is_pipe_connected is False:
                                try:
                                    self.pipe = os.open(PIPE_FILENAME, os.O_WRONLY | os.O_NONBLOCK)
                                    os.write(self.pipe, PCAPFormatter.build_global_header_data())
                                    self.is_pipe_connected = True
                                except OSError as e:
                                    if e.errno == errno.ENXIO:
                                        print("Wireshark not listening yet ...")
                            else:
                                try:
                                    os.write(self.pipe, PCAPFormatter.build_record_data(serialData.get_raw_data()))
                                except OSError as e:
                                    if e.errno == errno.EPIPE:
                                        print("Wireshark stopped listening, waiting for reconnection ...")
                                        self.is_pipe_connected = False
            except Exception as inst:
                printError(inst)

class display_d7(threading.Thread):
    def __init__(self, disQueue):
        self.keep_running = True
        self.queue = disQueue
        threading.Thread.__init__(self)

    def run(self):
        while self.keep_running:
            try:
                while not self.queue.empty():
                    data = self.queue.get()
                    print(data, end='')
            except Exception as inst:
                printError(inst)

class write_d7(threading.Thread):
    def __init__(self, fileStream, queue):
        self.keep_running = True
        self.file = fileStream
        self.queue = queue
        threading.Thread.__init__(self)

    def run(self):
        while self.keep_running and settings['file'] != None:
            try:
                while not self.queue.empty():
                    data = self.queue.get()
                    encoded = (data.write()).encode('utf-8')
                    self.file.write(data.datetime.strftime("%Y/%m/%d %H:%M:%S  "))
                    self.file.write(encoded)
                time.sleep(10)
            except Exception as inst:
                printError(inst)

## Helper functions ##
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
    log_phy_res = LogPhyRes()


    result = {
             "01" : log_string.read,
             "02" : log_data.read,
             "03" : log_stack.read,
             "FD" : log_dll_res.read,
             "FE" : log_phy_res.read,
             "FF" : log_trace.read, }.get(logtype, dummy)

    # See if we have found our type in the LOG_TYPES
    #print("We got logtype: %s" % logtype)
    #result = processedread[logtype]()
    return result()

def empty_serial_buffer():
    while serial_port.inWaiting() > 0:
        serial_port.read(1)

def list_serial_ports():
    # Check for windows
    available = []
    if os.name == 'nt':
        # Scan available ports
        for i in range(256):
            try:
                s = serial.Serial(i)
                available.append('COM'+str(i+1))
                s.close()
            except serial.SerialException:
                pass
    else:
        # for linux only include the USB0 com ports
        available = glob.glob('/dev/ttyUSB*')

    for port in available:
        print(formatHeader("PORT", "GREEN", datetime.datetime.now()) + port + Style.RESET_ALL)

## Main function ##
def main():
    global serial_port, settings
    keep_running = True
    # Some variables we need
    init()
    dateTime = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")

    # Setup the console parser
    parser = argparse.ArgumentParser(description = "DASH7 logger for the OSS-7 stack. You can exit the logger using Ctrl-c, it takes some time.")
    parser.add_argument('serial', default="COM12", metavar="serial port", help="serial port (eg COM7 or /dev/ttyUSB0)", nargs='?')
    parser.add_argument('-b', '--baud' , default=115200, metavar="baudrate", type=int, help="set the baud rate (default: 9600)")
    parser.add_argument('-v', '--version', action='version', version='DASH7 Logger 0.5', help="show the current version")
    parser.add_argument('-f', '--file', metavar="file", help="write to a pcap file", nargs='?', default=None, const=dateTime)
    parser.add_argument('-p', '--pipe', help="stream live pcap data to a named pipe", action="store_true", default=False) # TODO print filename
    parser.add_argument('-l', '--list', help="Lists available serial ports", action="store_true", default=False)
    general_options = parser.add_argument_group('general logging')
    general_options.add_argument('--string', help="Disable string logs", action="store_false", default=True)
    general_options.add_argument('--data', help="Disable data logs", action="store_false", default=True)
    general_options.add_argument('--trace', help="Disable trace logs", action="store_false", default=True)
    stack_options = parser.add_argument_group('stack logging')
    stack_options.add_argument('--phy', help="Disable logs for phy", action="store_false", default=True)
    stack_options.add_argument('--dll', help="Disable logs for dll", action="store_false", default=True)
    stack_options.add_argument('--mac', help="Disable logs for mac", action="store_false", default=True)
    stack_options.add_argument('--nwl', help="Disable logs for nwl", action="store_false", default=True)
    stack_options.add_argument('--trans', help="Disable logs for trans", action="store_false", default=True)
    stack_options.add_argument('--fwk', help="Disable logs for fwk", action="store_false", default=True)
    stack_options.add_argument('--stack', help="Disable all stack logs", action="store_false", default=True)
    special_options = parser.add_argument_group('special logging')
    special_options.add_argument('--dllres', help="Disable DLL RES logs", action="store_false", default=True)
    special_options.add_argument('--phyres', help="Disable PHY RES Logs", action="store_false", default=True)
    special_options.add_argument('--display', help="Format the data of PHY RES", choices=['hex', 'bin', 'txt', 'dec'], default='hex')
    settings = vars(parser.parse_args())

    # We only want to list the serial ports, then exit
    if settings["list"]:
        list_serial_ports()
        sys.exit()

    # Setup the serial port
    if settings["serial"] is None:
        printError("You didn't specify a serial port!")
        sys.exit()

    serial_port = serial.Serial(settings['serial'], settings['baud'])
    empty_serial_buffer()

    # Array containing all the threads
    threads = []
    # Only write a file if we have a file defined
    pcap_file = None
    if settings["file"] != None:
        # TODO check if file already exists
        pcap_file = open(settings["file"], 'w')
        pcap_file.write(PCAPFormatter.build_global_header_data())
        pcap_file.flush()

    if settings["pipe"]:
        if not os.path.exists(PIPE_FILENAME):
            os.mkfifo(PIPE_FILENAME)

    threads.append(parse_d7(pcap_file, settings["pipe"], displayQueue))
    threads.append(display_d7(displayQueue))

    try:
        for t in threads:
            t.start()
    except Exception as inst:
        printError("Error unable to start thread")
        printError(inst)

    while keep_running:
        try:
            # Sleep a very short time, we are just waiting for a keyboard intterupt really
            time.sleep(0.0001)
        except KeyboardInterrupt:
            print("\nCtrl-c received! Sending Kill to the threads...")
            for t in threads:
                t.keep_running = False
            keep_running = False

    print("The logger is stopping, please wait")
    sys.exit()

if __name__ == "__main__":
    main()
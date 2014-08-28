__author__ = 'Glenn Ergeerts'

import struct
import os
import errno

class WiresharkNamedPipeLogger:
    PIPE_FILENAME = '/tmp/oss7-pipe'

    is_pipe_connected = False

    def __init__(self):
        if not os.path.exists(self.PIPE_FILENAME):
            os.mkfifo(self.PIPE_FILENAME)

    def write(self, serialData):
        if self.is_pipe_connected is False:
            try:
                self.pipe = os.open(self.PIPE_FILENAME, os.O_WRONLY | os.O_NONBLOCK)
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

class PCAPFormatter:
    MAGIC_NUMBER = 0xa1b2c3d4
    VERSION_MAJOR = 2
    VERSION_MINOR = 4
    THISZONE = 0
    SIGFIGS = 0
    SNAPLEN = 0xffff
    NETWORK = 147 # LINKTYPE_USER0

    STRUCT_FMT_GLOBAL_HDR = '<LHHlLLL'
    STRUCT_FMT_RECORD_HDR = '<LLLL'

    @staticmethod
    def build_global_header_data():
        return struct.pack(PCAPFormatter.STRUCT_FMT_GLOBAL_HDR,
                           PCAPFormatter.MAGIC_NUMBER,
                           PCAPFormatter.VERSION_MAJOR,
                           PCAPFormatter.VERSION_MINOR,
                           PCAPFormatter.THISZONE,
                           PCAPFormatter.SIGFIGS,
                           PCAPFormatter.SNAPLEN,
                           PCAPFormatter.NETWORK)
    @staticmethod
    def build_record_data(packet):
        print len(packet)
        return struct.pack(PCAPFormatter.STRUCT_FMT_RECORD_HDR,
                          0, # TODO seconds since epoch
                          0, # TODO usec relative to seconds since epoch
                          len(packet),
                          len(packet)
                          ) + packet
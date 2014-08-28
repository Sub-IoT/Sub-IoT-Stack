__author__ = 'Glenn Ergeerts'

import struct
import os
import errno
import sys
if sys.platform == 'win32':
    import win32pipe, win32file, pywintypes

class WiresharkNamedPipeLogger:
    PIPE_FILENAME_POSIX = '/tmp/oss7-pipe'
    PIPE_FILENAME_WIN32 = r'\\.\pipe\oss7-pipe'

    pipe_filename = PIPE_FILENAME_POSIX
    is_pipe_connected = False
    is_win32 = False

    def __init__(self):
        if sys.platform == 'win32':
            self.is_win32 = True
            self.pipe_filename = self.PIPE_FILENAME_WIN32

        if not os.path.exists(self.pipe_filename):
            if self.is_win32:
                self.pipe = win32pipe.CreateNamedPipe(self.pipe_filename, win32pipe.PIPE_ACCESS_OUTBOUND,
                                                      win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_NOWAIT,
                                                      1, 65536, 65536, 30000, None)
            else:
                os.mkfifo(self.pipe_filename)

    def write(self, serialData):
        if self.is_pipe_connected is False:
            try:
                if self.is_win32:
                    win32pipe.ConnectNamedPipe(self.pipe, None)
                    win32file.WriteFile(self.pipe, PCAPFormatter.build_global_header_data())
                else:
                    self.pipe = os.open(self.pipe_filename, os.O_WRONLY | os.O_NONBLOCK)
                    os.write(self.pipe, PCAPFormatter.build_global_header_data())

                self.is_pipe_connected = True
                print("Wireshark connected!")
            except OSError as e:
                if e.errno == errno.ENXIO:
                    print("Wireshark not listening yet ...")
                else:
                    raise
            except pywintypes.error as e:
                if e.winerror == 536:
                    print("Wireshark not listening yet ...")
                else:
                    raise
        else:
            try:
                data = PCAPFormatter.build_record_data(serialData.get_raw_data())
                if self.is_win32:
                    win32file.WriteFile(self.pipe, data)
                else:
                    os.write(self.pipe, data)
            except OSError as e:
                if e.errno == errno.EPIPE:
                    print("Wireshark stopped listening, waiting for reconnection ...")
                    self.is_pipe_connected = False
            except pywintypes.error as e:
                if e.winerror == 232:
                    print("Wireshark stopped listening, waiting for reconnection ...")
                    win32pipe.DisconnectNamedPipe(self.pipe)
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
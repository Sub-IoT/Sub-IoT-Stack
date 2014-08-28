__author__ = 'glenn'

import struct

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

if __name__ == "__main__":
    s = PCAPFormatter.build_global_header_data() + PCAPFormatter.build_record_data('test') + PCAPFormatter.build_record_data('test')
    newfile = open('temp.pcap', 'wb')
    newfile.write(bytearray(s))
    newfile.close()
    print " ".join(["0x" + s[i].encode("hex").upper() for i in range(0, len(s))])

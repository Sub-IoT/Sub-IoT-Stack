#!/usr/bin/env python

# simple python wrapper around the serial connection, to dump its contents
# author: Christophe VG <contact@christophe.vg>

import argparse
import serial
import sys

from signal import signal, SIGPIPE, SIG_DFL
signal(SIGPIPE, SIG_DFL) 

def dump(config):
  ser = serial.Serial(config.serial, config.baudrate)
  if config.verbose:
    print "*** connected to {0}:{1}\n".format(config.serial, config.baudrate)

  try:
    while True:
      sys.stdout.write(ser.read())
      sys.stdout.flush()
  except KeyboardInterrupt:
    sys.exit(0)

if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description="Dumps received bytes from a serial connection to stdout."
  )

  parser.add_argument("-v", "--verbose", help="be verbose",
                      action='store_true', default=False)
  parser.add_argument("-b", "--baudrate", help="baudrate", default=9600)
  parser.add_argument("-s", "--serial",   help="serial port to dump")

  config = parser.parse_args()

  dump(config)

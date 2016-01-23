import sys, serial, argparse
import numpy as np
from time import sleep
from collections import deque

import time
import os as os

import matplotlib.pyplot as plt 
import matplotlib.animation as animation


# 868N016,3929,-080,-076,-075,-079,-076,-077,-074,-077,-077,-076

global analogPlot
	
# plot class
class AnalogPlot:
	# constr
	def __init__(self, strPort, maxLen, axes, a0, a1):
		# open serial port
		self.ser = serial.Serial(strPort, 115200)

		self.ax = deque([-140.0]*maxLen)
		self.max = deque([-140.0]*maxLen)
		self.maxLen = maxLen
		self.previousChannel = -1
		self.rolling = False #rolling -> same channel, otherwise x-axis= channels
		self.index = 0
		self.axes = axes
		self.a0 = a0
		self.a1 = a1

	# add to buffer
	def addToBuf(self, buf, index, val):
		#print "index %d len(buf) %d" % (index, len(buf))
		while (len(buf) < index):
			extra = index+1-len(buf)
			buf.extend([-140.0]*extra)
			self.max.extend([-140.0]*extra)
			#print "index %d len(buf) %d" % (index, len(buf))
			
		
		#print "self.maxLen %d len(buf) %d" % (self.maxLen, len(buf))
		if self.maxLen < len(buf):
			self.maxLen = len(buf)
			self.axes.set_xlim([0, self.maxLen])
		
		if (len(buf) < self.maxLen):
			buf.append(index, val)
			self.max.append(index, val)
		else:
			buf[index] = val
			#print "self.max %d len(buf) %d index %d" % (len(self.max), len(buf), index)
			if self.max[index] < val:
				self.max[index]= val

	# add data
	def add(self, data):
		assert(len(data) == 12)
		channel = int(data[0][4:])
		rss_values = [float(val) for val in data[2:12]]
		rss = np.amax(rss_values)
		
		rss_values = [float(val) for val in data[2:12]]
		rss = np.amax(rss_values)
		
		#print "channel %d previous %d rolling %d" % (channel, self.previousChannel, self.rolling)
		
		if self.previousChannel != -1:
			if (self.rolling):
				if self.previousChannel != channel:
					self.rolling = False
					#self.ax = deque([-130.0]*self.maxLen)
					self.axes.set_xlabel('Channel Number')
					self.axes.set_title("Noise logger " + data[0][:4])
				else:
					self.addToBuf(self.ax, self.index, rss)
					self.index += 1
					if self.index >= self.maxLen:
						self.index = 0
			else:
				if self.previousChannel == channel:
					self.rolling = True
					self.maxLen = 100
					self.ax = deque([-130.0]*self.maxLen)
					self.axes.set_xlim([0, self.maxLen])					
					self.axes.set_xlabel('Measurement')
					self.axes.set_title("Noise logger " + data[0])
				else:
					self.addToBuf(self.ax, channel, rss)
		else:
			self.axes.set_title("Noise logger " + data[0][:4])
		
		self.previousChannel = channel
		
		
			
		#self.addToBuf(self.ay, float(data[2]))

	# update plot
	def update(self, frameNum):
		try:
			line = self.ser.readline()
			#line = "868N016,3929,-080,-076,-075,-079,-076,-077,-074,-077,-077,-076"
			print line
			data = line.split(',')#[float(val) for val in line.split(',')]
			#print data
			if(len(data) == 12):
				self.add(data)				
				#print "xlen %d ylen %d" % (len(range(self.maxLen)), len(self.ax))
				self.a0.set_data(range(self.maxLen), self.ax)
				self.a1.set_data(range(self.maxLen), self.max)
		except serial.serialutil.SerialException as inst:
			if (str(inst) == "call to ClearCommError failed"):
				print("lost COM port, retrying...")
				self.close()
				time.sleep(5)
				relaunch()
			else:
				print(inst)
		except KeyboardInterrupt:
			print('exiting')
		
		return self.a0,

	# clean up
	def close(self):
		# close serial
		self.ser.flush()
		self.ser.close()	

# main() function
def main():
	global analogPlot
	# create parser
	parser = argparse.ArgumentParser(description="NoiseLogger Serial")
	# add expected arguments
	parser.add_argument('--port', dest='port', required=True)

	# parse args
	args = parser.parse_args()
	
	
	strPort = args.port
	maxValue = 100

	print('reading from serial port %s...' % strPort)

	

	

	# set up animation
	fig = plt.figure()
	ax = plt.axes(xlim=(0, maxValue), ylim=(-140, 30))
	a0, = ax.plot([], [], '.')
	a1, = ax.plot([], [], '.')
	ax.set_title("Noise logger")
	ax.set_xlabel('Channel Number')
	ax.set_ylabel('RSS (dBm)')
	ax.grid(True)
	
	print('plotting data...')
	
	# plot parameters
	analogPlot = AnalogPlot(strPort, maxValue, ax, a0, a1)
	anim = animation.FuncAnimation(fig, analogPlot.update, interval=5, blit=False)
								
	
								
	# # show plot
	plt.show()
	
	# # clean up
	analogPlot.close()

	print('exiting.')

def relaunch():
	global analogPlot
	analogPlot.close()
	args = "" # must declare first
	for i in sys.argv:
		args = args + " " + str(i) # force casting to strings with str() to concatenate
	os.system("python" + args) # do it!


# call main
if __name__ == '__main__':
	main()
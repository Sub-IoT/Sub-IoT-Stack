import sys, serial, argparse
import numpy as np
from time import sleep
from collections import deque

import matplotlib.pyplot as plt 
import matplotlib.animation as animation


# 868N016,3929,-080,-076,-075,-079,-076,-077,-074,-077,-077,-076

	
# plot class
class AnalogPlot:
	# constr
	def __init__(self, strPort, maxLen):
		# open serial port
		self.ser = serial.Serial(strPort, 115200)

		self.ax = deque([-130.0]*maxLen)
		self.ay = deque([0.0]*maxLen)
		self.maxLen = maxLen

	# add to buffer
	def addToBuf(self, buf, index, val):
		if len(buf) < self.maxLen:
			buf.append(index, val)
		else:
			buf[index] = val

	# add data
	def add(self, data):
		assert(len(data) == 12)
		channel = int(data[0][4:])
		rss_values = [float(val) for val in data[2:12]]
		rss = np.amax(rss_values)
		
		self.addToBuf(self.ax, channel, rss)	
		#self.addToBuf(self.ay, float(data[2]))

	# update plot
	def update(self, frameNum, a0, a1):
		print ("update")
		try:
			line = self.ser.readline()
			#line = "868N016,3929,-080,-076,-075,-079,-076,-077,-074,-077,-077,-076"
			print line
			data = line.split(',')#[float(val) for val in line.split(',')]
			print data
			if(len(data) == 12):
				self.add(data)
				a0.set_data(range(self.maxLen), self.ax)
				#a1.set_data(range(self.maxLen), self.ay)
		except KeyboardInterrupt:
			print('exiting')
		
		return a0, 

	# clean up
	def close(self):
		# close serial
		self.ser.flush()
		self.ser.close()	

# main() function
def main():
	# create parser
	parser = argparse.ArgumentParser(description="NoiseLogger Serial")
	# add expected arguments
	parser.add_argument('--port', dest='port', required=True)

	# parse args
	args = parser.parse_args()
	
	
	strPort = args.port
	maxValue = 280

	print('reading from serial port %s...' % strPort)

	# plot parameters
	analogPlot = AnalogPlot(strPort, maxValue)

	print('plotting data...')

	# set up animation
	fig = plt.figure()
	ax = plt.axes(xlim=(0, maxValue), ylim=(-120, 30))
	a0, = ax.plot([], [], '.')
	a1, = ax.plot([], [])
	anim = animation.FuncAnimation(fig, analogPlot.update, 
								fargs=(a0, a1), 
								interval=50)

	# # show plot
	plt.show()
	
	# # clean up
	analogPlot.close()

	print('exiting.')
	

# call main
if __name__ == '__main__':
	main()
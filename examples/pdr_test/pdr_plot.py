#! /usr/bin/python

import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
import sys

# Main function
def main():
	if (len(sys.argv) != 2):
		print "Usage: data.csv"
		sys.exit(2)

	data = mlab.csv2rec(sys.argv[1])
	data.sort(order='distance')
	plt.plot(data['distance'], data['pdr'], 'o-')	
	plt.ylabel("PDR (%)")
	plt.xlabel("distance (m)")
	plt.axis([0, data['distance'].max(), 0, 100])
	plt.title("Package Delivery Ratio in function of distance")
	plt.grid(True)
	plt.show()

# Main function
if __name__ == "__main__":
    main()
    
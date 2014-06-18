#! /usr/bin/python

import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
import sys

def main():
	if (len(sys.argv) != 2):
		print "Usage: <filename (eg data.csv)"
		sys.exit(2)

	data = mlab.csv2rec(sys.argv[1])
	data.sort(order='distance')
	ax_pdr = plt.subplot(111)
	pdr = plt.plot(data['distance'], data['pdr'], 'o-', label = 'PDR')	
	plt.ylabel("PDR (%)")
	plt.xlabel("distance (m)")
	plt.axis([0, data['distance'].max(), 0, 110])
	plt.title("Package Delivery Ratio and avg RSSI in function of distance")
	plt.grid(True)
	ax_pdr.legend(loc=0)
	ax_rssi = plt.twinx()
	plt.ylabel("RSSI (dBm)")
	rssi = plt.plot(data['distance'], data['rssi_avg'], 'r--', label = 'Avg RSSI')
	lines = pdr + rssi
	labels = [l.get_label() for l in lines]
	ax_rssi.legend(lines, labels, loc='best')
	plt.show()

if __name__ == "__main__":
    main()
    
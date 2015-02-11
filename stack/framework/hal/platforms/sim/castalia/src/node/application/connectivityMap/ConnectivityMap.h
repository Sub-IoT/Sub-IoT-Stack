/*******************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2011                            *
 *  Developed at the ATP lab, Networked Systems research theme                 *
 *  Author(s): Athanassios Boulis, Dimosthenis Pediaditakis, Yuriy Tselishchev *
 *  This file is distributed under the terms in the attached LICENSE file.     *
 *  If you do not find this file, copies can be found by writing to:           *
 *                                                                             *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia                *
 *      Attention:  License Inquiry.                                           *
 *                                                                             *
 *******************************************************************************/

#ifndef _CONNECTIVITYMAP_H_
#define _CONNECTIVITYMAP_H_

#include "VirtualApplication.h"

using namespace std;

struct neighborRecord {
	int id;
	int timesRx;
	int receivedPackets;
};

enum ConnectivityMapTimers {
	SEND_PACKET = 1,
};

class ConnectivityMap: public VirtualApplication {
 private:
	// parameters and variables
	int priority;
	int packetHeaderOverhead;
	bool printConnMap;
	int constantDataPayload;
	double packetSpacing;
	int packetsPerNode;
	int packetSize;

	vector<neighborRecord> neighborTable;
	int packetsSent;
	int serialNumber;
	int totalSNnodes;
	double txInterval_perNode;
	double txInterval_total;

 protected:
	void startup();
	void finishSpecific();
	void fromNetworkLayer(ApplicationPacket *, const char *, double, double);
	void timerFiredCallback(int);
	void updateNeighborTable(int nodeID, int theSN);
};

#endif				// _CONNECTIVITYMAP_APPLICATIONMODULE_H_

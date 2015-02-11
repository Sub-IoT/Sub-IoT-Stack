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

#include "ConnectivityMap.h"

Define_Module(ConnectivityMap);

void ConnectivityMap::startup()
{
	packetSpacing = (double)par("packetSpacing") / 1000.0;
	packetsPerNode = par("packetsPerNode");
	packetSize = par("packetSize");

	neighborTable.clear();
	packetsSent = 0;
	totalSNnodes = getParentModule()->getParentModule()->par("numNodes");

	txInterval_perNode = packetsPerNode * packetSpacing;
	txInterval_total = (txInterval_perNode * totalSNnodes);

	if (strtod(ev.getConfig()->getConfigValue("sim-time-limit"), NULL) < txInterval_total) {
		trace() << "ERROR: Total sim time should be at least = " << txInterval_total;
		opp_error("\nError: simulation time not large enough for the conectivity map application\n");
	}

	double startTxTime = txInterval_perNode * self;
	setTimer(SEND_PACKET, startTxTime);
}

void ConnectivityMap::fromNetworkLayer(ApplicationPacket * rcvPacket, const char *source, double rssi, double lqi)
{
	updateNeighborTable(atoi(source), rcvPacket->getSequenceNumber());
}

void ConnectivityMap::timerFiredCallback(int timerIndex)
{
	switch (timerIndex) {

		case SEND_PACKET:{
			if (packetsSent >= packetsPerNode)
				break;
			toNetworkLayer(createGenericDataPacket(0.0, packetsSent, packetSize), BROADCAST_NETWORK_ADDRESS);
			packetsSent++;
			setTimer(SEND_PACKET, packetSpacing);
			break;
		}
	}
}

void ConnectivityMap::finishSpecific()
{
	declareOutput("Packets received");
	for (int i = 0; i < (int)neighborTable.size(); i++) {
		collectOutput("Packets received", neighborTable[i].id,
			      "Success", neighborTable[i].receivedPackets);
	}
}

void ConnectivityMap::updateNeighborTable(int nodeID, int serialNum)
{
	int i = 0, pos = -1;
	int tblSize = (int)neighborTable.size();

	for (i = 0; i < tblSize; i++)
		if (neighborTable[i].id == nodeID)
			pos = i;

	if (pos == -1) {
		neighborRecord newRec;
		newRec.id = nodeID;
		newRec.timesRx = 1;

		if ((serialNum >= 0) && (serialNum < packetsPerNode))
			newRec.receivedPackets = 1;

		neighborTable.push_back(newRec);
	} else {
		neighborTable[pos].timesRx++;

		if ((serialNum >= 0) && (serialNum < packetsPerNode))
			neighborTable[pos].receivedPackets++;
	}
}


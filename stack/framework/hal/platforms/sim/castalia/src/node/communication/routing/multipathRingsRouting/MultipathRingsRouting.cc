/*******************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2011                            *
 *  Developed at the ATP lab, Networked Systems research theme                 *
 *  Author(s): Athanassios Boulis, Yuriy Tselishchev                           *
 *  This file is distributed under the terms in the attached LICENSE file.     *
 *  If you do not find this file, copies can be found by writing to:           *
 *                                                                             *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia                *
 *      Attention:  License Inquiry.                                           *
 *                                                                             *  
 *******************************************************************************/

#include "MultipathRingsRouting.h"

Define_Module(MultipathRingsRouting);

void MultipathRingsRouting::startup()
{
	netSetupTimeout = (double)par("netSetupTimeout") / 1000.0;
	mpathRingsSetupFrameOverhead = par("mpathRingsSetupFrameOverhead");

	// check that the Application module used has the boolean parameter "isSink"
	cModule *appModule = getParentModule()->getParentModule()->getSubmodule("Application");
	if (appModule->hasPar("isSink"))
		isSink = appModule->par("isSink");
	else
		opp_error("\nMultiPathRings routing has to be used with an application that defines the parameter isSink");

	currentLevel = tmpLevel = isSink ? 0 : NO_LEVEL;
	currentSinkID = tmpSinkID = isSink ? self : NO_SINK;

	isConnected = (isSink) ? true : false;
	isScheduledNetSetupTimeout = false;
	currentSequenceNumber = 0;

	if (isSink)
		sendTopologySetupPacket();
}

void MultipathRingsRouting::sendTopologySetupPacket()
{
	MultipathRingsRoutingPacket *setupPkt =
	    new MultipathRingsRoutingPacket("Multipath rings routing setup packet", NETWORK_LAYER_PACKET);
	setupPkt->setMultipathRingsRoutingPacketKind(MPRINGS_TOPOLOGY_SETUP_PACKET);
	setupPkt->setSource(SELF_NETWORK_ADDRESS);
	setupPkt->setDestination(BROADCAST_NETWORK_ADDRESS);
	setupPkt->setSinkID(currentSinkID);
	setupPkt->setSenderLevel(currentLevel);
	toMacLayer(setupPkt, BROADCAST_MAC_ADDRESS);
}

void MultipathRingsRouting::sendControlMessage(multipathRingsRoutingControlDef kind)
{
	MultipathRingsRoutingControlMessage *ctrlMsg =
	    new MultipathRingsRoutingControlMessage("Multipath routing control message",NETWORK_CONTROL_MESSAGE);
	ctrlMsg->setMultipathRingsRoutingControlMessageKind(kind);
	ctrlMsg->setLevel(currentLevel);
	ctrlMsg->setSinkID(currentSinkID);
	toApplicationLayer(ctrlMsg);
}

void MultipathRingsRouting::timerFiredCallback(int index)
{
	if (index != TOPOLOGY_SETUP_TIMEOUT)
		return;

	isScheduledNetSetupTimeout = false;
	if (tmpLevel == NO_LEVEL) {
		setTimer(TOPOLOGY_SETUP_TIMEOUT, netSetupTimeout);
		isScheduledNetSetupTimeout = true;
	} else if (currentLevel == NO_LEVEL) {
		//Broadcast to all nodes of currentLevel-1
		currentLevel = tmpLevel + 1;
		currentSinkID = tmpSinkID;

		if (!isConnected) {
			isConnected = true;
			sendControlMessage(MPRINGS_CONNECTED_TO_TREE);
			trace() << "Connected to " << currentSinkID << " at level " << currentLevel;
			if (!TXBuffer.empty())
				processBufferedPacket();
		} else {
			sendControlMessage(MPRINGS_TREE_LEVEL_UPDATED);
			trace() << "Reconnected to " << currentSinkID << " at level " << currentLevel;
		}
		sendTopologySetupPacket();
	}

	tmpLevel = isSink ? 0 : NO_LEVEL;
	tmpSinkID = isSink ? self : NO_SINK;
}

void MultipathRingsRouting::processBufferedPacket()
{
	while (!TXBuffer.empty()) {
		toMacLayer(TXBuffer.front(), BROADCAST_MAC_ADDRESS);
		TXBuffer.pop();
	}
}

void MultipathRingsRouting::fromApplicationLayer(cPacket * pkt, const char *destination)
{
	string dst(destination);

	MultipathRingsRoutingPacket *netPacket =
	    new MultipathRingsRoutingPacket("Multipath rings routing data packet", NETWORK_LAYER_PACKET);
	netPacket->setMultipathRingsRoutingPacketKind(MPRINGS_DATA_PACKET);
	netPacket->setSource(SELF_NETWORK_ADDRESS);
	netPacket->setDestination(destination);
	netPacket->setSinkID(currentSinkID);
	netPacket->setSenderLevel(currentLevel);
	encapsulatePacket(netPacket, pkt);

	if (dst.compare(SINK_NETWORK_ADDRESS) == 0 || dst.compare(PARENT_NETWORK_ADDRESS) == 0) {
		netPacket->setSequenceNumber(currentSequenceNumber);
		currentSequenceNumber++;
		if (bufferPacket(netPacket)) {
			if (isConnected)
				processBufferedPacket();
			else
				sendControlMessage(MPRINGS_NOT_CONNECTED);
		} else {
			//Here we could send a control message to upper layer informing that our buffer is full
		}
	} else {		//++++ need to control flooding
		toMacLayer(netPacket, BROADCAST_MAC_ADDRESS);
	}
}

void MultipathRingsRouting::fromMacLayer(cPacket * pkt, int macAddress, double rssi, double lqi)
{
	MultipathRingsRoutingPacket *netPacket = dynamic_cast <MultipathRingsRoutingPacket*>(pkt);
	if (!netPacket)
		return;

	switch (netPacket->getMultipathRingsRoutingPacketKind()) {

		case MPRINGS_TOPOLOGY_SETUP_PACKET:{
			if (isSink)
				break;
			if (!isScheduledNetSetupTimeout) {
				isScheduledNetSetupTimeout = true;
				setTimer(TOPOLOGY_SETUP_TIMEOUT, netSetupTimeout);
				tmpLevel = NO_LEVEL;
				tmpSinkID = NO_SINK;
			}
			if (tmpLevel == NO_LEVEL || tmpLevel > netPacket->getSenderLevel()) {
				tmpLevel = netPacket->getSenderLevel();
				tmpSinkID = netPacket->getSinkID();
			}
			break;
		}

		case MPRINGS_DATA_PACKET:{
			string dst(netPacket->getDestination());
			string src(netPacket->getSource());
			int senderLevel = netPacket->getSenderLevel();
			int sinkID = netPacket->getSinkID();

			if (dst.compare(BROADCAST_NETWORK_ADDRESS) == 0 ||
					dst.compare(SELF_NETWORK_ADDRESS) == 0) {
				// We are not filtering packets that are sent to this node directly or to 
				// broadcast network address, making application layer responsible for them
				toApplicationLayer(pkt->decapsulate());

			} else if (dst.compare(SINK_NETWORK_ADDRESS) == 0) {
				if (senderLevel == currentLevel + 1) {
					if (self == sinkID) {
						// Packet is for this node, if filter passes, forward it to application
						if (isNotDuplicatePacket(pkt))
							toApplicationLayer(decapsulatePacket(pkt));
						else
							trace() << "Discarding duplicate packet from node " << src;
					} else if (sinkID == currentSinkID) {
						// We want to rebroadcast this packet since we are not its destination
						// For this, a copy of the packet is created and sender level field is 
						// updated before calling toMacLayer() function
						MultipathRingsRoutingPacket *dupPacket = netPacket->dup();
						dupPacket->setSenderLevel(currentLevel);
						toMacLayer(dupPacket, BROADCAST_MAC_ADDRESS);
					}
				}

			} else if (dst.compare(PARENT_NETWORK_ADDRESS) == 0) {
				if (senderLevel == currentLevel + 1 && sinkID == currentSinkID) {
					// Packet is for this node, if filter passes, forward it to application
					if (isNotDuplicatePacket(pkt))
						toApplicationLayer(decapsulatePacket(pkt));
					else
						trace() << "Discarding duplicate packet from node " << src;
				}
			}
			break;
		}
	}
}


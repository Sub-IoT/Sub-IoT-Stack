/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2010                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Athanassios Boulis, Dimosthenis Pediaditakis                 *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *  
 ****************************************************************************/

#include "SimpleAggregation.h"

Define_Module(SimpleAggregation);

void SimpleAggregation::startup()
{
	sampleInterval = (double)par("sampleInterval") / 1000;
	aggregatedValue = 0.0;
	waitingTimeForLowerLevelData = 0.0;
	lastSensedValue = 0.0;
	totalPackets = 0;
	setTimer(REQUEST_SAMPLE, 0);
}

void SimpleAggregation::timerFiredCallback(int index)
{
	switch (index) {
		
		case REQUEST_SAMPLE:{
			requestSensorReading();
			setTimer(REQUEST_SAMPLE, sampleInterval);
			break;
		}
		
		case SEND_AGGREGATED_VALUE:{
			toNetworkLayer(createGenericDataPacket(aggregatedValue, totalPackets), PARENT_NETWORK_ADDRESS);
			totalPackets++;
			break;
		}
	}
}

void SimpleAggregation::fromNetworkLayer(ApplicationPacket * rcvPacket, 
		const char *source, double rssi, double lqi)
{
	double theData = rcvPacket->getData();

	// do the aggregation bit. For this example aggregation function is a simple max.
	if (theData > aggregatedValue)
		aggregatedValue = theData;

	if (isSink)
		trace() << "from " << source << "received value " << theData;
}

void SimpleAggregation::handleSensorReading(SensorReadingMessage * rcvReading)
{
	string sensType(rcvReading->getSensorType());
	double sensValue = rcvReading->getSensedValue();
	lastSensedValue = sensValue;
}

void SimpleAggregation::handleNeworkControlMessage(cMessage * msg)
{
/*
    switch(msg->getKind()) {
    
	case TREE_LEVEL_UPDATED: {
	    // this message notifies the application of routing state (level)
	    // for certain routing protocols.
	    Network_ControlMessage *levelMsg = check_and_cast<Network_ControlMessage *>(msg);
	    routingLevel = levelMsg->getLevel();

	    waitingTimeForLowerLevelData = sampleInterval/pow((double)2,routingLevel);
	    trace() << "Routing level " << routingLevel;
	
	    setTimer(SEND_AGGREGATED_VALUE,waitingTimeForLowerLevelData);
	    break;
	}

	case CONNECTED_TO_TREE: {
	    Network_ControlMessage *connectedMsg = check_and_cast<Network_ControlMessage *>(msg);

	    int treeLevel = connectedMsg->getLevel();
	    string parents;
	    parents.assign(connectedMsg->getParents());

	    trace() << "Tree level " << treeLevel;

	    routingLevel = treeLevel;

	    waitingTimeForLowerLevelData = sampleInterval/pow((double)2,routingLevel);
	    setTimer(SEND_AGGREGATED_VALUE,waitingTimeForLowerLevelData);
	    break;
	}
    }
*/
}


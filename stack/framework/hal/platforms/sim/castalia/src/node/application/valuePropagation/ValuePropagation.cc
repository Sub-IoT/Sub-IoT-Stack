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

#include "ValuePropagation.h"

Define_Module(ValuePropagation);

void ValuePropagation::startup()
{
	tempThreshold = par("tempThreshold");
	totalPackets = 0;
	currMaxReceivedValue = -1.0;
	currMaxSensedValue = -1.0;
	sentOnce = 0;
	theValue = 0;
	setTimer(REQUEST_SAMPLE, 0);
}

void ValuePropagation::timerFiredCallback(int index)
{
	switch (index) {
		case REQUEST_SAMPLE:{
			requestSensorReading();
			break;
		}
	}
}

void ValuePropagation::fromNetworkLayer(ApplicationPacket * rcvPacket, const char *source, double rssi, double lqi)
{
	double receivedData = rcvPacket->getData();

	totalPackets++;
	if (receivedData > currMaxReceivedValue)
		currMaxReceivedValue = receivedData;

	if (receivedData > tempThreshold && !sentOnce) {
		theValue = receivedData;
		toNetworkLayer(createGenericDataPacket(receivedData, 1), BROADCAST_NETWORK_ADDRESS);
		sentOnce = 1;
		trace() << "Got the value: " << theValue;
	}
}

void ValuePropagation::handleSensorReading(SensorReadingMessage * rcvReading)
{
	double sensedValue = rcvReading->getSensedValue();

	if (sensedValue > currMaxSensedValue)
		currMaxSensedValue = sensedValue;

	if (sensedValue > tempThreshold && !sentOnce) {
		theValue = sensedValue;
		toNetworkLayer(createGenericDataPacket(sensedValue, 1), BROADCAST_NETWORK_ADDRESS);
		sentOnce = 1;
	}
}

void ValuePropagation::finishSpecific()
{
	declareOutput("got value");
	if (theValue > tempThreshold)
		collectOutput("got value", "yes/no", 1);
	else
		collectOutput("got value", "yes/no", 0);
	declareOutput("app packets received");
	collectOutput("app packets received", "", totalPackets);
}


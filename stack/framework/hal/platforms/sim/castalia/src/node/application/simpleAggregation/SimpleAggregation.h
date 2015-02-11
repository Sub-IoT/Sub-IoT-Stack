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

#ifndef _SIMPLEAGGREGATION_H_
#define _SIMPLEAGGREGATION_H_

#include "VirtualApplication.h"

using namespace std;

enum SimpleAggregationTimers {
	REQUEST_SAMPLE = 1,
	SEND_AGGREGATED_VALUE = 2
};

class SimpleAggregation: public VirtualApplication {
 private:
	double aggregatedValue;
	int routingLevel;
	double waitingTimeForLowerLevelData;
	double lastSensedValue;
	double sampleInterval;
	int totalPackets;

 protected:
	void startup();
	void timerFiredCallback(int);
	void handleSensorReading(SensorReadingMessage *);
	void handleNeworkControlMessage(cMessage *);
	void fromNetworkLayer(ApplicationPacket *, const char *, double, double);
};

#endif				// _SIMPLEAGGREGATION_APPLICATIONMODULE_H_

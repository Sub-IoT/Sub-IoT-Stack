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

#ifndef _VALUEPROPAGATION_H_
#define _VALUEPROPAGATION_H_

#include "VirtualApplication.h"

using namespace std;

enum ValuePropagationTimers {
	REQUEST_SAMPLE = 1,
};

class ValuePropagation: public VirtualApplication {
 private:
	int totalPackets;
	double currMaxReceivedValue;
	double currMaxSensedValue;
	int sentOnce;
	double theValue;
	double tempThreshold;
	vector<double> sensedValues;

 protected:
	void startup();
	void finishSpecific();
	void fromNetworkLayer(ApplicationPacket *, const char *, double, double);
	void handleSensorReading(SensorReadingMessage *);
	void timerFiredCallback(int);
};

#endif				// _VALUEPROPAGATION_APPLICATIONMODULE_H_

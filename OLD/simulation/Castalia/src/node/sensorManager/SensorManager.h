/*******************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2010                            *
 *  Developed at the ATP lab, Networked Systems research theme                 *
 *  Author(s): Athanassios Boulis, Dimosthenis Pediaditakis, Yuriy Tselishchev *
 *  This file is distributed under the terms in the attached LICENSE file.     *
 *  If you do not find this file, copies can be found by writing to:           *
 *                                                                             *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia                *
 *      Attention:  License Inquiry.                                           *
 *                                                                             *  
 *******************************************************************************/

#ifndef _APPLICATIONMODULESIMPLE_H_
#define _APPLICATIONMODULESIMPLE_H_

#include "SensorManagerMessage_m.h"
#include "PhysicalProcessMessage_m.h"

#include "VirtualMobilityManager.h"
#include "CastaliaModule.h"

using namespace std;

class SensorManager: public CastaliaModule {
 private:
	/*--- The .ned file's parameters ---*/
	bool printDebugInfo;
	vector<int> corrPhyProcess;
	vector<double> pwrConsumptionPerDevice;
	vector<simtime_t> minSamplingIntervals;
	vector<string> sensorTypes;
	vector<double> sensorBiasSigma;
	vector<double> sensorNoiseSigma;
	vector<double> sensorSensitivity;
	vector<double> sensorResolution;
	vector<double> sensorSaturation;

	/*--- Custom class member variables ---*/
	int self;		// the node's ID
	int totalSensors;
	vector<simtime_t> sensorlastSampleTime;
	vector<double> sensorLastValue;
	vector<double> sensorBias;
	VirtualMobilityManager *nodeMobilityModule;
	int disabled;

 protected:
	virtual void initialize();
	virtual void handleMessage(cMessage * msg);
	void parseStringParams(void);

 public:
	double getSensorDeviceBias(int index);

};

#endif				// _APPLICATIONMODULESIMPLE_H_

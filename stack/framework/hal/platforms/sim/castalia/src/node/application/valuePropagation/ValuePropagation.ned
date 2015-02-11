//***************************************************************************************
//*  Copyright: National ICT Australia, 2007 - 2010                                     *
//*  Developed at the ATP lab, Networked Systems research theme                         *
//*  Author(s): Athanassios Boulis, Dimosthenis Pediaditakis                            *
//*  This file is distributed under the terms in the attached LICENSE file.             *
//*  If you do not find this file, copies can be found by writing to:                   *
//*                                                                                     *
//*      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia                        *
//*      Attention:  License Inquiry.                                                   *
//*                                                                                     *
//***************************************************************************************

package node.application.valuePropagation;

// The sensor node module. Connects to the wireless channel in order to communicate 
// with other nodes. Connects to psysical processes so it can sample them.

simple ValuePropagation like node.application.iApplication {
 parameters:
 	string applicationID = default ("valuePropagation");
	bool collectTraceInfo = default (true);
	int priority = default (1);
	int packetHeaderOverhead = default (8);	// in bytes
	int constantDataPayload = default (2);	// in bytes
	double tempThreshold = default (15.0);

 gates:
 	output toCommunicationModule;
	output toSensorDeviceManager;
	input fromCommunicationModule;
	input fromSensorDeviceManager;
	input fromResourceManager;
}


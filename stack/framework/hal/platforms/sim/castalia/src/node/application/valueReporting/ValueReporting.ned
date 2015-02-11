//***************************************************************************************
//*  Copyright: National ICT Australia, 2007 - 2010                                     *
//*  Developed at the ATP lab, Networked Systems research theme                         *
//*  Author(s): Dimosthenis Pediaditakis, Yuriy Tselishchev                             *
//*  This file is distributed under the terms in the attached LICENSE file.             *
//*  If you do not find this file, copies can be found by writing to:                   *
//*                                                                                     *
//*      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia                        *
//*      Attention:  License Inquiry.                                                   *
//*                                                                                     *
//***************************************************************************************

package node.application.valueReporting;

// The sensor node module. Connects to the wireless channel in order to communicate 
// with other nodes. Connects to psysical processes so it can sample them.

simple ValueReporting like node.application.iApplication {
 parameters:
 	string applicationID = default ("valueReporting");
	bool collectTraceInfo = default (true);
	int priority = default (1);
	int packetHeaderOverhead = default (8);	// in bytes
	int constantDataPayload = default (12);	// in bytes
	int maxSampleInterval = default (60000);// in msec
	int minSampleInterval = default (1000);	// in msec
	bool isSink = default (false);

 gates:
 	output toCommunicationModule;
	output toSensorDeviceManager;
	input fromCommunicationModule;
	input fromSensorDeviceManager;
	input fromResourceManager;
}


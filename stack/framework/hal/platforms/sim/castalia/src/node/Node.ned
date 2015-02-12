//********************************************************************************
//*  Copyright: National ICT Australia,  2007 - 2010                             *
//*  Developed at the ATP lab, Networked Systems research theme                  *
//*  Author(s): Athanassios Boulis, Dimosthenis Pediaditakis, Yuriy Tselishchev  *
//*  This file is distributed under the terms in the attached LICENSE file.      *
//*  If you do not find this file, copies can be found by writing to:            *
//*                                                                              *
//*      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia                 *
//*      Attention:  License Inquiry.                                            *
//*                                                                              *
//*******************************************************************************/

package node;

import node.mobilityManager;

// The sensor node module. Connects to the wireless channel in order to communicate 
// with other nodes. Connects to psysical processes so it can sample them.

module Node {
 parameters:
 	//node location is defined by five parameters below
	double xCoor = default (0);
	double yCoor = default (0);
	double zCoor = default (0);
	double phi = default (0);
	double theta = default (0);
	
	double startupOffset = default (0);							//node startup offset (i.e. delay), in seconds 
	double startupRandomization = default (0.05);				//node startup randomisation, in seconds
	// Node will become active startupOffset + random(startupRandomization) 
	// seconds after the start of simulation

	string MobilityManagerName = default ("NoMobilityManager");	//the name of the implemented Mobility Module

 gates:
	output toWirelessChannel;
	output toPhysicalProcess[];
	input fromWirelessChannel;
	input fromPhysicalProcess[];

 submodules:
 	MobilityManager: <MobilityManagerName> like node.mobilityManager.iMobilityManager;
	ResourceManager: node.resourceManager.ResourceManager;
	SensorManager: node.sensorManager.SensorManager {
	 gates:
		fromNodeContainerModule[sizeof(toPhysicalProcess)];
		toNodeContainerModule[sizeof(toPhysicalProcess)];
	} 
	Communication: node.communication.CommunicationModule;
	Hal: node.hal.HalModule;
	
 connections:
	Communication.toWirelessChannel --> toWirelessChannel;
	fromWirelessChannel --> Communication.fromWirelessChannel;
		
	Hal.toCommunicationModule --> Communication.fromHalModule;
	Communication.toHalModule --> Hal.fromCommunicationModule;
	
	Hal.toSensorDeviceManager --> SensorManager.fromHalModule;
	SensorManager.toHalModule --> Hal.fromSensorDeviceManager;

	for i = 0..sizeof(toPhysicalProcess) - 1 {
		fromPhysicalProcess[i] --> SensorManager.fromNodeContainerModule[i];
		SensorManager.toNodeContainerModule[i] --> toPhysicalProcess[i];
	}

	ResourceManager.toSensorDevManager --> SensorManager.fromResourceManager;
	ResourceManager.toHal --> Hal.fromResourceManager;
	ResourceManager.toCommunicationModule --> Communication.fromResourceManager;
}


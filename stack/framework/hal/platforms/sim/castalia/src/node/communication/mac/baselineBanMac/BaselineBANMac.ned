//****************************************************************************
//*  Copyright: National ICT Australia,  2007 - 2010                         *
//*  Developed at the ATP lab, Networked Systems research theme              *
//*  Author(s): Athanassios Boulis, Yuriy Tselishchev                        *
//*  This file is distributed under the terms in the attached LICENSE file.  *
//*  If you do not find this file, copies can be found by writing to:        *
//*                                                                          *
//*      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
//*      Attention:  License Inquiry.                                        *
//*                                                                          *  
//****************************************************************************/

package node.communication.mac.baselineBanMac;

simple BaselineBANMac like node.communication.mac.iMac {
 parameters:
	bool collectTraceInfo = default(false);
	int macMaxPacketSize = default(1000);
	int macBufferSize = default(32);
	int macPacketOverhead = default(7);			//in bytes, 7 bytes is the BaselineBAN header size
	
	bool isHub = default(false);
	double allocationSlotLength = default(10);	//in msecs, defines how long is a slot

	int beaconPeriodLength = default(32);		//in slots, defined for the Hub
	int RAP1Length= default(8);					//in slots, defined for the Hub
	int scheduledAccessLength = default(0);		//in slots, asked by the sensor, default: no scheduled access
	int scheduledAccessPeriod = default(1);		//in beacon periods, asked by the sensor, default: 1-periodic

	int maxPacketTries = default(2);
	double contentionSlotLength = default(0.36);	//in msecs, mini-slots used in contention 
	bool enhanceGuardTime = default(false);
	bool enhanceMoreData = default(false);
	bool pollingEnabled = default(false);
	bool naivePollingScheme = default(false);

	// parameters related to the PHY layer. As the proposal suggests we define them independantly.
		
	double pTIFS = default(0.03);				//in msecs, time to start TXing a frame after you received one
	double pTimeSleepToTX = default(0.2);		//in msecs, time to start TXing after being sleeping, NOT included in spec.
	int phyLayerOverhead = default(6);			//in bytes
	double phyDataRate;				//in Kbps (1024 used to be the default, but now we have to define it to avoid mistakes) 
	double mClockAccuracy = default(0.0001);	//fraction, clock drift
 
 gates:
	output toNetworkModule;
	output toRadioModule;
	input fromNetworkModule;
	input fromRadioModule;
	input fromCommModuleResourceMgr;
}


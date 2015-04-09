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

package node.communication.radio;

simple Radio {
 parameters:
 	bool collectTraceInfo = default (false);

	string RadioParametersFile = default ("");	// variable points to the file that cointains most radio parameters

	string mode = default ("");	// we can choose a mode to begin with. Modes are defined in the
								// RadioParametersFile. Empty string means use the first mode defined

	string state = default ("RX");	// we can choose a radio state to begin with. RX and TX are always there.
									// according to the radio defined we can choose from a different set of sleep states

	string TxOutputPower = default ("");	// we can choose a Txpower to begin with. Possible tx power values 
											// are defined in the RadioParametersFile. Empty string means use 
											// the first tx power defined (which is also the highest)

	string sleepLevel = default ("");	// we can choose a sleep level which will be used when a transition to SLEEP state
										// is requested. Empty string means use first level defined (will usually be the fastest
										// and most energy consuming sleep state)

	double carrierFreq = default (2400.0);	// the carrier frequency (in MHz) to begin with.

	int collisionModel = default (2);	// 0-> No interference
										// 1-> Simple interference
										// 2-> Additive interefence
										// 3-> Advanced interference (not implemented)

	double CCAthreshold = default (-95.0);	// the threshold of the RSSI register (in dBm) 
											// were above it channel is NOT clear

	int symbolsForRSSI = default (8);
	bool carrierSenseInterruptEnabled = default (false);

	int bufferSize = default (16);	// in number of frames
	int maxPhyFrameSize = default (1024);	// in bytes
	int phyFrameOverhead = default (6);	// in bytes (802.15.4. = 6 bytes)

 gates:
	output toCommunicationModule;
	output toMacModule;
	input fromCommunicationModule;
	input fromMacModule;
	input fromCommModuleResourceMgr;
}


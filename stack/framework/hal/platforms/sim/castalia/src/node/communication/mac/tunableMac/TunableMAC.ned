//*******************************************************************************
//*  Copyright: National ICT Australia,  2007 - 2010                            *
//*  Developed at the ATP lab, Networked Systems research theme                 *
//*  Author(s): Athanassios Boulis, Dimosthenis Pediaditakis, Yuriy Tselishchev *
//*  This file is distributed under the terms in the attached LICENSE file.     *
//*  If you do not find this file, copies can be found by writing to:           *
//*                                                                             *
//*      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia                *
//*      Attention:  License Inquiry.                                           *
//*                                                                             *  
//******************************************************************************/

package node.communication.mac.tunableMac;

// ===========================================
// Default parameter values will result in 
// non-persistent CSMA-CA behaviour
// ============================================

simple TunableMAC like node.communication.mac.iMac {
 parameters: 
	bool collectTraceInfo = default (false);
	bool printStateTransitions = default (false);

	//=============== These are main parameters which can be tuned ================

	double dutyCycle = default (1.0);	// listening / (sleeping+listening)
	double listenInterval = default (10);	// how long do we leave the radio in listen mode, in ms
	double beaconIntervalFraction = default (1.0);	// fraction of the sleeping interval that we send beacons
	double probTx = default (1.0);		// the probability of a single try of Transmission to happen
	int numTx = default (1);		// when we have something to Tx, how many times we try
	double randomTxOffset = default (0.0);	// Tx after time chosen randomly from interval [0..randomTxOffset]
	double reTxInterval = default (0.0);	// Interval between retransmissions in ms, (numTx-1) retransmissions
	double backoffType = default (1);	// 0-->(backoff = sleepinterval), 
						// 1-->(backoff = constant value), 
						// 2-->(backoff = multiplying value - e.g. 1*a, 2*a, 3*a, 4*a ...), 
						// 3-->(backoff = exponential value - e.g. 2, 4, 8, 16, 32...)
	int backoffBaseValue = default (16);	// the backoff base value in ms
	double CSMApersistance = default (0); 	// value in [0..1], is CSMA non-persistent, p-persistent, or 1-persistent?
	bool txAllPacketsInFreeChannel = default (true); // if you find the channel free, tx all packets in buffer? 
	bool sleepDuringBackoff = default (false);	// for no dutyCycle case: sleep when backing off?

	//=============== End of tunable parameters ===================================

	int macMaxPacketSize = default (0);
	int macPacketOverhead = default (9);
	int beaconFrameSize = default (125);  // have a big beacon, to avoid much processing overhead, but fit at least 2 in the listening interval
	int macBufferSize = default (32);

	//========== Make sure you update the data rate if you change radios ==========
	double phyDataRate = default (250.0);
	double phyDelayForValidCS = default (0.128);
	int phyFrameOverhead = default (6);

 gates:
	output toNetworkModule;
	output toRadioModule;
	input fromNetworkModule;
	input fromRadioModule;
	input fromCommModuleResourceMgr;
}


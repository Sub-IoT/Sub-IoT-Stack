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

package node.communication.mac.tMac;

simple TMAC like node.communication.mac.iMac {
 parameters:
    //debug parameters
	bool collectTraceInfo = default (false);
	bool printStateTransitions = default (false);

	//mac layer packet sizes, these parameters are described in TMacFrame.msg file
	int ackPacketSize = default (11);
	int syncPacketSize = default (11);
	int rtsPacketSize = default (13);
	int ctsPacketSize = default (13);

	//mac layer parameters
	int macMaxPacketSize = default (0);		//no limit on frame size
	int macPacketOverhead = default (11);	//DATA frame overhead is described in TMacFrame.msg
	int macBufferSize = default (32);		//buffer of 32 packets by default

	//TMAC protocol parameters
	int maxTxRetries = default (2);
	bool allowSinkSync = default (true);	//This parameter allows sink node to start synchronisation immediately
	bool useFrts = default (false);			//enable/disable FRTS (Future Request To Send), true value not supported
	bool useRtsCts = default (true);		//This allows to enable/disable RTS/CTS handshake
	bool disableTAextension = default (false);	//disabling TA extension effectively creates an SMAC protocol
	bool conservativeTA = default (true);	//conservative activation timeout - will always stay awake for 
											//atleast 15 ms after any activity on the radio

	double resyncTime = default (6);		// timer for re-sending SYNC msg, in seconds
	double contentionPeriod = default (10);	// 10 ms
	double listenTimeout = default (15);	// 15 ms, is the timeout TA (Activation event)
	double waitTimeout = default (5);		// timeout for expecting a reply to DATA or RTS packet
	double frameTime = default (610);		// frame time (standard = 610ms)

	int collisionResolution = default (0);	// collision resolution mechanism, choose from 
											//      0 - immediate retry (low collision avoidance)
											//      1 - based on overhearing (default)
											//      2 - retry next frame (aggressive collision avoidance)

	//parameters dependent on physical layer
	double phyDelayForValidCS = default (0.128);
	double phyDataRate = default (250);
	int phyFrameOverhead = default (6);

 gates:
	output toNetworkModule;
	output toRadioModule;
	input fromNetworkModule;
	input fromRadioModule;
	input fromCommModuleResourceMgr;
}


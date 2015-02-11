/*******************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2011                            *
 *  Developed at the ATP lab, Networked Systems research theme                 *
 *  Author(s): Athanassios Boulis, Yuriy Tselishchev                           *
 *  This file is distributed under the terms in the attached LICENSE file.     *
 *  If you do not find this file, copies can be found by writing to:           *
 *                                                                             *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia                *
 *      Attention:  License Inquiry.                                           *
 *                                                                             *
 *******************************************************************************/

#ifndef _MULTIPATHRINGSROUTING_H_
#define _MULTIPATHRINGSROUTING_H_

#include <map>
#include "VirtualRouting.h"
#include "MultipathRingsRoutingPacket_m.h"
#include "MultipathRingsRoutingControl_m.h"

#define NO_LEVEL  -110
#define NO_SINK   -120

using namespace std;

enum MultipathRingsRoutingTimers {
	TOPOLOGY_SETUP_TIMEOUT = 1,
};

class MultipathRingsRouting: public VirtualRouting {
 private:
	int mpathRingsSetupFrameOverhead;	// in bytes
	double netSetupTimeout;

	// multipathRingsRouting-related member variables
	int currentSequenceNumber;
	int currentSinkID;
	int currentLevel;
	int tmpSinkID;
	int tmpLevel;
	bool isSink;		//is a .ned file parameter of the Application module
	bool isConnected;	//attached under a parent node
	bool isScheduledNetSetupTimeout;

 protected:
	void startup();
	void fromApplicationLayer(cPacket *, const char *);
	void fromMacLayer(cPacket *, int, double, double);

	void sendTopologySetupPacket();
	void sendControlMessage(multipathRingsRoutingControlDef);

	void timerFiredCallback(int);
	void processBufferedPacket();
};

#endif				//MULTIPATHRINGSROUTINGMODULE

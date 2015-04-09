/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2011                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Yuriy Tselishchev, Athanassios Boulis                        *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *
 ****************************************************************************/

#ifndef _VIRTUALROUTING_H_
#define _VIRTUALROUTING_H_

#include <queue>
#include <vector>
#include <omnetpp.h>

#include "CastaliaModule.h"
#include "TimerService.h"
#include "CastaliaMessages.h"
#include "Radio.h"
#include "ResourceManager.h"
#include "RoutingPacket_m.h"
#include "ApplicationPacket_m.h"

#define SELF_NETWORK_ADDRESS selfAddress.c_str()
#define ROUTE_DEST_DELIMITER "#"
#define PACKET_HISTORY_SIZE 5

using namespace std;

class VirtualRouting: public CastaliaModule, public TimerService {
 protected:
	/*--- The .ned file's parameters ---*/
	int maxNetFrameSize;		//in bytes
	int netDataFrameOverhead;	//in bytes
	int netBufferSize;			//in # of messages
	unsigned int currentSequenceNumber;

	/*--- Custom class parameters ---*/
	double radioDataRate;
	ResourceManager *resMgrModule;

	queue< cPacket* > TXBuffer;
	vector< list< unsigned int> > pktHistory;

	double cpuClockDrift;
	bool disabled;

	Radio *radioModule;
	string selfAddress;
	int self;

	virtual void initialize();
	virtual void startup() { }
	virtual void handleMessage(cMessage * msg);
	virtual void finish();

	virtual void fromApplicationLayer(cPacket *, const char *) = 0;
	virtual void fromMacLayer(cPacket *, int, double, double) = 0;

	int bufferPacket(cPacket *);

	void toApplicationLayer(cMessage *);
	void toMacLayer(cMessage *);
	void toMacLayer(cPacket *, int);
	bool isNotDuplicatePacket(cPacket *);

	void encapsulatePacket(cPacket *, cPacket *);
	cPacket *decapsulatePacket(cPacket *);
	int resolveNetworkAddress(const char *);

	virtual void handleMacControlMessage(cMessage *);
	virtual void handleRadioControlMessage(cMessage *);
	virtual void handleNetworkControlCommand(cMessage *) { }
};

#endif				//_VIRTUALROUTING_H_

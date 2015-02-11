/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2010                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Athanassios Boulis, Yuriy Tselishchev                        *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *  
 ****************************************************************************/

#ifndef _TMAC_H_
#define _TMAC_H_

#include "VirtualMac.h"
#include "TMacPacket_m.h"

#define TX_TIME(x)		(phyLayerOverhead + x)*1/(1000*phyDataRate/8.0)	//x are in BYTES

using namespace std;

enum MacStates {
	MAC_STATE_SETUP = 100,
	MAC_STATE_SLEEP = 101,
	MAC_STATE_ACTIVE = 102,
	MAC_STATE_ACTIVE_SILENT = 103,
	MAC_STATE_IN_TX = 104,

	MAC_CARRIER_SENSE_FOR_TX_RTS = 110,	//These states are used to distinguish whether we are trying to TX
	MAC_CARRIER_SENSE_FOR_TX_DATA = 111,	// Data OR Cts OR Rts OR Ack
	MAC_CARRIER_SENSE_FOR_TX_SYNC = 114,
	MAC_CARRIER_SENSE_BEFORE_SLEEP = 115,

	MAC_STATE_WAIT_FOR_DATA = 120,
	MAC_STATE_WAIT_FOR_CTS = 121,
	MAC_STATE_WAIT_FOR_ACK = 122,
};

enum TMacTimers {
	SYNC_SETUP = 1,
	SYNC_CREATE = 2,
	SYNC_RENEW = 3,
	FRAME_START = 4,
	CHECK_TA = 5,
	CARRIER_SENSE = 6,
	TRANSMISSION_TIMEOUT = 7,
	WAKEUP_SILENT = 8,	// this timer has to be last as TMAC will 
						// schedule multiple timers for silent wakeup 
						// if more than one secondary schedule is present
};

struct TMacSchedule {
	simtime_t offset;
	int ID;
	int SN;
};

class TMAC: public VirtualMac {
 private:
	/*--- A map from int value of state to its description (used in debug) ---*/
	map<int,string> stateDescr;

	/*--- The .ned file's parameters ---*/
	bool printDebugInfo;
	bool printStateTransitions;

	int maxTxRetries;

	int ackPacketSize;	//in bytes
	int syncPacketSize;	//in bytes
	int rtsPacketSize;	//in bytes
	int ctsPacketSize;	//in bytes

	bool allowSinkSync;
	int resyncTime;
	simtime_t contentionPeriod;
	simtime_t listenTimeout;
	simtime_t waitTimeout;
	bool useRtsCts;
	bool useFRTS;
	simtime_t frameTime;
	bool disableTAextension;
	bool conservativeTA;
	int collisionResolution;

	/*--- General MAC variable ---*/
	bool isSink;
	int phyLayerOverhead;
	simtime_t phyDelayForValidCS;	// delay for valid CS
	double phyDataRate;

	/*--- TMAC state variables  ---*/
	int macState;
	int txAddr;			//current communication peer (can be BROADCAST)
	int txRetries;		//number of transmission attempts to txAddr (when reaches 0 - packet is dropped)
	int txSequenceNum;	//sequence number for current transmission
	bool primaryWakeup;	//used to distinguish between primary and secondary schedules
	bool needResync;	//set to 1 when a SYNC packet has to be sent
	simtime_t currentFrameStart;	//recorded start time of the current frame

	/*--- TMAC activation timeout variable ---*/
	simtime_t activationTimeout;	//time untill MAC_CHECK_TA message arrival

	/*--- TMAC packet pointers (sometimes packet is created not immediately before sending) ---*/
	TMacPacket *syncPacket;
	TMacPacket *rtsPacket;
	TMacPacket *ctsPacket;
	TMacPacket *ackPacket;

	/*--- TMAC Schedule table (list of effective schedules) ---*/
	vector<TMacSchedule> scheduleTable;

 protected:
	void startup();
	void timerFiredCallback(int);
	void fromNetworkLayer(cPacket *, int);
	void fromRadioLayer(cPacket *, double, double);
	int handleRadioControlMessage(cMessage *);

	void resetDefaultState(const char *descr = NULL);
	void setMacState(int newState, const char *descr = NULL);
	void createPrimarySchedule();
	void scheduleSyncPacket(simtime_t when);
	void sendDataPacket();
	void processMacPacket(TMacPacket *);
	void carrierIsClear();
	void carrierIsBusy();
	void updateScheduleTable(simtime_t wakeup, int ID, int SN);
	void performCarrierSense(int newState, simtime_t delay = 0);
	void extendActivePeriod(simtime_t extra = 0);
	void checkTxBuffer();
	void popTxBuffer();
};

#endif				//TMACMODULE

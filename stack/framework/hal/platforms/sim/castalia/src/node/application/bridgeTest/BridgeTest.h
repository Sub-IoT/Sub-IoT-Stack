/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2011                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Yuriy Tselishchev                                            *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *
 ****************************************************************************/

#ifndef _BridgeTest_H_
#define _BridgeTest_H_

#include "VirtualApplication.h"

#define REPROGRAM_PACKET_NAME "Bridge test reprogram packet"
#define REPORT_PACKET_NAME "Bridge test report packet"

using namespace std;

struct version_info {
	double version;
	int seq;
	vector<int> parts;
};

struct report_info {
	int source;
	int seq;
	vector<int> parts;
};

enum BridgeTestTimers {
	REQUEST_SAMPLE = 1,
	REPROGRAM_NODES = 2,
	SEND_REPROGRAM_PACKET = 3,
};

class BridgeTest:public VirtualApplication {
 private:
	int reportTreshold;
	double sampleInterval;
	simtime_t reprogramInterval;
	simtime_t reprogramPacketDelay;
	int reprogramPayload;
	int sampleSize;
	int maxPayload;

	simtime_t outOfEnergy;

	int currentVersion;
	int currentVersionPacket;
	int currSampleSN;
	int currentSampleAccumulated;
	int maxSampleAccumulated;
	int totalVersionPackets;
	int routingLevel;
	vector<version_info> version_info_table;
	vector<report_info> report_info_table;

	string reportDestination;

 protected:
	virtual void startup();
	void finishSpecific();
	void send2NetworkDataPacket(const char *destID, const char *pcktID, int data, int pckSeqNumber, int size);
	int updateVersionTable(double version, int seq);
	int updateReportTable(int src, int seq);
	void fromNetworkLayer(ApplicationPacket *, const char *, double, double);
	void timerFiredCallback(int);
	void handleSensorReading(SensorReadingMessage *);
};

#endif				// _BridgeTest_H_

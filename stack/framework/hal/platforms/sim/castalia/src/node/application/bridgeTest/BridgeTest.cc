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

#include "BridgeTest.h"

Define_Module(BridgeTest);

void BridgeTest::startup()
{
	reportTreshold = par("reportTreshold");
	sampleInterval = (double)par("sampleInterval") / 1000;
	reportDestination = par("reportDestination").stringValue();
	reprogramInterval = par("reprogramInterval");
	reprogramPacketDelay = (double)par("reprogramPacketDelay") / 1000;
	reprogramPayload = par("reprogramPayload");
	sampleSize = par("sampleSize");

	currentVersionPacket = 0;
	currentVersion = 0;
	currSampleSN = 1;
	outOfEnergy = 0;
	currentSampleAccumulated = 0;

	maxPayload = par("maxPayloadPacketSize");
	div_t tmp_div = div(reprogramPayload, maxPayload);
	totalVersionPackets = tmp_div.rem > 0 ? tmp_div.quot + 1 : tmp_div.quot;
	tmp_div = div(maxPayload, sampleSize);
	maxSampleAccumulated = tmp_div.quot * sampleSize;

	version_info_table.clear();
	report_info_table.clear();

	if (isSink) {
		setTimer(REPROGRAM_NODES, 1);	//add initial delay parameter
	} else {
		setTimer(REQUEST_SAMPLE, sampleInterval);
	}
}

void BridgeTest::timerFiredCallback(int timer)
{
	switch (timer) {

		case REQUEST_SAMPLE:{
			setTimer(REQUEST_SAMPLE, sampleInterval);
			requestSensorReading();
			break;
		}

		case REPROGRAM_NODES:{
			currentVersion++;
			currentVersionPacket = 1;
			setTimer(REPROGRAM_NODES, reprogramInterval);
			setTimer(SEND_REPROGRAM_PACKET, 0);
			break;
		}

		case SEND_REPROGRAM_PACKET:{
			ApplicationPacket *newPkt =
			    createGenericDataPacket(currentVersion, currentVersionPacket,  maxPayload);
			newPkt->setName(REPROGRAM_PACKET_NAME);
			trace() << "Sending reprogram packet, version " <<
			    currentVersion << ", sequence " << currentVersionPacket;
			toNetworkLayer(newPkt, BROADCAST_NETWORK_ADDRESS);
			currentVersionPacket++;
			if (currentVersionPacket < totalVersionPackets)
				setTimer(SEND_REPROGRAM_PACKET, reprogramPacketDelay);
			break;
		}
	}
}

void BridgeTest::fromNetworkLayer(ApplicationPacket * rcvPacket,
			const char *source, double rssi, double lqi)
{
	string packetName(rcvPacket->getName());

	double data = rcvPacket->getData();
	int sequenceNumber = rcvPacket->getSequenceNumber();

	if (packetName.compare(REPORT_PACKET_NAME) == 0) {
		// this is report packet which contains sensor reading information
		// NOTE that data field is used to store source address instead of using char *source
		// this is done because some routing and flooding is done on the application layer
		// and source address will not always correctly represent the author of the sensed data
		trace() << "Received report from " << (int)data;
		if (updateReportTable((int)data, sequenceNumber)) {
			// forward the packet only if we broadcast reports and this is a new (unseen) report
			// updateReportTable returns 0 for duplicate packets
			if (!isSink) {
				trace() << "Forwarding report packet from node " << (int)data;
				toNetworkLayer(rcvPacket->dup(), reportDestination.c_str());
			}
		}

	} else if (packetName.compare(REPROGRAM_PACKET_NAME) == 0) {
		// this is version (reprogramming) packet
		if (!isSink && updateVersionTable(data, sequenceNumber)) {
			// forward the packet only if not sink and its a new packet
			// updateVersionTable returns 0 for duplicate packets
			toNetworkLayer(rcvPacket->dup(), BROADCAST_NETWORK_ADDRESS);
		}

	} else {
		trace() << "unknown packet received: [" << packetName << "]";
	}
}

void BridgeTest::handleSensorReading(SensorReadingMessage * sensorMsg)
{
	string sensType(sensorMsg->getSensorType());
	double sensValue = sensorMsg->getSensedValue();

	if (isSink) {
		trace() << "Sink recieved SENSOR_READING (while it shouldnt) "
		    << sensValue << " (int)" << (int)sensValue;
		return;
	}

	if (sensValue < reportTreshold) {
		trace() << "Sensed value " << sensValue << " is less than the treshold ("
			<< reportTreshold << "), discarding";
		return;
	}

	currentSampleAccumulated += sampleSize;
	if (currentSampleAccumulated < maxSampleAccumulated) {
		trace() << "Accumulated " << currentSampleAccumulated << "/" << maxSampleAccumulated << " bytes of samples";
		return;
	}

	trace() << "Sending report packet, sequence number " << currSampleSN;
	ApplicationPacket *newPkt = createGenericDataPacket((double)self, currSampleSN, currentSampleAccumulated);
	newPkt->setName(REPORT_PACKET_NAME);
	toNetworkLayer(newPkt, reportDestination.c_str());
	currentSampleAccumulated = 0;
	currSampleSN++;
}

void BridgeTest::finishSpecific()
{
	if (isSink) {
		declareOutput("Report reception");
		for (int i = 0; i < (int)report_info_table.size(); i++) {
			collectOutput("Report reception", report_info_table[i].source,
					"Success", report_info_table[i].parts.size());
			collectOutput("Report reception", report_info_table[i].source,
					"Fail", report_info_table[i].seq - report_info_table[i].parts.size());
		}
	} else {
		declareOutput("Reprogram reception");
		for (int i = 0; i < (int)version_info_table.size(); i++) {
			collectOutput("Reprogram reception", "Success",
				      version_info_table[i].parts.size());
			collectOutput("Reprogram reception", "Fail",
				      version_info_table[i].seq - version_info_table[i].parts.size());
		}
	}
}

int BridgeTest::updateReportTable(int src, int seq)
{
	int pos = -1;
	for (int i = 0; i < (int)report_info_table.size(); i++) {
		if (report_info_table[i].source == src)
			pos = i;
	}

	if (pos == -1) {
		report_info newInfo;
		newInfo.source = src;
		newInfo.parts.clear();
		newInfo.parts.push_back(seq);
		newInfo.seq = seq;
		report_info_table.push_back(newInfo);
	} else {
		for (int i = 0; i < (int)report_info_table[pos].parts.size(); i++) {
			if (report_info_table[pos].parts[i] == seq)
				return 0;
		}
		report_info_table[pos].parts.push_back(seq);
		if (seq > report_info_table[pos].seq) {
			report_info_table[pos].seq = seq;
		}
	}
	return 1;
}

int BridgeTest::updateVersionTable(double version, int seq)
{
	int pos = -1;
	for (int i = 0; i < (int)version_info_table.size(); i++) {
		if (version_info_table[i].version == version)
			pos = i;
	}

	if (pos == -1) {
		version_info newInfo;
		newInfo.version = version;
		newInfo.parts.clear();
		newInfo.parts.push_back(seq);
		newInfo.seq = seq;
		version_info_table.push_back(newInfo);
	} else {
		for (int i = 0; i < (int)version_info_table[pos].parts.size(); i++) {
			if (version_info_table[pos].parts[i] == seq)
				return 0;
		}
		version_info_table[pos].parts.push_back(seq);
		if (seq > version_info_table[pos].seq) {
			version_info_table[pos].seq = seq;
		}

	}
	return 1;
}


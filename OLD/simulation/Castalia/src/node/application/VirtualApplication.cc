/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2011                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Athanassios Boulis, Yuriy Tselishchev                        *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *
 ****************************************************************************/

#include "VirtualApplication.h"

void VirtualApplication::initialize()
{
	/* Get a valid references to the objects of the Resources Manager module
	 * the Mobility module and the Radio module, so that we can make direct
	 * calls to their public methods
	 */
	cModule *parent = getParentModule();
	resMgrModule = check_and_cast <ResourceManager*>(parent->getSubmodule("ResourceManager"));
	mobilityModule = check_and_cast <VirtualMobilityManager*>(parent->getSubmodule("MobilityManager"));
	radioModule = check_and_cast <Radio*>(parent->getSubmodule("Communication")->getSubmodule("Radio"));
	// check that all the pointers are valid
	if (!resMgrModule || !mobilityModule || !radioModule)
		opp_error("\n Virtual App init: Error in geting a valid reference module(s).");

	self = parent->getIndex();
	// create the routing level address using self
	stringstream out; out << self; selfAddress = out.str();

	cpuClockDrift = resMgrModule->getCPUClockDrift();
	setTimerDrift(cpuClockDrift);
	disabled = true;

	applicationID = par("applicationID").stringValue(); // make sure par() returns a string
	priority = par("priority");
	packetHeaderOverhead = par("packetHeaderOverhead");
	constantDataPayload = par("constantDataPayload");
	isSink = hasPar("isSink") ? par("isSink") : false;

	double startup_delay = parent->par("startupOffset");
	// Randomize the delay if the startupRandomization is non-zero
	startup_delay += genk_dblrand(0) * (double)parent->par("startupRandomization");

	/* Send the STARTUP message to 1)Sensor_Manager, 2)Commmunication module,
	 * 3) Resource Manager, and $)APP (self message) so that the node starts
	 * operation. Note that we send the message to the Resource Mgr through
	 * the unconnected gate "powerConsumption" using sendDirect()
	 */
	sendDelayed(new cMessage("Sensor Dev Mgr [STARTUP]", NODE_STARTUP),
		    simTime() +  startup_delay, "toSensorDeviceManager");
	sendDelayed(new cMessage("Communication [STARTUP]", NODE_STARTUP),
		    simTime() +  startup_delay, "toCommunicationModule");
	sendDirect(new cMessage("Resource Mgr [STARTUP]", NODE_STARTUP),
		    startup_delay, 0, resMgrModule, "powerConsumption");
	scheduleAt(simTime() + startup_delay, new cMessage("App [STARTUP]", NODE_STARTUP));

	/* Latency measurement is optional. An application can define the
	 * following two parameters. If they are not defined then the
	 * declareHistogram and collectHistogram statement are not called.
	 */
	latencyMax = hasPar("latencyHistogramMax") ? par("latencyHistogramMax") : 0;
	latencyBuckets = hasPar("latencyHistogramBuckets") ? par("latencyHistogramBuckets") : 0;
	if (latencyMax > 0 && latencyBuckets > 0)
		declareHistogram("Application level latency, in ms", 0, latencyMax, latencyBuckets);
}

void VirtualApplication::handleMessage(cMessage * msg)
{
	int msgKind = msg->getKind();

	if (disabled && msgKind != NODE_STARTUP)
	{
		delete msg;
		return;
	}

	switch (msgKind) {

		case NODE_STARTUP:
		{
			disabled = false;
			startup();
			break;
		}

		case APPLICATION_PACKET:
		{
			ApplicationPacket *rcvPacket = check_and_cast <ApplicationPacket*>(msg);
			AppNetInfoExchange_type info = rcvPacket->getAppNetInfoExchange();
			// If the packet has the correct appID OR the appID is the empty string,
			// the packet is delivered by calling the app-specific function fromNetworkLayer()
			if (applicationID.compare(rcvPacket->getApplicationID()) == 0 || applicationID.compare("") == 0) {
				fromNetworkLayer(rcvPacket, info.source.c_str(), info.RSSI, info.LQI);
				if (latencyMax > 0 && latencyBuckets > 0)
					collectHistogram("Application level latency, in ms", 1000 * SIMTIME_DBL(simTime() - info.timestamp));
			}
			break;
		}

		case TIMER_SERVICE:
		{
			handleTimerMessage(msg);
			break;
		}

		case SENSOR_READING_MESSAGE:
		{
			SensorReadingMessage *sensMsg = check_and_cast <SensorReadingMessage*>(msg);
			handleSensorReading(sensMsg);
			break;
		}

		case NETWORK_CONTROL_MESSAGE:
		{
			handleNetworkControlMessage(msg);
			break;
		}

		case MAC_CONTROL_MESSAGE:
		{
			handleMacControlMessage(msg);
			break;
		}

		case RADIO_CONTROL_MESSAGE:
		{
			RadioControlMessage *radioMsg = check_and_cast <RadioControlMessage*>(msg);
			handleRadioControlMessage(radioMsg);
			break;
		}

		case OUT_OF_ENERGY:
		{
			disabled = true;
			break;
		}

		case DESTROY_NODE:
		{
			disabled = true;
			break;
		}

		default:
		{
			opp_error("Application module received unexpected message");
		}
	}

	delete msg;
}

void VirtualApplication::finish()
{
	CastaliaModule::finish();
	DebugInfoWriter::closeStream();
}

ApplicationPacket* VirtualApplication::createGenericDataPacket(double data, unsigned int seqNum, int size)
{
	ApplicationPacket *newPacket = new ApplicationPacket("App generic packet", APPLICATION_PACKET);
	newPacket->setData(data);
	newPacket->setSequenceNumber(seqNum);
	if (size > 0) newPacket->setByteLength(size);
	return newPacket;
}

void VirtualApplication::requestSensorReading(int index)
{
	// send the request message to the Sensor Device Manager
	SensorReadingMessage *reqMsg =
		new SensorReadingMessage("App to Sensor Mgr: sample request", SENSOR_READING_MESSAGE);

	// We need the index of the vector in the sensorTypes vector
	// to distinguish the self messages for each sensor
	reqMsg->setSensorIndex(index);

	send(reqMsg, "toSensorDeviceManager");
}

// A function used to send control messages
void VirtualApplication::toNetworkLayer(cMessage * msg)
{
	if (msg->getKind() == APPLICATION_PACKET)
		opp_error("toNetworkLayer() function used incorrectly to send APPLICATION_PACKET without destination Network address");
	send(msg, "toCommunicationModule");
}

// A function used to send data packets
void VirtualApplication::toNetworkLayer(cPacket * pkt, const char *dst)
{
	ApplicationPacket *appPkt = check_and_cast <ApplicationPacket*>(pkt);
	appPkt->getAppNetInfoExchange().destination = string(dst);
	appPkt->getAppNetInfoExchange().source = selfAddress;
	appPkt->getAppNetInfoExchange().timestamp = simTime();
	appPkt->setApplicationID(applicationID.c_str());
	int size = appPkt->getByteLength();
	if (size == 0)
		size = constantDataPayload;
	if (packetHeaderOverhead > 0) size += packetHeaderOverhead;
	trace() << "Sending [" << appPkt->getName() << "] of size " <<
		size << " bytes to communication layer";
	appPkt->setByteLength(size);
	send(appPkt, "toCommunicationModule");
}


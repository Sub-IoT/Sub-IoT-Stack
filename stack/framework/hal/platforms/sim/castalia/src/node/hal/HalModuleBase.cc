/*
 * HalModuleBase.cc
 *
 *  Created on: 12 Feb 2015
 *      Author: guust
 */

#include "HalModuleBase.h"



HalModuleBase::HalModuleBase():self(-1), resMgrModule(0x0), disabled(true), cpuClockDrift(0.0)
{}
HalModuleBase::~HalModuleBase(){}

void HalModuleBase::initialize()
{
	/* Get a valid references to the objects of the Resources Manager module
	 * the Mobility module and the Radio module, so that we can make direct
	 * calls to their public methods
	 */
	cModule *parent = getParentModule();
	resMgrModule = check_and_cast <ResourceManager*>(parent->getSubmodule("ResourceManager"));
//	mobilityModule = check_and_cast <VirtualMobilityManager*>(parent->getSubmodule("MobilityManager"));
//	radioModule = check_and_cast <Radio*>(parent->getSubmodule("Communication")->getSubmodule("Radio"));
	// check that all the pointers are valid
	//if (!resMgrModule || !mobilityModule || !radioModule)
	if (!resMgrModule)
		opp_error("\n Virtual App init: Error in geting a valid reference module(s).");

	self = parent->getIndex();
	// create the routing level address using self

	cpuClockDrift = resMgrModule->getCPUClockDrift();
	setTimerDrift(cpuClockDrift);
	disabled = true;

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

}

void HalModuleBase::handleMessage(cMessage * msg)
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

		case RADIO_PACKET:
		{
			RadioPacket *rcvPacket = check_and_cast <RadioPacket*>(msg);
			fromRadio(rcvPacket);
			break;
		}
		case RADIO_CONTROL_MESSAGE:
		{
			RadioControlMessage* cntrl= check_and_cast <RadioControlMessage*>(msg);
			handleRadioControlMessage(cntrl);
			break;
		}
		case TIMER_SERVICE:
		{
			handleTimerMessage(msg);
			break;
		}
		case SENSOR_READING_MESSAGE:
		{
			//ignore sensor readings for now
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

void HalModuleBase::toRadio(cMessage* msg)
{
	send(msg, "toCommunicationModule");
}

void HalModuleBase::finish()
{
	CastaliaModule::finish();
	DebugInfoWriter::closeStream();
}

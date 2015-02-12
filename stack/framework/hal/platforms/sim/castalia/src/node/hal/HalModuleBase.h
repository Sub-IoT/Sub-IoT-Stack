/*
 * HalModuleBase.h
 *
 *  Created on: 12 Feb 2015
 *      Author: guust
 */

#ifndef _HALMODULEBASE_H_
#define _HALMODULEBASE_H_

#include <omnetpp.h>
#include "RadioPacket_m.h"
#include "RadioControlMessage_m.h"
#include "ResourceManager.h"
#include "TimerService.h"
#include "CastaliaModule.h"


class HalModuleBase:  public CastaliaModule, public TimerService
{
protected:

		int self;								// the node's ID

		ResourceManager *resMgrModule;			// a pointer to the Resource Manager module
//		VirtualMobilityManager *mobilityModule;	// a pointer to the mobility Manager module
//		Radio *radioModule;						// a pointer to the Radio module
		bool disabled;
		double cpuClockDrift;

		virtual void initialize();
		virtual void handleMessage(cMessage * msg);
		virtual void finish();
		virtual void toRadio(cMessage* msg);


		virtual void startup(){};
		virtual void finishSpecific() {}
		virtual void fromRadio(RadioPacket* packet) {}
		virtual void handleRadioControlMessage(RadioControlMessage* msg){}

	public:
		HalModuleBase();
		virtual ~HalModuleBase();
};

#endif

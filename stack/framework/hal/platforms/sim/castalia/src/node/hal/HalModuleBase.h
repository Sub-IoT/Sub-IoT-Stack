/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
#include "CRadio.h"
#include "hwradio.h"

class HalModuleBase:  public CastaliaModule, public TimerService
{
protected:
		static HalModuleBase*	activeModule;
		int self;								// the node's ID

		ResourceManager *resMgrModule;			// a pointer to the Resource Manager module
//		VirtualMobilityManager *mobilityModule;	// a pointer to the mobility Manager module
		Radio *radioModule;						// a pointer to the Radio module
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

/*
 * HalModule.h
 *
 *  Created on: 12 Feb 2015
 *      Author: guust
 */

#ifndef _HAL_HALMODULE_H_
#define _HAL_HALMODULE_H_

#include "HalModuleBase.h"

class HalModule: public HalModuleBase
{
	protected:

		virtual void startup();
		virtual void finishSpecific();
		virtual void fromRadio(RadioPacket* packet);
		virtual void handleRadioControlMessage(RadioControlMessage* msg);
		virtual void timerFiredCallback(int index);
	public:
		HalModule();
		virtual ~HalModule();
};

#endif /* _HAL_HALMODULE_H_ */


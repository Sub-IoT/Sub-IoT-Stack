/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2010                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Yuriy Tselishchev                                            *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *  
 ****************************************************************************/

#ifndef _NOMOBILITYMANAGER_H_
#define _NOMOBILITYMANAGER_H_

#include <omnetpp.h>
#include "DebugInfoWriter.h"
#include "VirtualMobilityManager.h"

using namespace std;

class NoMobilityManager: public VirtualMobilityManager {
 protected:
	void handleMessage(cMessage * msg);
};

#endif

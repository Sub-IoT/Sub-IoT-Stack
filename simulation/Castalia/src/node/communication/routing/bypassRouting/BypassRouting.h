/*******************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2011                            *
 *  Developed at the ATP lab, Networked Systems research theme                 *
 *  Author(s): Athanassios Boulis, Yuriy Tselishchev                           *
 *  This file is distributed under the terms in the attached LICENSE file.     *
 *  If you do not find this file, copies can be found by writing to:           *
 *                                                                             *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia                *
 *      Attention:  License Inquiry.                                           *
 *                                                                             *
 *******************************************************************************/

#ifndef _BYPASSROUTING_H_
#define _BYPASSROUTING_H_

#include "VirtualRouting.h"
#include "BypassRoutingPacket_m.h"

using namespace std;

class BypassRouting: public VirtualRouting {
 protected:
	void fromApplicationLayer(cPacket *, const char *);
	void fromMacLayer(cPacket *, int, double, double);
};

#endif				//BYPASSROUTINGMODULE

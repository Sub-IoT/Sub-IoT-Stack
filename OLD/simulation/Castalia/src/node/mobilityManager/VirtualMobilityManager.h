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

#ifndef _VIRTUALBILITYMANAGER_H_
#define _VIRTUALBILITYMANAGER_H_

#include "WirelessChannelMessages_m.h"
#include "CastaliaModule.h"

using namespace std;

struct NodeLocation_type {
	double x;
	double y;
	double z;
	double phi;		// orientation info provided by 2 angles.
	double theta;
	int cell;		// store the cell ID that corresponds to coordinates xyz so we do not have to recompute it
};

class VirtualMobilityManager: public CastaliaModule {
 protected:
	NodeLocation_type nodeLocation;
	int index;
	
	cModule *node, *wchannel, *network;
	WirelessChannelNodeMoveMessage *positionUpdateMsg;

	virtual void initialize();
	virtual void notifyWirelessChannel();
	virtual void setLocation(double x, double y, double z, double phi = 0, double theta = 0);
	virtual void setLocation(NodeLocation_type);
	void parseDeployment();

 public:
	 virtual NodeLocation_type getLocation();
};

#endif

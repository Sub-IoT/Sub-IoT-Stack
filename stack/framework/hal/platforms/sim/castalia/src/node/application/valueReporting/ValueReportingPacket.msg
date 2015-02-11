//***************************************************************************************
//*  Copyright: National ICT Australia, 2007 - 2011                                     *
//*  Developed at the ATP lab, Networked Systems research theme                         *
//*  Author(s): Dimosthenis Pediaditakis, Yuriy Tselishchev                             *
//*  This file is distributed under the terms in the attached LICENSE file.             *
//*  If you do not find this file, copies can be found by writing to:                   *
//*                                                                                     *
//*      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia                        *
//*      Attention:  License Inquiry.                                                   *
//*                                                                                     *
//***************************************************************************************

cplusplus {{
#include "ApplicationPacket_m.h"
}}

class ApplicationPacket;

struct ValueReportData {
	unsigned short nodeID;	//the ID of the Node
	double locX;			// x-coordinate of the node
	double locY;			// y-coordinate of the node
} 

packet ValueReportingDataPacket extends ApplicationPacket {
	ValueReportData extraData;
}


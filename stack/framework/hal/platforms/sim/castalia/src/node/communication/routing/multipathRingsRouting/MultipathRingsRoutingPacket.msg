//****************************************************************************
//*  Copyright: National ICT Australia,  2007 - 2011                         *
//*  Developed at the ATP lab, Networked Systems research theme              *
//*  Author(s): Athanassios Boulis, Yuriy Tselishchev                        *
//*  This file is distributed under the terms in the attached LICENSE file.  *
//*  If you do not find this file, copies can be found by writing to:        *
//*                                                                          *
//*      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
//*      Attention:  License Inquiry.                                        *
//*                                                                          *  
//****************************************************************************/

cplusplus {{
#include "RoutingPacket_m.h"
}}

class RoutingPacket;

enum multipathRingsRoutingPacketDef {
	MPRINGS_DATA_PACKET = 1;
	MPRINGS_TOPOLOGY_SETUP_PACKET = 2;
} 

packet MultipathRingsRoutingPacket extends RoutingPacket {
	int multipathRingsRoutingPacketKind enum (multipathRingsRoutingPacketDef);	// 1 byte
	int sinkID;			// 2 bytes
	int senderLevel;	// 1 byte

	//DATA packet overhead contains all fields, making its total size 13 bytes
	//SETUP packet does not contain sequence number field, making its size 12 bytes
}


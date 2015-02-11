//****************************************************************************
//*  Copyright: National ICT Australia,  2007 - 2010                         *
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
#include "MacPacket_m.h"
}}

class MacPacket;

enum MAC802154Packet_type {
	MAC_802154_BEACON_PACKET = 1001;
	MAC_802154_ASSOCIATE_PACKET = 1002;
	MAC_802154_DATA_PACKET = 1003;
	MAC_802154_ACK_PACKET = 1004;
	MAC_802154_GTS_REQUEST_PACKET = 1005;
}

struct GTSspec {
	int owner;
	int start;
	int length;
}

packet Mac802154Packet extends MacPacket {
	int Mac802154PacketType enum (MAC802154Packet_type);
	int PANid;
	int srcID;
	int dstID;

	// those fields belong to beacon packet (MAC_802154_BEACON_PACKET)
	int beaconOrder;
	int frameOrder;
	int BSN;
	int CAPlength;
	int GTSlength;
	GTSspec GTSlist[];
}


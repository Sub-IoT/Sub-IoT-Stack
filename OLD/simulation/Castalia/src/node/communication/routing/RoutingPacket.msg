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

// We need to pass information between routing and MAC which is external to
// the packet i.e. not carried by a real packet (e.g., what is the next hop,
// or what was the RSSI for the packet received) but this information is related
// to the specific packet. Since information is passed between modules with
// messages/packets, we decided to encode this kind of external info as a 
// separate structure in the packet. The fields there are handled by the
// virtualRouting and virtualMAC code, setting a framework of interaction.

struct NetMacInfoExchange_type {
	double RSSI;
	double LQI;
	int nextHop;
	int lastHop;
}

// A generic routing packet header. An app packet will be encapsulated in it.
// If definining your own routing protocol and need a more specialized packet
// you have to create one the extends this generic packet.

packet RoutingPacket {
	NetMacInfoExchange_type netMacInfoExchange;
	
	string source;
	string destination;
	unsigned int sequenceNumber; 
}


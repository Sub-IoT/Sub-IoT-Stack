//********************************************************************************
//*  Copyright: National ICT Australia,  2007 - 2011                             *
//*  Developed at the ATP lab, Networked Systems research theme                  *
//*  Author(s): Yuriy Tselishchev, Athanassios Boulis                            *
//*  This file is distributed under the terms in the attached LICENSE file.      *
//*  If you do not find this file, copies can be found by writing to:            *
//*                                                                              *
//*      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia                 *
//*      Attention:  License Inquiry.                                            *
//*                                                                              *
//*******************************************************************************/

// We need to pass information between app and routing layer which is external to
// the packet i.e. not carried by a real packet (e.g., which is the destination,
// or what was the RSSI for the packet received) but this information is related
// to the specific packet. Since information is passed between modules with
// messages/packets, we decided to encode this kind of external info as a 
// separate structure in the packet. The fields there are handled by the
// virtualApp and virtualRouting code, setting a framework of interaction.

struct AppNetInfoExchange_type {
	double RSSI;	// the RSSI of the received packet
	double LQI;		// the LQI of the received packet
	string source;	// the routing layer source of the received packet
	string destination;	// the routing layer dest of the packet to be sent
	simtime_t timestamp;	// creation timestamp of the received packet 
} 

// A generic application packet. If defining your own packet you have to extend
// from this packet. You do not have to use the fields already defined, and you
// can always define your own size.

packet ApplicationPacket {
	AppNetInfoExchange_type appNetInfoExchange;

	string applicationID;		// virtual app uses application ID to filter packet delivery.
	unsigned int sequenceNumber;// a field to distinguish between packets
	double data;				// a simple type to carry some data
}


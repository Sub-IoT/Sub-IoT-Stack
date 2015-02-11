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

#define BASELINEBAN_HEADER_SIZE 7
#define BASELINEBAN_BEACON_SIZE 17 + 7
#define BASELINEBAN_CONNECTION_REQUEST_SIZE 28 + 7
#define BASELINEBAN_CONNECTION_ASSIGNMENT_SIZE 29 + 7

#define BROADCAST_NID 255
#define UNCONNECTED_BROADCAST_NID 0

}}

class MacPacket;

enum SecurityLevel_type {
    NOT_SECURED	= 1;
    AUTHENTICATED_NOT_ENCRYPTED = 2;
    AUTHENTICATED_AND_ENCRYPTED = 3;
    //RESERVED = 4; 
}

enum AcknowledgementPolicy_type {
    N_ACK_POLICY = 1;
    I_ACK_POLICY = 2;
    L_ACK_POLICY = 3;
    B_ACK_POLICY = 4;
}

enum Frame_type {
    MANAGEMENT = 1;
    CONTROL = 2;
    DATA = 3;
}

enum Frame_subtype {
    RESERVED = 0;
    BEACON = 1;
    ASSOCIATION = 2;
    DISASSOCIATION = 3;
    PTK = 4;
    GTK = 5;
    CONNECTION_REQUEST = 6;
    CONNECTION_ASSIGNMENT = 7;
    DISCONNECTION = 8;
    I_ACK = 9;
    B_ACK = 10;
    I_ACK_POLL = 11;
    B_ACK_POLL = 12;
    POLL = 13;
    T_POLL = 14;
}

enum statusCode_type {
    ACCEPTED = 0;
    REJ_NO_BANDWIDTH = 1;
    REJ_NO_NID = 2;
    REJ_NO_RESOURCES = 3;
    REJ_NO_HIGH_SECURITY = 4;
    REJ_NO_LOW_SECURITY = 5;
    REJ_NO_REASON = 6;
    MODIFIED = 7;
}

packet BaselineMacPacket extends MacPacket {
    //MAC Header fields
    int HID;										//2 bytes, 2 last bytes of full MAC address
    int NID;										//1 byte, short MAC address
    //MAC header control fields
    int protocolVersion;							//2 bits
    int securityLevel enum(SecurityLevel_type);		//2 bits
    int TKindex;									//1 bit
    int retry;										//1 bit
    int ackPolicy enum(AcknowledgementPolicy_type);	//2 bits	total: 1 byte
    int frameType enum(Frame_type);					//2 bits
    int frameSubtype enum(Frame_subtype);			//2 bits
    int moreData;									//1 bit
    int firstFrame;									//1 bit		total: 2 bytes
    int sequenceNum;								//1 byte
    int fragmentNumber; 							//3 bits
    //reserved 5 bits											total: 4 bytes
													//			total header: 7 bytes
}

// We define all different packet types that carry extra information on their 
// packet body, as separate packets. All derived from BaselinePacket.

packet BaselineBeaconPacket extends BaselineMacPacket {
    //Beacon payload fields
    int senderAddress;							//6 bytes
    int beaconShiftingSequenceIndex; 			//1 byte
    int beaconShiftingSequencePhase;			
    int beaconPeriodLength;						//1 byte
    int allocationSlotLength;					//1 byte
    int RAP1Length;								//1 byte
    int RAP2Length;								//1 byte
    // 6 more bytes for capabilites and channel hopping state
												//		total: 17 bytes + 7 bytes in header
}

packet BaselineConnectionRequestPacket extends BaselineMacPacket {
    int recipientAddress;						//6 bytes
    int senderAddress;							//6 bytes
    int formerHubAddress;						//6 bytes
    //4 bytes for capabilities and security
    int changeIndication;						//1 byte
    int nextWakeup;								//1 byte
    int wakeupInterval;							//1 byte
    int uplinkRequest;							//these requests are complex structures of atleast 
    int downlinkRequest;						//atleast 7 bytes length, here we will simplyfy it
    int bilinkRequest;							//by saying that this particular number of slots
												//is requested
												//    		total: 28 bytes + 7 bytes in header
}

packet BaselineConnectionAssignmentPacket extends BaselineMacPacket {
    int recipientAddress;                       //6 bytes
    int senderAddress;                          //6 bytes
    int channelDwellLength;						//1 byte
    int channelDwellPhase;						//1 byte
    int minRAPlength;							//1 byte
    int statusCode enum(statusCode_type);		//1 byte
    //4 bytes for capabilities and security
    int assignedNID;							//1 byte
    int changeIndication;                       //1 byte
    int nextWakeup;								//1 byte
    int wakeupinterval;							//1 byte
    
    int uplinkRequestStart;						//these requests are complex structures of at least 
    int uplinkRequestEnd;						//7 bytes length, here we will simplify them
    int downlinkRequestStart;					//by saying that this particular number of slots assigned
    int downlinkRequestEnd;
    int bilinkRequestStart;
    int bilinkRequestEnd;
												//		total: 29 bytes + 7 bytes in header
}

packet BaselineBAckPacket extends BaselineMacPacket {
    int nextBlockSize;							//1 byte
    int oldestFrameExpected;					//1 byte
    int frameStatusBitmap;						//1 byte
												//		total: 3 bytes + 7 bytes in header
}

packet BaselineTPollPacket extends BaselineMacPacket {
    int currentBeaconPeriod;				//1 byte
    int currentAllocationSlot;				//1 byte
    int currentSlotOffset;					//2 byte
    //the next two fields present only if NIC == UnconnectedBroadcast
    int beaconPeriodLength;					//1 byte
    int allocationSlotLength;				//1 byte
											//if connected,	total: 4 bytes + 7 bytes in header
											//unconnected, 	total: 6 bytes + 7 bytes in header
}

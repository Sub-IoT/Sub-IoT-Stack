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
#include "MacPacket_m.h"
}}

class MacPacket;

enum TmacPacket_type {
	SYNC_TMAC_PACKET = 1;
	RTS_TMAC_PACKET = 2;
	CTS_TMAC_PACKET = 3;
	DS_TMAC_PACKET = 4;
	FRTS_TMAC_PACKET = 5;
	DATA_TMAC_PACKET = 6;
	ACK_TMAC_PACKET = 7;
}

packet TMacPacket extends MacPacket {
	// This a basic field, essential for any packet
	// size including source and destination field (found 
	// in the generic MacPAcket) is 9 bytes in total
	int type enum (TmacPacket_type);	// 1 byte

	// RTS and CTS frames also contain nav field, bringing their size to 13 bytes
	simtime_t nav = 0;					// 4 bytes

	// Sequence number is essential for ACK and DATA frames, but they do not 
	// contain NAV field, therefore the size of ACK packet and MAC 
	// layer overhead in general is 11 bytes. We use the field in the
	// generic MacPacket and count it as 2 bytes.

	// SYNC packet has only three fields: sequence number, sync value and sync ID, 
	// making its total size only 11 bytes (1 extra byte comes from packet type) 
	// In reality, SYNC id would probably be stored in source field and sync 
	// value in NAV field. But in this model we do not create a separate packet 
	// subclass for each packet type
	simtime_t sync = 0;					// 4 bytes
	int syncId = 0;						// 4 bytes
}


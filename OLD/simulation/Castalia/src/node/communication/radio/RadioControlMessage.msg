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

// These messages will be initiated by the radio and sent to upper layer
enum RadioControlMessage_type {
	CARRIER_SENSE_INTERRUPT = 1;
	RADIO_BUFFER_FULL = 2;
}

message RadioControlMessage {
	int radioControlMessageKind enum (RadioControlMessage_type);
}

enum BasicState_type {
	RX = 0;
	TX = 1;
	SLEEP = 2;
}

// These commands will be received by the radio from upper layers
enum RadioControlCommand_type {
	SET_STATE = 0;
	SET_MODE = 1;
	SET_TX_OUTPUT = 2;
	SET_SLEEP_LEVEL = 3;
	SET_CARRIER_FREQ = 4;
	SET_CCA_THRESHOLD = 5;
	SET_CS_INTERRUPT_ON = 6;
	SET_CS_INTERRUPT_OFF = 7;
	SET_ENCODING = 8;
} 

message RadioControlCommand {
	int radioControlCommandKind enum (RadioControlCommand_type);
	int state enum (BasicState_type) = RX;	//to be used with SET_STATE
	double parameter = 0.0;	//to be used with SET_TX_OUTPUT, SET_CARRIER_FREQ, SET_CCA_THRESHOLD
	string name = "";	//to be used with SET_MODE, SET_SLEEP_LEVEL and SET_ENCODING
}

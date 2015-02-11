//****************************************************************************
//*  Copyright: National ICT Australia,  2007 - 2010                         *
//*  Developed at the ATP lab, Networked Systems research theme              *
//*  Author(s): Yuriy Tselishchev                                            *
//*  This file is distributed under the terms in the attached LICENSE file.  *
//*  If you do not find this file, copies can be found by writing to:        *
//*                                                                          *
//*      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
//*      Attention:  License Inquiry.                                        *
//*                                                                          *  
//****************************************************************************/

enum TunableMacCommandDef {
	SET_DUTY_CYCLE = 1;
	SET_LISTEN_INTERVAL = 2;
	SET_BEACON_INTERVAL_FRACTION = 3;
	SET_PROB_TX = 4;
	SET_NUM_TX = 5;
	SET_RANDOM_TX_OFFSET = 6;
	SET_RETX_INTERVAL = 7;
	SET_BACKOFF_TYPE = 8;
	SET_BACKOFF_BASE_VALUE = 9;
}

message TunableMacControlCommand {
	int tunableMacCommandKind enum (TunableMacCommandDef);
	double parameter = 0;
}


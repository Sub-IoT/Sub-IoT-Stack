//****************************************************************************
//*  Copyright: National ICT Australia,  2007 - 2010                         *
//*  Developed at the ATP lab, Networked Systems research theme              *
//*  Author(s): Dimosthenis Pediaditakis, Yuriy Tselishchev                  *
//*  This file is distributed under the terms in the attached LICENSE file.  *
//*  If you do not find this file, copies can be found by writing to:        *
//*                                                                          *
//*      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
//*      Attention:  License Inquiry.                                        *
//*                                                                          *  
//****************************************************************************/

enum multipathRingsRoutingControlDef {
	MPRINGS_NOT_CONNECTED = 1;
	MPRINGS_CONNECTED_TO_TREE = 2;
	MPRINGS_TREE_LEVEL_UPDATED = 3;
}

message MultipathRingsRoutingControlMessage {
	int multipathRingsRoutingControlMessageKind enum (multipathRingsRoutingControlDef);
	int sinkID;
	int level;
}


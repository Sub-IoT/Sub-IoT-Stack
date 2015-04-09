//********************************************************************************
//*  Copyright: National ICT Australia,  2007 - 2010                             *
//*  Developed at the ATP lab, Networked Systems research theme                  *
//*  Author(s): Athanassios Boulis, Dimosthenis Pediaditakis, Yuriy Tselishchev  *
//*  This file is distributed under the terms in the attached LICENSE file.      *
//*  If you do not find this file, copies can be found by writing to:            *
//*                                                                              *
//*      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia                 *
//*      Attention:  License Inquiry.                                            *
//*                                                                              *
//*******************************************************************************/

message WirelessChannelSignalBegin {
	int nodeID;
	double power_dBm;
	double carrierFreq;
	double bandwidth;
	int modulationType;
	int encodingType;
}

// NOTE: signal end is of type 'packet' so we can encapsulate the 
// actual packet being transfered
packet WirelessChannelSignalEnd {
	int nodeID;		//references the original nodeID in signal begin
}

message WirelessChannelNodeMoveMessage {
	int nodeID;
	double x;
	double y;
	double z;
	double phi;
	double theta;
}


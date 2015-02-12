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

package node.communication;

module CommunicationModule {
 parameters:

 gates:
	output toWirelessChannel;
	output toHalModule;
	input fromWirelessChannel;
	input fromHalModule;
	input fromResourceManager;

 submodules:
 	Radio: node.communication.radio.Radio;
 connections:

	fromWirelessChannel --> Radio.fromWirelessChannel;
	fromHalModule --> Radio.fromStackModule;
	fromResourceManager --> Radio.fromResourceManager;
	Radio.toWirelessChannel --> toWirelessChannel;
	Radio.toStackModule --> toHalModule;
	
}


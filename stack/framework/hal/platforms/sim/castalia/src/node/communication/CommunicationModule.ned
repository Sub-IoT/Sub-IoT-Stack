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
 	string MACProtocolName = default ("BypassMAC");
	string RoutingProtocolName = default ("BypassRouting");

 gates:
 	output toApplicationModule;
	output toNodeContainerModule;
	input fromApplicationModule;
	input fromNodeContainerModule;
	input fromResourceManager2Net;
	input fromResourceManager2Mac;
	input fromResourceManager2Radio;

 submodules:
 	Radio: node.communication.radio.Radio;
	MAC: <MACProtocolName> like node.communication.mac.iMac;
	Routing: <RoutingProtocolName> like node.communication.routing.iRouting;

 connections:
 	fromApplicationModule --> Routing.fromCommunicationModule;
	Routing.toCommunicationModule --> toApplicationModule;
	Routing.toMacModule --> MAC.fromNetworkModule;
	MAC.toNetworkModule --> Routing.fromMacModule;
	MAC.toRadioModule --> Radio.fromMacModule;
	Radio.toMacModule --> MAC.fromRadioModule;
	
	fromNodeContainerModule --> Radio.fromCommunicationModule;
	Radio.toCommunicationModule --> toNodeContainerModule;

	fromResourceManager2Net --> Routing.fromCommModuleResourceMgr;
	fromResourceManager2Mac --> MAC.fromCommModuleResourceMgr;
	fromResourceManager2Radio --> Radio.fromCommModuleResourceMgr;
}


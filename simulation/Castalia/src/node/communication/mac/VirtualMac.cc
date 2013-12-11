/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2011                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Yuriy Tselishchev, Athanassios Boulis                        *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *
 ****************************************************************************/

#include "VirtualMac.h"

void VirtualMac::initialize()
{
	macBufferSize = par("macBufferSize");
	macFrameOverhead = par("macPacketOverhead");
	macMaxFrameSize = par("macMaxPacketSize");

	self = getParentModule()->getParentModule()->getIndex();

	/* Get a valid references to the Resources Manager module and the
	 * Radio module, so that we can make direct calls to their public methods
	 */
	radioModule = check_and_cast <Radio*>(getParentModule()->getSubmodule("Radio"));
	resMgrModule = check_and_cast <ResourceManager*>(getParentModule()->getParentModule()->getSubmodule("ResourceManager"));

	if (!resMgrModule || !radioModule)
		opp_error("\n Virtual Routing init: Error in geting a valid reference module(s).");

	setTimerDrift(resMgrModule->getCPUClockDrift());
	pktHistory.clear();
	disabled = true;
	currentSequenceNumber = 1;
}

int VirtualMac::handleControlCommand(cMessage * msg)
{
	trace() << "WARNING: handleControlCommand not defined in this module";
	return 0;
}

int VirtualMac::handleRadioControlMessage(cMessage * msg)
{
	toNetworkLayer(msg);
	return 1;
}

int VirtualMac::bufferPacket(cPacket * rcvFrame)
{
	if ((int)TXBuffer.size() >= macBufferSize) {
		cancelAndDelete(rcvFrame);
		// send a control message to the upper layer
		MacControlMessage *fullBuffMsg =
		    new MacControlMessage("MAC buffer full", MAC_CONTROL_MESSAGE);
		fullBuffMsg->setMacControlMessageKind(MAC_BUFFER_FULL);
		send(fullBuffMsg, "toNetworkModule");
		return 0;
	} else {
		TXBuffer.push(rcvFrame);
		trace() << "Packet buffered from network layer, buffer state: "
		    << TXBuffer.size() << "/" << macBufferSize;
		return 1;
	}
}

void VirtualMac::handleMessage(cMessage * msg)
{

	int msgKind = (int)msg->getKind();

	if (disabled && msgKind != NODE_STARTUP) {
		delete msg;
		return;
	}

	switch (msgKind) {

		case NODE_STARTUP:{
			disabled = false;
			send(new cMessage("MAC --> Radio startup message", NODE_STARTUP), "toRadioModule");
			startup();
			break;
		}

		case NETWORK_LAYER_PACKET:{
			RoutingPacket *pkt = check_and_cast <RoutingPacket*>(msg);
			if (macMaxFrameSize > 0 && macMaxFrameSize < pkt->getByteLength() + macFrameOverhead) {
				trace() << "Oversized packet dropped. Size:" << pkt->getByteLength() <<
						", MAC layer overhead:" << macFrameOverhead <<
						", max MAC frame size:" << macMaxFrameSize;
				break;
			}
			/* Control is now passed to a specific MAC protocol by calling fromNetworkLayer()
			 * Notice that after the call we RETURN (not BREAK) so that the packet is not deleted.
			 * This is done since the packet will most likely be encapsulated and forwarded to the
			 * Radio layer. If the protocol specific function wants to discard the packet is has
			 * to delete it.
			 */
			fromNetworkLayer(pkt, pkt->getNetMacInfoExchange().nextHop);
			return;
		}

		case MAC_LAYER_PACKET:{
			MacPacket *pkt = check_and_cast <MacPacket*>(msg);
			/* Control is now passed to a specific routing protocol by calling fromRadioLayer()
			 * Notice that after the call we BREAK so that the MAC packet gets deleted.
			 * This will not delete the encapsulated NET packet if it gets decapsulated
			 * by fromMacLayer(), i.e., the normal/expected action.
			 */
			fromRadioLayer(pkt, pkt->getMacRadioInfoExchange().RSSI,
								pkt->getMacRadioInfoExchange().LQI);
			break;
		}

		case TIMER_SERVICE:{
			handleTimerMessage(msg);
			break;
		}

		case MAC_CONTROL_COMMAND:{
			if (handleControlCommand(msg))
				return;
			break;
		}

		case RADIO_CONTROL_COMMAND:{
			toRadioLayer(msg);
			return; // do not delete msg
		}

		case RADIO_CONTROL_MESSAGE:{
			if (handleRadioControlMessage(msg))
				return;
			break;
		}

		case OUT_OF_ENERGY:{
			disabled = true;
			break;
		}

		case DESTROY_NODE:{
			disabled = true;
			break;
		}

		default:{
			opp_error("MAC module received message of unknown kind %i", msgKind);
		}
	}

	delete msg;
}

void VirtualMac::finish()
{
	CastaliaModule::finish();
	while (!TXBuffer.empty()) {
		cancelAndDelete(TXBuffer.front());
		TXBuffer.pop();
	}
}

void VirtualMac::toNetworkLayer(cMessage * macMsg)
{
	trace() << "Delivering [" << macMsg->getName() << "] to Network layer";
	send(macMsg, "toNetworkModule");
}

void VirtualMac::toRadioLayer(cMessage * macMsg)
{
	send(macMsg, "toRadioModule");
}

void VirtualMac::encapsulatePacket(cPacket * pkt, cPacket * netPkt)
{
	MacPacket *macPkt = check_and_cast <MacPacket*>(pkt);
	macPkt->setByteLength(macFrameOverhead);
	macPkt->setKind(MAC_LAYER_PACKET);
	macPkt->setSequenceNumber(currentSequenceNumber++);
	macPkt->setSource(SELF_MAC_ADDRESS);
	// by default the packet created has its generic destination address to broadcast
	// a specific protocol can change this, and/or set more specific dest addresses
	macPkt->setDestination(BROADCAST_MAC_ADDRESS);
	macPkt->encapsulate(netPkt);
}

cPacket *VirtualMac::decapsulatePacket(cPacket * pkt)
{
	MacPacket *macPkt = check_and_cast <MacPacket*>(pkt);
	RoutingPacket *netPkt = check_and_cast <RoutingPacket*>(macPkt->decapsulate());
	netPkt->getNetMacInfoExchange().RSSI = macPkt->getMacRadioInfoExchange().RSSI;
	netPkt->getNetMacInfoExchange().LQI = macPkt->getMacRadioInfoExchange().LQI;
	// The lastHop field has valid information only if the specific MAC protocol
	// updates the generic'source' field in the MacPacket.
	netPkt->getNetMacInfoExchange().lastHop = macPkt->getSource();
	return netPkt;
}

bool VirtualMac::isNotDuplicatePacket(cPacket * pkt)
{
	//extract source address and sequence number from the packet
	MacPacket *macPkt = check_and_cast <MacPacket*>(pkt);
	int src = macPkt->getSource();
	unsigned int sn = macPkt->getSequenceNumber();

	//resize packet history vector if necessary
	if (src >= (int)pktHistory.size())
		pktHistory.resize(src+1,0);

	//if recorded sequence number is less than new
	//then packet is new (i.e. not duplicate)
	if (pktHistory[src] < sn) {
		pktHistory[src] = sn;
		return true;
	}

	return false;
}


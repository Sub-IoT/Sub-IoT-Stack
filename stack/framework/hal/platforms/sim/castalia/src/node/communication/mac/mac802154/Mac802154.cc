/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2010                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Athanassios Boulis, Yuriy Tselishchev                        *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *  
 ****************************************************************************/

#include "Mac802154.h"

Define_Module(Mac802154);

void Mac802154::startup()
{
	readIniFileParameters();

	//Initialise state descriptions used in debug output
	if (printStateTransitions) {
		stateDescr[1000] = "MAC_STATE_SETUP";
		stateDescr[1001] = "MAC_STATE_SLEEP";
		stateDescr[1002] = "MAC_STATE_IDLE";
		stateDescr[1003] = "MAC_STATE_CSMA_CA";
		stateDescr[1004] = "MAC_STATE_CCA";
		stateDescr[1005] = "MAC_STATE_IN_TX";
		stateDescr[1006] = "MAC_STATE_WAIT_FOR_ASSOCIATE_RESPONSE";
		stateDescr[1007] = "MAC_STATE_WAIT_FOR_DATA_ACK";
		stateDescr[1008] = "MAC_STATE_WAIT_FOR_BEACON";
		stateDescr[1009] = "MAC_STATE_WAIT_FOR_GTS";
		stateDescr[1011] = "MAC_STATE_PROCESSING";
	}

	phyDataRate = par("phyDataRate");
	phyDelaySleep2Tx = (double)par("phyDelaySleep2Tx") / 1000.0;
	phyDelayRx2Tx = (double)par("phyDelayRx2Tx") / 1000.0;
	phyDelayForValidCS = (double)par("phyDelayForValidCS") / 1000.0;
	phyLayerOverhead = par("phyFrameOverhead");
	phyBitsPerSymbol = par("phyBitsPerSymbol");

	guardTime = (double)par("guardTime") / 1000.0;

	/**************************************************************************************************
	 *			802.15.4 specific intialize
	 **************************************************************************************************/

	symbolLen = 1 / (phyDataRate * 1000 / phyBitsPerSymbol);
	ackWaitDuration = symbolLen * unitBackoffPeriod + 
		phyDelayRx2Tx * 2 + TX_TIME(ACK_PKT_SIZE);

	beaconPacket = NULL;
	associateRequestPacket = NULL;
	nextPacket = NULL;
	nextPacketResponse = 0;
	nextPacketRetries = 0;
	nextPacketState = "";
	nextMacState = 0;

	macState = MAC_STATE_SETUP;
	macBSN = rand() % 255;
	lockedGTS = false;
	associatedPAN = -1;
	currentFrameStart = 0;
	GTSstart = 0;
	GTSend = 0;
	CAPend = 0;
	lostBeacons = 0;
	sentBeacons = 0;
	recvBeacons = 0;
	packetoverflow = 0;

	desyncTime = 0;
	desyncTimeStart = 0;

	packetBreak.clear();

	if (isFFD && isPANCoordinator) {
		associatedPAN = SELF_MAC_ADDRESS;
		setTimer(FRAME_START, 0);	//frame start is NOW
	}
}

void Mac802154::timerFiredCallback(int index)
{
	switch (index) {

		case FRAME_START:{
			if (isPANCoordinator) {	// as a PAN coordinator, create and broadcast beacon packet
				beaconPacket = new Mac802154Packet("PAN beacon packet", MAC_LAYER_PACKET);
				beaconPacket->setDstID(BROADCAST_MAC_ADDRESS);
				beaconPacket->setPANid(SELF_MAC_ADDRESS);
				beaconPacket->setMac802154PacketType(MAC_802154_BEACON_PACKET);
				beaconPacket->setBeaconOrder(beaconOrder);
				beaconPacket->setFrameOrder(frameOrder);
				if (macBSN > 254) {
					macBSN = 0;
				} else {
					macBSN++;
				}
				beaconPacket->setBSN(macBSN);

				CAPlength = numSuperframeSlots;
				beaconPacket->setGTSlistArraySize(GTSlist.size());
				for (int i = 0; i < (int)GTSlist.size(); i++) {
					if (CAPlength > GTSlist[i].length) {
						CAPlength -= GTSlist[i].length;
						GTSlist[i].start = CAPlength + 1;
						beaconPacket->setGTSlist(i, GTSlist[i]);
					} else {
						GTSlist[i].length = 0;
						trace() << "Internal ERROR: GTS list corrupted";
					}
				}

				beaconPacket->setCAPlength(CAPlength);
				beaconPacket->setByteLength(BASE_BEACON_PKT_SIZE + GTSlist.size() * GTS_SPEC_FIELD_SIZE);
				CAPend = CAPlength * baseSlotDuration * (1 << frameOrder) * symbolLen;
				sentBeacons++;

				setMacState(MAC_STATE_IN_TX);
				toRadioLayer(beaconPacket);
				toRadioLayer(createRadioCommand(SET_STATE, TX));
				setTimer(ATTEMPT_TX, TX_TIME(beaconPacket->getByteLength()));
				beaconPacket = NULL;

				setTimer(FRAME_START, beaconInterval * symbolLen);
				currentFrameStart = getClock() + phyDelayRx2Tx;
			} else {	// if not a PAN coordinator, then wait for beacon
				toRadioLayer(createRadioCommand(SET_STATE, RX));
				setMacState(MAC_STATE_WAIT_FOR_BEACON);
				setTimer(BEACON_TIMEOUT, guardTime * 3);
			}
			break;
		}

		// beacon timeout fired - indicates that beacon was missed by this node
		case BEACON_TIMEOUT:{
			lostBeacons++;
			if (lostBeacons >= maxLostBeacons) {
				trace() << "Lost synchronisation with PAN " << associatedPAN;
				setMacState(MAC_STATE_SETUP);
				associatedPAN = -1;
				lockedGTS = false;
				desyncTimeStart = getClock();
			} else if (associatedPAN != -1) {
				trace() << "Missed beacon from PAN " << associatedPAN <<
				    ", will wake up to receive next beacon in " << 
				    beaconInterval * symbolLen - guardTime * 3 << " seconds";
				setMacState(MAC_STATE_SLEEP);
				toRadioLayer(createRadioCommand(SET_STATE, SLEEP));
				setTimer(FRAME_START, beaconInterval * symbolLen - guardTime * 3);
			}
			break;
		}

		case GTS_START:{
			if (macState == MAC_STATE_WAIT_FOR_DATA_ACK ||
			    macState == MAC_STATE_WAIT_FOR_ASSOCIATE_RESPONSE ||
			    macState == MAC_STATE_PROCESSING) {
				break;
			}

			toRadioLayer(createRadioCommand(SET_STATE, RX));
			// we delay transmission attempt by the time requred by radio to wake up
			// note that GTS_START timer was scheduled exactly phyDelaySleep2Tx seconds
			// earlier than the actual start time of GTS slot
			setMacState(MAC_STATE_PROCESSING);
			setTimer(ATTEMPT_TX, phyDelaySleep2Tx);
			
			// set a timer to go to sleep after our GTS slot ends
			setTimer(START_SLEEPING, phyDelaySleep2Tx + GTSlength);
			break;
		}

		// previous transmission is reset, attempt a new transmission 
		case ATTEMPT_TX:{
			if (macState != MAC_STATE_IN_TX
			    && macState != MAC_STATE_WAIT_FOR_DATA_ACK
			    && macState != MAC_STATE_WAIT_FOR_ASSOCIATE_RESPONSE
			    && macState != MAC_STATE_WAIT_FOR_GTS
			    && macState != MAC_STATE_PROCESSING) {
				trace() << "WARNING ATTEMPT_TX timer was not cancelled, macState is " << macState;
				break;
			}

			if (macState == MAC_STATE_WAIT_FOR_DATA_ACK)
				collectPacketState("NoAck");

			attemptTransmission("ATTEMPT_TX timer");
			break;
		}

		// timer to preform carrier sense
		case PERFORM_CCA:{
			if (macState != MAC_STATE_CSMA_CA)
				break;
			CCA_result CCAcode = radioModule->isChannelClear();

			if (CCAcode == CLEAR) {
				CW--;
				if (CW != 0) {
					// since carrier is clear, no need to generate another random delay
					setTimer(PERFORM_CCA, unitBackoffPeriod * symbolLen);
				} else if (!nextPacket) {
					trace() << "ERROR: CSMA_CA algorithm executed without any data to transmit";
					attemptTransmission("ERROR in CSMA_CA");
				} else {
					// CSMA-CA successful (CW == 0), can transmit the queued packet
					transmitNextPacket();
				}
			} else if (CCAcode == BUSY) {
				//if MAC is preforming CSMA-CA, update corresponding parameters
				CW = enableSlottedCSMA ? 2 : 1;
				NB++;
				BE++;
				if (BE > macMaxBE)
					BE = macMaxBE;
				if (NB > macMaxCSMABackoffs) {
					collectPacketState("CSfail");
					nextPacketRetries--;
					attemptTransmission("NB exeeded maxCSMAbackoffs");
				} else {
					setMacState(MAC_STATE_CSMA_CA);
					continueCSMACA();
				}
			} else if (CCAcode == CS_NOT_VALID_YET) {
				setTimer(PERFORM_CCA, phyDelayForValidCS);
			} else {	//CS_NOT_VALID
				opp_error("802.15.4 MAC internal error, isChannelClear() function called when radio is NOT READY");
			}
			break;
		}

		case START_SLEEPING:{
			setMacState(MAC_STATE_SLEEP);
			toRadioLayer(createRadioCommand(SET_STATE, SLEEP));
			break;
		}
	}
}

/* This indicates that a packet is received from upper layer (Network)
 * First we check that the MAC buffer is capable of holding the new packet, also check
 * that length does not exceed the valid mac frame size. Then store a copy of the packet
 * is stored in transmission buffer. We dont need to encapsulate the message here - it will
 * be done separately for each transmission attempt
 */
void Mac802154::fromNetworkLayer(cPacket * pkt, int dstMacAddress)
{
	Mac802154Packet *macPacket = new Mac802154Packet("802.15.4 MAC data packet", MAC_LAYER_PACKET);
	macPacket->setSrcID(SELF_MAC_ADDRESS);	//if we are connected, we would have short MAC address assigned, 
	//but we are not using short addresses in this model
	macPacket->setDstID(dstMacAddress);
	macPacket->setMac802154PacketType(MAC_802154_DATA_PACKET);
	encapsulatePacket(macPacket, pkt);

	if (bufferPacket(macPacket)) {
		if (macState == MAC_STATE_IDLE)
			attemptTransmission("New packet from network layer");
	} else {
		packetoverflow++;
		//full buffer message
	}
}

void Mac802154::finishSpecific()
{
	if (nextPacket)
		cancelAndDelete(nextPacket);
	if (desyncTimeStart >= 0)
		desyncTime += getClock() - desyncTimeStart;

	map <string,int>::const_iterator iter;
	declareOutput("Packet breakdown");
	if (packetoverflow > 0)
		collectOutput("Packet breakdown", "Failed, buffer overflow", packetoverflow);
	for (iter = packetBreak.begin(); iter != packetBreak.end(); ++iter) {
		if (iter->first.compare("Success") == 0) {
			collectOutput("Packet breakdown", "Success, first try", iter->second);
		} else if (iter->first.compare("Broadcast") == 0) {
			collectOutput("Packet breakdown", "Broadcast", iter->second);
		} else if (iter->first.find("Success") != string::npos) {
			collectOutput("Packet breakdown", "Success, not first try", iter->second);
		} else if (iter->first.find("NoAck") != string::npos) {
			collectOutput("Packet breakdown", "Failed, no ack", iter->second);
		} else if (iter->first.find("CSfail") != string::npos) {
			collectOutput("Packet breakdown", "Failed, busy channel", iter->second);
		} else if (iter->first.find("NoPAN") != string::npos) {
			collectOutput("Packet breakdown", "Failed, no PAN", iter->second);
		} else {
			trace() << "Unknown packet breakdonw category: " << 
				iter->first << " with " << iter->second << " packets";
		}
	}

	if (!isPANCoordinator) {
		if (desyncTime > 0) {
			declareOutput("Fraction of time without PAN connection");
			collectOutput("Fraction of time without PAN connection", "",
				SIMTIME_DBL(desyncTime) / SIMTIME_DBL(getClock()));
		}
		declareOutput("Number of beacons received");
		collectOutput("Number of beacons received", "", recvBeacons);
	} else {
		declareOutput("Number of beacons sent");
		collectOutput("Number of beacons sent", "", sentBeacons);
	}
}

void Mac802154::readIniFileParameters(void)
{
	printStateTransitions = par("printStateTransitions");

	enableSlottedCSMA = par("enableSlottedCSMA");
	enableCAP = par("enableCAP");
	requestGTS = par("requestGTS");

	isPANCoordinator = par("isPANCoordinator");
	isFFD = par("isFFD");

	unitBackoffPeriod = par("unitBackoffPeriod");
	baseSlotDuration = par("baseSlotDuration");
	numSuperframeSlots = par("numSuperframeSlots");
	maxLostBeacons = par("maxLostBeacons");
	minCAPLength = par("minCAPLength");
	macMinBE = par("macMinBE");
	macMaxBE = par("macMaxBE");
	macMaxCSMABackoffs = par("macMaxCSMABackoffs");
	macMaxFrameRetries = par("macMaxFrameRetries");
	batteryLifeExtention = par("batteryLifeExtention");
	baseSuperframeDuration = baseSlotDuration * numSuperframeSlots;

	if (isPANCoordinator) {
		if (!isFFD) {
			opp_error("Only full-function devices (isFFD=true) can be PAN coordinators");
		}

		requestGTS = 0;
		enableCAP = true;
		frameOrder = par("frameOrder");
		beaconOrder = par("beaconOrder");
		if (frameOrder < 0 || beaconOrder < 0 || beaconOrder > 14
		    || frameOrder > 14 || beaconOrder < frameOrder) {
			opp_error("Invalid combination of frameOrder and beaconOrder parameters. Must be 0 <= frameOrder <= beaconOrder <= 14");
		}

		beaconInterval = baseSuperframeDuration * (1 << beaconOrder);
		frameInterval = baseSuperframeDuration * (1 << frameOrder);
		CAPlength = numSuperframeSlots;

		if (beaconInterval <= 0 || frameInterval <= 0) {
			opp_error("Invalid parameter combination of baseSlotDuration and numSuperframeSlots");
		}
	}
}

/* Helper function to change internal MAC state and print a debug statement if neccesary */
void Mac802154::setMacState(int newState)
{
	if (macState == newState)
		return;
	if (printStateTransitions)
		trace() << "state changed from " << stateDescr[macState] << " to " << stateDescr[newState];
	macState = newState;
}

/* This function will handle a MAC frame received from the lower layer (physical or radio)
 */
void Mac802154::fromRadioLayer(cPacket * pkt, double rssi, double lqi)
{
	Mac802154Packet *rcvPacket = dynamic_cast < Mac802154Packet * >(pkt);
	if (!rcvPacket)
		return;

	switch (rcvPacket->getMac802154PacketType()) {

		/* received a BEACON frame */
		case MAC_802154_BEACON_PACKET:{
			int PANaddr = rcvPacket->getPANid();
			recvBeacons++;
			if (associatedPAN == -1) {
				//if not associated - create an association request packet
				if (nextPacket) {
					if (collectPacketState("NoPAN"))
						packetBreak[nextPacketState]++;
					cancelAndDelete(nextPacket);
				}
				nextPacket = new Mac802154Packet("PAN associate request", MAC_LAYER_PACKET);
				nextPacket->setDstID(PANaddr);
				nextPacket->setPANid(PANaddr);
				nextPacket->setMac802154PacketType(MAC_802154_ASSOCIATE_PACKET);
				nextPacket->setSrcID(SELF_MAC_ADDRESS);
				nextPacket->setByteLength(COMMAND_PKT_SIZE);
				initiateCSMACA(9999, MAC_STATE_WAIT_FOR_ASSOCIATE_RESPONSE,
					ackWaitDuration + TX_TIME(COMMAND_PKT_SIZE));
			} else if (associatedPAN != PANaddr) {
				// if associated to a different PAN - do nothing
				return;
			}
			//update frame parameters
			currentFrameStart = getClock() - TX_TIME(rcvPacket->getByteLength());
			lostBeacons = 0;
			frameOrder = rcvPacket->getFrameOrder();
			beaconOrder = rcvPacket->getBeaconOrder();
			beaconInterval = baseSuperframeDuration * (1 << beaconOrder);
			macBSN = rcvPacket->getBSN();
			CAPlength = rcvPacket->getCAPlength();
			CAPend = CAPlength * baseSlotDuration * (1 << frameOrder) * symbolLen;
			GTSstart = 0;
			GTSend = 0;
			GTSlength = 0;

			for (int i = 0; i < (int)rcvPacket->getGTSlistArraySize(); i++) {
				if (lockedGTS && rcvPacket->getGTSlist(i).owner == SELF_MAC_ADDRESS) {
					GTSstart = (rcvPacket->getGTSlist(i).start - 1) * 
						baseSlotDuration * (1 << frameOrder) * symbolLen;
					GTSend = GTSstart + rcvPacket->getGTSlist(i).length *
					    baseSlotDuration * (1 << frameOrder) * symbolLen;
					GTSlength = GTSend - GTSstart;
					trace() << "GTS slot from " << getClock() +
					    GTSstart << " to " << getClock() + GTSend;
				}
			}

			//cancel beacon timeout message (if present)
			cancelTimer(BEACON_TIMEOUT);

			if (requestGTS) {
				if (lockedGTS) {
					if (GTSstart == 0) {
						trace() << "invalid state, GTS descriptor not found in beacon frame";
						lockedGTS = false;
					}
				} else if (associatedPAN == PANaddr) {
					issueGTSrequest();
				}
			}

			if (associatedPAN == PANaddr) {
				if (enableCAP) {
					attemptTransmission("CAP started");
					//set timer to start sleeping
					if (GTSstart == CAPend)
						// if GTS slot starts right after CAP, then
						// we start sleeping after both CAP and GTS slots end
						setTimer(START_SLEEPING, CAPend + GTSlength);
					else
						// if GTS slot does not start after CAP (or no GTS at all)
						// then we start sleeping right after CAP ends
						setTimer(START_SLEEPING, CAPend);
				} else {
					setMacState(MAC_STATE_SLEEP);
					toRadioLayer(createRadioCommand(SET_STATE, SLEEP));
				}
				if (GTSstart != 0 && (!enableCAP || GTSstart != CAPend))
					// if GTS slot exists and does not start after CAP (or CAP is disabled) 
					// then we set GTS timer phyDelaySleep2Tx seconds earlier as radio will be sleeping
					setTimer(GTS_START, GTSstart - phyDelaySleep2Tx);
			}

			setTimer(FRAME_START, baseSuperframeDuration * (1 << beaconOrder) *
				 symbolLen - guardTime - TX_TIME(rcvPacket->getByteLength()));
			break;
		}

		// request to associate
		case MAC_802154_ASSOCIATE_PACKET:{
			// only PAN coordinators can accept association requests
			// if multihop communication is to be allowed - then this has to be changed
			// in particular, any FFD can become a coordinator and accept requests
			if (!isPANCoordinator)
				break;

			// if PAN id is not the same as my ID - do nothing
			if (rcvPacket->getPANid() != SELF_MAC_ADDRESS)
				break;

			// update associatedDevices and reply with an ACK (i.e. association is always allowed)
			trace() << "Received association request from " << rcvPacket->getSrcID();
			associatedDevices[rcvPacket->getSrcID()] = true;
			Mac802154Packet *ackPacket = new Mac802154Packet("PAN associate response", MAC_LAYER_PACKET);
			ackPacket->setPANid(SELF_MAC_ADDRESS);
			ackPacket->setMac802154PacketType(MAC_802154_ACK_PACKET);
			ackPacket->setDstID(rcvPacket->getSrcID());
			ackPacket->setByteLength(ACK_PKT_SIZE);
			toRadioLayer(ackPacket);
			toRadioLayer(createRadioCommand(SET_STATE, TX));

			setMacState(MAC_STATE_IN_TX);
			setTimer(ATTEMPT_TX, TX_TIME(ACK_PKT_SIZE));
			break;
		}

		case MAC_802154_GTS_REQUEST_PACKET:{
			if (!isPANCoordinator)
				break;
			if (rcvPacket->getPANid() != SELF_MAC_ADDRESS)
				break;
			trace() << "Received GTS request from " << rcvPacket->getSrcID();

			Mac802154Packet *ackPacket = new Mac802154Packet("PAN GTS response", MAC_LAYER_PACKET);
			ackPacket->setPANid(SELF_MAC_ADDRESS);
			ackPacket->setMac802154PacketType(MAC_802154_ACK_PACKET);
			ackPacket->setDstID(rcvPacket->getSrcID());
			ackPacket->setByteLength(ACK_PKT_SIZE);
			ackPacket->setGTSlength(0);

			int index = -1;
			for (int i = 0; i < (int)GTSlist.size(); i++) {
				if (GTSlist[i].owner == rcvPacket->getSrcID()) {
					if (GTSlist[i].length == rcvPacket->getGTSlength()) {
						ackPacket->setGTSlength(GTSlist[i].length);
					} else {
						CAPlength += GTSlist[i].length;
						GTSlist[i].length = 0;
						index = i;
					}
				}
			}

			if (ackPacket->getGTSlength() == 0) {
				if ((CAPlength - rcvPacket->getGTSlength()) *
				    baseSlotDuration * (1 << frameOrder) < minCAPLength) {
					trace() << "GTS request from " << rcvPacket->getSrcID() <<
					    " cannot be acocmodated";
				} else if (index != -1) {
					GTSlist[index].length = rcvPacket->getGTSlength();
					ackPacket->setGTSlength(GTSlist[index].length);
				} else {
					GTSspec newGTSspec;
					newGTSspec.length = rcvPacket->getGTSlength();
					newGTSspec.owner = rcvPacket->getSrcID();
					GTSlist.push_back(newGTSspec);
				}
			}

			toRadioLayer(ackPacket);
			toRadioLayer(createRadioCommand(SET_STATE, TX));
			setMacState(MAC_STATE_IN_TX);
			setTimer(ATTEMPT_TX, TX_TIME(ACK_PKT_SIZE));

			break;
		}

		// ack frames are handled by a separate function
		case MAC_802154_ACK_PACKET:{
			if (rcvPacket->getDstID() != SELF_MAC_ADDRESS)
				break;
			handleAckPacket(rcvPacket);
			break;
		}

		// data frame
		case MAC_802154_DATA_PACKET:{
			int dstAddr = rcvPacket->getDstID();
			if (dstAddr != SELF_MAC_ADDRESS && dstAddr != BROADCAST_MAC_ADDRESS)
				break;

			toNetworkLayer(decapsulatePacket(rcvPacket));

			// If the frame was sent to broadcast address, nothing else needs to be done
			if (dstAddr == BROADCAST_MAC_ADDRESS)
				break;

			Mac802154Packet *ackPacket = new Mac802154Packet("Ack packet", MAC_LAYER_PACKET);
			ackPacket->setPANid(SELF_MAC_ADDRESS);
			ackPacket->setMac802154PacketType(MAC_802154_ACK_PACKET);
			ackPacket->setDstID(rcvPacket->getSrcID());
			ackPacket->setByteLength(ACK_PKT_SIZE);

			toRadioLayer(ackPacket);
			toRadioLayer(createRadioCommand(SET_STATE, TX));
			setMacState(MAC_STATE_IN_TX);
			setTimer(ATTEMPT_TX, TX_TIME(ACK_PKT_SIZE));

			break;
		}

		default:{
			trace() << "WARNING: unknown packet type received " << rcvPacket->getMac802154PacketType() << 
					" [" << rcvPacket->getName() << "]";
		}
	}
}

void Mac802154::handleAckPacket(Mac802154Packet * rcvPacket)
{
	switch (macState) {

		//received an ack while waiting for a response to association request
		case MAC_STATE_WAIT_FOR_ASSOCIATE_RESPONSE:{
			associatedPAN = rcvPacket->getPANid();
			if (desyncTimeStart >= 0) {
				desyncTime += getClock() - desyncTimeStart;
				desyncTimeStart = -1;
			}
			trace() << "associated with PAN:" << associatedPAN;
			cancelAndDelete(nextPacket);
			nextPacket = NULL;
			if (requestGTS) {
				issueGTSrequest();
			} else {
				attemptTransmission("Associated with PAN");
			}
			break;
		}

		//received an ack while waiting for a response to data packet
		case MAC_STATE_WAIT_FOR_DATA_ACK:{
			if (isPANCoordinator || associatedPAN == rcvPacket->getSrcID()) {
				collectPacketState("Success");
				trace() << "Packet successfully transmitted to " << rcvPacket->getSrcID();
				nextPacketRetries = 0;
				attemptTransmission("ACK received");
			}
			break;
		}

		case MAC_STATE_WAIT_FOR_GTS:{
			lockedGTS = true;
			cancelAndDelete(nextPacket);
			nextPacket = NULL;
			if (enableCAP) {
				attemptTransmission("GTS request granted");
			} else {
				setMacState(MAC_STATE_SLEEP);
				toRadioLayer(createRadioCommand(SET_STATE, SLEEP));
			}
			break;
		}

		default:{
			trace() << "WARNING: received unexpected ACK in state " << stateDescr[macState];
			break;
		}
	}
}

// This function will initiate a transmission (or retransmission) attempt after a given delay
void Mac802154::attemptTransmission(const char * descr)
{
	cancelTimer(ATTEMPT_TX);
	trace() << "Attempt transmission, description: " << descr;

	if (currentFrameStart + CAPend > getClock()) {	// we could use a timer here
		// still in CAP period of the frame
		if (!enableCAP) {
			setMacState(MAC_STATE_IDLE);
			return;
		}
	} else if (requestGTS == 0 || GTSstart == 0) {
		// not in CAP period and not in GTS, no transmissions possible
		setMacState(MAC_STATE_IDLE);
		return;
	} else if (currentFrameStart + GTSstart > getClock() ||	//we could use a timer here too!
		   currentFrameStart + GTSend < getClock()) {
		// outside GTS, no transmissions possible
		setMacState(MAC_STATE_IDLE);
		return;
	}
	// if a packet already queued for transmission - check avaliable retries
	if (nextPacket) {
		if (nextPacketRetries <= 0) {
			if (nextPacket->getMac802154PacketType() == MAC_802154_DATA_PACKET) {
				if (nextPacket->getDstID() != BROADCAST_MAC_ADDRESS)
					packetBreak[nextPacketState]++;
				else
					packetBreak["Broadcast"]++;
			}
			cancelAndDelete(nextPacket);
			nextPacket = NULL;
		} else {
			trace() << "Continuing transmission of [" << nextPacket->getName() << 
				"], retries left: " << nextPacketRetries;
			initiateCSMACA();
			return;
		}
	}
	// if not associated to a PAN - cannot initiate transmissions other than association requests
	if (associatedPAN == -1)
		return;

	// extract a packet from transmission buffer
	if (TXBuffer.size() > 0) {
		nextPacket = check_and_cast <Mac802154Packet*>(TXBuffer.front());
		TXBuffer.pop();
		int txAddr = nextPacket->getDstID();
		nextPacketState = "";
		if (txAddr == BROADCAST_MAC_ADDRESS)
			initiateCSMACA(0, MAC_STATE_IN_TX, TX_TIME(nextPacket->getByteLength()));
		else
			initiateCSMACA(macMaxFrameRetries, MAC_STATE_WAIT_FOR_DATA_ACK,
				       ackWaitDuration + TX_TIME(nextPacket->getByteLength()));
		return;
	}

	setMacState(MAC_STATE_IDLE);
}

// initiate CSMA-CA algorithm, initialising retries, next state and response values
void Mac802154::initiateCSMACA(int retries, int nextState, simtime_t response)
{
	trace() << "Initiating new transmission of [" << nextPacket->getName() << 
		"], retries left: " << retries;
	nextPacketRetries = retries;
	nextMacState = nextState;
	nextPacketResponse = response;
	initiateCSMACA();
}

// initiate CSMA-CA algorithm
void Mac802154::initiateCSMACA()
{
	if (requestGTS && lockedGTS && currentFrameStart + GTSstart < getClock()
	    && currentFrameStart + GTSend > getClock()) {
		//we are in GTS, no need to run CSMA-CA - transmit right away
		trace() << "Transmitting packet in GTS";
		transmitNextPacket();
		return;
	}

	if (macState == MAC_STATE_CSMA_CA) {
		trace() << "WARNING: cannot initiate CSMA-CA algorithm while in MAC_STATE_CSMA_CA";
		return;
	}

	setMacState(MAC_STATE_CSMA_CA);
	NB = 0;
	CW = enableSlottedCSMA ? 2 : 1;
	BE = batteryLifeExtention ? (macMinBE < 2 ? macMinBE : 2) : macMinBE;
	continueCSMACA();
}

// continue CSMA-CA algorithm
void Mac802154::continueCSMACA()
{
	if (macState != MAC_STATE_CSMA_CA) {
		trace() << "WARNING: continueCSMACA called not in MAC_STATE_CSMA_CA";
		return;
	}
	//generate a random delay, multiply it by backoff period length
	int rnd = genk_intrand(1, (1 << BE) - 1) + 1;
	simtime_t CCAtime = rnd * (unitBackoffPeriod * symbolLen);

	//if using slotted CSMA - need to locate backoff period boundary
	if (enableSlottedCSMA) {
		simtime_t backoffBoundary = (ceil((getClock() - currentFrameStart) / (unitBackoffPeriod * symbolLen)) -
		     (getClock() - currentFrameStart) / (unitBackoffPeriod * symbolLen)) * (unitBackoffPeriod * symbolLen);
		CCAtime += backoffBoundary;
	}

	trace() << "Random backoff value: " << rnd << ", in " << CCAtime << " seconds";

	//set a timer to perform carrier sense after calculated time
	setTimer(PERFORM_CCA, CCAtime);
}

/* Transmit a packet by sending it to the radio */
void Mac802154::transmitNextPacket()
{
	//check if transmission is allowed given current time and tx time
	double txTime = TX_TIME(nextPacket->getByteLength());
	int allowTx = 1;
	if (currentFrameStart + CAPend > getClock() + txTime) {
		// still in CAP period of the frame
		if (!enableCAP && nextPacket->getMac802154PacketType() == MAC_802154_DATA_PACKET)
			allowTx = 0;
	} else if (requestGTS == 0 || GTSstart == 0) {
		// not in CAP period and not in GTS, no transmissions possible
		allowTx = 0;
	} else if (currentFrameStart + GTSstart > getClock() ||
		   currentFrameStart + GTSend < getClock() + txTime) {
		// outside GTS, no transmissions possible
		allowTx = 0;
	}

	if (!allowTx) {
		setMacState(MAC_STATE_IDLE);
		return;
	}
	//transmission is allowed, decrement retry counter and modify mac and radio states.
	nextPacketRetries--;
	if (nextPacketResponse > 0) {
		setMacState(nextMacState);
		setTimer(ATTEMPT_TX, nextPacketResponse);
	} else {
		setMacState(MAC_STATE_IN_TX);
		setTimer(ATTEMPT_TX, txTime);
	}

	toRadioLayer(nextPacket->dup());
	toRadioLayer(createRadioCommand(SET_STATE, TX));
}

/* Create a GTS request packet and schedule it for transmission */
void Mac802154::issueGTSrequest()
{
	if (nextPacket) {
		if (collectPacketState("NoPAN"))
			packetBreak[nextPacketState]++;
		cancelAndDelete(nextPacket);
	}

	nextPacket = new Mac802154Packet("GTS request", MAC_LAYER_PACKET);
	nextPacket->setPANid(associatedPAN);
	nextPacket->setDstID(associatedPAN);
	nextPacket->setMac802154PacketType(MAC_802154_GTS_REQUEST_PACKET);
	nextPacket->setSrcID(SELF_MAC_ADDRESS);
	nextPacket->setGTSlength(requestGTS);
	nextPacket->setByteLength(COMMAND_PKT_SIZE);
	initiateCSMACA(9999, MAC_STATE_WAIT_FOR_GTS, ackWaitDuration + TX_TIME(COMMAND_PKT_SIZE));
}

int Mac802154::collectPacketState(const char *s)
{
	if (!nextPacket)
		opp_error("MAC 802.15.4 internal error: collectPacketState called while nextPacket pointer is NULL");
	if (nextPacket->getMac802154PacketType() == MAC_802154_DATA_PACKET
	    && nextPacket->getDstID() != BROADCAST_MAC_ADDRESS) {
		if (nextPacketState.size()) {
			nextPacketState.append(",");
			nextPacketState.append(s);
		} else {
			nextPacketState = s;
		}
		//packetBreak[nextPacketState]++;
		return 1;
	}
	return 0;
}


/*
 * HalModule.cc
 *
 *  Created on: 12 Feb 2015
 *      Author: guust
 */

#include "HalModule.h"
#include "CRadio.h"
Define_Module(HalModule);


HalModule::HalModule()
{
}

HalModule::~HalModule()
{
}


void HalModule::startup()
{

	setTimer(0, 10.0+5*self);
	toRadio(createRadioCommand(SET_STATE, RX));
}

void HalModule::timerFiredCallback(int index)
{
	RadioPacket* packet = new RadioPacket("RadioPacket", RADIO_PACKET);
	packet->setBufferArraySize(10);
	for(int i = 0; i < 10; i++)
		packet->getBufferPtr()[i] = i+self;
	packet->setByteLength(10);
	toRadio(packet);
	toRadio(createRadioCommand(SET_STATE, TX));
}

void HalModule::finishSpecific()
{

}
void HalModule::fromRadio(RadioPacket* packet)
{
	std::cout << "Node: " << self << "Got Packet: ";
	for(int i = 0; i < packet->getBufferArraySize(); i++)
		std::cout << "[" << (int)packet->getBuffer(i) << "]";
	std::cout << std::endl;
}
void HalModule::handleRadioControlMessage(RadioControlMessage* msg)
{

}

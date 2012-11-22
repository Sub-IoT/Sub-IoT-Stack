/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#include "log.h"

#include "hal/uart.h"

u8 logCounter = 0;

// TODO only in debug mode?

void Log_PrintString(char* message, u8 length)
{
	Uart_TransmitData(0xDD);
	Uart_TransmitData(0x01);
	Uart_TransmitData(logCounter++);
	Uart_TransmitData(length);
	Uart_TransmitMessage((unsigned char*) message, length);
	Uart_TransmitData(0x00);
}

void Log_Packet(u8* packet) // TODO take direction param (tx/rx)?
{
	Uart_TransmitData(0xDD);
	Uart_TransmitData(0x00);
	Uart_TransmitData(logCounter++);
	Uart_TransmitData(packet[0]+1);
	Uart_TransmitMessage(packet, packet[0]+1);
	Uart_TransmitData(0x00);

}


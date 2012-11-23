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

void log_print_string(char* message, u8 length)
{
	uart_transmit_data(0xDD);
	uart_transmit_data(0x01);
	uart_transmit_data(logCounter++);
	uart_transmit_data(length);
	uart_transmit_message((unsigned char*) message, length);
	uart_transmit_data(0x00);
}

void log_packet(u8* packet) // TODO take direction param (tx/rx)?
{
	uart_transmit_data(0xDD);
	uart_transmit_data(0x00);
	uart_transmit_data(logCounter++);
	uart_transmit_data(packet[0]);
	uart_transmit_message(packet, packet[0]);
	uart_transmit_data(0x00);

}


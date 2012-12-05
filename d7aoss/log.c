/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#include "log.h"
#include "string.h"

#include "hal/uart.h"

// TODO only in debug mode?

void log_print_string(char* message)
{
	u8 len = strlen(message);
	uart_transmit_data(0xDD);
	uart_transmit_data(LOG_TYPE_STRING);
	uart_transmit_data(len);
	uart_transmit_message((unsigned char*) message, len);
}

void log_packet(u8* packet) // TODO take direction param (tx/rx)?
{
	uart_transmit_data(0xDD);
	uart_transmit_data(LOG_TYPE_PACKET);
	uart_transmit_data(packet[0]);
	uart_transmit_message(packet, packet[0]);
}

void log_phy_rx_res(phy_rx_res_t* res)
{
	u8 length = sizeof(phy_rx_res_t) - 2; // substract pointer length // TODO crossplatform?
	uart_transmit_data(0xDD);
	uart_transmit_data(LOG_TYPE_PHY_RX_RES);
	uart_transmit_data(length);
	uart_transmit_message((unsigned char*)res, length); // submit header without frame data
}


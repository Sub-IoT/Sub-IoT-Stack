/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef __UART_H__
#define _UART_H__

#define BAUDRATE 115200

void uart_init();

void uart_enable_interrupt();

void uart_transmit_data(unsigned char data);
void uart_transmit_message(unsigned char *data, unsigned char length);

unsigned char uart_tx_ready();

unsigned char uart_receive_data();

#endif // __UART_H__

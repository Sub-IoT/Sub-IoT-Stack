/*! \file uart.h
 *
 * \copyright (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.\n
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.\n
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * \author maarten.weyn@uantwerpen.be
 * \author glenn.ergeerts@uantwerpen.be
 *
 */

#ifndef __UART_H__
#define _UART_H__

//#define BAUDRATE 9600
#define BAUDRATE 115200

void uart_init();

void uart_enable_interrupt();

void uart_transmit_data(unsigned char data);
void uart_transmit_message(unsigned char *data, unsigned char length);

unsigned char uart_tx_ready();

unsigned char uart_receive_data();

#endif // __UART_H__

/*! \file uart.h
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

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
 * \author daniel.vandenakker@uantwerpen.be
 *
 */

#ifndef __UART_H__
#define __UART_H__

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/*! \brief Initialise the uart port
 *
 *  This function initialised the UART port of the selected MCU. This function is NOT part of the
 *  'user' API and should only be called from the initialisation code of the specific platform
 *
 */
void __uart_init();

/*! \brief Transmit a single byte over the UART.
 *
 *  if another UART transfer is still in progress ( uart_tx_ready() returns true)
 *  this function call blocks until that transfer is completed.
 *
 * \param data the character to transmit
 */
void uart_transmit_data(int8_t data);

/*! \brief Transmit an array of characters over the UART
 *
 * \param data 		pointer to the start of the data segment to send
 * \param length	the number of bytes to send
 */
void uart_transmit_message(void const *data, size_t length);

/*! \brief Check whether the UART is ready to send a single byte
 *
 * If this function returns true, a single byte can be sent without blocking.
 * If uart_tx_ready() returns false, the uart is still busy sending the previous byte.
 *
 * \returns bool	true is the UART subsystem is not busy false otherwise
 */
bool uart_tx_ready();

//these functions disabled for now (not used in dash7 code base) we'll add them when needed
//void uart_enable_interrupt();
//unsigned char uart_receive_data();

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __UART_H__

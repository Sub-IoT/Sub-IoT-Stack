/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \file hwuart.h
 * \addtogroup UART
 * \ingroup HAL
 * @{
 * \brief UART API
 * \author maarten.weyn@uantwerpen.be
 * \author glenn.ergeerts@uantwerpen.be
 * \author daniel.vandenakker@uantwerpen.be
 */

#ifndef __UART_H__
#define __UART_H__

#include "types.h"
#include "link_c.h"

/*! \brief Initialise the uart port
 *
 *  This function initialised the UART port of the selected MCU. This function is NOT part of the
 *  'user' API and should only be called from the initialisation code of the specific platform
 *
 */
__LINK_C void __uart_init();

/*! \brief Transmit a single byte over the UART.
 *
 *  if another UART transfer is still in progress ( uart_tx_ready() returns true)
 *  this function call blocks until that transfer is completed.
 *
 * \param data the character to transmit
 */
__LINK_C void uart_transmit_data(int8_t data);

/*! \brief Transmit an array of characters over the UART
 *
 * \param data 		pointer to the start of the data segment to send
 * \param length	the number of bytes to send
 */
__LINK_C void uart_transmit_message(void const *data, size_t length);

/*! \brief Transmit a terminated string over the UART
 *
 * \param string    pointer to the start of the terminated string to send
 */
__LINK_C void uart_transmit_string(char const *string);

/*! \brief Check whether the UART is ready to send a single byte
 *
 * If this function returns true, a single byte can be sent without blocking.
 * If uart_tx_ready() returns false, the uart is still busy sending the previous byte.
 *
 * \returns bool	true is the UART subsystem is not busy false otherwise
 */
__LINK_C bool uart_tx_ready();

/*! \brief RX callback function pointer prototype
 *  \param rx_data  the received byte
 */
typedef void (*uart_rx_inthandler_t)(char rx_data);

/*! \brief Configure the callback function to be called on UART RX interrupt
 *
 * Please note this callback is executed in interrupt process and thus should return asap.
 *
 * \param cb   function pointer to the callback function to be executed
 */
__LINK_C void uart_set_rx_interrupt_callback(uart_rx_inthandler_t cb);


/*! \brief Enable or disable UART RX interrupt from occuring
 *
 * Enabling the interrupt requires setting the callback function using uart_set_rx_interrupt_callback() beforehand.
 *
 * \param enabled   true if interrupt should enabled, false if interrupt should not be enabled
 * \return error_t	SUCCESS if the interrupt was enabled/disables successfully
 *			EOFF if the interrupt callback has not yet been configured while enabling
 */
__LINK_C error_t uart_rx_interrupt_enable(bool enabled);

#endif // __UART_H__

/** @}*/

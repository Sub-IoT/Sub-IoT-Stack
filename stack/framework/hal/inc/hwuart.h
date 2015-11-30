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
 * \author contact@christophe.vg
 */


#ifndef __UART_H__
#define __UART_H__

#include <stdlib.h>
#include "types.h"

typedef void (*uart_rx_inthandler_t)(uint8_t byte);

void    __uart_init();
void    __uart_init_port(uint8_t idx);
void    uart_send_byte(uint8_t idx, uint8_t data);
void    uart_send_bytes(uint8_t idx, void const *data, size_t length);
void    uart_send_string(uint8_t idx, const char *string);
error_t uart_rx_interrupt_enable(uint8_t idx);
void    uart_rx_interrupt_disable(uint8_t idx);
void    uart_set_rx_interrupt_callback(uint8_t idx, uart_rx_inthandler_t rx_handler);


#endif // __UART_H__

/** @}*/

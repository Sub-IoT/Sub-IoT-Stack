/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
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
#include "link_c.h"

// expose uart_handle with unknown internals
typedef struct uart_handle uart_handle_t;

// callback handler for received byte
typedef void (*uart_rx_inthandler_t)(uart_handle_t* uart, uint8_t byte);

// callback handler for transmit complete (only used when DMA is used)
typedef void (*uart_tx_inthandler_t)(uart_handle_t* uart);

typedef enum
{
  UART_NO_ERROR,
  UART_NOISE_ERROR,
  UART_FRAMING_ERROR,
  UART_OVERRUN_ERROR,
  UART_PARITY_ERROR,
} uart_error_t;

typedef void (*uart_error_handler_t)(uart_handle_t* uart, uart_error_t error);

__LINK_C uart_handle_t* uart_init(uint8_t port_idx, uint32_t baudrate, uint8_t pins);
__LINK_C uart_handle_t* uart_get_handle(uint8_t port_idx);
__LINK_C bool           uart_is_enabled(uart_handle_t* uart);
__LINK_C bool           uart_disable(uart_handle_t* uart);
__LINK_C bool           uart_get_rx_port_state(uart_handle_t* uart);
__LINK_C void           uart_pull_down_rx(uart_handle_t* uart);
__LINK_C bool           uart_enable(uart_handle_t* uart);

__LINK_C void           uart_send_byte(uart_handle_t* uart, uint8_t data);
__LINK_C void           uart_send_bytes(uart_handle_t* uart, void const *data, size_t length);
__LINK_C void           uart_send_string(uart_handle_t* uart, const char *string);
__LINK_C void           uart_send_bytes_via_DMA(uart_handle_t* uart, void const *data, size_t length, uint8_t dma_channel_idx);

__LINK_C void           uart_start_read_bytes_via_DMA(uart_handle_t* uart, void *data, size_t length, uint8_t dma_channel_idx);
__LINK_C size_t         uart_stop_read_bytes_via_DMA(uart_handle_t* uart);

__LINK_C error_t        uart_rx_interrupt_enable(uart_handle_t* uart);
__LINK_C void           uart_rx_interrupt_disable(uart_handle_t* uart);

__LINK_C error_t        uart_tx_interrupt_enable(uart_handle_t* uart);
__LINK_C void           uart_tx_interrupt_disable(uart_handle_t* uart);

__LINK_C void           uart_set_rx_interrupt_callback(uart_handle_t* uart,
                                                       uart_rx_inthandler_t rx_handler);
__LINK_C void           uart_set_tx_interrupt_callback(uart_handle_t* uart,
                                                       uart_tx_inthandler_t tx_handler);

__LINK_C void           cdc_set_rx_interrupt_callback(uart_handle_t* uart,uart_rx_inthandler_t rx_handler);

__LINK_C void           uart_set_error_callback(uart_handle_t* uart, uart_error_handler_t error_handler);

#endif

/** @}*/

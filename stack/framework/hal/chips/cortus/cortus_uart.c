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
#include <string.h>

#include "hwuart.h"
#include "errors.h"

#include "machine/sfradr.h"
#include "machine/ic.h"
#include "machine/uart.h"

#if defined(FRAMEWORK_LOG_ENABLED)
#include <stdio.h>
#endif

struct uart_handle {
	Uart* uart_sfradr;
};

static uart_handle_t handle;
static uart_rx_inthandler_t rx_inthandler = 0x0;

uart_handle_t* uart_init(uint8_t idx, uint32_t baudrate, uint8_t pins) {

	// Initialize the UART to reasonable values
	uart1->config = 0; // 8 bits, no parity, 1 stop bit, flow control disabled
	uart1->selclk = 0; // 0:50MHz, 1:25MHz, 2:12.5MHZ, 3:3.125MHz
	uart1->divider = 8*(50000000/baudrate);

	handle.uart_sfradr = uart1;
	return &handle;
}

bool uart_enable(uart_handle_t* uart_handle) {
	volatile int dump;

	/* Flush the RX buffer. */
	while(uart1->rx_status)
		dump = uart1->rx_data;

	uart1->rx_mask = 1;  /* start Rx interrupt */

	return true;
}

bool uart_disable(uart_handle_t* uart) {

	uart1->rx_mask = 0; /* stop Rx interrupt */
	uart1->tx_mask = 0; /* stop Tx interrupt */

	return true;
}

void uart_set_rx_interrupt_callback(uart_handle_t* uart,
									uart_rx_inthandler_t rx_handler)
{
	rx_inthandler = rx_handler;
}

static void uart_outch_raw (int c)
{
	while (! (uart1->tx_status & 0x01)) ;
	uart1->tx_data = c;
}

void uart_send_byte(uart_handle_t* uart, uint8_t data) {

	if (data == '\n')
		uart_outch_raw ('\r');

	uart_outch_raw (data);
}

void uart_send_bytes(uart_handle_t* uart, void const *data, size_t length) {

	for(uint8_t i=0; i<length; i++)	{
		uart_send_byte(uart, ((uint8_t const*)data)[i]);
	}
}

void uart_send_string(uart_handle_t* uart, const char *string) {
	uart_send_bytes(uart, string, strnlen(string, 100));
}

error_t uart_rx_interrupt_enable(uart_handle_t* uart) {

	irq[IRQ_UART1_RX].ien = 1;
	uart1->rx_mask = 1;  /* start Rx interrupt */
	return SUCCESS;
}

void uart_rx_interrupt_disable(uart_handle_t* uart) {

	uart1->rx_mask = 0; //stop Rx interrupt
}

void interrupt_handler(IRQ_UART1_RX)
{
	volatile unsigned ch;

	while(((uart1->rx_status)&(0x01)) != 0){
		ch = uart1->rx_data;
		if (rx_inthandler != 0x0)
			rx_inthandler(ch);
	}
}

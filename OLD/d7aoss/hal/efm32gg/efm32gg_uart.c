/*! \file efm32gg_system.c
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
 *  \author glenn.ergeerts@uantwerpen.be
 *
 */

#include <em_usart.h>
#include <em_cmu.h>
#include <em_gpio.h>

#include "uart.h"

#define UART_CHANNEL        UART0
#define UART_BAUDRATE       BAUDRATE
#define UART_CLOCK          cmuClock_UART0
#define UART_ROUTE_LOCATION UART_ROUTE_LOCATION_LOC1

#define UART_PORT           gpioPortE   // UART0 location #1: PE0 and PE1
#define UART_PIN_TX         0           // PE0
#define UART_PIN_RX         1           // PE1

void uart_init()
{
    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(UART_CLOCK, true);

    GPIO_PinModeSet(UART_PORT, UART_PIN_TX, gpioModePushPullDrive,  1); // Configure UART TX pin as digital output, initialize high since UART TX idles high (otherwise glitches can occur)
    GPIO_PinModeSet(UART_PORT, UART_PIN_RX, gpioModeInput,          0);    // Configure UART RX pin as input (no filter)

    USART_InitAsync_TypeDef uartInit =
    {
      .enable       = usartDisable,   // Wait to enable the transmitter and receiver
      .refFreq      = 0,              // Setting refFreq to 0 will invoke the CMU_ClockFreqGet() function and measure the HFPER clock
      .baudrate     = UART_BAUDRATE,  // Desired baud rate
      .oversampling = usartOVS16,     // Set oversampling value to x16
      .databits     = usartDatabits8, // 8 data bits
      .parity       = usartNoParity,  // No parity bits
      .stopbits     = usartStopbits1, // 1 stop bit
      .mvdis        = false,          // Use majority voting
      .prsRxEnable  = false,          // Not using PRS input
      .prsRxCh      = usartPrsRxCh0,  // Doesn't matter which channel we select
    };

    USART_InitAsync(UART_CHANNEL, &uartInit);
    UART_CHANNEL->ROUTE = UART_ROUTE_RXPEN | UART_ROUTE_TXPEN | UART_ROUTE_LOCATION; // Clear RX/TX buffers and shift regs, enable transmitter and receiver pins

    USART_IntClear(UART_CHANNEL, _UART_IF_MASK);
    NVIC_ClearPendingIRQ(UART1_RX_IRQn);
    NVIC_ClearPendingIRQ(UART1_TX_IRQn);

    USART_Enable(UART_CHANNEL, usartEnable);
}

void uart_transmit_data(unsigned char data)
{
    while(!(UART_CHANNEL->STATUS & (1 << 6))) {}; // wait for TX buffer to empty
    UART_CHANNEL->TXDATA = data;
}

// TODO move to generic uart, not HAL implementation specific?
void uart_transmit_message(unsigned char *data, unsigned char length)
{
    unsigned char i=0;
    for (; i<length; i++)
    {
        uart_transmit_data(data[i]);
    }
}

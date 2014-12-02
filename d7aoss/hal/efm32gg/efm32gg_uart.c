/*! \file efm32gg_system.c
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

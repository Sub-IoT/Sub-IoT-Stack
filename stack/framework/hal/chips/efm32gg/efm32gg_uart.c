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

/*! \file efm32gg_uart.c
 *
 *  \author jeremie@wizzilab.com
 *  \author maarten.weyn@uantwerpen.be
 *
 */

#include <em_usart.h>
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_usbd.h>
#include "hwgpio.h"
#include "hwuart.h"
#include <assert.h>
//contains the wiring for the uart
//#include "platform.h"
#include "em_gpio.h"


void __uart_init()
{
    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(UART_CLOCK, true);

    //GPIO_PinModeSet(UART_PORT, UART_PIN_TX, gpioModePushPullDrive,  1); // Configure UART TX pin as digital output, initialize high since UART TX idles high (otherwise glitches can occur)
    //GPIO_PinModeSet(UART_PORT, UART_PIN_RX, gpioModeInput,          0);    // Configure UART RX pin as input (no filter)
    //edit: do this via the hw_gpio_configure_pin interface to signal that the pins are in use
    //note: normally this should be done in the platform-specific initialisation code BUT, since this is a driver for a device (uart) that is
    //an integral part of the MCU we are certain this code will NOT be used in combination with a different MCU so we can do this here
    error_t err;
    err = hw_gpio_configure_pin(UART_PIN_TX, false, gpioModePushPullDrive, 1); assert(err == SUCCESS);// Configure UART TX pin as digital output, initialize high since UART TX idles high (otherwise glitches can occur)
    err = hw_gpio_configure_pin(UART_PIN_RX, false, gpioModeInput, 0); assert(err == SUCCESS);    // Configure UART RX pin as input (no filter)


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

void uart_transmit_data(int8_t data)
{
#ifdef PLATFORM_USE_USB_CDC
		//while(USBD_EpIsBusy(0x81)){};
		uint32_t tempData = data;
		USBD_Write( 0x81, (void*) &tempData, 1, NULL);
#else
		while(!(UART_CHANNEL->STATUS & (1 << 6))) {}; // wait for TX buffer to empty
		UART_CHANNEL->TXDATA = data;
#endif
}

void uart_transmit_message(void const *data, size_t length)
{
#ifdef PLATFORM_USE_USB_CDC
		USBD_Write( 0x81, (void*) data, length, NULL);
#else
		unsigned char i=0;
		for (; i<length; i++)
		{
			uart_transmit_data(((char const*)data)[i]);
		}
#endif
}

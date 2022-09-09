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

/*! \file efm32hg_uart.c
*
*  \author jeremie@wizzilab.com
*  \author maarten.weyn@uantwerpen.be
*  \author contact@christophe.vg
*/

#include <string.h>
#include "em_usart.h"
#include "em_cmu.h"
#include "em_gpio.h"
//#include "em_usbd.h"
#include "em_gpio.h"
#include "efm32lg_uart.h"

#include "hwgpio.h"
#include "hwuart.h"
#include <assert.h>
#include "hwsystem.h"

#include "platform.h"

#define UARTS     5   // 2 UARTs + 3 USARTs
#define LOCATIONS 6

typedef struct {
 IRQn_Type  tx;
 IRQn_Type  rx;
} uart_irq_t;

typedef struct {
 uint32_t location;
 pin_id_t tx;
 pin_id_t rx;
} uart_pins_t;

#define UNDEFINED_LOCATION {                      \
  .location = 0,                                  \
  .tx       = PIN(0, 0),   \
  .rx       = PIN(0, 0)    \
}

// configuration of uart/location mapping to tx and rx pins
// TODO to be completed with all documented locations
// TODO move to platform's port.h
static uart_pins_t location[UARTS][LOCATIONS] = {
  {
    // 0: UART 0
    {
      .location = UART_ROUTE_LOCATION_LOC0,
      .tx       = PIN(GPIO_PORTF, 6),
      .rx       = PIN(GPIO_PORTF, 7)
    },
    {
      .location = UART_ROUTE_LOCATION_LOC1,
      .tx       = PIN(GPIO_PORTE, 0),
      .rx       = PIN(GPIO_PORTE, 1)
    },
    {
      .location = UART_ROUTE_LOCATION_LOC2,
      .tx       = PIN(GPIO_PORTA, 3),
      .rx       = PIN(GPIO_PORTA, 4)
    },
    // no LOCATION 3
    UNDEFINED_LOCATION
  },
  {
    // 1: UART 1
    // no LOCATION 0
    {
      .location = UART_ROUTE_LOCATION_LOC0,
      .tx       = PIN(0, 0),
      .rx       = PIN(0, 0)
    },
    {
      .location = UART_ROUTE_LOCATION_LOC1,
      .tx       = PIN(GPIO_PORTF, 11),
      .rx       = PIN(GPIO_PORTF, 11)
    },
    {
      .location = UART_ROUTE_LOCATION_LOC2,
      .tx       = PIN(GPIO_PORTB, 9),
      .rx       = PIN(GPIO_PORTF, 10)
    },
    {
      .location = UART_ROUTE_LOCATION_LOC3,
      .tx       = PIN(GPIO_PORTE, 2),
      .rx       = PIN(GPIO_PORTE, 3)
    }
  },
  {
    // 2: USART 1
      // no LOCATION 0
      UNDEFINED_LOCATION,
    {
      .location = USART_ROUTE_LOCATION_LOC1,
      .tx       = PIN(GPIO_PORTD, 0),
      .rx       = PIN(GPIO_PORTD, 1)
    },
    {
      .location = USART_ROUTE_LOCATION_LOC2,
      .tx       = PIN(GPIO_PORTD, 7),
      .rx       = PIN(GPIO_PORTD, 6)
    },
    // no LOCATION 3
    UNDEFINED_LOCATION
  },
  {
    // 3: USART 2
  // no LOCATION 0
  UNDEFINED_LOCATION,
    {
      .location = USART_ROUTE_LOCATION_LOC1,
      .tx       = PIN(GPIO_PORTB, 3),
      .rx       = PIN(GPIO_PORTB, 4)
    },
      //no LOCATION 2
       UNDEFINED_LOCATION,
    // no LOCATION 3
    UNDEFINED_LOCATION
  }
};

// references to registered handlers
static uart_rx_inthandler_t handler[UARTS];

// private definition of the UART handle, passed around publicly as a pointer
struct uart_handle {
 uint8_t              idx;
 USART_TypeDef*       channel;
 CMU_Clock_TypeDef    clock;
 uart_irq_t           irq;
 uart_pins_t*         pins;
 uint32_t             baudrate;
};

// private storage of handles, pointers to these records are passed around
static uart_handle_t handleuart[UARTS] = {
 {
   .idx     = 0,
   .channel = UART0,
   .clock   = cmuClock_UART0,
   .irq     = { .tx = UART0_TX_IRQn,  .rx = UART0_RX_IRQn  }
 },
 {
   .idx     = 1,
   .channel = UART1,
   .clock   = cmuClock_UART1,
   .irq     = { .tx = UART1_TX_IRQn,  .rx = UART1_RX_IRQn  }
 },
 {
   .idx     = 2,
   .channel = USART0,
   .clock   = cmuClock_USART0,
   .irq     = { .tx = USART0_TX_IRQn, .rx = USART0_RX_IRQn }
 },
 {
   .idx     = 3,
   .channel = USART1,
   .clock   = cmuClock_USART1,
   .irq     = { .tx = USART1_TX_IRQn, .rx = USART1_RX_IRQn }
 },
 {
   .idx     = 4,
   .channel = USART2,
   .clock   = cmuClock_USART2,
   .irq     = { .tx = USART2_TX_IRQn, .rx = USART2_RX_IRQn }
 }
};

uart_handle_t* uart_init(uint8_t idx, uint32_t baudrate, uint8_t pins) {
 // configure pins
 handleuart[idx].pins     = &location[idx][pins];
 handleuart[idx].baudrate = baudrate;
  
 // configure UART TX pin as digital output
 hw_gpio_configure_pin(handleuart[idx].pins->tx, false, gpioModePushPullDrive, 0);
 // configure UART RX pin as input (no filter)
 hw_gpio_configure_pin(handleuart[idx].pins->rx, false, gpioModeInput, 0);

 return &handleuart[idx];
}

bool uart_enable(uart_handle_t* uart) {
 // CMU_ClockEnable(cmuClock_GPIO,    true); // TODO future use: hw_gpio_enable
 CMU_ClockEnable(uart->clock, true);

 USART_InitAsync_TypeDef uartInit = {
   .enable       = usartDisable,   // wait to enable the transceiver
   .refFreq      = 0,              // setting refFreq to 0 will invoke the
                                   // CMU_ClockFreqGet() function and measure
                                   // the HFPER clock
   .baudrate     = uart->baudrate, // desired baud rate
   .oversampling = usartOVS16,     // set oversampling value to x16
   .databits     = usartDatabits8, // 8 data bits
   .parity       = usartNoParity,  // no parity bits
   .stopbits     = usartStopbits1, // 1 stop bit
   .mvdis        = false,          // use majority voting
   .prsRxEnable  = false,          // not using PRS input
   .prsRxCh      = usartPrsRxCh0,  // doesn't matter which channel we select
 };

 USART_InitAsync(uart->channel, &uartInit);
  
 // clear RX/TX buffers and shift regs, enable transmitter and receiver pins
 uart->channel->ROUTE = UART_ROUTE_RXPEN
                      | UART_ROUTE_TXPEN
                      | uart->pins->location;
 USART_IntClear(uart->channel, _UART_IF_MASK);
 NVIC_ClearPendingIRQ(uart->irq.rx);
 NVIC_ClearPendingIRQ(uart->irq.tx);

 USART_Enable(uart->channel, usartEnable);

 return true;
}

bool uart_disable(uart_handle_t* uart) {
 // reset route to make sure that TX pin will become low after disable
 uart->channel->ROUTE = _UART_ROUTE_RESETVALUE;

 USART_Enable(uart->channel, usartDisable);
 CMU_ClockEnable(uart->clock, false);
 // CMU_ClockEnable(cmuClock_GPIO, false); // TODO future use: hw_gpio_disable

 return true;
}

void uart_set_rx_interrupt_callback(uart_handle_t* uart,
                                   uart_rx_inthandler_t rx_handler)
{
 handler[uart->idx] = rx_handler;
}

void uart_send_byte(uart_handle_t* uart, uint8_t data) {
#ifdef PLATFORM_USE_USB_CDC
		uint16_t timeout = 0;
		while(USBD_EpIsBusy(0x81) && timeout < 100){
			timeout++;
			hw_busy_wait(1000);
		};
		uint32_t tempData = data;
		int ret = USBD_Write( 0x81, (void*) &tempData, 1, NULL);
#else
 while(!(uart->channel->STATUS & (1 << 6))); // wait for TX buffer to empty
	uart->channel->TXDATA = data;
#endif
}

void uart_send_bytes(uart_handle_t* uart, void const *data, size_t length) {
#ifdef PLATFORM_USE_USB_CDC
   // print misaliged bytes first as individual bytes.
		int8_t* tempData = (int8_t*) data;
		while(((uint32_t)tempData & 3) && (length > 0)) {
			uart_send_byte(uart, tempData[0]);
			tempData++;
			length--;
		}

		if (length > 0)
		{
			uint16_t timeout = 0;
			while(USBD_EpIsBusy(0x81) && timeout < 100){
				timeout++;
				hw_busy_wait(1000);
			};
			int ret = USBD_Write( 0x81, (void*) tempData, length, NULL);
		}
#else
	for(uint8_t i=0; i<length; i++)	{
		uart_send_byte(uart, ((uint8_t const*)data)[i]);
	}
#endif
}

void uart_send_string(uart_handle_t* uart, const char *string) {
 int length = strnlen(string, 100);
 uart_send_bytes(uart, string, strnlen(string, 100));
}

error_t uart_rx_interrupt_enable(uart_handle_t* uart) {
 if(handler[uart->idx] == NULL) { return EOFF; }
 USART_IntClear(uart->channel, _UART_IF_MASK);
 USART_IntEnable(uart->channel, UART_IF_RXDATAV);
 NVIC_ClearPendingIRQ(uart->irq.tx);
 NVIC_ClearPendingIRQ(uart->irq.rx);
 NVIC_EnableIRQ(uart->irq.rx);
 return SUCCESS;
}

void uart_rx_interrupt_disable(uart_handle_t* uart) {
 USART_IntClear(uart->channel, _UART_IF_MASK);
 USART_IntDisable(uart->channel, UART_IF_RXDATAV);
 NVIC_ClearPendingIRQ(uart->irq.rx);
 NVIC_ClearPendingIRQ(uart->irq.tx);
 NVIC_DisableIRQ(uart->irq.rx);
}

void UART0_RX_IRQHandler(void) {
 if(handleuart[0].channel->STATUS & UART_STATUS_RXDATAV) {
   handler[0](&handle[0], USART_Rx(handleuart[0].channel));
   USART_IntClear(handleuart[0].channel, UART_IF_RXDATAV);
 }
}

void UART1_RX_IRQHandler(void) {
 if(handleuart[1].channel->STATUS & UART_STATUS_RXDATAV) {
   handler[1](&handle[1], USART_Rx(handleuart[1].channel));
   USART_IntClear(handleuart[1].channel, UART_IF_RXDATAV);
 }
}

void USART0_RX_IRQHandler(void) {
 if(handleuart[2].channel->STATUS & UART_STATUS_RXDATAV) {
   handler[2](&handle[2], USART_Rx(handleuart[2].channel));
   USART_IntClear(handleuart[2].channel, UART_IF_RXDATAV);
 }
}

void USART1_RX_IRQHandler(void) {
 if(handleuart[3].channel->STATUS & UART_STATUS_RXDATAV) {
   handler[3](&handle[3], USART_Rx(handleuart[3].channel));
   USART_IntClear(handleuart[3].channel, UART_IF_RXDATAV);
 }
}

void USART2_RX_IRQHandler(void) {
 if(handleuart[4].channel->STATUS & UART_STATUS_RXDATAV) {
   handler[4](&handle[4], USART_Rx(handleuart[4].channel));
   USART_IntClear(handleuart[4].channel, UART_IF_RXDATAV);
 }
}

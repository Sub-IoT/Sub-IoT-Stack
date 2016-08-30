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
 *  \author contact@christophe.vg
 */

#include <em_usart.h>
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_usbd.h>
#include "hwgpio.h"
#include "hwuart.h"
#include <assert.h>
#include "em_gpio.h"
#include "hwsystem.h"

#include "ezr32lg_pins.h"

#include "platform.h"

#define UARTS     4   // 2 UARTs + 3 USARTs
#define LOCATIONS 4

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
  .tx       = { .port = 0,         .pin =  0 },   \
  .rx       = { .port = 0,         .pin =  0 }    \
}

// configuration of uart/location mapping to tx and rx pins
// TODO to be completed with all documented locations
static uart_pins_t location[UARTS][LOCATIONS] = {
  {
    // 0: UART 0
    {
      .location = UART_ROUTE_LOCATION_LOC0,
      .tx       = { .port = gpioPortF, .pin =  6 },
      .rx       = { .port = gpioPortF, .pin =  7 }
    },
    {
      .location = UART_ROUTE_LOCATION_LOC1,
      .tx       = { .port = gpioPortE, .pin =  0 },
      .rx       = { .port = gpioPortE, .pin =  1 }
    },
    {
      .location = UART_ROUTE_LOCATION_LOC2,
      .tx       = { .port = gpioPortA, .pin =  3 },
      .rx       = { .port = gpioPortA, .pin =  4 }
    },
    // no LOCATION 3
    UNDEFINED_LOCATION
  },
  {
    // 1: UART 1
    // no LOCATION 0
    {
      .location = UART_ROUTE_LOCATION_LOC0,
      .tx       = { .port = 0,         .pin =  0 },
      .rx       = { .port = 0,         .pin =  0 }
    },
    {
      .location = UART_ROUTE_LOCATION_LOC1,
      .tx       = { .port = gpioPortF, .pin = 10 },
      .rx       = { .port = gpioPortF, .pin = 11 }
    },
    {
      .location = UART_ROUTE_LOCATION_LOC2,
      .tx       = { .port = gpioPortB, .pin =  9 },
      .rx       = { .port = gpioPortB, .pin = 10 }
    },
    {
      .location = UART_ROUTE_LOCATION_LOC3,
      .tx       = { .port = gpioPortE, .pin =  2 },
      .rx       = { .port = gpioPortE, .pin =  3 }
    }
  },
  {
    // 2: USART 1
		  // no LOCATION 0
		  UNDEFINED_LOCATION,
    {
      .location = USART_ROUTE_LOCATION_LOC1,
      .tx       = { .port = gpioPortD, .pin =  0 },
      .rx       = { .port = gpioPortD, .pin =  1 }
    },
    {
      .location = USART_ROUTE_LOCATION_LOC2,
      .tx       = { .port = gpioPortD, .pin =  7 },
      .rx       = { .port = gpioPortD, .pin =  6 }
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
      .tx       = { .port = gpioPortB, .pin =  3 },
      .rx       = { .port = gpioPortB, .pin =  4 }
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
};

// private storage of handles, pointers to these records are passed around
static uart_handle_t handle[UARTS] = {
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
    .channel = USART1,
    .clock   = cmuClock_USART1,
    .irq     = { .tx = USART1_TX_IRQn, .rx = USART1_RX_IRQn }
  },
  {
    .idx     = 3,
    .channel = USART2,
    .clock   = cmuClock_USART2,
    .irq     = { .tx = USART2_TX_IRQn, .rx = USART2_RX_IRQn }
  }
};

uart_handle_t* uart_init(uint8_t idx, uint32_t baudrate, uint8_t pins) {
  CMU_ClockEnable(cmuClock_GPIO, true);

  handle[idx].pins = &location[idx][pins];

  CMU_ClockEnable(handle[idx].clock, true);

  // configure UART TX pin as digital output, initialize high since UART TX
  // idles high (otherwise glitches can occur)
  assert(hw_gpio_configure_pin(handle[idx].pins->tx, false, gpioModePushPullDrive, 1) == SUCCESS);
  // configure UART RX pin as input (no filter)
  assert(hw_gpio_configure_pin(handle[idx].pins->rx, false, gpioModeInput, 0) == SUCCESS);

  USART_InitAsync_TypeDef uartInit = {
    .enable       = usartDisable,   // wait to enable the transceiver
    .refFreq      = 0,              // setting refFreq to 0 will invoke the
                                    // CMU_ClockFreqGet() function and measure
                                    // the HFPER clock
    .baudrate     = baudrate,       // desired baud rate
    .oversampling = usartOVS16,     // set oversampling value to x16
    .databits     = usartDatabits8, // 8 data bits
    .parity       = usartNoParity,  // no parity bits
    .stopbits     = usartStopbits1, // 1 stop bit
    .mvdis        = false,          // use majority voting
    .prsRxEnable  = false,          // not using PRS input
    .prsRxCh      = usartPrsRxCh0,  // doesn't matter which channel we select
  };

  USART_InitAsync(handle[idx].channel, &uartInit);
  // clear RX/TX buffers and shift regs, enable transmitter and receiver pins
  handle[idx].channel->ROUTE = UART_ROUTE_RXPEN | UART_ROUTE_TXPEN | handle[idx].pins->location;
  USART_IntClear(handle[idx].channel, _UART_IF_MASK);
  NVIC_ClearPendingIRQ(handle[idx].irq.rx);
  NVIC_ClearPendingIRQ(handle[idx].irq.tx);

  USART_Enable(handle[idx].channel, usartEnable);

  return &handle[idx];
}

bool uart_disable(uart_handle_t* uart) {
  // reset route to make sure that TX pin will become low after disable
  uart->channel->ROUTE = _UART_ROUTE_RESETVALUE;

  USART_Enable(uart->channel, usartDisable);
  CMU_ClockEnable(uart->clock, false);

  return true;
}

bool uart_enable(uart_handle_t* uart) {
  uart->channel->ROUTE = UART_ROUTE_RXPEN | UART_ROUTE_TXPEN | uart->pins->location;

  USART_Enable(uart->channel, usartEnable);
  CMU_ClockEnable(uart->clock, true);

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
      //hw_busy_wait(1000); // TODO validate, commented for now
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
  for(size_t i=0; i<length; i++)	{
		uart_send_byte(uart, ((uint8_t const*)data)[i]);
	}
#endif
}

void uart_send_string(uart_handle_t* uart, const char *string) {
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
  if(handle[0].channel->STATUS & UART_STATUS_RXDATAV) {
    handler[0](USART_Rx(handle[0].channel));
    USART_IntClear(handle[0].channel, UART_IF_RXDATAV);
  }
}

void UART1_RX_IRQHandler(void) {
  if(handle[1].channel->STATUS & UART_STATUS_RXDATAV) {
    handler[1](USART_Rx(handle[1].channel));
    USART_IntClear(handle[1].channel, UART_IF_RXDATAV);
  }
}

void USART1_RX_IRQHandler(void) {
  if(handle[2].channel->STATUS & USART_STATUS_RXDATAV) {
    handler[2](USART_Rx(handle[2].channel));
    USART_IntClear(handle[2].channel, USART_IF_RXDATAV);
  }
}

void USART2_RX_IRQHandler(void) {
  if(handle[3].channel->STATUS & USART_STATUS_RXDATAV) {
    handler[3](USART_Rx(handle[3].channel));
    USART_IntClear(handle[3].channel, USART_IF_RXDATAV);
  }
}

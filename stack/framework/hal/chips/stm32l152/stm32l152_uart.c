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

/*! \file stm32l152_uart.c
 *
 */

#include "hwgpio.h"
#include "hwuart.h"
#include <assert.h>
#include "hwsystem.h"

#include "stm32l152_pins.h"
#include "stm32l1xx_hal.h"

#include "platform.h"
#include "string.h"

#define UARTS     6   // Dummy + 2 UARTs + 3 USARTs
#define LOCATIONS 1

 typedef struct {
   IRQn_Type  tx;
   IRQn_Type  rx;
 } uart_irq_t;

 typedef struct {
   pin_id_t tx;
   pin_id_t rx;
 } uart_pins_t;

 #define UNDEFINED_LOCATION {                      \
   .tx       = { .port = 0xFF,         .pin =  0xFF },   \
   .rx       = { .port = 0xFF,         .pin =  0xFF }    \
 }

 // configuration of uart/location mapping to tx and rx pins
 // TODO to be completed with all documented locations
 static uart_pins_t location[UARTS][LOCATIONS] = {
 {
		   // DUMMY
		   UNDEFINED_LOCATION
	},
	{
		   // USART 1
		   UNDEFINED_LOCATION
   },
   {
		   // USART 2
		   {
		      .tx       = { .port = 0, .pin =  2 },
		      .rx       = { .port = 0, .pin =  3 }
		   },
   },
   {
	   	   // USART 3
	   	   UNDEFINED_LOCATION
   },
   {
	   	   // UART 1
	   	   UNDEFINED_LOCATION
   },
   {
	   	   // UART 2
	   	   UNDEFINED_LOCATION
   }
  };

// references to registered handlers
static uart_rx_inthandler_t handler[UARTS];

 // private definition of the UART handle, passed around publicly as a pointer
 struct uart_handle {
   uint8_t              idx;
   uint8_t 	mapping;
   UART_HandleTypeDef 	uart;
   uart_irq_t           irq;
   uart_pins_t*         pins;
   uint32_t baudrate;
 };

 // private storage of handles, pointers to these records are passed around
 static uart_handle_t handle[UARTS] = {
   {
     .idx     = 0,
     .mapping = 0,
     .irq     = { .tx = 0,  .rx = 0  }
   },
   {
		   .idx     = 1,
		   .mapping = GPIO_AF7_USART1,
		   .irq     = { .tx = 0,  .rx = 0  }
   },
   {
		   .idx     = 2,
		   .mapping = GPIO_AF7_USART2,
		   .uart.Instance  = USART2,
		   .irq     = { .tx = 0, .rx = 0 }
   },
   {
		   .idx     = 3,
		   .mapping = GPIO_AF7_USART3,
		   .irq     = { .tx = 0, .rx = 0 }
   },
   {
     .idx     = 4,
	   .mapping = GPIO_AF8_UART4,
     .irq     = { .tx = 0, .rx = 0 }
   },
   {
     .idx     = 5,
	   .mapping = GPIO_AF8_UART5,
     .irq     = { .tx = 0, .rx = 0 }
   }
 };

uart_handle_t* uart_init(uint8_t idx, uint32_t baudrate, uint8_t pins) {
   handle[idx].pins = &location[idx][pins];
   handle[idx].baudrate = baudrate;

   GPIO_InitTypeDef GPIO_InitStruct;
   GPIO_InitStruct.Pin = (1<<handle[idx].pins->tx.pin) | (1<<handle[idx].pins->rx.pin) ;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_PULLUP;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate = handle[idx].mapping;
   hw_gpio_configure_pin_stm(handle[idx].pins->tx, &GPIO_InitStruct);

   return &handle[idx];
}

bool uart_enable(uart_handle_t* uart) {

	switch (uart->idx)
	{
	case 2:
		__HAL_RCC_USART2_CLK_ENABLE();
		break;
	default:
		assert("not defined");
		return false;
	}

   uart->uart.Init.BaudRate = uart->baudrate;
   uart->uart.Init.WordLength = UART_WORDLENGTH_8B;
   uart->uart.Init.StopBits = UART_STOPBITS_1;
   uart->uart.Init.Parity = UART_PARITY_NONE;
   uart->uart.Init.Mode = UART_MODE_TX_RX;
   uart->uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
   uart->uart.Init.OverSampling = UART_OVERSAMPLING_16;
   if (HAL_UART_Init(&(uart->uart)) != HAL_OK)
   {
     assert ("cannot init");
     return false;
   }

   HAL_NVIC_SetPriority(uart->irq.rx, 0, 0);
  
   return true;
 }

 void uart_set_rx_interrupt_callback(uart_handle_t* uart,
                                     uart_rx_inthandler_t rx_handler)
 {
   handler[uart->idx] = rx_handler;
 }

 void uart_send_byte(uart_handle_t* uart, uint8_t data) {
//   while(!(uart->channel->STATUS & (1 << 6))); // wait for TX buffer to empty
// 	uart->channel->TXDATA = data;
 	HAL_UART_Transmit(&uart->uart, &data, 1, HAL_MAX_DELAY);
 }

void uart_send_bytes(uart_handle_t* uart, void const *data, size_t length) {

	HAL_UART_Transmit(&uart->uart, data, length, HAL_MAX_DELAY);
// 	for(uint8_t i=0; i<length; i++)	{
// 		uart_send_byte(uart, ((uint8_t const*)data)[i]);
// 	}
 }

 void uart_send_string(uart_handle_t* uart, const char *string) {
   uart_send_bytes(uart, string, strnlen(string, 100));
 }

 error_t uart_rx_interrupt_enable(uart_handle_t* uart) {
   if(handler[uart->idx] == NULL) { return EOFF; }
//   USART_IntClear(uart->channel, _UART_IF_MASK);
//   USART_IntEnable(uart->channel, UART_IF_RXDATAV);
//   NVIC_ClearPendingIRQ(uart->irq.tx);
//   NVIC_ClearPendingIRQ(uart->irq.rx);
//   NVIC_EnableIRQ(uart->irq.rx);

   HAL_NVIC_ClearPendingIRQ(uart->irq.rx);
   HAL_NVIC_EnableIRQ(uart->irq.rx);
   return SUCCESS;
 }

 void uart_rx_interrupt_disable(uart_handle_t* uart) {
	 HAL_NVIC_ClearPendingIRQ(uart->irq.rx);
	 HAL_NVIC_DisableIRQ(uart->irq.rx);
 }

 void USART2_IRQHandler(void) {
	 HAL_UART_IRQHandler(&handle[2].uart);

//   if(handle[0].channel->STATUS & UART_STATUS_RXDATAV) {
//     handler[0](USART_Rx(handle[0].channel));
//     USART_IntClear(handle[0].channel, UART_IF_RXDATAV);
//   }
 }

 void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
  /* Set transmission flag: transfer complete*/
  //UartReady = SET;
 }

 void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
//   if(RingBuffer_GetDataLength(&txBuf) > 0) {
//     RingBuffer_Read(&txBuf, &txData, 1);
//     HAL_UART_Transmit_IT(huart, &txData, 1);
//   }
 }

 void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
//   if(huart->ErrorCode == HAL_UART_ERROR_ORE)
//     HAL_UART_Receive_IT(huart, (uint8_t *)readBuf, 1);
 }

// void UART1_RX_IRQHandler(void) {
//   if(handle[1].channel->STATUS & UART_STATUS_RXDATAV) {
//     handler[1](USART_Rx(handle[1].channel));
//     USART_IntClear(handle[1].channel, UART_IF_RXDATAV);
//   }
// }

// void USART0_RX_IRQHandler(void) {
//   if(handle[2].channel->STATUS & UART_STATUS_RXDATAV) {
//     handler[2](USART_Rx(handle[2].channel));
//     USART_IntClear(handle[2].channel, UART_IF_RXDATAV);
//   }
// }

// void USART1_RX_IRQHandler(void) {
//   if(handle[3].channel->STATUS & UART_STATUS_RXDATAV) {
//     handler[3](USART_Rx(handle[3].channel));
//     USART_IntClear(handle[3].channel, UART_IF_RXDATAV);
//   }
// }

// void USART2_RX_IRQHandler(void) {
//   if(handle[4].channel->STATUS & UART_STATUS_RXDATAV) {
//     handler[4](USART_Rx(handle[4].channel));
//     USART_IntClear(handle[4].channel, UART_IF_RXDATAV);
//   }
// }

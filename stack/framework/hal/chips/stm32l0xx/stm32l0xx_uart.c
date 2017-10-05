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

/*! \file stm32l0xx_uart.c
 *
 */

#include "hwgpio.h"
#include "hwuart.h"
#include <assert.h>
#include "hwsystem.h"
#include "stm32l0xx_mcu.h"
#include "stm32l0xx_hal.h"
#include "stm32l0xx_ll_usart.h"

#include "stm32l0xx_gpio.h"
#include "platform.h"
#include "string.h"
#include "ports.h"

// private definition of the UART handle, passed around publicly as a pointer
struct uart_handle {
  const uart_port_t* uart_port;
  UART_HandleTypeDef handle;
  uint32_t baudrate;
  uart_rx_inthandler_t rx_cb;
};

// private storage of handles, pointers to these records are passed around
static uart_handle_t handle[UART_COUNT] = {
  {
    .uart_port = &uart_ports[0],
    .rx_cb = NULL
  }
};

uart_handle_t* uart_init(uint8_t port_idx, uint32_t baudrate, uint8_t pins) {
  assert(port_idx < UART_COUNT);

  handle[port_idx].baudrate = baudrate;

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = (1 << GPIO_PIN(handle[port_idx].uart_port->tx)) | (1 << GPIO_PIN(handle[port_idx].uart_port->rx));
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = handle[port_idx].uart_port->alternate;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  assert(hw_gpio_configure_pin_stm(handle[port_idx].uart_port->rx, &GPIO_InitStruct) == SUCCESS);
  assert(hw_gpio_configure_pin_stm(handle[port_idx].uart_port->tx, &GPIO_InitStruct) == SUCCESS);

  return &handle[port_idx];
}

bool uart_enable(uart_handle_t* uart) {
  switch ((intptr_t)*(&uart->uart_port->uart))
  {
    case USART1_BASE:
      __HAL_RCC_USART1_CLK_ENABLE();
      break;
    case USART2_BASE:
      __HAL_RCC_USART2_CLK_ENABLE();
      break;
    default:
      assert(false);
  }

  uart->handle.Init.BaudRate = uart->baudrate;
  uart->handle.Init.WordLength = UART_WORDLENGTH_8B;
  uart->handle.Init.StopBits = UART_STOPBITS_1;
  uart->handle.Init.Parity = UART_PARITY_NONE;
  uart->handle.Init.Mode = UART_MODE_TX_RX;
  uart->handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  uart->handle.Init.OverSampling = UART_OVERSAMPLING_16;
  uart->handle.Instance = uart->uart_port->uart;
  if (HAL_UART_Init(&(uart->handle)) != HAL_OK)
  {
    assert ("cannot init");
    return false;
  }

  //   switch (uart->idx)
  //   	{
  //   	case 1:
  //   		__HAL_RCC_USART1_FORCE_RESET();
  //   		__HAL_RCC_USART1_RELEASE_RESET();
  //   		break;
  //   	case 2:
  //
  //   		__HAL_RCC_USART2_FORCE_RESET();
  //   		__HAL_RCC_USART2_RELEASE_RESET();
  //   		break;
  //   	default:
  //   		assert("not defined");
  //   		return false;
  //   	}



  HAL_NVIC_SetPriority(uart->uart_port->irq, 0, 3);
  
  return true;
}

bool uart_disable(uart_handle_t* uart) {
  HAL_UART_DeInit(&(uart->handle));
  switch ((intptr_t)*(&uart->uart_port->uart))
  {
    case USART1_BASE:
      __HAL_RCC_USART1_CLK_DISABLE();
      break;
    case USART2_BASE:
      __HAL_RCC_USART2_CLK_DISABLE();
      break;
    default:
      assert(false);
  }

  return true;
}

void uart_set_rx_interrupt_callback(uart_handle_t* uart,
                                    uart_rx_inthandler_t rx_handler)
{
  uart->rx_cb = rx_handler;
}

void uart_send_byte(uart_handle_t* uart, uint8_t data) {
  //   while(!(uart->channel->STATUS & (1 << 6))); // wait for TX buffer to empty
  // 	uart->channel->TXDATA = data;
  HAL_UART_Transmit(&uart->handle, &data, 1, HAL_MAX_DELAY);
}

// TODO remove or extend API?
//uint8_t uart_read_byte(uart_handle_t* uart, uint32_t timeout) {
//  uint8_t rec;
//  HAL_UART_Receive(&uart->uart, &rec, 1, timeout);
//  return rec;
//}

//void uart_read_bytes(uart_handle_t* uart,  uint8_t  *data, size_t length, uint32_t timeout) {
//  HAL_UART_Receive(&uart->uart, data, length, timeout);
//}

void uart_send_bytes(uart_handle_t* uart, void const *data, size_t length) {

  HAL_UART_Transmit(&uart->handle, (uint8_t*) data, length, HAL_MAX_DELAY);
  // 	for(uint8_t i=0; i<length; i++)	{
  // 		uart_send_byte(uart, ((uint8_t const*)data)[i]);
  // 	}
}

void uart_send_string(uart_handle_t* uart, const char *string) {
  uart_send_bytes(uart, string, strlen(string));
}

error_t uart_rx_interrupt_enable(uart_handle_t* uart) {
  if(uart->rx_cb == NULL) { return EOFF; }
  //   USART_IntClear(uart->channel, _UART_IF_MASK);
  //   USART_IntEnable(uart->channel, UART_IF_RXDATAV);
  //   NVIC_ClearPendingIRQ(uart->irq.tx);
  //   NVIC_ClearPendingIRQ(uart->irq.rx);
  //   NVIC_EnableIRQ(uart->irq.rx);

  __HAL_RCC_GPIOA_CLK_ENABLE(); // TODO not sure why this is needed here, should already be enabled. Look into this later

  HAL_NVIC_ClearPendingIRQ(uart->uart_port->irq);
  HAL_NVIC_EnableIRQ(uart->uart_port->irq);
  LL_USART_EnableIT_RXNE(uart->handle.Instance);
  LL_USART_EnableIT_ERROR(uart->handle.Instance);
  return SUCCESS;
}

void uart_rx_interrupt_disable(uart_handle_t* uart) {
  HAL_NVIC_ClearPendingIRQ(uart->uart_port->irq);
  HAL_NVIC_DisableIRQ(uart->uart_port->irq);
  LL_USART_DisableIT_RXNE(uart->handle.Instance);
  LL_USART_DisableIT_ERROR(uart->handle.Instance);
}

void USART2_IRQHandler(void) {
  if(LL_USART_IsActiveFlag_RXNE(USART2) && LL_USART_IsEnabledIT_RXNE(USART2))
  {
    uint8_t idx = 0;
    do {
      if(handle[idx].handle.Instance == USART2) {
        handle[idx].rx_cb(LL_USART_ReceiveData8(USART2)); // RXNE flag will be cleared by reading of DR register
      }

      idx++;
    } while(idx < UART_COUNT);
  }

  if(LL_USART_IsEnabledIT_ERROR(USART2) && LL_USART_IsActiveFlag_NE(USART2))
  {
    assert(false); // TODO how to handle this?
  }
}

void USART1_IRQHandler(void) {
  assert(false); // TODO
}

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
//  /* Set transmission flag: transfer complete*/
//  //UartReady = SET;
//}

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

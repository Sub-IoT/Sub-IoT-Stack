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

#define UARTS     3   // TODO add LPUART?

typedef struct {
  pin_id_t tx;
  pin_id_t rx;
} uart_pins_t;

#define UNDEFINED_LOCATION                       \
  .tx       = PIN_UNDEFINED,   \
  .rx       = PIN_UNDEFINED    \

// configuration of uart/location mapping to tx and rx pins
// TODO to be completed with all documented locations
// TODO move to ports.h
static uart_pins_t location[UARTS] = {
  {
    // DUMMY
    UNDEFINED_LOCATION
  },
  {
    // USART 1
    .tx       = PIN(0, 9),
    .rx       = PIN(0, 10)

  },
  {
    // USART 2
    .tx       = PIN(0, 2),
    .rx       = PIN(0, 3)
  }
};

// references to registered handlers
static uart_rx_inthandler_t handler[UARTS];

// private definition of the UART handle, passed around publicly as a pointer
struct uart_handle {
  uint8_t              idx;
  uint8_t 	mapping;
  UART_HandleTypeDef 	uart;
  IRQn_Type           irq;
  uart_pins_t*         pins;
  uint32_t baudrate;
};

// private storage of handles, pointers to these records are passed around
static uart_handle_t handle[UARTS] = {
  {
    .idx     = 0,
    .mapping = 0,
    //.irq     = 0
  },
  {
    .idx     = 1,
    .mapping = GPIO_AF4_USART1,
    .uart.Instance  = USART1,
    .irq     = USART1_IRQn
  },
  {
    .idx     = 2,
    .mapping = GPIO_AF4_USART2,
    .uart.Instance  = USART2,
    .irq     = USART2_IRQn
  }
};

uart_handle_t* uart_init(uint8_t idx, uint32_t baudrate, uint8_t pins) {
  assert(idx == 1 || idx == 2);

  handle[idx].pins = &location[idx];
  handle[idx].baudrate = baudrate;

  GPIO_InitTypeDef GPIO_InitStruct;

  GPIO_InitStruct.Pin = (1 << GPIO_PIN(handle[idx].pins->tx)) | (1 << GPIO_PIN(handle[idx].pins->rx));
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = handle[idx].mapping;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  assert(hw_gpio_configure_pin_stm(handle[idx].pins->rx, &GPIO_InitStruct) == SUCCESS);
  assert(hw_gpio_configure_pin_stm(handle[idx].pins->tx, &GPIO_InitStruct) == SUCCESS);

  return &handle[idx];
}

bool uart_enable(uart_handle_t* uart) {

  switch (uart->idx)
  {
    case 1:
      __HAL_RCC_USART1_CLK_ENABLE();
      break;
    case 2:
      __HAL_RCC_USART2_CLK_ENABLE();
      break;
    default:
      assert(false);
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



  HAL_NVIC_SetPriority(USART2_IRQn, 0, 3);
  
  return true;
}

bool uart_disable(uart_handle_t* uart) {
  HAL_UART_DeInit(&(uart->uart));
  switch (uart->idx)
  {
    case 1:
      __HAL_RCC_USART1_CLK_DISABLE();
      break;
    case 2:
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
  handler[uart->idx] = rx_handler;
}

void uart_send_byte(uart_handle_t* uart, uint8_t data) {
  //   while(!(uart->channel->STATUS & (1 << 6))); // wait for TX buffer to empty
  // 	uart->channel->TXDATA = data;
  HAL_UART_Transmit(&uart->uart, &data, 1, HAL_MAX_DELAY);
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

  HAL_UART_Transmit(&uart->uart, (uint8_t*) data, length, HAL_MAX_DELAY);
  // 	for(uint8_t i=0; i<length; i++)	{
  // 		uart_send_byte(uart, ((uint8_t const*)data)[i]);
  // 	}
}

void uart_send_string(uart_handle_t* uart, const char *string) {
  uart_send_bytes(uart, string, strlen(string));
}

error_t uart_rx_interrupt_enable(uart_handle_t* uart) {
  if(handler[uart->idx] == NULL) { return EOFF; }
  //   USART_IntClear(uart->channel, _UART_IF_MASK);
  //   USART_IntEnable(uart->channel, UART_IF_RXDATAV);
  //   NVIC_ClearPendingIRQ(uart->irq.tx);
  //   NVIC_ClearPendingIRQ(uart->irq.rx);
  //   NVIC_EnableIRQ(uart->irq.rx);

  __HAL_RCC_GPIOA_CLK_ENABLE(); // TODO not sure why this is needed here, should already be enabled. Look into this later

  HAL_NVIC_ClearPendingIRQ(uart->irq);
  HAL_NVIC_EnableIRQ(uart->irq);
  LL_USART_EnableIT_RXNE(uart->uart.Instance);
  LL_USART_EnableIT_ERROR(uart->uart.Instance);
  return SUCCESS;
}

void uart_rx_interrupt_disable(uart_handle_t* uart) {
  HAL_NVIC_ClearPendingIRQ(uart->irq);
  HAL_NVIC_DisableIRQ(uart->irq);
  LL_USART_DisableIT_RXNE(uart->uart.Instance);
  LL_USART_DisableIT_ERROR(uart->uart.Instance);
}

void USART2_IRQHandler(void) {
  if(LL_USART_IsActiveFlag_RXNE(USART2) && LL_USART_IsEnabledIT_RXNE(USART2))
  {
    handler[2](LL_USART_ReceiveData8(USART2)); // RXNE flag will be cleared by reading of DR register
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

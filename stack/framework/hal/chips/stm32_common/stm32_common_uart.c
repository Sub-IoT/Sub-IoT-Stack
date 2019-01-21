/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2018 University of Antwerp
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

/*! \file stm32_common_uart.c
 *
 */

#include "hwgpio.h"
#include "hwuart.h"
#include "debug.h"
#include "hwsystem.h"
#include "stm32_device.h"
#include "stm32_common_gpio.h"
#include "platform.h"
#include "string.h"
#include "ports.h"
#include "errors.h"
#include "log.h"

DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart2_tx;

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
  },
  {
    .uart_port = &uart_ports[1],
    .rx_cb = NULL
  }
};

uart_handle_t* uart_init(uint8_t port_idx, uint32_t baudrate, uint8_t pins) {
  assert(port_idx < UART_COUNT);

  handle[port_idx].baudrate = baudrate;
  return &handle[port_idx];
}

/**
* @brief UART MSP Initialization
* This function configures the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspInitCustom(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(huart->Instance==USART1)
  {
    /* USART1 DMA Init */
    /* USART1_TX Init */
    hdma_usart1_tx.Instance = DMA1_Channel2;
    hdma_usart1_tx.Init.Request = DMA_REQUEST_3;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      //Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmatx,hdma_usart1_tx);

    /* USART1 interrupt Init */
    //HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    //HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(huart->Instance==USART2)
  {
    /* USART2 DMA Init */
    /* USART2_TX Init */
    hdma_usart2_tx.Instance = DMA1_Channel4;
    hdma_usart2_tx.Init.Request = DMA_REQUEST_4;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    {
      //Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmatx,hdma_usart2_tx);

    /* USART2 interrupt Init */
    //HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    //HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }

}

bool uart_enable(uart_handle_t* uart) {
  // enable GPIO
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = 1 << GPIO_PIN(uart->uart_port->tx);
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = uart->uart_port->alternate;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  assert(hw_gpio_configure_pin_stm(uart->uart_port->tx, &GPIO_InitStruct) == SUCCESS);
  GPIO_InitStruct.Pin = 1 << GPIO_PIN(uart->uart_port->rx);
  GPIO_InitStruct.Pull = GPIO_PULLUP; // TODO this is only required on some boards (eg nucleo when using usb connection). Make sure there is no drawback (for example energy consumption)
  assert(hw_gpio_configure_pin_stm(uart->uart_port->rx, &GPIO_InitStruct) == SUCCESS);

  switch ((intptr_t)*(&uart->uart_port->uart))
  {
    case LPUART1_BASE:
      __HAL_RCC_LPUART1_CLK_ENABLE();
      break;
    case USART1_BASE:
      __HAL_RCC_USART1_CLK_ENABLE();
      break;
    case USART2_BASE:
      __HAL_RCC_USART2_CLK_ENABLE();
      break;
#ifdef USART4_BASE
    case USART4_BASE:
      __HAL_RCC_USART4_CLK_ENABLE();
      break;
#endif
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

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

//TODO SELECT ONE
 /* DMA interrupt init */
  /* DMA1_Channel2_3_IRQn interrupt configuration */
  //HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);
  //HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
  /* DMA1_Channel4_5_6_7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, 0, 3);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);

  HAL_UART_MspInitCustom(&uart->handle);

  HAL_NVIC_SetPriority(uart->uart_port->irq, 0, 3);
  
  return true;
}

bool uart_disable(uart_handle_t* uart) {
  HAL_UART_DeInit(&(uart->handle));
  switch ((intptr_t)*(&uart->uart_port->uart))
  {
    case LPUART1_BASE:
      __HAL_RCC_LPUART1_CLK_DISABLE();
    case USART1_BASE:
      __HAL_RCC_USART1_CLK_DISABLE();
      break;
    case USART2_BASE:
      __HAL_RCC_USART2_CLK_DISABLE();
      break;
    default:
      assert(false);
  }

  GPIO_InitTypeDef GPIO_InitStruct= { 0 };
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = 1 << GPIO_PIN(uart->uart_port->tx);
  assert(hw_gpio_configure_pin_stm(uart->uart_port->tx, &GPIO_InitStruct) == SUCCESS);
  GPIO_InitStruct.Pin = 1 << GPIO_PIN(uart->uart_port->rx);
  assert(hw_gpio_configure_pin_stm(uart->uart_port->rx, &GPIO_InitStruct) == SUCCESS);

  //log_print_string("!!! uart disabled");
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
  HAL_UART_Transmit_DMA(&uart->handle, &data, 1);
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

  HAL_UART_Transmit_DMA(&uart->handle, (uint8_t*) data, length);
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

static void uart_irq_handler(USART_TypeDef* uart) {
  if(LL_USART_IsEnabledIT_ERROR(uart))
  {
    if(LL_USART_IsActiveFlag_NE(uart))
    {
      //assert(false); // TODO how to handle this?
      LL_USART_ClearFlag_NE(uart);
    }

    if(LL_USART_IsActiveFlag_FE(uart))
    {
      LL_USART_ClearFlag_FE(uart);
    }

    // TODO other flags?
  }

  if(LL_USART_IsActiveFlag_RXNE(uart) && LL_USART_IsEnabledIT_RXNE(uart))
  {
    uint8_t idx = 0;
    do {
      if(handle[idx].handle.Instance == uart) {
        handle[idx].rx_cb(LL_USART_ReceiveData8(uart)); // RXNE flag will be cleared by reading of DR register
        return;
      }

      idx++;
    } while(idx < UART_COUNT);

    assert(false); // we should not reach this point
  }


  if (((READ_REG(uart->ISR) & USART_ISR_TC) != 0U) && ((READ_REG(uart->CR1) & USART_CR1_TCIE) != 0U))
  {
      /* Disable the UART Transmit Complete Interrupt */
  CLEAR_BIT(uart->CR1, USART_CR1_TCIE);

  /* Tx process is ended, restore huart->gState to Ready */
  handle[0].handle.gState = HAL_UART_STATE_READY;

  //HAL_UART_TxCpltCallback(huart);

    //UART_EndTransmit_IT(handle[0].handle);
    //USART_EndTransmit_IT(handle[0]->handle);
    return;
  }
}

void USART2_IRQHandler(void) {
  uart_irq_handler(USART2);
}

void USART1_IRQHandler(void) {
  uart_irq_handler(USART1);
}

void LPUART1_IRQHandler(void) {
  uart_irq_handler(LPUART1);
}

/**
  * @brief This function handles DMA1 channel 2 and channel 3 interrupts.
  */
void DMA1_Channel2_3_IRQHandler(void)
{

  HAL_DMA_IRQHandler(&hdma_usart1_tx);

}
/**
  * @brief This function handles DMA1 channel 4, channel 5, channel 6 and channel 7 interrupts.
  */
void DMA1_Channel4_5_6_7_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
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

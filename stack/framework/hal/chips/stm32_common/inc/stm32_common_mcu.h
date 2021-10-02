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

#ifndef __STM32_COMMON_MCU_H_
#define __STM32_COMMON_MCU_H_

#include "stm32_device.h"
#include "hwgpio.h"
#include "hwdma.h"

#define PIN(port, pin)  ((GPIOA_BASE + (port << 10)) | pin)
#define PIN_UNDEFINED 0xFFFFFFFF

enum
{
  GPIO_PORTA = 0,
  GPIO_PORTB,
  GPIO_PORTC,
  GPIO_PORTD,
  GPIO_PORTE,
  GPIO_PORTF,
  GPIO_PORTG,
  GPIO_PORTH,
};

typedef struct {
  pin_id_t sck_pin;
  pin_id_t miso_pin;
  pin_id_t mosi_pin;
  uint32_t sck_alternate;
  uint32_t miso_alternate;
  uint32_t mosi_alternate;
  SPI_TypeDef* spi;
} spi_port_t;

typedef struct {
  pin_id_t tx;
  pin_id_t rx;
  uint32_t alternate;
  USART_TypeDef* uart;
  IRQn_Type irq;
  bool swap_tx_rx;
} uart_port_t;

typedef struct {
  I2C_TypeDef* i2c;
  pin_id_t scl_pin;
  pin_id_t sda_pin;
  uint32_t scl_alternate;
  uint32_t sda_alternate;
} i2c_port_t;


typedef struct {
  dma_peripheral_t peripheral;
  uint8_t channel_nr;
  IRQn_Type irq;
} dma_channel_t;

void stm32_common_mcu_init();

#endif

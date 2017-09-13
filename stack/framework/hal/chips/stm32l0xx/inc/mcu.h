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

#ifndef __MCU_H_
#define __MCU_H_

#include "stm32l0xx.h"
#include "stm32l0xx_hal.h"
#include "hwgpio.h"

#define PIN(port, pin)  ((GPIOA_BASE + (port << 10)) | pin)
#define PIN_UNDEFINED 0xFFFFFFFF

enum
{
  GPIO_PORTA = 0,
  GPIO_PORTB,
  GPIO_PORTC,
  GPIO_PORTD,
  GPIO_PORTE,
};

typedef struct {
  pin_id_t sck_pin;
  pin_id_t miso_pin;
  pin_id_t mosi_pin;
  uint32_t pins;
  uint32_t alternate;
  SPI_TypeDef* spi;
} spi_port_t;

void __stm32l0xx_mcu_init();

#endif

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

#include "hwgpio.h"

#define PIN(port, pin)  ((port << 4) | pin)
#define PIN_UNDEFINED 0xFFFFFFFF

enum
{
    GPIO_PORTA = 0,
    GPIO_PORTB,
    GPIO_PORTC,
    GPIO_PORTD,
    GPIO_PORTE,
    GPIO_PORTF,
};


typedef struct {
    uint32_t location;
    pin_id_t mosi;
    pin_id_t miso;
    pin_id_t clk;
} spi_port_t;

void __stm32l0xx_mcu_init();

#endif

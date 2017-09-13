/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2017 University of Antwerp
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

#ifndef __PORTS_H_
#define __PORTS_H_

#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal_gpio.h"
#include "stm32l0xx_chip.h"
#include "mcu.h"

static const spi_port_t spi_ports[] = {
    {
        .spi = SPI1,
        .miso_pin = PIN(GPIO_PORTA, 6),
        .mosi_pin = PIN(GPIO_PORTA, 7),
        .sck_pin = PIN(GPIO_PORTB, 3),
        .alternate = GPIO_AF0_SPI1,
    }
};

#define SPI_COUNT sizeof(spi_ports) / sizeof(spi_port_t)

#endif

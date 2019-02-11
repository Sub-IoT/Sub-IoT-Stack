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

#include "stm32_device.h"
#include "stm32_common_mcu.h"
#include "platform_defs.h"
#include "hwgpio.h"
#include "stm32_common_eeprom.h"

static const spi_port_t spi_ports[] = {
  {
    .spi = SPI1,
    .miso_pin = PIN(GPIO_PORTA, 6),
    .mosi_pin = PIN(GPIO_PORTA, 7),
    .sck_pin = PIN(GPIO_PORTB, 3),
    .alternate = GPIO_AF0_SPI1,
  },
  {
    .spi = SPI2,
    .miso_pin = PIN(GPIO_PORTB, 14),
    .mosi_pin = PIN(GPIO_PORTB, 15),
    .sck_pin = PIN(GPIO_PORTB, 13),
    .alternate = GPIO_AF0_SPI2,
  }
  
};

#define SPI_COUNT sizeof(spi_ports) / sizeof(spi_port_t)

static const uart_port_t uart_ports[] = {
  {
    // USART2, connected to VCOM of debugger USB connection
    .tx = PIN(GPIO_PORTA, 2),
    .rx = PIN(GPIO_PORTA, 3),
    .alternate = GPIO_AF4_USART2,
    .uart = USART2,
    .irq = USART2_IRQn
  },
  {
    // USART1, exposed on CN3 header: TX=PA9, RX=PA10
    .tx = PIN(GPIO_PORTA, 9),
    .rx = PIN(GPIO_PORTA, 10),
    .alternate = GPIO_AF4_USART1,
    .uart = USART1,
    .irq = USART1_IRQn
  }
};

#define UART_COUNT sizeof(uart_ports) / sizeof(uart_port_t)

static const i2c_port_t i2c_ports[] = {
  {
    .i2c = I2C1,
    .scl_pin = PIN(GPIO_PORTB, 8),
    .sda_pin = PIN(GPIO_PORTB, 9),
    .alternate = GPIO_AF4_I2C1,
  }
};

#define I2C_COUNT sizeof(i2c_ports) / sizeof(i2c_port_t)

static pin_id_t debug_pins[PLATFORM_NUM_DEBUGPINS] = {
  PIN(GPIO_PORTB, 9), // exposed on CN3 header, pin 24
  PIN(GPIO_PORTB, 8), // exposed on CN3 header, pin 25
};


static blockdevice_stm32_eeprom_t eeprom_bd = (blockdevice_stm32_eeprom_t){
  .base.driver = &blockdevice_driver_stm32_eeprom,
};

static blockdevice_t* d7_systemfiles_blockdevice = (blockdevice_t*)&eeprom_bd;

#endif

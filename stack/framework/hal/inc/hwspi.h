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

/*!
 * \file hwspi.h
 * \addtogroup SPI
 * \ingroup HAL
 * {@
 * \brief HAL API for SPI
 *
 * \author jan.stevens@ieee.org
 * \author contact@christophe.vg
 */

#ifndef SPI_H_
#define SPI_H_

#include "hwgpio.h"

#include "types.h"
#include "link_c.h"

typedef struct spi_definition {
  uint8_t   usart;
  uint32_t  baudrate;
  uint8_t   databits;
  uint8_t   location;
} spi_definition_t;

__LINK_C void spi_init(spi_definition_t spi);

__LINK_C void spi_init_slave(pin_id_t slave);

__LINK_C void spi_auto_cs_on(void);

__LINK_C void spi_auto_cs_off(void);

#define spi_select(slave)   hw_gpio_clr(slave)
#define spi_deselect(slave) hw_gpio_set(slave);

__LINK_C uint8_t spi_byte(uint8_t channel, uint8_t data); // bi-dir transfer
__LINK_C void    spi_send(uint8_t channel, uint16_t data); // send only

__LINK_C void spi_string(uint8_t channel, uint8_t *TxData, uint8_t *RxData, size_t length);

#endif /* SPI_H_ */

/** @}*/

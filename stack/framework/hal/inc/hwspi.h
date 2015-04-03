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

/*
 * spi.h
 *
 *  Created on: Sep 12, 2013
 *      Author: jan.stevens@ieee.org
 */

#ifndef SPI_H_
#define SPI_H_

#include "types.h"
#include "link_c.h"

#define DATARATE 115200

__LINK_C void spi_init();

__LINK_C void spi_auto_cs_on(void);

__LINK_C void spi_auto_cs_off(void);

__LINK_C void spi_select_chip(void);

__LINK_C void spi_deselect_chip(void);

__LINK_C uint8_t spi_byte(uint8_t data);

__LINK_C void spi_string(uint8_t *TxData, uint8_t *RxData, size_t length);

#endif /* SPI_H_ */

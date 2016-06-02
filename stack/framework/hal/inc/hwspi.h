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

// expose spi_handles with unknown internals
typedef struct spi_handle spi_handle_t;
typedef struct spi_slave_handle spi_slave_handle_t;

// create handle from basic & minimal parameters
__LINK_C spi_handle_t*       spi_init(uint8_t uart,     uint32_t baudrate,
                                      uint8_t databits, bool     msbf,
                                      uint8_t pins);

// manual power mgmt
__LINK_C void                spi_enable(spi_handle_t* spi);
__LINK_C void                spi_disable(spi_handle_t* spi);

// initializes a pin to be used as Chip Select pin
__LINK_C spi_slave_handle_t* spi_init_slave(spi_handle_t* spi, pin_id_t cs_pin,
                                            bool          cs_is_active_low);

// (de)selection of a slave
__LINK_C void                spi_select(spi_slave_handle_t* slave);
__LINK_C void                spi_deselect(spi_slave_handle_t* slave);

__LINK_C uint8_t             spi_exchange_byte(spi_slave_handle_t* spi,
                                               uint8_t data);
__LINK_C void                spi_send_byte_with_control(spi_slave_handle_t* spi,
                                                        uint16_t data);

__LINK_C void                spi_exchange_bytes(spi_slave_handle_t* spi,
                                                uint8_t *TxData,
                                                uint8_t *RxData, size_t length);

#endif

/** @}*/

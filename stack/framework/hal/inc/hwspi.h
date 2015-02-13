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

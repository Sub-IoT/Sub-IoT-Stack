/*
 * spi.h
 *
 *  Created on: Sep 12, 2013
 *      Author: jan.stevens@ieee.org
 */

#ifndef SPI_H_
#define SPI_H_
#include "types.h"

#define DATARATE 115200

void spi_init();

void spi_auto_cs_on(void);

void spi_auto_cs_off(void);

void spi_select_chip(void);

void spi_deselect_chip(void);

uint8_t spi_byte(uint8_t data);

void spi_string(uint8_t *TxData, uint8_t *RxData, size_t length);

#endif /* SPI_H_ */

/*
 * spi.h
 *
 *  Created on: Sep 12, 2013
 *      Author: jan.stevens@ieee.org
 */

#ifndef SPI_H_
#define SPI_H_

#define DATARATE 115200

void spi_init();

void spi_auto_cs_on(void);

void spi_auto_cs_off(void);

void spi_select_chip(void);

void spi_deselect_chip(void);

unsigned char spi_byte(unsigned char data);

void spi_string(unsigned char *TxData, unsigned char *RxData, unsigned int length);

#endif /* SPI_H_ */

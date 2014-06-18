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

void spi_enable_interrupt();

void spi_transmit_data(unsigned char data);
void spi_transmit_message(unsigned char *data, unsigned char length);

unsigned char spi_tx_ready();

unsigned char spi_receive_data();

#endif /* SPI_H_ */

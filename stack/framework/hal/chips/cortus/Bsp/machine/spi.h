/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * \file spi.h
 * \author philippe.nunes@cortus.com
 */

#ifndef _SPI_H
#define _SPI_H
#include <machine/sfradr.h>

#define SPI_COUNT 2

typedef struct SPI
{
    /* Transmit data to tx buffer */
    volatile unsigned tx_data;

    /* Receive data from rx buffer */
    volatile unsigned rx_data;

    /* Status register */
    volatile unsigned tx_status;
    volatile unsigned rx_status;

    /* Input clock selection */
    volatile unsigned selclk;

    /* Clock divider */
    volatile unsigned divider;

    /* Internal clock enable - Sleep mode */
    volatile unsigned clk_en;

    /* SPI in master mode or slave mode */
    volatile unsigned master;

    /* Mode fault enable */
    volatile unsigned mode_fault;

    /* Configuration 
       bit       config
       4         bidirectionnal direction
       3         bidirectionnal mode enable
       2         lsb first
       1         sck phase 0 odd edges 1 even edges
       0         sck polarity
    */
    volatile unsigned config;

    /* Activity on bus - should always check before writing fifo */
    volatile unsigned bus_active;

    /* Mask register for interrupt */
    volatile unsigned tx_mask;
    volatile unsigned rx_mask;

} SPI;

/***************  Bit definition for SPI_TX_STATUS register ******************/
#define SPI_TX_SR_FIFO_NOT_FULL       0x01 /*!< space in the fifo - bit 0 of spi.tx_status*/
#define SPI_RX_SR_DATA_AVAILABLE      0x01 /*!< data available in fifo - bit 0 of spi.rx_status*/
#define SPI_BUS_ACTIVE                0x01 /*!< activity on spi bus - bit 0 of spi.bus_active*/

#ifdef __APS__
#define spi1 ((SPI *)SFRADR_SPI)
#define spi2 ((SPI *)SFRADR_SPI2)
#else
extern SPI __spi;
#define spi1 (&__spi)
#define spi2 (&__spi)
#endif

#endif

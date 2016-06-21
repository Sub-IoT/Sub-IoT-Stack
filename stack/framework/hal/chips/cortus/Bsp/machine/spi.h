/*********************************************************************************
 * This confidential and proprietary software may be used only as authorized 
 *                      by a licensing agreement from                           
 *                           Cortus S.A.
 *
 *             (C) Copyright 2004, 2005, 2006 Cortus S.A.
 *                           ALL RIGHTS RESERVED
 *
 * The entire notice above must be reproduced on all authorized copies
 * and any such reproduction must be pursuant to a licensing agreement 
 * from Cortus S.A. (http://www.cortus.com)
 *
 * $CortusRelease$
 * $FileName$
 *
 *********************************************************************************/

#ifndef _SPI_H
#define _SPI_H
#include <machine/sfradr.h>

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

#ifdef __APS__
#define spi1 ((SPI *)SFRADR_SPI)
#define spi2 ((SPI *)SFRADR_SPI2)
#else
extern SPI __spi;
#define spi1 (&__spi)
#endif
#endif

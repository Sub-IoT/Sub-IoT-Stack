/*******************************************************************************
* File: spi.h
* @section License
* <b>(C) Copyright 2005 Cortus S.A, http://www.cortus.com
*******************************************************************************
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Cortus S.A has no
* obligation to support this Software. Cortus S.A is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Cortus S.A will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
******************************************************************************/

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

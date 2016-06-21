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

#ifndef _I2C_H
#define _I2C_H
#include <machine/sfradr.h>

typedef struct I2C
{
    /* Transmit data to tx buffer - 9 bits
     MSB indicates if this is an address 
     LSB indicates the direction */
    volatile unsigned tx_data;

    /* Receive data from rx buffer */
    volatile unsigned rx_data;

    /* Status register */
    volatile unsigned tx_status;
    volatile unsigned rx_status;

    /* Input clock selection */
    volatile unsigned selclk;

    /* Internal clock active */
    volatile unsigned clk_en;

    /* Clock divider - minimum baudrate*5*/
    volatile unsigned divider;

    /* Emition clock enabled and master mode */
    volatile unsigned master;

    /* Software wants to access the bus */
    volatile unsigned onbus;

    /* Slave address of the device */
    volatile unsigned address;
    
    /* Checks if there an activity on the line */
    volatile unsigned activity;

    /* Mask register for interrupt */
    volatile unsigned tx_mask;
    volatile unsigned rx_mask;

    /* RX and TX fifo flush */
    volatile unsigned flush;
    
    /* Number of RX byte to receive */
    volatile unsigned n_rx_bytes;

} I2C;

#ifdef __APS__
#define i2c ((I2C *)SFRADR_I2C)
#else
extern I2C __i2c;
#define i2c (&__i2c)
#endif
#endif

/*********************************************************************************
 *  This confidential and proprietary software may be used only as authorized 
 *                       by a licensing agreement from                           
 *                            Cortus S.A.
 *
 *           (C) Copyright 2004, 2005, Cortus S.A.
 *                           ALL RIGHTS RESERVED
 *
 *  The entire notice above must be reproduced on all authorized copies
 *  and any such reproduction must be pursuant to a licensing agreement 
 *  from Cortus S.A. (http://www.cortus.com)
 *
 * File: gpio.h
 *
 *********************************************************************************/

#ifndef _GPIO_INT_H
#define _GPIO_INT_H
#include <machine/sfradr.h>

typedef struct GPio_int
{
    volatile unsigned out;
    volatile unsigned in;
    volatile unsigned dir;
    volatile unsigned old_in;
    volatile unsigned mask;
} GPio_int;
#ifdef __APS__
#define gpio_int ((GPio_int *)SFRADR_GPIO_INT)
#else
extern GPio_int __gpio_int;
#define gpio_int (&__gpio_int)
#endif
#endif

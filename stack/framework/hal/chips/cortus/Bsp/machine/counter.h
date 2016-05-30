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

#ifndef _COUNTER_H
#define _COUNTER_H
#include <machine/sfradr.h>

typedef struct Counter
{
    /* Current value - counts down once each clock tick */
    volatile unsigned value;

    /* Reload value */
    volatile unsigned reload;

    /* Has the counter expired (reached zero and been reloaded) 
       since last time this was reset? */
    volatile unsigned expired;
    
    /* Interrupt mask/interrupt enable */
    volatile unsigned mask;

} Counter;

#ifdef __APS__
#define counter1 ((Counter *)SFRADR_COUNTER1)
#define counter2 ((Counter *)SFRADR_COUNTER2)
#else
extern Counter __counter1;
#define counter1 (&__counter1)
extern Counter __counter2;
#define counter2 (&__counter2)
#endif
#endif

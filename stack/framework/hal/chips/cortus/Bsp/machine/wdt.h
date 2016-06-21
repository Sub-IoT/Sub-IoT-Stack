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
 * File: wdt.h
 *
 *********************************************************************************/

#ifndef _WDT_H
#define _WDT_H
#include <machine/sfradr.h>

typedef struct WatchdogTimer
{
    /* Initial value of the timer */
    volatile unsigned value;

    /* Key value to acces value and status*/
    volatile unsigned key;

    /* Restart the timer (avoid to count down to 0) */
    volatile unsigned restart;
    
    /* Status of the WDT, 1 for enable, 0 for disable */
    volatile unsigned status;

    /* Clock input select 0 to 3 */
    volatile unsigned sel_clk;

    /* Enable */
    volatile unsigned enable;
  
} WatchdogTimer;

#ifdef __APS__
#define wdt ((WatchdogTimer *)SFRADR_WDT)
#else
extern WatchdogTimer __wdt;
#define wdt (&__wdt)
#endif
#endif

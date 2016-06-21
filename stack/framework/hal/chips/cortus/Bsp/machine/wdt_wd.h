/*********************************************************************************
 *  This confidential and proprietary software may be used only as authorized 
 *                       by a licensing agreement from                           
 *                            Cortus S.A.
 *
 *           (C) Copyright 2004, 2005, 2014 Cortus S.A.
 *                           ALL RIGHTS RESERVED
 *
 *  The entire notice above must be reproduced on all authorized copies
 *  and any such reproduction must be pursuant to a licensing agreement 
 *  from Cortus S.A. (http://www.cortus.com)
 *
 * File: wdt_wd.h
 *
 *********************************************************************************/

#ifndef _WDT_WD_H
#define _WDT_WD_H
#include <machine/sfradr.h>

typedef struct WatchdogTimerWD
{
    /* Initial value of the timer */
    volatile unsigned window_hi;

    /* Initial value of the timer */
    volatile unsigned window_lo;

    /* Key value to acces value and status*/
    volatile unsigned key;

    /* Restart the timer (avoid to count down to 0) */
    volatile unsigned restart;
    
    /* Status of the WDT, 1 for enable, 0 for disable */
    volatile unsigned control;

    /* Clock input select 0 to 3 */
    volatile unsigned sel_clk;
  
} WatchdogTimerWD;

#ifdef __APS__
#define wdt_wd ((WatchdogTimerWD *)SFRADR_WDT_WD)
#else
extern WatchdogTimerWD __wdt_wd;
#define wdt (&__wdt_wd)
#endif
#endif

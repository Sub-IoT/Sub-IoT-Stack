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
 * File: capint.h
 *
 *********************************************************************************/

#ifndef _CAPINT_H
#define _CAPINT_H
#include <machine/sfradr.h>

typedef struct 
{
    /* Mode
       0 - high level sensitive
       1 - low level sensitive
       2 - positive edge sensitive
       3 - negative edge sensitive
       4 - both edge sensitive
    */
    volatile unsigned mode;
    volatile unsigned status;
    volatile unsigned mask;
} CAPINT;

#ifdef __APS__
#ifdef SFRADR_CAPINT0
#define capint0 ((CAPINT *)SFRADR_CAPINT0)
#endif
#ifdef SFRADR_CAPINT1
#define capint1 ((CAPINT *)SFRADR_CAPINT1)
#endif
#ifdef SFRADR_CAPINT2
#define capint2 ((CAPINT *)SFRADR_CAPINT2)
#endif
#ifdef SFRADR_CAPINT3
#define capint3 ((CAPINT *)SFRADR_CAPINT3)
#endif
#else
extern CAPINT __capint;
#define capint0 (&__capint)
#define capint1 (&__capint)
#define capint2 (&__capint)
#define capint3 (&__capint)
#endif
#endif

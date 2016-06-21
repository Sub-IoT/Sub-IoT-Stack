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

#ifndef _SND_H
#define _SND_H
#include <machine/sfradr.h>
#include <stdio.h>

typedef struct SND
{
    /* SND status - 1 on, 0 off */
    volatile unsigned enable;
    volatile unsigned sample;
} SND;

#ifdef __APS__
#define snd ((SND *)SFRADR_SND)
#else
extern SND __snd1;
#define snd (&__snd1)
#endif

#endif

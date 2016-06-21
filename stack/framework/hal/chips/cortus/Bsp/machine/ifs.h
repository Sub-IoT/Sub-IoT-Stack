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
 * File: ifs.h
 *
 *********************************************************************************/

#ifndef _IFS_H
#define _IFS_H
#include <machine/sfradr.h>

typedef struct IFS
{
    /* Access cycle width */
    volatile unsigned i_ws;
    volatile unsigned d_ws;
    volatile unsigned cache_en;
} IFS;

#ifdef __APS__
#define ifs ((IFS *)SFRADR_IFS)
#else
extern IFS __ifs;
#define ifs (&__ifs)
#endif
#endif

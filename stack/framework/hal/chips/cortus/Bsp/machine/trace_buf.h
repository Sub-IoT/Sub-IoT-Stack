/*********************************************************************************
 * This confidential and proprietary software may be used only as authorized 
 *                      by a licensing agreement from                           
 *                           Cortus S.A.
 *
 *                     (C) Copyright 2009 Cortus S.A.
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

#ifndef _TRACE_BUF_H
#define _TRACE_BUF_H
#include <machine/sfradr.h>

typedef struct Trace_Buf_Mem
{
    /* 0 for PC, other register number. 16 == RTT, 17 == PSR */
    unsigned tag;

    /* Value */
    unsigned value;
} Trace_Buf_Mem;

typedef struct Trace_Buf_Sfr
{
    /* Enable writing of trace information */
    unsigned en:1;

    /* Write register values as well as PC */
    unsigned reg_en:1;

    /* Index within trace memory of next location to be written */
    unsigned idx;
} Trace_Buf_Sfr;

#ifdef __APS__
#define trace_buf_mem 	((Trace_Buf_Mem*)SFRADR_TRACE_MEM)
#define trace_buf 	((Trace_Buf_Sfr*)SFRADR_TRACE_BUF)
#else
extern Trace_Buf_Mem __trace_buf_mem;
#define trace_buf_mem (&__trace_buf_mem)
extern Trace_Buf __trace_buf;
#define trace_buf (&__trace_buf);
#endif

#endif

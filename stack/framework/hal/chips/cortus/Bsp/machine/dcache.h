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

#ifndef _DCACHE_H
#define _DCACHE_H
#include <machine/sfradr.h>

typedef struct Dcache
{
    volatile unsigned flush;         
    volatile unsigned flush_line;          
    volatile unsigned invalidate;          
    volatile unsigned invalidate_line;      
    volatile unsigned lock;  
    volatile unsigned lock_status;   
    volatile unsigned unlock;
    volatile unsigned dad;     
    volatile unsigned wthrough;        
    volatile unsigned enable;    
    volatile unsigned pending;
} Dcache;

#ifdef __APS__
#define dcache ((Dcache *)SFRADR_DCACHE)
#define dcache1 ((Dcache *)SFRADR_DCACHE1)
#else
extern Dcache __dcache;
extern Dcache __dcache1;
#define dcache (&__dcache)
#define dcache1 (&__dcache1)
#endif

#endif

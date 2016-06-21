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

#ifndef _CPU_H
#define _CPU_H

static inline void icache_flush (void)
{
    __asm__ __volatile__ ("icache_flush");
}

static inline void cpu_int_enable (void)
{
    /* Set psr.pien = 1, psr.ien = 1, psr.psuper = 1, psr.super = 1 */
    __asm__ __volatile__ ("movhi r7, #0xf\n"
	 "mov psr, r7" :
	 /* no outputs */ : 
	 /* no inputs*/ : 
	 /* clobber */ "r7", "memory" );
}

static inline void cpu_int_disable (void) 
{
    /* Set psr.pien = 1, psr.ien = 0, psr.psuper = 1, psr.super = 1 */
    __asm__ __volatile__ ("movhi r7, #0xe\n"
	 "mov psr, r7" :
	 /* no outputs */ : 
	 /* no inputs*/ : 
	 /* clobber */ "r7", "memory" );
}

static inline void cpu_user_mode (void)
{
    /* Set psr.pien = 1, psr.ien = 1, psr.psuper = 0, psr.super = 0 */
    __asm__ __volatile__ ("movhi r7, #0x3\n"
	 "mov psr, r7" :
	 /* no outputs */ : 
	 /* no inputs*/ : 
	 /* clobber */ "r7", "memory" );
}

/* Return ID of executing CPU */
static inline int cpu_id (void)
{
    int id;
    __asm__ __volatile__ ("; get cpu id\n\t"
	 "mov\t%0, asr2" : "=r" (id));
    return id;
}

/* Aquire 'lock' in shared memory - busy waiting if needed */
static inline void cpu_spin_lock(volatile unsigned* lock)
{
    __asm__ __volatile__ (
	 "; aquire spin lock\n\t"
	 "mov\tr7, #1\n"
	 "1:\tldst.cc r6, [%0]\n\t"
  	 "bne\t1b" : 
         /* no outputs */ : 
         /* inputs */ "r" (lock) : 
         /* clobber */ "r6", "r7", "psr", "memory"); 
}

/* Unlock 'lock' assuming it was locked */
static inline void cpu_spin_unlock(volatile unsigned* lock)
{
    *lock = 0;
}

/* Type of trap handler functions */
typedef void (*trap_handler_fp)(void);

/* Trap vector table */
extern volatile trap_handler_fp trap_vectors[];

#endif


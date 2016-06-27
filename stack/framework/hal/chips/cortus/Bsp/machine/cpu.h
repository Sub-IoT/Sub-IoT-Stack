/*******************************************************************************
* File: cpu.h
* @section License
* <b>(C) Copyright 2005 Cortus S.A, http://www.cortus.com
*******************************************************************************
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Cortus S.A has no
* obligation to support this Software. Cortus S.A is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Cortus S.A will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
******************************************************************************/

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


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

#ifndef __CRITICAL_H
#define __CRITICAL_H
#include <machine/cpu.h>

int __critical_section_nesting;

static inline void enter_critical_section(void)
{
    cpu_int_disable ();
    __critical_section_nesting = ((volatile int) __critical_section_nesting) + 1;
}

static inline void leave_critical_section(void)
{
    __critical_section_nesting--;
    if (__critical_section_nesting == 0) {
        cpu_int_enable ();
    }
}

#endif

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

#ifndef _SYS_MEMSIZE_H
#define _SYS_MEMSIZE_H

/* Override the standard heap size for the linker */
#define HEAP_SIZE(sz) \
    __asm__(".global _HEAP_SIZE\n" \
        ".equ _HEAP_SIZE," # sz)

/* Override the standard stack size for the linker */
#define STACK_SIZE(sz) \
    __asm__(".global _STACK_SIZE\n" \
        ".equ _STACK_SIZE," # sz)

/* Define the size of program memory (ROM/FLASH etc) for the linker */
#define PROGRAM_MEMORY_SIZE(sz) \
    __asm__(".global _PROGRAM_MEMORY_SIZE\n" \
        ".equ _PROGRAM_MEMORY_SIZE," # sz)

/* Define the size of data memory (RAM) for the linker */
#define DATA_MEMORY_SIZE(sz) \
    __asm__(".global _DATA_MEMORY_SIZE\n" \
        ".equ _DATA_MEMORY_SIZE," # sz)


#endif



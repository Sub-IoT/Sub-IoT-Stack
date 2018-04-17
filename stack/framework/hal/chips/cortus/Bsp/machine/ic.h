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

#ifndef _IC_H
#define _IC_H
#include <machine/sfradr.h>

#ifdef __APS3B__
/* Big endian version */

/* Interrupt controller */
typedef struct Ic
{
    /* Priority of last accepted interrupt - this is only
       valid after an interrupt! */
    volatile unsigned char ipl;

    /* Global priority level - interrupt must have higher priority
       than this to be taken into account */
    volatile unsigned char cpl;

    /* Interrupt pending */
    volatile unsigned char irq;

    /* Global interrupt enable/disable. */
    volatile unsigned char ien;
} Ic;

/* Individual interrupt line control registers - 
   index with interrupt number e.g. irq[3] for interrupt line 3 */
typedef struct Irq
{
    unsigned char _fill1;
    unsigned char _fill0;

    /* Priority for this interrupt line */
    volatile unsigned char ipl;

    /* Enable this interupt line */
    volatile unsigned char ien;
} Irq;

#else
/* Little endian version */

/* Interrupt controller */
typedef struct Ic
{
    /* Global interrupt enable/disable. */
    volatile unsigned char ien;

    /* Interrupt pending */
    volatile unsigned char irq;

    /* Global priority level - interrupt must have higher priority
       than this to be taken into account */
    volatile unsigned char cpl;

    /* Priority of last accepted interrupt - this is only
       valid after an interrupt! */
    volatile unsigned char ipl;

} Ic;

/* Individual interrupt line control registers - 
   index with interrupt number e.g. irq[3] for interrupt line 3 */
typedef struct Irq
{
    /* Enable this interupt line */
    volatile unsigned char ien;

    /* Priority for this interrupt line */
    volatile unsigned char ipl;
    unsigned char _fill0;
    unsigned char _fill1;
} Irq;
#endif

#define ic   ((Ic*)SFRADR_IC)
#define ic1  ((Ic*)SFRADR_IC1)
#define irq  ((Irq*)SFRADR_IC)
#define irq1 ((Irq*)SFRADR_IC1)

/* Define an interrupt handler to deal with interrupt number x.
   This should be preceded by void and followed by the body of the 
   handler between { } 
   This interrupt handler will save and restore the registers which
   are used in the interrupt handler body. */
#define interrupt_handler(x) \
    __attribute__((interrupt,used,noinline)) _concat (interrupt,x,_handler)(void)

/* Define an interrupt handler to deal with interrupt number x.
   This should be preceded by void and followed by the body of the 
   handler between { } 
   This interrupt handler will not save and restore any registers
   at all. 
   This is used with certain RTOS'es such as FreeRTOS */
#define naked_interrupt_handler(x) \
    __attribute__((naked,used,noinline)) _concat (interrupt,x,_handler)(void)

#define _concat(x,y,z) x ## y ## z

#endif


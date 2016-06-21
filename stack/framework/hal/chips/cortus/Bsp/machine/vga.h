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

#ifndef _VGA_H
#define _VGA_H
#include <machine/sfradr.h>
#include <stdio.h>

typedef struct VGA
{
    /* VGA status - 1 on, 0 off */
    volatile unsigned enable;
    volatile unsigned invert;
    /* VGA background and foreground colors, 3 bits */
    volatile unsigned background;
    volatile unsigned foreground;
} VGA;

#ifdef __APS__
#define vga1 ((VGA *)SFRADR_VGA)
#else
extern VGA __vga1;
#define vga1 (&__vga1)
#endif

#ifdef __APS__
#define vga_mem ((volatile unsigned char*)SFRADR_VGA_RAM)
#else
extern volatile unsigned char __vga_mem[];
#define vga_mem (__vga_mem)
#endif

typedef struct VGA2
{
    /* VGA status - 1 on, 0 off */
    volatile unsigned enable;
    volatile unsigned invert;
    /* VGA look up table for colors, 12 bits. 
       The look up table consists of a table that compares the 
       value of the pixel (2 bits, says 0, 1 ,2, 3) to select which
       RGB line to activate - in order R-G-B - 4 colors available */
    volatile unsigned table;
} VGA2;

#ifdef __APS__
#define vga2 ((VGA2 *)SFRADR_VGA)
#else
extern VGA2 __vga2;
#define vga2 (&__vga2)
#endif

/* Write character 'ch' at given line and column on screen.
   The lines are numbered from 0 at the top to 36 for the bottom.
   The columns are numbered from 0 at the left to 99 at the right */
void vga_putc(int line, int col, int ch);

/* Clear entire screen */
void vga_clear_screen(void);

/* Scroll one line of characters up */
void vga_scroll_line(void);

/* Goto given position on screen */
void vga_goto(int line, int col);

/* Stdio FILE for use as argument to fprintf etc to output to screen */
extern FILE* vga_out;

#endif

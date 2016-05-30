/*********************************************************************************
 * This confidential and proprietary software may be used only as authorized 
 *                      by a licensing agreement from                           
 *                           Cortus S.A.
 *
 *                   (C) Copyright 2006 Cortus S.A.
 *                         ALL RIGHTS RESERVED
 *
 * The entire notice above must be reproduced on all authorized copies
 * and any such reproduction must be pursuant to a licensing agreement 
 * from Cortus S.A. (http://www.cortus.com)
 *
 * $CortusRelease$
 * $FileName$
 *
 *********************************************************************************/

#include <stdio.h>

/* Uart input and output routines for stdio */
extern void uart1_outch (void* f, int c);
extern int uart1_inch (void* f);

static const FILE file_uart1_out = { {.out = {uart1_outch}}};
static FILE file_uart1_in = { {.in = {uart1_inch, -1}}};

FILE* stdout = (FILE*) &file_uart1_out;
FILE* stderr = (FILE*) &file_uart1_out;
FILE* stdin = (FILE*) &file_uart1_in;


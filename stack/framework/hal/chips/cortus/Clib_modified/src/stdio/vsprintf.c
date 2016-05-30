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

#include <stdio.h>
#include <stdarg.h>

typedef struct sFILE {
    void (*outch) (FILE* f, int c);
    char* buffer;
} sFILE;

static void sprintf_outch (FILE* f, int c)
{
    sFILE* sf = (sFILE*) f;
    *sf->buffer++ = c;
}

int vsprintf (char* buffer, const char* format, va_list argp)
{
    int count;
    sFILE sf = {sprintf_outch, buffer};
    FILE* f = (void*) &sf;
    count = vfprintf (f, format, argp);
    *sf.buffer = 0;
    return count;
}

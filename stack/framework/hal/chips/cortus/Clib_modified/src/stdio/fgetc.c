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

int fgetc (FILE* f)
{
    int c;
    if (f->in.ungetc >= 0) {
        c = f->in.ungetc;
        f->in.ungetc = -1;
        return c;
    }
    c = (*f->in.inch) (f);
    return c;
}


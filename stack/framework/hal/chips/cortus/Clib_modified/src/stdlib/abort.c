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

#include <stdlib.h>
#include <stdio.h>

#ifndef NDEBUG

void __abort (const char* filename, int lineno)
{
    fprintf (stderr, "abort at %s:%d\n", filename, lineno);

#ifdef __APS__
    /* Reboot system */
    exit (1);
#else
    abort();
#endif
}

#endif


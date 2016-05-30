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

#include <string.h>

int memcmp (const void* s1, const void* s2, size_t n)
{
    while (n > 0) {
        int c1 = *(signed char *) s1++;
        int c2 = *(signed char *) s2++;
        int d = c1 - c2;
        if (d)
            return d;
        n--;
    }
    return 0;
}

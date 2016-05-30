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

void* memmove (void *out, const void *in, size_t nbytes)
{
    char* dest = out;
    const char* src = in;

    if (dest < src) {
        while (nbytes > 0) {
            nbytes--;
            *dest++ = *src++;
        }
    } else {
        dest += nbytes;
        src += nbytes;
        while (nbytes > 0) {
            nbytes--;
            *--dest = *--src;
        }
    }
    return out;
}

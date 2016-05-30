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

char* strncpy (char* dst, const char* src, size_t len)
{
    char* out = dst;

    while (*src && len > 0) {
        *out++ = *src++;
        len--;
    }

    while (len > 0) {
        *out++ = '\0';
        len--;
    }
    return dst;
}


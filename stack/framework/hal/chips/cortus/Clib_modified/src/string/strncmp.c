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

int strncmp (const char *s1, const char *s2, size_t len)
{
    while (*s1 != '\0' && *s1 == *s2 && len > 0) {
        s1++;
        s2++;
        len--;
    }
    if (len == 0)
        return 0;
    return (*(unsigned char *) s1) - (*(unsigned char *) s2);
}


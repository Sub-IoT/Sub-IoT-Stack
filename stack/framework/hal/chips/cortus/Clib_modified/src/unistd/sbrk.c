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
#include <errno.h>
#include <sys/heap.h>

extern char __heap_bottom[];
extern char __heap_top[];
char * __heap_marker = __heap_bottom;

void* sbrk(int nbytes)
{
    char * old_heap_marker = __heap_marker;
    nbytes = (nbytes+3)&~3;

    __heap_marker += nbytes;
    if (__heap_marker <= __heap_top) {
        return old_heap_marker;
    }
    __heap_marker = old_heap_marker;
    errno = ENOMEM;
    return (void*)-1;
}

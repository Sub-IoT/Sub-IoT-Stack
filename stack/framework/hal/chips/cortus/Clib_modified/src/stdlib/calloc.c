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
#include <string.h>

void *aps_calloc (size_t nelems, size_t elem_sz)
{
    void *ptr = aps_malloc (nelems * elem_sz);
    memset (ptr, 0, nelems * elem_sz);
    return ptr;
}

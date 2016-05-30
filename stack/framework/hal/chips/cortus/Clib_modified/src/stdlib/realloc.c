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

void* aps_realloc (void* p, size_t nbytes)
{
    void* new = aps_malloc (nbytes);
    if (!new)
        return 0;

    /* This is not very clean since it will copy junk after the end of p
     but this does not worry us too much */
    memcpy (new, p, nbytes);

    free(p);
    return new;
}

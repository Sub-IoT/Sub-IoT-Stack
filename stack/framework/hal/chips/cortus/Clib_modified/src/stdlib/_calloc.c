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

/* Function definitions for ANSI compatability.
 *
 * These are needed in case the user did not include
 * the header files and ignored the warnings....
 */

#include <stdlib.h>

#ifdef __APS__

#undef calloc
void *calloc (size_t nelems, size_t elem_sz)
{
#ifdef DEBUG_HEAP
    return aps_calloc(nelems, elem_sz, __FILE__,__LINE__);
#else
    return aps_calloc (nelems, elem_sz);
#endif
}

#endif

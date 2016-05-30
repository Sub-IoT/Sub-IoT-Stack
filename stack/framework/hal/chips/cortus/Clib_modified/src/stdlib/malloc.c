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
#include <stdlib.h>
#include <unistd.h>
#include <sys/heap.h>
#include <assert.h>

void* aps_malloc (size_t nbytes)
{
    Heap_Region* r;
    int i;
    for (i = 0, r = __heap_regions; i < __heap_nregions; i++, r++) {
        if (nbytes <= __heap_region_sizes[i]) {
            /* Fit for this region */
            Heap_Block *b = r->free_list;
            if (!b) {
                /* Nothing in free list, allocate from raw heap */
                b = sbrk (sizeof(int) + __heap_region_sizes[i]);
                if (b == (void*) -1) {
                    /* Nothing left in raw heap - try next bigger region */
                    break;
                }
                /* Assign it to this region */
                b->ri = i;
                b->next = 0;
            }
            r->free_list = b->next;
            r->nused++;
            if (r->nused > r->max_nused)
                r->max_nused = r->nused;
            return ((char *) b) + sizeof(int);
        }
    }
    return 0;
}

void aps_free (void *p)
{
    Heap_Block* b = (Heap_Block*) (void *) (((char*) p) - sizeof(int));
    Heap_Region* r = &__heap_regions[b->ri];
    b->next = r->free_list;
    r->free_list = b;
    r->nused--;
}

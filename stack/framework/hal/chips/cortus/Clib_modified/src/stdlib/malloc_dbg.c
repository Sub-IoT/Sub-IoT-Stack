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

#define DEBUG_HEAP
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/heap.h>
#include <assert.h>

void* aps_malloc_dbg (size_t nbytes, const char* filename, int lineno)
{
    Heap_Dbg_Region* r;
    int i;

    for (i = 0, r = __heap_dbg_regions; i < __heap_nregions; i++, r++) {
        if (nbytes <= __heap_region_sizes[i]) {
            /* Fit for this region */
            Heap_Dbg_Block *b = r->free_list;
            if (!b) {
                /* Nothing in free list, allocate from raw heap_dbg */
                b = sbrk (sizeof(Heap_Dbg_Block) + __heap_region_sizes[i]);
                if (b == (void*) -1) {
                    /* Nothing left in raw heap - try next bigger region */
                    break;
                }
                /* Assign it to this region */
                b->ri = i;
                b->next = 0;
            }
            r->free_list = b->next;
            b->next = r->allocated_list;
            r->allocated_list = b;
            b->src_filename = filename;
            b->src_lineno = lineno;
            r->nused++;
            if (r->nused > r->max_nused)
                r->max_nused = r->nused;
            return ((char *) b) + sizeof(Heap_Dbg_Block);
        }
    }
    return 0;
}

void aps_free_dbg (void *p, const char* filename, int lineno)
{
    Heap_Dbg_Block* b = (Heap_Dbg_Block*) (void *) (((char*) p)
            - sizeof(Heap_Dbg_Block));
    Heap_Dbg_Block** pb;
    Heap_Dbg_Region* r = &__heap_dbg_regions[b->ri];

    /* Find pointer which points to this block */
    for (pb = &(r->allocated_list); *pb != b && *pb; pb = &((*pb)->next))
        ;

    /* Must be on the allocated list */
    assert(*pb);

    /* Remove 'b' from list and close gap */
    *pb = b->next;

    /* Put 'b' onto free list */
    b->next = r->free_list;
    b->src_filename = filename;
    b->src_lineno = lineno;
    r->free_list = b;

    /* Update stats */
    r->nused--;
}


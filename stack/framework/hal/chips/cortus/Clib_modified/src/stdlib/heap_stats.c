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
#include <sys/heap.h>
#include <stdio.h>

extern char __heap_bottom[];
extern char __heap_top[];
extern char * __heap_marker;

void fprint_heap_stats (FILE* f)
{
    Heap_Region* r;
    int i;
    int heap_size = (int) (__heap_top - __heap_bottom);
    int used = (int) (__heap_marker - __heap_bottom);

    fprintf (f, "\nHeap usage: %d bytes of %d\n", used, heap_size);
    for (i = 0, r = __heap_regions; i < __heap_nregions; i++, r++) {
        fprintf (f, "%5d byte blocks: peek %3d, current %3d\n",
            __heap_region_sizes[i], r->max_nused, r->nused);
    }
    fprintf (f, "\n");
}

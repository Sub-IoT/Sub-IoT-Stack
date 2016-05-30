/*********************************************************************************
 * This confidential and proprietary software may be used only as authorized
 *                      by a licensing agreement from
 *                           Cortus S.A.
 *
 *             (C) Copyright 2004, 2005, 2006, 2009 Cortus S.A.
 *                           ALL RIGHTS RESERVED
 *
 * The entire notice above must be reproduced on all authorized copies
 * and any such reproduction must be pursuant to a licensing agreement
 * from Cortus S.A. (http://www.cortus.com)
 *
 * Example memory configuration file.
 *
 *
 *********************************************************************************/

#include <sys/heap.h>
#include <machine/memsize.h>

/* Define memory sizes.
   These are the same as the defaults which are provided by the default linker script
*/
STACK_SIZE(64*1024);
HEAP_SIZE(64*1024);
PROGRAM_MEMORY_SIZE(512*1024);
DATA_MEMORY_SIZE(512*1024);


/* If the heap is not used, then set HEAP_SIZE to 0 above and the rest of this file
   can then be removed.

   This is the configuration for the Cortus non fragmenting heap is compiled
   into libc.

   If you are using the heap, you will certainly want to customize the
   HEAP_REGION_SIZES and link this into your program.

   Objects are allocated on the heap in one of a number of regions depending on
   their size. When an object is free'd it is returned to the free list of the
   relevant region.

   If a request in a region cannot be satisfied by that region, then a new chunk
   is allocated via sbrk to satifsy that request. If no more memory can be
   allocated via sbrk, then the next bigger regions are used.
*/

/* This macro defines the size of each of the regions in bytes (which must
   be a multiple of 4). These sizes must be in ascending sequence.

   These sizes correspond to the usable size of the allocated structure.
   The heap management adds one word to this (no debug) or several (debug enabled).
   NB a malloc with nbytes greater than the last value in this array will always
   fail.

   The first element in this array corresponds to the size of a dynamically a
   allocated doubly linked list cell.
*/
#define HEAP_REGION_SIZES 12,16,32,64,128,256,512,1024,2*1024,4*1024,8*1024,16*1024

const unsigned short __heap_region_sizes[] = {HEAP_REGION_SIZES};

/* The number of separate heap regions */
#define HEAP_NREGIONS  (sizeof(__heap_region_sizes)/2)

const unsigned __heap_nregions = HEAP_NREGIONS;

/* Which of these is used, depends on whether the user code is compiled with
   DEBUG_HEAP defined or not.

   DEBUG_HEAP should be defined for all or none of the code and should not be
   mixed and matched.

   The unused definition will be removed from the executable if -fdata-sections and
   --gc-sections are used during compilation and linking respectively.
*/
Heap_Dbg_Region __heap_dbg_regions[HEAP_NREGIONS] = {{0}};
Heap_Region __heap_regions[HEAP_NREGIONS] = {{0}};


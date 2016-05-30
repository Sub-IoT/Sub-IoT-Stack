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
 * $CortusRelease$
 * $FileName$
 *
 *********************************************************************************/

#include <sys/heap.h>

/* This is the configuration for the Cortus non fragmenting heap that is compiled 
   into libc.

   If you are using the heap, you will certainly want to customize this and 
   link it into your program. Note that the total heap size is specified as 
   a parameter to the linker using 
      --defsym _HEAP_SIZE=xxx
   or alternatively by compiling into the program somewhere a definition for 
   the symbol _HEAP_SIZE. (See sys/memsize.h on how to do this).
   
   The stack size can be specified in a similiar way.
   
   Objects are allocated on the heap in one of a number of regions depending on 
   their size. When an object is free'd it is returned to the free list of the
   relevant region.

   If a request in a region cannot be satisfied by that region, then a new chunk
   is allocated via sbrk to satifsy that request. If no more memory can be 
   allocated via sbrk, then the next bigger regions are used.
*/

/*
   This next macro defines the size of each of the regions in bytes (which must
   be multiple a of 4). These sizes must be in ascending sequence.

   These sizes correspond to the useable size of the allocated structure. 
   The heap management adds one word to this (no debug) or several (debug enabled). 
   NB a malloc with nbytes greater than the last value in this array will always
   fail. 

   The first element in this array corresponds to the size of a dynamically a
   allocated doubly linked list cell.

*/
#define HEAP_REGION_SIZES 12,16,32,64,128,256,512,1024,2*1024,4*1024,8*1024,16*1024

const unsigned short __heap_region_sizes[] __attribute__((weak)) = {HEAP_REGION_SIZES};

/* The number of separate heap regions */
#define HEAP_NREGIONS sizeof(__heap_region_sizes)/2

const unsigned __heap_nregions __attribute__((weak)) = HEAP_NREGIONS;

/* Which of these is used, depends on whether the user code is compiled with 
   DEBUG_HEAP defined or not.

   DEBUG_HEAP should be defined for all or none of the code and should not be
   mixed and matched. 
*/
Heap_Dbg_Region __heap_dbg_regions[HEAP_NREGIONS] __attribute__((weak)) = {{0}};
Heap_Region __heap_regions[HEAP_NREGIONS] __attribute__((weak)) = {{0}};

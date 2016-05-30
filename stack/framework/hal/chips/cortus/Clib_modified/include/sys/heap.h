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

#ifndef _SYS_HEAP_H
#define _SYS_HEAP_H

typedef struct Heap_Dbg_Block Heap_Dbg_Block;
typedef struct Heap_Dbg_Region Heap_Dbg_Region;

typedef struct Heap_Block Heap_Block;
typedef struct Heap_Region Heap_Region;

struct Heap_Dbg_Region {
    /* Number of currently used from this region */
    short nused;

    /* Maximum used from this region */
    short max_nused;

    /* Lists for free and allocated blocks.
       All blocks are on one or the other of the lists */
    Heap_Dbg_Block* free_list;

    /* allocated list */
    Heap_Dbg_Block* allocated_list;
};

struct Heap_Dbg_Block
{
    /* Where allocated */
    const char* src_filename;
    unsigned short src_lineno;

    /* Index to indicate which region is used */
    short ri;

    /* Next block in chain (free list or allocated list) */
    Heap_Dbg_Block* next;
};

struct Heap_Region {
    /* Number of currently used from this region */
    short nused;

    /* Maximum used from this region */
    short max_nused;

    /* region free list */
    Heap_Block* free_list;
};

struct Heap_Block
{
    /* Index to indicate which region is used */
    int ri;

    /* Next in free list - this is overwritten by user
       data once allocated */
    Heap_Block* next;
};

/* Only one of these should be used in an executable */
extern Heap_Dbg_Region __heap_dbg_regions[];
extern Heap_Region __heap_regions[];

/* These are common to debug/nodebug */
extern char __heap[];
extern int __heap_high_water;
extern const unsigned short __heap_region_sizes[];
extern const unsigned __heap_total_size;
extern const unsigned __heap_nregions;
#endif

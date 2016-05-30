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

#ifndef _STDLIB_H
#define _STDLIB_H

#define __need_size_t
#include <stddef.h>

#ifndef NULL
#define NULL 0
#endif

#ifdef NDEBUG
void    abort(void) __attribute__ ((noreturn));
#else
void    __abort(const char* filename, int lineno) __attribute__ ((noreturn));
#define abort() __abort(__FILE__,__LINE__)
#endif

void    exit(int code) __attribute__ ((noreturn));

int     atoi(const char *s);
long    atol(const char *s);

/* malloc, free and realloc are defined by macros to be aps_malloc,
   aps_free and aps_realloc to avoid problems when trying to run on a PC
   or other platform which insists on supplying malloc, free and co. 
   e.g. for shared library stuff 
*/
#ifdef DEBUG_HEAP
#define malloc(nbytes) aps_malloc_dbg((nbytes),__FILE__,__LINE__)
void*   aps_malloc_dbg(size_t nbytes, const char* filename, int lineno);
#else
#define malloc(nbytes) aps_malloc(nbytes)
void*   aps_malloc(size_t nbytes);
#endif

#ifdef DEBUG_HEAP
#define calloc(nelems, elem_sz) aps_calloc_dbg((nelems), (elem_sz), __FILE__, __LINE__)
void*   aps_calloc_dbg(size_t nelems, size_t elem_sz, const char* filename, int lineno);
#else
#define calloc(nelems, elem_sz) aps_calloc((nelems), (elem_sz))
void*   aps_calloc(size_t nelems, size_t elem_sz);
#endif

/* Only pass pointers obtained from malloc or realloc */
#ifdef DEBUG_HEAP
#define free(p) aps_free_dbg((p), __FILE__, __LINE__)
void    aps_free_dbg(void* p, const char* filename, int lineno);
#else
#define free(p) aps_free(p)
void    aps_free(void* p);
#endif

#ifdef DEBUG_HEAP
#define realloc(p,nbytes) aps_realloc_dbg((p),(nbytes), __FILE__, __LINE__)
void*   aps_realloc_dbg(void* p, size_t nbytes, const char* filename, int lineno);
#else
#define realloc(p,nbytes) aps_realloc((p),(nbytes))
void*   aps_realloc(void* p, size_t nbytes);
#endif

#ifdef DEBUG_HEAP
#define fprint_heap_stats fprint_heap_dbg_stats
#endif

#endif


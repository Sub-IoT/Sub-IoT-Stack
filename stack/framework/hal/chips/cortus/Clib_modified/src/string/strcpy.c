/*********************************************************************************
 * $CortusRelease$
 * $FileName$
 *
 *********************************************************************************/

#include <string.h>
#include <limits.h>

/* Nonzero if either X or Y is not aligned on a "long" boundary.  */
#define UNALIGNED(X, Y) \
  (((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)))

#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)

char *strcpy (char *dst0, const char *src0)
{
    char *dst = dst0;
    const char *src = src0;
    long *aligned_dst;
    const long *aligned_src;

    /* If SRC or DEST is unaligned, then copy bytes.  */
    if (!UNALIGNED (src, dst)) {
        aligned_dst = (long*) (void *) dst;
        aligned_src = (long*) (void *) src;

        /* SRC and DEST are both "long int" aligned, try to do "long int"
         sized copies.  */
        while (!DETECTNULL(*aligned_src)) {
            *aligned_dst++ = *aligned_src++;
        }

        dst = (char*) aligned_dst;
        src = (char*) aligned_src;
    }

    while ((*dst++ = *src++))
        ;
    return dst0;
}

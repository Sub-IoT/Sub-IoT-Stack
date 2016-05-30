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

#ifndef _STRING_H_
#define _STRING_H_

#define __need_size_t
#include <stddef.h>

#ifndef NULL
#define NULL 0
#endif

char *   memchr(const char *, int, size_t);
int      memcmp(const void *, const void *, size_t);
void *   memcpy(void *, const void *, size_t);
void *   memmove(void *, const void *, size_t);
void *   memset(void  *, int, size_t);
char *   strcat(char *, const char *);
char *   strchr(const char *, int);
int      strcmp(const char *, const char *);
int      strcoll(const char *, const char *);
char *   strcpy(char *, const char *);
size_t   strcspn(const char *, const char *);
char *   strerror(int);
size_t   strlen(const char *);
char *   strncat(char *, const char *, size_t);
int      strncmp(const char *, const char *, size_t);
char *   strncpy(char *, const char *, size_t);
char *   strpbrk(const char *, const char *);
char *   strrchr(const char *, int);
size_t   strspn(const char *, const char *);
char *   strstr(const char *, const char *);

#endif /* _STRING_H_ */

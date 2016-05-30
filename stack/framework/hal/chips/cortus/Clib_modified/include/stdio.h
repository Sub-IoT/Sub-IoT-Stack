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

#ifndef _STDIO_H
#define _STDIO_H

typedef struct __FILE FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

#define __need_size_t
#include <stddef.h>

#define __need___va_list
#include <stdarg.h>
#define __VALIST __gnuc_va_list

#ifndef NULL
#define NULL 0
#endif

int     fflush(FILE *);
int     fprintf(FILE *, const char *, ...);
int     sprintf(char *, const char *, ...);
int     fscanf(FILE *, const char *, ...);
int     printf(const char *, ...);
int     scanf(const char *, ...);
int     sscanf(const char *, const char *, ...);
int     vfprintf(FILE *, const char *, __VALIST);
int     vprintf(const char *, __VALIST);
int     vsprintf(char *, const char *, __VALIST);
int     fgetc(FILE *);
#define getc(f) fgetc(f)
#define getchar() getc(stdin)
char *  fgets(char *, int, FILE *);
int     fputc(int, FILE *);
#define putc(c,f) fputc(c,f)
int     putchar(int);
int     fputs(const char *, FILE *);
char *  gets(char *);
int     puts(const char *);
int     ungetc(int, FILE *);
size_t  fread(char * , size_t _size, size_t _n, FILE *);
size_t  fwrite(const char * , size_t _size, size_t _n, FILE *);

/* End of file character. */
#ifndef EOF
#define EOF (-1)
#endif

/* FILE type.
   There are different variations on this for different types of stream IO -
   but all have the inch or outch function as the first member. */
struct __FILEout {
    void (*outch) (void* f, int c);
};

struct __FILEin {
    int (*inch) (void* f);
    
    /* character pushed back by ungetc or -1 if none */
    int ungetc;
};
    
struct __FILE {
    union {
	struct __FILEout out;
	struct __FILEin  in;
    };
};

#endif

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

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Prototypes */
static void utod (char* buffer, unsigned long x);
static void utoh (char* buffer, unsigned long x, int upper);
static unsigned scandec (const char** s);

/* Limited base function version for the entire [.*]printf family of functions.
 *
 * Format is %[flags][width][.prec][size][type]
 *
 * The '#' flag is not supported.
 *
 * Note that the behaviour when strange combinations of flags and types are used
 * will probably differ from standard implementations.
 * (e.g. %+s will precede string value by a '+'sign). 
 * 
 * All ANSI defined normal usages should produce the same results as required by 
 * the ANSI standard.
 *
 * Width and precision can be specified as '*'.
 * 
 * The size flags accepted are 'l' and 'h' - both have no effect (long is the
 * same length as int, and shorts are promoted to int - so for normal usage this
 * is the correct behaviour).
 * 
 * Supported types are 'c', 's', 'd', 'u', 'i', 'x', 'X', 'p' with the usual meanings.
 * 'p' is equivalent to '08x'.
 *
 * The floating point formats 'e' 'E' 'g' 'G' are not supported.
 * The 'n' format is not supported either - but when did you use that one???
 */
int vfprintf (FILE* f, const char* format, va_list argp)
{
    int count = 0; /* count of characters transferred */
    const char* fp;

    enum Flags {
        flag_left = 1,
        flag_plus = 2,
        flag_blank = 4,
        flag_zero = 8
    };

    for (fp = format; *fp != 0; fp++) {
        if (fp[0] == '%' && fp[1] != 0) {
            int len;
            int flags = 0;
            int width = 0;
            int precision = -1;
            int lsz;

            /* Sign character to be output for conversion */
            char sign = 0;

            /* Fill character to use */
            char fill = ' ';

            /* Buffer for conversions */
            char buffer[20];

            /* Start of converted string to be output */
            char* s = buffer;

            /* Process flags */
            int valid = 1;
            while (valid) {
                char c = *++fp;
                /* Process flags */
                switch (c) {
                case '-':
                    flags |= flag_left;
                    break;
                case '+':
                    flags |= flag_plus;
                    sign = '+';
                    break;
                case ' ':
                    flags |= flag_blank;
                    sign = ' ';
                    break;
                case '0':
                    flags |= flag_zero;
                    fill = '0';
                    break;
                default:
                    valid = 0;
                    break;
                }
            }

            /* Process width */
            if (*fp == '*') {
                fp++;
                width = va_arg(argp,int);
            } else {
                width = scandec (&fp);
            }
            if (width < 0) {
                width = -width;
                flags |= flag_left;
            }

            /* Process precision (only for fixed point and strings) */
            if (*fp == '.') {
                fp++;
                if (*fp == '*') {
                    fp++;
                    precision = va_arg(argp,int);
                } else {
                    precision = scandec (&fp);
                }
            }

            lsz = 0;

            /* skip over size */
            if (*fp == 'l' || *fp == 'h')
                fp++;

            /* Process type */
            switch (*fp) {
            case '%': {
                buffer[0] = '%';
                buffer[1] = 0;
                break;
            }

            case 'c': {
                /* char is promoted to int */
                char ch = va_arg(argp,int);
                buffer[0] = ch;
                buffer[1] = 0;
                break;
            }

            case 's': {
                s = va_arg(argp,char*);
                break;
            }

            case 'i':
            case 'd': {
                long x = lsz ? va_arg(argp,long) : va_arg(argp,int);
                if (x < 0) {
                    sign = '-';
                    x = -x;
                }
                utod (buffer, x);
                break;
            }

            case 'u': {
                unsigned long x =
                        lsz ? va_arg(argp,unsigned long) : va_arg(argp,unsigned);
                utod (buffer, x);
                break;
            }

            case 'p':
                width = 8;
                fill = '0';
                /* no break */
            case 'x': {
                unsigned long x =
                        lsz ? va_arg(argp,unsigned long) : va_arg(argp,unsigned);
                utoh (buffer, x, 0);
                break;
            }

            case 'X': {
                unsigned long x =
                        lsz ? va_arg(argp,unsigned long) : va_arg(argp,unsigned);
                utoh (buffer, x, 1);
                break;
            }

            default:
                buffer[0] = *fp;
                buffer[1] = 0;
                break;
            }

            len = strlen (s);
            if (*fp == 's' && precision >= 0) {
                if (len > precision)
                    len = precision;
            }
            if (sign)
                width--;

            if (flags & flag_left) {
                /* Pad on right */
                int i;
                fill = ' ';
                if (sign) {
                    (*f->out.outch) (f, sign);
                    count++;
                }
                for (i = 0; i < len; i++) {
                    (*f->out.outch) (f, *s++);
                    count++;
                }
                for (; i < width; i++) {
                    (*f->out.outch) (f, fill);
                    count++;
                }
            } else {
                int i;
                /* Pad on left */
                if (fill != ' ' && sign) {
                    (*f->out.outch) (f, sign);
                    count++;
                }
                for (i = len; i < width; i++) {
                    (*f->out.outch) (f, fill);
                    count++;
                }
                if (fill == ' ' && sign) {
                    (*f->out.outch) (f, sign);
                    count++;
                }
                for (i = 0; i < len; i++) {
                    (*f->out.outch) (f, *s++);
                    count++;
                }
            }
        } else {
            (*f->out.outch) (f, *fp);
            count++;
        }
    }
    return count;
}

/* Convert unsigned value x to decimal string in buffer.
 Buffer must be at least 11 characters long */
static void utod (char* buffer, long unsigned x)
{
    static const unsigned long q[] = {1000000000L, 100000000L, 10000000L,
            1000000L, 100000L, 10000L, 1000L, 100L, 10L, 1L};
    if (x == 0) {
        *buffer++ = '0';
    } else {
        int i;
        int significant = 0;

        for (i = 0; i < 10; i++) {
            unsigned long qq = q[i];
            char digit = 0;
            while (x >= qq) {
                x -= qq;
                digit++;
            }
            if (significant || digit != 0) {
                significant = 1;
                *buffer++ = digit + '0';
            }
        }
    }
    *buffer = 0;
}

/* Convert unsigned value x to hex string in buffer.
 Buffer must be at least 9 characters long.
 upper indicates that upper case letters should be used */
static void utoh (char* buffer, unsigned long x, int upper)
{
    if (x == 0) {
        *buffer++ = '0';
    } else {
        int i;
        int significant = 0;

        for (i = 0; i < 32; i += 4) {
            char digit = (x >> (32 - 4 - i)) & 15;
            if (significant || digit != 0) {
                unsigned char ch;
                significant = 1;
                if (digit > 9) {
                    ch = digit - 10 + (upper ? 'A' : 'a');
                } else {
                    ch = digit + '0';
                }
                *buffer++ = ch;
            }
        }
    }
    *buffer = 0;
}

/* Convert character string pointed to by the pointer 
 pointed to by s to decimal and return that value
 as well as updating the pointer over scanned characters.
 TODO: replace with more standard function */
static unsigned scandec (const char** s)
{
    unsigned result = 0;
    unsigned c;

    c = **s;
    while ('0' <= c && c <= '9') {
        (*s)++;
        /* Be careful about evaluation order to avoid overflow if
         at all possible */
        result *= 10;
        c -= '0';
        result += c;
        c = **s;
    }
    return result;
}


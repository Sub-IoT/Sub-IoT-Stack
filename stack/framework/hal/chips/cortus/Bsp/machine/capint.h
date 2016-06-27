/*******************************************************************************
 * File: capint.h
 * @section License
 * <b>(C) Copyright 2005 Cortus S.A, http://www.cortus.com
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Cortus S.A has no
 * obligation to support this Software. Cortus S.A is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Cortus S.A will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/

#ifndef _CAPINT_H
#define _CAPINT_H
#include <machine/sfradr.h>

typedef struct 
{
    /* Mode
       0 - high level sensitive
       1 - low level sensitive
       2 - positive edge sensitive
       3 - negative edge sensitive
       4 - both edge sensitive
    */
    volatile unsigned mode;
    volatile unsigned status;
    volatile unsigned mask;
} CAPINT;

#ifdef __APS__
#ifdef SFRADR_CAPINT0
#define capint0 ((CAPINT *)SFRADR_CAPINT0)
#endif
#ifdef SFRADR_CAPINT1
#define capint1 ((CAPINT *)SFRADR_CAPINT1)
#endif
#ifdef SFRADR_CAPINT2
#define capint2 ((CAPINT *)SFRADR_CAPINT2)
#endif
#ifdef SFRADR_CAPINT3
#define capint3 ((CAPINT *)SFRADR_CAPINT3)
#endif
#else
extern CAPINT __capint;
#define capint0 (&__capint)
#define capint1 (&__capint)
#define capint2 (&__capint)
#define capint3 (&__capint)
#endif
#endif

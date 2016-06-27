/*******************************************************************************
* File: gpio_int.h
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

#ifndef _GPIO_INT_H
#define _GPIO_INT_H
#include <machine/sfradr.h>

typedef struct GPio_int
{
    volatile unsigned out;
    volatile unsigned in;
    volatile unsigned dir;
    volatile unsigned old_in;
    volatile unsigned mask;
} GPio_int;
#ifdef __APS__
#define gpio_int ((GPio_int *)SFRADR_GPIO_INT)
#else
extern GPio_int __gpio_int;
#define gpio_int (&__gpio_int)
#endif
#endif

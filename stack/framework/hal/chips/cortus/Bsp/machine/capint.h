/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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

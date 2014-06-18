
/*
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2010, 2012, Roel Verdult, Armin van der Togt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 *   __exidx_start
 *   __exidx_end
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 */
// These are defined and created by the linker, locating them in memory
extern unsigned char __etext;
extern unsigned char __data_start__;
extern unsigned char __data_end__;
extern unsigned char __bss_start__;
extern unsigned char __bss_end__;


// Prototype the systemInit function
extern void SystemInit(void);
extern void SystemCoreClockUpdate(void);
// Prototype the required startup functions
extern void main(void);
// The entry point of the application, prepare segments,
// initialize the cpu and execute main()
void boot_entry(void)
{
  register unsigned char *src, *dst;

  // Get physical data address and copy it to sram
  src = &__etext;
  dst = &__data_start__;
  while(dst < &__data_end__) {
    *dst++ = *src++;
  }

  // Clear the bss segment
  dst = &__bss_start__;
  while(dst < &__bss_end__) {
    *dst++ = 0;
  }

  // Run SystemInit for initializing clock
  SystemInit();
  SystemCoreClockUpdate();

  // Execute the code at the program entry point
  main();

  // Do nothing when returned from main, just keep looping
  while(1);
}

#include "STM32L1xx_MD_Handlers.h"

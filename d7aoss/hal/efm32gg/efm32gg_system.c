/*! \file efm32gg_system.c
 *
 * \copyright (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.\n
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.\n
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 *  \author glenn.ergeerts@uantwerpen.be
 *
 */

#include <sys/stat.h>
#include <stdlib.h>

#include "system.h"
#include "uart.h"

#include "em_cmu.h"
#include "em_chip.h"

#define UDID_ADDRESS 0x1FF800A0

extern char _end;                 /**< Defined by the linker */

/**************************************************************************//**
 * @brief
 *  Increase heap. Needed for printf
 *
 * @param[in] incr
 *  Number of bytes you want increment the program's data space.
 *
 * @return
 *  Returns a pointer to the start of the new area.
 *****************************************************************************/
caddr_t _sbrk(int incr)
{
  static char       *heap_end;
  char              *prev_heap_end;

  if (heap_end == 0)
  {
    heap_end = &_end;
  }

  prev_heap_end = heap_end;
  if ((heap_end + incr) > (char*) __get_MSP())
  {
    exit(1);
  }
  heap_end += incr;

  return (caddr_t) prev_heap_end;
}


void system_init(uint8_t* tx_buffer, uint16_t tx_buffer_size, uint8_t* rx_buffer, uint16_t rx_buffer_size)
{
    /* Chip errata */
    CHIP_Init();

    //if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1) ;

    // init clock
    CMU_ClockDivSet(cmuClock_HF, cmuClkDiv_2);       // Set HF clock divider to /2 to keep core frequency < 32MHz
    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);   // Enable XTAL Osc and wait to stabilize
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO); // Select HF XTAL osc as system clock source. 48MHz XTAL, but we divided the system clock by 2, therefore our HF clock will be 24MHz

    led_init();
    //button_init();
    uart_init();
}

void system_watchdog_timer_stop()
{
    // TODO
}

void system_watchdog_timer_start()
{
    // TODO
}

void system_watchdog_timer_reset()
{
    // TODO
}

void system_watchdog_timer_enable_interrupt()
{
    // TODO
}

void system_watchdog_timer_init(unsigned char clockSelect, unsigned char clockDivider) // TODO refactor (params?)
{
    // TODO
}

void system_watchdog_init(unsigned char clockSelect, unsigned char clockDivider) // TODO refactor (params?)
{
    // TODO
}

void system_lowpower_mode(unsigned char mode, unsigned char enableInterrupts)
{
    // TODO
}

void system_get_unique_id(unsigned char *tagId)
{
    uint8_t* udid = (uint8_t*) UDID_ADDRESS;
    unsigned char i;
    for (i = 0; i < 8; i++)
    {
    	tagId[i] = udid[i];
    }
}


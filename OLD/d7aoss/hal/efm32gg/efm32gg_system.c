/*! \file efm32gg_system.c
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

    // TODO not hardware specific
    queue_init_with_header(&tx_queue, tx_buffer, tx_buffer_size, 1, 30);
    queue_init(&rx_queue, rx_buffer, rx_buffer_size, 1);
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


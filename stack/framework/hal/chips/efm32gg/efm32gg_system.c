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

/*! \file efm32gg_system.c
 *
 *
 */


#include "hwsystem.h"
#include "em_system.h"
#include "em_emu.h"
#include "em_cmu.h"
#include <assert.h>

void hw_enter_lowpower_mode(uint8_t mode)
{
    switch(mode)
    {
	case 0:
	{
	    EMU_EnterEM1();
	    break;	    
	}
	case 1:
	{
	    EMU_EnterEM2(true);
	    break;
	}
	case 2:
	{
	    EMU_EnterEM3(true);
	    break;
	}
	case 4:
	{
	    EMU_EnterEM4();
	    break;
	}
	default:
	{
	    assert(0);
	}
    }
}

uint64_t hw_get_unique_id()
{
    return SYSTEM_GetUnique();
}

void hw_busy_wait(int16_t microseconds)
{
    // note: uses core debugger cycle counter mechanism for now,
    // may switch to timer later if more accuracy is needed.
    uint32_t counter = microseconds * (CMU_ClockFreqGet(cmuClock_CORE) / 1000000);

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL        |= 1;
    DWT->CYCCNT       = 0;

    while (DWT->CYCCNT < counter) ;
}

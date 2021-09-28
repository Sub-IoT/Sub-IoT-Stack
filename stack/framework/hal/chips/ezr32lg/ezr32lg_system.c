/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
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
 */

/*! \file efm32gg_system.c
 *
 *
 */


#include "hwsystem.h"
#include "hwadc.h"
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
		//EMU_EnterEM2(false);
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


system_reboot_reason_t hw_system_reboot_reason()
{
  return REBOOT_REASON_NOT_IMPLEMENTED; // TODO
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

void hw_reset()
{
    NVIC_SystemReset();
}

// Factory calibration temperature (from device information page)
#define CAL_TEMP_0 (float)((DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK) >> _DEVINFO_CAL_TEMP_SHIFT)

// Factory ADC readout at CAL_TEMP_0 temperature (from device information page)
#define ADC_TEMP_0_READ_1V25 (float)((DEVINFO->ADC0CAL2 & _DEVINFO_ADC0CAL2_TEMP1V25_MASK) >> _DEVINFO_ADC0CAL2_TEMP1V25_SHIFT)

// temperature gradient (from datasheet)
#define T_GRAD -6.3f

float hw_get_internal_temperature()
{
  adc_init(adcReference1V25, adcInputSingleTemp, 400000);

  // TODO take into account warmup time
  uint32_t value = adc_read_single();

  return (CAL_TEMP_0 - ((ADC_TEMP_0_READ_1V25 - value)  / T_GRAD));
}


uint32_t hw_get_battery(void)
{
	adc_init(adcReference1V25, adcInputSingleVDDDiv3, 100);

	/* Manually set some calibration values */
	//ADC0->CAL = (0x7C << _ADC_CAL_SINGLEOFFSET_SHIFT) | (0x1F << _ADC_CAL_SINGLEGAIN_SHIFT);

	uint32_t vData;
	/* Sample ADC */
	uint32_t value = adc_read_single();

	vData = 3 * 1250 * (value / 4095.0);
	return vData;
}

//
// OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
// lowpower wireless sensor communication
//
// Copyright 2015 University of Antwerp
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "hwtimer.h"
#include "HalModule.h"
#include <assert.h>


__LINK_C error_t hw_timer_init(hwtimer_id_t timer_id, uint8_t frequency, timer_callback_t compare_callback, timer_callback_t overflow_callback)
{
    uint32_t ticks_per_sec;
    switch(frequency)
    {
    	case HWTIMER_FREQ_1MS:
    	{
    		ticks_per_sec = HWTIMER_TICKS_1MS;
    		break;
    	}
    	case HWTIMER_FREQ_32K:
		{
			ticks_per_sec = HWTIMER_TICKS_32K;
			break;
		}
    	default:
    	{
    		return EINVAL;
    	}
    }
	return HalModule::getActiveModule()->create_hwtimer(timer_id, 1.0/ticks_per_sec, compare_callback, overflow_callback);
}
__LINK_C hwtimer_tick_t hw_timer_getvalue(hwtimer_id_t timer_id)
{
	return HalModule::getActiveModule()->hw_timer_getvalue(timer_id);
}
__LINK_C error_t hw_timer_schedule(hwtimer_id_t timer_id, hwtimer_tick_t tick )
{
	return HalModule::getActiveModule()->hw_timer_schedule(timer_id, tick);
}
__LINK_C error_t hw_timer_cancel(hwtimer_id_t timer_id)
{
	return HalModule::getActiveModule()->hw_timer_cancel(timer_id);
}
__LINK_C error_t hw_timer_counter_reset(hwtimer_id_t timer_id)
{
	return HalModule::getActiveModule()->hw_timer_counter_reset(timer_id);
}
__LINK_C bool hw_timer_is_overflow_pending(hwtimer_id_t id)
{
    //since in omnet++ events happen 'instantaneos'
	//it is impossible for an interrupt to be 'pending'
	return false;
}
__LINK_C bool hw_timer_is_interrupt_pending(hwtimer_id_t id)
{
	//since in omnet++ events happen 'instantaneos'
	//it is impossible for an interrupt to be 'pending'
	return false;
}

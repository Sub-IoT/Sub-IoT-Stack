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

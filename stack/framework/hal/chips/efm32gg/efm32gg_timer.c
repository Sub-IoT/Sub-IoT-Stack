/*
 *  Created on: October 7, 2014
 *  Largely rewritten on: February 5, 2015
 *  Authors:
 *  	jeremie@wizzilab.com
 *		daniel.vandenakker@uantwerpen.be
 */
#include <stdbool.h>
#include <stdint.h>

#include "em_cmu.h"
#include "em_rtc.h"
#include "em_int.h"

#include "hwtimer.h"
#include "hwatomic.h"
#include "efm32gg_mcu.h"

/**************************************************************************//**
 * @brief  Start LFRCO for RTC
 * Starts the low frequency RC oscillator (LFRCO) and routes it to the RTC
 *****************************************************************************/
void startLfxoForRtc(uint8_t freq)
{
    /* Starting LFRCO and waiting until it is stable */
    CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

    /* Routing the LFRCO clock to the RTC */
    CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_LFXO);
    CMU_ClockEnable(cmuClock_RTC, true);

    /* Set Clock prescaler */
    if(freq == HWTIMER_FREQ_1MS)
    	CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_32);
    else
    	CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_1);

    /* Enabling clock to the interface of the low energy modules */
    CMU_ClockEnable(cmuClock_CORELE, true);
}

static timer_callback_t compare_f = 0x0;
static timer_callback_t overflow_f = 0x0;
static bool timer_inited = false;
/**************************************************************************//**
 * @brief Enables LFACLK and selects LFXO as clock source for RTC.
 *        Sets up the RTC to count at 1024 Hz.
 *        The counter should not be cleared on a compare match and keep running.
 *        Interrupts should be cleared and enabled.
 *        The counter should run.
 *****************************************************************************/
error_t hw_timer_init(timer_id_t timer_id, uint8_t frequency, timer_callback_t compare_callback, timer_callback_t overflow_callback)
{
    if(timer_id >= HWTIMER_NUM)
    	return ESIZE;
    if(timer_inited)
    	return EALREADY;
    if(frequency != HWTIMER_FREQ_1MS && frequency != HWTIMER_FREQ_32K)
    	return EINVAL;
	
    start_atomic();
		compare_f = compare_callback;
		overflow_f = overflow_callback;
		timer_inited = true;

		/* Configuring clocks in the Clock Management Unit (CMU) */
		startLfxoForRtc(frequency);

		RTC_Init_TypeDef rtcInit = RTC_INIT_DEFAULT;
		rtcInit.enable   = false;   /* Don't enable RTC after init has run */
		rtcInit.comp0Top = true;   /* Clear counter on compare 0 match: cmp 0 is used to limit the value of the rtc to 0xffff */
		rtcInit.debugRun = false;   /* Counter shall not keep running during debug halt. */


		/* Initialize the RTC */
		RTC_Init(&rtcInit);

		//disable all rtc interrupts while we're still configuring
		RTC_IntDisable(RTC_IEN_OF | RTC_IEN_COMP0 | RTC_IEN_COMP1);
		RTC_IntClear(RTC_IFC_OF | RTC_IFC_COMP0 | RTC_IFC_COMP1);
		//Set maximum value for the RTC
		RTC_CompareSet( 0, 0x0000FFFF );
		RTC_CounterReset();

		RTC_IntEnable(RTC_IEN_COMP0);

		NVIC_EnableIRQ(RTC_IRQn);
		RTC_Enable(true);
    end_atomic();
}

timer_tick_t hw_timer_getvalue(timer_id_t timer_id)
{
	if(timer_id >= HWTIMER_NUM || (!timer_inited))
		return 0;
	else
		return (uint16_t)(RTC->CNT & 0xFFFF);
}

error_t hw_timer_schedule(timer_id_t timer_id, timer_tick_t tick )
{
	if(timer_id >= HWTIMER_NUM)
		return ESIZE;
	if(!timer_inited)
		return EOFF;

	start_atomic();
	   RTC_IntDisable(RTC_IEN_COMP1);
	   RTC_CompareSet( 1, tick );
	   RTC_IntClear(RTC_IEN_COMP1);
	   RTC_IntEnable(RTC_IEN_COMP1);
	end_atomic();
}

error_t hw_timer_cancel(timer_id_t timer_id)
{
	if(timer_id >= HWTIMER_NUM)
		return ESIZE;
	if(!timer_inited)
		return EOFF;

	start_atomic();
	   RTC_IntDisable(RTC_IEN_COMP1);
	   RTC_IntClear(RTC_IEN_COMP1);
	end_atomic();
}

error_t hw_timer_counter_reset(timer_id_t timer_id)
{
	if(timer_id >= HWTIMER_NUM)
		return ESIZE;
	if(!timer_inited)
		return EOFF;

	start_atomic();
		RTC_IntDisable(RTC_IEN_COMP0 | RTC_IEN_COMP1);
		RTC_IntClear(RTC_IEN_COMP0 | RTC_IEN_COMP1);
		RTC_CounterReset();
		RTC_IntEnable(RTC_IEN_COMP0);
	end_atomic();

}

bool hw_timer_is_overflow_pending(timer_id_t id)
{
    if(timer_id >= HWTIMER_NUM)
	return false;
    start_atomic();
	//COMP0 is used to detect overflow
	bool is_pending = !!((RTC_IntGet() & RTC->IEN) & RTC_IFS_COMP0);
    end_atomic()
    return is_pending;	
}
bool hw_timer_is_interrupt_pending(timer_id_t id)
{
    if(timer_id >= HWTIMER_NUM)
	return false;

    start_atomic();
	bool is_pending = !!((RTC_IntGet() & RTC->IEN) & RTC_IFS_COMP1);
    end_atomic()
    return is_pending;	
}



INT_HANDLER(RTC_IRQHandler)
{
	//retrieve flags. We 'OR' this with the enabled interrupts
	//since the COMP1 flag may be set if it wasn't used before (compare register == 0 -> ifs flag set regardless of whether interrupt is enabled)
	//by AND ing with the IEN we make sure we only consider the flags of the ENABLED interrupts
	uint32_t flags = (RTC_IntGet() & RTC->IEN);
	RTC_IntClear(RTC_IFC_OF | RTC_IFC_COMP0 | RTC_IFC_COMP1);

	//evaluate flags to see which one(s) fired:
	if((flags & RTC_IFS_COMP0) && (overflow_f != 0x0))
		overflow_f();
	if((flags & RTC_IFS_COMP1))
	{
		RTC_IntDisable(RTC_IEN_COMP1);
		if(compare_f != 0x0)
			compare_f();
	}
}

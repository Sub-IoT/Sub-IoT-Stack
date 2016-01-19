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

/*! \file kl02z_timer.c
 *
 *  \author glenn.ergeerts@uantwerpen.be
 *
 */

#include <stdbool.h>
#include <stdint.h>


#include "hwtimer.h"
#include "hwatomic.h"
#include "MKL02Z4.h"
#include "fsl_tpm_hal.h"

/**************************************************************************//**
 * @brief  Start LFRCO for RTC
 * Starts the low frequency RC oscillator (LFRCO) and routes it to the RTC
 *****************************************************************************/
//void startLfxoForRtc(uint8_t freq)
//{
//    /* Starting LFRCO and waiting until it is stable */
//    CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
//
//    /* Routing the LFRCO clock to the RTC */
//    CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_LFXO);
//    CMU_ClockEnable(cmuClock_RTC, true);
//
//    /* Set Clock prescaler */
//    if(freq == HWTIMER_FREQ_1MS)
//    	CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_32);
//    else
//    	CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_1);
//
//    /* Enabling clock to the interface of the low energy modules */
//    CMU_ClockEnable(cmuClock_CORELE, true);
//}

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
error_t hw_timer_init(hwtimer_id_t timer_id, uint8_t frequency, timer_callback_t compare_callback, timer_callback_t overflow_callback)
{
// TODO
//    if(timer_id >= HWTIMER_NUM)
//    	return ESIZE;
//    if(timer_inited)
//    	return EALREADY;
//    if(frequency != HWTIMER_FREQ_1MS && frequency != HWTIMER_FREQ_32K)
//    	return EINVAL;
//    start_atomic();
//		compare_f = compare_callback;
//		overflow_f = overflow_callback;
//		timer_inited = true;
//
//		/* Configuring clocks in the Clock Management Unit (CMU) */
//		startLfxoForRtc(frequency);
//
//		RTC_Init_TypeDef rtcInit = RTC_INIT_DEFAULT;
//		rtcInit.enable   = false;   /* Don't enable RTC after init has run */
//		rtcInit.comp0Top = true;   /* Clear counter on compare 0 match: cmp 0 is used to limit the value of the rtc to 0xffff */
//		rtcInit.debugRun = false;   /* Counter shall not keep running during debug halt. */
//
//
//		/* Initialize the RTC */
//		RTC_Init(&rtcInit);
//
//		//disable all rtc interrupts while we're still configuring
//		RTC_IntDisable(RTC_IEN_OF | RTC_IEN_COMP0 | RTC_IEN_COMP1);
//		RTC_IntClear(RTC_IFC_OF | RTC_IFC_COMP0 | RTC_IFC_COMP1);
//		//Set maximum value for the RTC
//		RTC_CompareSet( 0, 0x0000FFFF );
//		RTC_CounterReset();
//
//		RTC_IntEnable(RTC_IEN_COMP0);
//
//		NVIC_EnableIRQ(RTC_IRQn);
//		RTC_Enable(true);
//    end_atomic();

    if(timer_id >= HWTIMER_NUM) return ESIZE;
    if(timer_inited) return EALREADY;
    if(frequency != HWTIMER_FREQ_1MS) return EINVAL;

    overflow_f = overflow_callback;
    compare_f = compare_callback;

    SIM_SOPT2 = (SIM_SOPT2&~SIM_SOPT2_TPMSRC_MASK)|SIM_SOPT2_TPMSRC(3);//select MCGIRCLK to reference (4MHz) // TODO use 1 kHz LPO input clock instead
    SIM_SCGC6 |= (1<<SIM_SCGC6_TPM0_SHIFT);//enable bus clock to module
    TPM0_CNT = 0x00;//clear count register
    TPM0_MOD = 0xFFFF;
    TPM0_SC = (1<<TPM_SC_TOF_SHIFT)|(0<<TPM_SC_TOIE_SHIFT)|(0<<TPM_SC_CPWMS_SHIFT)|(TPM_SC_CMOD(1)|TPM_SC_PS(5));//TOF cleared, interrupt disable, clock references "module clock,"
                                                                                                                 //prescale 4 MHz to 125000 Hz, timer counts up
    TPM_HAL_SetCpwms(TPM0, 1);
    TPM_HAL_EnableChnInt(TPM0, 0);
    MCG_C1 |= (1<<MCG_C1_IRCLKEN_SHIFT);//start reference clock

    NVIC_ClearPendingIRQ(TPM0_IRQn);
    NVIC_DisableIRQ(TPM0_IRQn);

    timer_inited = true;

    return SUCCESS;
}

hwtimer_tick_t hw_timer_getvalue(hwtimer_id_t timer_id)
{
    if(timer_id >= HWTIMER_NUM || (!timer_inited)) return 0;

    return TPM0_CNT;
}

error_t hw_timer_schedule(hwtimer_id_t timer_id, hwtimer_tick_t tick )
{
    if(timer_id >= HWTIMER_NUM) return ESIZE;
    if(!timer_inited) return EOFF;

    TPM0_CNT = 0x00;    //clear count register
    TPM_HAL_SetChnCountVal(TPM0, 0, 125 /** tick*/); // TODO validate + test overflow
    NVIC_EnableIRQ(TPM0_IRQn);
    TPM_HAL_EnableTimerOverflowInt(TPM0);
}

error_t hw_timer_cancel(hwtimer_id_t timer_id)
{
    if(timer_id >= HWTIMER_NUM) return ESIZE;
    if(!timer_inited) return EOFF;

    NVIC_DisableIRQ(TPM0_IRQn);
    TPM_HAL_EnableTimerOverflowInt(TPM0);
}

error_t hw_timer_counter_reset(hwtimer_id_t timer_id)
{
    if(timer_id >= HWTIMER_NUM) return ESIZE;
    if(!timer_inited) return EOFF;

    NVIC_DisableIRQ(TPM0_IRQn);
    TPM0_CNT = 0x00;    //clear count register
    NVIC_EnableIRQ(TPM0_IRQn);
}

bool hw_timer_is_overflow_pending(hwtimer_id_t timer_id)
{
	// TODO
//    if(timer_id >= HWTIMER_NUM)
//	return false;
//    start_atomic();
//	//COMP0 is used to limit thc RTC to 16 bits -> use this one to check
//	bool is_pending = !!((RTC_IntGet() & RTC->IEN) & RTC_IFS_COMP0);
//    end_atomic();
//    return is_pending;
}
bool hw_timer_is_interrupt_pending(hwtimer_id_t timer_id)
{
// TODO
//    if(timer_id >= HWTIMER_NUM)
//	return false;
//
//    start_atomic();
//	bool is_pending = !!((RTC_IntGet() & RTC->IEN) & RTC_IFS_COMP1);
//    end_atomic();
//    return is_pending;
}

void TPM0_IRQHandler()
{
    NVIC_DisableIRQ(TPM0_IRQn);
    if(TPM_HAL_GetTimerOverflowStatus(TPM0) && overflow_f != 0)
    {
        TPM_HAL_ClearTimerOverflowFlag(TPM0);
        overflow_f();
    }
    else if(compare_f != 0)
    {
        TPM_HAL_ClearChnInt(TPM0, 0);
        compare_f();
    }

}


// TODO
//INT_HANDLER(RTC_IRQHandler)
//{
//	//retrieve flags. We 'OR' this with the enabled interrupts
//	//since the COMP1 flag may be set if it wasn't used before (compare register == 0 -> ifs flag set regardless of whether interrupt is enabled)
//	//by AND ing with the IEN we make sure we only consider the flags of the ENABLED interrupts
//	uint32_t flags = (RTC_IntGet() & RTC->IEN);
//	RTC_IntClear(RTC_IFC_OF | RTC_IFC_COMP0 | RTC_IFC_COMP1);
//
//	//evaluate flags to see which one(s) fired:
//	if((flags & RTC_IFS_COMP0) && (overflow_f != 0x0))
//		overflow_f();
//	if((flags & RTC_IFS_COMP1))
//	{
//		RTC_IntDisable(RTC_IEN_COMP1);
//		if(compare_f != 0x0)
//			compare_f();
//	}
//}

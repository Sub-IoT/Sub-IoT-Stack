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

static timer_callback_t compare_f = 0x0;
static timer_callback_t overflow_f = 0x0;
static bool timer_inited = false;

error_t hw_timer_init(hwtimer_id_t timer_id, uint8_t frequency, timer_callback_t compare_callback, timer_callback_t overflow_callback)
{

    if(timer_id >= HWTIMER_NUM) return ESIZE;
    if(timer_inited) return EALREADY;
    if(frequency != HWTIMER_FREQ_1MS) return EINVAL;

    overflow_f = overflow_callback;
    compare_f = compare_callback;

    SIM_SOPT2 = (SIM_SOPT2&~SIM_SOPT2_TPMSRC_MASK)|SIM_SOPT2_TPMSRC(3); //select MCGIRCLK to reference (4MHz) // TODO use 1 kHz LPO input clock instead + validate accuracy
    SIM_SCGC6 |= (1<<SIM_SCGC6_TPM0_SHIFT); //enable bus clock to module
    TPM0_CNT = 0x00;
    TPM0_MOD = 0xFFFF;
    TPM0_SC = (1<<TPM_SC_TOF_SHIFT)|(0<<TPM_SC_TOIE_SHIFT)|(0<<TPM_SC_CPWMS_SHIFT)|(TPM_SC_CMOD(1)|TPM_SC_PS(5));//TOF cleared, interrupt disable, clock references "module clock,"
                                                                                                                 //prescale 4 MHz to 125000 Hz, timer counts up
    TPM_HAL_SetCpwms(TPM0, 1);
    TPM_HAL_EnableChnInt(TPM0, 0);
    MCG_C1 |= (1<<MCG_C1_IRCLKEN_SHIFT); //start reference clock

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
    TPM0_MOD = 125 * tick; // TODO validate accuracy + change when switching ref clock
    //TPM_HAL_SetChnCountVal(TPM0, 0, 125 /** tick*/); // TODO use compare instead of MOD
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
    TPM0_CNT = 0x00;
    NVIC_EnableIRQ(TPM0_IRQn);
}

bool hw_timer_is_overflow_pending(hwtimer_id_t timer_id)
{
	// TODO
}
bool hw_timer_is_interrupt_pending(hwtimer_id_t timer_id)
{
    // TODO
}

void TPM0_IRQHandler()
{
// TODO we are using MOD register for configured interval causing overflow interrupt when timer elapses.
// When this happens call compare_f callback. This means we currently do not support real timer overflow (for long intervals),
// until we find out how to implement compare functionality with TPM.

//    NVIC_DisableIRQ(TPM0_IRQn);
//    if(TPM_HAL_GetTimerOverflowStatus(TPM0) && overflow_f != 0)
//    {
//        TPM_HAL_ClearTimerOverflowFlag(TPM0);
//        overflow_f();
//    }
//    else if(compare_f != 0)
//    {
//        TPM_HAL_ClearChnInt(TPM0, 0);
//        compare_f();
//    }

    NVIC_DisableIRQ(TPM0_IRQn);
    TPM_HAL_DisableTimerOverflowInt(TPM0);
    TPM_HAL_ClearTimerOverflowFlag(TPM0);
    TPM_HAL_ClearChnInt(TPM0, 0);
    if(compare_f != 0)
        compare_f();

}


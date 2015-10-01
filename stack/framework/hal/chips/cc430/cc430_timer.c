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


#include <stdbool.h>
#include <stdint.h>
#include "debug.h"

#include "hwtimer.h"
#include "hwatomic.h"

#include "msp430.h"
#include "timer_a.h"

#define CC430_TIMER_BASE_ADDRESS TIMER_A0_BASE
#define CC430_TIMER_COMPARE_REGISTER TIMER_A_CAPTURECOMPARE_REGISTER_1

static timer_callback_t compare_f = 0x0;
static timer_callback_t overflow_f = 0x0;
static bool timer_inited = false;

// Timer A0 interrupt service routine
void __attribute__ ((interrupt(TIMER0_A1_VECTOR))) TIMER0_A1_VECTOR_ISR (void)
{
    switch(TA0IV)
    {
        case 0x0E:
            // timer overflowed
            overflow_f();
            break;
    	case 0x02:
            // compare value for register 1 reached
            Timer_A_disableCaptureCompareInterrupt(CC430_TIMER_BASE_ADDRESS, CC430_TIMER_COMPARE_REGISTER);
            if(compare_f != 0x0)
                compare_f();

            break;
        default:
            assert(false);
    }

    Timer_A_clearTimerInterruptFlag(CC430_TIMER_BASE_ADDRESS);
    Timer_A_enableInterrupt(CC430_TIMER_BASE_ADDRESS); // ensure interrupt enabled to allow interrupting on overflows before hw_timer_schedule is called
    __bic_SR_register_on_exit(CPUOFF); // go back to active mode to re-enter scheduler
}

error_t hw_timer_init(hwtimer_id_t timer_id, uint8_t frequency, timer_callback_t compare_callback, timer_callback_t overflow_callback)
{
    if(timer_id >= HWTIMER_NUM)
        return ESIZE;

    if(timer_inited)
        return EALREADY;

    if(frequency != HWTIMER_FREQ_1MS && frequency != HWTIMER_FREQ_32K)
        return EINVAL;

    Timer_A_initContinuousModeParam param =
    {
        .clockSource = TIMER_A_CLOCKSOURCE_ACLK,
        .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_8, // ACLK = REFO = 32768 Hz => /8 = 4096 Hz
        .timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE,
        .timerClear = TIMER_A_DO_CLEAR,
        .startTimer = true
    };

    start_atomic();
    {
        compare_f = compare_callback;
        overflow_f = overflow_callback;
        timer_inited = true;

    	Timer_A_clearTimerInterruptFlag(CC430_TIMER_BASE_ADDRESS);
    	Timer_A_initContinuousMode(CC430_TIMER_BASE_ADDRESS, &param);
    	TA0EX0 = TAIDEX_3; // extra division by 4: 4096 Hz / 4 = 1 tick
    	Timer_A_clear(CC430_TIMER_BASE_ADDRESS); // reset needed to ensure proper divider logic is used after setting TA1EX0
    }
    end_atomic();

    return SUCCESS;
}

hwtimer_tick_t hw_timer_getvalue(hwtimer_id_t timer_id)
{
    if(timer_id >= HWTIMER_NUM || (!timer_inited))
        return 0;

    return Timer_A_getCounterValue(CC430_TIMER_BASE_ADDRESS);
}

error_t hw_timer_schedule(hwtimer_id_t timer_id, hwtimer_tick_t tick )
{
    if(timer_id >= HWTIMER_NUM)
        return ESIZE;

    if(!timer_inited)
        return EOFF;

    Timer_A_initCompareModeParam param =
    {
        .compareRegister = CC430_TIMER_COMPARE_REGISTER,
        .compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE,
        .compareOutputMode = TIMER_A_OUTPUTMODE_SET,
        .compareValue = tick
    };

    start_atomic();
    {
        Timer_A_initCompareMode(CC430_TIMER_BASE_ADDRESS, &param);
        Timer_A_clearTimerInterruptFlag(CC430_TIMER_BASE_ADDRESS);
    }
    end_atomic();
}

error_t hw_timer_cancel(hwtimer_id_t timer_id)
{
    if(timer_id >= HWTIMER_NUM)
        return ESIZE;

    if(!timer_inited)
        return EOFF;

    start_atomic();
    {
        Timer_A_disableInterrupt(CC430_TIMER_BASE_ADDRESS);
        Timer_A_clearTimerInterruptFlag(CC430_TIMER_BASE_ADDRESS);
    }
    end_atomic();
}

error_t hw_timer_counter_reset(hwtimer_id_t timer_id)
{
    if(timer_id >= HWTIMER_NUM)
        return ESIZE;

    if(!timer_inited)
        return EOFF;

    start_atomic();
    {
        Timer_A_clearTimerInterruptFlag(CC430_TIMER_BASE_ADDRESS);
        TA0R = 0;
    }
    end_atomic();
}

bool hw_timer_is_overflow_pending(hwtimer_id_t timer_id)
{
    if(timer_id >= HWTIMER_NUM)
        return false;

    // TODO
}

bool hw_timer_is_interrupt_pending(hwtimer_id_t timer_id)
{
    if(timer_id >= HWTIMER_NUM)
        return false;

    // TODO
}


#include <stdbool.h>
#include <stdint.h>

#include "hwtimer.h"
#include "hwatomic.h"

#include "msp430.h"
#include "timer_a.h"

static timer_callback_t compare_f = 0x0;
static timer_callback_t overflow_f = 0x0;
static bool timer_inited = false;

// Timer A0 interrupt service routine
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) TIMER0_A0_VECTOR_ISR (void)
{
    uint16_t ta0cctl0 = TA0CCTL0; // TODO tmp
    uint16_t ta0iv = TA0IV; // TODO tmp
    uint16_t ta0r = TA0R; // TODO tmp
    uint16_t interrupt_flags = Timer_A_getCaptureCompareInterruptStatus(
                TIMER_A0_BASE,
                TIMER_A_CAPTURECOMPARE_REGISTER_0, // TODO def
                TIMER_A_CAPTURE_OVERFLOW | TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG);
    //if((interrupt_flags & TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG) == TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG)
    {
        Timer_A_disableCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
        if(compare_f != 0x0)
            compare_f();
    }
    // TODO check overflow

    Timer_A_clearTimerInterruptFlag(TIMER_A0_BASE);
    __bic_SR_register_on_exit(CPUOFF);//(LPM0_bits); // go back to active mode to re-enter scheduler // TODO detect which LPM we where in before?
}

error_t hw_timer_init(hwtimer_id_t timer_id, uint8_t frequency, timer_callback_t compare_callback, timer_callback_t overflow_callback)
{
    // TODO check id
    compare_f = compare_callback;
    overflow_f = overflow_callback;
    timer_inited = true;

    Timer_A_initContinuousModeParam param =
    {
        .clockSource = TIMER_A_CLOCKSOURCE_ACLK,
        .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_8, // ACLK = REFO = 32768 Hz => /8 = 1024 Hz = 1 tick
        .timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE,
        .timerClear = TIMER_A_DO_CLEAR,
        .startTimer = true
    };

    Timer_A_initContinuousMode(TIMER_A0_BASE, &param);
    return SUCCESS;
}

hwtimer_tick_t hw_timer_getvalue(hwtimer_id_t timer_id)
{
    // TODO check id + inited
    return Timer_A_getCounterValue(TIMER_A0_BASE);
}

error_t hw_timer_schedule(hwtimer_id_t timer_id, hwtimer_tick_t tick )
{
    // TODO check id + inited
    Timer_A_initCompareModeParam param =
    {
        .compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0,
        .compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE,
        .compareOutputMode = TIMER_A_OUTPUTMODE_SET, // TODO
        .compareValue = tick
    };

    start_atomic();
    {
        Timer_A_initCompareMode(TIMER_A0_BASE, &param);
        Timer_A_clearTimerInterruptFlag(TIMER_A0_BASE);
    }
    end_atomic();
}

error_t hw_timer_cancel(hwtimer_id_t timer_id)
{
    // TODO check id + inited
    start_atomic();
    {
        Timer_A_disableInterrupt(TIMER_A0_BASE);
        Timer_A_clearTimerInterruptFlag(TIMER_A0_BASE);
    }
    end_atomic();
}

error_t hw_timer_counter_reset(hwtimer_id_t timer_id)
{
    // TODO check id + inited
    start_atomic();
    {
        Timer_A_clearTimerInterruptFlag(TIMER_A0_BASE);
        TA0R = 0;
    }
    end_atomic();
}

bool hw_timer_is_overflow_pending(hwtimer_id_t timer_id)
{
    // TODO
}

bool hw_timer_is_interrupt_pending(hwtimer_id_t timer_id)
{
    // TODO
}

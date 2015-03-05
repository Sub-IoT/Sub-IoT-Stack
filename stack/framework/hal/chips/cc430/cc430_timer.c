
#include <stdbool.h>
#include <stdint.h>

#include "hwtimer.h"
#include "hwatomic.h"


error_t hw_timer_init(hwtimer_id_t timer_id, uint8_t frequency, timer_callback_t compare_callback, timer_callback_t overflow_callback)
{
    // TODO
}

hwtimer_tick_t hw_timer_getvalue(hwtimer_id_t timer_id)
{
    // TODO
}

error_t hw_timer_schedule(hwtimer_id_t timer_id, hwtimer_tick_t tick )
{
    // TODO
}

error_t hw_timer_cancel(hwtimer_id_t timer_id)
{
    // TODO
}

error_t hw_timer_counter_reset(hwtimer_id_t timer_id)
{
    // TODO
}

bool hw_timer_is_overflow_pending(hwtimer_id_t timer_id)
{
    // TODO
}

bool hw_timer_is_interrupt_pending(hwtimer_id_t timer_id)
{
    // TODO
}

// TODO int handler

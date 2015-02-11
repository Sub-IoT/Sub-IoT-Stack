#include "hwtimer.h"
#include <assert.h>
error_t hw_timer_init(hwtimer_id_t timer_id, uint8_t frequency, timer_callback_t compare_callback, timer_callback_t overflow_callback)
{
    assert(false);
    return FAIL;
}
hwtimer_tick_t hw_timer_getvalue(hwtimer_id_t timer_id)
{
    assert(false);
    return 0;
}
error_t hw_timer_schedule(hwtimer_id_t timer_id, hwtimer_tick_t tick )
{
    assert(false);
    return FAIL;
}
error_t hw_timer_cancel(hwtimer_id_t timer_id)
{
    assert(false);
    return FAIL;
}
error_t hw_timer_counter_reset(hwtimer_id_t timer_id)
{
    assert(false);
    return FAIL;
}
bool hw_timer_is_overflow_pending(hwtimer_id_t id)
{
    assert(false);
    return false;
}
bool hw_timer_is_interrupt_pending(hwtimer_id_t id)
{
    assert(false);
    return false;
}

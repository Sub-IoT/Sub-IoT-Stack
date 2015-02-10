#include "hwleds.h"
#include "scheduler.h"
#include "timer.h"
#include "log.h"

void timer0_callback()
{
	led_toggle(0);
	timer_post_task_delay(&timer0_callback, TIMER_TICKS_PER_SEC);
	log_print_string("Toggled led %d", 0);
}

void timer1_callback()
{
	led_toggle(1);
	timer_post_task_delay(&timer1_callback, 2*TIMER_TICKS_PER_SEC);
	log_print_string("Toggled led %d", 1);
}

void bootstrap()
{
    led_on(0);
    led_on(1);
    log_print_string("Device booted at time: %d\n", timer_get_counter_value());

    sched_register_task(&timer0_callback);
    sched_register_task(&timer1_callback);

    timer_post_task_delay(&timer0_callback, TIMER_TICKS_PER_SEC);
    timer_post_task_delay(&timer1_callback, 2*TIMER_TICKS_PER_SEC);
    led_off(0);
    led_off(1);
}


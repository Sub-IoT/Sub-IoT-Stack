#include "hwleds.h"
#include "scheduler.h"
#include "hwuart.h"
#include "hwtimer.h"
#include "hwatomic.h"
#include "hwsystem.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "log.h"

void cmp_callback_task()
{
	uint32_t i = 0xCAFEBABE;
	led_toggle(1);
	hw_timer_schedule(0,0x7FFF);
	log_print_string("Hello %s", "World");
	log_print_stack_string('H', "Hello STACK %s", "World");
	log_print_data((uint8_t*)(&i), sizeof(uint32_t));
}

void of_callback_task()
{

	led_toggle(0);
}


void cmp_callback()
{
	sched_post_task(&cmp_callback_task);
}

void of_callback()
{
	sched_post_task(&of_callback_task);
}


void bootstrap()
{
    led_on(0);
    led_on(1);
    sched_register_task(&cmp_callback_task);
    sched_register_task(&of_callback_task);
    hw_timer_init(0,HWTIMER_FREQ_32K, &cmp_callback, &of_callback);
    hw_timer_schedule(0,0x7FFF);
}

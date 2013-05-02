/*
 * timer.c
 *
 *  Created on: 26-nov.-2012
 *      Author: Maarten Weyn
 */

#include <stddef.h>
#include <stdbool.h>

#include "timer.h"
#include "queue.h"
#include "log.h"

#include "hal/system.h"

static timer_event event_array[20];
static queue event_queue;
//static u16 max_time;
static bool started;

static void timer_setvalue(u16 next_event)
{
	TA1CCR0 = next_event;
	TA1CTL |= TACLR;
}

static void timer_enable_interrupt()
{
	TA1CCTL0 = CCIE; // Enable interrupt for CCR0
	TA1CTL |= MC__UP;
	started = true;
}

static void timer_disable_interrupt()
{
	TA1CCTL0 &= ~CCIE; // Disable interrupt for CCR0
	TA1CTL &= ~MC__UP;
	started = false;
}

void timer_init()
{
	//TODO: put this in ral

	//set timer to ticks (=1024 Hz)
	TA1CTL = TASSEL_1 + MC__UP + ID_3 + TACLR;           // ACLK/8, up mode, clear timer
	TA1EX0 = TAIDEX_3;							// divide /4

	queue_init(&event_queue, (u8*)&event_array, sizeof(event_array));
	//max_time = 0;
	started = false;
}

bool timer_insert_value_in_queue(timer_event* event)
{
	// TODO: substract time already gone
	u8 position = 0;
	int16_t sum_next_event = -TA1R;

	while (position < event_queue.length)
	{
		timer_event *temp_event = (timer_event*) queue_read_value(&event_queue, position, sizeof(timer_event));
		if (event->next_event > sum_next_event + temp_event->next_event)
		{
			sum_next_event += temp_event->next_event;
		} else {
			uint16_t elapsed = 0;
			if (position == 0)
			{
				elapsed = TA1R;
				timer_disable_interrupt();
				started = false;
			} else {
				event->next_event -= sum_next_event;
			}

			queue_insert_value(&event_queue, (void*) event, position, sizeof(timer_event));
			temp_event = (timer_event*) queue_read_value(&event_queue, position+1, sizeof(timer_event));

			temp_event->next_event -= (event->next_event + elapsed);
			return true;
		}
		position++;
	}

	if (position == event_queue.length)
	{
		if (started) event->next_event -= sum_next_event;
		return queue_push_value(&event_queue, (void*) event, sizeof(timer_event));
	}

	return true;
}


bool timer_add_event(timer_event* event)
{
	if (event->next_event == 0)
	{
		event->f(NULL);
		return true;
	}

	timer_event new_event;
	new_event.f = event->f;
	new_event.next_event = event->next_event;

	if (timer_insert_value_in_queue(&new_event))
	{
		if (!started)
		{
			timer_enable_interrupt();
			timer_setvalue(event->next_event);
		}
	} else {
		log_print_string("Cannot add event, queue is full");
		return false;
	}


	return true;
}

// Timer A0 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer_A (void)
{
	timer_event* event = (timer_event*) queue_pop_value(&event_queue, sizeof(timer_event));

	if (event_queue.length == 0)
	{
		timer_disable_interrupt();
	} else {
		timer_setvalue(((timer_event*) event_queue.front)->next_event);
	}

	event->f(NULL);
}


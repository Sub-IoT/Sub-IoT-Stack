#include "hal/system.h"
#include "hal/rtc.h"
#include "hal/leds.h"

#include "queue.h"
#include "log.h"
#include "timer.h"

u8 series_number = 10;
//u16 timing_series[] = { 1024, 1024, 2048, 1024, 2, 1024, 1024, 1024, 4096, 1024};
u16 timing_series[] = {4096, 5014, 1024, 2048, 5012, 6036, 7060, 8024, 10000, 10048};

queue q;

void callback2(void* arg);

void callback1(void* arg)
{
	log_print_string("callback 1", 10);
	led_toggle(3);
	timer_event event;
	event.next_event = 1024;
	event.f = &callback2;
	timer_add_event(&event);
}

void callback2(void* arg)
{
	log_print_string("callback 2", 10);
	led_toggle(1);
	timer_event event;
	event.next_event = 1024;
	event.f = &callback1;
	timer_add_event(&event);
}

int main(void) {
    system_init();

    log_print_string("started", 7);

   //TA1CCTL0 = CCIE;							// CCR0 interrupt enabled
//    TA1CTL = TASSEL_1 + MC__UP + ID_3 + TACLR;           // ACLK/8, up mode, clear timer
//    TA1EX0 = TAIDEX_3;							// divide /4
//    TA1CCR0 =  1024;                             // 1024Hz 203.125
	
    timer_init();

    u16 test[] = {1024, 2048, 4096, 5012, 5014, 6036, 7060, 8024, 10000, 10048};

    u16 queue_storage[30];

    queue_init(&q, (u8*) &queue_storage, sizeof(queue_storage));

    queue_push_u16(&q, timing_series[0]);
    int i = 1;
    for (;i<series_number;i++)
    {
    	u8 position = 0;

    	while (position<q.length)
    	{
    		if (queue_read_u16(&q, position) > timing_series[i])
    		{
    			queue_insert_u16(&q, timing_series[i], position);
    			break;
    		}
    		position++;
    	}

    	if (position == q.length)
    	    queue_push_u16(&q, timing_series[i]);
    }

    //test
    for (i=0;i<series_number;i++)
	{
		u16 value = queue_read_u16(&q, i);
		if (value != test[i])
		{
			__no_operation();
		}
	}

    // make diff que
    u16 previous = queue_read_u16(&q, 0);
    for (i=1;i<q.length;i++)
	{
    	u16 newvalue = queue_read_u16(&q, i);
    	queue_set_u16(&q,newvalue  - previous, i);
    	previous = newvalue;
	}


    __no_operation();

    //TA1CCR0 =  queue_pop_u16(&q);
    //TA1CCTL0 = CCIE; // Enable interrupt for CCR0


	timer_event event;
	event.next_event = 1024;
	event.f = &callback1;
	timer_add_event(&event);

    log_print_string("Counting", 8);

	while(1)
	{
		system_lowpower_mode(4,1);
	}
}

u16 counter = 0;
u8 index;
/*
// Timer A0 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer_A (void)
{
//	if (++counter >= 1023)
//	{
		led_toggle(3);

		log_print_string("Timer", 5);

		if (q.length > 0)
		{
			TA1CCR0 = queue_pop_u16(&q);
			TA1CTL |= TACLR;
		}
		else
			TA1CCR0 = 1024;

//
//		counter = 0;
//	}

}
*/

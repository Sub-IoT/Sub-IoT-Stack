#include "types.h"

#include "hal/system.h"
#include "hal/rtc.h"
#include "hal/leds.h"

#include "queue.h"
#include "log.h"
#include "timer.h"

void callback1(void* arg);
void callback2(void* arg);


u8 series_number = 10;
//u16 timing_series[] = { 1024, 1024, 2048, 1024, 2, 1024, 1024, 1024, 4096, 1024};
u16 timing_series[] = {4096, 5014, 1024, 2048, 5012, 6036, 7060, 8024, 10000, 10048};
void* event_series[] ={&callback1, &callback1, &callback2, &callback1, &callback1, &callback1, &callback1, &callback1, &callback1, &callback1};

queue q;

void callback2(void* arg);

void callback1(void* arg)
{
	log_print_string("callback 1");
	led_toggle(3);
//	timer_event event;
//	event.next_event = 1024;
//	event.f = &callback2;
//	timer_add_event(&event);
}

void callback2(void* arg)
{
	log_print_string("callback 2");
	led_toggle(1);
//	timer_event event;
//	event.next_event = 1024;
//	event.f = &callback1;
//	timer_add_event(&event);
}

int main(void) {
    system_init();

    log_print_string("started");
	
    timer_init();



    uint8_t i=0;
    for (; i<sizeof(timing_series) / sizeof(u16); i++)
    {
    	timer_event event;
		event.next_event = timing_series[i];
		event.f = event_series[i];
		log_print_string("add event");
		timer_add_event(&event);
    }


    log_print_string("Counting");

	while(1)
	{
		system_lowpower_mode(4,1);
	}
}

u16 counter = 0;
u8 index;

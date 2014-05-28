#include "types.h"

#include "hal/system.h"
#include "hal/rtc.h"
#include "hal/leds.h"

#include "framework/queue.h"
#include "framework/log.h"
#include "framework/timer.h"

#include <msp430.h>


void callback1();
void callback2();


uint8_t series_number = 10;
//u16 timing_series[] = { 1024, 1024, 2048, 1024, 2, 1024, 1024, 1024, 4096, 1024};
uint16_t timing_series[] = {4096, 5014, 1024, 2048, 5012, 6036, 7060, 8024, 10000, 2};
void* event_series[] ={&callback1, &callback1, &callback2, &callback1, &callback1, &callback1, &callback1, &callback1, &callback1, &callback1};

queue_t q;

uint8_t count = 0;
timer_event event;
uint32_t prev_counter_1 = 0;
uint32_t prev_counter_2 = 0;

void callback2();

void callback1()
{
//	uint32_t counter_2 = benchmarking_timer_getvalue();
//	benchmarking_timer_stop();
//	log_print_string("benchmark: %d", counter_2);
//
//	benchmarking_timer_start();
//	timer_add_event(&event);
//	count++;

//	uint32_t diff_2 = counter_2 - prev_counter_2;
//	prev_counter_2 = counter_2;
//
//	//log_print_data(&count, 1);
//	char msg[16];
//	itoa(diff_2, msg);
//	log_print_string(msg);

//	uint32_t counter_1 = timer_get_counter_value();
//	uint32_t diff_1 = counter_1 - prev_counter_1;
	//char msg[16];

	//itoa(diff_1, msg);

	uint16_t current_timer = hal_timer_getvalue();
	log_print_string("callback 1: %d", current_timer);
	led_toggle(3);
//	timer_event event;
//	event.next_event = 1024;
//	event.f = &callback2;
//	timer_add_event(&event);
}

void callback2()
{
	uint16_t current_timer = hal_timer_getvalue();
	log_print_string("callback 2: %d", current_timer);
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
    for (; i<sizeof(timing_series) / sizeof(uint16_t); i++)
    {
    	timer_event event;
		event.next_event = timing_series[i];
		event.f = event_series[i];
		log_print_string("add event %d", timing_series[i]);
		timer_add_event(&event);
    }


//    event.next_event = 10;
//    event.f = &callback1;

//    benchmarking_timer_init();
//    benchmarking_timer_start();
//
//    prev_counter_1 = timer_get_counter_value();
//    timer_add_event(&event);

    log_print_string("Counting");

	while(1)
	{
		system_lowpower_mode(4,1);
	}
}


#pragma vector=AES_VECTOR,COMP_B_VECTOR,DMA_VECTOR,PORT1_VECTOR,PORT2_VECTOR,SYSNMI_VECTOR,UNMI_VECTOR,USCI_A0_VECTOR,USCI_B0_VECTOR,WDT_VECTOR,TIMER0_A0_VECTOR,TIMER1_A1_VECTOR
__interrupt void ISR_trap(void)
{
  /* For debugging purposes, you can trap the CPU & code execution here with an
     infinite loop */
  //while (1);
	__no_operation();

  /* If a reset is preferred, in scenarios where you want to reset the entire system and
     restart the application from the beginning, use one of the following lines depending
     on your MSP430 device family, and make sure to comment out the while (1) line above */

  /* If you are using MSP430F5xx or MSP430F6xx devices, use the following line
     to trigger a software BOR.   */
  PMMCTL0 = PMMPW | PMMSWBOR;          // Apply PMM password and trigger SW BOR
}

/*
 *  Created on: Jan 23, 2013
 *  Authors:
 * 		maarten.weyn@artesis.be
 */


#include <string.h>

#include <trans/trans.h>

#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <log.h>
#include <timer.h>
#include <hal/rtc.h>

#define SEND_INTERVAL_MS	1000
#define SYNC_INTERVAL_MS	500
#define SYNC_LENGTH			10
#define SEND_CHANNEL 0x1C

#define USE_LEDS

#define INTERRUPT_RTC 		(1 << 3)
#define BATTERY_OFFSET	-10
#define CLOCKS_PER_1us	20

//static u8 interrupt_flags = 0;
//static u8 tx = 0;

u8 data[32];
uint32_t counter;
u8 dataLength = 0;
u16 battery_voltage;
u8 sync_position = 0;

timer_event event;
timer_event sync_event;

uint16_t adc12_result;
volatile uint8_t  adc12_data_ready;

uint16_t battery_measurement(void);
Calendar currentTime;


void start_tx(void* arg)
{
	log_print_string("tx_event removed");

	//if (tx)
	//	return;

	//tx = 1;
	#ifdef USE_LEDS
	led_on(2);
	#endif

	sync_position = SYNC_LENGTH;

	timer_add_event(&sync_event);
	log_print_string("sync_event added");

	counter = sync_position * SYNC_INTERVAL_MS;

	trans_tx_background_frame((uint8_t*) &counter, 0xFF, SEND_CHANNEL, 10);
	log_print_string("BF");
	log_print_data((uint8_t*) &sync_position, 1);
	//trans_tx_foreground_frame(data, dataLength, SEND_CHANNEL, 10);
}

void start_tx_sync(void* arg)
{

	log_print_string("sync_event removed");
	sync_position--;

	if (sync_position == 0)
	{
		#ifdef USE_LEDS
		led_on(3);
		#endif

		trans_tx_foreground_frame(data, dataLength, SEND_CHANNEL, 10);

		log_print_string("tx_event added");
		log_print_string("FF");

		timer_add_event(&event);
	} else {
		log_print_string("sync_event added");
		counter = sync_position * SYNC_INTERVAL_MS;
		#ifdef USE_LEDS
		led_on(2);
		#endif
		log_print_string("BF");

		log_print_data((uint8_t*) &sync_position, 1);
		trans_tx_background_frame((uint8_t*) &counter, 0xFF, SEND_CHANNEL, 10);
		timer_add_event(&sync_event);
	}
}

void tx_callback(Trans_Tx_Result result)
{
	system_watchdog_timer_reset();
#ifdef USE_LEDS
	if(result == TransPacketSent)
	{

		led_off(3);
		led_off(2);
		log_print_string("TX OK");
	}
	else
	{
		led_toggle(1);
		log_print_string("TX CCA FAIL");
	}
#endif

	//tx = 0;
	//timer_add_event(&event);
}


int main(void) {
	system_init();


	data[0] = 1;
	data[1] = 2;
	data[2] = 3;
	dataLength = 3;


	trans_init();
	trans_set_tx_callback(&tx_callback);

	event.next_event = SEND_INTERVAL_MS;
	event.f = &start_tx;

	sync_event.next_event = SYNC_INTERVAL_MS;
	sync_event.f = &start_tx_sync;

	log_print_string("sync node 1 started");

	timer_add_event(&event);

	log_print_data(tag_id, 8);

	system_watchdog_init(WDTSSEL0, 0x03); // 32KHz / 2^19
	system_watchdog_timer_start();

	while(1)
	{

		system_lowpower_mode(4,1);
	}
}

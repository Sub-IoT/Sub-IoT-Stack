/*
 *  Created on: Jan 23, 2013
 *  Authors:
 * 		maarten.weyn@artesis.be
 */


#include <string.h>

#include <trans/trans.h>

#include <dll/dll.h>

#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <framework/log.h>
#include <framework/timer.h>
#include <hal/rtc.h>

#define SEND_INTERVAL_MS	10000
#define SYNC_PERIOD_MS		1000
#define SYNC_LENGTH			50
#define SEND_CHANNEL 0x10

#define USE_LEDS

#define INTERRUPT_RTC 		(1 << 3)
#define BATTERY_OFFSET	-10
#define CLOCKS_PER_1us	20

#define LED_RED	2
#define LED_ORANGE 1
#define LED_GREEN 3

//static u8 interrupt_flags = 0;
//static u8 tx = 0;

uint8_t data[32];
uint16_t eta;
uint8_t dataLength = 0;
uint16_t battery_voltage;
uint8_t sync_position = 0;
uint32_t volatile targetTimeStamp;

timer_event event;
timer_event sync_event;

uint16_t adc12_result;
volatile uint8_t  adc12_data_ready;

uint16_t battery_measurement(void);
//Calendar currentTime;


void start_tx()
{
	log_print_string("tx_event removed");

	//if (tx)
	//	return;

	//tx = 1;
	#ifdef USE_LEDS
	led_on(LED_GREEN);
	#endif

	sync_position = SYNC_LENGTH;

	//timer_add_event(&sync_event);
	//log_print_string("sync_event added");

	targetTimeStamp = timer_get_counter_value() + SYNC_PERIOD_MS;

	eta = 1000;

	//trans_tx_background_frame((uint8_t*) &counter, 0xFF, SEND_CHANNEL, 10);
	nwl_build_advertising_protocol_data(SEND_CHANNEL, eta, 10, 0xFF);
	dll_tx_frame();
	//log_print_string("BF");
	//log_print_data((uint8_t*) &sync_position, 1);
	//trans_tx_foreground_frame(data, dataLength, SEND_CHANNEL, 10);

	timer_add_event(&sync_event);
}

void start_tx_sync()
{

	//log_print_string("sync_event removed");
	eta = targetTimeStamp - timer_get_counter_value();

	if (eta < 10 || eta > SYNC_PERIOD_MS)
	{
		#ifdef USE_LEDS
		led_off(LED_GREEN);
		led_on(LED_ORANGE);
		#endif

		trans_tx_foreground_frame(data, dataLength, 0xFF, SEND_CHANNEL, 10);

		//log_print_string("tx_event added");
		//log_print_string("FF");

		timer_add_event(&event);
	} else {
		//log_print_string("sync_event added");
		#ifdef USE_LEDS
		led_on(LED_GREEN);
		#endif
		//log_print_string("BF");

		//log_print_data((uint8_t*) &sync_position, 1);
		//trans_tx_background_frame((uint8_t*) &counter, 0xFF, SEND_CHANNEL, 10);

		nwl_build_advertising_protocol_data(SEND_CHANNEL, eta, 10, 0xFF);
		dll_tx_frame();
		timer_add_event(&sync_event);
	}
}

void tx_callback(Trans_Tx_Result result)
{
	system_watchdog_timer_reset();
#ifdef USE_LEDS
	if(result == TransPacketSent)
	{

		led_off(LED_ORANGE);
		log_print_string("TX OK");
	}
	else
	{
		led_toggle(LED_RED);
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

	sync_event.next_event = 5;
	sync_event.f = &start_tx_sync;

	log_print_string("sync node 1 started");

	timer_add_event(&event);

	//log_print_data(tag_id, 8);

	//system_watchdog_init(WDTSSEL0, 0x03); // 32KHz / 2^19
	//system_watchdog_timer_start();

	while(1)
	{

		system_lowpower_mode(0,1);
	}
}

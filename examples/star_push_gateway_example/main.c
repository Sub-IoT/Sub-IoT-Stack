/*
 *  Created on: July 10, 2013
 *  Authors:
 * 		maarten.weyn@uantwerpen.be
 *
 * 	Example code for Star topology, push model
 * 	This is the Gateway example
 *
 * 	add the link to d7aoss library in de lnk_*.cmd file, e.g. -l "../../../d7aoss/Debug/d7aoss.lib"
 * 	Make sure to select the correct platform in d7aoss/hal/cc430/platforms.platform.h
 * 	If your platform is not present, you can add a header file in platforms and commit it to the repository.
 * 	Exclude the stub directories in d7aoss from the build when building for a device.
 */


#include <string.h>

#include <trans/trans.h>
#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <hal/uart.h>
#include <framework/log.h>
#include <framework/timer.h>
#include <msp430.h>


#define RECEIVE_CHANNEL 0x1E

// event to create a led blink
static timer_event dim_led_event;
static bool start_channel_scan = false;

static const dll_channel_scan_t scan_cfg = {
		RECEIVE_CHANNEL,
		FrameTypeForegroundFrame,
		0,
		0
};

static dll_channel_scan_series_t scan_series_cfg;

void blink_led()
{
	led_on(1);

	timer_add_event(&dim_led_event);
}

void dim_led(void* arg)
{
	led_off(1);
}

void start_rx()
{
	start_channel_scan = false;
	dll_channel_scan_series(&scan_series_cfg);
}

void rx_callback(dll_rx_res_t* rx_res)
{
	system_watchdog_timer_reset();

	blink_led();

	// log endpoint's device_id, RSS of link, and payload of device
	dll_foreground_frame_t* frame = (dll_foreground_frame_t*) (rx_res->frame);
	uart_transmit_data(0xCE); // NULL
	uart_transmit_data(11 + frame->payload_length);
	uart_transmit_data(0x10); // deviceid
	uart_transmit_message(frame->address_ctl->source_id, 8); // id mobile node
	uart_transmit_data(0x20); // netto rss
	uart_transmit_data(rx_res->rssi - frame->frame_header.tx_eirp); // signal strenght mobile node -> fixed node
	uart_transmit_message(frame->payload, frame->payload_length);
	uart_transmit_data(0x0D); // carriage return

	// Restart channel scanning
	start_channel_scan = true;
}



int main(void) {

	// Initialize the OSS-7 Stack
	system_init();

	// Set channel scan series to 1 entry
	// This should be done by writing the specific configuration file, when the uper layers are created.
	dll_channel_scan_t scan_confgs[1];
	scan_confgs[0] = scan_cfg;
	scan_series_cfg.length = 1;
	scan_series_cfg.values = scan_confgs;

	// Currently we address the Data Link Layer for RX, this should go to an upper layer once it is working.
	dll_init();
	dll_set_rx_callback(&rx_callback);

	start_channel_scan = true;

	log_print_string("gateway started");

	// Log the device id
	log_print_data(device_id, 8);

	// configure blinking led event
	dim_led_event.next_event = 50;
	dim_led_event.f = &dim_led;

	system_watchdog_init(WDTSSEL0, 0x03);
	system_watchdog_timer_start();

	blink_led();

	while(1)
	{
		if (start_channel_scan) start_rx();

		system_lowpower_mode(4,1);
	}
}


#pragma vector=ADC12_VECTOR,RTC_VECTOR,AES_VECTOR,COMP_B_VECTOR,DMA_VECTOR,PORT1_VECTOR,PORT2_VECTOR,SYSNMI_VECTOR,UNMI_VECTOR,USCI_A0_VECTOR,USCI_B0_VECTOR,WDT_VECTOR,TIMER0_A0_VECTOR,TIMER1_A1_VECTOR
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


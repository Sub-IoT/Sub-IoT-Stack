/*
 *  Created on: July 10, 2013
 *  Authors:
 * 		maarten.weyn@uantwerpen.be
 *
 * 	Example code for Star topology, push model
 * 	This is the endpoint example
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
#include <framework/log.h>
#include <framework/timer.h>
#include <msp430.h>


#define SEND_INTERVAL_MS 2000
#define SEND_CHANNEL 0x12
#define TX_EIRP 10

// Macro which can be removed in production environment
#define USE_LEDS

static uint8_t tx = 0;
static uint16_t counter = 0;

timer_event event;

void start_tx(void* ar)
{
	if (!tx)
	{
		// Kicks the watchdog timer
		system_watchdog_timer_reset();

		tx = 1;

		#ifdef USE_LEDS
		led_on(3);
		#endif

		log_print_string("TX...");

		trans_tx_foreground_frame((uint8_t*)&counter, sizeof(counter), 0xFF, SEND_CHANNEL, TX_EIRP);
	}
	timer_add_event(&event);
}

void tx_callback(Trans_Tx_Result result)
{

	counter++;

	if(result == TransPacketSent)
	{
		#ifdef USE_LEDS
		led_off(3);
		#endif
		log_print_string("TX OK");
	}
	else
	{
		#ifdef USE_LEDS
		led_toggle(1);
		#endif
		log_print_string("TX CCA FAIL");
	}

	tx = 0;
}

int main(void) {

	// Initialize the OSS-7 Stack
	system_init();

	// Currently we address the Transport Layer, this should go to an upper layer once it is working.
	trans_init();
	trans_set_tx_callback(&tx_callback);
	// The initial Tca for the CSMA-CA in
	trans_set_initial_t_ca(200);


	event.next_event = SEND_INTERVAL_MS;
	event.f = &start_tx;

	log_print_string("endpoint started");

	timer_add_event(&event);

	// Log the device id
	log_print_data(device_id, 8);

	system_watchdog_init(WDTSSEL0, 0x03);
	system_watchdog_timer_start();

	while(1)
	{
		system_lowpower_mode(4,1);
	}
}


//#pragma vector=ADC12_VECTOR,RTC_VECTOR,AES_VECTOR,COMP_B_VECTOR,DMA_VECTOR,PORT1_VECTOR,PORT2_VECTOR,SYSNMI_VECTOR,UNMI_VECTOR,USCI_A0_VECTOR,USCI_B0_VECTOR,WDT_VECTOR,TIMER0_A0_VECTOR,TIMER1_A1_VECTOR
//__interrupt void ISR_trap(void)
//{
//  /* For debugging purposes, you can trap the CPU & code execution here with an
//     infinite loop */
//  //while (1);
//	__no_operation();
//
//  /* If a reset is preferred, in scenarios where you want to reset the entire system and
//     restart the application from the beginning, use one of the following lines depending
//     on your MSP430 device family, and make sure to comment out the while (1) line above */
//
//  /* If you are using MSP430F5xx or MSP430F6xx devices, use the following line
//     to trigger a software BOR.   */
//  PMMCTL0 = PMMPW | PMMSWBOR;          // Apply PMM password and trigger SW BOR
//}


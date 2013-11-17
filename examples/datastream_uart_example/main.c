/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 *     maarten.weyn@uantwerpen.be
 *
 * 	Example of datastream communication
 * 	Uart <-> datastream
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
#define SEND_CHANNEL 0x1E
#define TX_EIRP 10

#define USE_LEDS

int main(void) {
	// Initialize the OSS-7 Stack
	system_init();

	// Set channel scan series to 1 entry
	// This should be done by writing the specific configuration file, when the uper layers are created.
	dll_channel_scan_t scan_confgs[1];
	scan_confgs[0] = scan_cfg;
	scan_series_cfg.length = 1;
	scan_series_cfg.values = scan_confgs;

	// Currently we address the Transport Layer, this should go to an upper layer once it is working.
	trans_init();
	trans_set_tx_callback(&tx_callback);
	// The initial Tca for the CSMA-CA in
	trans_set_initial_t_ca(200);
	trans_set_datastream_rx_callback(&datastream_callback);

	start_channel_scan = true;

	log_print_string("node started");

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
		if (start_channel_scan)
		{
			start_rx();
		}

		// Don't know why but system reboots when LPM > 1 since ACLK is uses for UART
		system_lowpower_mode(3,1);
	}
}

#pragma vector=ADC12_VECTOR,RTC_VECTOR,AES_VECTOR,COMP_B_VECTOR,DMA_VECTOR,PORT1_VECTOR,PORT2_VECTOR,SYSNMI_VECTOR,UNMI_VECTOR,USCI_A0_VECTOR,USCI_B0_VECTOR,WDT_VECTOR,TIMER0_A0_VECTOR,TIMER1_A1_VECTOR,TIMER0_A1_VECTOR
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

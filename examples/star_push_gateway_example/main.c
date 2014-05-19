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


#define RECEIVE_CHANNEL 0x10

// event to create a led blink
static timer_event dim_led_event;
static bool start_channel_scan = false;

void blink_led()
{
	led_on(1);

	timer_add_event(&dim_led_event);
}

void dim_led()
{
	led_off(1);
}

void start_rx()
{
	start_channel_scan = false;
	trans_rx_query_start(0xFF, RECEIVE_CHANNEL);
}

void rx_callback(Trans_Rx_Query_Result* rx_res)
{
	system_watchdog_timer_reset();

	blink_led();

	dll_foreground_frame_t* frame = (dll_foreground_frame_t*) (rx_res->nwl_rx_res->dll_rx_res->frame);
	log_print_string("Received Query from %02x%02x%02x%02x%02x%02x%02x", frame->address_ctl->source_id[0], frame->address_ctl->source_id[1], frame->address_ctl->source_id[2], frame->address_ctl->source_id[3], frame->address_ctl->source_id[4], frame->address_ctl->source_id[5], frame->address_ctl->source_id[6], frame->address_ctl->source_id[7]);
	log_print_string("RSS: %d dBm", rx_res->nwl_rx_res->dll_rx_res->rssi);
	log_print_string("Netto Link: %d dBm", rx_res->nwl_rx_res->dll_rx_res->rssi  - frame->frame_header.tx_eirp);

	// log endpoint's device_id, RSS of link, and payload of device
//	dll_foreground_frame_t* frame = (dll_foreground_frame_t*) (rx_res->nwl_rx_res->dll_rx_res->frame);
//	uart_transmit_data(0xCE); // NULL
//	uart_transmit_data(11 + frame->payload_length);
//	uart_transmit_data(0x10); // deviceid
//	uart_transmit_message(frame->address_ctl->source_id, 8); // id mobile node
//	uart_transmit_data(0x20); // netto rss
//	uart_transmit_data(rx_res->nwl_rx_res->dll_rx_res->rssi - frame->frame_header.tx_eirp); // signal strenght mobile node -> fixed node
//	uart_transmit_message(frame->payload, frame->payload_length);
//	uart_transmit_data(0x0D); // carriage return

	switch (rx_res->d7aqp_command.command_code & 0x0F)
	{
		case D7AQP_OPCODE_ANNOUNCEMENT_FILE:
		{
			D7AQP_Single_File_Return_Template* sfr_tmpl = (D7AQP_Single_File_Return_Template*) rx_res->d7aqp_command.command_data;
			log_print_string("D7AQP File Announcement received");
			log_print_string(" - file 0x%x starting from byte %d", sfr_tmpl->return_file_id, sfr_tmpl->file_offset);
			log_print_data(sfr_tmpl->file_data, sfr_tmpl->isfb_total_length - sfr_tmpl->file_offset);
		}
	}




	// Restart channel scanning
	start_channel_scan = true;
}



int main(void) {
	// Initialize the OSS-7 Stack
	system_init();

	// Currently we address the Transport Layer for RX, this should go to an upper layer once it is working.
	trans_init();
	trans_set_query_rx_callback(&rx_callback);

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
		if (start_channel_scan)
		{
			start_rx();
		}

		// Don't know why but system reboots when LPM > 1 since ACLK is uses for UART
		system_lowpower_mode(0,1);
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


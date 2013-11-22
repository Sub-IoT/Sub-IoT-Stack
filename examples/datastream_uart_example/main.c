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

// event to create a led blink
static timer_event dim_led_event;
static timer_event check_uart_event;
static volatile bool start_channel_scan = false;
static volatile bool new_uart_data = false;
static volatile bool check_for_uart_data = false;
static volatile bool is_checking_for_uart_data = false;

static uint8_t data[32];
static uint8_t data_lenght = 0;

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
	trans_rx_datastream_start(0xFF, RECEIVE_CHANNEL);
}

void tx_callback(Trans_Tx_Result result)
{
	if(result == TransPacketSent)
	{
		#ifdef USE_LEDS
		led_off(3);
		#endif
		//log_print_string("TX OK");
	}
	else
	{
		#ifdef USE_LEDS
		led_toggle(1);
		#endif
		//log_print_string("TX CCA FAIL");
	}

	start_channel_scan = true;
}

void send_uart_data()
{
	uint8_t send_data[32];
	memcpy((void*)send_data, (void*)data, data_lenght);
	trans_tx_datastream((uint8_t*)&send_data, data_lenght, 0xFF, SEND_CHANNEL, TX_EIRP);
	data_lenght = 0;
}


void check_uart(void* arg)
{
	if (data_lenght > 0  && !new_uart_data)
	{
		send_uart_data();
		is_checking_for_uart_data = false;
	} else if (new_uart_data)
	{
		new_uart_data = false;
		timer_add_event(&check_uart_event);
	}
}

void datastream_rx_callback(Trans_Rx_Datastream_Result result)
{
	system_watchdog_timer_reset();

	blink_led();

	uart_transmit_message(result.payload, result.lenght);

	// Restart channel scanning
	start_channel_scan = true;
}

int main(void) {
	// Initialize the OSS-7 Stack
	system_init();

	// Currently we address the Transport Layer, this should go to an upper layer once it is working.
	trans_init();
	trans_set_tx_callback(&tx_callback);
	// The initial Tca for the CSMA-CA in
	trans_set_initial_t_ca(200);
	trans_set_datastream_rx_callback(&datastream_rx_callback);

	start_channel_scan = true;

	//log_print_string("node started");

	// Log the device id
	log_print_data(device_id, 8);

	// configure blinking led event
	dim_led_event.next_event = 50;
	dim_led_event.f = &dim_led;

	check_uart_event.next_event = 1;
	check_uart_event.f = &check_uart;

	//system_watchdog_init(WDTSSEL0, 0x03);
	//ystem_watchdog_timer_start();

	uart_enable_interrupt();

	blink_led();

	while(1)
	{
		if (check_for_uart_data)
		{
			is_checking_for_uart_data = true;
			check_for_uart_data = false;
			timer_add_event(&check_uart_event);
		}

		if (start_channel_scan)
		{
			start_rx();
		}


		system_lowpower_mode(3,1);
	}
}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	//switch(__even_in_range(UCA0IV,4))
	switch(UCA0IV)
	{
		case 0:break;                             // Vector 0 - no interrupt
		case 2:
		{
			if (!is_checking_for_uart_data)
				check_for_uart_data = true;

			if (data_lenght == 32)
			{
				send_uart_data();
			}

			if (data_lenght < 32)
			{
				data[data_lenght++] = UCA0RXBUF;
				new_uart_data = true;
			}

			LPM3_EXIT;
			break;
		}
		case 4:break;                             // Vector 4 - TXIFG
		default: break;
	}
}

#pragma vector=ADC12_VECTOR,RTC_VECTOR,AES_VECTOR,COMP_B_VECTOR,DMA_VECTOR,PORT1_VECTOR,PORT2_VECTOR,SYSNMI_VECTOR,UNMI_VECTOR,USCI_B0_VECTOR,WDT_VECTOR,TIMER0_A0_VECTOR,TIMER1_A1_VECTOR,TIMER0_A1_VECTOR
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

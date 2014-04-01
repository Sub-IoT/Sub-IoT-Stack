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
 * Test to see if uart is working
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
#include <hal/uart.h>


#define SEND_INTERVAL_MS 100
#define SEND_CHANNEL 0x1E
#define TX_EIRP 10

// Macro which can be removed in production environment
#define USE_LEDS

//static uint8_t tx = 0;
static volatile bool add_tx_event = true;
static volatile uint8_t dataLength = 0;

static volatile int16_t temperature_internal;
static volatile uint16_t battery_voltage;

static volatile uint16_t counter = 0;

timer_event event;
uint8_t data[] = {0xCE,0x10,0x10,0x46,0x94,0x20,0x27,0x00,0x13,0x00,0x1A,0x20,0x00,0x65,0x01,0x30,0x70,0x02,0x0D};


void tx()
{
	//log_print_string("TX.................................... %d", counter++);
	uart_transmit_message(data, 19);
	led_toggle(1);
	timer_add_event(&event);
}

int main(void) {

	// Initialize the OSS-7 Stack
	system_init();

	// Currently we address the Transport Layer, this should go to an upper layer once it is working.
	trans_init();
	// The initial Tca for the CSMA-CA in
	trans_set_initial_t_ca(200);

	event.next_event = SEND_INTERVAL_MS;
	event.f = &tx;


	log_print_string("test started");

	timer_add_event(&event);

	while(1)
	{
		system_lowpower_mode(0,1);
	}
}


#pragma vector=RTC_VECTOR,AES_VECTOR,COMP_B_VECTOR,DMA_VECTOR,PORT1_VECTOR,PORT2_VECTOR,SYSNMI_VECTOR,UNMI_VECTOR,USCI_A0_VECTOR,USCI_B0_VECTOR,WDT_VECTOR,TIMER0_A0_VECTOR,TIMER1_A1_VECTOR
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


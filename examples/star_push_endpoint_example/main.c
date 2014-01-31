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


#define SEND_INTERVAL_MS 1000
#define SEND_CHANNEL 0x1A
#define TX_EIRP 10

// Macro which can be removed in production environment
#define USE_LEDS

static uint8_t tx = 0;
static uint16_t counter = 0;
static volatile bool add_tx_event = true;

timer_event event;

void start_tx()
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
	add_tx_event = true;
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

//	system_watchdog_init(WDTSSEL0, 0x03);
//	system_watchdog_timer_start();

	while(1)
	{
		if (add_tx_event)
		{
			add_tx_event = false;
			timer_add_event(&event);
		}

		system_lowpower_mode(3,1);
	}
}




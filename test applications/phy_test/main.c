/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 */

#include <string.h>
#include <stdio.h>

#include <d7aoss.h>

#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <hal/system.h>
#include <framework/log.h>


#define RX_MODE

static uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

phy_rx_cfg_t rx_cfg = {
	.spectrum_id={ 0x04, 0x00}, // TODO
	.sync_word_class=1,
	.length=0,
	.timeout=0,
	.scan_minimum_energy = -140
};

void start_rx(void)
{
	phy_rx(&rx_cfg);
}

void start_tx(void)
{
	phy_idle();
	queue_push_u8(&tx_queue, sizeof(data)); // length byte
	queue_push_u8_array(&tx_queue, data, sizeof(data));
	phy_tx_cfg_t tx_cfg = {
	    .spectrum_id = { 0x04, 0x00}, // TODO
		.sync_word_class = 1,
	    .eirp = 0,
	};

	phy_tx(&tx_cfg);
}

void rx_callback(phy_rx_data_t* rx_data)
{
	log_print_string("RX callback");
	if(memcmp(data, rx_data->data + 1, sizeof(data)) != 0) // first byte of rx_data->data is length (for now), skip this
		log_print_string("ERROR Unexpected data received:");

	log_print_data(rx_data->data + 1, rx_data->length);
	led_toggle(1);

	start_rx();
}

void tx_callback()
{
	log_print_string("TX callback");
	led_toggle(1);
	queue_clear(&tx_queue);
	//start_tx();
}

uint8_t tx_buffer[128];
uint8_t rx_buffer[128];


int main(void)
{
    system_init(tx_buffer, sizeof(tx_buffer), rx_buffer, sizeof(rx_buffer));
	log_print_string("started...");

	phy_init();
	phy_set_rx_callback(rx_callback);
	phy_set_tx_callback(tx_callback);

	#ifdef RX_MODE
		start_rx();
		while(1);
	#else
		start_tx();
		while(1);
	#endif

	return 0;
}

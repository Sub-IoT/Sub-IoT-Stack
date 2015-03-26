/*
 *  Created on: Mar 24, 2015
 *  Authors:
 *  	glenn.ergeerts@uantwerpen.be
 */


#include <string.h>
#include <stdio.h>

#include <hwleds.h>
#include <hwradio.h>
#include <log.h>

#define RX_MODE

hw_rx_cfg_t rx_cfg = {
    .channel_id = {
        .ch_coding = PHY_CODING_PN9,
        .ch_class = PHY_CLASS_NORMAL_RATE,
        .ch_freq_band = PHY_BAND_433,
        .center_freq_index = 5
    },
    .syncword_class = PHY_SYNCWORD_CLASS0
};

hw_tx_cfg_t tx_cfg = {
    .channel_id = {
        .ch_coding = PHY_CODING_PN9,
        .ch_class = PHY_CLASS_NORMAL_RATE,
        .ch_freq_band = PHY_BAND_433,
        .center_freq_index = 5
    },
    .syncword_class = PHY_SYNCWORD_CLASS0,
    .eirp = 10
};

static uint8_t tx_buffer[sizeof(hw_radio_packet_t) + 255] = { 0 };
static uint8_t rx_buffer[sizeof(hw_radio_packet_t) + 255] = { 0 };
hw_radio_packet_t* tx_packet = (hw_radio_packet_t*)tx_buffer;
hw_radio_packet_t* rx_packet = (hw_radio_packet_t*)rx_buffer;
static uint8_t data[] = {16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};


hw_radio_packet_t received_packet;

void transmit_packet()
{
    memcpy(&tx_packet->data, data, sizeof(data));
    hw_radio_send_packet(tx_packet);
}

hw_radio_packet_t* alloc_new_packet(uint8_t length)
{
	return rx_packet;
}

void release_packet(hw_radio_packet_t* packet)
{
	memset(rx_buffer, 0, sizeof(hw_radio_packet_t) + 255);
}

void packet_received(hw_radio_packet_t* packet)
{
	log_print_string("packet received");
	if(memcmp(data, packet->data, sizeof(data)) != 0)
		log_print_string("Unexpected data received!");
}

void packet_transmitted(hw_radio_packet_t* packet)
{
	led_on(0);
	log_print_string("packet transmitted");
    //timer_post_task(&transmit_packet, 100);
}

void rssi_measurement_done()
{}

void bootstrap()
{
    log_print_string("Device booted at time: %d\n", timer_get_counter_value()); // TODO not printed for some reason, debug later

    hw_radio_init(&alloc_new_packet, &release_packet, &packet_received, &packet_transmitted, &rssi_measurement_done);

    tx_packet->tx_meta.tx_cfg = tx_cfg;

	#ifdef RX_MODE
        hw_radio_set_rx(&rx_cfg);
	#else
        sched_register_task(&transmit_packet);
        timer_post_task(&transmit_packet, 0);
	#endif
}

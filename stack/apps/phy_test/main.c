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

//#define RX_MODE

hw_rx_cfg_t rx_cfg = {
    .channel_id = {
        .ch_coding = PHY_CODING_PN9,
        .ch_class = PHY_CLASS_NORMAL_RATE,
        .ch_freq_band = PHY_BAND_433,
        .center_freq_index = 0
    },
    .syncword_class = PHY_SYNCWORD_CLASS0
};

hw_tx_cfg_t tx_cfg = {
    .channel_id = {
        .ch_coding = PHY_CODING_PN9,
        .ch_class = PHY_CLASS_NORMAL_RATE,
        .ch_freq_band = PHY_BAND_433,
        .center_freq_index = 0
    },
    .syncword_class = PHY_SYNCWORD_CLASS0,
    .eirp = 10
};

static uint8_t buffer[sizeof(hw_radio_packet_t) + 255] = { 0 };
static uint8_t data[] = {16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
hw_radio_packet_t* packet = (hw_radio_packet_t*)buffer;

hw_radio_packet_t received_packet;

void tx_packet()
{
    memcpy(packet->data, data, sizeof(data));
    hw_radio_send_packet(packet);
}

hw_radio_packet_t* alloc_new_packet(uint8_t length)
{}

void release_packet(hw_radio_packet_t* packet)
{}

void packet_received(hw_radio_packet_t* packet)
{}

void packet_transmitted(hw_radio_packet_t* packet)
{
	led_on(0);
	log_print_string("packet transmitted");
	timer_post_task(&tx_packet, 100);
}

void bootstrap()
{
    log_print_string("Device booted at time: %d\n", timer_get_counter_value()); // TODO not printed for some reason, debug later

    hw_radio_init(&alloc_new_packet, &release_packet, &packet_received, &packet_transmitted, NULL);

    packet->tx_meta.tx_cfg = tx_cfg;

	#ifdef RX_MODE
        hw_radio_set_rx(&rx_cfg);
	#else
        sched_register_task(&tx_packet);
        timer_post_task(&tx_packet, 0);
	#endif
}

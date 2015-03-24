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
static uint8_t buffer[sizeof(hw_radio_packet_t) + 255] = { 0 };
static uint8_t data[] = {16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
hw_radio_packet_t* packet = (hw_radio_packet_t*)buffer;


hw_radio_packet_t received_packet;

hw_radio_packet_t* alloc_new_packet(uint8_t length)
{}

void release_packet(hw_radio_packet_t* packet)
{}

void packet_received(hw_radio_packet_t* packet)
{}

void packet_transmitted(hw_radio_packet_t* packet)
{}

void tx_packet()
{
    memcpy(packet->data, data, sizeof(data));
    hw_radio_send_packet(packet);
}

hw_rx_cfg_t rx_cfg = {
    .channel_id = {
        .ch_coding = 0x00, // PN9 // TODO define
        .ch_class = 0x00, // low rate // TODO define
        .ch_freq_band = 0x02, // 433 MHz band // TODO define
        .center_freq_index = 0x00 // TODO define
    },
    .syncword_class = 0x00 // TODO define
};

void bootstrap()
{
    log_print_string("Device booted at time: %d\n", timer_get_counter_value()); // TODO not printed for some reason, debug later

    hw_radio_init(&alloc_new_packet, &release_packet, &packet_received, &packet_transmitted, NULL);

	#ifdef RX_MODE
        hw_radio_set_rx(&rx_cfg);
	#else
        sched_register_task(&tx_packet);
        timer_post_task(&tx_packet, 0);
	#endif
}

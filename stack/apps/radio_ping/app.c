#include "types.h"
#include "scheduler.h"
#include "timer.h"
#include "hwradio.h"
#include "hwsystem.h"
#include "log.h"
#include "ng.h"
#include "random.h"


typedef struct __attribute__((__packed__))
{
	uint8_t length;
	uint16_t src_node;
	uint16_t dst_node;
	uint16_t counter;
} packet_struct_t;


uint8_t* rx_callback(uint8_t* buffer, uint8_t length)
{
	if(length < sizeof(packet_struct_t))
		log_print_string("Got an invalid packet at time %d: packet was too short", timer_get_counter_value());
	else
	{
		packet_struct_t* packet = (packet_struct_t*)buffer;
		if(packet->length != sizeof(packet_struct_t))
			log_print_string("Got an invalid packet at time %d: packet length didn't match", timer_get_counter_value());
		log_print_string("Got packet from %u to %u, seq: %u at time %u", packet->src_node, packet->dst_node, packet->counter, timer_get_counter_value());
	}
	log_print_string("Device booted at time: %d", timer_get_counter_value());
	return buffer;
}

uint8_t NGDEF(rx_buffer)[256];
packet_struct_t NGDEF(tx_buffer);

void send_packet()
{
	hw_radio_send_packet((uint8_t*)(&NG(tx_buffer)), sizeof(packet_struct_t));
	log_print_string("Sent packet with counter %u", NG(tx_buffer).counter);
	NG(tx_buffer).counter++;
	timer_post_task_delay(&send_packet, TIMER_TICKS_PER_SEC + (get_rnd() %TIMER_TICKS_PER_SEC));
}

void bootstrap()
{
    log_print_string("Device booted at time: %d\n", timer_get_counter_value());

    hw_radio_init(NG(rx_buffer), &rx_callback);
    hw_radio_setenabled(true);
    NG(tx_buffer).src_node = hw_get_unique_id();
    NG(tx_buffer).dst_node = 0xFFFF;
    NG(tx_buffer).counter = 0;
    NG(tx_buffer).length = sizeof(packet_struct_t);

    sched_register_task(&send_packet);
    timer_post_task_delay(&send_packet, TIMER_TICKS_PER_SEC + (get_rnd() %TIMER_TICKS_PER_SEC));

}



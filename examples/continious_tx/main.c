/*
 *  This program does a continuous TX on a specific channel
 *  Authors:
 *  	glenn.ergeerts@artesis.be
 */


#include <string.h>

#include <phy/phy.h>

#include <hal/system.h>
#include <hal/leds.h>


#define PACKET_LEN 19

static u8 packet[PACKET_LEN] = { 0x13, 0x50, 0xF1, 0x20, 0x59, 0x40, 0x46, 0x93, 0x21, 0xAB, 0x00, 0x31, 0x00, 0x24, 0x00, 0x00, 0x00, 0x01, 0x01 };

phy_tx_cfg_t tx_cfg = {
	0x12, // spectrum ID
	0, // coding scheme
	0,
	PACKET_LEN,
	packet
};

void main(void) {
	system_init();

	phy_init();

	while(1)
	{
        if (!phy_is_tx_in_progress()) {
        	led_toggle(3);
        	phy_tx(&tx_cfg);
        }

		system_lowpower_mode(4, 1);
	}
}

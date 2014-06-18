/*
 *  This program does a continuous TX on a specific channel
 *  Authors:
 *      glenn.ergeerts@artesis.be
 */


#include <string.h>

#include <phy/phy.h>

#include <hal/system.h>
#include <hal/leds.h>

#include <msp430.h>

#define PACKET_LEN 19
#define INTERRUPT_BUTTON1       (1)
#define CHANNEL_COUNT   8

static uint8_t packet[PACKET_LEN] = { 0x13, 0x50, 0xF1, 0x20, 0x59, 0x40, 0x46, 0x93, 0x21, 0xAB, 0x00, 0x31, 0x00, 0x24, 0x00, 0x00, 0x00, 0x01, 0x01 };

static uint8_t spectrum_ids[CHANNEL_COUNT] = { 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E }; // TODO only normal channel classes?

static uint8_t current_spectrum_index = 0;

static uint8_t interrupt_flags = 0;

phy_tx_cfg_t tx_cfg = {
	0x10,
	0,
	0,
	PACKET_LEN,
	packet
};

uint8_t get_next_spectrum_id()
{
        current_spectrum_index++;
        if(current_spectrum_index == CHANNEL_COUNT)
                current_spectrum_index = 0;

        return spectrum_ids[current_spectrum_index];
}

void main(void) {
        system_init();
        button_enable_interrupts();

        phy_init();

        while(1)
        {
        	if(!phy_is_tx_in_progress())
        	{
                led_toggle(3);
                phy_tx(&tx_cfg);
        	}

			if(INTERRUPT_BUTTON1 & interrupt_flags)
			{
				interrupt_flags &= ~INTERRUPT_BUTTON1;

				tx_cfg.spectrum_id = get_next_spectrum_id();

				button_clear_interrupt_flag();
				button_enable_interrupts();
			}

			system_lowpower_mode(4, 1);
        }
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR (void)
{
        if(button_is_active(1))
                interrupt_flags |= INTERRUPT_BUTTON1;
        else
                interrupt_flags &= ~INTERRUPT_BUTTON1;

        if(interrupt_flags != 0)
        {
                button_disable_interrupts();
                // TODO: debouncing
                LPM4_EXIT;
        }
}

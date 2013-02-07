/*
 * This program measures the RSSI values in all normal channels for noise measurements
 * The measured RSSI value and channel number is logged over UART.
 * The rssi_logger.py script parses this data.
 *  Created on: Feb 4, 2012
 *  Authors:
 *  	glenn.ergeerts@artesis.be
 */



#include <msp430.h> 

#include <phy/phy.h>

#include <ral/cc430/rf1a.h>
#include <ral/cc430/cc430_registers.h>

#define CC430_RSSI_OFFSET 74
#define INTERRUPT_RTC 		(1 << 3)
#define CHANNEL_COUNT 8

static u8 interrupt_flags = 0;

static u8 logbuffer[3] = {0xFE /* sync byte */, 0x00 /* channr */, 0x00 /* RSSI */};

static u8 spectrum_ids[CHANNEL_COUNT] = { 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E }; // TODO only normal channel classes?
static u8 current_spectrum_index = 0;

phy_rx_cfg_t rx_cfg = {
		0, // timeout
		0, // multiple
		0x10, // spectrum ID
		0, // coding scheme
		0, // RSSI min filter
		0x01  // sync word class
};

s8 get_rssi()
{
	// ensure the RSSI value is valid // TODO add to RAL code as well? (check after merge of phy_modifications branch)
	while ((RF1AIFG & BIT1) == 0x00);

    s8 rssi_raw = (s8) ReadSingleReg(RSSI);      // CC430 RSSI is 0.5 dBm units, signed byte
    s8 rssi = (int)rssi_raw;         // Convert to signed 16 bit (1 instr on MSP)
    rssi += 128;                      // Make it positive...
    rssi >>= 1;                        // ...So division to 1 dBm units can be a shift...
    rssi -= 64 + CC430_RSSI_OFFSET;     // ...and then rescale it, including offset
    RF1AIFG &= ~BIT1; // TODO not sure if this is still needed after the phy_modification branch is merged, but it is for now ...
    return rssi;
}

u8 get_next_spectrum_id()
{
	current_spectrum_index++;
	if(current_spectrum_index == CHANNEL_COUNT)
		current_spectrum_index = 0;

	return spectrum_ids[current_spectrum_index];
}

void rx_callback(phy_rx_res_t* res)
{
	// we received a valid packet
	// do nothing, RTC interrupt will restart rx
}

int main(void) {
	system_init();

	rtc_init_counter_mode();
	rtc_start();
	rtc_enable_interrupt();

	phy_init();
	phy_set_rx_callback(&rx_callback);

	// reconfigure radio so when don't wait for sync words etc
//	char mdmcfg2 =  RADIO_MDMCFG2_DEM_DCFILT_ON
//			| RADIO_MDMCFG2_MOD_FORMAT_GFSK // TODO MOD_FORMAT?
//			| RADIO_MDMCFG2_MANCHESTER_EN // TODO disable?
//			| RADIO_MDMCFG2_SYNC_MODE_16in16CS; // | RADIO_MDMCFG2_SYNC_MODE_NONE;
//	WriteSingleReg(MDMCFG2, mdmcfg2);

	phy_rx_start(&rx_cfg);

	while(1)
	{
        if (INTERRUPT_RTC & interrupt_flags)
		{
			led_toggle(3);
			logbuffer[1] = rx_cfg.spectrum_id;
			logbuffer[2] = get_rssi();
			uart_transmit_message(logbuffer, sizeof(logbuffer));

			phy_rx_stop();
			rx_cfg.spectrum_id = get_next_spectrum_id();
			phy_rx_start(&rx_cfg);

			interrupt_flags &= ~INTERRUPT_RTC;
		}

		system_lowpower_mode(4,1); // TODO skip this to keep radio powered on?
	}
}

#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR (void)
{
    switch (RTCIV){
        case 0: break;  //No interrupts
        case 2: break;  //RTCRDYIFG
        case 4:         //RTCEVIFG
        	interrupt_flags |= INTERRUPT_RTC;
        	LPM4_EXIT;
            break;
        case 6: break;  //RTCAIFG
        case 8: break;  //RT0PSIFG
        case 10: break; //RT1PSIFG
        default: break;
    }
}

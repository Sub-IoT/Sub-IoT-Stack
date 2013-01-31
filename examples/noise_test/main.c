#include <msp430.h> 

#include <phy/phy.h>

#include <ral/cc430/rf1a.h>
#include <ral/cc430/cc430_registers.h>

#define CC430_RSSI_OFFSET 74;

#define INTERRUPT_RTC 		(1 << 3)

static u8 interrupt_flags = 0;

static u8 logbuffer[3] = {0xFE /* sync byte */, 0x00 /* channr */, 0x00 /* RSSI */};

phy_rx_cfg_t rx_cfg = {
		0, // timeout
		0, // multiple TODO
		0x27, // spectrum ID TODO
		0, // coding scheme TODO
		0, // RSSI min filter TODO
		0x01  // sync word class TODO
};

s8 get_rssi()
{
    s8 rssi_raw = (s8) ReadSingleReg(RSSI);      // CC430 RSSI is 0.5 dBm units, signed byte
    s8 rssi = (int)rssi_raw;         // Convert to signed 16 bit (1 instr on MSP)
    rssi += 128;                      // Make it positive...
    rssi >>= 1;                        // ...So division to 1 dBm units can be a shift...
    rssi -= 64 + CC430_RSSI_OFFSET;     // ...and then rescale it, including offset
    return rssi;
}

void rx_callback(phy_rx_res_t* res)
{
	// we should not get here!
	led_on(1);
}

int main(void) {
	system_init();

	rtc_init_counter_mode();
	rtc_start();
	rtc_enable_interrupt();

	phy_init();
	phy_set_rx_callback(&rx_callback);

	// reconfigure radio so when don't wait for sync words etc
	char mdmcfg2 =  RADIO_MDMCFG2_DEM_DCFILT_ON
			| RADIO_MDMCFG2_MOD_FORMAT_GFSK // TODO MOD_FORMAT?
			// | RADIO_MDMCFG2_MANCHESTER_EN // TODO disable?
			| RADIO_MDMCFG2_SYNC_MODE_NONE;
	WriteSingleReg(MDMCFG2, mdmcfg2);

	phy_rx_start(&rx_cfg);

	while(1)
	{
        if (INTERRUPT_RTC & interrupt_flags)
		{
			led_toggle(3);
			logbuffer[2] = get_rssi();
			uart_transmit_message(logbuffer, sizeof(logbuffer));

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

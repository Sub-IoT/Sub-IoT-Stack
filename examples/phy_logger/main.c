#include <d7aoss.h>
#include <framework/log.h>
#include <hal/leds.h>


static dll_channel_scan_t scan_cfg;

static uint8_t buffer[128];


void start_rx()
{
	phy_rx_cfg_t rx_cfg;
	rx_cfg.length = 0;
	rx_cfg.timeout = scan_cfg.timeout_scan_detect; // timeout
	rx_cfg.spectrum_id[0] = scan_cfg.spectrum_id[0];
	rx_cfg.spectrum_id[1] = scan_cfg.spectrum_id[1];
	rx_cfg.scan_minimum_energy = -140;
	if (scan_cfg.scan_type == FrameTypeBackgroundFrame)
	{
		rx_cfg.sync_word_class = 0;
	} else {
		rx_cfg.sync_word_class = 1;
	}

	bool phy_rx_result = phy_rx(&rx_cfg);
	if (!phy_rx_result)
	{
		log_print_string("Starting channel scan FAILED");

		led_off(2);
		led_off(3);
	}
	else
	{
		led_on(2);
	}
}

void rx_callback(phy_rx_data_t* rx_res)
{

	if (rx_res != NULL)
	{
		led_on(3);
		log_phy_rx_res(rx_res);
		led_off(3);
	}

	start_rx();


}


int main(void) {
	// Initialize the OSS-7 Stack
	d7aoss_init(buffer, 128, buffer, 128);

	scan_cfg.spectrum_id[1] = 0x04;
	scan_cfg.spectrum_id[0] = 0x00;
	scan_cfg.scan_type = FrameTypeForegroundFrame;
	scan_cfg.time_next_scan = 0;
	scan_cfg.timeout_scan_detect = 0;


	phy_set_rx_callback(&rx_callback);
	
	log_print_string("started");
	while(1)
	{

		system_lowpower_mode(0,1);
	}

	return 0;
}

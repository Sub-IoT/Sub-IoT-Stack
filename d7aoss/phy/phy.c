/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 */

#include <stdbool.h>
#include <stdint.h>

#include "phy.h"
#include "../d7aoss.h"
#include "../ral/ral.h"

// Private function prototypes
static bool translate_settings(uint8_t spectrum_id, uint8_t sync_word_class, uint8_t* channel_center_freq_index, uint8_t* channel_bandwith_index, uint8_t* preamble_size, uint16_t* sync_word);

/*
 *
 * Public functions
 *
 */

void phy_init()
{
	RAL_IMPLEMENTATION.init();
}

void phy_tx(phy_tx_cfg* cfg)
{
	ral_tx_cfg ral_tx_cfg;

	if(RAL_IMPLEMENTATION.is_rx_in_progress() || RAL_IMPLEMENTATION.is_tx_in_progress())
		return; //TODO return error

	if(!translate_settings(cfg->spectrum_id, cfg->sync_word_class, &ral_tx_cfg.channel_center_freq_index, &ral_tx_cfg.channel_bandwidth_index, &ral_tx_cfg.preamble_size, &ral_tx_cfg.sync_word))
		return;  //TODO return error

	ral_tx_cfg.length = cfg->length;
	ral_tx_cfg.eirp = cfg->eirp;
	ral_tx_cfg.data = cfg->data;

	// TODO u16 timeout; // mac level?
	// TODO u8  cca;

	RAL_IMPLEMENTATION.tx(&ral_tx_cfg);

	//TODO return error
}

void phy_rx_start(phy_rx_cfg* cfg)
{
	ral_rx_cfg ral_rx_cfg;

	if(RAL_IMPLEMENTATION.is_rx_in_progress() || RAL_IMPLEMENTATION.is_tx_in_progress())
		return; //TODO return error

	if(!translate_settings(cfg->spectrum_id, cfg->sync_word_class, &ral_rx_cfg.channel_center_freq_index, &ral_rx_cfg.channel_bandwidth_index, &ral_rx_cfg.preamble_size, &ral_rx_cfg.sync_word))
		return; //TODO return error

	ral_rx_cfg.length = cfg->length;
	ral_rx_cfg.timeout = cfg->timeout;

	RAL_IMPLEMENTATION.rx_start(&ral_rx_cfg);

	//TODO return error
}

bool phy_read(phy_rx_data* data)
{
	ral_rx_data rx_data;

	if(RAL_IMPLEMENTATION.read(&rx_data))
	{
		data->data = rx_data.data;
		data->length = rx_data.length;
		data->rssi = rx_data.rssi;
		data->lqi = rx_data.lqi;
		data->crc_ok = rx_data.crc_ok;
		return true;
	}

	return false;
}

void phy_rx_stop(void)
{
	RAL_IMPLEMENTATION.rx_stop();
}

bool phy_is_rx_in_progress()
{
	return RAL_IMPLEMENTATION.is_rx_in_progress();
}

bool phy_is_tx_in_progress(void)
{
	return RAL_IMPLEMENTATION.is_tx_in_progress();
}

/*
 *
 * Private functions
 *
 */

static bool translate_settings(uint8_t spectrum_id, uint8_t sync_word_class, uint8_t* channel_center_freq_index, uint8_t* channel_bandwidth_index, uint8_t* preamble_size, uint16_t* sync_word)
{
	uint8_t fec = spectrum_id >> 7;
	*channel_center_freq_index = spectrum_id & 0x0F;
	*channel_bandwidth_index = (spectrum_id >> 4) & 0x07;

	//Assert valid spectrum id and set preamble size;
	if(*channel_bandwidth_index == 0) {
		if(*channel_center_freq_index == 0 || *channel_center_freq_index == 1)
			*preamble_size = 4;
		else
			return false;
	} else if(*channel_bandwidth_index == 1) {
		if(~*channel_center_freq_index & 0x01)
			*preamble_size = 4;
		else
			return false;
	} else if (*channel_bandwidth_index == 2) {
		if(*channel_center_freq_index & 0x01)
			*preamble_size = 6;
		else
			return false;
	} else if (*channel_bandwidth_index == 3) {
		if(*channel_center_freq_index == 2 || *channel_center_freq_index == 12)
			*preamble_size = 6;
		else
			return false;
	} else {
		return false;
	}

	//Assert valid sync word class and set sync word
	if(sync_word_class == 0) {
		if(fec == 0)
			*sync_word = SYNC_CLASS0_NON_FEC;
		else
			*sync_word = SYNC_CLASS0_FEC;
	} else if(sync_word_class == 1) {
		if(fec == 0)
			*sync_word = SYNC_CLASS1_NON_FEC;
		else
			*sync_word = SYNC_CLASS1_FEC;
	} else {
	   return false;
	}

	return true;
}

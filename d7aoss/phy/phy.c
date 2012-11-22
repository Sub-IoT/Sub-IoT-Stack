/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#include "../log.h"

#include "phy.h"
#include "../d7aoss.h" // TODO remove

static phy_rx_callback_t rx_callback;
static phy_tx_callback_t tx_callback;

static void translate_spectrum_id(u8 spectrum_id, u8* channel_center_freq_index, u8* channel_bandwith_index)
{
	(*channel_center_freq_index) = spectrum_id & 0x0F;
	(*channel_bandwith_index) = (spectrum_id >> 4) & 0x0F;
}

static void rx_completed(ral_rx_res_t* rx_result)
{
	// TODO add PHY processing, FEC, ... ?
	Log_Packet(rx_result->data); // TODO other params
	rx_callback((phy_rx_res_t*)rx_result); // TODO phy_rx_res_t same as ral_rx_res_t for now ...
}

static void tx_completed()
{
	// TODO add PHY processing ... ?
	// TODO log
	tx_callback(); // TODO what do this params mean?
}

void phy_init()
{
	RAL_IMPLEMENTATION.init();
	RAL_IMPLEMENTATION.set_rx_callback(&rx_completed);
	RAL_IMPLEMENTATION.set_tx_callback(&tx_completed);
}

phy_result_t phy_tx(phy_tx_cfg_t* cfg)
{
	if(RAL_IMPLEMENTATION.is_rx_in_progress())
		return PHY_RADIO_IN_RX_MODE;

	ral_tx_cfg_t ral_tx_cfg;
	translate_spectrum_id(cfg->spectrum_id, &ral_tx_cfg.channel_center_freq_index, &ral_tx_cfg.channel_bandwith_index);
	ral_tx_cfg.coding_scheme = 0; // TODO
	ral_tx_cfg.sync_word_class = 0; // TODO
	ral_tx_cfg.data = cfg->data;
	ral_tx_cfg.len = cfg->data[0];
	// TODO u16 timeout; // mac level?
	// TODO u8  cca;
	// TODO s8  eirp;

	Log_Packet(cfg->data); // TODO log other params

	RAL_IMPLEMENTATION.tx(&ral_tx_cfg);

	return PHY_OK;
}

void phy_set_tx_callback(phy_tx_callback_t cb)
{
	tx_callback = cb;
}

void phy_set_rx_callback(phy_rx_callback_t cb)
{
	rx_callback = cb;
}

void phy_rx_start(phy_rx_cfg_t* cfg)
{
	// TODO check valid spectrum id

	ral_rx_cfg_t ral_rx_cfg;
	ral_rx_cfg.timeout = cfg->timeout;
	ral_rx_cfg.multiple = cfg->multiple;
	ral_rx_cfg.rssi_min = cfg->rssi_min;
	ral_rx_cfg.sync_word_class = cfg->sync_word_class;
	translate_spectrum_id(cfg->spectrum_id, &ral_rx_cfg.channel_center_freq_index, &ral_rx_cfg.channel_bandwith_index);
	ral_rx_cfg.coding_scheme = cfg->coding_scheme;

	RAL_IMPLEMENTATION.rx_start(&ral_rx_cfg);
}

void phy_rx_stop()
{
	RAL_IMPLEMENTATION.rx_stop();
}

u8 phy_is_rx_in_progress()
{
	return RAL_IMPLEMENTATION.is_rx_in_progress();
}

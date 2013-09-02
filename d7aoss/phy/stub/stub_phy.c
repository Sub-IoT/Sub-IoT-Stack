/*
 * stub_phy.c
 *
 *  Created on: 18-feb.-2013
 *      Author: Miesalex
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../phy.h"

phy_tx_callback_t phy_tx_callback = NULL;
phy_rx_callback_t phy_rx_callback = NULL;

void phy_set_tx_callback(phy_tx_callback_t cb)
{
	phy_tx_callback = cb;
}
void phy_set_rx_callback(phy_rx_callback_t cb)
{
	phy_rx_callback = cb;
}

/*
 * Phy implementation functions
 */
void phy_init(void)
{
	return;
}

void phy_idle(void)
{
	return;
}

bool phy_tx(phy_tx_cfg_t* cfg)
{
	return true;
}

bool phy_rx(phy_rx_cfg_t* cfg)
{
	return true;
}

bool phy_read(phy_rx_data_t* data)
{
	return false;
}

bool phy_is_rx_in_progress(void)
{
	return false;
}

bool phy_is_tx_in_progress(void)
{
	return false;
}

bool phy_cca(uint8_t spectrum_id, uint8_t sync_word_class)
{
	return true;
}

/*
 * stub_phy.c
 *
 *  Created on: 18-feb.-2013
 *      Author: Miesalex
 */

#include <stdbool.h>
#include <stdint.h>

#include "../phy.h"

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

bool phy_cca(void)
{
	return true;
}

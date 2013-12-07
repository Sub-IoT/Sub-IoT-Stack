/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 *     	glenn.ergeerts@uantwerpen.be
 */

#include <stdbool.h>
#include <stdint.h>

#include "../phy.h"
#include "../../simulation/Castalia/src/node/application/oss7Test/SimPhyInterface.h"

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
    return sim_phy_tx(cfg);
}

bool phy_rx(phy_rx_cfg_t* cfg)
{
    return sim_phy_rx(cfg);
}

bool phy_read(phy_rx_data_t* data)
{
	return false;
}

bool phy_is_rx_in_progress(void)
{
    return false; // TODO
}

bool phy_is_tx_in_progress(void)
{
    return false; // TODO
}

int16_t phy_get_rssi(uint8_t spectrum_id, uint8_t sync_word_class)
{
    return 0;
}

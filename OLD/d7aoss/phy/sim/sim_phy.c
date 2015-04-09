/*
 * (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

int16_t phy_get_rssi(uint8_t spectrum_id[2], uint8_t sync_word_class)
{
    // TODO get actuall rssi value
    // problem: we need to wait for the radio to enter RX and the RSSI to become valid before returning here.
    // However we cannot block here until this value can be read since we need to return from handleMessage() before
    // the message which switches the radio to RX is delivered/processed.
    // I don't see a solution right now...
    // Alternatively this could work if the phy API would use a callback for when the RSSI reading is completed.
    // Or if the DLL would ensure the radio is switched to RX and the time needed for a valid RSSI reading is reached before calling this.
    return -120;
}

void phy_keep_radio_on(bool keep_on)
{
    
}
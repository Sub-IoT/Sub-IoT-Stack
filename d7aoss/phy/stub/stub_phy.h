/*

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
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
 *     	maarten.weyn@uantwerpen.be
 *		Miesalex
 */


#ifndef STUB_PHY_H_
#define STUB_PHY_H_

#include "../phy.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Phy implementation function prototypes
 */
void phy_init(void);
void phy_idle(void);
bool phy_tx(phy_tx_cfg_t* cfg);
bool phy_rx(phy_rx_cfg_t* cfg);
bool phy_is_rx_in_progress(void);
bool phy_is_tx_in_progress(void);
bool phy_cca(uint8_t spectrum_id, uint8_t sync_word_class);

#ifdef __cplusplus
}
#endif

#endif /* STUB_PHY_H_ */


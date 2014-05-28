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


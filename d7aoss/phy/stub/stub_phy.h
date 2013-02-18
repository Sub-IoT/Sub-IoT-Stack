/*
 * stub_phy.h
 *
 *  Created on: 18-feb.-2013
 *      Author: Miesalex
 */

#ifndef STUB_PHY_H_
#define STUB_PHY_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Phy implementation function prototypes
 */
void phy_init(void);
bool phy_tx(phy_tx_cfg* cfg);
bool phy_rx_start(phy_rx_cfg* cfg);
void phy_rx_stop(void);
bool phy_read(phy_rx_data* data);
bool phy_is_rx_in_progress(void);
bool phy_is_tx_in_progress(void);
bool phy_cca(void);

#ifdef __cplusplus
}
#endif

#endif /* STUB_PHY_H_ */

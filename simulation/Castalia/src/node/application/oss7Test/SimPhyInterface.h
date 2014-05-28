#ifndef SIMPHYINTERFACE_H
#define SIMPHYINTERFACE_H

#include <stdint.h>

// callable from OSS7 stack
#ifdef __cplusplus
extern "C" {
#endif

#include "phy.h"

bool sim_phy_rx(phy_rx_cfg_t*);
bool sim_phy_tx(phy_tx_cfg_t*);
//void sim_phy_get_rssi();

#ifdef __cplusplus
}
#endif

#endif // SIMPHYINTERFACE_H

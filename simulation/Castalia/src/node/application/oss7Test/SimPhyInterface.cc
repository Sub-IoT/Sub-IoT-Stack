#include "SimPhyInterface.h"

#include "Oss7Test.h"

bool sim_phy_rx(phy_rx_cfg_t* rx_cfg)
{
    return oss7_node()->oss7_api_phy_rx(rx_cfg);
}

bool sim_phy_tx(phy_tx_cfg_t* tx_cfg)
{
    return oss7_node()->oss7_api_phy_tx(tx_cfg);
}

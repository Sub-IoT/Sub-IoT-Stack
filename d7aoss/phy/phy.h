/*
 * The PHY layer API
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef PHY_H_
#define PHY_H_

#include "../types.h"

// TODO implement FEC

// =======================================================================
// phy_rx_cfg_t
// -----------------------------------------------------------------------
/// Configuration structure for packet reception
// =======================================================================
typedef struct
{
    /// Timeout value (0 : continuous) in ticks
    u16 timeout;
    /// Multiple or single reception flag
    u8  multiple;
    /// Spectrum ID
    u8  spectrum_id;
    /// Coding scheme (0 : PN9 coding, 1 : PN9 + D7 FEC)
    u8  coding_scheme;
    /// RSSI min filter
    u8  rssi_min;
    /// sync word class (0 or 1)
    u8 sync_word_class;
    /// TODO CCA
} phy_rx_cfg_t;

// =======================================================================
// phy_tx_cfg_t
// -----------------------------------------------------------------------
/// Configuration structure for packet transmission
// =======================================================================
typedef struct
{
    /// Timeout
    // TODO u16 timeout; // mac level?
    /// CCA mode
    // TODO u8  cca;
    /// Spectrum ID
    u8  spectrum_id;
    /// Channel bandhwith index
	u8  coding_scheme;
	/// sync word class (0 or 1)
	u8 sync_word_class;
    /// pointer to the payload
    u8* data; // TODO data should exclude length?
    /// data length
    u8  len;
    /// Transmission power level in dBm ranged [-39, +10]
    s8  eirp; // TODO set by MAC? part of data?
} phy_tx_cfg_t;

// =======================================================================
// phy_rx_res_t
// -----------------------------------------------------------------------
/// Packet reception result structure
// =======================================================================
typedef struct
{
    /// Reception status
    //u8  crc_ok;
    /// Reception level
    s8  rssi;
    /// Reported EIRP
    s8  eirp;
    /// Link quality indicator
    u8  lqi;
    /// packet length in bytes
    u8  len;
    /// packet data
    u8*  data;

} phy_rx_res_t; // TODO same as ral_rx_res_t for now ...

typedef enum
{
	PHY_OK,
	PHY_RADIO_IN_RX_MODE
} phy_result_t; // TODO return this for all functions


typedef void (*phy_tx_callback_t)(); // TODO result param?
typedef void (*phy_rx_callback_t)(phy_rx_res_t*);

void phy_init();
phy_result_t phy_tx(phy_tx_cfg_t*);
void phy_set_tx_callback(phy_tx_callback_t);
void phy_set_rx_callback(phy_rx_callback_t);

void phy_rx_start(phy_rx_cfg_t*);
void phy_rx_stop();
bool phy_is_rx_in_progress();
u8 phy_cca(void);

#endif /* PHY_H_ */

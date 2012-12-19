/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef RAL_H_
#define RAL_H_

#include "../types.h"
#include <stdbool.h>

// TODO who decides to power down the radio?

typedef struct
{
    /// Timeout value (0 : continuous)
    u16 timeout;
    /// Multiple or single reception flag
    u8  multiple;
    /// Channel center frequency index
    u8  channel_center_freq_index;
    /// Channel bandhwith index
    u8	channel_bandwith_index;
    /// Coding scheme (0 : PN9 coding, 1 : PN9 + D7 FEC)
	u8  coding_scheme;
	/// sync word class (0 or 1)
	u8 sync_word_class;
	/// RSSI min filter
    u8  rssi_min; // TODO

    /// TODO CCA

} ral_rx_cfg_t;

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

} ral_rx_res_t;

typedef struct
{
    /// Timeout
    // TODO u16 timeout; // mac level?
    /// CCA mode
    // TODO u8  cca;
    /// Channel center frequency index
    u8  channel_center_freq_index;
    /// Channel bandhwith index
    u8	channel_bandwith_index;
    /// Coding scheme (0 : PN9 coding, 1 : PN9 + D7 FEC)
	u8  coding_scheme;
	/// sync word class (0 or 1)
	u8 sync_word_class;
    /// pointer to the payload
    u8* data;
    /// data length
    u8  len;
    /// Transmission power level in dBm ranged [-39, +10]
    // TODO s8  eirp;
} ral_tx_cfg_t;

typedef void (*ral_tx_callback_t)(); // TODO param
typedef void (*ral_rx_callback_t)(ral_rx_res_t*);

// The interface a RAL implementation has to implement
struct ral_interface {
  void (* init)(void);
  void (* tx)(ral_tx_cfg_t*);
  void (* set_tx_callback)(ral_tx_callback_t);
  void (* set_rx_callback)(ral_rx_callback_t);
  void (* rx_start)(ral_rx_cfg_t*);
  void (* rx_stop)();
  bool (* is_rx_in_progress)();
  bool (* cca)();
//  int (* on)(void);
//  int (* off)(void);
};

#endif /* RAL_H_ */

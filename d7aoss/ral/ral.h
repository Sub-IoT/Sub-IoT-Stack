/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 */

#ifndef RAL_H
#define RAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// TODO who decides to power down the radio?

typedef struct
{
	uint8_t channel_center_freq_index;		//Channel center frequency index
	uint8_t	channel_bandwidth_index;		//Channel bandwidth index
	uint8_t preamble_size;					//Number of preamble symbols
	uint8_t length;							//Packet length (0 : variable)
	uint16_t sync_word;						//Sync Word
	uint16_t timeout;						//Timeout value (0 : continuous)
} ral_rx_cfg;

typedef struct
{
	uint8_t crc_ok;		//Cyclic redundancy check status
    uint8_t lqi;		//Link quality indicator
    uint8_t length;		//Packet length
    uint8_t* data;		//Packet data
    int16_t rssi;		//Received signal strength indicator
} ral_rx_data;

typedef struct
{
	uint8_t channel_center_freq_index;		//Channel center frequency index
	uint8_t	channel_bandwidth_index;		//Channel bandwidth index
	uint8_t eirp;							//Transmission power level in dBm ranged [-39, +10]
	uint8_t preamble_size;					//Number of preamble symbols
	uint8_t length;							//Packet length (0 : variable)
	uint8_t* data;							//Packet data
    uint16_t sync_word;						//Sync Word
} ral_tx_cfg;

// The interface a RAL implementation has to implement
struct ral_interface
{
  void (* init)(void);
  void (* tx)(ral_tx_cfg*);
  void (* rx_start)(ral_rx_cfg*);
  void (* rx_stop)(void);
  bool (* read)(ral_rx_data*);
  bool (* is_rx_in_progress)(void);
  bool (* is_tx_in_progress)(void);
  bool (* cca)(void);

};

#ifdef __cplusplus
}
#endif

#endif /* RAL_H */

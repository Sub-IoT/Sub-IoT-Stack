/*
 * The PHY layer API
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 */

#ifndef PHY_H
#define PHY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define SYNC_CLASS0_NON_FEC      0xE6D0
#define SYNC_CLASS0_FEC          0xF498
#define SYNC_CLASS1_NON_FEC      0x0B67
#define SYNC_CLASS1_FEC          0x192F

//Configuration structure for packet reception
typedef struct
{
	uint8_t spectrum_id;		//Spectrum ID
	uint8_t sync_word_class;	//Sync word class
	uint8_t length;				//Packet length (0 : variable)
	uint16_t timeout;			//Timeout value (0 : continuous) in milliseconds
} phy_rx_cfg;

//Packet reception result structure
typedef struct
{
	uint8_t crc_ok;				//Cyclic redundancy check status
    uint8_t lqi;				//Link quality indicator
    uint8_t length;				//Packet length
    uint8_t* data;				//Packet data
    int16_t rssi;				//Received signal strength indicator
} phy_rx_data;

//Configuration structure for packet transmission
typedef struct
{
	uint8_t spectrum_id;		//Spectrum ID
	uint8_t sync_word_class;	//Sync word class
	uint8_t eirp;				//Transmission power level in dBm ranged [-39, +10]
	uint8_t* data;				//Packet data
	uint16_t length;			//Packet length
} phy_tx_cfg;

void phy_init(void);
void phy_tx(phy_tx_cfg* cfg);
void phy_rx_start(phy_rx_cfg* cfg);
void phy_rx_stop(void);
bool phy_read(phy_rx_data* data);
bool phy_is_rx_in_progress(void);
bool phy_is_tx_in_progress(void);
bool phy_cca(void);

#ifdef __cplusplus
}
#endif

#endif /* PHY_H */

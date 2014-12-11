/*! \file phy.h
 *
 * \copyright (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.\n
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.\n
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * \author glenn.ergeerts@uantwerpen.be
 * \author maarten.weyn@uantwerpen.be
 *	\author alexanderhoet@gmail.com
 *
 *	\brief The PHY layer API
 */

#ifndef PHY_H
#define PHY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define CHANNEL_CODE_INDEX_CODING_PN9						0 << 6
#define CHANNEL_CODE_INDEX_CODING_FEC_PN9					2 << 6
#define CHANNEL_CODE_INDEX_CODING_CHANNEL_CLASS_LO_RATE		0 << 4
#define CHANNEL_CODE_INDEX_CODING_CHANNEL_CLASS_NORMAL_RATE	1 << 4
#define CHANNEL_CODE_INDEX_CODING_CHANNEL_CLASS_HI_RATE		2 << 4

//#define D7_PHY_USE_FEC //TODO move to general config file

#define SYNC_CLASS0_CS0     0xE6D0
#define SYNC_CLASS0_CS1     0xF498
#define SYNC_CLASS1_CS0     0x0B67
#define SYNC_CLASS1_CS1     0x192F
#define INITIAL_PN9				0x01FF
#define CCA_RSSI_THRESHOLD		-86 // TODO configurable



//Configuration structure for packet reception
typedef struct
{
	uint8_t spectrum_id[2];		//Spectrum ID
	uint8_t sync_word_class;	//Sync word class
	uint8_t length;				//Packet length (0 : variable)
	uint16_t timeout;			//Timeout value (0 : continuous) in milliseconds
	int16_t scan_minimum_energy;
} phy_rx_cfg_t;

//Packet reception result structure
typedef struct
{
    uint8_t lqi;				//Link quality indicator
    uint8_t length;				//Packet length
    uint8_t* data;				//Packet data
    int16_t rssi;				//Received signal strength indicator
    uint8_t spectrum_id[2];
    uint8_t sync_word_class;
} phy_rx_data_t;

//Configuration structure for packet transmission
typedef struct
{
	uint8_t spectrum_id[2];		//Spectrum ID
	uint8_t sync_word_class;	//Sync word class
	int8_t eirp;				//Transmission power level in dBm ranged [-39, +10]
	//uint8_t length;				//Packet length
	//uint8_t* data;				//Packet data
} phy_tx_cfg_t;

//Define callback funtion pointer types
typedef void (*phy_tx_callback_t)(void);
typedef void (*phy_rx_callback_t)(phy_rx_data_t*);

//Declare callback function pointers
extern phy_tx_callback_t phy_tx_callback;
extern phy_rx_callback_t phy_rx_callback;

//Phy interface
extern void phy_init(void);
extern void phy_idle(void);
extern bool phy_tx(phy_tx_cfg_t* cfg);
extern bool phy_rx(phy_rx_cfg_t* cfg);
extern bool phy_read(phy_rx_data_t* data);
extern bool phy_is_rx_in_progress(void);
extern bool phy_is_tx_in_progress(void);
extern int16_t phy_get_rssi(uint8_t spectrum_id[2], uint8_t sync_word_class);

extern void dissable_autocalibration();
extern void enable_autocalibration();
extern void manual_calibration();

extern void phy_keep_radio_on(bool);

//Implementation independent phy functions
void phy_set_tx_callback(phy_tx_callback_t);
void phy_set_rx_callback(phy_rx_callback_t);
bool phy_cca(uint8_t spectrum_id[2], uint8_t sync_word_class);
bool phy_translate_settings(uint8_t spectrum_id[2], uint8_t sync_word_class, bool* fec, uint8_t* frequency_band, uint8_t* channel_center_freq_index, uint8_t* channel_bandwidth_index, uint8_t* preamble_size, uint16_t* sync_word);


#ifdef __cplusplus
}
#endif

#endif /* PHY_H */

/*! \file phy.c
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
 * \author alexanderhoet@gmail.com
 *
 *	\brief The PHY layer API implementation
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "phy.h"


phy_tx_callback_t phy_tx_callback = NULL;
phy_rx_callback_t phy_rx_callback = NULL;

void phy_set_tx_callback(phy_tx_callback_t cb)
{
	phy_tx_callback = cb;
}
void phy_set_rx_callback(phy_rx_callback_t cb)
{
	phy_rx_callback = cb;
}

bool phy_cca(uint8_t spectrum_id[2], uint8_t sync_word_class)
{
    return (bool)(phy_get_rssi(spectrum_id, sync_word_class) < CCA_RSSI_THRESHOLD);
}

bool phy_translate_settings(uint8_t spectrum_id[2], uint8_t sync_word_class, bool* fec, uint8_t* frequency_band, uint8_t* channel_center_freq_index, uint8_t* channel_channel_class, uint8_t* preamble_size, uint16_t* sync_word)
{
	// TODO: implement freq band
	*fec = (bool)((spectrum_id[0] & 0x02) >> 1);
	*channel_center_freq_index = spectrum_id[1];
	*channel_channel_class = (spectrum_id[0] >> 2) & 0x03;
	*frequency_band = (spectrum_id[0] >> 4) & 0x0F;

	//Assert valid spectrum id and set preamble size;
	if(*channel_channel_class == 0) { // Lo-Rate
		//if(*channel_center_freq_index == 0 || *channel_center_freq_index == 1)
			*preamble_size = 4;
		//else
		//	return false;
	} else if(*channel_channel_class == 1) { // Normal Rate
		//if(~*channel_center_freq_index & 0x01)
			*preamble_size = 4;
		//else
		//	return false;
	} else if (*channel_channel_class == 2) { // H-Rate
		//f(*channel_center_freq_index & 0x01)
			*preamble_size = 6;
		//else
			//return false;
	}
	//else if (*channel_bandwidth_index == 3) {
//		if(*channel_center_freq_index == 2 || *channel_center_freq_index == 12)
//			*preamble_size = 6;
//		else
//			return false;
//	} else {
//		return false;
//	}

	//Assert valid sync word class and set sync word
	if(sync_word_class == 0) {
		if(*fec == 0)
			*sync_word = SYNC_CLASS0_CS0;
		else
			*sync_word = SYNC_CLASS0_CS1;
	} else if(sync_word_class == 1) {
		if(*fec == 0)
			*sync_word = SYNC_CLASS1_CS0;
		else
			*sync_word = SYNC_CLASS1_CS1;
	} else {
	   return false;
	}

	return true;
}

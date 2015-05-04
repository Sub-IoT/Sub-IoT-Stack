/*! \file phy.c
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

/*
 * The PHY layer API
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 */

#include "phy.h"

bool phy_translate_settings(uint8_t spectrum_id, uint8_t sync_word_class, bool* fec, uint8_t* channel_center_freq_index, uint8_t* channel_bandwidth_index, uint8_t* preamble_size, uint16_t* sync_word)
{
	*fec = (bool)spectrum_id >> 7;
	*channel_center_freq_index = spectrum_id & 0x0F;
	*channel_bandwidth_index = (spectrum_id >> 4) & 0x07;

	//Assert valid spectrum id and set preamble size;
	if(*channel_bandwidth_index == 0) {
		if(*channel_center_freq_index == 0 || *channel_center_freq_index == 1)
			*preamble_size = 4;
		else
			return false;
	} else if(*channel_bandwidth_index == 1) {
		if(~*channel_center_freq_index & 0x01)
			*preamble_size = 4;
		else
			return false;
	} else if (*channel_bandwidth_index == 2) {
		if(*channel_center_freq_index & 0x01)
			*preamble_size = 6;
		else
			return false;
	} else if (*channel_bandwidth_index == 3) {
		if(*channel_center_freq_index == 2 || *channel_center_freq_index == 12)
			*preamble_size = 6;
		else
			return false;
	} else {
		return false;
	}

	//Assert valid sync word class and set sync word
	if(sync_word_class == 0) {
		if(*fec == 0)
			*sync_word = SYNC_CLASS0_NON_FEC;
		else
			*sync_word = SYNC_CLASS0_FEC;
	} else if(sync_word_class == 1) {
		if(*fec == 0)
			*sync_word = SYNC_CLASS1_NON_FEC;
		else
			*sync_word = SYNC_CLASS1_FEC;
	} else {
	   return false;
	}

	return true;
}

/*
 * trans.h
 *
 *  Created on: 21-feb.-2013
 *      Author: Dragan Subotic
 *      		Maarten Weyn
 */

#ifndef TRANS_H_
#define TRANS_H_

#include "../types.h"
#include "../nwl/nwl.h"

typedef enum {
	TransPacketSent,
	TransPacketFail,
	TransTCAFail
} Trans_Tx_Result;

typedef void (*trans_tx_callback_t)(Trans_Tx_Result);
void trans_init();

void trans_set_tx_callback(trans_tx_callback_t);
void trans_tx_foreground_frame(uint8_t* data, uint8_t length, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp);
//void trans_tx_background_frame(uint8_t* data, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp);


void trans_rigd_ccp(uint8_t spectrum_id, bool init_status);

#endif /* TRANS_H_ */

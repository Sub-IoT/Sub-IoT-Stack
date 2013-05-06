/*
 * trans.h
 *
 *  Created on: 21-feb.-2013
 *      Author: Dragan Subotic
 */

#ifndef TRANS_H_
#define TRANS_H_

#include "../types.h"
#include "../phy/phy.h"
#include "../dll/dll.h"

typedef enum {
	TransPacketSent,
	TransPacketFail,
	TransTCAFail
} Trans_Tx_Result;

typedef void (*trans_tx_callback_t)(Trans_Tx_Result);
void trans_init();

void trans_set_tx_callback(trans_tx_callback_t);
void trans_tx_foreground_frame(u8* data, u8 length, uint8_t subnet, u8 spectrum_id, s8 tx_eirp);
void trans_tx_background_frame(u8* data, u8 subnet, u8 spectrum_id, s8 tx_eirp);


void trans_rigd_ccp(u8 spectrum_id, bool init_status);

#endif /* TRANS_H_ */

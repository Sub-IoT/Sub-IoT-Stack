/*! \file trans.h
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
 * \author maarten.weyn@uantwerpen.be
 * \author Dragan.Subotic@uantwerpen.be
 *
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

typedef struct {
	uint8_t lenght;
	uint8_t* payload;
} Trans_Rx_Datastream_Result;

typedef void (*trans_tx_callback_t)(Trans_Tx_Result);
typedef void (*trans_rx_datastream_callback_t)(Trans_Rx_Datastream_Result);
void trans_init();

void trans_set_tx_callback(trans_tx_callback_t);
void trans_set_datastream_rx_callback(trans_rx_datastream_callback_t);
void trans_set_initial_t_ca(uint16_t t_ca);


void trans_tx_foreground_frame(uint8_t* data, uint8_t length, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp);
void trans_tx_datastream(uint8_t* data, uint8_t length, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp);
//void trans_tx_background_frame(uint8_t* data, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp);


void trans_rx_datastream_start(uint8_t subnet, uint8_t spectrum_id);
void trans_rx_datastream_stop();



void trans_rigd_ccp(uint8_t spectrum_id, bool init_status, bool wait_for_t_ca_timeout);

#endif /* TRANS_H_ */

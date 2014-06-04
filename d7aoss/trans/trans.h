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

#define D7AQP_COMMAND_CODE_EXTENSION				1 << 7
#define D7AQP_COMMAND_TYPE_RESPONSE					0 << 4
#define D7AQP_COMMAND_TYPE_ERROR_RESPONSE			1 << 4
#define D7AQP_COMMAND_TYPE_NA2P_REQUEST				2 << 4
#define D7AQP_COMMAND_TYPE_A2P_INIT_REQUEST			4 << 4
#define D7AQP_COMMAND_TYPE_A2P_INTERMEDIATE_REQUEST	5 << 4
#define D7AQP_COMMAND_TYPE_A2P_FINAL_REQUEST		7 << 4
#define D7AQP_OPCODE_ANNOUNCEMENT_FILE				0
#define D7AQP_OPCODE_ANNOUNCEMENT_FILE_SERIES		1
#define D7AQP_OPCODE_INVENTORY_FILE					2
#define D7AQP_OPCODE_INVENTORY_FILE_SERIES			3
#define D7AQP_OPCODE_COLLECTION_FILE_FILE			4
#define D7AQP_OPCODE_COLLECTION_SERIES_FILE			5
#define D7AQP_OPCODE_COLLECTION_FILE_SERIES			6
#define D7AQP_OPCODE_COLLECTION_SERIES_SERIES		7
#define D7AQP_OPCODE_REQUEST_DATASTREAM				8
#define D7AQP_OPCODE_PROPOSE_DATASTREAM				9
#define D7AQP_OPCODE_ACKNOWLEDGE_DATASTREAM			10
#define D7AQP_OPCODE_APPLICATION_SHELL				15

#define D7AQP_COMMAND_EXTENSION_CA_RIGD				0 << 3
#define D7AQP_COMMAND_EXTENSION_CA_RAIND			1 << 3
#define D7AQP_COMMAND_EXTENSION_CA_AIND				2 << 3
#define D7AQP_COMMAND_EXTENSION_CA_DEFAULT			7 << 3
#define D7AQP_COMMAND_EXTENSION_NOCSMA				1 << 2
#define D7AQP_COMMAND_EXTENSION_NORESPONSE			1 << 1


#include "../types.h"
#include "../nwl/nwl.h"

typedef enum {
	TransCsmaCaAind,
	TransCsmaCaRaind,
	TransCsmaCaRigd
} Trans_CSMA_CA_Type;

typedef enum {
	TransPacketSent,
	TransPacketFail,
	TransTCAFail
} Trans_Tx_Result;

typedef struct {
	uint16_t response_timeout;
	uint8_t response_channel_list_lenght;
	uint8_t* response_channel_list;
} D7AQP_Dialog_Template;

typedef struct {
	uint8_t number_of_acks;
	uint8_t* device_ids;
} D7AQP_Ack_Template;

typedef struct {
	uint8_t rfu;
} D7AQP_Global_Query_Template;

typedef struct {
	uint8_t rfu;
} D7AQP_Local_Query_Template;


typedef struct {
	uint8_t rfu;
} D7AQP_Error_Template;

typedef struct {
	uint8_t return_file_id;
	uint8_t	file_offset;
	uint8_t isfb_total_length;
	uint8_t* file_data;
} D7AQP_Single_File_Return_Template;

typedef struct {
	uint8_t max_returned_bytes;
	uint8_t	return_file_id;
	uint8_t return_file_entry_offset;
} D7AQP_Single_File_Call_Template;

typedef struct {

	uint8_t lenght;
	uint8_t* payload;
} Trans_Rx_Datastream_Result;

typedef struct {
	uint8_t command_code;
	uint8_t command_extension;
	D7AQP_Dialog_Template* dialog_template;
	D7AQP_Ack_Template* ack_template;
	D7AQP_Global_Query_Template* global_query_template;
	D7AQP_Local_Query_Template* local_query_template;
	D7AQP_Error_Template* error_template;
	void* command_data;
} D7AQP_Command;

typedef struct {
	D7AQP_Command d7aqp_command;
	nwl_rx_res_t* nwl_rx_res;
} Trans_Rx_Query_Result;

typedef void (*trans_tx_callback_t)(Trans_Tx_Result);
typedef void (*trans_rx_datastream_callback_t)(Trans_Rx_Datastream_Result*);
typedef void (*trans_rx_query_callback_t)(Trans_Rx_Query_Result*);

void trans_init();

void trans_set_tx_callback(trans_tx_callback_t);
void trans_set_query_rx_callback(trans_rx_query_callback_t);
void trans_set_datastream_rx_callback(trans_rx_datastream_callback_t);
void trans_set_initial_t_ca(uint16_t t_ca);
void trans_set_csma_ca(Trans_CSMA_CA_Type type);


void trans_tx_foreground_frame(uint8_t* data, uint8_t length, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp);
void trans_tx_datastream(uint8_t* data, uint8_t length, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp);
//void trans_tx_background_frame(uint8_t* data, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp);

void trans_tx_query(D7AQP_Command* command, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp);


void trans_rx_datastream_start(uint8_t subnet, uint8_t spectrum_id);
void trans_rx_query_start(uint8_t subnet, uint8_t spectrum_id);
void trans_rx_stop();

static void trans_initiate_csma_ca(uint8_t spectrum_id);
static void trans_process_csma_ca();
//AIND
void trans_aind_ccp(bool init_status);
static void trans_aind_ccp_process();
//RIGD
void trans_rigd_ccp(bool wait_for_t_ca_timeout);
static void t_ca_timeout_rigd();
static void final_rigd();


#endif /* TRANS_H_ */

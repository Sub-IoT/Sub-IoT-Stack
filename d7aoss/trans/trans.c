/*! \file trans.c
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


#include "trans.h"
#include "../hal/system.h"
#include "../dll/dll.h"
#include "../alp/alp.h"
#include "../nwl/nwl.h"
#include "../framework/timer.h"
#include "../framework/log.h"
#include <string.h>

static trans_tx_callback_t trans_tx_callback;
//static trans_rx_datastream_callback_t trans_rx_datastream_callback;
static trans_rx_alp_callback_t trans_rx_alp_callback;

static Trans_Rx_Alp_Result alp_result;
//static D7AQP_Dialog_Template dialog_template;

//Flow Control Process
static void control_tx_callback(Dll_Tx_Result Result)
{
	switch(Result){
			case DLLTxResultOK:
				#ifdef LOG_TRANS_ENABLED
				log_print_stack_string(LOG_TRANS, "Trans: Packet is sent");
				#endif
				trans_tx_callback(TransPacketSent);
				break;
			case DLLTxResultFail:
				trans_tx_callback(TransPacketFail);
				#ifdef LOG_TRANS_ENABLED
				log_print_stack_string(LOG_TRANS, "Trans: Fail to sent");
				#endif
				break;
		}
	return;
}

static void nwl_rx_callback(nwl_rx_res_t* result)
{
	alp_result.nwl_rx_res = result;

	if (result->protocol_type == ProtocolTypeNetworkProtocol)
	{
		nwl_ff_D7ANP_t* data = (nwl_ff_D7ANP_t*) result->data;
		D7AQP_Command command;

		command.control = data->payload[0];
		command.transaction_id = data->payload[1];
		command.query_template = NULL;
		command.ack_template = NULL;
		command.alp_data = &data->payload[2];
		command.alp_length = data->payload_length - 2;

		alp_result.alp_record.record_flags = data->payload[2];
		alp_result.alp_record.record_lenght = data->payload[3];
		alp_result.alp_record.alp_id = data->payload[4];
		alp_result.alp_record.alp_templates = &data->payload[5];

		if ((uint8_t)(alp_result.alp_record.record_flags & ALP_REC_FLG_TYPE_COMMAND_RESPONSE) == ALP_REC_FLG_TYPE_COMMAND_RESPONSE)
		{
			// todo: from where to get subnet and tx_eirp, get spectrum_id, target_id from query
			uint8_t subnet = result->dll_rx_res->frame->subnet;
			int8_t tx_eirp = (int8_t) (result->dll_rx_res->frame->control && 0x3F) - 32;

			trans_execute_query(command.alp_data, ALP_REC_FLG_TYPE_RESPONSE, file_system_user_guest, subnet, result->dll_rx_res->spectrum_id, tx_eirp, 0, NULL);
		}



		if (trans_rx_alp_callback != NULL)
		{
			trans_rx_alp_callback(&alp_result);
		}


	}


	/*
	else if (result->protocol_type == ProtocolTypeDatastreamProtocol)
	{
		Trans_Rx_Datastream_Result datastream_result;
		nwl_ff_D7ADP_t* data = (nwl_ff_D7ADP_t*) result->data;

		//TODO: check data->frame_id?
		datastream_result.lenght = data->payload_length;
		datastream_result.payload = data->payload;

		trans_rx_datastream_callback(&datastream_result);
	}
	*/
}

void trans_init(){
	nwl_init();
	nwl_set_tx_callback(&control_tx_callback);
	nwl_set_rx_callback(&nwl_rx_callback);
}

void trans_set_tx_callback(trans_tx_callback_t cb)
{
	trans_tx_callback = cb;
}

void trans_set_alp_rx_callback(trans_rx_alp_callback_t cb)
{
	trans_rx_alp_callback = cb;
}

void trans_tx_foreground_frame(uint8_t* data, uint8_t length, uint8_t subnet, uint8_t spectrum_id[2], int8_t tx_eirp) {
	queue_clear(&tx_queue);
	queue_push_u8_array(&tx_queue, data, length);

	nwl_build_network_protocol_data(NWL_CONTRL_SRC_UID, NULL, NULL, NULL, 0, subnet, spectrum_id, tx_eirp);
	dll_initiate_csma_ca();
}


/*! \brief Creates a D7AQP Query (transport layer)
 *
 *  Creates a D7AQP Query based on the Query Template
 *  An ALP Records should be added to the Tx Queue prior to calling this method
 *
 *  \param D7AQP_Query_Template Optional query template
 *  \param subnet The subnet which needs to be used to send the query
 *  \param spectrum_id The spectrum_id which needs to be used to send the query
 *  \param tx_eirp The transmit EIRP which need to be used to send the query
 *  \param cca Use CCA (default = true)
 *  \todo Todo: implement unicast
 */
void trans_tx_query(D7AQP_Query_Template* query,  uint8_t subnet, uint8_t spectrum_id[2], int8_t tx_eirp, bool cca)
{

	if (query != NULL)
	{
		#ifdef LOG_TRANS_ENABLED
		log_print_stack_string(LOG_TRANS, "Query is not yet implemented");
		#endif

		return;
	}

	queue_create_header_space(&tx_queue, 2);
	tx_queue.front[0] =  D7AQP_CONTROL_DIALOG_SINGLE; // Control byte
	tx_queue.front[1] =  0; // Transaction ID
	nwl_build_network_protocol_data(NWL_CONTRL_SRC_UID, NULL, NULL, NULL, 0, subnet, spectrum_id, tx_eirp);
	if (cca)
		dll_initiate_csma_ca();
	else
		dll_tx_frame();

	//uint8_t data[64];//TODO: should be dynamic or queue
	//uint8_t pointer = 0;
	//uint8_t i = 0;


	/*
	data[pointer++] = command->command_code;
	if (command->command_code & D7AQP_COMMAND_CODE_EXTENSION)
		data[pointer++] = command->command_extension;

	if (command->dialog_template != NULL)
	{
		data[pointer++] = command->dialog_template->response_timeout >> 8;
		data[pointer++] = command->dialog_template->response_timeout & 0xFF;
		data[pointer++] = command->dialog_template->response_channel_list_lenght;

		for (i=0;i<command->dialog_template->response_channel_list_lenght;i++)
			data[pointer++] = command->dialog_template->response_channel_list[i];
	}

	switch (command->command_code & 0x70)
	{
		// Response
		case D7AQP_COMMAND_TYPE_RESPONSE:
		{
			switch (command->command_code & 0x0F)
			{
				case D7AQP_OPCODE_ANNOUNCEMENT_FILE:
				{
					break;
				}
				case D7AQP_OPCODE_COLLECTION_FILE_FILE:
				{
					D7AQP_Single_File_Return_Template* sfr_tmpl = (D7AQP_Single_File_Return_Template*) command->command_data; //following convetion, feels unsafe
					data[pointer++] = sfr_tmpl->return_file_id;
					data[pointer++] = sfr_tmpl->file_offset;
					data[pointer++] = sfr_tmpl->isfb_total_length;
					memcpy(&data[pointer], sfr_tmpl->file_data, sfr_tmpl->isfb_total_length - sfr_tmpl->file_offset);
					pointer+= sfr_tmpl->isfb_total_length - sfr_tmpl->file_offset;
					break;
				}
			}
			break;
		}

		// NA2P Request
		case D7AQP_COMMAND_TYPE_NA2P_REQUEST:
		{
			switch (command->command_code & 0x0F)
			{
				case D7AQP_OPCODE_ANNOUNCEMENT_FILE:
				{
					D7AQP_Single_File_Return_Template* sfr_tmpl = (D7AQP_Single_File_Return_Template*) command->command_data;
					data[pointer++] = sfr_tmpl->return_file_id;
					data[pointer++] = sfr_tmpl->file_offset;
					data[pointer++] = sfr_tmpl->isfb_total_length;

					memcpy(&data[pointer], sfr_tmpl->file_data, sfr_tmpl->isfb_total_length - sfr_tmpl->file_offset);
					pointer+= sfr_tmpl->isfb_total_length - sfr_tmpl->file_offset;
				}
				break;
			}
			break;
		}
	}
	 */



}

//void trans_tx_datastream(uint8_t* data, uint8_t length, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp) {
//	nwl_build_datastream_protocol_data(data, length, NULL, subnet, spectrum_id, tx_eirp, dialogid++);
//	dll_initiate_csma_ca();
//}

//void trans_rx_datastream_start(uint8_t subnet, uint8_t spectrum_id)
//{
//	nwl_rx_start(subnet, spectrum_id, ProtocolTypeDatastreamProtocol);
//}


void trans_rx_query_start(uint8_t subnet, uint8_t spectrum_id[2])
{
	nwl_rx_start(subnet, spectrum_id, ProtocolTypeNetworkProtocol);
}

void trans_rx_stop()
{
	nwl_rx_stop();
}

void trans_execute_query(uint8_t* alp, uint8_t alp_response_type, file_system_user user, uint8_t subnet, uint8_t spectrum_id[2], int8_t tx_eirp, uint8_t target_id_length, uint8_t* target_id)
{

	//uint8_t alp_rec_flags =  alp[pointer++]; // 0
	//uint8_t alp_rec_length =  alp[pointer++]; // 1
	//uint8_t alp_alp_id =  alp[pointer++]; // 2
	//uint8_t alp_op =  alp[pointer++]; // 3
	//uint8_t alp_id =  fs_read_byte(&nf, nf_pointer++); // 4
	uint8_t alp_offset=  MERGEUINT16(alp[5], alp[6]); // 5, 6
	uint8_t alp_length =  MERGEUINT16(alp[7], alp[8]); // 7, 8

	// TODO: only singular templates are currently supported
	uint8_t alp_nr_of_templates = 1;

	ALP_File_Data_Template data_template;
	ALP_Template alp_template;

	alp_template.op = ALP_OP_RESP_DATA;
	alp_template.data = (uint8_t*) &data_template;

	if (alp[3] == ALP_OP_READ_DATA)
	{
		data_template.file_id = alp[4];
		data_template.start_byte_offset = alp_offset;
		data_template.bytes_accessing = alp_length;

		file_handler fh;
		uint8_t res = fs_open(&fh, data_template.file_id, user, file_system_access_type_read);

		if (res != 0)
		{
			#ifdef LOG_TRANS_ENABLED
			log_print_stack_string(LOG_TRANS, "Cannot access requested file");
			#endif

			return;
		}

		data_template.data = fs_get_data_pointer(&fh, (uint8_t) alp_offset);
	}

	alp_create_structure_for_tx(alp_response_type, alp[2], alp_nr_of_templates, &alp_template);
	trans_tx_query(NULL, subnet, spectrum_id, tx_eirp, true);
}


//void trans_tx_background_frame(uint8_t* data, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp){
//	nwl_build_background_frame(data, subnet, spectrum_id, tx_eirp);
//}

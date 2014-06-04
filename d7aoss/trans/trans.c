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
#include "../framework/timer.h"
#include "../framework/log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static uint16_t current__t_ca = 0;
static uint8_t current__t_g = 0;
//static uint8_t current__spectrum_id = 0;
static uint16_t init_t_ca = 400;
static uint16_t last_ca = 0;

static Trans_CSMA_CA_Type csma_ca_type = TransCsmaCaAind;

static uint8_t dialogid = 0;

static trans_tx_callback_t trans_tx_callback;
static trans_rx_datastream_callback_t trans_rx_datastream_callback;
static trans_rx_query_callback_t trans_rx_query_callback;

static Trans_Rx_Query_Result query_result;
static D7AQP_Dialog_Template dialog_template;

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
			case DLLTxResultCCAOK:
				dll_tx_frame();
				break;
			case DLLTxResultCCA1Fail:
			case DLLTxResultCCA2Fail:
				trans_process_csma_ca();
				#ifdef LOG_TRANS_ENABLED
				log_print_stack_string(LOG_TRANS, "Trans: CCA fail");
				#endif
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
	if (result->protocol_type == ProtocolTypeBackgroundProtocol)
	{
		// Should this be here?
	}
	else if (result->protocol_type == ProtocolTypeNetworkProtocol)
	{
		query_result.nwl_rx_res = result;

		nwl_ff_D7ANP_t* data = (nwl_ff_D7ANP_t*) result->data;

		uint8_t pointer = 0;
		query_result.d7aqp_command.command_code = data->payload[pointer++];
		if (query_result.d7aqp_command.command_code & D7AQP_COMMAND_CODE_EXTENSION)
			query_result.d7aqp_command.command_extension = data->payload[pointer++];
		else
			query_result.d7aqp_command.command_extension = 0;

		if (query_result.d7aqp_command.command_extension & D7AQP_COMMAND_EXTENSION_NORESPONSE)
		{
			query_result.d7aqp_command.dialog_template = NULL;
		} else {
			dialog_template.response_timeout = (uint16_t)(data->payload[pointer++] << 8) | (uint16_t)(data->payload[pointer++]);
			dialog_template.response_channel_list_lenght = data->payload[pointer++];
			dialog_template.response_channel_list = &data->payload[pointer];
			pointer += dialog_template.response_channel_list_lenght;

			query_result.d7aqp_command.dialog_template = &dialog_template;
		}

		//TODO: implement other queries
		query_result.d7aqp_command.ack_template = NULL;
		query_result.d7aqp_command.global_query_template = NULL;
		query_result.d7aqp_command.local_query_template = NULL;
		query_result.d7aqp_command.error_template = NULL;

		switch (query_result.d7aqp_command.command_code & 0x70)
		{
			// Response
			case D7AQP_COMMAND_TYPE_RESPONSE:
			{
				switch (query_result.d7aqp_command.command_code & 0x0F)
				{
					case D7AQP_OPCODE_ANNOUNCEMENT_FILE:
					{
						query_result.d7aqp_command.command_data = NULL;
						break;
					}
				}
				break;
			}

			// NA2P Request
			case D7AQP_COMMAND_TYPE_NA2P_REQUEST:
			{
				switch (query_result.d7aqp_command.command_code & 0x0F)
				{
					case D7AQP_OPCODE_ANNOUNCEMENT_FILE:
					{
						D7AQP_Single_File_Return_Template sfr_tmpl;
						sfr_tmpl.return_file_id = data->payload[pointer++];
						sfr_tmpl.file_offset = data->payload[pointer++];
						sfr_tmpl.isfb_total_length = data->payload[pointer++];
						sfr_tmpl.file_data = &data->payload[pointer];

						pointer += sfr_tmpl.isfb_total_length - sfr_tmpl.file_offset;

						query_result.d7aqp_command.command_data = &sfr_tmpl;
					}
				}
				break;
			}
		}

		trans_rx_query_callback(&query_result);
	}
	else if (result->protocol_type == ProtocolTypeDatastreamProtocol)
	{
		Trans_Rx_Datastream_Result datastream_result;
		nwl_ff_D7ADP_t* data = (nwl_ff_D7ADP_t*) result->data;

		//TODO: check data->frame_id?
		datastream_result.lenght = data->payload_length;
		datastream_result.payload = data->payload;

		trans_rx_datastream_callback(&datastream_result);
	}
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

void trans_set_datastream_rx_callback(trans_rx_datastream_callback_t cb)
{
	trans_rx_datastream_callback = cb;
}

void trans_set_query_rx_callback(trans_rx_query_callback_t cb)
{
	trans_rx_query_callback = cb;
}


void trans_set_initial_t_ca(uint16_t t_ca)
{
	init_t_ca = t_ca;
}


/*! \brief Sets the type of CSMA CA
 *
 *  Sets the type of CSMA CA, options are AIND, RAIND and RIGD
 *
 *  \todo implement RAIND
 *
 *  \param type The CSMA CA Algorithm to be used
 */
void trans_set_csma_ca(Trans_CSMA_CA_Type type)
{
	csma_ca_type = type;
}

static void trans_aind_ccp_process()
{
	uint16_t time_since_last_ca = timer_get_counter_value() - last_ca;
	if (current__t_ca < time_since_last_ca)
	{
		#ifdef LOG_TRANS_ENABLED
		log_print_stack_string(LOG_TRANS, "AIND: Failed");
		#endif
		trans_tx_callback(TransTCAFail);
		return;
	}

	current__t_ca -= time_since_last_ca;

	last_ca = timer_get_counter_value();
	dll_csma(true);
}

static void final_rigd() {
	 dll_csma(true);
}

static void t_ca_timeout_rigd() {
	trans_rigd_ccp(false);
}

static void trans_initiate_csma_ca(uint8_t spectrum_id)
{
	//current__spectrum_id = spectrum_id;
	current__t_ca = init_t_ca;

	// Calculate correct t_g
	uint8_t channel_bandwidth_index = (spectrum_id >> 4) & 0x07;
	uint8_t fec = (bool)spectrum_id >> 7;

	if (channel_bandwidth_index == 1)
		current__t_g = fec == 0 ? 5 : 10;
	else
		current__t_g = fec == 0 ? 2 : 3;

	switch (csma_ca_type)
	{
	case TransCsmaCaAind:
		trans_aind_ccp(true);
		break;
	case TransCsmaCaRaind:
		//break;
	case TransCsmaCaRigd:
		trans_rigd_ccp(false);
		break;
	}
}

static void trans_process_csma_ca()
{
	switch (csma_ca_type)
	{
	case TransCsmaCaAind:
		trans_aind_ccp(false);
		break;
	case TransCsmaCaRaind:
		break;
	case TransCsmaCaRigd:
		trans_rigd_ccp(true);
		break;
	}
}


void trans_tx_foreground_frame(uint8_t* data, uint8_t length, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp) {
	nwl_build_network_protocol_data(data, length, NULL, NULL, subnet, spectrum_id, tx_eirp, dialogid++);
	trans_initiate_csma_ca(spectrum_id);
}


/*! \brief Creates a D7AQP Query (transport layer)
 *
 *  Creates a D7AQP Query based on the Command Request Template
 *  Currently only Announcement of file is uses
 *
 *  \todo implement other query
 *
 *  \param request_template Pointer to the Command Request Template
 *  \param file_template The corresponding file template
 *  \param subnet The subnet which needs to be used to send the query
 *  \param spectrum_id The spectrum_id which needs to be used to send the query
 *  \param tx_eirp The transmit EIRP which need to be used to send the query
 */
void trans_tx_query(D7AQP_Command* command, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp)
{
	uint8_t data[64];//TODO: should be dynamic or queue
	uint8_t pointer = 0;
	uint8_t i = 0;

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

	nwl_build_network_protocol_data(data, pointer, NULL, NULL, subnet, spectrum_id, tx_eirp, dialogid++);
	trans_initiate_csma_ca(spectrum_id);
}

void trans_tx_datastream(uint8_t* data, uint8_t length, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp) {
	nwl_build_datastream_protocol_data(data, length, NULL, subnet, spectrum_id, tx_eirp, dialogid++);
	trans_initiate_csma_ca(spectrum_id);
}

void trans_rx_datastream_start(uint8_t subnet, uint8_t spectrum_id)
{
	nwl_rx_start(subnet, spectrum_id, ProtocolTypeDatastreamProtocol);
}


void trans_rx_query_start(uint8_t subnet, uint8_t spectrum_id)
{
	nwl_rx_start(subnet, spectrum_id, ProtocolTypeNetworkProtocol);
}

void trans_rx_stop()
{
	nwl_rx_stop();
}


//void trans_tx_background_frame(uint8_t* data, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp){
//	nwl_build_background_frame(data, subnet, spectrum_id, tx_eirp);
//}

/*! \brief Transport Layer CSMA-CA Congestion Control Process according to the Adaptive Increase No Division (AIND) algorithm
 *
 *  The Congentstion Control Process acccording to the AIND algorithm
 *  AIND CSMA-CA is a process where ad-hoc slotting takes place, the insertion happens at the beginning of the
 *	slot, and the slot duration is equal (approximately) to the duration of the transmission being queued.
 *
 *	\todo Calculate wait duration
 *
 *  \param spectrum_id The Spectrum ID used for the CCA
 *  \param init_status Flag to indicate if the process needs to be initiated.
 */
void trans_aind_ccp(bool init_status)
{
	timer_event event;

	// Initialisation of the parameters, only for new packets
	if (init_status) {
		last_ca = timer_get_counter_value();
		trans_aind_ccp_process();
	} else {
		// wait for transmission duration
		event.next_event = 5; // todo: calculate read transmission duration
		event.f = &trans_aind_ccp_process;
		timer_add_event(&event);
		return;
	}
}

/*! \brief Transport Layer CSMA-CA Congestion Control Process according to the Random Increase Geometric Division (RIGD) algorithm
 *
 *  The Congentstion Control Process acccording to the RIGD algorithm
 *  RIGD CA is a process where ad-hoc slotting takes place, the slot insertion is random, and the slot duration decays
 *	by the model (TCA0)(1 / 2(n+1)), where n >= 0 and TCA0 is the duration of the timeout for all slots.
 *
 *
 *  \param spectrum_id The Spectrum ID used for the CCA
 *  \param wait_for_t_ca_timeout Flag to indicate if the process needs to wait for a Tca timeout.
 */
void trans_rigd_ccp(bool wait_for_t_ca_timeout){
	timer_event event;

	if (wait_for_t_ca_timeout)
	{
		uint16_t time_since_last_ca = timer_get_counter_value() - last_ca;
		if (time_since_last_ca < current__t_ca)
		{
			event.next_event = current__t_ca - time_since_last_ca;
			event.f = &t_ca_timeout_rigd;
			timer_add_event(&event);
			return;
		}
	}

	current__t_ca = current__t_ca >> 1; // = % 2
	if (current__t_ca > current__t_g) {
		uint32_t n_time = rand();
		n_time = (n_time * current__t_ca) >> 15; // Random Time before the CCA will be executed

		#ifdef LOG_TRANS_ENABLED
		log_print_stack_string(LOG_TRANS, "RIGD: Wait Random Time: %d", (uint16_t) n_time);
		#endif

		event.next_event = (uint16_t) n_time; // Wait random time (0 - new__t_ca)
		event.f = &final_rigd;
		last_ca = timer_get_counter_value();
		timer_add_event(&event);
	} else {
		#ifdef LOG_TRANS_ENABLED
		log_print_stack_string(LOG_TRANS, "RIGD: Failed");
		#endif
		trans_tx_callback(TransTCAFail);
		return;
	}
}


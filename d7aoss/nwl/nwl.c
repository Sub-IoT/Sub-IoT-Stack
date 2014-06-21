/*! \file nwl.h
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
 * \author	maarten.weyn@uantwerpen.be
 *
 */

#include <string.h>

#include "nwl.h"
#include "../framework/log.h"
#include "../hal/system.h"


static nwl_rx_callback_t nwl_rx_callback;
static nwl_tx_callback_t nwl_tx_callback;

static uint8_t dll_data[100]; //TODO: get rid of fixed array

nwl_rx_res_t res;
static nwl_background_frame_t bf;
//static nwl_ff_D7ADP_t d7adp_frame;
static nwl_ff_D7ANP_t d7anp_frame;;

static void dll_tx_callback(Dll_Tx_Result status)
{
	nwl_tx_callback(status);
}

static void dll_rx_callback(dll_rx_res_t* result)
{
	if (result == NULL)
		return;

	res.dll_rx_res = result;

	if (result->frame_type == FrameTypeBackgroundFrame)
	{
		bf.tx_eirp = 0;
		bf.subnet = result->frame->subnet;
		bf.bpid = result->frame->payload[0];
		memcpy((void*) bf.protocol_data,(void*) &(result->frame->payload[1]), 2);

		res.data = &bf;
		res.protocol_type = ProtocolTypeBackgroundProtocol;

		//TODO: should a BF be send to transport layer??
	}
	{
		d7anp_frame.d7anls_auth_data = NULL;
		d7anp_frame.d7anls_header = NULL;
		d7anp_frame.d7anp_routing_header = NULL;
		d7anp_frame.payload_length = result->frame->payload_length;
		d7anp_frame.payload = &(result->frame->payload[0]);

		res.data = &d7anp_frame;
		res.protocol_type = ProtocolTypeNetworkProtocol;
	}

	nwl_rx_callback(&res);
}

void nwl_init()
{
	dll_init();
	dll_set_tx_callback(&dll_tx_callback);
	dll_set_rx_callback(&dll_rx_callback);
}

void nwl_set_tx_callback(nwl_tx_callback_t cb)
{
	nwl_tx_callback = cb;
}

void nwl_set_rx_callback(nwl_rx_callback_t cb)
{
	nwl_rx_callback = cb;
}

void nwl_build_background_frame(nwl_background_frame_t* frame, uint8_t spectrum_id, int8_t tx_eirp, uint8_t subnet)
{
	dll_tx_cfg_t dll_params;
	dll_params.eirp = tx_eirp;
	dll_params.spectrum_id = spectrum_id;
	dll_params.subnet = subnet;
	dll_params.frame_type = FrameTypeBackgroundFrame;

	dll_create_frame((uint8_t*) &(frame->bpid), 3, NULL, 0, &dll_params);
}

void nwl_build_advertising_protocol_data(uint8_t channel_id, uint16_t eta, int8_t tx_eirp, uint8_t subnet)
{
	nwl_background_frame_t frame;
	frame.bpid = BPID_AdvP;
	// change to MSB
	frame.protocol_data[0] = eta >> 8;
	frame.protocol_data[1] = eta & 0XFF;

	nwl_build_background_frame(&frame, channel_id, tx_eirp, subnet);
}

void nwl_build_network_protocol_data(uint8_t* data, uint8_t length, nwl_security* security, nwl_routing_header* routing, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp, uint8_t dialog_id)
{
	uint8_t offset = 0;

	dll_tx_cfg_t dll_params;
	dll_params.eirp = tx_eirp;
	dll_params.spectrum_id = spectrum_id;
	dll_params.subnet = subnet;
	dll_params.frame_type = FrameTypeForegroundFrame;

	if (security != NULL)
	{
		#ifdef LOG_NWL_ENABLED
		log_print_stack_string(LOG_NWL, "NWL: security not implemented");
		#endif

	}

	if (routing != NULL)
	{
		#ifdef LOG_NWL_ENABLED
		log_print_stack_string(LOG_NWL, "NWL: routing not implemented");
		#endif
	}

	//dll_data[offset++] = length; // payload length;

	memcpy(&dll_data[offset], data, length);

	uint8_t dll_data_length = offset + length;

	//TODO: assert dll_data_length < 255-7
	dll_create_frame(dll_data, dll_data_length, NULL, 0, &dll_params);
}

void nwl_build_datastream_protocol_data(uint8_t* data, uint8_t length, nwl_security* security, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp, uint8_t dialog_id)
{
	uint8_t offset = 0;

	dll_tx_cfg_t dll_params;
	dll_params.eirp = tx_eirp;
	dll_params.spectrum_id = spectrum_id;
	dll_params.subnet = subnet;
	dll_params.frame_type = FrameTypeForegroundFrame;


	memcpy(&dll_data[offset], data, length);

	uint8_t dll_data_length = offset + length;

	//TODO: assert dll_data_length < 255-7
	dll_create_frame(dll_data, dll_data_length, NULL, 0, &dll_params);
}


void nwl_rx_start(uint8_t subnet, uint8_t spectrum_id, Protocol_Type type)
{
	dll_channel_scan_t scan_cfg = {
			spectrum_id,
			FrameTypeForegroundFrame,
			0,
			0
	};

	//scan_cfg.spectrum_id = spectrum_id;

	if (type == ProtocolTypeBackgroundProtocol)
		scan_cfg.scan_type = FrameTypeBackgroundFrame;

	dll_channel_scan_series_t scan_series_cfg;
	scan_series_cfg.length = 1;
	scan_series_cfg.values = &scan_cfg;

	dll_channel_scan_series(&scan_series_cfg);
}

void nwl_rx_stop()
{
	dll_stop_channel_scan();
}

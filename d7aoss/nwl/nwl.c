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
#include "../framework/timer.h"
#include "../hal/system.h"


static nwl_rx_callback_t nwl_rx_callback;
static nwl_tx_callback_t nwl_tx_callback;

nwl_rx_res_t res;
static nwl_background_frame_t bf;
//static nwl_ff_D7ADP_t d7adp_frame;
static nwl_ff_D7ANP_t d7anp_frame;
static volatile bool process_callback = true;
static volatile bool tx_callback_received = false;
static dll_channel_scan_t scan_cfg;
static dll_channel_scan_series_t scan_series_cfg;


static void dll_tx_callback(Dll_Tx_Result status)
{
	if (process_callback)
		nwl_tx_callback(status);

	tx_callback_received = true;
}

static void dll_rx_callback(dll_rx_res_t* result)
{
	if (result == NULL)
		return;

	res.dll_rx_res = result;
	dll_frame_t* frame = (dll_frame_t*) result->frame;

	if (result->frame_type == FrameTypeBackgroundFrame)
	{
		bf.bpid = result->frame->payload[0];
		bf.data_length = 2; // currently both types are lenght 2
		bf.protocol_data = &(frame->payload[1]);

		res.data = &bf;

		switch (bf.bpid)
		{
		case 0xF0:
			res.protocol_type = ProtocolTypeAdvertisementProtocol;
			break;
//		case 0xF1:
//			res.protocol_type = ProtocolTypeBeaconProtocol;
//			break;
		}
	} else
	{

		d7anp_frame.control = frame->payload[0];

		d7anp_frame.d7anls_header = NULL;

		switch(d7anp_frame.control & NWL_CONTRL_SRC_FULL)
		{
			case NWL_CONTRL_SRC_VID:
				d7anp_frame.source_access_templ_length = 2;
				break;
			case NWL_CONTRL_SRC_UID:
				d7anp_frame.source_access_templ_length = 8;
				break;
			case NWL_CONTRL_SRC_FULL:
			{
				nwl_full_access_template* access = (nwl_full_access_template*) &(frame->payload[1]);
				if (access->control & NWL_ACCESS_TEMPL_CTRL_VID)
					d7anp_frame.source_access_templ_length = 4;
				else
					d7anp_frame.source_access_templ_length = 10;

				d7anp_frame.source_access_templ_length += access->control & 0x0F;
				break;
			}
			default:
				d7anp_frame.source_access_templ_length = 0;
		}

		if (d7anp_frame.source_access_templ_length == 0)
		{
			d7anp_frame.d7anp_source_access_templ = NULL;
		} else {
			d7anp_frame.d7anp_source_access_templ = &(frame->payload[1]);
		}

		d7anp_frame.d7anls_auth_data = NULL;

		d7anp_frame.payload_length = frame->payload_length - (1 + d7anp_frame.source_access_templ_length);
		d7anp_frame.payload = &(frame->payload[1 +  d7anp_frame.source_access_templ_length]);

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

/*! \brief Prepares the data link layer for a background frame  (Network Layer)
 *
 * 	Sets the headers for the background frame. The should be in the TX_Queue prior to calling this function.
 *
 *  \param uint8_t spectrum_id 	The channel on which to send the background frame.
 *  \param uint8_t tx_eirp 		The send EIRP.
 *  \param uint8_t subnet 		The subnet to of the background frame.
 */
static void nwl_build_background_frame(uint8_t spectrum_id[2], int8_t tx_eirp, uint8_t subnet)
{
	dll_tx_cfg_t dll_params;
	dll_params.eirp = tx_eirp;
	memcpy(dll_params.spectrum_id, spectrum_id, 2);
	dll_params.subnet = subnet;
	dll_params.frame_type = FrameTypeBackgroundFrame;

	dll_create_frame(NULL, 0, &dll_params);
}

/** \copydoc nwl_build_advertising_protocol_data */
void nwl_build_advertising_protocol_data(uint16_t eta, uint8_t spectrum_id[2], int8_t tx_eirp, uint8_t subnet)
{
	queue_clear(&tx_queue);

	// BPID
	queue_push_u8(&tx_queue, BPID_AdvP);

	// protocol data
	// change to MSB
	queue_push_u8(&tx_queue, eta >> 8);
	queue_push_u8(&tx_queue, eta & 0XFF);

	nwl_build_background_frame(spectrum_id, tx_eirp, subnet);
}
/** \copydoc nwl_build_advp_sync_train */
void nwl_build_advp_sync_train(uint16_t duration, uint8_t spectrum_id[2], int8_t tx_eirp, uint8_t subnet)
{
	uint16_t advp_target_timestamp = timer_get_counter_value() + duration;

	phy_keep_radio_on(true);
	process_callback = false;

	uint16_t eta = duration; //advp_target_timestamp - timer_get_counter_value();

	//nwl_event.next_event =

	while (eta > 5 && eta <= duration)
	{
		tx_callback_received = false;

		nwl_build_advertising_protocol_data(eta, spectrum_id, tx_eirp, subnet);
		dll_tx_frame();

		while (!tx_callback_received);

		__delay_cycles(8000);

		eta = advp_target_timestamp - timer_get_counter_value();
	}


	phy_keep_radio_on(false);


	process_callback = true;
}

/** \copydoc nwl_build_beaconprotocol_data */
/*
void nwl_build_beaconprotocol_data(uint8_t spectrum_id[2], int8_t tx_eirp, uint8_t subnet)
{
	queue_clear(&tx_queue);

	// BPID
	queue_push_u8(&tx_queue, BPID_BeaconP);

	// protocol data
	// change to MSB
	queue_push_u8(&tx_queue, virtual_id[1]);
	queue_push_u8(&tx_queue, virtual_id[0]);

	nwl_build_background_frame(spectrum_id, tx_eirp, subnet);
}
*/

/** \copydoc nwl_build_network_protocol_data */

void nwl_build_network_protocol_data(uint8_t control, nwl_security* security, nwl_full_access_template* source_access, uint8_t* target_address, uint8_t target_address_lenght, uint8_t subnet, uint8_t spectrum_id[2], int8_t tx_eirp)
{
	uint8_t access_tmpl_length = 0;

	dll_tx_cfg_t dll_params;
	dll_params.eirp = tx_eirp;
	memcpy(dll_params.spectrum_id, spectrum_id, 2);
	dll_params.subnet = subnet;
	dll_params.frame_type = FrameTypeForegroundFrame;

	if ((security != NULL) || (control & NWL_CONTRL_NLS))
	{
		#ifdef LOG_NWL_ENABLED
		log_print_stack_string(LOG_NWL, "NWL: security not implemented");
		#endif
	}

	if (control & NWL_CONTRL_CFG(0x0F))
	{
		#ifdef LOG_NWL_ENABLED
		log_print_stack_string(LOG_NWL, "NWL: Routing not implemented");
		#endif
	}

	switch(control & NWL_CONTRL_SRC_FULL)
	{
		case NWL_CONTRL_SRC_VID:
			access_tmpl_length = 2;
			break;
		case NWL_CONTRL_SRC_UID:
			access_tmpl_length = 8;
			break;
		case NWL_CONTRL_SRC_FULL:
		{
			nwl_full_access_template* access = (nwl_full_access_template*) source_access;
			if (access->control & NWL_ACCESS_TEMPL_CTRL_VID)
				access_tmpl_length = 4;
			else
				access_tmpl_length = 10;

			access_tmpl_length += (access->control & 0x0F);
			break;
		}
	}

	queue_create_header_space(&tx_queue, 1 + access_tmpl_length);

	tx_queue.front[0] = control;

	switch(control & NWL_CONTRL_SRC_FULL)
	{
		case NWL_CONTRL_SRC_VID:
			memcpy(&tx_queue.front[1], virtual_id, access_tmpl_length);
			break;
		case NWL_CONTRL_SRC_UID:
			memcpy(&tx_queue.front[1], device_id, access_tmpl_length);
			break;
		case NWL_CONTRL_SRC_FULL:
		{
			memcpy(&tx_queue.front[1], source_access, access_tmpl_length);
			break;
		}
	}


	dll_create_frame(target_address, target_address_lenght, &dll_params);
}

//
//void nwl_build_datastream_protocol_data(uint8_t* data, uint8_t length, nwl_security* security, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp, uint8_t dialog_id)
//{
//	uint8_t offset = 0;
//
//	dll_tx_cfg_t dll_params;
//	dll_params.eirp = tx_eirp;
//	dll_params.spectrum_id = spectrum_id;
//	dll_params.subnet = subnet;
//	dll_params.frame_type = FrameTypeForegroundFrame;
//
//
//	memcpy(&dll_data[offset], data, length);
//
//	uint8_t dll_data_length = offset + length;
//
//	//TODO: assert dll_data_length < 255-7
//	dll_create_frame(dll_data, dll_data_length, NULL, 0, &dll_params);
//}


void nwl_rx_start(uint8_t subnet, uint8_t spectrum_id[2], Protocol_Type type)
{


	scan_cfg.spectrum_id[0] = spectrum_id[0];
	scan_cfg.spectrum_id[1] = spectrum_id[1];
	scan_cfg.time_next_scan = 0;
	scan_cfg.timeout_scan_detect = 0;


	if (type != ProtocolTypeNetworkProtocol)
		scan_cfg.scan_type = FrameTypeBackgroundFrame;
	else
		scan_cfg.scan_type = FrameTypeForegroundFrame;


	scan_series_cfg.length = 1;
	scan_series_cfg.values = &scan_cfg;

	dll_channel_scan_series(&scan_series_cfg);
}

void nwl_rx_stop()
{
	dll_stop_channel_scan();
}

/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 *     	maarten.weyn@uantwerpen.be
 *
 */


#include "nwl.h"
#include "../framework/log.h"
#include "../hal/system.h"


static nwl_rx_callback_t nwl_rx_callback;
static nwl_tx_callback_t nwl_tx_callback;

static void dll_tx_callback(Dll_Tx_Result status)
{
	nwl_tx_callback(status);
}

static void dll_rx_callback(dll_rx_res_t* result)
{
	nwl_rx_res_t res;
	res.frame_type = result->frame_type;

	if (res.frame_type == FrameTypeBackgroundFrame)
	{
		nwl_background_frame_t bf;
		bf.tx_eirp = 0;
		bf.subnet = ((dll_background_frame_t*) result->frame)->subnet;
		bf.bpid = ((dll_background_frame_t*) result->frame)->payload[0];
		memcpy((void*) bf.protocol_data,(void*) &(((dll_background_frame_t*) result->frame)->payload[1]), 2);

		res.frame = &bf;
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

void nwl_build_background_frame(nwl_background_frame_t* data, uint8_t spectrum_id)
{
	dll_create_background_frame(&(data->bpid), data->subnet, spectrum_id, data->tx_eirp);
}

void nwl_build_advertising_protocol_data(uint8_t channel_id, uint16_t eta, int8_t tx_eirp, uint8_t subnet)
{
	nwl_background_frame_t frame;
	frame.tx_eirp = tx_eirp;
	frame.subnet = subnet;
	frame.bpid = BPID_AdvP;
	// change to MSB
	frame.protocol_data[0] = eta >> 8;
	frame.protocol_data[1] = eta & 0XFF;

	nwl_build_background_frame(&frame, channel_id);
}

void nwl_build_network_protocol_data(uint8_t* data, uint8_t length, nwl_security* security, nwl_routing_header* routing, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp, uint8_t dialog_id)
{
	uint8_t dll_data[248];
	uint8_t offset = 0;

	dll_ff_tx_cfg_t dll_params;
	dll_params.eirp = tx_eirp;
	dll_params.spectrum_id = spectrum_id;
	dll_params.subnet = subnet;

	//TODO: get from params
	dll_params.listen = false;
	dll_params.security = NULL;

	dll_foreground_frame_adressing adressing;
	adressing.dialog_id = dialog_id;
	adressing.addressing_option = ADDR_CTL_BROADCAST; //TODO: enable other
	adressing.virtual_id = false;
	adressing.source_id = device_id;
	dll_params.addressing = &adressing;


	if (security != NULL)
	{
		#ifdef LOG_NWL_ENABLED
		log_print_stack_string(LOG_NWL, "NWL: security not implemented");
		#endif

		//dll_params.nwl_security = true;
		dll_params.nwl_security = false;
	} else {
		dll_params.nwl_security = false;
	}

	if (routing != NULL)
	{
		#ifdef LOG_NWL_ENABLED
		log_print_stack_string(LOG_NWL, "NWL: routing not implemented");
		#endif
	}

	memcpy(&dll_data[offset], data, length);

	uint8_t dll_data_length = offset + length;

	//TODO: assert dll_data_length < 255-7
	dll_create_foreground_frame(dll_data, dll_data_length, &dll_params);
}


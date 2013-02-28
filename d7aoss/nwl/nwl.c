/*
 * nwl.c
 *
 *  Created on: 27-feb.-2013
 *      Author: Maarten Weyn
 */

#include "nwl.h"


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
		bf.length = 6;
		bf.tx_eirp = 0;
		bf.subnet = ((dll_background_frame_t*) result->frame)->subnet;
		bf.bpid = ((dll_background_frame_t*) result->frame)->payload[0];
		memcpy((void*) bf.protocol_data,(void*) &(((dll_background_frame_t*) result->frame)->payload[1]), 3);

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

void nwl_tx_background_frame(nwl_background_frame_t* data, u8 spectrum_id)
{
	dll_tx_background_frame(&(data->bpid), data->subnet, spectrum_id, data->tx_eirp);
}

void nwl_tx_advertising_protocol_data(u8 channel_id, u16 eta, u8 tx_eirp, u8 subnet, u8 spectrum_id)
{
	nwl_background_frame_t frame;
	frame.length = 6;
	frame.tx_eirp = tx_eirp;
	frame.subnet = subnet;
	frame.bpid = BPID_AdvP;
	AdvP_Data *data = (AdvP_Data*) frame.protocol_data;
	data->channel_id = channel_id;
	memcpy((void*) data->eta, (void*) &eta, 2);


	nwl_tx_background_frame(&frame, spectrum_id);
}

void nwl_tx_reservation_protocol_data(u8 res_type, u16 res_duration, u8 tx_eirp, u8 subnet, u8 spectrum_id)

{
	nwl_background_frame_t frame;
	frame.length = 6;
	frame.tx_eirp = tx_eirp;
	frame.subnet = subnet;
	frame.bpid = BPID_AdvP;
	ResP_Data *data = (ResP_Data*) frame.protocol_data;
	data->res_type = res_type;
	memcpy((void*) data->res_duration, (void*) &res_duration, 2);


	nwl_tx_background_frame(&frame, spectrum_id);
}


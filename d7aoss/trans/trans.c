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
 *     maarten.weyn@uantwerpen.be
 *     Dragan Subotic
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
static uint8_t current__spectrum_id = 0;
static uint16_t init_t_ca = 400;
static uint16_t last_ca = 0;

static int temp_time= 0;
static uint8_t dialogid = 0;

static trans_tx_callback_t trans_tx_callback;
static trans_rx_datastream_callback_t trans_rx_datastream_callback;

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
				trans_rigd_ccp(current__spectrum_id, false, true);
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
		assert("not implemented yet");
	}
	else if (result->protocol_type == ProtocolTypeDatastreamProtocol)
	{
		Trans_Rx_Datastream_Result datastream_result;
		nwl_ff_D7ANP_t* data = (nwl_ff_D7ANP_t*) result->data;

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

void trans_set_initial_t_ca(uint16_t t_ca)
{
	init_t_ca = t_ca;
}

static void final_rigd(void* arg){
	 dll_csma(true);
}

static void t_ca_timeout_rigd(void* arg){
	trans_rigd_ccp(current__spectrum_id, false, false);
}

void trans_tx_foreground_frame(uint8_t* data, uint8_t length, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp){
	nwl_build_network_protocol_data(data, length, NULL, NULL, subnet, spectrum_id, tx_eirp, dialogid++);
	trans_rigd_ccp(spectrum_id, true, false);
}

void trans_tx_datastream(uint8_t* data, uint8_t length, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp) {
	nwl_build_datastream_protocol_data(data, length, NULL, subnet, spectrum_id, tx_eirp, dialogid++);
	trans_rigd_ccp(spectrum_id, true, false);
}


//void trans_tx_background_frame(uint8_t* data, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp){
//	nwl_build_background_frame(data, subnet, spectrum_id, tx_eirp);
//}

// transport layer, Random Increase Geometric decaying slot back-off , Congestion Control Process
void trans_rigd_ccp(uint8_t spectrum_id, bool init_status, bool wait_for_t_ca_timeout){
	timer_event event;

	if(init_status){//initialization of the parameters, only for new packets
		//TODO: Dragan: fix overflow
		current__t_ca = init_t_ca;
		current__t_g = 5;
		current__spectrum_id = spectrum_id;
	}
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
	current__t_ca = current__t_ca/2;
	if(current__t_ca > current__t_g){
		float n_time = rand();
		n_time = (n_time / 32767) * current__t_ca; // Random Time before the CCA will be executed
		temp_time = (int)n_time;

		#ifdef LOG_TRANS_ENABLED
		log_print_stack_string(LOG_TRANS, "RIGD: Wait Random Time: %d", temp_time);
		#endif

		event.next_event = temp_time; // Wait random time (0 - new__t_ca)
		event.f = &final_rigd;
		last_ca = timer_get_counter_value();
		timer_add_event(&event);
	}
	else{
		#ifdef LOG_TRANS_ENABLED
		log_print_stack_string(LOG_TRANS, "RIGD: Failed");
		#endif
		trans_tx_callback(TransTCAFail);
		return;
	}
}


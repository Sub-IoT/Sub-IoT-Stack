/*
 * trans.c
 *
 *  Created on: 21-feb.-2013
 *      Author: Dragan Subotic
 *      		Maarten Weyn
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

static int temp_time= 0;
static uint8_t dialogid = 0;

static trans_tx_callback_t trans_tx_callback;

//Flow Control Process
static void control_tx_callback(Dll_Tx_Result Result)
{
	switch(Result){
			case DLLTxResultOK:
				#ifdef LOG_TRANS_ENABLED
					log_print_string("Trans: Packet is sent");
				#endif
				trans_tx_callback(TransPacketSent);
				break;
			case DLLTxResultCCAOK:
				dll_tx_frame();
				break;
			case DLLTxResultCCA1Fail:
			case DLLTxResultCCA2Fail:
				trans_rigd_ccp(current__spectrum_id, false);
				#ifdef LOG_TRANS_ENABLED
					log_print_string("Trans: CCA fail");
				#endif
				break;
			case DLLTxResultFail:
				trans_tx_callback(TransPacketFail);
				#ifdef LOG_TRANS_ENABLED
					log_print_string("Trans: Fail to sent");
				#endif
				break;
		}
	return;
}

void trans_init(){
	nwl_init();
	nwl_set_tx_callback(&control_tx_callback);
}

void trans_set_tx_callback(trans_tx_callback_t cb)
{
	trans_tx_callback = cb;
}

static void final_rigd(void* arg){
	 dll_csma(true);
}

void trans_tx_foreground_frame(uint8_t* data, uint8_t length, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp){
	nwl_build_network_protocol_data(data, length, NULL, NULL, subnet, spectrum_id, tx_eirp, dialogid++);
	trans_rigd_ccp(spectrum_id, true);
}

//void trans_tx_background_frame(uint8_t* data, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp){
//	nwl_build_background_frame(data, subnet, spectrum_id, tx_eirp);
//}

// transport layer, Random Increase Geometric decaying slot back-off , Congestion Control Process
void trans_rigd_ccp(uint8_t spectrum_id, bool init_status){
	if(init_status){//initialization of the parameters, only for new packets
		//TODO: Dragan: fix overflow
		current__t_ca = 400;
		current__t_g = 5;
		current__spectrum_id = spectrum_id;
	}
	current__t_ca = current__t_ca/2;
	if(current__t_ca > current__t_g){
		float n_time = rand();
		n_time = (n_time / 32767) * current__t_ca; // Random Time before the CCA will be executed
		temp_time = (int)n_time;
		timer_event event;
		event.next_event = temp_time; // Wait random time (0 - new__t_ca)
		event.f = &final_rigd;
		timer_add_event(&event);
	}
	else{
		trans_tx_callback(TransTCAFail);
		return;
	}
}


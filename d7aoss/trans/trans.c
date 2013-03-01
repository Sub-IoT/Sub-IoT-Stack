/*
 * trans.c
 *
 *  Created on: 21-feb.-2013
 *      Author: Dragan Subotic
 */


#include "trans.h"
#include "../timer.h"
#include "../log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static u8 current__t_ca = 0;
static u8 current__t_g = 0;
static u8 current__spectrum_id = 0;

static int temp_time= 0;

static trans_tx_callback_t trans_tx_callback;

//Flow Control Process
static void control_tx_callback(Dll_Tx_Result Result)
{
	switch(Result){
			case DLLTxResultOK:
				trans_tx_callback(TransPacketSent);
				#ifdef LOG_TRANS_ENABLED
					log_print_string("Packet is sent");
				#endif
				break;
			case DLLTxResultCCAFail:
				trans_rigd_ccp(current__spectrum_id, false);
				#ifdef LOG_TRANS_ENABLED
					log_print_string("CCA fail");
				#endif
				break;
			case DLLTxResultFail:
				trans_tx_callback(TransPacketFail);
				#ifdef LOG_TRANS_ENABLED
					log_print_string("Fail to send");
				#endif
				break;
		}
	return;
}

void trans_init(){
	dll_init();
	dll_set_tx_callback(&control_tx_callback);
	timer_init();
}

void trans_set_tx_callback(trans_tx_callback_t cb)
{
	trans_tx_callback = cb;
}

static void final_rigd(void* arg){
	 dll_csma();
}

void trans_tx_foreground_frame(u8* data, u8 length, u8 spectrum_id, s8 tx_eirp){
	dll_tx_foreground_frame(data, length, spectrum_id, tx_eirp);
	trans_rigd_ccp(spectrum_id, true);
}

// transport layer, Random Increase Geometric decaying slot back-off , Congestion Control Process
void trans_rigd_ccp(u8 spectrum_id, bool init_status){
	if(init_status){//initialization of the parameters, only for new packets
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


/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#include "stub_ral.h"
#include "../../log.h"

void stub_ral_init()
{
	log_print_string("stub_ral_init()");
}

void stub_ral_tx(ral_tx_cfg_t* cfg)
{
	log_print_string("stub_ral_send()");
}

void stub_ral_set_rx_callback(ral_rx_callback_t cb)
{
	log_print_string("stub_ral_set_rx_callback()");
}

void stub_ral_set_tx_callback(ral_tx_callback_t cb)
{
	log_print_string("stub_ral_set_tx_callback()");
}

void stub_ral_rx_start(ral_rx_cfg_t* cfg)
{
	log_print_string("stub_ral_rx_start()");
}

void stub_ral_rx_stop()
{
	log_print_string("stub_ral_rx_stop()");
}

bool stub_ral_is_rx_in_progress()
{
	log_print_string("stub_ral_is_rx_in_progress()");
	return false;
}

// An empty stub implementation of the RAL interface
const struct ral_interface stub_ral = {
	stub_ral_init,
	stub_ral_tx,
	stub_ral_set_tx_callback,
	stub_ral_set_rx_callback,
	stub_ral_rx_start,
	stub_ral_rx_stop,
	stub_ral_is_rx_in_progress,
};

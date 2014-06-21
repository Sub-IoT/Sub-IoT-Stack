/*! \file dll.c
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
 * \author glenn.ergeerts@uantwerpen.be
 */

#include "dll.h"
#include "../framework/timer.h"
#include "../hal/system.h"
#include "../hal/crc.h"
#include "../framework/log.h"
#include <string.h>

static dll_rx_callback_t dll_rx_callback;
static dll_tx_callback_t dll_tx_callback;
static dll_tx_callback_t dll_tx_callback_temp;
static dll_rx_res_t dll_res;

static dll_channel_scan_series_t* current_css;
static uint8_t current_scan_id = 0;

phy_tx_cfg_t *current_phy_cfg;


uint8_t timeout_listen; // TL
uint8_t frame_data[100]; // TODO: get rid of fixed buffer
uint16_t timestamp;
uint8_t timeout_ca; 	// T_ca

Dll_State_Enum dll_state;


//Scan parameters
int16_t scan_minimum_energy = -140; // E_sm
uint16_t background_scan_detection_timeout;
uint16_t foreground_scan_detection_timeout;
uint8_t spectrum_id = 0;


phy_tx_cfg_t frame_tx_cfg = {
            0x10, 	// spectrum ID
			1, 		// Sync word class
			0,		// Transmission power level in dBm ranged [-39, +10]
			0,		// Packet length
            frame_data	//Packet data
};

/*! \brief Check the frame subnet with the device subnet
 *
 *
 *  \param device_subnet The subnet of the receiving device (from filesystem)
 *  \param frame_subet The subnet specified in the received message
 */
static bool check_subnet(uint8_t device_subnet, uint8_t frame_subnet)
{
	// FFS = 0xF?
	if (frame_subnet & 0xF0 != 0xF0)
	{
		// No -> FSS = DSS?
		if (frame_subnet & 0xF0 != device_subnet & 0xF0)
			return 0;
	}

	uint8_t fsm = frame_subnet & 0x0F;
	uint8_t dsi = device_subnet & 0x0F;

	// FSM & DSI != 0?
	if ((fsm & dsi) == 0)
			return 0;

	return 1;
}

/*! \brief Starts next scan according to scan series
 *
 */
static void scan_next()
{
	dll_channel_scan_series(current_css);
}

/*! \brief Handles RX scan timeout event from phy
 *
 */
static void scan_timeout()
{
	if (dll_state == DllStateNone)
		return;

	#ifdef LOG_DLL_ENABLED
		log_print_stack_string(LOG_DLL, "DLL scan time-out");
	#endif
	phy_idle();

	if (current_css == NULL)
	{
		if(dll_rx_callback != NULL)
			dll_rx_callback(NULL);
		return;
	}

	//Channel scan series
	timer_event event;
	event.next_event = current_css->values[current_scan_id].time_next_scan;
	event.f = &scan_next;

	current_scan_id = current_scan_id < current_css->length - 1 ? current_scan_id + 1 : 0;

	timer_add_event(&event);
}

/*! \brief Forwards phy tx callback
 *
 */
static void tx_callback()
{
	#ifdef LOG_DLL_ENABLED
		log_print_stack_string(LOG_DLL, "DLL TX OK");
	#endif

	if (dll_tx_callback != NULL)
		dll_tx_callback(DLLTxResultOK);
}

/*! \brief RX Callback function of the DLL which parses the frame received from the Phy
 *
 *  RX Callback function of the DLL which parses the frame received from the Physical layer
 *
 *  \param res struct which containts the data received from the physical layer
 */
static void rx_callback(phy_rx_data_t* res)
{
	//log_packet(res->data);
	if (res == NULL)
	{
		scan_timeout();
		return;
	}

	// Data Link Filtering
	// 1. CRC Validation
	// 2. Subnet filtering
	// 3. Link quality assesment

	uint16_t crc = crc_calculate(res->data, res->length - 2);
	if (memcmp((uint8_t*) &(res->data[res->length - 2]), (uint8_t*) &crc, 2) != 0)
	{
		#ifdef LOG_DLL_ENABLED
			log_print_stack_string(LOG_DLL, "DLL CRC ERROR");
		#endif
		scan_next(); // how to reïnitiate scan on CRC Error, PHY should stay in RX
		return;
	}
	if (!check_subnet(0xFF, res->data[1])) // TODO: get device_subnet from datastore
	{
		#ifdef LOG_DLL_ENABLED
			log_print_stack_string(LOG_DLL, "DLL Subnet mismatch");
		#endif
			scan_next(); // how to reïnitiate scan on subnet mismatch, PHY should stay in RX

		return;
	}

	//Todo: implement link quality assement


	//Todo: work further on rx callback

	// parse packet
	dll_res.rssi = res->rssi;
	dll_res.lqi = res->lqi;
	dll_res.spectrum_id = current_css->values[current_scan_id].spectrum_id;

	dll_frame_t* frame = (dll_frame_t*)frame_data;
	frame->length = res->data[0];
	frame->subnet = res->data[1];
	frame->control = res->data[2];
	if (frame->control & 0x80) // target address present
	{
		frame->target_address = &res->data[3];
		if (frame->control & 0x40) // VID
		{
			frame->payload = &res->data[6];
			frame->payload_length = frame->length - 8;
		}
		else // UID
		{
			frame->payload = &res->data[12];
			frame->payload_length = frame->length - 14;
		}
	} else {
		frame->target_address = NULL;
		frame->payload_length = frame->length - 6;
	}

	dll_res.frame = frame;

	if (dll_state == DllStateScanBackgroundFrame)
	{
		dll_res.frame_type = FrameTypeBackgroundFrame;
	}
	else
	{
		dll_res.frame_type = FrameTypeForegroundFrame;
	}

	#ifdef LOG_DLL_ENABLED
		log_dll_rx_res(&dll_res);
	#endif

	dll_rx_callback(&dll_res);

	if (current_css == NULL)
	{
		#ifdef LOG_DLL_ENABLED
			log_print_stack_string(LOG_DLL, ("DLL no series so stop listening"));
		#endif
		return;
	}

	// in current spec reset channel scan
	#ifdef LOG_DLL_ENABLED
		log_print_stack_string(LOG_DLL, ("DLL restart channel scan series"));
	#endif

	current_scan_id = 0;
	scan_next();
}

void dll_init()
{
	timer_init();

	phy_init();
	phy_set_rx_callback(rx_callback);
	phy_set_tx_callback(tx_callback);

	dll_state = DllStateNone;
}

void dll_set_tx_callback(dll_tx_callback_t cb)
{
	dll_tx_callback = cb;
}
void dll_set_rx_callback(dll_rx_callback_t cb)
{
	dll_rx_callback = cb;
}

void dll_set_scan_minimum_energy(int16_t e_sm)
{
	scan_minimum_energy = e_sm; // E_sm
}

void dll_set_background_scan_detection_timeout(uint16_t t_bsd)
{
	background_scan_detection_timeout = t_bsd;
}

void dll_set_foreground_scan_detection_timeout(uint16_t t_fsd)
{
	foreground_scan_detection_timeout = t_fsd;
}

void dll_set_scan_spectrum_id(uint8_t spect_id)
{
	spectrum_id = spect_id;
}

void dll_stop_channel_scan()
{
	// TODO remove scan_timeout events from queue?
	current_css = NULL;
	dll_state = DllStateNone;
	phy_idle();
}

uint8_t dll_background_scan()
{
	#ifdef LOG_DLL_ENABLED
		log_print_stack_string(LOG_DLL, "DLL Starting background scan");
	#endif

	dll_state = DllStateScanBackgroundFrame;

	//check for signal detected above E_sm
	// TODO: is this the best method?
	int rss = phy_get_rssi(spectrum_id, 0);
	if (rss <= scan_minimum_energy)
	{
		#ifdef LOG_DLL_ENABLED
			log_print_stack_string(LOG_DLL, "DLL No signal detected: %d", rss);
		#endif
		return 0;
	}

	#ifdef LOG_DLL_ENABLED
		log_print_stack_string(LOG_DLL, "DLL Background Scan signal detected: %d", rss);
	#endif

	phy_rx_cfg_t rx_cfg;
	rx_cfg.length = 0;
	rx_cfg.timeout = background_scan_detection_timeout; // timeout
	rx_cfg.spectrum_id = spectrum_id; // spectrum ID
	rx_cfg.scan_minimum_energy = scan_minimum_energy;
	rx_cfg.sync_word_class = 0;

	current_css = NULL;

	#ifdef LOG_DLL_ENABLED
	bool phy_rx_result = phy_rx(&rx_cfg);
	if (!phy_rx_result)
	{
		log_print_stack_string(LOG_DLL, "DLL Starting channel scan FAILED");
	}
	#else
	phy_rx(&rx_cfg);
	#endif

	return 1;
}
void dll_foreground_scan()
{
	#ifdef LOG_DLL_ENABLED
		log_print_stack_string(LOG_DLL, "Starting foreground scan");
	#endif

	dll_state = DllStateScanForegroundFrame;

	phy_rx_cfg_t rx_cfg;
	rx_cfg.length = 0;
	rx_cfg.timeout = foreground_scan_detection_timeout; // timeout
	rx_cfg.spectrum_id = spectrum_id; // spectrum ID
	rx_cfg.scan_minimum_energy = scan_minimum_energy;
	rx_cfg.sync_word_class = 1;

	current_css = NULL;

	#ifdef LOG_DLL_ENABLED
	bool phy_rx_result = phy_rx(&rx_cfg);
	if (!phy_rx_result)
	{
		log_print_stack_string(LOG_DLL, "DLL Starting channel scan FAILED");
	}
	#else
	phy_rx(&rx_cfg);
	#endif
}

void dll_channel_scan_series(dll_channel_scan_series_t* css)
{
	#ifdef LOG_DLL_ENABLED
		log_print_stack_string(LOG_DLL, "DLL Starting channel scan series");
	#endif

	phy_rx_cfg_t rx_cfg;
	rx_cfg.length = 0;
	rx_cfg.timeout = css->values[current_scan_id].timeout_scan_detect; // timeout
	//rx_cfg.multiple = 0; // multiple TODO
	rx_cfg.spectrum_id = css->values[current_scan_id].spectrum_id; // spectrum ID TODO
	//rx_cfg.coding_scheme = 0; // coding scheme TODO
	rx_cfg.scan_minimum_energy = scan_minimum_energy;
	if (css->values[current_scan_id].scan_type == FrameTypeBackgroundFrame)
	{
		rx_cfg.sync_word_class = 0;
		dll_state = DllStateScanBackgroundFrame;
	} else {
		rx_cfg.sync_word_class = 1;
		dll_state = DllStateScanForegroundFrame;
	}

	current_css = css;


	#ifdef LOG_DLL_ENABLED
	bool phy_rx_result = phy_rx(&rx_cfg);
	if (!phy_rx_result)
	{
		log_print_stack_string(LOG_DLL, "DLL Starting channel scan FAILED");
	}
	#else
	phy_rx(&rx_cfg);
	#endif
}

static void dll_cca2()
{
	bool cca2 = phy_cca(current_phy_cfg->spectrum_id, current_phy_cfg->sync_word_class);;
	if (!cca2)
	{
		dll_tx_callback(DLLTxResultCCA2Fail);
		return;
	}

	dll_tx_callback(DLLTxResultCCAOK);
}

void dll_tx_frame()
{
	if (!phy_tx(current_phy_cfg))
	{
		dll_tx_callback(DLLTxResultFail);
	}
}

void dll_csma(bool enabled)
{
	if (!enabled)
	{
		dll_tx_callback(DLLTxResultCCAOK);
		return;
	}

	bool cca1 = phy_cca(current_phy_cfg->spectrum_id, current_phy_cfg->sync_word_class);

	if (!cca1)
	{
		dll_tx_callback(DLLTxResultCCA1Fail);
		return;
	}

	timer_event event;

	// TODO: calculate Tg only once
	// Calculate correct t_g

	uint8_t channel_bandwidth_index = (spectrum_id >> 4) & 0x07;
	uint8_t fec = (bool)spectrum_id >> 7;

	if (channel_bandwidth_index == 1)
		event.next_event = fec == 0 ? 5 : 10;
	else
		event.next_event = fec == 0 ? 2 : 3;

	event.f = &dll_cca2;

	if (!timer_add_event(&event))
	{
		dll_tx_callback(DLLTxResultFail);
		return;
	}
}


void dll_ca_callback(Dll_Tx_Result result)
{
	dll_tx_callback = dll_tx_callback_temp;

	if (result == DLLTxResultCCAOK)
	{
		dll_tx_callback(DLLTxResultCCAOK);
	} else
	{

        uint16_t new_time = timer_get_counter_value();
		uint16_t diff = (new_time - timestamp) >> 6;
		if (diff < (timeout_ca - 5))
		{
			timeout_ca-= diff;
			dll_ca(timeout_ca);
		} else {
			dll_tx_callback(DLLTxResultCAFail);
		}
	}
}

void dll_ca(uint8_t t_ca)
{
	dll_tx_callback_temp = dll_tx_callback;
	dll_tx_callback = dll_ca_callback;

	timeout_ca = t_ca;
    timestamp = timer_get_counter_value();
	dll_csma(true);
}

/** \copydoc dll_create_frame */
void dll_create_frame(uint8_t* data, uint8_t length, uint8_t* target_address, uint8_t address_length, dll_tx_cfg_t* params)
{
	//TODO: check if in idle state
	frame_tx_cfg.spectrum_id = params->spectrum_id; // TODO check valid (should be done in the upper layer of stack)
	frame_tx_cfg.eirp = params->eirp;
	frame_tx_cfg.sync_word_class = (params->frame_type == FrameTypeForegroundFrame) ? 1 : 0;
	frame_tx_cfg.length = length + 5 + address_length;
	current_phy_cfg = &frame_tx_cfg;

	frame_data[0] = frame_tx_cfg.length;	// Lenght
	frame_data[1] = params->subnet; 				// Subnet
	frame_data[2] = 0x3F & (params->eirp + 32);
	if (address_length == 2)
	{
		frame_data[2] |= 0xC0;
	} else if (address_length == 8)
	{
		frame_data[2] |= 0x80;
	}

	memcpy(&frame_data[3], target_address, address_length);
	memcpy(&frame_data[3 + address_length], data, length);

	uint16_t crc16 = crc_calculate(frame_data, frame_tx_cfg.length - 2);
	memcpy(&frame_data[frame_tx_cfg.length - 2], &crc16, 2);
}

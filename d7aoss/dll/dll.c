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


phy_tx_cfg_t foreground_frame_tx_cfg = {
            0x10, 	// spectrum ID
			1, 		// Sync word class
			0,		// Transmission power level in dBm ranged [-39, +10]
			0,		// Packet length
            frame_data	//Packet data
};

phy_tx_cfg_t background_frame_tx_cfg = {
		0x10, 	// spectrum ID
		0, 		// Sync word class
		0,		// Transmission power level in dBm ranged [-39, +10]
		6,
		frame_data	//Packet data
};



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
	uint8_t dsm = device_subnet & 0x0F;

	// FSM & DSM = DSM?
	if ((fsm & dsm) == dsm)
			return 1;

	return 0;
}

static void scan_next()
{
	dll_channel_scan_series(current_css);
}

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

static void tx_callback()
{
	#ifdef LOG_DLL_ENABLED
		log_print_stack_string(LOG_DLL, "DLL TX OK");
	#endif
	dll_tx_callback(DLLTxResultOK);
}

static void rx_callback(phy_rx_data_t* res)
{
	//log_packet(res->data);
	if (res == NULL)
	{
		scan_timeout();
		return;
	}

	// Data Link Filtering
	// Subnet Matching do not parse it yet
	if (dll_state == DllStateScanBackgroundFrame)
	{
		uint16_t crc = crc_calculate(res->data, 4);
		if (memcmp((uint8_t*) &(res->data[4]), (uint8_t*) &crc, 2) != 0)
		{
			#ifdef LOG_DLL_ENABLED
				log_print_stack_string(LOG_DLL, "DLL CRC ERROR");
			#endif
			scan_next(); // how to reïnitiate scan on CRC Error, PHY should stay in RX
			return;
		}

		if (!check_subnet(0xFF, res->data[0])) // TODO: get device_subnet from datastore
		{
			#ifdef LOG_DLL_ENABLED
				log_print_stack_string(LOG_DLL, "DLL Subnet mismatch");
			#endif
			scan_next(); // how to reïnitiate scan on subnet mismatch, PHY should stay in RX
			return;
		}
	} else if (dll_state == DllStateScanForegroundFrame)
	{
		uint16_t crc = crc_calculate(res->data, res->length - 2);
		if (memcmp((uint8_t*) &(res->data[res->length - 2]), (uint8_t*) &crc, 2) != 0)
		{
			#ifdef LOG_DLL_ENABLED
				log_print_stack_string(LOG_DLL, "DLL CRC ERROR");
			#endif
			scan_next(); // how to reïnitiate scan on CRC Error, PHY should stay in RX
			return;
		}
		if (!check_subnet(0xFF, res->data[2])) // TODO: get device_subnet from datastore
		{
			#ifdef LOG_DLL_ENABLED
				log_print_stack_string(LOG_DLL, "DLL Subnet mismatch");
			#endif
				scan_next(); // how to reïnitiate scan on subnet mismatch, PHY should stay in RX

			return;
		}
	} else
	{
		#ifdef LOG_DLL_ENABLED
			log_print_stack_string(LOG_DLL, "DLL You fool, you can't be here");
		#endif
	}

	// Optional Link Quality Assessment

	// parse packet
	dll_res.rssi = res->rssi;
	dll_res.lqi = res->lqi;
	dll_res.spectrum_id = current_css->values[current_scan_id].spectrum_id;

	if (dll_state == DllStateScanBackgroundFrame)
	{
		dll_background_frame_t* frame = (dll_background_frame_t*)frame_data;
		frame->subnet = res->data[0];
		memcpy(frame->payload, res->data+1, 4);

		dll_res.frame_type = FrameTypeBackgroundFrame;
		dll_res.frame = frame;
	}
	else
	{
		dll_foreground_frame_t* frame = (dll_foreground_frame_t*)frame_data;
		frame->length = res->data[0];

		frame->frame_header.tx_eirp = res->data[1] * 0.5 - 40;
		frame->frame_header.subnet = res->data[2];
		frame->frame_header.frame_ctl = res->data[3];

		uint8_t* data_pointer = res->data + 4;

		if (frame->frame_header.frame_ctl & FRAME_CTL_LISTEN) // Listen
			timeout_listen = 10;
		else
			timeout_listen = 0;

		if (frame->frame_header.frame_ctl & FRAME_CTL_DLLS) // DLLS present
		{
			// TODO parse DLLS Header
			frame->dlls_header = NULL;
		} else {
			frame->dlls_header = NULL;
		}

		if (frame->frame_header.frame_ctl & 0x20) // Enable Addressing
		{
			// Address Control Header
			dll_foreground_frame_address_ctl_t address_ctl;// = (dll_foreground_frame_address_ctl_t*) data_pointer;
			frame->address_ctl = &address_ctl;
			frame->address_ctl->dialogId = *data_pointer;
			data_pointer++;
			frame->address_ctl->flags = *data_pointer;
			data_pointer++;
			//data_pointer += sizeof(uint8_t*);

			uint8_t addressing = (frame->address_ctl->flags & 0xC0) >> 6;
			uint8_t vid = (frame->address_ctl->flags & 0x20) >> 5;
			uint8_t nls = (frame->address_ctl->flags & 0x10) >> 4;
			// TODO parse Source ID Header

			frame->address_ctl->source_id = data_pointer;
			if (vid)
			{
				data_pointer += 2;
			}
			else
			{
				data_pointer += 8;
			}

			if (addressing == 0 && nls == 0)
			{
				uint8_t id_target[8];
				if (vid)
				{
					memcpy(data_pointer, &id_target, 2);
					data_pointer += 2;
				}
				else
				{
					memcpy(data_pointer, &id_target, 8);
					data_pointer += 8;
				}
				frame->address_ctl->target_id = (uint8_t*) &id_target;
			} else {
				frame->address_ctl->target_id = NULL;
			}
		} else {
			frame->address_ctl = NULL;
			frame->address_ctl->source_id = NULL;
		}

		if (frame->frame_header.frame_ctl & 0x10) // Frame continuity
		{
			// TODO handle more than 1 frame
		}

		if (frame->frame_header.frame_ctl & 0x04) // Note Mode 2
		{
			// Not supported
		}

		// Frame Type
		dll_res.frame_type = (Frame_Type) (frame->frame_header.frame_ctl & 0x03);

		if (dll_res.frame_type == FrameTypeForegroundFrameStreamFrame)
		{
			frame->payload_length = frame->length - (data_pointer - res->data) - 2;
			frame->payload = data_pointer;
		} else {

			// TODO: should be done in upper layer
			//data_pointer++; // TODO what is this?
			//data_pointer++; //isfid
			//data_pointer++; //isfoffset

			frame->payload_length = frame->length - (data_pointer - res->data) - 2;
			//data_pointer++;
			frame->payload = data_pointer;
		}

		dll_res.frame = frame;
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



void dll_create_foreground_frame(uint8_t* data, uint8_t length, dll_ff_tx_cfg_t* params)
{
	//TODO: check if in idle state
	foreground_frame_tx_cfg.spectrum_id = params->spectrum_id; // TODO check valid (should be done in the upper layer of stack)
	foreground_frame_tx_cfg.eirp = params->eirp;
	foreground_frame_tx_cfg.sync_word_class = 1;

	dll_foreground_frame_t* frame = (dll_foreground_frame_t*) frame_data;
	frame->frame_header.tx_eirp = (params->eirp + 40) * 2; // (-40 + 0.5n) dBm
	frame->frame_header.subnet = params->subnet;
	frame->frame_header.frame_ctl = 0;

	if (params->listen) frame->frame_header.frame_ctl |= FRAME_CTL_LISTEN;

	if (params->security != NULL)
	{
		#ifdef LOG_DLL_ENABLED
			log_print_stack_string(LOG_DLL, "DLL: security not implemented");
		#endif
		//frame->frame_header.frame_ctl |= FRAME_CTL_DLLS;
	}

	uint8_t* pointer = frame_data + 1 + sizeof(dll_foreground_frame_header_t);

	if (params->addressing != NULL)
	{
		frame->frame_header.frame_ctl |= FRAME_CTL_EN_ADDR;

		dll_foreground_frame_address_ctl_t address_ctl;
		address_ctl.dialogId = params->addressing->dialog_id;
		address_ctl.flags = params->addressing->addressing_option;
		if (params->addressing->virtual_id) address_ctl.flags |= ADDR_CTL_VID;
		if (params->nwl_security) address_ctl.flags |= ADDR_CTL_NLS;

		memcpy(pointer, &address_ctl, 2);
		pointer += 2;

		uint8_t address_length = params->addressing->virtual_id ? 2 : 8;
		memcpy(pointer, params->addressing->source_id, address_length);
		pointer += address_length;

		if (params->addressing->addressing_option == ADDR_CTL_UNICAST && !params->nwl_security)
		{
			memcpy(pointer, params->addressing->target_id, address_length);
			pointer += address_length;
		}
	}

	if (params->frame_continuity) frame->frame_header.frame_ctl |= FRAME_CTL_FR_CONT;

	frame->frame_header.frame_ctl |= params->frame_type;

	memcpy(pointer, data, length); // TODO fixed size for now
	pointer += length;

	frame->length = (pointer - frame_data) + 2;  // length includes CRC

	uint16_t crc16 = crc_calculate(frame_data, frame->length - 2);
	memcpy(pointer, &crc16, 2);

	foreground_frame_tx_cfg.length = frame->length;

	current_phy_cfg = &foreground_frame_tx_cfg;
}

void dll_create_background_frame(uint8_t* data, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp)
{
	background_frame_tx_cfg.spectrum_id = spectrum_id;
	background_frame_tx_cfg.eirp = tx_eirp;
	background_frame_tx_cfg.length = 6;

	dll_background_frame_t* frame = (dll_background_frame_t*) frame_data;
	frame->subnet = subnet;
	memcpy(frame->payload, data, 3);

	uint8_t* pointer = frame_data + 4;

	uint16_t crc16 = crc_calculate(frame_data, 4);
	memcpy(pointer, &crc16, 2);

	current_phy_cfg = &background_frame_tx_cfg;
}

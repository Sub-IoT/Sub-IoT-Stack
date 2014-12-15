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
#include "../dae/fs.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static dll_rx_callback_t dll_rx_callback;
static dll_tx_callback_t dll_tx_callback;
static dll_tx_callback_t dll_tx_callback_temp;
static dll_rx_res_t dll_res;

static dll_channel_scan_series_t* current_css;
static uint8_t current_scan_id = 0;

phy_tx_cfg_t *current_phy_cfg;


uint8_t timeout_listen; // TL
//uint8_t frame_data[100]; // TODO: get rid of fixed buffer
uint16_t timestamp;
uint8_t timeout_ca; 	// T_ca

Dll_State_Enum dll_state;

static uint16_t current__t_ca = 0;
static const uint8_t current__t_g = 5;
static uint16_t init_t_ca = 400;
static uint16_t last_ca = 0;

static Dll_CSMA_CA_Type csma_ca_type = DllCsmaCaAind;


//Scan parameters
int16_t scan_minimum_energy = -140; // E_sm
uint16_t background_scan_detection_timeout;
uint16_t foreground_scan_detection_timeout;
uint8_t spectrum_id[2] = {0, 0};


// DLL Parameters
uint8_t* device_subnet;
uint8_t* destination_subnet;

phy_tx_cfg_t frame_tx_cfg = {
            {0x04, 0x00}, 	// spectrum ID
			1, 		// Sync word class
			0//,		// Transmission power level in dBm ranged [-39, +10]
			//0,		// Packet length
            //frame_data	//Packet data
};

dll_frame_t frame;

static void dll_aind_ccp_process();
static void t_ca_timeout_rigd();
static void final_rigd();

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
	// 4. target address?

	uint16_t crc = crc_calculate(res->data, res->length - 2);
	if ((res->data[res->length - 2] != (crc >> 8)) || (res->data[res->length - 1] != (crc & 0xFF)))
	{
		//#ifdef LOG_DLL_ENABLED
			log_print_stack_string(LOG_DLL, "DLL CRC ERROR 0x%x vs 0x%x%x", crc, res->data[res->length - 2], res->data[res->length - 1]);
		//#endif
		scan_next(); // how to re�nitiate scan on CRC Error, PHY should stay in RX
		return;
	}
	if (!check_subnet(*device_subnet, res->data[1])) // TODO: get device_subnet from datastore
	{
		#ifdef LOG_DLL_ENABLED
			log_print_stack_string(LOG_DLL, "DLL Subnet mismatch");
		#endif
			scan_next(); // how to re�nitiate scan on subnet mismatch, PHY should stay in RX

		return;
	}

	//Todo: implement link quality assement

	// parse packet
	dll_res.rssi = res->rssi;
	dll_res.lqi = res->lqi;
	memcpy(dll_res.spectrum_id, current_css->values[current_scan_id].spectrum_id, 2);
	// todo: take into account band / CS ...

	frame.length = res->data[0];
	frame.subnet = res->data[1];
	frame.control = res->data[2];
	if (frame.control & 0x80) // target address present
	{
		frame.target_address = &res->data[3];
		if (frame.control & 0x40) // VID
		{
			if (memcmp(frame.target_address, virtual_id, 2) == 0)
			{
				#ifdef LOG_DLL_ENABLED
				log_print_stack_string(LOG_DLL, "DLL this device is not the target");
				#endif
				scan_next(); // how to re�nitiate scan  PHY should stay in RX

				return;
			}

			frame.payload = &res->data[6];
			frame.payload_length = frame.length - 8;
		}
		else // UID
		{
			if (memcmp(frame.target_address, &device_id, 8) == 0)
			{
				#ifdef LOG_DLL_ENABLED
				log_print_stack_string(LOG_DLL, "DLL this device is not the target");
				#endif
				scan_next(); // how to re�nitiate scan  PHY should stay in RX

				return;
			}

			frame.payload = &res->data[12];
			frame.payload_length = frame.length - 14;
		}
	} else {
		frame.target_address = NULL;
		frame.payload = &res->data[3];
		frame.payload_length = frame.length - 5;
	}

	dll_res.frame = &frame;

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

	current_css = NULL;

	/*
	IS this correct?? after reception of packet -> stop channel scan.
	if (current_css == NULL)
	{
		#ifdef LOG_DLL_ENABLED
			log_print_stack_string(LOG_DLL, "DLL no series so stop listening");
		#endif
		return;
	}

	// in current spec reset channel scan
	#ifdef LOG_DLL_ENABLED
		log_print_stack_string(LOG_DLL, "DLL restart channel scan series");
	#endif

	current_scan_id = 0;
	scan_next();
	*/
}

void dll_init()
{
	timer_init();

	file_handler fh;
	fs_open(&fh, DA_FILE_DLL_CONFIGURATION, file_system_user_root, file_system_access_type_read);
	device_subnet = fs_get_data_pointer(&fh, 0);
	destination_subnet = fs_get_data_pointer(&fh, 1);
	virtual_id = fs_get_data_pointer(&fh, 2);
	fs_close(&fh);

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

void dll_set_scan_spectrum_id(uint8_t spect_id[2])
{
	memcpy(spectrum_id, spect_id, 2);
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
	memcpy(rx_cfg.spectrum_id, spectrum_id, 2); // spectrum ID
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
	memcpy(rx_cfg.spectrum_id, spectrum_id, 2); // spectrum ID
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
	memcpy(rx_cfg.spectrum_id, css->values[current_scan_id].spectrum_id, 2); // spectrum ID
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
		dll_initiate_csma_ca();
		//dll_tx_callback(DLLTxResultCCA2Fail);
		return;
	}

	dll_tx_frame();
	//dll_tx_callback(DLLTxResultCCAOK);
}

void dll_tx_frame()
{
	if (!phy_tx(current_phy_cfg))
	{
		dll_tx_callback(DLLTxResultFail);
	} else
	{
		dll_tx_callback(DLLTxInitiated);
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
		dll_initiate_csma_ca();//dll_tx_callback(DLLTxResultCCA1Fail);
		return;
	}

	timer_event event;

	// TODO: calculate Tg only once
	// Calculate correct t_g

	event.next_event = 5;

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
		if (diff < (uint16_t) (timeout_ca - 5))
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
void dll_create_frame(uint8_t* target_address, uint8_t address_length, dll_tx_cfg_t* params)
{
	//TODO: check if in idle state
	memcpy(frame_tx_cfg.spectrum_id, params->spectrum_id, 2); // TODO check valid (should be done in the upper layer of stack)
	frame_tx_cfg.eirp = params->eirp;
	frame_tx_cfg.sync_word_class = (params->frame_type == FrameTypeForegroundFrame) ? 1 : 0;
	//frame_tx_cfg.length = tx_queue.length + 5 + address_length;
	current_phy_cfg = &frame_tx_cfg;

	queue_create_header_space(&tx_queue, 3 + address_length);

	tx_queue.front[0] = tx_queue.length + 2;// + 5 + address_length;	// Lenght
	tx_queue.front[1] = params->subnet; 				// Subnet
	tx_queue.front[2] = 0x3F & (params->eirp + 32);

	if (address_length != 0)
	{
		if (address_length == 2)
		{
			tx_queue.front[2] |= 0xC0;
		} else if (address_length == 8)
		{
			tx_queue.front[2] |= 0x80;
		}

		if (address_length != 0)
			memcpy(&tx_queue.front[3], target_address, address_length);
	}

	uint16_t crc16 = crc_calculate(tx_queue.front, tx_queue.length);
	queue_push_u8(&tx_queue, crc16 >> 8);
	queue_push_u8(&tx_queue, crc16 & 0xFF);
}


void dll_set_initial_t_ca(uint16_t t_ca)
{
	init_t_ca = t_ca;
}


/*! \brief Sets the type of CSMA CA
 *
 *  Sets the type of CSMA CA, options are AIND, RAIND and RIGD
 *
 *  \todo implement RAIND
 *
 *  \param type The CSMA CA Algorithm to be used
 */
void dll_set_csma_ca(Dll_CSMA_CA_Type type)
{
	csma_ca_type = type;
}

static void dll_aind_ccp_process()
{
	uint16_t time_since_last_ca = timer_get_counter_value() - last_ca;
	if (current__t_ca < time_since_last_ca)
	{
		#ifdef LOG_TRANS_ENABLED
		log_print_stack_string(LOG_TRANS, "AIND: Failed");
		#endif
		dll_tx_callback(DLLTxResultCAFail);
		return;
	}

	current__t_ca -= time_since_last_ca;

	last_ca = timer_get_counter_value();
	dll_csma(true);
}

static void final_rigd() {
	 dll_csma(true);
}

static void t_ca_timeout_rigd() {
	dll_rigd_ccp(false);
}

void dll_initiate_csma_ca()
{
	current__t_ca = init_t_ca;

	switch (csma_ca_type)
	{
	case DllCsmaCaAind:
		dll_aind_ccp(true);
		break;
	case DllCsmaCaRaind:
		//break;
	case DllCsmaCaRigd:
		dll_rigd_ccp(false);
		break;
	}
}
//
//static void dll_process_csma_ca()
//{
//	switch (csma_ca_type)
//	{
//	case DllCsmaCaAind:
//		dll_aind_ccp(false);
//		break;
//	case DllCsmaCaRaind:
//		break;
//	case DllCsmaCaRigd:
//		dll_rigd_ccp(true);
//		break;
//	}
//}



/*! \brief Transport Layer CSMA-CA Congestion Control Process according to the Adaptive Increase No Division (AIND) algorithm
 *
 *  The Congentstion Control Process acccording to the AIND algorithm
 *  AIND CSMA-CA is a process where ad-hoc slotting takes place, the insertion happens at the beginning of the
 *	slot, and the slot duration is equal (approximately) to the duration of the transmission being queued.
 *
 *	\todo Calculate wait duration
 *
 *  \param spectrum_id The Spectrum ID used for the CCA
 *  \param init_status Flag to indicate if the process needs to be initiated.
 */
void dll_aind_ccp(bool init_status)
{
	timer_event event;

	// Initialisation of the parameters, only for new packets
	if (init_status) {
		last_ca = timer_get_counter_value();
		dll_aind_ccp_process();
	} else {
		// wait for transmission duration
		event.next_event = 5; // todo: calculate read transmission duration
		event.f = &dll_aind_ccp_process;
		timer_add_event(&event);
		return;
	}
}

/*! \brief Transport Layer CSMA-CA Congestion Control Process according to the Random Increase Geometric Division (RIGD) algorithm
 *
 *  The Congentstion Control Process acccording to the RIGD algorithm
 *  RIGD CA is a process where ad-hoc slotting takes place, the slot insertion is random, and the slot duration decays
 *	by the model (TCA0)(1 / 2(n+1)), where n >= 0 and TCA0 is the duration of the timeout for all slots.
 *
 *
 *  \param spectrum_id The Spectrum ID used for the CCA
 *  \param wait_for_t_ca_timeout Flag to indicate if the process needs to wait for a Tca timeout.
 */
void dll_rigd_ccp(bool wait_for_t_ca_timeout){
	timer_event event;

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

	current__t_ca = current__t_ca >> 1; // = % 2
	if (current__t_ca > current__t_g) {
		uint32_t n_time = rand();
		n_time = (n_time * current__t_ca) >> 15; // Random Time before the CCA will be executed

		#ifdef LOG_TRANS_ENABLED
		log_print_stack_string(LOG_TRANS, "RIGD: Wait Random Time: %d", (uint16_t) n_time);
		#endif

		event.next_event = (uint16_t) n_time; // Wait random time (0 - new__t_ca)
		event.f = &final_rigd;
		last_ca = timer_get_counter_value();
		timer_add_event(&event);
	} else {
		#ifdef LOG_TRANS_ENABLED
		log_print_stack_string(LOG_TRANS, "RIGD: Failed");
		#endif
		dll_tx_callback(DLLTxResultCAFail);
		return;
	}
}



/*! \file dll.h
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
 *
 * \brief The Data Link Layer API
 */

#ifndef DLL_H_
#define DLL_H_

#include "../types.h"
#include "../phy/phy.h"

typedef enum {
	FrameTypeForegroundFrame,
	FrameTypeBackgroundFrame
} Frame_Type;

typedef enum {
	DllStateNone,
    DllStateScanBackgroundFrame,
    DllStateScanForegroundFrame
} Dll_State_Enum;

typedef enum {
	DLLTxResultOK,
	DLLTxResultCCA1Fail,
	DLLTxResultCCA2Fail,
	DLLTxResultCAFail,
	DLLTxResultCCAOK,
	DLLTxResultFail,
	DLLTxInitiated
} Dll_Tx_Result;

// Frame Control
#define FRAME_CTRL_TARGET	1 << 7
#define FRAME_CTRL_VID		1 << 6
#define FRAME_EIRP(VAL)		(VAL & 0x3F)

#define CHANNEL_GUARD_INTERVAL		5
#define CHANNEL_SILENCE_INTERVAL	1


typedef struct {
	uint8_t length;
	uint8_t subnet;
	uint8_t control;
	uint8_t* target_address;
	uint8_t payload_length;
	uint8_t* payload;
} dll_frame_t;


typedef struct
{
	uint8_t subnet;				// Subnet
	uint8_t spectrum_id[2];		// Spectrum ID
	int8_t 	eirp;				// Transmission power level in dBm ranged [-39, +10]
	Frame_Type frame_type;		// Frame Type
} dll_tx_cfg_t;


// =======================================================================
// dll_rx_res_t
// -----------------------------------------------------------------------
/// Data Link Layer Packet reception result structure
// =======================================================================
typedef struct
{
    /// Frame Type
    Frame_Type  frame_type;
    /// Reception level
    int8_t rssi;
    /// Link quality indicator
    uint8_t  lqi;
    /// spectrum id
    uint8_t spectrum_id[2];
    /// Frame
    dll_frame_t* frame;
} dll_rx_res_t;

typedef struct
{
	uint8_t spectrum_id[2];
	Frame_Type scan_type; // BF / FF
	uint16_t timeout_scan_detect; // 0-65535 ti
	uint16_t time_next_scan; // 0-65535 ti
} dll_channel_scan_t;

typedef struct
{
	uint8_t length;
	dll_channel_scan_t* values;
} dll_channel_scan_series_t;


typedef enum {
	DllCsmaCaAind,
	DllCsmaCaRaind,
	DllCsmaCaRigd
} Dll_CSMA_CA_Type;


extern phy_tx_cfg_t *current_phy_cfg;

typedef void (*dll_tx_callback_t)(Dll_Tx_Result); // TODO result param?
typedef void (*dll_rx_callback_t)(dll_rx_res_t*);

void dll_init();
void dll_set_tx_callback(dll_tx_callback_t);
void dll_set_rx_callback(dll_rx_callback_t);

void dll_set_scan_minimum_energy(int16_t e_sm);
void dll_set_background_scan_detection_timeout(uint16_t t_bsd);
void dll_set_foreground_scan_detection_timeout(uint16_t t_fsd);
void dll_set_scan_spectrum_id(uint8_t spectrum_id[2]);

void dll_csma(bool enabled);
void dll_ca(uint8_t t_ca);
void dll_stop_channel_scan();
uint8_t dll_background_scan();
void dll_foreground_scan();
void dll_channel_scan_series(dll_channel_scan_series_t*);


void dll_set_initial_t_ca(uint16_t t_ca);
void dll_set_csma_ca(Dll_CSMA_CA_Type type);

void dll_initiate_csma_ca();
//static void dll_process_csma_ca();
//AIND
void dll_aind_ccp(bool init_status);
//RIGD
void dll_rigd_ccp(bool wait_for_t_ca_timeout);




/*! \brief Create a frame (Data Link Layer)
 *
 *  Creates a DLL Frame based on the parameters, optional target address and data.
 *  The payload should be added to the TX Queue prior to calling this method
 *
 *  \param target_address Optional target address, there is no target address on broadcast messages.
 *  \param address_length Lenght in bytes of the target address. 0 (broadcast message), 2 (VID), or 8 (UID).
 *  \param params the TX configuration (subnet, spectrum ID, EIRP, frame type (BF or FF)).
 */
void dll_create_frame(uint8_t* target_address, uint8_t address_length, dll_tx_cfg_t* params);
void dll_tx_frame();

#endif /* DLL_H_ */

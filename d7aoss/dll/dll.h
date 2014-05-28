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
	FrameTypeForegroundFrameDialogFrame = 0x00,
	FrameTypeForegroundFrameDialogNACK = 0x01,
	FrameTypeForegroundFrameStreamFrame = 0x02,
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
	DLLTxResultFail
} Dll_Tx_Result;

// Frame Control
#define FRAME_CTL_LISTEN 		(1 << 7)
#define FRAME_CTL_DLLS			(1 << 6)
#define FRAME_CTL_EN_ADDR		(1 << 5)
#define FRAME_CTL_FR_CONT 		(1 << 4)
#define FRAME_CTL_CRC32			(1 << 3)
#define FRAME_CTL_NM2			(1 << 2)
#define FRAME_CTL_DIALOGFRAME	(0)
#define FRAME_CTL_DIALOGNACK	(1)
#define FRAME_CTL_STREAMFRAME	(2)
#define FRAME_CTL_RFU			(3)

typedef struct {
	uint8_t dialogId;
	uint8_t flags; // see ADDR_CTL_* defines
	uint8_t* source_id; // only when framectrl en addr bit is set
	uint8_t* target_id; // only when framectrl nls = 0 and unicast
} dll_foreground_frame_address_ctl_t;

// Address Control Header
#define ADDR_CTL_UNICAST	(0 << 6)
#define ADDR_CTL_BROADCAST	(1 << 6)
#define ADDR_CTL_ANYCAST	(2 << 6)
#define ADDR_CTL_MULTICAST	(3 << 6)
#define ADDR_CTL_VID		(1 << 5)
#define ADDR_CTL_NLS		(1 << 4)
#define ADDR_CTL_APPFLAGS(VAL)	(VAL&0x0F)

typedef struct {
	uint8_t tx_eirp; // (-40 + 0.5n) dBm
	uint8_t subnet;
	uint8_t frame_ctl; // see FRAME_CTL_* defines
} dll_foreground_frame_header_t;

typedef struct {
	uint8_t dlls_code;
		uint8_t* dlls_initalization_data;
		uint8_t* dlls_footer;
} dll_foreground_frame_security_t;

typedef struct {
	uint8_t length;
	dll_foreground_frame_header_t frame_header;
	dll_foreground_frame_security_t* dlls_header;  // only when DLLS enabled in frame ctl
	dll_foreground_frame_address_ctl_t* address_ctl; // only when addressing enabled in frame ctl
	uint8_t payload_length;
	uint8_t* payload;
	// TODO DLLS footer
} dll_foreground_frame_t;

typedef struct {
	uint8_t subnet;
	uint8_t payload[4];
} dll_background_frame_t;

typedef struct {
	uint8_t dialog_id;
	uint8_t addressing_option;  // ADDR_CTL_UNICAST / ADDR_CTL_BROADCAST / ADDR_CTL_ANYCAST / ADDR_CTL_MULTICAST
	bool virtual_id; // 1> 2 byte virtual id // 0 > 8 byte device id
	uint8_t application_flags; // 4 bit
	uint8_t* source_id; // only when framectrl en addr bit is set
	uint8_t* target_id; // only when framectrl nls = 0 and unicast
} dll_foreground_frame_adressing;

typedef struct
{
	uint8_t subnet;				// Subnet
	uint8_t spectrum_id;		// Spectrum ID
	int8_t 	eirp;				// Transmission power level in dBm ranged [-39, +10]
	bool listen;				// Listen for T_l time for new packet after contention period
	dll_foreground_frame_security_t* security;		// DLL security header and footer
	dll_foreground_frame_adressing* addressing;
	bool nwl_security;			// Network Layer security enabled / disabled
	bool frame_continuity; 		// A frame follow directly after the current
	uint8_t frame_type;			// FRAME_CTL_DIALOGFRAME / FRAME_CTL_DIALOGNACK / FRAME_CTL_STREAMFRAME
} dll_ff_tx_cfg_t;


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
    uint8_t spectrum_id;
    /// Frame
    void* frame;
} dll_rx_res_t;

typedef struct
{
	uint8_t spectrum_id; // 0-255
	Frame_Type scan_type; // BF / FF
	uint16_t timeout_scan_detect; // 0-65535 ti
	uint16_t time_next_scan; // 0-65535 ti
} dll_channel_scan_t;

typedef struct
{
	uint8_t length;
	dll_channel_scan_t* values;
} dll_channel_scan_series_t;


extern phy_tx_cfg_t *current_phy_cfg;

typedef void (*dll_tx_callback_t)(Dll_Tx_Result); // TODO result param?
typedef void (*dll_rx_callback_t)(dll_rx_res_t*);

void dll_init();
void dll_set_tx_callback(dll_tx_callback_t);
void dll_set_rx_callback(dll_rx_callback_t);

void dll_set_scan_minimum_energy(int16_t e_sm);
void dll_set_background_scan_detection_timeout(uint16_t t_bsd);
void dll_set_foreground_scan_detection_timeout(uint16_t t_fsd);
void dll_set_scan_spectrum_id(uint8_t spectrum_id);

void dll_csma(bool enabled);
void dll_ca(uint8_t t_ca);
void dll_stop_channel_scan();
uint8_t dll_background_scan();
void dll_foreground_scan();
void dll_channel_scan_series(dll_channel_scan_series_t*);

void dll_create_foreground_frame(uint8_t* data, uint8_t length, dll_ff_tx_cfg_t* params);
void dll_create_background_frame(uint8_t* data, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp);

void dll_tx_frame();

#endif /* DLL_H_ */

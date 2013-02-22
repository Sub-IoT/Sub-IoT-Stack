/*
 * The PHY layer API
 *  Created on: Nov 23, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef DLL_H_
#define DLL_H_

#include "../types.h"
#include "../phy/phy.h"

typedef enum {
	FrameTypeBackgroundFrame,
	FrameTypeForegroundFrame
} Frame_Type;

typedef enum {
	DllStateNone,
    DllStateScanBackgroundFrame,
    DllStateScanForegroundFrame
} Dll_State_Enum;

typedef enum {
	DLLTxResultOK,
	DLLTxResultCCAFail,
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
	u8 dialogId;
	u8 flags; // see ADDR_CTL_* defines
} dll_foreground_frame_address_ctl_header_t;

// Address Control Header
#define ADDR_CTL_UNICAST	(0 << 6)
#define ADDR_CTL_BROADCAST	(1 << 6)
#define ADDR_CTL_ANYCAST	(2 << 6)
#define ADDR_CTL_MULTICAST	(3 << 6)
#define ADDR_CTL_VID		(1 << 5)
#define ADDR_CTL_NLS		(1 << 4)
#define ADDR_CTL_APPFLAGS(VAL)	(VAL&0x0F)

typedef struct {
	u8 tx_eirp; // (-40 + 0.5n) dBm
	u8 subnet;
	u8 frame_ctl; // see FRAME_CTL_* defines
} dll_foreground_frame_header_t;

typedef struct {
	u8 dlls_code;
	// TODO dlls_init_data
} dll_foreground_frame_dlls_header_t;

typedef struct {
	u8 length;
	dll_foreground_frame_header_t frame_header;
	dll_foreground_frame_dlls_header_t* dlls_header;  // only when DLLS enabled in frame ctl
	dll_foreground_frame_address_ctl_header_t* address_ctl_header; // only when addressing enabled in frame ctl
	u8* source_id_header; // only when framectrl en addr bit is set
	u8* target_id_header; // only when framectrl nls = 0 and unicast
	u8 payload_length;
	u8* payload;
	// TODO DLLS footer
} dll_foreground_frame_t;

typedef struct {
	u8 subnet;
	u8 payload[4];
} dll_background_frame_t;


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
    s8  rssi;
    /// Link quality indicator
    u8  lqi;
    /// spectrum id
    u8 spectrum_id;
    /// Frame
    void* frame;
} dll_rx_res_t;

typedef struct
{
	u8 spectrum_id; // 0-255
	Frame_Type scan_type; // BF / FF
	u16 timout_scan_detect; // 0-65535 ti
	u16 time_next_scan; // 0-65535 ti
} dll_channel_scan_t;

typedef struct
{
	u8 length;
	dll_channel_scan_t* values;
} dll_channel_scan_series_t;




typedef void (*dll_tx_callback_t)(Dll_Tx_Result); // TODO result param?
typedef void (*dll_rx_callback_t)(dll_rx_res_t*);

void dll_init();
void dll_set_tx_callback(dll_tx_callback_t);
void dll_set_rx_callback(dll_rx_callback_t);

void dll_stop_channel_scan();
void dll_channel_scan_series(dll_channel_scan_series_t*);

void dll_tx_foreground_frame(u8* data, u8 length, u8 spectrum_id, s8 tx_eirp); // TODO spectrum id (and other tx params) should come from ALP file later
void dll_tx_background_frame(u8* data, u8 subnet, u8 spectrum_id, s8 tx_eirp);

#endif /* DLL_H_ */

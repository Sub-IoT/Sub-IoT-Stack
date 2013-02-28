/*
 * The Network layer API
 *  Created on: February 2, 2013
 *  Authors:
 * 		maarten.weyn@artesis.be
 */
#include "../types.h"
#include "../dll/dll.h"

#define BPID_AdvP 0xF0
#define BPID_ResP 0xF1

typedef struct {
	u8 length;
	u8 tx_eirp;
	u8 subnet;
	u8 bpid;
	u8 protocol_data[3];
} nwl_background_frame_t;


typedef struct {
	u8 channel_id;
	u8 eta[2];
} AdvP_Data;

typedef struct {
	u8 res_type;
	u8 res_duration[2];
} ResP_Data;

typedef struct
{
    /// Frame Type
    Frame_Type  frame_type;
    /// Frame
    void* frame;
} nwl_rx_res_t;


typedef void (*nwl_tx_callback_t)(Dll_Tx_Result);
typedef void (*nwl_rx_callback_t)(nwl_rx_res_t *);

void nwl_init();
void nwl_set_tx_callback(nwl_tx_callback_t);
void nwl_set_rx_callback(nwl_rx_callback_t);

void nwl_tx_advertising_protocol_data(u8 channel_id, u16 eta, u8 tx_eirp, u8 subnet, u8 spectrum_id);
void nwl_tx_reservation_protocol_data(u8 res_type, u16 res_duration, u8 tx_eirp, u8 subnet, u8 spectrum_id);
void nwl_tx_background_frame(nwl_background_frame_t* data, u8 spectrum_id);

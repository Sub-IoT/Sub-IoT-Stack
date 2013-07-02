/*
 * The Network layer API
 *  Created on: February 2, 2013
 *  Authors:
 * 		maarten.weyn@artesis.be
 */
#include "../types.h"
#include "../dll/dll.h"

#define BPID_AdvP 0xF0

typedef struct {
	uint8_t tx_eirp;
	uint8_t subnet;
	uint8_t bpid;
	uint8_t protocol_data[2];
} nwl_background_frame_t;

typedef struct {
	uint16_t eta;
} AdvP_Data;


typedef struct
{
    /// Frame Type
    Frame_Type  frame_type;
    /// Frame
    void* frame;
} nwl_rx_res_t;

typedef struct {
	uint8_t nls_code;
	uint8_t* nls_initialization_data;
	uint8_t* target_address;
} nwl_security;

typedef struct {
	uint8_t hop_control;
	uint8_t hop_ext;
	uint8_t* origin_device_id;
	uint8_t* destination_device_id;
} nwl_routing_header;


typedef void (*nwl_tx_callback_t)(Dll_Tx_Result);
typedef void (*nwl_rx_callback_t)(nwl_rx_res_t *);

void nwl_init();
void nwl_set_tx_callback(nwl_tx_callback_t);
void nwl_set_rx_callback(nwl_rx_callback_t);

// parameters for DLL
//TODO: set this through configuration files


// Background frames
void nwl_build_advertising_protocol_data(uint8_t channel_id, uint16_t eta, int8_t tx_eirp, uint8_t subnet);
//void nwl_tx_background_frame(nwl_background_frame_t* data, uint8_t spectrum_id);

// Foreground frames
void nwl_build_network_protocol_data(uint8_t* data, uint8_t length, nwl_security* security, nwl_routing_header* routing, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp, uint8_t dialog_id);

void nwl_build_datastream_protocol_data(uint8_t* data, uint8_t length, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp, uint8_t dialog_id);

/*
 * The PHY layer API
 *  Created on: Nov 23, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */
#include "dll.h"
#include "../timer.h"
#include "../hal/system.h"
#include "../hal/crc.h"
#include "../log.h"
#include <string.h>

static dll_rx_callback_t dll_rx_callback;
static dll_tx_callback_t dll_tx_callback;
static dll_rx_res_t dll_res;

static dll_channel_scan_series_t* current_css;
static u8 current_scan_id = 0;

u8 timeout_listen; // TL

u8 frame_data[50]; // TODO max frame size

Dll_State_Enum dll_state;

phy_tx_cfg_t foreground_frame_tx_cfg = {
	    0x10, // spectrum ID
		0, // coding scheme
		1, // sync word class
	    frame_data,
	    0
};


static bool check_subnet(u8 device_subnet, u8 frame_subnet)
{
	if (frame_subnet & 0xF0 != 0xF0)
	{
		if (frame_subnet & 0xF0 != device_subnet & 0xF0)
			return 0;
	}

	if (frame_subnet & device_subnet & 0x0F != device_subnet & 0x0F)
			return 0;

	return 1;
}

static void phy_tx_callback()
{
	#ifdef LOG_DLL_ENABLED
		log_print_string("TX OK");
	#endif
	dll_tx_callback(DLLTxResultOK);
}

static void phy_rx_callback(phy_rx_res_t* res)
{
	//log_packet(res->data);

	// Data Link Filtering
	// Subnet Matching do not parse it yet
	if (dll_state == DllStateScanBackgroundFrame)
	{
		u16 crc = crc_calculate(res->data, 5);
		if (memcmp((u8*) &(res->data[5]), (u8*) &crc, 2) != 0)
		{
			#ifdef LOG_DLL_ENABLED
				log_print_string("CRC ERROR");
			#endif
			return;
		}

		if (!check_subnet(0xFF, res->data[0])) // TODO: get device_subnet from datastore
		{
			#ifdef LOG_DLL_ENABLED
				log_print_string("Subnet mismatch");
			#endif
			return;
		}
	} else if (dll_state == DllStateScanForegroundFrame)
	{
		u16 crc = crc_calculate(res->data, res->len - 2);
		if (memcmp((u8*) &(res->data[res->len - 2]), (u8*) &crc, 2) != 0)
		{
			#ifdef LOG_DLL_ENABLED
				log_print_string("CRC ERROR");
			#endif
			return;
		}
		if (!check_subnet(0xFF, res->data[3])) // TODO: get device_subnet from datastore
		{
			#ifdef LOG_DLL_ENABLED
				log_print_string("Subnet mismatch");
			#endif

			return;
		}
	} else
	{
		#ifdef LOG_DLL_ENABLED
			log_print_string("You fool, you can't be here");
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

		u8* data_pointer = res->data + 4;

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
			dll_foreground_frame_address_ctl_header_t* address_ctl = (dll_foreground_frame_address_ctl_header_t*) data_pointer;
			frame->address_ctl_header = address_ctl;
			data_pointer += sizeof(u8*);

			u8 addressing = (address_ctl->flags & 0xC0) >> 6;
			u8 vid = (address_ctl->flags & 0x20) >> 5;
			u8 nls = (address_ctl->flags & 0x10) >> 4;
			// TODO parse Source ID Header

			frame->source_id_header = data_pointer;
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
				u8 id_target[8];
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
				frame->target_id_header = (u8*) &id_target;
			} else {
				frame->target_id_header = NULL;
			}
		} else {
			frame->address_ctl_header = NULL;
			frame->source_id_header = NULL;
		}

		if (frame->frame_header.frame_ctl & 0x10) // Frame continuity
		{
			// TODO handle more than 1 frame
		}

		if (frame->frame_header.frame_ctl & 0x08) // CRC 32
		{
			// TODO support CRC32
		}

		if (frame->frame_header.frame_ctl & 0x04) // Note Mode 2
		{
			// Not supported
		}

		// Frame Type
		//u8 ffType = frame_header.frame_ctl & 0x03;

		data_pointer++; // TODO what is this?
		data_pointer++; //isfid
		data_pointer++; //isfoffset

		frame->payload_length = (*data_pointer);
		data_pointer++;
		frame->payload = data_pointer;

		dll_res.frame_type = FrameTypeForegroundFrame;
		dll_res.frame = frame;
	}

	#ifdef LOG_DLL_ENABLED
		log_dll_rx_res(&dll_res);
	#endif
	dll_rx_callback(&dll_res);
}

static void scan_next(void* arg)
{
	dll_channel_scan_series(current_css);
}

static void scan_timeout(void* arg)
{
	if (dll_state == DllStateNone)
		return;

	#ifdef LOG_DLL_ENABLED
		log_print_string("scan time-out");
	#endif
	phy_rx_stop();
	timer_event event;
	event.next_event = current_css->values[current_scan_id].time_next_scan;
	event.f = &scan_next;

	current_scan_id = current_scan_id < current_css->length - 1 ? current_scan_id + 1 : 0;

	timer_add_event(&event);
}

void dll_init()
{
	phy_init();
	phy_set_tx_callback(&phy_tx_callback);
	phy_set_rx_callback(&phy_rx_callback);

	dll_state = DllStateNone;

	timer_init();
}

void dll_set_tx_callback(dll_tx_callback_t cb)
{
	dll_tx_callback = cb;
}
void dll_set_rx_callback(dll_rx_callback_t cb)
{
	dll_rx_callback = cb;
}

void dll_stop_channel_scan()
{
	// TODO remove scan_timeout events from queue?
	dll_state = DllStateNone;
	phy_rx_stop();
}

void dll_channel_scan_series(dll_channel_scan_series_t* css)
{
	#ifdef LOG_DLL_ENABLED
		log_print_string("Starting channel scan");
	#endif

	phy_rx_cfg_t rx_cfg;
	rx_cfg.timeout = css->values[current_scan_id].timout_scan_detect; // timeout
	rx_cfg.multiple = 0; // multiple TODO
	rx_cfg.spectrum_id = css->values[current_scan_id].spectrum_id; // spectrum ID TODO
	rx_cfg.coding_scheme = 0; // coding scheme TODO
	rx_cfg.rssi_min = 0; // RSSI min filter TODO
	if (css->values[current_scan_id].scan_type == FrameTypeForegroundFrame)
	{
		rx_cfg.sync_word_class = 1;
		dll_state = DllStateScanForegroundFrame;
	} else {
		rx_cfg.sync_word_class = 0;
		dll_state = DllStateScanBackgroundFrame;
	}

	current_css = css;
	phy_rx_start(&rx_cfg);

	//TODO: timeout should be implemented using rF timer in phy
	timer_event event;
	event.next_event = rx_cfg.timeout;
	event.f = &scan_timeout;

	timer_add_event(&event);
}

static void dll_cca2(void* arg)
{
	bool cca2 = phy_cca();
	if (!cca2)
	{
		dll_tx_callback(DLLTxResultCCAFail);
		return;
	}

	phy_result_t res = phy_tx(&foreground_frame_tx_cfg);
}

void dll_tx_foreground_frame(u8* data, u8 length, u8 spectrum_id, s8 tx_eirp)
{
	//TODO: check if not already sending
	foreground_frame_tx_cfg.spectrum_id = spectrum_id; // TODO check valid
	foreground_frame_tx_cfg.eirp = tx_eirp;

	dll_foreground_frame_t* frame = (dll_foreground_frame_t*) frame_data;
	frame->frame_header.tx_eirp = (tx_eirp + 40) * 2; // (-40 + 0.5n) dBm
	frame->frame_header.subnet = 0xf1; // TODO hardcoded, get from app?
	frame->frame_header.frame_ctl = !FRAME_CTL_LISTEN | !FRAME_CTL_DLLS | FRAME_CTL_EN_ADDR | !FRAME_CTL_FR_CONT | !FRAME_CTL_CRC32 | !FRAME_CTL_NM2 | FRAME_CTL_DIALOGFRAME; // TODO hardcoded

	dll_foreground_frame_address_ctl_header_t address_ctl_header;
	address_ctl_header.dialogId = 0x00; // TODO hardcoded
	address_ctl_header.flags = ADDR_CTL_BROADCAST | !ADDR_CTL_VID | !ADDR_CTL_NLS; // TODO appl flags?

	u8* pointer = frame_data + 1 + sizeof(dll_foreground_frame_header_t);
	memcpy(pointer, &address_ctl_header, sizeof(dll_foreground_frame_address_ctl_header_t));
	pointer += sizeof(dll_foreground_frame_address_ctl_header_t);
	memcpy(pointer, tag_id, 8 * sizeof(u8)); // TODO get from HAL, using global defined in system.h for now
	pointer += 8 * sizeof(u8);

	*pointer++ = 0; //dunno
	*pointer++ = 0; //isfid
	*pointer++ = 0; //isfoffset
	*pointer++ = length; // payload length;


	memcpy(pointer, data, length); // TODO fixed size for now
	pointer += length;

	frame->length = (pointer - frame_data) + 2;  // length includes CRC

	u16 crc16 = crc_calculate(frame_data, frame->length - 2);
	memcpy(pointer, &crc16, 2);

	foreground_frame_tx_cfg.len = frame->length;

	bool cca1 = phy_cca();

	if (!cca1)
	{
		dll_tx_callback(DLLTxResultCCAFail);
		return;
	}

	timer_event event;
	event.next_event = 5; // TODO: get T_G fron config
	event.f = &dll_cca2;

	if (!timer_add_event(&event))
		dll_tx_callback(DLLTxResultFail);

//	phy_result_t res = phy_tx(&foreground_frame_tx_cfg);
//	if(res == PHY_RADIO_IN_RX_MODE)
//	{
//		phy_rx_stop(); // TODO who is responsible for starting rx again? appl or DLL?
//		res = phy_tx(&foreground_frame_tx_cfg);
//	}
}

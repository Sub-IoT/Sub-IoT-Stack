/*
 * The PHY layer API
 *  Created on: Nov 23, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */
#include "dll.h"
#include "../log.h"

static dll_rx_callback_t dll_rx_callback;
static dll_tx_callback_t dll_tx_callback;


Dll_State_Enum dll_state;

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

void tx_callback()
{
	log_print_string("TX OK", 5);
}

void rx_callback(phy_rx_res_t* res)
{
	// Data Link Filtering

	// CRC Validation
	if (!res->crc_ok)
	{
		log_print_string("CRC ERROR", 9);
		return;
	}

	// Subnet Matching
	if (dll_state == DllStateScanBackgroundFrame)
	{
		if (!check_subnet(0xFF, res->data[0])) // TODO: get device_subnet from datastore
		{
			log_print_string("Subnet mismatch", 15);
			return;
		}
	} else if (dll_state == DllStateScanBackgroundFrame)
	{
		if (!check_subnet(0xFF, res->data[3])) // TODO: get device_subnet from datastore
		{
			log_print_string("Subnet mismatch", 15);
			return;
		}
	} else
	{
		Log_PrintString("You fool, you can't be here", 27);
	}

	// Optional Link Quality Assessment
}

void dll_init()
{
	phy_init();
	phy_set_tx_callback(&tx_callback);
	phy_set_rx_callback(&rx_callback);

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

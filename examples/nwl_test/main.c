/*
 *  Created on: Feb 2, 2013
 *  Authors:
 * 		maarten.weyn@artesis.be
 */


#include <string.h>
#include <nwl/nwl.h>
#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <log.h>
#include <timer.h>

static u16 counter = 0;

static u8 tx = 0;
static timer_event event;
static u16 timer = 30000;

void send_adv_prot_data()
{
	nwl_tx_advertising_protocol_data(0x10, timer, 0, 0xFF, 0x10);
	timer -= 1024;
}

void rx_callback(dll_rx_res_t* rx_res)
{
	log_print_string("RX CB");
}

void tx_callback(Dll_Tx_Result result)
{
	if(result == DLLTxResultOK)
	{
		counter++;
		led_off(3);
		log_print_string("TX OK");
	}
	else if (result == DLLTxResultCCAFail)
	{
		led_toggle(1);
		log_print_string("TX CCA FAIL");
	} else
	{
		led_toggle(1);
		log_print_string("TX FAIL");
	}

	tx = 0;

	timer_add_event(&event);
}


void main(void) {
	system_init();
	button_enable_interrupts();

	nwl_init();
	nwl_set_tx_callback(&tx_callback);
	nwl_set_rx_callback(&rx_callback);

	log_print_string("started");


	event.f = &send_adv_prot_data;
	event.next_event = 1024;

	timer_add_event(&event);

	while(1)
	{
		system_lowpower_mode(4,1);
	}
}


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


#define ADV_TIMESPAN 5
#define MSG_TIMESPAN 2000;
static u16 counter = 0;

static timer_event event;
static u16 timer = 500;
bool csma_ok = false;

void send_adv_prot_data(void * arg)
{
	led_on(1);

	nwl_build_advertising_protocol_data(0x10, timer, 0, 0xFF, 0x10);
	log_print_data((uint8_t*)&timer, 2);


	if (!csma_ok)
	{
		dll_ca(100);
		return;
	}



	timer -= ADV_TIMESPAN;
	if (timer > 0)
	{
		timer_add_event(&event);
	} else {
		led_on(3);
		nwl_build_network_protocol_data((uint8_t*) &counter, 2, NULL, NULL, 0xFF, 0x10, 0, counter & 0xFF);


		csma_ok = false;

		counter++;
		event.next_event = MSG_TIMESPAN;

		timer_add_event(&event);

		timer = 500;
		event.next_event = ADV_TIMESPAN;
	}
	dll_tx_frame();
}

void rx_callback(nwl_rx_res_t* rx_res)
{
	log_print_string("RX CB");
}

void tx_callback(Dll_Tx_Result result)
{


	if(result == DLLTxResultOK)
	{
		counter++;
		led_off(1);
		led_off(3);
		log_print_string("TX OK");
	}
	else if (result == DLLTxResultCCAOK)
	{
		csma_ok = true;
		send_adv_prot_data(NULL);
	}
	else if (result == DLLTxResultCCA1Fail || result == DLLTxResultCCA2Fail)
	{
		led_off(1);
		led_toggle(2);
		log_print_string("TX CCA FAIL");
	} else
	{
		led_toggle(2);
		log_print_string("TX FAIL");
	}

}


void main(void) {
	system_init();
	//button_enable_interrupts();

	nwl_init();
	nwl_set_tx_callback(&tx_callback);
	nwl_set_rx_callback(&rx_callback);



	log_print_string("started");

	event.f = &send_adv_prot_data;
	event.next_event = ADV_TIMESPAN;

	timer_add_event(&event);

	while(1)
	{
		system_lowpower_mode(4,1);
	}
}


#pragma vector=ADC12_VECTOR,AES_VECTOR,COMP_B_VECTOR,DMA_VECTOR,PORT1_VECTOR,PORT2_VECTOR,RTC_VECTOR,SYSNMI_VECTOR,UNMI_VECTOR,USCI_A0_VECTOR,USCI_B0_VECTOR,WDT_VECTOR,TIMER0_A1_VECTOR,TIMER0_A0_VECTOR,TIMER1_A1_VECTOR
__interrupt void ISR_trap(void)
{
  /* For debugging purposes, you can trap the CPU & code execution here with an
     infinite loop */
  //while (1);
	__no_operation();

  /* If a reset is preferred, in scenarios where you want to reset the entire system and
     restart the application from the beginning, use one of the following lines depending
     on your MSP430 device family, and make sure to comment out the while (1) line above */

  /* If you are using MSP430F5xx or MSP430F6xx devices, use the following line
     to trigger a software BOR.   */
  PMMCTL0 = PMMPW | PMMSWBOR;          // Apply PMM password and trigger SW BOR
}


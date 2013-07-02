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
#include <framework/log.h>
#include <framework/timer.h>
#include <hal/cc430/driverlib/5xx_6xx/rtc.h>
#include <phy/cc430/cc430_phy.h>
#include <hal/crc.h>
#include <dll/dll.h>

#include <msp430.h>

#define ADV_TIMESPAN 5
#define MSG_TIMESPAN 5000;

#define SEND_CHANNEL 0x1C
#define SEND_SUBNET 0xFF
#define USE_LEDS

#define CLOCKS_PER_1us	1
#define BATTERY_OFFSET	-10

static uint16_t counter = 0;

static timer_event event;
static
volatile bool csma_ok = false;
volatile bool initialized = false;

uint8_t data[64];
uint8_t dataLength = 0;
uint8_t check = 0;

uint16_t battery_voltage;

uint16_t adc12_result;
volatile uint8_t  adc12_data_ready;
volatile bool update_battery_level = false;


uint16_t battery_measurement(void);
Calendar currentTime;
uint16_t timetosend = 0;

uint32_t t1, t2;

Frame_Type frameType;


void send_adv_prot_data(void * arg)
{
	led_on(1);

//	event.next_event = MSG_TIMESPAN;
//	timer_add_event(&event);
//
//	log_print_string("FF");
//
//	nwl_build_network_protocol_data((uint8_t*) &data, dataLength, NULL, NULL, 0xFF, SEND_CHANNEL, 10, counter & 0xFF);
//	dll_tx_frame();
//	led_off(1);
//
//	return;


	int16_t timer = 500;


	uint8_t packet[255];
	packet[0] = SEND_SUBNET;
	packet[1] = BPID_AdvP;
	packet[2] = timer >> 8;
	packet[3] = timer & 0XFF;
	uint16_t crc16 = crc_calculate(&packet[0], 4);
	memcpy(&packet[4], &crc16, 2);

	// 4 byte preamble
	packet[6] = 0xAA;
	packet[7] = 0xAA;
	packet[8] = 0xAA;
	packet[9] = 0xAA;

	// 2 byte sync word
	packet[10] = 0xE6;
	packet[11] = 0xD0;
	packet[12] = SEND_SUBNET;
	packet[13] = BPID_AdvP;
	packet[14] = timer >> 8;
	packet[15] = timer & 0XFF;
	crc16 = crc_calculate(&packet[12], 4);
	memcpy(&packet[16], &crc16, 2);

	uint8_t length = 19;


	//log_print_data((uint8_t*)&timer, 2);
	//check++;



//	if (!csma_ok)
//	{
//		nwl_build_advertising_protocol_data(SEND_CHANNEL, 500, 10, SEND_SUBNET);
//		//dissable_autocalibration();
//		dll_ca(100);
//		return;
//	}

	if (!initialized)
	{
			initialized = true;
//			phy_init_tx();
//			phy_set_tx(current_phy_cfg);
//			phy_keep_radio_on(true);
			timetosend = timer_get_counter_value() + 500;
	}

	uint16_t currentTime = timer_get_counter_value();

	timer = timetosend - currentTime;

	log_print_string("timer value: %d", timer);
	if (timer > 500)
	{
		__no_operation();
	}
	else if (timer > 3)
	{
		// building takes 400us
		nwl_build_advertising_protocol_data(SEND_CHANNEL, timer, 10, SEND_SUBNET);
		frameType = FrameTypeBackgroundFrame;

		// sending frame takes 2600us
		dll_tx_frame();
//		if (timer < ADV_TIMESPAN)
//		{
//			event.next_event = timer;
//		}

		//timer_add_event(&event);

	} else {
		led_on(3);
		nwl_build_network_protocol_data((uint8_t*) &data, dataLength, NULL, NULL, 0xFF, SEND_CHANNEL, 10, counter & 0xFF);

		frameType = FrameTypeForegroundFrame;
		//phy_keep_radio_on(false);

		csma_ok = false;
		initialized = false;

		log_print_string("FF");
		dll_tx_frame();


		//phy_idle();

		counter++;
		event.next_event = MSG_TIMESPAN;

		timer_add_event(&event);

		timer = 500;
		event.next_event = ADV_TIMESPAN;


		system_watchdog_timer_reset();
	}
	//dll_tx_frame();
}

void rx_callback(nwl_rx_res_t* rx_res)
{
	log_print_string("RX CB");
}

void tx_callback(Dll_Tx_Result result)
{
	//check--;
	if(result == DLLTxResultOK)
	{
		counter++;
		led_off(1);
		led_off(3);

		if (frameType == FrameTypeBackgroundFrame)
		{
			send_adv_prot_data(NULL);
		} else {
			//Strobe(RF_SCAL);
		}
		//log_print_string("TX OK");
	}
	else if (result == DLLTxResultCCAOK)
	{
		csma_ok = true;

		log_print_string("CA");
		send_adv_prot_data(NULL);

	}
	else if (result == DLLTxResultCCA1Fail || result == DLLTxResultCCA2Fail)
	{
		led_off(1);
		led_toggle(2);
		log_print_string("TX CCA FAIL");
	}
	else if (result == DLLTxResultCAFail)
	{
		led_off(1);
		led_toggle(2);
		log_print_string("TX CA FAIL");
	}
	else
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



	//manual_calibration();

	//Setup Current Time for Calendar
	currentTime.Seconds    = 0x50;
	currentTime.Minutes    = 0x59;
	currentTime.Hours      = 0x00;
	currentTime.DayOfWeek  = 0x03;
	currentTime.DayOfMonth = 0x01;
	currentTime.Month      = 0x01;
	currentTime.Year       = 0x2013;

	RTC_calendarInit(__MSP430_BASEADDRESS_RTC__,
			currentTime,
			RTC_FORMAT_BCD);

	RTC_setCalendarEvent(__MSP430_BASEADDRESS_RTC__,
			RTC_CALENDAREVENT_HOURCHANGE);

	RTC_enableInterrupt(__MSP430_BASEADDRESS_RTC__, RTCTEVIE + RTCAIE);


	rtc_start();

	data[0] = 0x65;
	data[1] = 0;
	data[2] = 0;
	dataLength = 3;

	log_print_string("started");

//	benchmarking_timer_init();
//	benchmarking_timer_start();

	event.f = &send_adv_prot_data;
	event.next_event = ADV_TIMESPAN;

	timer_add_event(&event);

	system_watchdog_init(WDTSSEL0, 0x03); // 32KHz / 2^19
	system_watchdog_timer_start();


	while(1)
	{
		if (update_battery_level)
		{
			dll_stop_channel_scan();
			update_battery_level = false;

			battery_voltage = battery_measurement();
			data[1] = (uint8_t) (battery_voltage>> 8);
			data[2] = (uint8_t) battery_voltage;
		}

		system_lowpower_mode(4,1);
	}
}

uint16_t adc12_single_conversion(uint16_t ref, uint16_t sht, uint16_t channel)
{
	// Initialize the shared reference module
	REFCTL0 |= REFMSTR + ref + REFON;    		// Enable internal reference (1.5V or 2.5V)

	// Initialize ADC12_A
	ADC12CTL0 = sht + ADC12ON;					// Set sample time
	ADC12CTL1 = ADC12SHP;                     	// Enable sample timer
	ADC12MCTL0 = ADC12SREF_1 + channel;  		// ADC input channel
	ADC12IE = 0x001;                          	// ADC_IFG upon conv result-ADCMEMO

  	// Wait 2 ticks (66us) to allow internal reference to settle
	__delay_cycles(66*CLOCKS_PER_1us);

	// Start ADC12
	ADC12CTL0 |= ADC12ENC;

	// Clear data ready flag
  	adc12_data_ready = 0;

  	// Sampling and conversion start
    ADC12CTL0 |= ADC12SC;

    // Wait until ADC12 has finished
    __delay_cycles(200*CLOCKS_PER_1us);
	while (!adc12_data_ready);

	// Shut down ADC12
	ADC12CTL0 &= ~(ADC12ENC | ADC12SC | sht);
	ADC12CTL0 &= ~ADC12ON;

	// Shut down reference voltage
	REFCTL0 &= ~(REFMSTR + ref + REFON);

	ADC12IE = 0;

	// Return ADC result
	return (adc12_result);
}

uint16_t battery_measurement(void)
{
	uint16_t voltage;

	voltage = adc12_single_conversion(REFVSEL_1, ADC12SHT0_10, ADC12INCH_11);
	voltage = (voltage * 2 * 2) / 41;
	voltage += BATTERY_OFFSET;

	return voltage;
}

#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR (void)
{
    switch (RTCIV){
        case 0: break;  //No interrupts
        case 2: break;  //RTCRDYIFG
        case 4:         //RTCEVIFG
        	update_battery_level = true;
        	LPM4_EXIT;
            break;
        case 6: break;  //RTCAIFG
        case 8: break;  //RT0PSIFG
        case 10: break; //RT1PSIFG
        default: break;
    }
}


#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR (void)
{
  switch(__even_in_range(ADC12IV,34))
  {
  case  0: break;                           // Vector  0:  No interrupt
  case  2: break;                           // Vector  2:  ADC overflow
  case  4: break;                           // Vector  4:  ADC timing overflow
  case  6:                                  // Vector  6:  ADC12IFG0
    		adc12_result = ADC12MEM0;                       // Move results, IFG is cleared
    		adc12_data_ready = 1;
    		//_BIC_SR_IRQ(LPM3_bits);   						// Exit active CPU
    		break;
  case  8: break;                           // Vector  8:  ADC12IFG1
  case 10: break;                           // Vector 10:  ADC12IFG2
  case 12: break;                           // Vector 12:  ADC12IFG3
  case 14: break;                           // Vector 14:  ADC12IFG4
  case 16: break;                           // Vector 16:  ADC12IFG5
  case 18: break;                           // Vector 18:  ADC12IFG6
  case 20: break;                           // Vector 20:  ADC12IFG7
  case 22: break;                           // Vector 22:  ADC12IFG8
  case 24: break;                           // Vector 24:  ADC12IFG9
  case 26: break;                           // Vector 26:  ADC12IFG10
  case 28: break;                           // Vector 28:  ADC12IFG11
  case 30: break;                           // Vector 30:  ADC12IFG12
  case 32: break;                           // Vector 32:  ADC12IFG13
  case 34: break;                           // Vector 34:  ADC12IFG14
  default: break;
  }
}

#pragma vector=AES_VECTOR,COMP_B_VECTOR,DMA_VECTOR,PORT1_VECTOR,PORT2_VECTOR,SYSNMI_VECTOR,UNMI_VECTOR,USCI_A0_VECTOR,USCI_B0_VECTOR,WDT_VECTOR,TIMER0_A0_VECTOR,TIMER1_A1_VECTOR
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

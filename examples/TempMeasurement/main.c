/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 *     maarten.weyn@uantwerpen.be
 *
 * 	Example of the notification system
 *
 * 	add the link to d7aoss library in de lnk_*.cmd file, e.g. -l "../../../d7aoss/Debug/d7aoss.lib"
 * 	Make sure to select the correct platform in d7aoss/hal/cc430/platforms.platform.h
 * 	If your platform is not present, you can add a header file in platforms and commit it to the repository.
 * 	Exclude the stub directories in d7aoss from the build when building for a device.
 *
 * 	Create the apporpriate file system settings for the FLASH system:
 *
 * 	Add following sections to the SECTIONS in .cmd linker file to use the filesystem
 *		.fs_fileinfo_bitmap : 	{} > FLASH_FS1
 *  	.fs_fileinfo: 			{} > FLASH_FS1
 *		.fs_files	: 			{} > FLASH_FS2
 *
 *	Add FLASH_FS_FI and FLASH_FS_FILES to the MEMORY section
 *  eg.
 *  	FLASH_FS1               : origin = 0xC000, length = 0x0200 // The file headers
 *	    FLASH_FS2               : origin = 0xC200, length = 0x0400 // The file contents
 */

#include <d7aoss.h>
//#include <framework/log.h>
#include <framework/timer.h>
#include <msp430.h>
#include <hal/leds.h>


#define TEMPERATURE_INTERVAL_MS 2000
#define CLOCKS_PER_1us	20
#define USE_LEDS 1
#define LED_OK	3
#define LED_ERROR	3

static uint8_t buffer[128];
static volatile uint8_t add_sensor_event = 0;
static volatile uint16_t adc12_result;
static volatile uint8_t adc12_data_ready;

// event to create a led blink
static timer_event dim_led_event;

int16_t temperature_internal;

void blink_led()
{
	led_on(LED_OK);

	timer_add_event(&dim_led_event);
}

void dim_led()
{
	led_off(LED_OK);
}

void get_temperature()
{
	add_sensor_event = true;
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

int16_t temperature_measurement()
{
	uint16_t adc_result;
	volatile int16_t temperaturemV;

	// Convert internal temperature diode voltage
	adc_result = adc12_single_conversion(REFVSEL_0, ADC12SHT0_8, ADC12INCH_10);
	temperaturemV = (int16_t) (adc_result * 0.3662109375); //(= /4096*1500)

    return temperaturemV;
}


void tx_callback(Trans_Tx_Result result)
{
	if(result == TransPacketSent)
	{
		#ifdef USE_LEDS
		led_off(LED_ERROR);
		blink_led();
		#endif
		//log_print_string("TX OK");
	}
	else
	{
		#ifdef USE_LEDS
		led_on(LED_ERROR);
		#endif
		//log_print_string("TX CCA FAIL");
	}
}


int main(void) {
	timer_event event;
	int16_t temperature_internal;
	file_handler fh;

	d7aoss_init(buffer, 32, buffer, 32);
	trans_set_tx_callback(&tx_callback);
	
	// Configure event to measure temperature
	event.next_event = TEMPERATURE_INTERVAL_MS;
	event.f = &get_temperature;

	// configure blinking led event
	dim_led_event.next_event = 50;
	dim_led_event.f = &dim_led;

	timer_add_event(&event);

	while(1)
	{
		if (add_sensor_event)
		{
			add_sensor_event = false;

			temperature_internal = temperature_measurement();

			fs_open(&fh, 32, file_system_user_user, file_system_access_type_write);

			uint8_t data[2];
			data[0] = (uint8_t) (temperature_internal>> 8);
			data[1] = (uint8_t) (temperature_internal);

			fs_write_data(&fh, 2, data, 2,true);

			fs_close(&fh);

			timer_add_event(&event);
		}

		system_lowpower_mode(3,1);
	}

}


#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR (void)
{
  switch(__even_in_range(ADC12IV,34))
  {
//  case  0: break;                           // Vector  0:  No interrupt
//  case  2: break;                           // Vector  2:  ADC overflow
//  case  4: break;                           // Vector  4:  ADC timing overflow
  case  6:                                  // Vector  6:  ADC12IFG0
    		adc12_result = ADC12MEM0;                       // Move results, IFG is cleared
    		adc12_data_ready = 1;
    		//_BIC_SR_IRQ(LPM3_bits);   						// Exit active CPU
    		break;
//  case  8: break;                           // Vector  8:  ADC12IFG1
//  case 10: break;                           // Vector 10:  ADC12IFG2
//  case 12: break;                           // Vector 12:  ADC12IFG3
//  case 14: break;                           // Vector 14:  ADC12IFG4
//  case 16: break;                           // Vector 16:  ADC12IFG5
//  case 18: break;                           // Vector 18:  ADC12IFG6
//  case 20: break;                           // Vector 20:  ADC12IFG7
//  case 22: break;                           // Vector 22:  ADC12IFG8
//  case 24: break;                           // Vector 24:  ADC12IFG9
//  case 26: break;                           // Vector 26:  ADC12IFG10
//  case 28: break;                           // Vector 28:  ADC12IFG11
//  case 30: break;                           // Vector 30:  ADC12IFG12
//  case 32: break;                           // Vector 32:  ADC12IFG13
//  case 34: break;                           // Vector 34:  ADC12IFG14
  default: break;
  }
}



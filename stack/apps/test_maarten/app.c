/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hwleds.h"
#include "hwsystem.h"
#include "scheduler.h"
#include "timer.h"
#include "log.h"
#include <assert.h>
#include "platform.h"
#include "descriptors.h"

#include <stdio.h>
#include <stdlib.h>

#include "segmentlcd.h"

#include "em_adc.h"
#include "em_cmu.h"
#include "cdc.h"
#include "bsp.h"

bool sensing = false;
uint16_t counter = 0;
uint32_t temp;
uint32_t temp_offset;

#ifdef PLATFORM_EFM32GG_STK3700
#include "userbutton.h"
#include "platform_sensors.h"

void userbutton_callback(button_id_t button_id)
{
	log_print_string("Button PB%u pressed.", button_id);
	led_toggle(1);

}

#endif


/// USB
static const USBD_Callbacks_TypeDef callbacks =
{
  .usbReset        = NULL,
  .usbStateChange  = CDC_StateChangeEvent,
  .setupCmd        = CDC_SetupCmd,
  .isSelfPowered   = NULL,
  .sofInt          = NULL
};

const USBD_Init_TypeDef usbInitStruct =
{
  .deviceDescriptor    = &USBDESC_deviceDesc,
  .configDescriptor    = USBDESC_configDesc,
  .stringDescriptors   = USBDESC_strings,
  .numberOfStrings     = sizeof(USBDESC_strings)/sizeof(void*),
  .callbacks           = &callbacks,
  .bufferingMultiplier = USBDESC_bufferingMultiplier,
  .reserved            = 0
};

/**************************************************************************//**
 * @brief Convert ADC sample values to celsius.
 * @note See section 2.3.4 in the reference manual for details on this
 *       calculatoin
 * @param adcSample Raw value from ADC to be converted to celsius
 * @return The temperature in degrees Celsius.
 *****************************************************************************/
float convertToCelsius(int32_t adcSample)
{
  float temp;
  /* Factory calibration temperature from device information page. */
  float cal_temp_0 = (float)((DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK)
                             >> _DEVINFO_CAL_TEMP_SHIFT);

  float cal_value_0 = (float)((DEVINFO->ADC0CAL2
                               & _DEVINFO_ADC0CAL2_TEMP1V25_MASK)
                              >> _DEVINFO_ADC0CAL2_TEMP1V25_SHIFT);

  /* Temperature gradient (from datasheet) */
  float t_grad = -6.27;

  temp = (cal_temp_0 - ((cal_value_0 - adcSample)  / t_grad));

  return temp;
}

void ADC0_IRQHandler(void)
{
	/* Clear interrupt flag */
	ADC_IntClear(ADC0, ADC_IFC_SINGLEOF);

	//printf("ADC IRQ: DMA couldn't keep up with ADC sample rate :(\n");

}

void setupInternalTempSensor(void)
{
	/* Base the ADC configuration on the default setup. */
	ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
	ADC_InitSingle_TypeDef sInit = ADC_INITSINGLE_DEFAULT;

	/* Initialize timebases */
	init.timebase = ADC_TimebaseCalc(0);
	init.prescale = ADC_PrescaleCalc(400000,0);
	ADC_Init(ADC0, &init);

	/* Set input to temperature sensor. Reference must be 1.25V */
	sInit.reference = adcRef1V25;
	sInit.input = adcSingleInpTemp;
	ADC_InitSingle(ADC0, &sInit);

	/* Setup interrupt generation on completed conversion. */
	ADC_IntEnable(ADC0, ADC_IF_SINGLE);
	//NVIC_EnableIRQ(ADC0_IRQn);

	/* This is a work around for Chip Rev.D Errata, Revision 0.6. */
	/* Check for product revision 16 and 17 and set the offset */
	/* for ADC0_TEMP_0_READ_1V25. */
	uint8_t prod_rev = (DEVINFO->PART & _DEVINFO_PART_PROD_REV_MASK) >> _DEVINFO_PART_PROD_REV_SHIFT;

	if( (prod_rev == 16) || (prod_rev == 17) )
	{
		temp_offset = 112;
	}
	else
	{
		temp_offset = 0;
	}
}

void measureTemperature()
{
	ADC_Start(ADC0, adcStartSingle);

	/* Wait in EM1 for ADC to complete */
	while(!ADC0->STATUS){};
	ADC0_IRQHandler();

	/* Read sensor value */
	/* According to rev. D errata ADC0_TEMP_0_READ_1V25 should be decreased */
	/* by the offset  but it is the same if ADC reading is increased - */
	/* reference manual 28.3.4.2. */
	temp = ADC_DataSingleGet(ADC0) + temp_offset;

	int i = (int)(convertToCelsius(temp) * 10);
	char string[8];
	snprintf(string, 8, "%2d,%1d%%C", (i/10), abs(i%10));
	SegmentLCD_Write(string);

	log_print_string("Temperature %s", string);
}

void timer0_callback()
{
	char string[8];
	int i;

	led_toggle(0);
	timer_post_task_delay(&timer0_callback, TIMER_TICKS_PER_SEC);
	log_print_string("Toggled led %d", 0);

	measureTemperature();

	/*
	if (sensing)
	{
		lightsensor_dissable();

		SegmentLCD_Write("DAG");
	}
	else
	{
		lightsensor_enable();

		SegmentLCD_Write("SCHATJE");
	}

	sensing = !sensing;
	*/
}

void timer1_callback()
{
	led_toggle(1);
	timer_post_task_delay(&timer1_callback, 0x0000FFFF + (uint32_t)100);
	log_print_string("Toggled led %d", 1);
}


void bootstrap()
{
	led_on(0);
	led_on(1);


	BSP_Init(BSP_INIT_DEFAULT);   /* Initialize DK board register access */

	/* If first word of user data page is non-zero, enable eA Profiler trace */
	//BSP_TraceProfilerSetup();

	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);

	/* Initialize the communication class device. */
	CDC_Init();

	/* Initialize and start USB device stack. */
	USBD_Init(&usbInitStruct);


	/*
	* When using a debugger it is practical to uncomment the following three
	* lines to force host to re-enumerate the device.
	*/
	USBD_Disconnect();
	USBTIMER_DelayMs(1000);
	USBD_Connect();

	log_print_string("Device booted at time: %d\n", timer_get_counter_value());

	/* Initialize segment LCD. */
	SegmentLCD_Init(false);
	/* Turn all LCD segments off. */
	SegmentLCD_AllOff();





	/* Enable peripheral clocks */
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_ADC0, true);

	setupInternalTempSensor();


    sched_register_task(&timer0_callback);
    //sched_register_task(&timer1_callback);

    timer_post_task_delay(&timer0_callback, TIMER_TICKS_PER_SEC);
    //timer_post_task_delay(&timer1_callback, 0x0000FFFF + (uint32_t)100);

#ifdef PLATFORM_EFM32GG_STK3700
    ubutton_register_callback(0, &userbutton_callback);
    ubutton_register_callback(1, &userbutton_callback);
#endif


    led_off(0);
    led_off(1);

    measureTemperature();




}


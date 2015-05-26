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

/*! \file efm32gg_adc.c
 *
 *  \author maarten.weyn@uantwerpen.be
 *
 */


#include <stdbool.h>
#include "hwadc.h"
#include <assert.h>

#include "em_adc.h"
#include "em_cmu.h"

#include "platform.h"


void adc_init(ADC_Reference reference, uint16_t input)
{
	// Initialises clocks
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_ADC0, true);

	/* Base the ADC configuration on the default setup. */
	ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
	ADC_InitSingle_TypeDef sInit = ADC_INITSINGLE_DEFAULT;

	/* Initialize timebases */
	init.timebase = ADC_TimebaseCalc(0);
	init.prescale = ADC_PrescaleCalc(400000,0);
	ADC_Init(ADC0, &init);

	switch (reference)
	{
	/** Internal 1.25V reference. */
	case adcReference1V25:
		sInit.reference = adcRef1V25;
		break;

	/** Internal 2.5V reference. */
	case adcReference2V5:
		sInit.reference = adcRef2V5;
		break;

	/** Buffered VDD. */
	case adcReferenceVDD:
		sInit.reference = adcRefVDD;
		break;

	/** Internal differential 5V reference. */
	case adcReference5VDIFF:
		sInit.reference = adcRef5VDIFF;
		break;

	/** Single ended ext. ref. from 1 pin. */
	case adcReferenceExtSingle:
		sInit.reference = adcRefExtSingle;
		break;

	/** Differential ext. ref. from 2 pins */
	case adcReference2xExtDiff:
		sInit.reference = adcRef2xExtDiff;
		break;

	/** Unbuffered 2xVDD. */
	case adcReference2xVDD:
		sInit.reference = adcRef2xVDD;
		break;

	}
	/* Set input to temperature sensor. Reference must be 1.25V */

	sInit.input = input;
	ADC_InitSingle(ADC0, &sInit);

	/* Setup interrupt generation on completed conversion. */
	ADC_IntEnable(ADC0, ADC_IF_SINGLE);
}


void adc_start()
{
	ADC_Start(ADC0, adcStartSingle);
}

uint32_t adc_get_value()
{
	ADC_DataSingleGet(ADC0);
}

bool adc_ready()
{
	return ADC0->STATUS;
}

void adc_clear_interrupt()
{
	ADC_IntClear(ADC0, ADC_IFC_SINGLEOF);
}

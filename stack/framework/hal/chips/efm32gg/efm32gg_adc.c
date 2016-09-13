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
#include "log.h"

/***************************************************************************//**
 * @brief
 *   Calibrate offset and gain for the specified reference.
 *   Supports currently only single ended gain calibration.
 *   Could easily be expanded to support differential gain calibration.
 *
 * @details
 *   The offset calibration routine measures 0 V with the ADC, and adjust
 *   the calibration register until the converted value equals 0.
 *   The gain calibration routine needs an external reference voltage equal
 *   to the top value for the selected reference. For example if the 2.5 V
 *   reference is to be calibrated, the external supply must also equal 2.5V.
 *
 * @param[in] adc
 *   Pointer to ADC peripheral register block.
 *
 * @param[in] ref
 *   Reference used during calibration. Can be both external and internal
 *   references.
 *
 * @return
 *   The final value of the calibration register, note that the calibration
 *   register gets updated with this value during the calibration.
 *   No need to load the calibration values after the function returns.
 ******************************************************************************/
uint32_t ADC_Calibration(ADC_TypeDef *adc, ADC_Ref_TypeDef ref)
{
  int32_t  sample;
  uint32_t cal;

  /* Binary search variables */
  uint8_t high;
  uint8_t mid;
  uint8_t low;

  /* Reset ADC to be sure we have default settings and wait for ongoing */
  /* conversions to be complete. */
  ADC_Reset(adc);

  ADC_Init_TypeDef       init       = ADC_INIT_DEFAULT;
  ADC_InitSingle_TypeDef singleInit = ADC_INITSINGLE_DEFAULT;

  /* Init common settings for both single conversion and scan mode */
  init.timebase = ADC_TimebaseCalc(0);
  /* Might as well finish conversion as quickly as possibly since polling */
  /* for completion. */
  /* Set ADC clock to 7 MHz, use default HFPERCLK */
  init.prescale = ADC_PrescaleCalc(7000000, 0);

  /* Set an oversampling rate for more accuracy */
  init.ovsRateSel = adcOvsRateSel4096;
  /* Leave other settings at default values */
  ADC_Init(adc, &init);

  /* Init for single conversion use, measure diff 0 with selected reference. */
  singleInit.reference = ref;
  singleInit.input     = adcSingleInpDiff0;
  singleInit.acqTime   = adcAcqTime16;
  singleInit.diff      = true;
  /* Enable oversampling rate */
  singleInit.resolution = adcResOVS;

  ADC_InitSingle(adc, &singleInit);

  /* ADC is now set up for offset calibration */
  /* Offset calibration register is a 7 bit signed 2's complement value. */
  /* Use unsigned indexes for binary search, and convert when calibration */
  /* register is written to. */
  high = 128;
  low  = 0;

  /* Do binary search for offset calibration*/
  while (low < high)
  {
    /* Calculate midpoint */
    mid = low + (high - low) / 2;

    /* Midpoint is converted to 2's complement and written to both scan and */
    /* single calibration registers */
    cal      = adc->CAL & ~(_ADC_CAL_SINGLEOFFSET_MASK | _ADC_CAL_SCANOFFSET_MASK);
    cal     |= (mid - 63) << _ADC_CAL_SINGLEOFFSET_SHIFT;
    cal     |= (mid - 63) << _ADC_CAL_SCANOFFSET_SHIFT;
    adc->CAL = cal;

    /* Do a conversion */
    ADC_Start(adc, adcStartSingle);

    /* Wait while conversion is active */
    while (adc->STATUS & ADC_STATUS_SINGLEACT) ;

    /* Get ADC result */
    sample = ADC_DataSingleGet(adc);

    /* Check result and decide in which part of to repeat search */
    /* Calibration register has negative effect on result */
    if (sample < 0)
    {
      /* Repeat search in bottom half. */
      high = mid;
    }
    else if (sample > 0)
    {
      /* Repeat search in top half. */
      low = mid + 1;
    }
    else
    {
      /* Found it, exit while loop */
      break;
    }
  }

  /* Now do gain calibration, only input and diff settings needs to be changed */
  adc->SINGLECTRL &= ~(_ADC_SINGLECTRL_INPUTSEL_MASK | _ADC_SINGLECTRL_DIFF_MASK);
  adc->SINGLECTRL |= (adcSingleInpCh4 << _ADC_SINGLECTRL_INPUTSEL_SHIFT);
  adc->SINGLECTRL |= (false << _ADC_SINGLECTRL_DIFF_SHIFT);

  /* ADC is now set up for gain calibration */
  /* Gain calibration register is a 7 bit unsigned value. */

  high = 128;
  low  = 0;

  /* Do binary search for gain calibration */
  while (low < high)
  {
    /* Calculate midpoint and write to calibration register */
    mid = low + (high - low) / 2;

    /* Midpoint is converted to 2's complement */
    cal      = adc->CAL & ~(_ADC_CAL_SINGLEGAIN_MASK | _ADC_CAL_SCANGAIN_MASK);
    cal     |= mid << _ADC_CAL_SINGLEGAIN_SHIFT;
    cal     |= mid << _ADC_CAL_SCANGAIN_SHIFT;
    adc->CAL = cal;

    /* Do a conversion */
    ADC_Start(adc, adcStartSingle);

    /* Wait while conversion is active */
    while (adc->STATUS & ADC_STATUS_SINGLEACT) ;

    /* Get ADC result */
    sample = ADC_DataSingleGet(adc);

    /* Check result and decide in which part to repeat search */
    /* Compare with a value atleast one LSB's less than top to avoid overshooting */
    /* Since oversampling is used, the result is 16 bits, but a couple of lsb's */
    /* applies to the 12 bit result value, if 0xffe is the top value in 12 bit, this */
    /* is in turn 0xffe0 in the 16 bit result. */
    /* Calibration register has positive effect on result */
    if (sample > 0xffd0)
    {
      /* Repeat search in bottom half. */
      high = mid;
    }
    else if (sample < 0xffd0)
    {
      /* Repeat search in top half. */
      low = mid + 1;
    }
    else
    {
      /* Found it, exit while loop */
      break;
    }
  }

  return adc->CAL;
}

void adc_calibrate()
{
	// Initialises clocks
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_ADC0, true);

	//todo: use new calibration values
	//todo: make adaptable for other ref voltages
	uint32_t old_gain_calibration_value =
	(DEVINFO->ADC0CAL0 & _DEVINFO_ADC0CAL0_1V25_GAIN_MASK)
	>> _DEVINFO_ADC0CAL0_1V25_GAIN_SHIFT;

	uint32_t old_offset_calibration_value =
	(DEVINFO->ADC0CAL0 & _DEVINFO_ADC0CAL0_1V25_OFFSET_MASK)
	>> _DEVINFO_ADC0CAL0_1V25_OFFSET_SHIFT;

	//RTCDRV_Trigger(100, NULL);

	uint32_t calibration_value = ADC_Calibration(ADC0, adcRef1V25);

	uint32_t offset_calibration_value = (calibration_value & _ADC_CAL_SINGLEOFFSET_MASK) >> _ADC_CAL_SINGLEOFFSET_SHIFT;
	uint32_t gain_calibration_value   = (calibration_value & _ADC_CAL_SINGLEGAIN_MASK) >> _ADC_CAL_SINGLEGAIN_SHIFT;
	log_print_stack_string(LOG_STACK_FWK, "ADC Calibration offset %d -> %d", old_offset_calibration_value, offset_calibration_value);
	log_print_stack_string(LOG_STACK_FWK, "ADC Calibration gain %d -> %d", old_gain_calibration_value, gain_calibration_value);
}


void adc_init(ADC_Reference reference, ADC_Input input, uint32_t adc_frequency)
{
	// Initialises clocks
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_ADC0, true);

	/* Base the ADC configuration on the default setup. */
	ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
	//adcWarmupKeepADCWarm?
	ADC_InitSingle_TypeDef sInit = ADC_INITSINGLE_DEFAULT;

	/* Initialize timebases */
	init.timebase = ADC_TimebaseCalc(0);
	init.prescale = ADC_PrescaleCalc(adc_frequency,0);
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

	switch (input)
	{
		/** Temperature reference. */
	case adcInputSingleTemp:
		sInit.input = adcSingleInpTemp;
		break;
	/** VDD / 3. */
	case adcInputSingleVDDDiv3:
		sInit.input = adcSingleInpVDDDiv3;
		break;
		/** Positive Ch4, negative Ch5. */
	case adcInputSingleCh4Ch5:
		sInit.input = adcSingleInpCh4Ch5;
		sInit.diff = true;
		break;
  case adcInputSingleInputCh2:
    sInit.input = adcSingleInputCh2;
    break;
  case adcInputSingleInputCh6:
    sInit.input = adcSingleInputCh6;
    break;
	}

	ADC_InitSingle(ADC0, &sInit);

	/* Setup interrupt generation on completed conversion. */
  ADC_IntEnable(ADC0, ADC_IF_SINGLE);
  NVIC_EnableIRQ(ADC0_IRQn);
}

void adc_start()
{
	ADC_Start(ADC0, adcStartSingle);
}

uint32_t adc_get_value()
{
	return ADC_DataSingleGet(ADC0);
}

uint32_t adc_read_single( void )
{
  ADC_Start(ADC0, adcStartSingle);
  while ( ( ADC0->STATUS & ADC_STATUS_SINGLEDV ) == 0 ){}
  return ADC_DataSingleGet(ADC0);
}


bool adc_ready()
{
	return ADC0->STATUS;
}

void adc_clear_interrupt()
{
  ADC_IntClear(ADC0, ADC_IF_SINGLE);
}


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

/*! \file hwadc.h
 * \addtogroup ADC
 * \ingroup HAL
 * @{
 * \brief Provides an API for controlling ADC
 * \author maarten.weyn@uantwerpen.be
 */

#ifndef __ADC_H__
#define __ADC_H__

#include "platform.h"

#include "types.h"
#include "link_c.h"

// TODO refactor: is now tied to EFM32, either make more generic or just use emlib directly

typedef enum
{
  /** Internal 1.25V reference. */
  adcReference1V25,
  /** Internal 2.5V reference. */
  adcReference2V5 ,
  /** Buffered VDD. */
  adcReferenceVDD ,
  /** Internal differential 5V reference. */
  adcReference5VDIFF ,
  /** Single ended ext. ref. from 1 pin. */
  adcReferenceExtSingle,
  /** Differential ext. ref. from 2 pins */
  adcReference2xExtDiff,
  /** Unbuffered 2xVDD. */
  adcReference2xVDD
} ADC_Reference;

typedef enum
{
	/** Temperature reference. */
	adcInputSingleTemp,
	 /** VDD / 3 */
	adcInputSingleVDDDiv3,
	/** Positive Ch4, negative Ch5. */
        adcInputSingleCh4Ch5,
        adcInputSingleInputCh2,
        adcInputSingleInputCh6
} ADC_Input;


/*! \brief Initialises the ADC
 * 	\param reference selects the reference voltage used by the ADC
 * 	\param input selects the input used by the ADC
 */
__LINK_C void adc_init(ADC_Reference reference, ADC_Input input, uint32_t adc_frequency);

/*! \brief Calibrates the ADC
 */
__LINK_C void adc_calibrate();

/*! \brief Starts a single ADC measurements
 */
__LINK_C void adc_start();

/*! \brief Gets the ADC value
 * 	\return ADC register value
 */
__LINK_C uint32_t adc_get_value();

/*! \brief Checks ADC status flag
 * 	\return value of status flag
 */
__LINK_C bool adc_ready();

/*! \brief Starts a single ADC measurement and gets the value
 * 	\return ADC register value
 */
__LINK_C uint32_t adc_read_single();

/*! \brief Clears the ADC interrupt flag
 */
__LINK_C void adc_clear_interrupt();

#endif // __ADC_H__

/** @}*/

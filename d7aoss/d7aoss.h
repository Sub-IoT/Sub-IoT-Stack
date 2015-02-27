/*! \file d7aoss.h
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * \author maarten.weyn@uantwerpen.be
 * \author glenn.ergeerts@uantwerpen.be
 *
 *	\brief High Level API for OSS-7
 *
 *  The high level API to be used by applications which use the dash7 stack
 */

#ifndef D7STACK_H_
#define D7STACK_H_

#define UART // Uncomment if you want to use logging or uart  (6k code size increase)
#ifdef UART
	// Set Logging options
	#define LOG_PHY_ENABLED
	#define LOG_DLL_ENABLED
	#define LOG_NWL_ENABLED
	#define LOG_TRANS_ENABLED
	#define LOG_FWK_ENABLED
#endif

//#define BUTTONS // Uncomment if you want to use buttons (100 bytes increase)
//#define DEBUG

// Select the correct platform if necesarry
#define PLATFORM_WIZZIMOTE
//#define PLATFORM_AGAIDI
//#define PLATFORM_ARTESIS

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  Configuration section
 *
 *  \todo Get from configure step or similar
 */



#include "phy/phy.h"
#include "dll/dll.h"
#include "nwl/nwl.h"
#include "trans/trans.h"
#include "alp/alp.h"
#include "dae/fs.h"
#include "hal/system.h"

/*! \brief Initializes the stack and HAL
 *
 *  \param uint8_t* tx_buffer 		A pointer to a data array which can be used for the TX Queue
 *  \param uint8_t 	tx_buffer_size 	The size of the TX Buffer
 *  \param uint8_t* rx_buffer 		A pointer to a data array which can be used for the RX Queue
 *  \paral uint8_t*	rx_buffer_size	The size of the RX Buffer
 */
void d7aoss_init(uint8_t* tx_buffer, uint16_t tx_buffer_size, uint8_t* rx_buffer, uint16_t rx_buffer_size);


#ifdef __cplusplus
extern }
#endif

#endif /* D7STACK_H_ */

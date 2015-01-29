/*! \file d7aoss.h
 *
 * \copyright (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.\n
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.\n
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
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

//#define UART // Uncomment if you want to use logging or uart  (6k code size increase)
#ifdef UART
	// Set Logging options
	//#define LOG_PHY_ENABLED
	//#define LOG_DLL_ENABLED
	//#define LOG_NWL_ENABLED
	//#define LOG_TRANS_ENABLED
	//#define LOG_FWK_ENABLED
#endif

//#define BUTTONS // Uncomment if you want to use buttons (100 bytes increase)
//#define DEBUG

// Select the correct platform if necesarry
//#define PLATFORM_WIZZIMOTE
//#define PLATFORM_AGAIDI
#define PLATFORM_ARTESIS

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

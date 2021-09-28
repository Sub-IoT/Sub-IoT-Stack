/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
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
 */

/*
 *
 *  Authors:
 * 		maarten.weyn@uantwerpen.be
 */

#ifndef SI4455_INTERFACE_H
#define SI4455_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "ecode.h"

#include "hwradio.h"

// Error code definitions of EZRadioDrv
#define ECODE_EMDRV_EZRADIODRV_OK                       ( ECODE_OK )                                   ///< Success return value.
#define ECODE_EMDRV_EZRADIODRV_ILLEGAL_HANDLE           ( ECODE_EMDRV_EZRADIODRV_BASE | 0x00000001 )   ///< Illegal SPI handle.

#define ECODE_EMDRV_EZRADIODRV_TRANSMIT_PLUGIN_BASE             ( ECODE_EMDRV_EZRADIODRV_BASE | 0x00000100 )   ///< Transmit plugin base error code.
#define ECODE_EMDRV_EZRADIODRV_RECEIVE_PLUGIN_BASE              ( ECODE_EMDRV_EZRADIODRV_BASE | 0x00000200 )   ///< Receive plugin base error code.
#define ECODE_EMDRV_EZRADIODRV_CRC_ERROR_PLUGIN_BASE            ( ECODE_EMDRV_EZRADIODRV_BASE | 0x00000300 )   ///< CRC error plugin base error code.
#define ECODE_EMDRV_EZRADIODRV_AUTO_ACK_PLUGIN_BASE             ( ECODE_EMDRV_EZRADIODRV_BASE | 0x00000400 )   ///< Receive plugin base error code.
#define ECODE_EMDRV_EZRADIODRV_UNMOD_CARRIER_PLUGIN_BASE        ( ECODE_EMDRV_EZRADIODRV_BASE | 0x00000500 )   ///< Receive plugin base error code.
#define ECODE_EMDRV_EZRADIODRV_PN9_PLUGIN_BASE                  ( ECODE_EMDRV_EZRADIODRV_BASE | 0x00000600 )   ///< Receive plugin base error code.
#define ECODE_EMDRV_EZRADIODRV_DIRECT_TRANSMIT_PLUGIN_BASE      ( ECODE_EMDRV_EZRADIODRV_BASE | 0x00000700 )   ///< Transmit plugin base error code.
#define ECODE_EMDRV_EZRADIODRV_DIRECT_RECEIVE_PLUGIN_BASE       ( ECODE_EMDRV_EZRADIODRV_BASE | 0x00000800 )   ///< Receive plugin base error code.

// Transmit plugin related error codes
#define ECODE_EMDRV_EZRADIODRV_TRANSMIT_FAILED          		( ECODE_EMDRV_EZRADIODRV_TRANSMIT_PLUGIN_BASE | 0x00000001 )   ///< Unable to start transmission.


typedef void (*ezradio_int_callback_t)();

void ezradioInit(ezradio_int_callback_t cb);
void ezradioResetTRxFifo(void);
Ecode_t ezradioStartRx(uint8_t channel, bool packet_handler);
Ecode_t ezradioStartTx(hw_radio_packet_t* packet, uint8_t channel_id, bool rx_after, uint8_t data_length);
Ecode_t ezradioStartTxUnmodelated(uint8_t channel_id);

const char *byte_to_binary(uint8_t x);


#ifdef __cplusplus
}
#endif

#endif /* SI4455_INTERFACE_H */

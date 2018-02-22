/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2017 University of Antwerp
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

#include "types.h"
#include "platform_defs.h"

/*!
 * \file phy.h
 * \addtogroup phy
 * \ingroup framework
 * @{
 * \brief The PHY module provides PHY functions not specific to one chip
 * \author glenn.ergeerts@uantwerpen.be
 */

#ifndef _PHY_H_
#define _PHY_H_

/* \brief The channel bands and corresponding band indices as defined in D7A
 *
 */
typedef enum
{
    PHY_BAND_433 = 0x02,
    PHY_BAND_868 = 0x03,
    PHY_BAND_915 = 0x04,
} phy_channel_band_t;

/* \brief The channel classes and corresponding indices as defined in D7A
 *
 */
typedef enum
{
    PHY_CLASS_LO_RATE = 0x00, // 9.6 kbps
#ifdef USE_SX127X
    PHY_CLASS_LORA = 0x01, // LoRa SF9, BW 125, CR 4/5. Note this is _not_ part of D7A spec (for now), and subject to change (or removal)
#endif
    PHY_CLASS_NORMAL_RATE = 0x02, // 55.555 kbps
    PHY_CLASS_HI_RATE = 0x03 // 166.667 kbps
} phy_channel_class_t;

/* \brief The coding schemes and corresponding indices as defined in D7A
 *
 */
typedef enum
{
    PHY_CODING_PN9 = 0x00,
    PHY_CODING_RFU = 0x01,
    PHY_CODING_FEC_PN9 = 0x02
} phy_coding_t;

/* \brief The syncword classes as defined in D7AP
 *
 */
typedef enum
{
    PHY_SYNCWORD_CLASS0 = 0x00,
    PHY_SYNCWORD_CLASS1 = 0x01
} phy_syncword_class_t;

typedef enum
{
    PREAMBLE_LOW_RATE_CLASS = 4, //(4 bytes, 32 bits)
    PREAMBLE_NORMAL_RATE_CLASS = 4, //(4 bytes, 32 bits)
    PREAMBLE_HI_RATE_CLASS = 6, //(6 bytes, 48 bits)
} phy_preamble_min_length_t;

/* \brief The channel header as defined in D7AP
 */
typedef struct
{
    phy_coding_t ch_coding: 2; 	/**< The 'coding' field in the channel header */
    phy_channel_class_t ch_class: 2;  	/**< The 'class' field in the channel header */
    phy_channel_band_t ch_freq_band: 3;	/**< The frequency 'band' field in the channel header */
    uint8_t _rfu: 1;
} phy_channel_header_t;

/** \brief channel id used to identify the spectrum settings
 *
 * This struct adheres to the 'Channel ID' format the Dash7 PHY layer. (@17/03/2015)
 */
typedef struct
{
    union
    {
        uint8_t channel_header_raw; 	/**< The raw (8-bit) channel header */
        phy_channel_header_t channel_header; /**< The channel header */
    };
    uint16_t center_freq_index;		/**< The center frequency index of the channel id */
} channel_id_t;

/* \brief Utility function to check whether two channel_id_t are equal
 *
 * \param a	The first channel_id
 * \param b	The second channel_id
 * \return bool	true if the two channel_id are equal, false otherwise.
 */
bool phy_radio_channel_ids_equal(const channel_id_t* a, const channel_id_t* b);

uint16_t phy_calculate_tx_duration(phy_channel_class_t channel_class, phy_coding_t ch_coding, uint8_t packet_length, bool payload_only);

#endif //_PHY_H_

/** @}*/

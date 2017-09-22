/*! \file dae.h
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
 * \author glenn.ergeerts@uantwerpen.be
 *
 */

#ifndef DAE_H_
#define DAE_H_

#include "stdint.h"

#include "hwradio.h" // TODO for phy_channel_header_t in subband_t, refactor

#define SUBPROFILES_NB	4
#define SUBBANDS_NB		8

#define ACCESS_SPECIFIER(val) (uint8_t)(val >> 4 & 0x0F)
#define ACCESS_MASK(val) (uint8_t)(val & 0x0F)

typedef enum
{
    CSMA_CA_MODE_UNC = 0,
    CSMA_CA_MODE_AIND = 1,
    CSMA_CA_MODE_RAIND = 2,
    CSMA_CA_MODE_RIGD = 3
} csma_ca_mode_t; // TODO move

typedef struct
{
    uint16_t channel_index_start;
    uint16_t channel_index_end;
    int8_t eirp;
    uint8_t cca;  // Default Clear channel assessment threshold (-dBm)
    uint8_t duty; // Maximum per-channel transmission duty cycle in per-mil (‰)
} subband_t;

typedef struct
{
    uint8_t subband_bitmap; // Bitmap of used subbands
    uint8_t scan_automation_period;
} subprofile_t;

typedef struct
{
    phy_channel_header_t channel_header;
    subprofile_t subprofiles[SUBPROFILES_NB];
    subband_t subbands[SUBBANDS_NB];
} dae_access_profile_t;

#endif /* DAE_H_ */

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

typedef enum
{
    CSMA_CA_MODE_UNC = 0,
    CSMA_CA_MODE_AIND = 1,
    CSMA_CA_MODE_RAIND = 2,
    CSMA_CA_MODE_RIGD = 3
} csma_ca_mode_t; // TODO move

typedef struct
{
    phy_channel_header_t channel_header;
    uint16_t channel_index_start;
    uint16_t channel_index_end;
    int8_t eirp;
    uint8_t ccao;
} subband_t; // TODO move?

typedef struct
{
    union
    {
        uint8_t control;
        struct
        {
            bool control_scan_type_is_foreground: 1;
            csma_ca_mode_t control_csma_ca_mode: 3;
            uint8_t control_number_of_subbands: 2;
        };
    };
    uint8_t subnet;
    uint8_t scan_automation_period;
    uint8_t transmission_timeout_period;  // Tc
    uint8_t _rfu;
    subband_t subbands[1]; // TODO only support 1 subband for now
} dae_access_profile_t;

#endif /* DAE_H_ */

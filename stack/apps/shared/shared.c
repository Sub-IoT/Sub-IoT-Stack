/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2018 University of Antwerp
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

/*
 * \author	glenn.ergeerts@uantwerpen.be
 */

#include "shared.h"

dae_access_profile_t default_access_profiles[DEFAULT_ACCESS_PROFILES_COUNT] = {
    {
        // AC used for pushing data to the GW, continuous FG scan
        .channel_header = {
            .ch_coding = PHY_CODING_FEC_PN9,
            .ch_class = PHY_CLASS_LO_RATE,
            .ch_freq_band = PHY_BAND_868
        },
        .subprofiles[0] = {
            .subband_bitmap = 0x01, // only the first subband is selectable
            .scan_automation_period = 0,
        },
        .subbands[0] = (subband_t){
            .channel_index_start = 0,
            .channel_index_end = 0,
            .eirp = 14,
            .cca = 86,
            .duty = 0,
        }
    },
    {
        // AC used by sensors for scanning for BG request every second
        .channel_header = {
            .ch_coding = PHY_CODING_FEC_PN9,
            .ch_class = PHY_CLASS_LO_RATE,
            .ch_freq_band = PHY_BAND_868
        },
        .subprofiles[0] = {
          .subband_bitmap = 0x01,
          .scan_automation_period = 112, // 1024 ticks
        },
        .subbands[0] = (subband_t){
            .channel_index_start = 0,
            .channel_index_end = 0,
            .eirp = 14,
            .cca = 86,
            .duty = 0,
        }
    },
    {
        // AC used by sensor, push only, no scanning
        .channel_header = {
            .ch_coding = PHY_CODING_FEC_PN9,
            .ch_class = PHY_CLASS_LO_RATE,
            .ch_freq_band = PHY_BAND_868
        },
        .subprofiles[0] = {
            .subband_bitmap = 0x00, // void scan automation channel list
            .scan_automation_period = 0,
        },
        .subbands[0] = (subband_t){
            .channel_index_start = 0,
            .channel_index_end = 0,
            .eirp = 14,
            .cca = 86,
            .duty = 0,
        }
    }
};

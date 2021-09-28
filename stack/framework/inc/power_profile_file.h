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

// Copyright Aloxy.io
// @authors: K. Schram
// @brief: Contains byte option data of the mcu

#ifndef __POWER_PROFILE_FILE_H
#define __POWER_PROFILE_FILE_H

// oss7
#include "d7ap_fs.h"
#include "framework_defs.h"
#include "timer.h"

// other
#include "errors.h"
#include "stdint.h"

#define POWER_PROFILE_FILE_ID   FRAMEWORK_POWER_PROFILE_FILE_ID

#ifdef FRAMEWORK_POWER_PROFILE_RF
#define POWER_PROFILE_FILE_SIZE 17
#else
#define POWER_PROFILE_FILE_SIZE 5
#endif // FRAMEWORK_POWER_PROFILE_RF

typedef enum
{
    POWER_PROFILE_LORA = 0,
    POWER_PROFILE_D7   = 1
} power_profile_transmit_mode_t;

typedef enum
{
    POWER_PROFILE_RADIO_TX      = 0,
    POWER_PROFILE_RADIO_RX      = 1,
    POWER_PROFILE_RADIO_STANDBY = 2,
    POWER_PROFILE_RADIO_SLEEP   = 3
} power_profile_radio_type_t;

typedef enum
{
    POWER_PROFILE_SLEEP   = 0,
    POWER_PROFILE_STOP    = 1,
    POWER_PROFILE_STANDBY = 2,
    POWER_PROFILE_ACTIVE  = 255
} power_profile_run_time_mode_t;

/**
 * @brief the option bytes file.
 */
typedef struct
{
    union
    {
        uint8_t bytes[POWER_PROFILE_FILE_SIZE];
        struct
        {
            timer_tick_t cpu_active_time;
            uint8_t boot_counter;
#ifdef FRAMEWORK_POWER_PROFILE_RF
            timer_tick_t temp_tx_time;
            timer_tick_t temp_rx_time;
            timer_tick_t temp_standby_time;
#endif // FRAMEWORK_POWER_PROFILE_RF
        } __attribute__((__packed__));
    };
} power_profile_file_t;

error_t power_profile_file_read(power_profile_file_t* power_profile_file);
error_t power_profile_register_run_time(timer_tick_t time);
#ifdef FRAMEWORK_POWER_PROFILE_RF
error_t power_profile_register_radio_action(power_profile_transmit_mode_t power_profile_transmit_mode,
    power_profile_radio_type_t type, timer_tick_t time, void* argument);
#endif // FRAMEWORK_POWER_PROFILE_RF
error_t power_profile_file_initialize();
error_t power_profile_persist_file();
void power_profile_file_toggle_persisting(bool persist);

#endif
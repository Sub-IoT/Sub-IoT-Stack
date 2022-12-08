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

#ifndef __POWER_TRACKING_FILE_H
#define __POWER_TRACKING_FILE_H

// oss7
#include "framework_defs.h"
#include "timer.h"

// other
#include "errors.h"
#include "stdint.h"

#define POWER_TRACKING_FILE_ID   FRAMEWORK_POWER_TRACKING_FILE_ID

#ifdef FRAMEWORK_POWER_TRACKING_RF
#define POWER_TRACKING_FILE_SIZE 17
#else
#define POWER_TRACKING_FILE_SIZE 5
#endif // FRAMEWORK_POWER_TRACKING_RF

typedef enum
{
    POWER_TRACKING_LORA = 0,
    POWER_TRACKING_D7   = 1
} power_tracking_transmit_mode_t;

typedef enum
{
    POWER_TRACKING_RADIO_TX      = 0,
    POWER_TRACKING_RADIO_RX      = 1,
    POWER_TRACKING_RADIO_STANDBY = 2,
    POWER_TRACKING_RADIO_SLEEP   = 3
} power_tracking_radio_type_t;

typedef enum
{
    POWER_TRACKING_SLEEP   = 0,
    POWER_TRACKING_STOP    = 1,
    POWER_TRACKING_STANDBY = 2,
    POWER_TRACKING_ACTIVE  = 255
} power_tracking_run_time_mode_t;

/**
 * @brief the option bytes file.
 */
typedef struct
{
    union
    {
        uint8_t bytes[POWER_TRACKING_FILE_SIZE];
        struct
        {
            timer_tick_t cpu_active_time;
            uint8_t boot_counter;
#ifdef FRAMEWORK_POWER_TRACKING_RF
            timer_tick_t temp_tx_time;
            timer_tick_t temp_rx_time;
            timer_tick_t temp_standby_time;
#endif // FRAMEWORK_POWER_TRACKING_RF
        } __attribute__((__packed__));
    };
} power_tracking_file_t;

error_t power_tracking_file_read(power_tracking_file_t* power_tracking_file);
error_t power_tracking_register_run_time(timer_tick_t time);
#ifdef FRAMEWORK_POWER_TRACKING_RF
error_t power_tracking_register_radio_action(power_tracking_transmit_mode_t power_tracking_transmit_mode,
    power_tracking_radio_type_t type, timer_tick_t time, void* argument);
#endif // FRAMEWORK_POWER_TRACKING_RF
error_t power_tracking_file_initialize();
error_t power_tracking_persist_file();
void power_tracking_file_toggle_persisting(bool persist);

#endif
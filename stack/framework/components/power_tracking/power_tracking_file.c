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
// @brief: Contains power consumption tracking

// application
#include "power_tracking_file.h"

// oss7
#include "d7ap_fs.h"
#include "debug.h"
#include "framework_defs.h"
#include "log.h"
#include "modules_defs.h"

// other
#include "errors.h"
#include "stdint.h"
#include "string.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_POWER_TRACKING_LOG_ENABLED)
#define DPRINT(...) log_print_string(__VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_DATA(...)
#endif

#ifndef MODULE_D7AP_FS
#error Module D7AP_FS is needed to use the power tracking framework
#endif

#define STORE_VALUE_DELTA (TIMER_TICKS_PER_MINUTE * 5) // 5 minutes
#define STORE_TIME_DELTA (TIMER_TICKS_PER_HOUR * 24) // 1 day

#define SECONDS_TILL_PERSIST 60

static power_tracking_file_t current_power_tracking_file;

static timer_tick_t cpu_active_time_prev_store_value;
static timer_tick_t last_store_time;

static bool persist_file = true;

static error_t power_tracking_file_write(power_tracking_file_t* power_tracking_file);

_Static_assert(POWER_TRACKING_FILE_SIZE == sizeof(power_tracking_file_t),
               "length define of power tracking file is not the same size as the define");

void power_tracking_persist_file_task() 
{
    power_tracking_persist_file();
}

error_t power_tracking_file_initialize()
{
    // perform check on file sizes (needed in order to have decent packaging using the bytes member of the file)
    d7ap_fs_file_header_t permanent_file_header = { .file_permissions
        = (file_permission_t) { .guest_read = true, .user_read = true }, // other permissions are default false
        .file_properties.storage_class = FS_STORAGE_PERMANENT,
        .length = POWER_TRACKING_FILE_SIZE,
        .allocated_length = 200 };
    // perform check on file sizes (needed in order to have decent packaging using the bytes member of the file)
    assert(permanent_file_header.allocated_length >= POWER_TRACKING_FILE_SIZE);

    error_t ret = d7ap_fs_init_file(POWER_TRACKING_FILE_ID, &permanent_file_header, NULL);
    switch (ret) {
    case -EEXIST:
    {
        uint32_t length = POWER_TRACKING_FILE_SIZE;
        d7ap_fs_read_file(POWER_TRACKING_FILE_ID, 0, current_power_tracking_file.bytes, &length, ROOT_AUTH);
        cpu_active_time_prev_store_value = current_power_tracking_file.cpu_active_time;
        break;
    }
    case SUCCESS:
        break;
    default:
        log_print_error_string("Error initialization of power tracking file: %d", ret);
        return ret;
    }
    sched_register_task((task_t)&power_tracking_persist_file_task);

    current_power_tracking_file.boot_counter++;

    sched_post_task((task_t)&power_tracking_persist_file_task);
    
    return SUCCESS;
}

error_t power_tracking_file_read(power_tracking_file_t* power_tracking_file)
{
    memcpy(power_tracking_file->bytes, current_power_tracking_file.bytes, POWER_TRACKING_FILE_SIZE);
    return SUCCESS;
}

error_t power_tracking_file_write(power_tracking_file_t* power_tracking_file)
{
    return d7ap_fs_write_file(POWER_TRACKING_FILE_ID, 0, power_tracking_file->bytes, POWER_TRACKING_FILE_SIZE, ROOT_AUTH);
}

void power_tracking_file_toggle_persisting(bool persist) { persist_file = persist; }

error_t power_tracking_persist_file()
{
    // don't persist the file if the application doesn't want any file writes at the moment
    if(!persist_file)
        return -EINTR;
#ifdef FRAMEWORK_POWER_TRACKING_RF
    DPRINT("persisting power tracking file with %i active, %i boots, %i tx, %i rx, %i standby",
        current_power_tracking_file.cpu_active_time, current_power_tracking_file.boot_counter,
        current_power_tracking_file.temp_tx_time, current_power_tracking_file.temp_rx_time,
        current_power_tracking_file.temp_standby_time);
#else
    DPRINT("persisting power tracking file with %i active, %i boots",
        current_power_tracking_file.cpu_active_time, current_power_tracking_file.boot_counter);
#endif // FRAMEWORK_POWER_TRACKING_RF
    DPRINT_DATA(current_power_tracking_file.bytes, POWER_TRACKING_FILE_SIZE);
    cpu_active_time_prev_store_value = current_power_tracking_file.cpu_active_time;
    last_store_time = timer_get_counter_value();
    return power_tracking_file_write(&current_power_tracking_file);
}

#ifdef FRAMEWORK_POWER_TRACKING_RF
error_t power_tracking_register_radio_action(power_tracking_transmit_mode_t power_tracking_transmit_mode,
    power_tracking_radio_type_t type, timer_tick_t time, void* argument)
{
    switch (type) {
    case POWER_TRACKING_RADIO_TX:
        current_power_tracking_file.temp_tx_time += time;
        int8_t power = *((int8_t*)argument);
        break;
    case POWER_TRACKING_RADIO_RX:
        current_power_tracking_file.temp_rx_time += time;
        break;
    case POWER_TRACKING_RADIO_STANDBY:
        current_power_tracking_file.temp_standby_time += time;
        break;
    }
    return SUCCESS;
}
#endif // FRAMEWORK_POWER_TRACKING_RF

error_t power_tracking_register_run_time(timer_tick_t time)
{
    current_power_tracking_file.cpu_active_time += time;
    if(timer_calculate_difference(cpu_active_time_prev_store_value, current_power_tracking_file.cpu_active_time) > STORE_VALUE_DELTA
    || timer_calculate_difference(last_store_time, timer_get_counter_value()) > STORE_TIME_DELTA)
    {
        //This function is always called just before the scheduler goes to sleep so scheduling this function will trigger a wake-up so call this function directly
        power_tracking_persist_file();
    }
    return SUCCESS;
}

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
 
#ifndef __ERROR_EVENT_FILE_H_
#define __ERROR_EVENT_FILE_H_


#include "errors.h"

#define ERROR_EVENT_FILE_ID   52
#define ERROR_EVENT_DATA_SIZE 30
#define ERROR_EVENT_SIZE      (ERROR_EVENT_DATA_SIZE + 2)
#define ERROR_HEADER_SIZE     4

typedef enum __attribute__((__packed__))
{
    WATCHDOG_EVENT,
    ASSERT_EVENT,
    LOG_EVENT
}error_event_type_t;

typedef error_t (*low_level_read_cb_t)(uint32_t address, uint8_t *data, uint8_t size);
typedef error_t (*low_level_write_cb_t)(uint32_t address, const uint8_t *data, uint8_t size);

error_t error_event_file_init(low_level_read_cb_t read_cb, low_level_write_cb_t write_cb);
error_t error_event_file_log_event(error_event_type_t event, uint8_t* event_data, uint8_t event_data_size);
bool error_event_file_has_event();
void error_event_file_reset(uint8_t file_id);
error_t error_event_get_file_with_latest_event_only(uint8_t* data, uint32_t* length);
void error_event_file_print();
error_t error_event_create_watchdog_event();
error_t error_event_create_assert_event();

// Below functions are not implemented by the error even file
// but can be used as function definition for the low level callbacks.
error_t low_level_read_cb(uint32_t address, uint8_t *data, uint8_t size);
error_t low_level_write_cb(uint32_t address, const uint8_t *data, uint8_t size);
#endif // __ERROR_EVENT_FILE_H_

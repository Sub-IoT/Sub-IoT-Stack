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

#include "bootstrap.h"
#include "hwgpio.h"
#include "hwleds.h"
#include "hwsystem.h"
#include "debug.h"
#include "hwdebug.h"
#include "hwradio.h"
#include "hwuart.h"
#include "errors.h"
#include "error_event_file.h"
#include "blockdevice_ram.h"
#include "framework_defs.h"

#define METADATA_SIZE (4 + 4 + (12 * FRAMEWORK_FS_FILE_COUNT))

// on native we use a RAM blockdevice as NVM as well for now
uint8_t d7ap_fs_metadata[METADATA_SIZE];
uint8_t d7ap_files_data[FRAMEWORK_FS_PERMANENT_STORAGE_SIZE];
uint8_t d7ap_volatile_files_data[FRAMEWORK_FS_VOLATILE_STORAGE_SIZE];

static blockdevice_ram_t metadata_bd = (blockdevice_ram_t){
    .base.driver = &blockdevice_driver_ram,
    .base.size = METADATA_SIZE,
    .buffer = d7ap_fs_metadata
};

static blockdevice_ram_t permanent_bd = (blockdevice_ram_t){
    .base.driver = &blockdevice_driver_ram,
    .base.size = FRAMEWORK_FS_PERMANENT_STORAGE_SIZE,
    .buffer = d7ap_files_data
};

static blockdevice_ram_t volatile_bd = (blockdevice_ram_t){
    .base.driver = &blockdevice_driver_ram,
    .base.size = FRAMEWORK_FS_VOLATILE_STORAGE_SIZE,
    .buffer = d7ap_volatile_files_data
};

blockdevice_t * const metadata_blockdevice = (blockdevice_t* const) &metadata_bd;
blockdevice_t * const persistent_files_blockdevice = (blockdevice_t* const) &permanent_bd;
blockdevice_t * const volatile_blockdevice = (blockdevice_t* const) &volatile_bd;


void __platform_init()
{
    blockdevice_init(metadata_blockdevice);
    blockdevice_init(persistent_files_blockdevice);
    blockdevice_init(volatile_blockdevice);
}

void __platform_post_framework_init()
{
}

error_t low_level_read_cb(uint32_t address, uint8_t *data, uint8_t size)
{
    return blockdevice_driver_ram.read(persistent_files_blockdevice, data, address, size);
}

error_t low_level_write_cb(uint32_t address, const uint8_t *data, uint8_t size)
{
    return blockdevice_driver_ram.program(persistent_files_blockdevice, data, address, size);
}

int main()
{
    //initialise the platform itself
    __platform_init();
    //do not initialise the scheduler, this is done by __framework_bootstrap()
    __framework_bootstrap();
    //initialise platform functionality that depends on the framework
    __platform_post_framework_init();

    scheduler_run();
    return 0;
}

// empty stubs
__LINK_C uart_handle_t* uart_init(uint8_t port_idx, uint32_t baudrate, uint8_t pins) {}
__LINK_C bool uart_enable(uart_handle_t* uart) {}
__LINK_C bool uart_disable(uart_handle_t* uart) {}
__LINK_C void uart_send_bytes(uart_handle_t* uart, void const *data, size_t length) {}
__LINK_C error_t uart_rx_interrupt_enable(uart_handle_t* uart) {}
__LINK_C void uart_set_rx_interrupt_callback(uart_handle_t* uart, uart_rx_inthandler_t rx_handler) {}
__LINK_C void uart_set_error_callback(uart_handle_t* uart, uart_error_handler_t error_handler) {}
__LINK_C error_t hw_gpio_set(pin_id_t pin_id) {}
system_reboot_reason_t hw_system_reboot_reason(void) {}
__LINK_C void hw_enter_lowpower_mode(uint8_t mode) {}
__LINK_C hwtimer_tick_t hw_timer_getvalue(hwtimer_id_t timer_id) {}
__LINK_C const hwtimer_info_t* hw_timer_get_info(hwtimer_id_t timer_id) {}
__LINK_C error_t hw_timer_schedule(hwtimer_id_t timer_id, hwtimer_tick_t tick ) {}
__LINK_C error_t hw_timer_init(hwtimer_id_t timer_id, uint8_t frequency, timer_callback_t compare_callback, timer_callback_t overflow_callback) {}
__LINK_C bool hw_timer_is_overflow_pending(hwtimer_id_t id) {}
__LINK_C error_t hw_timer_cancel(hwtimer_id_t timer_id) {}
__LINK_C uint64_t hw_get_unique_id(void) { return 0xFFFFFFFFFFFFFF;}
__LINK_C void hw_watchdog_feed(void) {};
__LINK_C void __watchdog_init(void) {};
__LINK_C void hw_watchdog_get_timeout(void) {};

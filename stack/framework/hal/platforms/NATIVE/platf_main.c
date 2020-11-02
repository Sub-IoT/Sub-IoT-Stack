/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
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

#include <unistd.h>
#include "bootstrap.h"
#include "hwgpio.h"
#include "hwleds.h"
#include "hwsystem.h"
#include "debug.h"
#include "hwdebug.h"
#include "hwradio.h"
#include "hwuart.h"
#include "errors.h"
#include "blockdevice_ram.h"
#include "framework_defs.h"
#include "button.h"

#define METADATA_SIZE (4 + 4 + (12 * FRAMEWORK_FS_FILE_COUNT))

// on native we use a RAM blockdevice as NVM as well for now
extern uint8_t d7ap_fs_metadata[METADATA_SIZE];
extern uint8_t d7ap_files_data[FRAMEWORK_FS_PERMANENT_STORAGE_SIZE];
extern uint8_t d7ap_volatile_files_data[FRAMEWORK_FS_VOLATILE_STORAGE_SIZE];

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
    // The emulated radio library for native platform requires a unique address
    // per application instance, so we use the PID to provide this
    hwradio_set_addr(getpid());
}

void __platform_post_framework_init()
{
    __ubutton_init();
}

int main(void)
{
	__platform_init();
    //do not initialise the scheduler, this is done by __framework_bootstrap()
    __framework_bootstrap();
    //initialise platform functionality that depends on the framework
    __platform_post_framework_init();

    scheduler_run();
    return 0;
}

// empty stubs
__LINK_C error_t hw_gpio_set(pin_id_t pin_id) {}
system_reboot_reason_t hw_system_reboot_reason(void) {}

// We don't really support low power mode, but we can sleep this process until an "interrupt"
// arises.  We use the timer tick or radio IRQ to force a wake-up.
__LINK_C void hw_enter_lowpower_mode(uint8_t mode)
{
	hwradio_enter_low_power_mode();   // Informs radio module we are entering low power state
	hwtimer_tick_t tick = hw_timer_getvalue(0);
	while (hw_timer_getvalue(0) == tick && !hwradio_wakeup_from_lowpower_mode())
		usleep(10);
}

__LINK_C uint64_t hw_get_unique_id(void) { return 0xFFFFFFFFFFFFFF;}
__LINK_C void hw_watchdog_feed(void) {};
__LINK_C void __watchdog_init(void) {};
__LINK_C void hw_watchdog_get_timeout(void) {};

void hw_reset() {
	printf("\n!!! hw_reset !!!");
	for (;;) usleep(1000);
}

void hw_busy_wait(int16_t us) { usleep(us); }

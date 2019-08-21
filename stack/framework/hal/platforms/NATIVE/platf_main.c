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

#include "bootstrap.h"
#include "hwgpio.h"
#include "hwleds.h"
#include "hwsystem.h"
#include "debug.h"
#include "hwdebug.h"
#include "hwradio.h"
#include "errors.h"
#include "hwuart.h"
#include "platform.h"


void __platform_init()
{
}

void __platform_post_framework_init()
{
}

//int main()
//{
//    //initialise the platform itself
//    __platform_init();
//    //do not initialise the scheduler, this is done by __framework_bootstrap()
//    __framework_bootstrap();
//    //initialise platform functionality that depends on the framework
//    __platform_post_framework_init();

//    scheduler_run();
//    return 0;
//}

// empty stubs
__LINK_C uart_handle_t* uart_init(uint8_t port_idx, uint32_t baudrate, uint8_t pins) {}
__LINK_C bool uart_enable(uart_handle_t* uart) {}
__LINK_C bool uart_disable(uart_handle_t* uart) {}
__LINK_C void uart_send_bytes(uart_handle_t* uart, void const *data, size_t length) {}
__LINK_C error_t uart_rx_interrupt_enable(uart_handle_t* uart) {}
__LINK_C void uart_set_rx_interrupt_callback(uart_handle_t* uart, uart_rx_inthandler_t rx_handler) {}
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


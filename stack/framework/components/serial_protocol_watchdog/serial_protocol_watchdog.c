#include "serial_protocol_watchdog.h"

#include "debug.h"
#include "fifo.h"
#include "log.h"
#include "scheduler.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_SERIAL_PROTOCOL_WATCHDOG_LOG_ENABLED)
  #define DPRINT(...) log_print_string(__VA_ARGS__)
  #define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
  #define DPRINT(...)
  #define DPRINT_DATA(...)
#endif

#define PING_TIMEOUT (15 * TIMER_TICKS_PER_SEC) // Same as SERIAL_PROTOCOL_TIMEOUT, longer than the boot time of the other party
#define PING_DATA 0xCC

typedef struct 
{
    serial_protocol_handle_t* sp_handle;
    reset_function_t reset_function;
} priv_serial_protocol_watchdog_handle_t;

void timeout(void *arg)
{
    assert(arg!=NULL);
    priv_serial_protocol_watchdog_handle_t* phandle = (priv_serial_protocol_watchdog_handle_t*)arg;
    DPRINT("No ping response received. Reset!!!");
    phandle->reset_function();
}

void send_ping(void *arg)
{
    assert(arg!=NULL);
    priv_serial_protocol_watchdog_handle_t* phandle = (priv_serial_protocol_watchdog_handle_t*)arg;
    timer_post_task_prio(&timeout, timer_get_counter_value() + PING_TIMEOUT, DEFAULT_PRIORITY, 0, arg);
    uint8_t data = PING_DATA;
    phandle->sp_handle->driver->serial_protocol_transfer_bytes(phandle->sp_handle, &data, 1, SERIAL_MESSAGE_TYPE_PING_REQUEST);
}

void ping_response_handler(serial_protocol_handle_t* handle, serial_message_type_t type, fifo_t* cmd_fifo)
{
    if(type != SERIAL_MESSAGE_TYPE_PING_RESPONSE)
    {
        return;
    }
    if(fifo_get_size(cmd_fifo) == 1)
    {
        uint8_t data;
        fifo_pop(cmd_fifo, &data, 1);
        if(data == PING_DATA)
        {
            timer_cancel_task(&timeout);
        }
    }
}

void serial_protocol_watchdog_init(serial_protocol_watchdog_handle_t* spw_handle,
    serial_protocol_handle_t* sp_handle, reset_function_t reset_function, timer_tick_t check_interval)
{
    assert(sp_handle != NULL);
    assert(spw_handle != NULL);
    assert(reset_function != NULL);
    assert(check_interval > PING_TIMEOUT);

    priv_serial_protocol_watchdog_handle_t* phandle = (priv_serial_protocol_watchdog_handle_t*)spw_handle->priv_data;

    phandle->sp_handle = sp_handle;
    phandle->reset_function = reset_function;

    // The timer doesn't support multiple timers for the same function.
    // This functionality is needed if we want multiple instances of the serial_protocol_watchdog
    // Change the following register tasks to true if this functionality comes available
    sched_register_task_allow_multiple(send_ping, false);
    sched_register_task_allow_multiple(timeout, false);

    sp_handle->driver->serial_protocol_register_handler(sp_handle, &ping_response_handler, SERIAL_MESSAGE_TYPE_PING_RESPONSE);

    timer_post_task_prio(&send_ping, timer_get_counter_value() + check_interval, DEFAULT_PRIORITY, check_interval, phandle);
}


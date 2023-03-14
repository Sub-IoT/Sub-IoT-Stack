#ifndef SERIAL_PROTOCOL_WATCHDOG_H
#define SERIAL_PROTOCOL_WATCHDOG_H

#include "serial_protocol.h"
#include "timer.h"

typedef void (*reset_function_t)(void);

#define WATCHDOG_PRIV_BUF_SIZE (2*(sizeof(void *)/4))

typedef struct 
{
    uint32_t priv_data[WATCHDOG_PRIV_BUF_SIZE];
}serial_protocol_watchdog_handle_t;

void serial_protocol_watchdog_init(serial_protocol_watchdog_handle_t* spw_handle,
    serial_protocol_handle_t* handle, reset_function_t reset_function, timer_tick_t check_interval);

void start(serial_protocol_watchdog_handle_t* spw_handle);
void stop(serial_protocol_watchdog_handle_t* spw_handle);

#endif // SERIAL_PROTOCOL_WATCHDOG_H
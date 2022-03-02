#ifndef __NETWORK_MANAGER_H
#define __NETWORK_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef enum {
    NETWORK_MANAGER_IDLE = 0,
    NETWORK_MANAGER_READY = 1,
    NETWORK_MANAGER_TRANSMITTING = 2,
} network_state_t;

typedef void (*last_transmit_completed_callback)(bool success);

void network_manager_init(last_transmit_completed_callback last_transmit_completed_cb);
error_t transmit_file(uint8_t file_id, uint32_t offset, uint32_t length, uint8_t *data);
void get_network_quality(uint8_t* acks, uint8_t* nacks);
network_state_t get_network_manager_state();

#endif //__NETWORK_MANAGER_H
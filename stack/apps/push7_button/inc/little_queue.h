#ifndef __LITTLE_QUEUE_H
#define __LITTLE_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_QUEUE_ELEMENTS  20

typedef enum {
    LOW_PRIORITY = 0,
    NORMAL_PRIORITY = 1,
    TOP_PRIORITY = 2,
} queue_priority_t;

typedef enum {
    VERSION_FILE_QUEUE_ACTIVE = 0,
    BUTTON_FILE_QUEUE_ACTIVE = 1,
    HEARTBEAT_FILE_QUEUE_ACTIVE = 2,
} active_file_queue_t;

typedef struct
{
    uint8_t store_index;
    uint8_t active_index;
    queue_priority_t priority;
    button_file_t button_file[MAX_QUEUE_ELEMENTS];
} button_file_queue_t;

typedef struct
{
    uint8_t store_index;
    uint8_t active_index;
    queue_priority_t priority;
    heartbeat_file_t heartbeat_file[MAX_QUEUE_ELEMENTS];
} heartbeat_file_queue_t;

typedef struct
{
    uint8_t store_index;
    uint8_t active_index;
    queue_priority_t priority;
    version_file_t version_file[MAX_QUEUE_ELEMENTS];
} version_file_queue_t;

void little_queue_init();
void queue_add_file(button_file_t* button_file_content, heartbeat_file_t* heartbeat_file_content, version_file_t* version_file_content);

#endif //__LITTLE_QUEUE_H
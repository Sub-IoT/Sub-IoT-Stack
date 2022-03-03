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


void little_queue_init();
void queue_add_file(button_file_t* button_file_content);

#endif //__LITTLE_QUEUE_H
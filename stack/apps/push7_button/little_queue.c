#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hwlcd.h"
#include "hwleds.h"
#include "hwsystem.h"

#include "d7ap_fs.h"
#include "debug.h"
#include "log.h"
#include "scheduler.h"
#include "timer.h"

#include "alp_layer.h"
#include "d7ap.h"
#include "dae.h"

#include "platform.h"

#include "button.h"
#include "led.h"
#include "network_manager.h"
#include "file_definitions.h"
#include "little_queue.h"

//#define FRAMEWORK_LITTLE_QUEUE_LOG 1

#ifdef FRAMEWORK_LITTLE_QUEUE_LOG
#include "log.h"
    #define DPRINT(...)      log_print_string(__VA_ARGS__)
    #define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
    #define DPRINT(...)
    #define DPRINT_DATA(...)
#endif

uint8_t button_fifo_buffer[MAX_QUEUE_ELEMENTS * BUTTON_FILE_SIZE];
fifo_t button_file_fifo;

static uint8_t transmitted_file_id = 0;

static void queue_transmit_files();

static void queue_transmit_completed(bool success)
{
    switch(transmitted_file_id)
    {
        case BUTTON_FILE_ID:
            if(success)
                fifo_skip(&button_file_fifo,BUTTON_FILE_SIZE);
            DPRINT("button transmit completed");
            break;
    }
    //TODO add backoff if !success based on #transmits + add max number of retry
    if(fifo_get_size(&button_file_fifo) > 0)
        timer_post_task_delay(&queue_transmit_files, 500);
    else 
        led_flash_white();
}

static void queue_transmit_files()
{
    error_t ret;
    if(fifo_get_size(&button_file_fifo) > 0)
    {
        DPRINT("sending button file to network manager.");
        uint8_t button_file_buffer[BUTTON_FILE_SIZE];
        fifo_peek(&button_file_fifo, button_file_buffer,0,BUTTON_FILE_SIZE);
        ret = transmit_file(BUTTON_FILE_ID, 0, BUTTON_FILE_SIZE, button_file_buffer);
        if(ret == SUCCESS)
            transmitted_file_id = BUTTON_FILE_ID;
        else
            log_print_string("could not send button file to network manager");
        return;
    }
   
}

void queue_add_file(button_file_t* button_file_content)
{
    if(button_file_content)
    {
        error_t ret = fifo_put(&button_file_fifo, button_file_content->bytes, BUTTON_FILE_SIZE);
        if(ret!=SUCCESS)
            log_print_error_string("queue was full. Message not added"); //TODO replace last element with this one
        DPRINT("added button file to the queue.");
    }
    if(get_network_manager_state() == NETWORK_MANAGER_READY && !timer_is_task_scheduled(&queue_transmit_files))
        sched_post_task(&queue_transmit_files);
}

void little_queue_init()
{
    network_manager_init(&queue_transmit_completed);
    sched_register_task(&queue_transmit_files);
    fifo_init(&button_file_fifo, button_fifo_buffer, sizeof(button_fifo_buffer));
}
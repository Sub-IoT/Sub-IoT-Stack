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

button_file_queue_t button_file_queue;
heartbeat_file_queue_t heartbeat_file_queue;
version_file_queue_t version_file_queue;

static uint8_t transmitted_file_id = 0;

uint8_t* test[] = {(uint8_t*)&version_file_queue, (uint8_t*)&heartbeat_file_queue, (uint8_t*)&button_file_queue};

static void queue_transmit_files();
static void increase_rotate(uint8_t* number);
static void queue_transmit_completed(bool success);

static void increase_rotate(uint8_t* number)
{
    *number = (*number + 1) >= MAX_QUEUE_ELEMENTS ? 0 : *number + 1;
}

static bool overwriting_queue(uint8_t lower, uint8_t higher)
{
    lower = (lower + 1) >= MAX_QUEUE_ELEMENTS ? 0 : lower + 1;
    return lower == higher;
}

static void queue_transmit_completed(bool success)
{
    switch(transmitted_file_id)
    {
        case BUTTON_FILE_ID:
            if(success)
                increase_rotate(&button_file_queue.active_index);
            //TODO Difference between active index and store index mustn't increase more than 1 when active index increases
            DPRINT("button transmit completed: store index: %d, active index: %d ",button_file_queue.store_index,button_file_queue.active_index );
            break;
        case HEARTBEAT_FILE_ID:
            if(success)
                increase_rotate(&heartbeat_file_queue.active_index);
            DPRINT("heartbeat transmit completed: store index: %d, active index: %d ",heartbeat_file_queue.store_index,heartbeat_file_queue.active_index );
            break;
        case VERSION_FILE_ID:
            if(success)
                increase_rotate(&version_file_queue.active_index);
            DPRINT("version transmit completed. store index: %d, active index: %d ",version_file_queue.store_index, version_file_queue.active_index );
            break;
    }
    //TODO add backoff if !success based on #transmits
    //sched_post_task(&queue_transmit_files);
    timer_post_task_delay(&queue_transmit_files,200); //check first if something has been added to the queue since last transmit
}

static void queue_transmit_files()
{
    error_t ret;

    if(version_file_queue.store_index != version_file_queue.active_index)
    {
        DPRINT("sending version file to network manager. store index: %d, active index: %d ",version_file_queue.store_index, version_file_queue.active_index );
        ret = transmit_file(VERSION_FILE_ID, 0, VERSION_FILE_SIZE, (uint8_t*)&version_file_queue.version_file[version_file_queue.store_index]);
        if(ret == SUCCESS)
            transmitted_file_id = VERSION_FILE_ID;
        else
            log_print_string("could not send version file to network manager");
        return;
    }
    else if(button_file_queue.store_index != button_file_queue.active_index)
    {
        DPRINT("sending button file to network manager. store index: %d, active index: %d ",button_file_queue.store_index,button_file_queue.active_index );
        ret = transmit_file(BUTTON_FILE_ID, 0, BUTTON_FILE_SIZE, (uint8_t*)&button_file_queue.button_file[button_file_queue.store_index]);
        if(ret == SUCCESS)
            transmitted_file_id = BUTTON_FILE_ID;
        else
            log_print_string("could not send button file to network manager");
        return;
    }
    else if(heartbeat_file_queue.store_index != heartbeat_file_queue.active_index)
    {
        DPRINT("sending heartbeat file to network manager. store index: %d, active index: %d ",heartbeat_file_queue.store_index,heartbeat_file_queue.active_index );
        ret = transmit_file(HEARTBEAT_FILE_ID, 0, HEARTBEAT_FILE_SIZE, (uint8_t*)&heartbeat_file_queue.heartbeat_file[heartbeat_file_queue.store_index]);
        if(ret == SUCCESS)
            transmitted_file_id = HEARTBEAT_FILE_ID;
        else
            log_print_string("could not send heartbeat file to network manager");
        return;
    }
    else
        led_flash_white();
}

void queue_add_file(button_file_t* button_file_content, heartbeat_file_t* heartbeat_file_content, version_file_t* version_file_content)
{
    if(button_file_content)
    {
        button_file_queue.button_file[button_file_queue.store_index] = *button_file_content;
        //TODO ensure we don't overwrite by checking active index
        if(!overwriting_queue(button_file_queue.store_index,button_file_queue.active_index))
            increase_rotate(&button_file_queue.store_index);
        DPRINT("added button file to the queue. New store index: %d, active index: %d ",button_file_queue.store_index,button_file_queue.active_index );
    }
    if(heartbeat_file_content)
    {
        heartbeat_file_queue.heartbeat_file[heartbeat_file_queue.store_index] = *heartbeat_file_content;
        if(!overwriting_queue(heartbeat_file_queue.store_index,heartbeat_file_queue.active_index))
            increase_rotate(&heartbeat_file_queue.store_index);
        DPRINT("added heartbeat file to the queue. New store index: %d, active index: %d ",heartbeat_file_queue.store_index,heartbeat_file_queue.active_index );
    }
    if(version_file_content)
    {
        version_file_queue.version_file[version_file_queue.store_index] = *version_file_content;
        if(!overwriting_queue(version_file_queue.store_index,version_file_queue.active_index))
            increase_rotate(&version_file_queue.store_index);
        DPRINT("added version file to the queue. New store index: %d, active index: %d ",version_file_queue.store_index, version_file_queue.active_index );
    }
    if(get_network_manager_state() == NETWORK_MANAGER_READY)
        sched_post_task(&queue_transmit_files);
}

void little_queue_init()
{
    button_file_queue.store_index=0;
    button_file_queue.active_index=0;
    button_file_queue.priority=NORMAL_PRIORITY;

    heartbeat_file_queue.store_index=0;
    heartbeat_file_queue.active_index=0;
    heartbeat_file_queue.priority=LOW_PRIORITY;

    version_file_queue.store_index=0;
    version_file_queue.active_index=0;
    version_file_queue.priority=TOP_PRIORITY;

    network_manager_init(&queue_transmit_completed);
    sched_register_task(&queue_transmit_files);
}

//Sub-IoT
#include "d7ap_fs.h"
#include "error_event_file.h"
#include "fs.h"
#include "log.h"
#include "modules_defs.h"

//Other
#include "string.h"

#ifndef MODULE_D7AP_FS
#error Module D7AP_FS is needed to use the error_event_file
#endif

#define ERROR_EVENT_COUNT 4
#define ERROR_EVENT_FILE_SIZE (4 + (ERROR_EVENT_COUNT * (ERROR_EVENT_DATA_SIZE + 2)))
#define ERROR_EVENT_FILE_ALLOCATED_SIZE 152

typedef struct __attribute__((__packed__))
{
    error_event_type_t type;
    uint8_t counter;
    uint8_t data[ERROR_EVENT_DATA_SIZE];
} error_event_t;

typedef struct __attribute__((__packed__))
{
    uint8_t start_event_idx;
    uint8_t stop_event_idx;
    uint16_t RFU;
} error_event_file_header_t;

typedef struct
{
    union
    {
        uint8_t bytes[ERROR_EVENT_FILE_SIZE];
        struct
        {
            error_event_file_header_t header;
            error_event_t events[ERROR_EVENT_COUNT];
        };
    } __attribute__((__packed__));
    
} error_event_file_t;

_Static_assert(ERROR_EVENT_FILE_SIZE == sizeof(error_event_file_t),
               "length define of error event file is not the same size as the define");
_Static_assert(ERROR_EVENT_FILE_ALLOCATED_SIZE >= ERROR_EVENT_FILE_SIZE,
               "length define of error event file is not the same size as the define");

// To limmit the amount of events logged for the same issue,
// define here the part of each event type that is exactly the same for multiple occurrences of an issue.
// If the complete event data is the same for each occurrence,
//specify 0 as the complete event is already checked with another comparision. 

// TODO check lengths after implementation
static uint8_t event_reduced_compare_size[] = {
    4, // WATCHDOG_EVENT
    0, // ASSERT_EVENT
    4, // LOG_EVENT
};

static bool is_init_complete;
static uint32_t start_of_file;
static error_event_file_header_t header = {.start_event_idx = 0xFF, .stop_event_idx = 0xFF, .RFU=0};
static low_level_read_cb_t low_level_read_cb_function;
static low_level_write_cb_t low_level_write_cb_function;

error_t error_event_file_init(low_level_read_cb_t read_cb, low_level_write_cb_t write_cb)
{
    is_init_complete = false;
    if(read_cb == NULL || write_cb == NULL)
    {
        return -EINVAL;
    }
    low_level_read_cb_function = read_cb;
    low_level_write_cb_function = write_cb;
    d7ap_fs_file_header_t permanent_file_header = { .file_permissions
        = (file_permission_t) { .guest_read = true, .user_read = true, .guest_write = true, .user_write = true}, // other permissions are default false
        .file_properties.storage_class = FS_STORAGE_PERMANENT,
        .length = ERROR_EVENT_FILE_SIZE,
        .allocated_length = ERROR_EVENT_FILE_ALLOCATED_SIZE };
    error_t ret = d7ap_fs_init_file(ERROR_EVENT_FILE_ID, &permanent_file_header, NULL);
    switch (ret) {
    case -EEXIST:
    {
        uint32_t length = sizeof(error_event_file_header_t);
        ret = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, 0, (uint8_t*)&header, &length, ROOT_AUTH);
        {
            return ret;
        }
        break;
    }
    case SUCCESS:
        ret = d7ap_fs_write_file(ERROR_EVENT_FILE_ID, 0, (uint8_t*)&header, sizeof(error_event_file_header_t), ROOT_AUTH);
        if(ret)
        {
            return ret;
        }
        break;
    default:
        log_print_error_string("Error initialization of error event file: %d", ret);
        return ret;
    }
    start_of_file = fs_get_address(ERROR_EVENT_FILE_ID) + sizeof(d7ap_fs_file_header_t);

    // Check if file is valid
    if(!((header.start_event_idx == 0xFF && header.stop_event_idx == 0xFF ) || (header.start_event_idx < ERROR_EVENT_COUNT && header.stop_event_idx < ERROR_EVENT_COUNT)))
    {
        log_print_error_string("error event file pointers are invalid. start: %d stop: %d", header.start_event_idx, header.stop_event_idx);
        return ERROR;
    }
    is_init_complete = true;
    return SUCCESS;
}

static inline uint32_t get_address_of_event(uint8_t index)
{
    return start_of_file + sizeof(error_event_file_header_t) + (sizeof(error_event_t) * index);
}

static inline uint8_t get_pointer_to_next_event(uint8_t index)
{
    return (index + 1) % ERROR_EVENT_COUNT;
}

error_t error_event_file_log_event(error_event_type_t event_type, uint8_t* event_data, uint8_t event_data_size)
{
    if(!is_init_complete)
    {
        return ERROR;
    }
    uint8_t event_idx;
    error_event_t event;
    if(event_data_size > ERROR_EVENT_DATA_SIZE)
    {
        event_data_size = ERROR_EVENT_DATA_SIZE;
    }
    // If error file is not empty, check if event is already present
    if(header.start_event_idx != 0xFF)
    {
        event_idx = header.start_event_idx;
        uint8_t reduced_compare_match_count = 0;
        bool all_slots_in_use = get_pointer_to_next_event(header.stop_event_idx) == header.start_event_idx;
        do
        {
            if(low_level_read_cb_function(get_address_of_event(event_idx), (uint8_t*)&event, sizeof(error_event_t)) == SUCCESS)
            {
                if(event.type == event_type)
                {
                    // Check if event is exactly the same
                    if(memcmp(event.data, event_data, event_data_size) == 0)
                    {
                        // Only increment of counter is needed
                        if(event.counter < UINT8_MAX)
                        {
                            event.counter++;
                            low_level_write_cb_function(get_address_of_event(event_idx) + sizeof(uint8_t), (const uint8_t*)&event.counter, sizeof(uint8_t));
                        }
                        return SUCCESS;
                    }
                    else if(event_reduced_compare_size[event_type] > 0 && memcmp(event.data, event_data, event_reduced_compare_size[event_type]) == 0)
                    {
                        reduced_compare_match_count++;
                        if(reduced_compare_match_count >= 2)
                        {
                            // Already 2 events logged of this type
                            return ERROR;
                        }
                    }

                }
            }
            event_idx = get_pointer_to_next_event(event_idx);
        }
        while(event_idx != get_pointer_to_next_event(header.stop_event_idx));

        // Check if all slots are in use
        if(all_slots_in_use)
        {
            // Use oldest slot for new event
            event_idx = header.start_event_idx;
            header.start_event_idx = get_pointer_to_next_event(header.start_event_idx);
        }
        else
        {
            event_idx = get_pointer_to_next_event(header.stop_event_idx);
        }
    }
    else
    {
        header.start_event_idx = 0;
        event_idx = 0;
    }
    event.type = event_type;
    event.counter = 1;
    memcpy(event.data, event_data, event_data_size);
    low_level_write_cb_function(get_address_of_event(event_idx), (const uint8_t*)&event, event_data_size + 2);
    header.stop_event_idx = event_idx;
    low_level_write_cb_function(start_of_file, (const uint8_t*)&header, sizeof(error_event_file_header_t));
    return SUCCESS;
}

bool error_event_file_has_event()
{
    if(is_init_complete)
    {
        return header.start_event_idx != 0xFF;
    }
    return false;
}

error_t error_event_file_reset()
{
    if(is_init_complete)
    {
        header.start_event_idx = 0xFF;
        header.stop_event_idx = 0xFF;
        header.RFU = 0;
        return low_level_write_cb_function(start_of_file, (const uint8_t*)&header, sizeof(error_event_file_header_t));
    }
    return ERROR;
}
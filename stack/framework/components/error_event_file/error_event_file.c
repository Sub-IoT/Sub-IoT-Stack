
//Sub-IoT
#include "callstack.h"
#include "d7ap_fs.h"
#include "error_event_file.h"
#include "fs.h"
#include "log.h"
#include "modules_defs.h"
#include "scheduler.h"

//Other
#include "string.h"

#ifndef MODULE_D7AP_FS
#error Module D7AP_FS is needed to use the error_event_file
#endif

#define ERROR_EVENT_COUNT 4
#define ERROR_EVENT_FILE_SIZE (ERROR_HEADER_SIZE + (ERROR_EVENT_COUNT * ERROR_EVENT_SIZE))
#define ERROR_EVENT_FILE_ALLOCATED_SIZE 152
#define INDEX_NOT_INITIALIZED 0x0F

typedef struct __attribute__((__packed__))
{
    error_event_type_t type;
    uint8_t counter;
} error_event_header_t;
typedef struct __attribute__((__packed__))
{
    error_event_header_t header;
    uint8_t data[ERROR_EVENT_DATA_SIZE];
} error_event_t;


// in practice this translates to 0x0123 because of endianess
typedef struct __attribute__((__packed__))
{
    uint8_t index_1 : 4;
    uint8_t index_0 : 4;
    uint8_t index_3 : 4;
    uint8_t index_2 : 4;
} indexes_t;

typedef struct __attribute__((__packed__))
{
    union
    {
        uint8_t bytes[4];
        struct
        {
            indexes_t indexes;
            uint16_t RFU;
        };
    };
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
    4, // ASSERT_EVENT
    4, // LOG_EVENT
};

static bool is_init_complete;
static uint32_t start_of_file;
static error_event_file_header_t header = 
{
    .indexes = 
    {
        .index_0 = INDEX_NOT_INITIALIZED, 
        .index_1 = INDEX_NOT_INITIALIZED, 
        .index_2 = INDEX_NOT_INITIALIZED, 
        .index_3 = INDEX_NOT_INITIALIZED
    }, 
    .RFU=0
};
static error_event_file_header_t written_header;
static low_level_read_cb_t low_level_read_cb_function;
static low_level_write_cb_t low_level_write_cb_function;
static uintptr_t error_event_data_buffer[ERROR_EVENT_DATA_SIZE/sizeof(uintptr_t)];

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
        if(ret)
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
    memcpy(written_header.bytes, header.bytes, sizeof(error_event_file_header_t));
    start_of_file = fs_get_address(ERROR_EVENT_FILE_ID) + sizeof(d7ap_fs_file_header_t);

    is_init_complete = true;
    d7ap_fs_register_file_modified_callback(ERROR_EVENT_FILE_ID, &error_event_file_reset);
    error_event_file_print();
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

static void translate_indexes_to_array(indexes_t indexes, uint8_t* array) 
{
    array[0] = indexes.index_0;
    array[1] = indexes.index_1;
    array[2] = indexes.index_2;
    array[3] = indexes.index_3;

    _Static_assert(ERROR_EVENT_COUNT == 4, "This function should get changed when changing the amount of events logged.");
}

static void rotate_indexes(uint8_t event_idx)
{
    bool found = false;
    indexes_t new_indexes;
    if(header.indexes.index_0 == event_idx)
        return;

    new_indexes.index_0 = event_idx;
    new_indexes.index_1 = header.indexes.index_0;

    if(header.indexes.index_1 == event_idx) 
    {
        found = true;
        new_indexes.index_2 = header.indexes.index_2;
    }
    else
    {
        new_indexes.index_2 = header.indexes.index_1;
    }

    if(header.indexes.index_2 == event_idx || found) 
    {
        found = true;
        new_indexes.index_3 = header.indexes.index_3;
    }
    else
    {
        new_indexes.index_3 = header.indexes.index_2;
    }

    header.indexes = new_indexes;
}

static void update_header_if_needed()
{
    if(memcmp(written_header.bytes, header.bytes, sizeof(error_event_file_header_t)))
    {
        low_level_write_cb_function(start_of_file, (const uint8_t*)&header, sizeof(error_event_file_header_t));
        memcpy(written_header.bytes, header.bytes, sizeof(error_event_file_header_t));
    }
}

error_t error_event_file_log_event(error_event_type_t event_type, uint8_t* event_data, uint8_t event_data_size)
{
    uint8_t index_array[ERROR_EVENT_COUNT];
    if(!is_init_complete)
    {
        return ERROR;
    }
    uint8_t event_idx;
    error_event_t event;
    uint8_t first_reduced_compare_match = 0xFF;
    if(event_data_size > ERROR_EVENT_DATA_SIZE)
    {
        event_data_size = ERROR_EVENT_DATA_SIZE;
    }
    translate_indexes_to_array(header.indexes, index_array);
    // If error file is not empty, check if event is already present
    if(index_array[0] != INDEX_NOT_INITIALIZED)
    {
        uint8_t reduced_compare_match_count = 0;
        for(uint8_t index = 0; index < ERROR_EVENT_COUNT; index++) {
            event_idx = index_array[index];
            
            if(event_idx == INDEX_NOT_INITIALIZED) {
                event_idx = index;
                break;
            }

            if(low_level_read_cb_function(get_address_of_event(event_idx), (uint8_t*)&event, sizeof(error_event_t)) == SUCCESS)
            {
                if(event.header.type == event_type)
                {
                    // Check if event is exactly the same
                    if(memcmp(event.data, event_data, event_data_size) == 0)
                    {
                        // Only increment of counter is needed
                        if(event.header.counter < UINT8_MAX)
                        {
                            event.header.counter++;
                            low_level_write_cb_function(get_address_of_event(event_idx) + sizeof(uint8_t), (const uint8_t*)&event.header.counter, sizeof(uint8_t));
                        }
                        rotate_indexes(event_idx);
                        update_header_if_needed();
                        return SUCCESS;
                    }
                    else if(event_reduced_compare_size[event_type] > 0 && memcmp(event.data, event_data, event_reduced_compare_size[event_type]) == 0)
                    {
                        if(first_reduced_compare_match == 0xFF)
                            first_reduced_compare_match = event_idx;
                        reduced_compare_match_count++;
                        if(reduced_compare_match_count >= 2)
                        {
                            rotate_indexes(first_reduced_compare_match);
                            update_header_if_needed();
                            // Already 2 events logged of this type
                            return ERROR;
                        }
                    }

                }
            }
        }

        rotate_indexes(event_idx);
    }
    else
    {
        header.indexes.index_0 = 0;
        event_idx = 0;
    }
    event.header.type = event_type;
    event.header.counter = 1;
    memcpy(event.data, event_data, event_data_size);
    low_level_write_cb_function(get_address_of_event(event_idx), (const uint8_t*)&event, event_data_size + 2);
    update_header_if_needed();
    return SUCCESS;
}

bool error_event_file_has_event()
{
    if(is_init_complete)
    {
        return header.indexes.index_0 != INDEX_NOT_INITIALIZED;
    }
    return false;
}

error_t error_event_get_file_with_latest_event_only(uint8_t* data, uint32_t* length) {
    error_t result;
    uint32_t offset;
    uint8_t* current_data_pointer = data;

    if(!error_event_file_has_event())
    {
        *length = 0;
        return ERROR;
    }

    memcpy(current_data_pointer, header.bytes, sizeof(error_event_file_header_t));
    current_data_pointer += sizeof(error_event_file_header_t);

    *length = sizeof(error_event_t);
    offset = sizeof(error_event_file_header_t) + (sizeof(error_event_t) * header.indexes.index_0);
    result = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, offset, current_data_pointer, length, ROOT_AUTH);
    if(result != SUCCESS)
    {
        *length = 0;
        return result;
    }

    *length = sizeof(error_event_file_header_t) + sizeof(error_event_t);
    return SUCCESS;
}

void error_event_file_reset(uint8_t file_id)
{
    if(is_init_complete)
    {
        header.indexes.index_0 = INDEX_NOT_INITIALIZED;
        header.indexes.index_1 = INDEX_NOT_INITIALIZED;
        header.indexes.index_2 = INDEX_NOT_INITIALIZED;
        header.indexes.index_3 = INDEX_NOT_INITIALIZED;
        header.RFU = 0;
        update_header_if_needed();
    }
}

void error_event_file_print()
{
#ifdef FRAMEWORK_LOG_ENABLED
    if(!is_init_complete || header.indexes.index_0 == INDEX_NOT_INITIALIZED)
    {
        return;
    }
    uint8_t index_array[ERROR_EVENT_COUNT];
    uint8_t event_idx;
    translate_indexes_to_array(header.indexes, index_array);
    log_print_string("indexes: %1X%1X%1X%1X", header.indexes.index_0, header.indexes.index_1, header.indexes.index_2, header.indexes.index_3);

    for(uint8_t index = 0; index < ERROR_EVENT_COUNT; index++)
    {
        event_idx = index_array[index];
        
        if(event_idx == INDEX_NOT_INITIALIZED) {
            break;
        }
        uint8_t offset = ERROR_HEADER_SIZE + (event_idx * ERROR_EVENT_SIZE);
        uint32_t length = sizeof(error_event_header_t);
        error_event_header_t event_header;
        d7ap_fs_read_file(ERROR_EVENT_FILE_ID, offset, (uint8_t*)&event_header, &length, ROOT_AUTH);
        log_print_string("Event %d count %d", event_header.type, event_header.counter);
        offset += sizeof(error_event_header_t);
        if(event_header.type == WATCHDOG_EVENT)
        {
            uintptr_t callstack[ERROR_EVENT_DATA_SIZE/sizeof(uintptr_t)];
            length = sizeof(callstack);
            d7ap_fs_read_file(ERROR_EVENT_FILE_ID, offset, (uint8_t*)&callstack, &length, ROOT_AUTH);
            log_print_string("  Task 0x%08x", callstack[0]);
            for(uint8_t pc_index = 1; pc_index < ERROR_EVENT_DATA_SIZE/sizeof(uintptr_t); pc_index ++)
            {
                if(callstack[pc_index] == 0)
                {
                    break;
                }
                log_print_string("  PC%d: 0x%08x", pc_index, callstack[pc_index]);
            }
        }
        else
        {
            uint8_t data[ERROR_EVENT_DATA_SIZE];
            length = ERROR_EVENT_DATA_SIZE;
            d7ap_fs_read_file(ERROR_EVENT_FILE_ID, offset, data, &length, ROOT_AUTH);
            log_print_data(data, length);
        }
    }
#endif
}

error_t error_event_create_watchdog_event()
{
    memset(error_event_data_buffer, 0, sizeof(error_event_data_buffer));
    error_event_data_buffer[0] = (uintptr_t)sched_get_current_task();
    callstack_from_isr(&error_event_data_buffer[1], ERROR_EVENT_DATA_SIZE/sizeof(uintptr_t) - 1);
    return error_event_file_log_event(WATCHDOG_EVENT, (uint8_t*) error_event_data_buffer, sizeof(error_event_data_buffer));
}

error_t error_event_create_assert_event()
{
    memset(error_event_data_buffer, 0, sizeof(error_event_data_buffer));
    error_event_data_buffer[0] = (uintptr_t)sched_get_current_task();
    callstack(&error_event_data_buffer[1], ERROR_EVENT_DATA_SIZE/sizeof(uintptr_t) - 1, 1);
    return error_event_file_log_event(ASSERT_EVENT, (uint8_t*) error_event_data_buffer, sizeof(error_event_data_buffer));
}

#include "d7ap_fs.h"
#include "debug.h"
#include "error_event_file.h"
#include "log.h"
#include "platform.h"
#include "scheduler.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

// Be sure to set FRAMEWORK_USE_ERROR_EVENT_FILE=y when configuring CMake


// define here now, since we are not using APP_BUILD() macro for tests
const char _APP_NAME[] = "error_event_file_test";
const char _GIT_SHA1[] = "";

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
    error_event_type_t type;
    uint8_t counter;
    uint8_t* data;
    uint8_t size;
}verify_content_t;


void run_test()
{
    uint8_t data[150];
    uint32_t length;
    error_t rc;

    // Init error event file
    log_print_string("Test initialization of error event file...");
    rc = error_event_get_file_with_latest_event_only(data, &length);
    assert(rc == ERROR);
    rc = error_event_file_init(&low_level_read_cb, &low_level_write_cb);
    assert(rc == SUCCESS);
    assert(error_event_file_has_event() == false);
    length = sizeof(error_event_file_header_t);
    error_event_file_header_t header;
    rc = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, 0, (uint8_t*)&header, &length, ROOT_AUTH);
    assert(rc == SUCCESS);
    assert(header.indexes.index_0 == 0xF && header.indexes.index_3 == 0xF);
    rc = error_event_get_file_with_latest_event_only(data, &length);
    assert(rc == ERROR);
    log_print_string("Test initialization of error event file successful.");

    // Add first event
    log_print_string("Test adding a first event...");
    uint8_t event_1_data[] = {0, 1, 2, 3, 4, 5, 6, 7};
    rc = error_event_file_log_event(WATCHDOG_EVENT, event_1_data, sizeof(event_1_data));
    assert(rc == SUCCESS);
    assert(error_event_file_has_event() == true);
    length = sizeof(error_event_file_header_t);
    rc = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, 0, (uint8_t*)&header, &length, ROOT_AUTH);
    assert(rc == SUCCESS);
    assert(header.indexes.index_0 == 0x0 && header.indexes.index_1 == 0xF);
    rc = error_event_get_file_with_latest_event_only(data, &length);
    assert(rc==SUCCESS);
    assert(!memcmp(data, &header, sizeof(error_event_file_header_t)));
    assert(!memcmp(data + sizeof(error_event_file_header_t) + 2, event_1_data, sizeof(event_1_data)));
    assert(length == ERROR_EVENT_SIZE + ERROR_HEADER_SIZE);
    log_print_string("Test adding a first event successful.");

    // Add a second event
    log_print_string("Test adding a second event...");
    uint8_t event_2_data[] = {0, 1, 2, 3};
    rc = error_event_file_log_event(ASSERT_EVENT, event_2_data, sizeof(event_2_data));
    assert(rc == SUCCESS);
    assert(error_event_file_has_event() == true);
    length = sizeof(error_event_file_header_t);
    rc = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, 0, (uint8_t*)&header, &length, ROOT_AUTH);
    assert(rc == SUCCESS);
    assert(header.indexes.index_0 == 0x1 && header.indexes.index_1 == 0x0);
    rc = error_event_get_file_with_latest_event_only(data, &length);
    assert(rc==SUCCESS);
    assert(!memcmp(data, &header, sizeof(error_event_file_header_t)));
    assert(!memcmp(data + sizeof(error_event_file_header_t) + 2, event_2_data, sizeof(event_2_data)));
    assert(length == ERROR_EVENT_SIZE + ERROR_HEADER_SIZE);
    log_print_string("Test adding a second event successful.");

    // Add a third event
    log_print_string("Test adding a third event...");
    uint8_t event_3_data[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    rc = error_event_file_log_event(LOG_EVENT, event_3_data, sizeof(event_3_data));
    assert(rc == SUCCESS);
    assert(error_event_file_has_event() == true);
    length = sizeof(error_event_file_header_t);
    rc = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, 0, (uint8_t*)&header, &length, ROOT_AUTH);
    assert(rc == SUCCESS);
    assert(header.indexes.index_0 == 0x2 && header.indexes.index_2 == 0x0);
    log_print_string("Test adding a third event successful.");

    // Adding first event again, no new event should be created
    log_print_string("Test adding first event again...");
    rc = error_event_file_log_event(WATCHDOG_EVENT, event_1_data, sizeof(event_1_data));
    assert(rc == SUCCESS);
    assert(error_event_file_has_event() == true);
    length = sizeof(error_event_file_header_t);
    rc = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, 0, (uint8_t*)&header, &length, ROOT_AUTH);
    assert(rc == SUCCESS);
    assert(header.indexes.index_0 == 0x0 && header.indexes.index_2 == 0x1);
    log_print_string("Test adding a first event again successful.");

    log_print_string("Test adding a 2nd count of the third event...");
    rc = error_event_file_log_event(LOG_EVENT, event_3_data, sizeof(event_3_data));
    assert(rc == SUCCESS);
    assert(error_event_file_has_event() == true);
    length = sizeof(error_event_file_header_t);
    rc = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, 0, (uint8_t*)&header, &length, ROOT_AUTH);
    assert(rc == SUCCESS);
    assert(header.indexes.index_0 == 0x2 && header.indexes.index_2 == 0x1);
    uint8_t counter;
    length = 1;
    rc = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, sizeof(error_event_file_header_t) + 2 * (ERROR_EVENT_DATA_SIZE + 2) + 1, &counter, &length, ROOT_AUTH);
    assert(rc == SUCCESS);
    assert(counter == 2);
    log_print_string("Test adding a 2nd count of the third event successful.");

    // Add a fourth event similar as the third event, a new event should be added
    log_print_string("Test adding a fourth event similar as the third event...");
    uint8_t event_4_data[] = {9, 8, 7, 6, 0, 1, 2, 3, 4, 5};
    rc = error_event_file_log_event(LOG_EVENT, event_4_data, sizeof(event_4_data));
    assert(rc == SUCCESS);
    assert(error_event_file_has_event() == true);
    length = sizeof(error_event_file_header_t);
    rc = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, 0, (uint8_t*)&header, &length, ROOT_AUTH);
    assert(rc == SUCCESS);
    assert(header.indexes.index_0 == 0x3 && header.indexes.index_3 == 0x1);
    log_print_string("Test adding a fourth event similar as the third event successful.");

    // reinitialize file again to mimic reboot
    log_print_string("Test reinitializing file to mimic reboot...");
    rc = error_event_file_init(&low_level_read_cb, &low_level_write_cb);
    assert(rc == SUCCESS);
    assert(error_event_file_has_event() == true);
    length = sizeof(error_event_file_header_t);
    rc = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, 0, (uint8_t*)&header, &length, ROOT_AUTH);
    assert(rc == SUCCESS);
    assert(header.indexes.index_0 == 0x3 && header.indexes.index_3 == 0x1);
    log_print_string("Test reinitializing file to mimic reboot successful.");

    // Add a fifth event that overwrites the first event
    log_print_string("Test adding a fifth event that overwrites the first event...");
    uint8_t event_5_data[] = {3, 2, 1, 0};
    rc = error_event_file_log_event(ASSERT_EVENT, event_5_data, sizeof(event_5_data));
    assert(rc == SUCCESS);
    assert(error_event_file_has_event() == true);
    length = sizeof(error_event_file_header_t);
    rc = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, 0, (uint8_t*)&header, &length, ROOT_AUTH);
    assert(rc == SUCCESS);
    assert(header.indexes.index_0 == 0x1 && header.indexes.index_3 == 0x0);
    log_print_string("Test adding a fifth event that overwrites the first event successful.");

    // Add a sixth event similar as the third and forth event, no new event should be added
    log_print_string("Test adding a sixth event similar as the third and forth event...");
    uint8_t event_6_data[] = {9, 8, 7, 6, 0, 1, 5, 4, 2, 3};
    rc = error_event_file_log_event(LOG_EVENT, event_6_data, sizeof(event_6_data));
    assert(rc == ERROR);
    assert(error_event_file_has_event() == true);
    length = sizeof(error_event_file_header_t);
    rc = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, 0, (uint8_t*)&header, &length, ROOT_AUTH);
    assert(rc == SUCCESS);
    assert(header.indexes.index_0 == 0x3 && header.indexes.index_3 == 0x0);
    log_print_string("Test adding a sixth event similar as the third and forth event successful.");

    // Adding fifth event again, no new event should be created
    log_print_string("Test adding a fifth event again...");
    rc = error_event_file_log_event(ASSERT_EVENT, event_5_data, sizeof(event_5_data));
    assert(rc == SUCCESS);
    assert(error_event_file_has_event() == true);
    length = sizeof(error_event_file_header_t);
    rc = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, 0, (uint8_t*)&header, &length, ROOT_AUTH);
    assert(rc == SUCCESS);
    assert(header.indexes.index_0 == 0x1 && header.indexes.index_3 == 0x0);
    log_print_string("Test adding a fifth event again successful.");

    // Checking content of file
    // Should be
    // 0: fifth event with counter 2
    // 1: second event with counter 1
    // 2: third event with counter 1
    // 3: fourth event with counter 1
    verify_content_t verify_content[] = {
        {
            WATCHDOG_EVENT,
            2,
            event_1_data,
            sizeof(event_1_data)
        },
        {
            ASSERT_EVENT,
            2,
            event_5_data,
            sizeof(event_5_data)
        },
        {
            LOG_EVENT,
            2,
            event_3_data,
            sizeof(event_3_data)
        },
        {
            LOG_EVENT,
            1,
            event_4_data,
            sizeof(event_4_data)
        }
    };
    log_print_string("Test verify content of file...");
    for (size_t i = 0; i < 4; i++)
    {
        uint8_t data[32];
        length = verify_content[i].size + 2;
        rc = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, 4 + 32 * i, (uint8_t*)&data, &length, ROOT_AUTH);
        assert(rc == SUCCESS);
        assert(data[0] == verify_content[i].type);
        assert(data[1] == verify_content[i].counter);
        assert(memcmp(&data[2], verify_content[i].data, verify_content[i].size) == 0);
    }
    log_print_string("Test verify content of file successful");

    log_print_string("Reset error verify file...");
    error_event_file_reset(ERROR_EVENT_FILE_ID);
    assert(error_event_file_has_event() == false);
    length = sizeof(error_event_file_header_t);
    rc = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, 0, (uint8_t*)&header, &length, ROOT_AUTH);
    assert(rc == SUCCESS);
    assert(header.indexes.index_0 == 0xF && header.indexes.index_3 == 0xF);
    log_print_string("Reset error verify file successful.");

    // Add first event again
    log_print_string("Test adding first event again...");
    rc = error_event_file_log_event(WATCHDOG_EVENT, event_1_data, sizeof(event_1_data));
    assert(rc == SUCCESS);
    assert(error_event_file_has_event() == true);
    length = sizeof(error_event_file_header_t);
    rc = d7ap_fs_read_file(ERROR_EVENT_FILE_ID, 0, (uint8_t*)&header, &length, ROOT_AUTH);
    assert(rc == SUCCESS);
    assert(header.indexes.index_0 == 0x0 && header.indexes.index_3 == 0xF);
    log_print_string("Test adding first event again successful.");
    
    exit(0);
}


void bootstrap(void *arg)
{
    d7ap_fs_init();
    sched_register_task(&run_test);
    sched_post_task(&run_test);
}
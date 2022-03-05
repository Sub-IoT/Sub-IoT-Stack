#ifndef __FILE_DEFINITION_H
#define __FILE_DEFINITION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUTTON_FILE_ID 51
#define BUTTON_FILE_SIZE sizeof(button_file_t)

#define RAW_BUTTON_FILE_SIZE 4

typedef struct
{
    union
    {
        uint8_t bytes[RAW_BUTTON_FILE_SIZE];
        struct
        {
            button_id_t button_id;
            uint8_t mask;
            uint8_t elapsed_deciseconds;
            buttons_state_t buttons_state;
        } __attribute__((__packed__));
    };
} button_file_t;

#define HEARTBEAT_FILE_ID 52
#define HEARTBEAT_FILE_SIZE sizeof(button_file_t)

#define RAW_HEARTBEAT_FILE_SIZE 5

typedef struct
{
    union
    {
        uint8_t bytes[RAW_BUTTON_FILE_SIZE];
        struct
        {
            uint16_t battery_voltage;
            uint8_t counter;
            uint8_t nacks;
            uint8_t acks;
        } __attribute__((__packed__));
    };
} heartbeat_file_t;

#define VERSION_FILE_ID 53
#define VERSION_FILE_SIZE sizeof(button_file_t)

#define RAW_VERSION_FILE_SIZE 2

typedef struct
{
    union
    {
        uint8_t bytes[RAW_BUTTON_FILE_SIZE];
        struct
        {
            uint8_t version;
            uint8_t application_id;
        } __attribute__((__packed__));
    };
} version_file_t;



#define PIR_FILE_ID 54
#define PIR_FILE_SIZE sizeof(button_file_t)

#define RAW_BUTTON_FILE_SIZE 4

typedef struct
{
    union
    {
        uint8_t bytes[RAW_BUTTON_FILE_SIZE];
        struct
        {
            uint32_t battery_voltage;
        } __attribute__((__packed__));
    };
} pir_file_t;

#endif //__FILE_DEFINITION_H
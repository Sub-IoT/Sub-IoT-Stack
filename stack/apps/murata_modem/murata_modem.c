/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2017 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 /*
  * Murata modem application supporting both lora and dash7
  * author      Mahfoudhi Farouk <farouk.mahfoudhi@uantwerpen.be>
  * date        01-2019
  */

#include "hwleds.h"
#include "hwsystem.h"
#include "scheduler.h"
#include "timer.h"
#include "debug.h"
#include "log.h"
#include "d7ap.h"
#include "dae.h"
#include "fs.h"
#include "alp_layer.h"
#include "lorawan_stack.h"
#include "string.h"
#include "platform_defs.h"
#include "modules_defs.h"
#include "shell.h"
#include "console.h"
#ifndef MODULE_LORAWAN
#error "Murata modem requires MODULE_LORAWAN=y"
#endif
#ifndef PLATFORM_MURATA_ABZ
#error "Murata modem requires MURATA_ABZ=y"
#endif

// registers used to make unique identifier
#define ID1 ( 0x1FF80050 )
#define ID2 ( 0x1FF80054 )
#define ID3 ( 0x1FF80064 )

#define MAX_SERIAL_RX_BUFFER_LEN        256
#define LORAWAN_MTU                     51

enum module_mode
{
    MODE_LORA,
    MODE_DASH7_TX,
    MODE_DASH7_RX,
    MODE_DASH7_TXRX
};

enum join_status
{
    NOT_JOINED,
    JOINING,
    JOINED,
    FAILURE
};

typedef struct {
    char name[4];
    uint16_t mtu;
    void (*init)(void);
    void (*stop)(void);
    uint8_t (*send)(uint8_t* buffer, uint16_t length);
} network_driver_t;

network_driver_t lora;
network_driver_t d7;
network_driver_t* current_network_driver;
static alp_init_args_t alp_init_args;

// Define the D7 interface configuration used for sending the ALP command on
static d7ap_session_config_t session_config = {
        .qos = {
                .qos_resp_mode = SESSION_RESP_MODE_PREFERRED,
                .qos_retry_mode = SESSION_RETRY_MODE_NO,
                .qos_stop_on_error       = false,
                .qos_record              = false
        },
        .dormant_timeout = 0,
        .addressee = {
                .ctrl = {
                        .nls_method = AES_NONE,
                        .id_type = ID_TYPE_NOID,
                },
                .access_class = 0x01,
                .id = 0
        }
};

uint8_t serialRxBuffer[MAX_SERIAL_RX_BUFFER_LEN];
uint8_t serialRxBufferLen = 0;
uint8_t dash7RxBuffer[MAX_SERIAL_RX_BUFFER_LEN];
uint8_t network_mode = MODE_DASH7_TX;
uint8_t lora_port = 3;
uint8_t lora_join_status = NOT_JOINED;
uint8_t dash7_alp_file_id = 0x40;
uint8_t sendData[MAX_SERIAL_RX_BUFFER_LEN/2];
uint8_t sendDataLen = 0;
static uint8_t devEui[8];
static uint8_t appEui[8] = { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D };
static uint8_t appKey[16] = { 0x86, 0xAE, 0x7E, 0xF1, 0xC1, 0x5F, 0xF3, 0xE5, 0xC3, 0x9E, 0x75, 0xBA, 0x84, 0x37, 0x65, 0x76 };

void modem_write_string(char* data)
{
    console_print_bytes(data, strnlen(data, 100));
}

void modem_write_buffer(uint8_t* data,uint8_t len)
{
    console_print_bytes(data, len);
}

uint8_t network_driver_send(uint8_t *data, uint16_t length)
{
    log_print_string("network_driver_send(): sending %d bytes of data", length);
    return current_network_driver->send(data, length);
}

void on_alp_command_completed_cb(uint8_t tag_id, bool success)
{
    if (success)
        log_print_string("Command completed successfully");
    else
        log_print_string("Command failed, no ack received");
}

void on_alp_command_result_cb(d7ap_session_result_t result, uint8_t* payload, uint8_t payload_length)
{
    log_print_string("recv response @ %i dB link budget from:", result.link_budget);
    log_print_data(result.addressee.id, 8);
}

void on_unsolicited_response_received(d7ap_session_result_t d7asp_result, uint8_t *alp_command, uint8_t alp_command_size)
{
    if((network_mode == MODE_DASH7_RX) || (network_mode == MODE_DASH7_TXRX))
    {
        if (alp_command_size == (alp_command[3] + 4))
        {
            char buff[8];
            sprintf(buff, "%d", 2 * alp_command[3]);
            modem_write_string("\r\n+DASH7RXDATA:");
            modem_write_string(buff);
            modem_write_string("\r\n");
            sprintf(dash7RxBuffer, "");

            for (uint8_t cpt = 4; cpt < alp_command[3] + 4; cpt++)
                sprintf(dash7RxBuffer, "%s%02x", dash7RxBuffer, alp_command[cpt]);
        }
        else
        {
            modem_write_string("\r\nINVALDRX\r\n");
        }
    }
}

static uint8_t transmit_d7ap(uint8_t* alp, uint16_t len)
{
    alp_layer_execute_command_over_d7a(alp, len, &session_config);
    return 0;
}

static void init_d7ap()
{
    d7ap_init();
    log_print_string("DASH7 init");
}

void on_join_completed(bool success,uint8_t app_port,bool request_ack)
{
    if(!success) {
        lora_join_status = FAILURE;
        log_print_string("LoRa Join failed !");
    } else {
        lora_join_status = JOINED;
        log_print_string("LoRa Join succeeded !");
    }
}
void lorwan_rx(lorawan_AppData_t *AppData)
{
    log_print_string("lorwan_rx"); //TODO
}
void lorwan_tx(bool error)
{
    log_print_string("lorwan_tx"); //TODO
}

static void lora_init(void) {
    log_print_string("LoRa init");
    lorawan_session_config_otaa_t lorawan_session_config;
    memcpy(&lorawan_session_config.devEUI,devEui ,8);
    memcpy(&lorawan_session_config.appEUI, appEui,8);
    memcpy(&lorawan_session_config.appKey,appKey,16);
    lorawan_register_cbs(lorwan_rx,lorwan_tx,on_join_completed);
    lora_join_status = JOINING;
    lorawan_stack_init_otaa(&lorawan_session_config);
}

static uint8_t lora_send(uint8_t* buffer, uint16_t length)
{
    lorawan_stack_error_t err;
    err = lorawan_stack_send(buffer, length, lora_port, false);

    log_print_string("network_driver_send(): lorawan_stack_send returned %d", err);

    if (err == LORAWAN_STACK_ERROR_OK) {
        log_print_string("Ok!");
        return 1;
    } else {
        log_print_string("Packet not sent!");
        return 0;
    }
}

void network_drivers_init()
{
    lora.mtu = LORAWAN_MTU;
    lora.init = &lora_init;
    lora.send = &lora_send;
    lora.stop = &lorawan_stack_deinit;
    memcpy(lora.name, "LoRa", 4);
    d7.init = &init_d7ap;
    d7.stop = &d7ap_stop;
    d7.send = &transmit_d7ap;
    memcpy(d7.name, "DSH7", 4);
}

static void set_network_mode(uint8_t mode)
{
    if (mode == MODE_LORA)
    {
        if(network_mode != MODE_LORA)
        {
            log_print_string("Switching from %s to %s", d7.name, lora.name);
            lora_join_status = NOT_JOINED;
            current_network_driver->stop();
            current_network_driver = &lora;
            current_network_driver->init();
            network_mode = MODE_LORA;
            log_print_string("Switching done");
        }
    }
    else
    {
        if(network_mode == MODE_LORA)
        {
            log_print_string("Switching from %s to %s", lora.name, d7.name);
            lora_join_status = NOT_JOINED;
            current_network_driver->stop();
            current_network_driver = &d7;
            current_network_driver->init();
            log_print_string("Switching done");
        }
        network_mode = mode;
    }
}

static void parse_hex_string(char in[], uint8_t out[], uint16_t length)
{
    uint16_t outindex = 0;
    char hexValue[2];
    for(uint16_t i=0; i<length/2; i++) {
        strncpy(hexValue, in+(i*2), 2);
        int converted = strtol(hexValue, NULL, 16);
        out[outindex] = converted;
        outindex++;
    }
}

static void set_unique_devEUI()
{
    devEui[7] = ((*(uint32_t*)ID1) + (*(uint32_t*)ID3)) >> 24;
    devEui[6] = ((*(uint32_t*)ID1) + (*(uint32_t*)ID3)) >> 16;
    devEui[5] = ((*(uint32_t*)ID1) + (*(uint32_t*)ID3)) >> 8;
    devEui[4] = ((*(uint32_t*)ID1) + (*(uint32_t*)ID3));
    devEui[3] = ((*(uint32_t*)ID2)) >> 24;
    devEui[2] = ((*(uint32_t*)ID2)) >> 16;
    devEui[1] = ((*(uint32_t*)ID2)) >> 8;
    devEui[0] = ((*(uint32_t*)ID2));
}

void send_data()
{
    if ((network_mode == MODE_DASH7_TX) || (network_mode == MODE_DASH7_TXRX))
    {
        // Generate ALP command. We do this manually for now (until we have an API for this).
        // We will be sending a return file data action, without a preceding file read request.
        // This is an unsolicited message, where we push the sensor data to the gateway(s).
        // Please refer to the spec for the format
        uint8_t alp_command[4 + MAX_SERIAL_RX_BUFFER_LEN / 2] = {
                // ALP Control byte
                ALP_OP_RETURN_FILE_DATA,
                // File Data Request operand:
                dash7_alp_file_id, // the file ID
                0, // offset in file
                sendDataLen // data length
                // the sensor data, see below
        };
        memcpy(alp_command + 4, sendData, sendDataLen);
        current_network_driver->send(alp_command, sendDataLen + 4);
    }
    else if (network_mode == MODE_LORA)
    {
        if (lora_join_status == JOINED)
            current_network_driver->send(sendData, sendDataLen);
        else
            log_print_string("Ignore sending: Lora not joined!");
    }
    memset(sendData, 0, sendDataLen);
    sendDataLen = 0;
}

uint8_t get_max_payload_len(void)
{
    if ((network_mode == MODE_LORA) && (lora_join_status == JOINED))
    {
        return lorawan_stack_get_max_payload_size();
    }
    return 0;
}

void handle_serial_data(void)
{
    if(strcmp(serialRxBuffer,"AT") == 0)
        modem_write_string("\r\nOK\r\n");
    else if (strstr(serialRxBuffer,"AT") == NULL)
        modem_write_string("\r\nERROR\r\n");
    else if (strstr(serialRxBuffer,"AT+NRB") != NULL)
    {
        modem_write_string("\r\nREBOOTING\r\n");
        hw_reset();
    }
    else if (strstr(serialRxBuffer,"AT+MODE?") != NULL)
    {
        switch(network_mode)
        {
            case MODE_DASH7_TX:
                modem_write_string("\r\n+MODE:DASH7TX\r\n\r\nOK\r\n");
                break;

            case MODE_DASH7_RX:
                modem_write_string("\r\n+MODE:DASH7RX\r\n\r\nOK\r\n");
                break;

            case MODE_DASH7_TXRX:
                modem_write_string("\r\n+MODE:DASH7TXRX\r\n\r\nOK\r\n");
                break;

            case MODE_LORA:
            default:
                modem_write_string("\r\n+MODE:LORA\r\n\r\nOK\r\n");
                break;
        }
    }
    else if (strstr(serialRxBuffer,"AT+MODE=") != NULL)
    {
        if (strstr(serialRxBuffer,"DASH7TXRX"))
        {
            modem_write_string("\r\nOK\r\n");
            set_network_mode(MODE_DASH7_TXRX);
        }
        else if (strstr(serialRxBuffer,"DASH7RX"))
        {
            modem_write_string("\r\nOK\r\n");
            set_network_mode(MODE_DASH7_RX);
        }
        else if (strstr(serialRxBuffer,"DASH7TX"))
        {
            modem_write_string("\r\nOK\r\n");
            set_network_mode(MODE_DASH7_TX);
        }
        else  if (strstr(serialRxBuffer,"LORA"))
        {
            modem_write_string("\r\nOK\r\n");
            set_network_mode(MODE_LORA);
        }
        else
        {
            modem_write_string("\r\nERROR\r\n");
        }
    }
    else if (strstr(serialRxBuffer,"AT+LORAPORT?") != NULL)
    {
        uint8_t buff[8];
        sprintf(buff,"%d", lora_port);
        modem_write_string("\r\n+LORAPORT:");
        modem_write_string(buff);
        modem_write_string("\r\n\r\nOK\r\n");
    }
    else if (strstr(serialRxBuffer,"AT+LORAPORT=") != NULL)
    {
        char* ptr = strstr(serialRxBuffer,"AT+LORAPORT=");
        lora_port=atoi(ptr + strlen("AT+LORAPORT="));
        modem_write_string("\r\nOK\r\n");
    }
    else if (strstr(serialRxBuffer,"AT+DASH7FILEID?") != NULL)
    {
        uint8_t buff[8];
        sprintf(buff,"%d", dash7_alp_file_id);
        modem_write_string("\r\n+DASH7FILEID:");
        modem_write_string(buff);
        modem_write_string("\r\n\r\nOK\r\n");
    }
    else if (strstr(serialRxBuffer,"AT+DASH7FILEID=") != NULL)
    {
        char* ptr = strstr(serialRxBuffer,"AT+DASH7FILEID=");
        dash7_alp_file_id=atoi(ptr + strlen("AT+DASH7FILEID="));
        modem_write_string("\r\nOK\r\n");
    }
    else if (strstr(serialRxBuffer,"AT+SEND=") != NULL)
    {
        char* ptr = strstr(serialRxBuffer,"AT+SEND=") + strlen("AT+SEND=");
        uint8_t hex_string_len = serialRxBufferLen - strlen("AT+SEND=");

        if(hex_string_len)
        {
            parse_hex_string(ptr, sendData, hex_string_len);
            sendDataLen = hex_string_len/2;
            sched_post_task_prio(&send_data, MIN_PRIORITY - 1, NULL);
        }
        modem_write_string("\r\nOK\r\n");
    }
    else if (strstr(serialRxBuffer,"AT+DEVEUI?") != NULL)
    {
        modem_write_string("\r\n+DEVEUI:");

        for(uint8_t cpt = 0 ;cpt <8; cpt++)
        {
            uint8_t buff[2];
            sprintf(buff,"%02x", devEui[cpt]);
            modem_write_string(buff);
        }
        modem_write_string("\r\n\r\nOK\r\n");
    }
    else if (strstr(serialRxBuffer,"AT+APPEUI?") != NULL)
    {
        modem_write_string("\r\n+APPEUI:");

        for(uint8_t cpt = 0 ;cpt <8; cpt++)
        {
            uint8_t buff[2];
            sprintf(buff,"%02x", appEui[cpt]);
            modem_write_string(buff);
        }
        modem_write_string("\r\n\r\nOK\r\n");
    }
    else if (strstr(serialRxBuffer,"AT+APPEUI=") != NULL)
    {
        char* ptr = strstr(serialRxBuffer,"AT+APPEUI=") + strlen("AT+APPEUI=");
        uint8_t hex_string_len = serialRxBufferLen - strlen("AT+APPEUI=");
        parse_hex_string(ptr, appEui, hex_string_len);
        modem_write_string("\r\nOK\r\n");

        if(network_mode == MODE_LORA)
            lora_init();
    }
    else if (strstr(serialRxBuffer,"AT+APPKEY?") != NULL)
    {
        modem_write_string("\r\n+APPKEY:");

        for(uint8_t cpt = 0 ;cpt <16; cpt++)
        {
            uint8_t buff[2];
            sprintf(buff,"%02x", appKey[cpt]);
            modem_write_string(buff);
        }
        modem_write_string("\r\n\r\nOK\r\n");
    }
    else if (strstr(serialRxBuffer,"AT+APPKEY=") != NULL)
    {
        char* ptr = strstr(serialRxBuffer,"AT+APPKEY=") + strlen("AT+APPKEY=");
        uint8_t hex_string_len = serialRxBufferLen - strlen("AT+APPKEY=");
        parse_hex_string(ptr, appKey, hex_string_len);
        modem_write_string("\r\nOK\r\n");

        if(network_mode == MODE_LORA)
            lora_init();
    }
    else if (strstr(serialRxBuffer,"AT+LORAJOIN?") != NULL)
    {
        switch(lora_join_status)
        {
            case NOT_JOINED:
                modem_write_string("\r\n+LORAJOIN:NOT_JOINED\r\n\r\nOK\r\n");
                break;

            case JOINING:
                modem_write_string("\r\n+LORAJOIN:JOINING\r\n\r\nOK\r\n");
                break;

            case JOINED:
                modem_write_string("\r\n+LORAJOIN:JOINED\r\n\r\nOK\r\n");
                break;

            case FAILURE:
                modem_write_string("\r\n+LORAJOIN:FAILURE\r\n\r\nOK\r\n");
                break;

            default:
                modem_write_string("\r\n+LORAJOIN:UNKNOWN\r\n\r\nOK\r\n");
                break;
        }
    }
    else if (strstr(serialRxBuffer,"AT+LORAMAXLEN?") != NULL)
    {
        char maxLen[8];
        sprintf(maxLen,"%d",get_max_payload_len());
        modem_write_string("\r\n+LORAMAXLEN:");
        modem_write_string(maxLen);
        modem_write_string("\r\n\r\nOK\r\n");
    }
    else if (strstr(serialRxBuffer,"AT+READ=") != NULL)
    {
        if((network_mode == MODE_DASH7_RX) || (network_mode == MODE_DASH7_TXRX))
        {
            char *ptr = strstr(serialRxBuffer, "AT+READ=");
            uint8_t len = atoi(ptr + strlen("AT+READ="));
            modem_write_string("\r\n+READ:");

            if (len >= strlen(dash7RxBuffer))
            {
                modem_write_string(dash7RxBuffer);
                sprintf(dash7RxBuffer, "");
            }
            else
            {
                uint8_t remainLen = strlen(dash7RxBuffer) - len;
                modem_write_buffer(dash7RxBuffer, len);
                strncpy(dash7RxBuffer, &dash7RxBuffer[len], remainLen);
                dash7RxBuffer[remainLen] = '\0';
            }
            modem_write_string("\r\n\r\nOK\r\n");
        }
        else
        {
            modem_write_string("\r\nERROR\r\n");
        }
    }
    else if (strstr(serialRxBuffer,"AT+DASH7RXDATALEN?") != NULL)
    {
        if((network_mode == MODE_DASH7_RX) || (network_mode == MODE_DASH7_TXRX))
        {
            char buff[8];
            sprintf(buff, "%d", strlen(dash7RxBuffer));
            modem_write_string("\r\n+DASH7RXDATALEN:");
            modem_write_string(buff);
            modem_write_string("\r\n\r\nOK\r\n");
        }
        else
        {
            modem_write_string("\r\nERROR\r\n");
        }
    }
    else
        modem_write_string("\r\nERROR\r\n");
    serialRxBufferLen = 0;
    memset(serialRxBuffer,0,MAX_SERIAL_RX_BUFFER_LEN);
}
void modem_rx_cb(fifo_t* cmd_fifo)
{
    uint8_t size = fifo_get_size(cmd_fifo);
    fifo_pop(cmd_fifo, serialRxBuffer, size);
    serialRxBufferLen = size;
    serialRxBuffer[size] = '\0';

    if(!sched_is_scheduled(&handle_serial_data))
        sched_post_task_prio(&handle_serial_data, MIN_PRIORITY - 1, NULL);
}

void bootstrap() {
    log_print_string("Device booted\n");
    modem_write_string("\r\nIDLAB_MURATA_LORA_DASH7_MODULE_BOOT\r\n\r\nOK\r\n");
    shell_init(false);
    shell_register_handler((cmd_handler_registration_t){ .id = CMD_DEFAULT_HANDLER_ID, .cmd_handler_callback = &modem_rx_cb });
    fs_init_args_t fs_init_args = (fs_init_args_t){
            .fs_d7aactp_cb = &alp_layer_process_d7aactp,
            .access_profiles_count = DEFAULT_ACCESS_PROFILES_COUNT,
            .fs_user_files_init_cb = NULL,
            .access_profiles = default_access_profiles,
            .access_class = 0x01
    };
    fs_init(&fs_init_args);
    set_unique_devEUI();
    network_drivers_init();
    network_mode = MODE_DASH7_TX;
    current_network_driver = &d7;
    current_network_driver->init();
    alp_init_args.alp_command_completed_cb = &on_alp_command_completed_cb;
    alp_init_args.alp_command_result_cb = &on_alp_command_result_cb;
    alp_init_args.alp_received_unsolicited_data_cb = &on_unsolicited_response_received;
    alp_layer_init(&alp_init_args, false);
    sched_register_task(&send_data);
    sched_register_task(&handle_serial_data);
}
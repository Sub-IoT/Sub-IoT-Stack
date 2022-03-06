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

#ifdef FRAMEWORK_NETWORK_MANAGER_LOG
    #define DPRINT(...)      log_print_string(__VA_ARGS__)
    #define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
    #define DPRINT(...)
    #define DPRINT_DATA(...)
#endif

#define CHANNEL_ID 100
#define USE_PUSH7_CHANNEL_SETTINGS false
#define NETWORK_TIMEOUT 10000


static network_state_t network_state = NETWORK_MANAGER_IDLE;
static alp_init_args_t alp_init_args;
static last_transmit_completed_callback transmit_completed_cb;
static uint8_t acked_messages = 0;
static uint8_t nacks_messages = 0;
static uint8_t active_tag_id;


static uint8_t key[]
    = { 0X00, 0X01, 0X02, 0X03, 0X02, 0X01, 0X00, 0X01, 0X02, 0X03, 0X02, 0X01, 0X00, 0X01, 0X02, 0X03 };

// Define the D7 interface configuration used for sending the ALP command on

static alp_interface_config_d7ap_t itf_config = (alp_interface_config_d7ap_t){
  .itf_id = ALP_ITF_ID_D7ASP,
  .d7ap_session_config = {
    .qos = {
        .qos_resp_mode = SESSION_RESP_MODE_PREFERRED,
        .qos_retry_mode = SESSION_RETRY_MODE_NO
    },
    .dormant_timeout = 0,
    .addressee = {
        .ctrl = {
            .nls_method = AES_CTR,
            .id_type = ID_TYPE_NBID,
        },
        .access_class = 0x01, // use access profile 0 and select the first subprofile
        .id = { 3 }
    }
  }
};

static void network_timeout()
{
    alp_layer_free_commands();
    if(transmit_completed_cb)
        transmit_completed_cb(false);
}

static void on_alp_command_completed_cb(uint8_t tag_id, bool success)
{
    if(active_tag_id != tag_id)
        return;
    timer_cancel_task(&network_timeout);
    if (success) {
        DPRINT("Command (%i) completed successfully", tag_id);
    } else
        DPRINT("Command failed, no ack received");
    if(transmit_completed_cb)
        transmit_completed_cb(success);
}

static void on_alp_command_result_cb(alp_command_t* alp_command, alp_interface_status_t* origin_itf_status)
{
    if (origin_itf_status && (origin_itf_status->itf_id == ALP_ITF_ID_D7ASP) && (origin_itf_status->len > 0)) {
        d7ap_session_result_t* d7_result = ((d7ap_session_result_t*)origin_itf_status->itf_status);
        DPRINT("recv response @ %i dB link budget from:", d7_result->rx_level);
    }
    network_state = NETWORK_MANAGER_READY;
}

error_t transmit_file(uint8_t file_id, uint32_t offset, uint32_t length, uint8_t *data)
{
    bool ret;
    if(network_state != NETWORK_MANAGER_READY)
        return EBUSY;
    // Generate ALP command.
    // We will be sending a return file data action, without a preceding file read request.
    // This is an unsolicited message, where we push the sensor data to the gateway(s).

    // alloc command. This will be freed when the command completes
    alp_command_t* command = alp_layer_command_alloc(true, true); 
    // forward to the D7 interface
    ret = alp_append_forward_action(command, (alp_interface_config_t*)&itf_config, sizeof(itf_config)); 
    // add the return file data action
    ret = alp_append_return_file_data_action(command, file_id, offset, length, data); 
    // and finally execute this
    active_tag_id = command->tag_id;
    alp_layer_process(command); 
    network_state = NETWORK_MANAGER_TRANSMITTING;
    timer_post_task_delay(&network_timeout, NETWORK_TIMEOUT);
}

void get_network_quality(uint8_t* acks, uint8_t* nacks)
{
    acks = &acked_messages;
    nacks = &nacks_messages;
}

network_state_t get_network_manager_state()
{
    return network_state;
}

void network_manager_init(last_transmit_completed_callback last_transmit_completed_cb)
{
    d7ap_fs_init();
    d7ap_init();
    alp_init_args.alp_command_completed_cb = &on_alp_command_completed_cb;
    alp_init_args.alp_command_result_cb = &on_alp_command_result_cb;
    alp_layer_init(&alp_init_args, false);
    transmit_completed_cb = last_transmit_completed_cb;
    sched_register_task(&network_timeout);

    if (USE_PUSH7_CHANNEL_SETTINGS) {
        dae_access_profile_t push7_access_profile;
        d7ap_fs_read_access_class(0, &push7_access_profile);

        push7_access_profile.subbands[0].channel_index_start = CHANNEL_ID;
        push7_access_profile.subbands[0].channel_index_end = CHANNEL_ID;
        push7_access_profile.subbands[0].eirp = 5;
        push7_access_profile.subbands[1].channel_index_start = CHANNEL_ID;
        push7_access_profile.subbands[1].channel_index_end = CHANNEL_ID;
        push7_access_profile.subbands[1].eirp = 0;

        d7ap_fs_write_access_class(0, &push7_access_profile);

        uint32_t length = D7A_FILE_NWL_SECURITY_KEY_SIZE;
        d7ap_fs_write_file(D7A_FILE_NWL_SECURITY_KEY, 0, key, length, ROOT_AUTH);
    }
    network_state = NETWORK_MANAGER_READY;
}
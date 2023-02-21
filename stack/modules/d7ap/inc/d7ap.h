

/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * \author philippe.nunes@cortus.com
 */

/**
 * @file d7ap.h
 * @addtogroup d7ap
 * @ingroup framework
 * @{
 * @brief D7AP public types (used for modem and/or D7AP stack itself)
 *
 */

#ifndef D7AP_H
#define D7AP_H

#include "types.h"
#include "debug.h"
#include "hwblockdevice.h"
#include "modem_region.h"

#define ID_TYPE_NBID_ID_LENGTH 1
#define ID_TYPE_NOID_ID_LENGTH 0
#define ID_TYPE_UID_ID_LENGTH   8
#define ID_TYPE_VID_LENGTH      2

#define ID_TYPE_IS_BROADCAST(id_type) (id_type == ID_TYPE_NBID || id_type == ID_TYPE_NOID)

#define D7A_PAYLOAD_MAX_SIZE 255 // TODO confirm this value when FEC and security are disabled

#define D7A_MAX_ADDRESS_ID_LENGTH 8


typedef enum {
    ID_TYPE_NBID = 0,
    ID_TYPE_NOID = 1,
    ID_TYPE_UID = 2,
    ID_TYPE_VID = 3
} d7ap_addressee_id_type_t;

typedef enum
{
    AES_NONE = 0, /* No security */
    AES_CTR = 0x01, /* data confidentiality */
    AES_CBC_MAC_128 = 0x02, /* data authenticity */
    AES_CBC_MAC_64 = 0x03, /* data authenticity */
    AES_CBC_MAC_32 = 0x04, /* data authenticity */
    AES_CCM_128 = 0x05, /* data confidentiality and authenticity*/
    AES_CCM_64 = 0x06, /* data confidentiality and authenticity*/
    AES_CCM_32 = 0x07, /* data confidentiality and authenticity*/
} nls_method_t;

typedef struct {
    union {
        uint8_t raw;
        struct {
            nls_method_t nls_method : 4;
            d7ap_addressee_id_type_t id_type : 2;
            uint8_t _rfu : 2;
        };
    };
} d7ap_addressee_ctrl_t;

typedef struct __attribute__((__packed__)) {
    d7ap_addressee_ctrl_t ctrl;
    union {
        uint8_t access_class;
        struct {
            uint8_t access_mask : 4;
            uint8_t access_specifier : 4;
        };
    };
    uint8_t id[D7A_MAX_ADDRESS_ID_LENGTH];
} d7ap_addressee_t;

typedef struct {
    union {
        uint8_t raw;
        struct {
            uint8_t _rfu : 4;
            bool ucast : 1;
            bool retry : 1;
            bool missed : 1;
            bool nls : 1;
        };
    };
} d7ap_sp_state_t;

typedef struct __attribute__((__packed__))
{
    uint8_t channel_header;         /**< PHY layer channel header */
    uint16_t center_freq_index;     /**< The center frequency index of the channel id */
} d7ap_channel_t;

typedef struct __attribute__((__packed__)) {
    d7ap_channel_t channel;
    uint8_t rx_level;
    uint8_t link_budget;
    uint8_t target_rx_level;
    d7ap_sp_state_t status;
    uint8_t fifo_token;
    uint8_t seqnr;
    uint8_t response_to;
    d7ap_addressee_t addressee;
} d7ap_session_result_t;

typedef enum  {
    SESSION_RESP_MODE_NO = 0,
    SESSION_RESP_MODE_ALL = 1,
    SESSION_RESP_MODE_ANY = 2,
    SESSION_RESP_MODE_NO_RPT = 4,
    SESSION_RESP_MODE_ON_ERR = 5,
    SESSION_RESP_MODE_PREFERRED = 6,
} d7ap_session_resp_mode_t;

typedef enum {
    SESSION_RETRY_MODE_NO = 0
} d7ap_session_retry_mode_t;

typedef struct {
    union {
        uint8_t raw;
        struct {
            d7ap_session_resp_mode_t qos_resp_mode : 3;
            d7ap_session_retry_mode_t qos_retry_mode: 3;
        };
    };
} d7ap_session_qos_t;

typedef struct __attribute__((__packed__)) {
    d7ap_session_qos_t qos;
    uint8_t dormant_timeout;
    d7ap_addressee_t addressee;
} d7ap_session_config_t;

typedef void (*d7ap_receive_callback)(uint16_t trans_id, uint8_t* payload, uint8_t len, d7ap_session_result_t result);
/**
 * @brief Called when the stack received an unsolicited message
 * @returns true when the unsolicited request will result in a response payload from the upper layer. If no response is expected
 * the upper layer should return false, so the stack can respond with an ack immediately (if requested by origin).
 */
typedef bool (*d7ap_receive_unsolicited_callback)(uint8_t* payload, uint8_t len, d7ap_session_result_t result, bool response_expected);
typedef void (*d7ap_transmitted_callback)(uint16_t trans_id, error_t error);

typedef struct{
    d7ap_receive_callback  receive_cb;                /*< receive callback,
                                                          if NULL, all message received for clientId will be discarded */
    d7ap_transmitted_callback transmitted_cb;         /*< send completion callback,
                                                          if NULL, the associated packet will be release without notification */
    d7ap_receive_unsolicited_callback unsolicited_cb; /*< unsolicited data callback,
                                                          if NULL, the associated packet will be release without notification */
} d7ap_resource_desc_t;

// override alp_interface_config_t
typedef struct {
    uint8_t itf_id;
    d7ap_session_config_t d7ap_session_config;
} __attribute__ ((__packed__)) alp_interface_config_d7ap_t;


//=========================== prototypes ======================================
/**
 * @brief   Initializes d7a module
 */
void d7ap_init(void);

/**
 * @brief Stop the D7A stack
 */
void d7ap_stop(void);

/**
 * @brief   Register the client callbacks
 *
 * @param[in] desc pointer to the client resource
 *
 * @return  the client Id
 */
uint8_t d7ap_register(d7ap_resource_desc_t* desc);


/**
 * @brief   Gets the device address UID/VID
 *
 * @param[out] *addr   buffer to store the device addressee UID/VID.
 *                     the buffer should be large enough to contain the 64 bits UID
 * @return the address type (either UID or VID)
 */
d7ap_addressee_id_type_t d7ap_get_dev_addr(uint8_t* addr);


/**
 * @brief Get the maximum payload size according the security configuration.
 *
 * @param[in] nls_method     The security configuration.
 *
 * @returns the maximum payload size in bytes.
 */
uint8_t d7ap_get_payload_max_size(nls_method_t nls_method);


/**
 * @brief   Send a packet over DASH7 network
 *
 * @param[in] clientID  The registered client Id
 * @param[in] config    The configuration for the d7a session. Set to NULL to use the current config
 * @param[in] payload   The pointer to the payload buffer
 * @param[in] len       The length of the payload
 * @param[in] expected_response_len The length of the expected response
 * @param[in,out] trans_id   Set the value of this parameter to NULL to cause the function to execute synchronously.
 *                           If this parameter is not NULL, the call executes asynchronously. Upon return from this function,
 *                           this points to the transaction identifier associated with the asynchronous operation.
 * @return 0 on success
 * @return an error (errno.h) in case of failure
 */
error_t d7ap_send(uint8_t clientId, d7ap_session_config_t* config, uint8_t* payload,
                   uint8_t len, uint8_t expected_response_len, uint16_t* trans_id);


/**
 * @brief   Sets the channels TX power index
 *
 * @param[in] power  The TX power index (from 1 to 16)
 */
void d7ap_set_tx_power(uint8_t power);


/**
 * @brief   Gets the channels TX power index
 *
 * @return  The TX power index (from 1 to 16)
 */
uint8_t d7ap_get_tx_power(void);


/**
 * @brief   Sets the device access class
 *
 * @param[in] access_class  The device access class
 */
void d7ap_set_access_class(uint8_t access_class);


/**
 * @brief   Gets the device access class
 *
 * @return  The device access class
 */
uint8_t d7ap_get_access_class(void);

/**
 * @brief   Gets the length of the addressee Id
 *
 * @param[in] id_type  The addressee Id type
 * @return  The length of the addressee Id according the addressee Id type
 */
static inline uint8_t d7ap_addressee_id_length(d7ap_addressee_id_type_t id_type) {
  switch(id_type)
  {
      case ID_TYPE_NOID:
        return ID_TYPE_NOID_ID_LENGTH;
      case ID_TYPE_NBID:
        return ID_TYPE_NBID_ID_LENGTH;
      case ID_TYPE_UID:
        return ID_TYPE_UID_ID_LENGTH;
      case ID_TYPE_VID:
        return ID_TYPE_VID_LENGTH;
      default:
        assert(false);
  }

  return ID_TYPE_NOID_ID_LENGTH;
}

static inline uint8_t d7ap_session_config_length(d7ap_session_config_t* itf_config)
{
    return sizeof(d7ap_session_config_t) - D7A_MAX_ADDRESS_ID_LENGTH + d7ap_addressee_id_length(itf_config->addressee.ctrl.id_type);
}

/**
 * @brief adds d7ap_session_result_t to array
 * 
 * @param[in] result    the D7 result
 * @param[in] array     the array where the d7 result has to go
 */
void d7ap_add_result_to_array(d7ap_session_result_t* result, uint8_t* array);

/**
 * @brief Gets the region configured in the active access class
 * 
 * @param region the region variable to store the value
 * @return error_t an error in case of failure
 */
error_t d7ap_get_modem_region(modem_region_t* region);

#endif // D7AP_H

/** @}*/

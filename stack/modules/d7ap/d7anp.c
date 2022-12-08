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
 *  \author glenn.ergeerts@uantwerpen.be
 *
 */

#include "debug.h"
#include "packet.h"
#include "d7ap_internal.h"
#include "d7anp.h"
#include "d7ap_fs.h"
#include "ng.h"
#include "log.h"
#include "math.h"
#include "hwdebug.h"
#include "aes.h"
#include "packet_queue.h"
#include "errors.h"
#include "timer.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_NP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_NWL, __VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_DATA(...)
#endif

#define CRC_SIZE 2

#if FRAMEWORK_FS_TRUSTED_NODE_TABLE_SIZE == 0
    #pragma GCC diagnostic ignored "-Wtype-limits"
#endif

typedef enum {
    D7ANP_STATE_STOPPED,
    D7ANP_STATE_IDLE,
    D7ANP_STATE_TRANSMIT,
    D7ANP_STATE_FOREGROUND_SCAN,
} state_t;

static state_t NGDEF(_d7anp_state) = D7ANP_STATE_STOPPED;
#define d7anp_state NG(_d7anp_state)

static state_t NGDEF(_d7anp_prev_state);
#define d7anp_prev_state NG(_d7anp_prev_state)

static timer_tick_t NGDEF(_fg_scan_timeout_ticks);
#define fg_scan_timeout_ticks NG(_fg_scan_timeout_ticks)

#if defined(MODULE_D7AP_NLS_ENABLED)
static dae_nwl_security_t NGDEF(_security_state);
#define security_state NG(_security_state)

static dae_nwl_ssr_t NGDEF(_node_security_state);
#define node_security_state NG(_node_security_state)

static dae_nwl_trusted_node_t* NGDEF(_latest_node);
#define latest_node NG(_latest_node)
#endif

static timer_event d7anp_fg_scan_expired_timer;
static timer_event d7anp_start_fg_scan_after_d7aadvp_timer;

d7ap_addressee_id_type_t address_id_type;
uint8_t address_id[8];

#if defined(MODULE_D7AP_NLS_ENABLED)
static inline uint8_t get_auth_len(uint8_t nls_method)
{
    switch(nls_method)
    {
    case AES_CTR:
        return 0;
    case AES_CBC_MAC_128:
    case AES_CCM_128:
        return 16;
    case AES_CBC_MAC_64:
    case AES_CCM_64:
        return 8;
    case AES_CBC_MAC_32:
    case AES_CCM_32:
        return 4;
    default:
        assert(false);
        return 0;
    }
}
#endif

static void switch_state(state_t next_state)
{
    switch(next_state)
    {
        case D7ANP_STATE_TRANSMIT:
          assert(d7anp_state == D7ANP_STATE_IDLE ||
                 d7anp_state == D7ANP_STATE_FOREGROUND_SCAN);
          d7anp_state = next_state;
          DPRINT("Switched to D7ANP_STATE_TRANSMIT");
          break;
        case D7ANP_STATE_IDLE:
          assert(d7anp_state == D7ANP_STATE_TRANSMIT ||
                 d7anp_state == D7ANP_STATE_FOREGROUND_SCAN);
          d7anp_state = next_state;
          DPRINT("Switched to D7ANP_STATE_IDLE");
          break;
        case D7ANP_STATE_FOREGROUND_SCAN:
          assert(d7anp_state == D7ANP_STATE_TRANSMIT ||
                 d7anp_state == D7ANP_STATE_IDLE);

          d7anp_state = next_state;
          DPRINT("Switched to D7ANP_STATE_FOREGROUND_SCAN");
          break;
    }

    // output state on debug pins
    d7anp_state == D7ANP_STATE_FOREGROUND_SCAN? DEBUG_PIN_SET(3) : DEBUG_PIN_CLR(3);
}

static void foreground_scan_expired(void *arg)
{
    (void)arg;

    // the FG scan expiration may also happen while Tx is busy (d7anp_state = D7ANP_STATE_TRANSMIT) // TODO validate
    assert(d7anp_state == D7ANP_STATE_FOREGROUND_SCAN || d7anp_state == D7ANP_STATE_TRANSMIT);
    DPRINT("Foreground scan expired @%i", timer_get_counter_value());

    if (d7anp_state == D7ANP_STATE_FOREGROUND_SCAN) // when in D7ANP_STATE_TRANSMIT d7anp_signal_packet_transmitted() will switch state
      switch_state(D7ANP_STATE_IDLE);

    dll_stop_foreground_scan();
    fg_scan_timeout_ticks = 0;
    d7atp_signal_foreground_scan_expired();
}

static void schedule_foreground_scan_expired_timer()
{
    // TODO in case of responder timeout_ticks counts from reception time , so subtract time passed between now and reception time
    // in case of requester timeout_ticks counts from transmission time, so subtract time passed between now and transmission time
    // since this FG scan is started directly from the ISR (transmitted callback), I don't expect a significative delta between now and the transmission time

    DPRINT("starting foreground scan expiration timer (%i ticks, now %i)", fg_scan_timeout_ticks, timer_get_counter_value());
    d7anp_fg_scan_expired_timer.next_event = fg_scan_timeout_ticks;
    error_t rtc = timer_add_event(&d7anp_fg_scan_expired_timer);
    assert(rtc == SUCCESS);
}

void d7anp_start_foreground_scan()
{
    // TODO we also do this for fg_scan_timeout_ticks = 0 for now to make sure task is executed async.
    // Might need refactoring later.

    // if the FG scan timer is already set, update only the tl timeout value
    schedule_foreground_scan_expired_timer();

    if (d7anp_state != D7ANP_STATE_FOREGROUND_SCAN)
    {
        switch_state(D7ANP_STATE_FOREGROUND_SCAN);
        dll_start_foreground_scan();
    }
}

void d7anp_set_foreground_scan_timeout(timer_tick_t timeout)
{
    DPRINT("Set FG scan timeout = %i", timeout);
    assert(d7anp_state == D7ANP_STATE_IDLE || d7anp_state == D7ANP_STATE_FOREGROUND_SCAN);

    fg_scan_timeout_ticks = timeout;
}

static void cancel_foreground_scan_task()
{
    timer_cancel_event(&d7anp_fg_scan_expired_timer);
    fg_scan_timeout_ticks = 0;
}

void d7anp_stop_foreground_scan()
{
    if (d7anp_state == D7ANP_STATE_FOREGROUND_SCAN)
    {
        cancel_foreground_scan_task();
        switch_state(D7ANP_STATE_IDLE);
    }

    /* set the radio to idle */
    dll_stop_foreground_scan();
}

void start_foreground_scan_after_D7AAdvP(void *arg)
{
    (void)arg;

    DPRINT("start_foreground_scan_after_D7AAdvP");
    fg_scan_timeout_ticks = FG_SCAN_TIMEOUT;
    d7anp_start_foreground_scan();
}

void d7anp_set_address_id(uint8_t file_id)
{
    d7ap_fs_read_uid(address_id);
}

static void set_key(uint8_t file_id)
{
    uint8_t key[AES_BLOCK_SIZE];
    assert(d7ap_fs_read_nwl_security_key(key) == SUCCESS);
    DPRINT("KEY");
    DPRINT_DATA(key, AES_BLOCK_SIZE);
    AES128_init(key);
}

void d7anp_init()
{
    assert(d7anp_state == D7ANP_STATE_STOPPED);

    d7anp_state = D7ANP_STATE_IDLE;
    fg_scan_timeout_ticks = 0;

    // Initialize timers
    timer_init_event(&d7anp_fg_scan_expired_timer, &foreground_scan_expired);
    timer_init_event(&d7anp_start_fg_scan_after_d7aadvp_timer, &start_foreground_scan_after_D7AAdvP);

    /*
     * vid or uid caching to prevent latency due to file access
     */
    d7ap_read_vid(address_id);

    // vid is not valid when set to FF
    if (memcmp(address_id, (uint8_t[2]){ 0xFF, 0xFF }, 2) == 0)
    {
        d7ap_fs_register_file_modified_callback(D7A_FILE_UID_FILE_ID, &d7anp_set_address_id);
        d7ap_fs_read_uid(address_id);
        address_id_type = ID_TYPE_UID;
    } else
        address_id_type = ID_TYPE_VID;

#if defined(MODULE_D7AP_NLS_ENABLED)
    /*
     * Init Security
     * Read the 128 bits key from the "NWL Security Key" file
     */

    d7ap_fs_register_file_modified_callback(D7A_FILE_NWL_SECURITY_KEY, &set_key);
    set_key(D7A_FILE_NWL_SECURITY_KEY);

    /* Read the NWL security parameters */
    d7ap_fs_read_nwl_security(&security_state);
    DPRINT("Initial Key counter %d", security_state.key_counter);
    DPRINT("Initial Frame counter %ld", security_state.frame_counter);
    /* Read the NWL security state of the successfully decrypted and authenticated devices */
    d7ap_fs_read_nwl_security_state_register(&node_security_state);
    latest_node = NULL;
#endif
}

void d7anp_stop()
{
    d7anp_state = D7ANP_STATE_STOPPED;
    timer_cancel_event(&d7anp_fg_scan_expired_timer);
    timer_cancel_event(&d7anp_start_fg_scan_after_d7aadvp_timer);
}

error_t d7anp_tx_foreground_frame(packet_t* packet, bool should_include_origin_template)
{
    assert(d7anp_state == D7ANP_STATE_IDLE || d7anp_state == D7ANP_STATE_FOREGROUND_SCAN);

    packet->d7anp_ctrl.hop_enabled = false;

    // we need to switch back to the current state after the transmission procedure
    d7anp_prev_state = d7anp_state;

    // No need to initialize the packet field in case of retry, except the security frame counter
    if (packet->type == RETRY_REQUEST)
        goto security;

    if (!should_include_origin_template)
    {
        packet->d7anp_ctrl.origin_id_type = ID_TYPE_NOID;
        packet->d7anp_ctrl.origin_void = true;
    }
    else
    {
        packet->d7anp_ctrl.origin_id_type = address_id_type;
        packet->d7anp_ctrl.origin_void = false;
        // note we set packet->origin_access_class in DLL, since we cache the active access class there already
    }

security:
#if defined(MODULE_D7AP_NLS_ENABLED)

    packet->d7anp_ctrl.nls_method = packet->d7anp_addressee->ctrl.nls_method;

    if (packet->d7anp_ctrl.nls_method == AES_CTR ||
        packet->d7anp_ctrl.nls_method == AES_CCM_32 ||
        packet->d7anp_ctrl.nls_method == AES_CCM_64 ||
        packet->d7anp_ctrl.nls_method == AES_CCM_128)
    {
        /* Check if frame counter reaches its maximum value */
        if (security_state.frame_counter == (uint32_t)~0)
            return EPERM;

        packet->d7anp_security.frame_counter = security_state.frame_counter++;
        packet->d7anp_security.key_counter = security_state.key_counter;
        DPRINT("Frame counter %ld", packet->d7anp_security.frame_counter);

        // Update the frame counter in the D7A file
        d7ap_fs_write_nwl_security(&security_state);
    }
#else
    assert(packet->d7anp_ctrl.nls_method == AES_NONE); // when encryption is requested the MODULE_D7AP_NLS_ENABLED cmake option should be set
#endif

    switch_state(D7ANP_STATE_TRANSMIT);
    dll_tx_frame(packet);
    return SUCCESS;
}

static void schedule_foreground_scan_after_D7AAdvP(timer_tick_t eta)
{
    DPRINT("Perform a dll foreground scan at the end of the delay period (%i ticks)", eta);
    d7anp_start_fg_scan_after_d7aadvp_timer.next_event = eta;
    error_t rtc = timer_add_event(&d7anp_start_fg_scan_after_d7aadvp_timer);
    assert(rtc == SUCCESS);
}

#if defined(MODULE_D7AP_NLS_ENABLED)
static inline void write_be32(uint8_t *buf, uint32_t val)
{
    buf[0] = (val >> 24) & 0xff;
    buf[1] = (val >> 16) & 0xff;
    buf[2] = (val >> 8) & 0xff;
    buf[3] = val & 0xff;
}

static inline uint32_t read_be32(const uint8_t *buf)
{
    return ((uint32_t) buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

static void build_header(packet_t *packet, uint8_t payload_len, uint8_t *header)
{
    /*
     * According DASH7 specification, this block is defined  as (LSB first):
     * B_0: Flags | NLS Method | Zeros padding | Origin ID | Control extension | Payload length
     */

    memset(header, 0, AES_BLOCK_SIZE); // for zero padding

    /* the CBC-MAC header is defined according Table 7.4.6.1 */
    header[0] = SET_NLS_METHOD(packet->d7anp_ctrl.nls_method);
    memcpy( header + 6, packet->origin_access_id, packet->d7anp_ctrl.origin_id_type == ID_TYPE_VID? 2 : 8);
    header[14] = packet->d7anp_ctrl.raw;
    header[15] = payload_len;

    DPRINT("Header for CBC-MAC");
    DPRINT_DATA(header, AES_BLOCK_SIZE);
}

static void build_iv(packet_t *packet, uint8_t payload_len, uint8_t *iv)
{
    /*
     * According DASH7 specification, the initialization vector is defined  as (LSB first):
     * IV: Block counter | NLS Method | Key counter | Frame counter | Origin ID | Control extension | Payload length
     */

    memset(iv, 0, AES_BLOCK_SIZE);

    /* AES-CTR/AES-CCM Initialization Vector (IV)*/
    iv[0] = SET_NLS_METHOD(packet->d7anp_ctrl.nls_method);
    iv[1] = packet->d7anp_security.key_counter;
    write_be32(&iv[2], packet->d7anp_security.frame_counter);
    /* When Origin ID is not provided in the NWL frame, it is provided by upper layer.*/
    memcpy( iv + 6, packet->origin_access_id, packet->d7anp_ctrl.origin_id_type == ID_TYPE_VID? 2 : 8);
    iv[14] = packet->d7anp_ctrl.raw;
    iv[15] = payload_len;

#if defined(FRAMEWORK_AES_LOG_ENABLED)
    DPRINT("iv for CTR/CCM");
    DPRINT_DATA(iv, AES_BLOCK_SIZE);
#endif
}

uint8_t d7anp_secure_payload(packet_t *packet, uint8_t *payload, uint8_t payload_len)
{
    uint8_t nls_method;
    uint8_t ctr_blk[AES_BLOCK_SIZE];
    uint8_t header[AES_BLOCK_SIZE];
    uint8_t auth[AES_BLOCK_SIZE];
    uint8_t auth_len;
    uint8_t add[AES_BLOCK_SIZE];
    uint8_t add_len = 0;

    DPRINT("Start Secure payload (len %d) ", payload_len );
    timer_tick_t time_elapsed = timer_get_counter_value();


    nls_method = packet->d7anp_ctrl.nls_method;
    auth_len = get_auth_len(nls_method);

    /* When unicast access, add the auxiliary authentication data composed of the destination address */
    if(auth_len && !ID_TYPE_IS_BROADCAST(packet->d7anp_addressee->ctrl.id_type))
    {
        add_len = packet->d7anp_addressee->ctrl.id_type == ID_TYPE_VID ? 2 : 8;
        memcpy(add, packet->d7anp_addressee->id, add_len);
    }

    switch (nls_method)
    {
    case AES_CTR:
        // Build the initial counter block
        build_iv(packet, payload_len, ctr_blk);

        // the encrypted payload replaces the plaintext
        AES128_CTR_encrypt(payload, payload, payload_len, ctr_blk);
        break;
    case AES_CBC_MAC_128:
    case AES_CBC_MAC_64:
    case AES_CBC_MAC_32:
        /* Build the header block to prepend to the payload */
        build_header(packet, payload_len, header);

        /* Set Header flags */
        header[0] |= ( add_len > 0 );

        /* Compute the CBC-MAC */
        AES128_CBC_MAC(auth, payload, payload_len, header, add, add_len, auth_len);

        /* Insert the authentication Tag */
        memcpy(payload + payload_len, auth, auth_len);
        break;
    case AES_CCM_128:
    case AES_CCM_64:
    case AES_CCM_32:
        /*
         * For CCM, the same IV is used for the header block and the counter block
         * Bits 0-3 are set with the flags in AES-CCM header whereas they are set
         * to the Block counter for the CTR block*/
        build_iv(packet, payload_len, header);
        memcpy(ctr_blk, header, AES_BLOCK_SIZE);

        /* Set Header flags */
        header[0] |= ( add_len > 0 );

        // TODO check that the payload length does not exceed the maximum size
        AES128_CCM_encrypt(payload, payload_len, header, add, add_len, ctr_blk, auth_len);
        break;
    }

    time_elapsed = timer_get_counter_value() - time_elapsed;
    DPRINT("Payload secured in %i Ti", time_elapsed);
    return auth_len;
}

bool d7anp_unsecure_payload(packet_t *packet, uint8_t index)
{
    uint8_t nls_method;
    uint8_t ctr_blk[AES_BLOCK_SIZE];
    uint8_t header[AES_BLOCK_SIZE];
    uint8_t auth[AES_BLOCK_SIZE];
    uint8_t auth_len;
    uint32_t payload_len;
    uint8_t *tag;
    uint8_t add[AES_BLOCK_SIZE];
    uint8_t add_len = 0;

    nls_method = packet->d7anp_ctrl.nls_method;

    if(packet->hw_radio_packet.length < (index + CRC_SIZE))
    {
        return false;
    }
    //this said payload_len = packet->hw_radio_packet.length + 1 - index - CRC_SIZE; but I don't know why we would add 1 and it seems to only work without it.
    payload_len = packet->hw_radio_packet.length - index - CRC_SIZE; // exclude the headers CRC bytes // TODO exclude footers
    auth_len = get_auth_len(nls_method); // the authentication length is given in bytes
    if(payload_len < auth_len)
    {
        return false;
    }

    /* remove the authentication tag from the payload length if relevant */
    payload_len -= auth_len;

    if (auth_len)
    {
        tag = packet->hw_radio_packet.data + index + payload_len;
        DPRINT("Tag  <%d>", auth_len);
        DPRINT_DATA(tag, auth_len);

        /* For unicast access, an additional authentication data is used by CBC-MAC */
        if(packet->dll_header.control_target_id_type == ID_TYPE_UID)
        {
            d7ap_fs_read_uid(add);
            add_len = 8;
        }
        else if(packet->dll_header.control_target_id_type == ID_TYPE_VID)
        {
            d7ap_read_vid(add);
            add_len = 2;
        }
    }

    switch (nls_method)
    {
    case AES_CTR:
        /* Build the initial counter block */
        build_iv(packet, payload_len, ctr_blk);

        // the decrypted payload replaces the encrypted data
        AES128_CTR_encrypt(packet->hw_radio_packet.data + index,
                           packet->hw_radio_packet.data + index,
                           payload_len, ctr_blk);
        break;
    case AES_CBC_MAC_128:
    case AES_CBC_MAC_64:
    case AES_CBC_MAC_32:
        /* Build the header block to prepend to the payload */
        build_header(packet, payload_len, header);

        /* Set Header flags */
        header[0] |= ( add_len > 0 );

        /* Compute the CBC-MAC and check the authentication Tag */
        AES128_CBC_MAC(auth, packet->hw_radio_packet.data + index,
                       payload_len, header, add, add_len, auth_len);

        if (memcmp(auth, tag, auth_len) != 0)
        {
            DPRINT("CBC-MAC: Auth mismatch");
            return false;
        }
        /* remove the authentication Tag */
        packet->hw_radio_packet.length -= auth_len;

        break;
    case AES_CCM_128:
    case AES_CCM_64:
    case AES_CCM_32:
        /* For CCM, the same IV is used for the header block and the counter block */
        build_iv(packet, payload_len, header);
        memcpy(ctr_blk, header, AES_BLOCK_SIZE);

        /* Set Header flags */
        header[0] |= ( add_len > 0 );

        if (AES128_CCM_decrypt(packet->hw_radio_packet.data + index,
                               payload_len, header, add, add_len, ctr_blk,
                               tag, auth_len) != 0)
            return false;

        /* remove the authentication Tag */
        packet->hw_radio_packet.length -= auth_len;
    }

    return true;
}
#endif

uint8_t d7anp_assemble_packet_header(packet_t *packet, uint8_t *data_ptr)
{
    assert(!packet->d7anp_ctrl.hop_enabled); // TODO hopping not yet supported

    uint8_t* d7anp_header_start = data_ptr;
    (*data_ptr) = packet->d7anp_ctrl.raw; data_ptr++;

    if (!packet->d7anp_ctrl.origin_void)
    {
        (*data_ptr) = packet->origin_access_class; data_ptr++;

        if (packet->d7anp_ctrl.origin_id_type == ID_TYPE_UID)
        {
            memcpy(packet->origin_access_id, address_id, 8);
            memcpy(data_ptr, address_id, 8);
            data_ptr += 8;
        }
        else if (packet->d7anp_ctrl.origin_id_type == ID_TYPE_VID)
        {
            memcpy(packet->origin_access_id, address_id, 2);
            memcpy(data_ptr, address_id, 2);
            data_ptr += 2;
        }
        else if (packet->d7anp_ctrl.origin_id_type == ID_TYPE_NBID)
        {
            (*data_ptr) = packet->origin_access_id[0]; data_ptr++;
            // who set the NBID?
        }
    }

    // TODO hopping ctrl

#if defined(MODULE_D7AP_NLS_ENABLED)
    if (packet->d7anp_ctrl.nls_method == AES_CTR ||
        packet->d7anp_ctrl.nls_method == AES_CCM_32 ||
        packet->d7anp_ctrl.nls_method == AES_CCM_64 ||
        packet->d7anp_ctrl.nls_method == AES_CCM_128)
    {
        (*data_ptr) = packet->d7anp_security.key_counter; data_ptr++;
        write_be32(data_ptr, packet->d7anp_security.frame_counter);
        data_ptr += sizeof(uint32_t);
    }
#endif

    return data_ptr - d7anp_header_start;
}

#if defined(MODULE_D7AP_NLS_ENABLED)
dae_nwl_trusted_node_t *get_trusted_node(uint8_t *address)
{
    //look up the sender's address in the trusted node table
    for(uint8_t i = 0; i < node_security_state.trusted_node_nb; i++)
    {
        if(memcmp(node_security_state.trusted_node_table[i].addr, address, 8) == 0)
            return &(node_security_state.trusted_node_table[i]);
    }

    return NULL;
}

dae_nwl_trusted_node_t *add_trusted_node(uint8_t *address, uint32_t frame_counter,
                                       uint8_t key_counter)
{
    uint8_t index = node_security_state.trusted_node_nb;
    dae_nwl_trusted_node_t *node;

    if (node_security_state.trusted_node_nb < FRAMEWORK_FS_TRUSTED_NODE_TABLE_SIZE)
        node_security_state.trusted_node_nb++;
    else
    {
        DPRINT("SSR is full !");
        return NULL;
    }

    node = &node_security_state.trusted_node_table[index];
    memcpy(node->addr, address, 8);
    node->frame_counter = frame_counter;
    node->key_counter = key_counter;

    DPRINT("Add node <%p> total number <%d>", node, node_security_state.trusted_node_nb);
    /* Update the FS */
    d7ap_fs_add_nwl_security_state_register_entry(node, node_security_state.trusted_node_nb);
    return node;
}
#endif

bool d7anp_disassemble_packet_header(packet_t* packet, uint8_t *data_idx)
{
    if(packet->hw_radio_packet.length < (*data_idx + 1))
    {
        return false;
    }
    packet->d7anp_ctrl.raw = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;

    if (!packet->d7anp_ctrl.origin_void)
    {
        if(packet->hw_radio_packet.length < (*data_idx + 1))
        {
            return false;
        }
        packet->origin_access_class = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;

        if (!ID_TYPE_IS_BROADCAST(packet->d7anp_ctrl.origin_id_type))
        {
            uint8_t origin_access_id_size = packet->d7anp_ctrl.origin_id_type == ID_TYPE_VID? 2 : 8;
            if(packet->hw_radio_packet.length < (*data_idx + origin_access_id_size))
            {
                return false;
            }
            memcpy(packet->origin_access_id, packet->hw_radio_packet.data + (*data_idx), origin_access_id_size); (*data_idx) += origin_access_id_size;
        }
        else if (packet->d7anp_ctrl.origin_id_type == ID_TYPE_NBID)
        {
            if(packet->hw_radio_packet.length < (*data_idx + 1))
            {
                return false;
            }
            packet->origin_access_id[0] = packet->hw_radio_packet.data[(*data_idx)];
            (*data_idx)++;
        }
    }

    // TODO hopping

#if defined(MODULE_D7AP_NLS_ENABLED)
    if (packet->d7anp_ctrl.nls_method)
    {
        dae_nwl_trusted_node_t *node;
        uint8_t nls_method = packet->d7anp_ctrl.nls_method;
        bool create_node = false;
        bool prevent_replay_attack = false;

        DPRINT("Received nls method %d", nls_method);

        if (nls_method == AES_CTR || nls_method == AES_CCM_32 ||
            nls_method == AES_CCM_64 || nls_method == AES_CCM_128)
        {
            if(packet->hw_radio_packet.length < (*data_idx + 1 + sizeof(uint32_t)))
            {
                return false;
            }
            // extract the key counter and the frame counter
            packet->d7anp_security.key_counter = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
            packet->d7anp_security.frame_counter = read_be32(packet->hw_radio_packet.data + (*data_idx));
            (*data_idx) += sizeof(uint32_t);

            DPRINT("Received key counter <%d>, frame counter <%ld>", packet->d7anp_security.key_counter, packet->d7anp_security.frame_counter);

            if (node_security_state.filter_mode & ENABLE_SSR_FILTER)
                prevent_replay_attack = true;
        }

        if (prevent_replay_attack)
        {
            /* When Origin ID is not provided, try to use the latest node */
            if (ID_TYPE_IS_BROADCAST(packet->d7anp_ctrl.origin_id_type))
            {
                // frame is not accepted if the Origin ID is really unknown
                if (!latest_node)
                     return false;

                node = latest_node;
            }
            else
                node = get_trusted_node(packet->origin_access_id);

            if (node && (node->frame_counter > packet->d7anp_security.frame_counter ||
                         node->frame_counter == (uint32_t)~0))
            {
                DPRINT("Replay attack detected cnt %ld->%ld shift back", node->frame_counter, packet->d7anp_security.frame_counter);
                return false;
            }

            // update the node
            if (node)
                node->frame_counter = packet->d7anp_security.frame_counter;
            else
            {
                if (ID_TYPE_IS_BROADCAST(packet->dll_header.control_target_id_type) &&
                     !(node_security_state.filter_mode & ALLOW_NEW_SSR_ENTRY_IN_BCAST))
                {
                    DPRINT("New SSR entry not authorized in broadcast");
                    return false;
                }
                else
                    create_node = true;
            }
        }

        if (!d7anp_unsecure_payload(packet, *data_idx))
            return false;

        if (create_node)
             add_trusted_node(packet->origin_access_id, packet->d7anp_security.frame_counter,
                              packet->d7anp_security.key_counter);
    }
#endif

    assert(!packet->d7anp_ctrl.hop_enabled); // TODO hopping not yet supported

    return true;
}

void d7anp_signal_transmission_failure()
{
    assert(d7anp_state == D7ANP_STATE_TRANSMIT);

    DPRINT("CSMA-CA insertion failed");

    // switch back to the previous state before the transmission
    switch_state(d7anp_prev_state);

    d7atp_signal_transmission_failure();
}

void d7anp_signal_packet_transmitted(packet_t* packet)
{
    assert(d7anp_state == D7ANP_STATE_TRANSMIT);

    /* switch back to the same state as before the transmission */
    switch_state(d7anp_prev_state);
    d7atp_signal_packet_transmitted(packet);

}

void d7anp_process_received_packet(packet_t* packet)
{
    // TODO handle case where we are intermediate node while hopping (ie start FG scan, after auth if needed, and return)

    if (d7anp_state == D7ANP_STATE_FOREGROUND_SCAN)
    {
        DPRINT("Received packet while in D7ANP_STATE_FOREGROUND_SCAN");
    }
    else if (d7anp_state == D7ANP_STATE_IDLE)
    {
        DPRINT("Received packet while in D7ANP_STATE_IDLE (scan automation)");

        // check if DLL was performing a background scan
        if (packet->type == BACKGROUND_ADV)
        {
            timer_tick_t time_elapsed = timer_get_counter_value() - packet->hw_radio_packet.rx_meta.timestamp;
            if (packet->ETA >= time_elapsed + FG_SCAN_STARTUP_TIME)
            {
                // TODO assert FG_SCAN_STARTUP_TIME + FG_SCAN_START_BEFORE_ETA_SAFETY_MARGIN < BG frame tx time,
                // or else we risk missing packets
                uint16_t delay = packet->ETA - time_elapsed - FG_SCAN_STARTUP_TIME;
                if(delay <= CLK_ACCURACY_100_MS * (packet->ETA/100 + 1))
                  delay = 0;
                else
                  delay -= CLK_ACCURACY_100_MS * (packet->ETA/100 + 1);

                DPRINT("FG scan start after %d", delay);
                DPRINT("FG scan start after %d - (%d + %d + %d)", packet->ETA, time_elapsed, FG_SCAN_STARTUP_TIME, CLK_ACCURACY_100_MS * (packet->ETA/100 + 1));
                schedule_foreground_scan_after_D7AAdvP(delay);
                // meanwhile stay in idle
                dll_stop_background_scan();
            }
            else
            {
                DPRINT("No time to switch to FG scan because ETA is too short %i", packet->ETA);
            }

            packet_queue_free_packet(packet);
            return;
        }
    }
    else
        assert(false);

    d7atp_process_received_packet(packet);
}


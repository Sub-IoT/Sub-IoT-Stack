/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2017 CORTUS
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
 *
 * @author philippe.nunes@cortus.com
 */

#include <stdlib.h>
#include "stdbool.h"
#include "string.h"
#include "types.h"
#include "math.h"

#include "debug.h"
#include "log.h"
#include "scheduler.h"

#include "net/netdev.h"
#include "hwdebug.h"
#include "phy.h"

#include "crc.h"
#include "pn9.h"
#include "fec.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_PHY_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_PHY, __VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_PACKET(...)
#define DPRINT_DATA(...)
#endif

#if PLATFORM_NUM_DEBUGPINS >= 2
    #define DEBUG_TX_START() hw_debug_set(0);
    #define DEBUG_TX_END() hw_debug_clr(0);
    #define DEBUG_RX_START() hw_debug_set(1);
    #define DEBUG_RX_END() hw_debug_clr(1);
#else
    #define DEBUG_TX_START()
    #define DEBUG_TX_END()
    #define DEBUG_RX_START()
    #define DEBUG_RX_END()
#endif

// modulation settings

// low rate
#define BITRATE_L 9600 // bps
#define FDEV_L    4800 // Hz
// Carson's rule: 2 x fm + 2 x fd  = 9.600 + 2 x 4.800 = 19.2 kHz
// assuming 10 ppm crystals gives max error of: 2 * 10 ppm * 868 = 17.36 kHz
// => BW > 19.2 + 17.36 kHz => > 36.5 kHZ.
#define RXBW_L    36500 //Hz

// normal rate
#define BITRATE_N 55555 // bps
#define FDEV_N    50000 // Hz
// Carson's rule: 2 x fm + 2 x fd  = 55.555 + 2 x 50 = 155.555 kHz
// assuming 10 ppm crystals gives max error of: 2 * 10 ppm * 868 = 17.36 kHz
// => BW > 155.555 + 17.36 => 172.91 KHz
#define RXBW_N   172910 //Hz

// high rate
#define BITRATE_H 166667 // bps
#define FDEV_H     41667 // Hz
// Carson's rule: 2 x fm + 2 x fd  = 166.667 + 2 x 41.667 = 250 kHz
// assuming 10 ppm crystals gives max error of: 2 * 10 ppm * 868 = 17.36 kHz
// => BW > 250 + 17.36 kHz => > 267.36 kHZ.
#define RXBW_H    267360 //Hz

typedef enum {
  STATE_IDLE,
  STATE_TX,
  STATE_RX,
  STATE_BG_SCAN
} state_t;

static netdev_t *netdev;

static alloc_packet_callback_t alloc_packet_callback;
static release_packet_callback_t release_packet_callback;
static rx_packet_callback_t rx_packet_callback;
static tx_packet_callback_t tx_packet_callback;
static rssi_valid_callback_t rssi_valid_callback;

static state_t state = STATE_IDLE;
static hw_radio_packet_t* current_packet;
static bool should_rx_after_tx_completed = false;
static syncword_class_t current_syncword_class = PHY_SYNCWORD_CLASS0;
static hw_rx_cfg_t pending_rx_cfg;

static channel_id_t default_channel_id = {
  .channel_header.ch_coding = PHY_CODING_PN9,
  .channel_header.ch_class = PHY_CLASS_NORMAL_RATE,
  .channel_header.ch_freq_band = PHY_BAND_868,
  .center_freq_index = 0
};

#define EMPTY_CHANNEL_ID { .channel_header_raw = 0xFF, .center_freq_index = 0xFF };

static channel_id_t current_channel_id = EMPTY_CHANNEL_ID;

/*
 * FSK packet handler structure
 */
typedef struct
{
    uint8_t Size;
    uint8_t NbBytes;
    uint8_t FifoThresh;
}FskPacketHandler_t;

FskPacketHandler_t FskPacketHandler;

/*
 * Background advertising packet handler structure
 */
typedef struct
{
    uint8_t dll_header[BACKGROUND_DLL_HEADER_LENGTH];
    uint8_t packet[24];  // 6 bytes preamble (PREAMBLE_HI_RATE_CLASS) + 2 bytes SYNC word + 16 bytes max for a background frame FEC encoded
    uint8_t *packet_payload;
    uint8_t packet_size;
    uint16_t eta;
    uint16_t tx_duration;
    timer_tick_t stop_time;
}bg_adv_t;

bg_adv_t bg_adv;

typedef struct
{
    uint8_t encoded_length;
    uint8_t encoded_packet[PREAMBLE_HI_RATE_CLASS + 2 + 256]; // include space for preamble and syncword
    uint8_t transmitted_index;
    bool bg_adv;
}fg_frame_t;

fg_frame_t fg_frame;

const uint16_t sync_word_value[2][4] = {
    { 0xE6D0, 0x0000, 0xF498, 0x0000 },
    { 0x0B67, 0x0000, 0x192F, 0x0000 }
};

/*
 * In background scan, the device has a period of To to successfully detect the sync word
 * To is at least equal to the duration of one background frame plus the duration
 * of the maximum preamble length for the used channel class.
 */
#define To_CLASS_LO_RATE 22 // (12(FEC encode payload) + 2 (SYNC) + 8 (Max preamble)) / 1 byte/tick
#define To_CLASS_NORMAL_RATE 4 // (12(FEC encode payload) + 2 (SYNC) + 8 (Max preamble)) / 6 bytes/tick
#define To_CLASS_HI_RATE 2 // (12(FEC encode payload) + 2 (SYNC) + 16 (Max preamble)) / 20 bytes/tick

const uint8_t bg_timeout[4] = {
    To_CLASS_LO_RATE,
    0, // RFU
    To_CLASS_NORMAL_RATE,
    To_CLASS_HI_RATE
};

static void fill_in_fifo();
static error_t hw_radio_send_packet_with_advertising(uint8_t dll_header_bg_frame[2], uint16_t tx_duration_bg_frame, uint16_t eta, hw_radio_packet_t* packet);

bool phy_radio_channel_ids_equal(const channel_id_t* a, const channel_id_t* b)
{
    //return memcmp(a,b, sizeof(channel_id_t)) == 0; //not working since channel_id_t not packed
    return (a->channel_header_raw == b->channel_header_raw) && (a->center_freq_index == b->center_freq_index);
}

uint16_t phy_calculate_tx_duration(phy_channel_class_t channel_class, phy_coding_t ch_coding, uint8_t packet_length)
{
    double data_rate = 6.0; // Normal rate: 6.9 bytes/tick

    if (ch_coding == PHY_CODING_FEC_PN9)
        packet_length = fec_calculated_decoded_length(packet_length);

    packet_length += sizeof(uint16_t); // Sync word

    switch (channel_class)
    {
    case PHY_CLASS_LO_RATE:
        packet_length += PREAMBLE_LOW_RATE_CLASS;
        data_rate = 1.0; // Lo Rate 9.6 kbps: 1.2 bytes/tick
        break;
    case PHY_CLASS_NORMAL_RATE:
        packet_length += PREAMBLE_NORMAL_RATE_CLASS;
        data_rate = 6.0; // Normal Rate 55.555 kbps: 6.94 bytes/tick
        break;
    case PHY_CLASS_HI_RATE:
        packet_length += PREAMBLE_HI_RATE_CLASS;
        data_rate = 20.0; // High rate 166.667 kbps: 20.83 byte/tick
        break;
#ifdef USE_SX127X
    case PHY_CLASS_LORA:
      assert(false); // TODO
#endif
    }

    // TODO Add the power ramp-up/ramp-down symbols in the packet length?
    return ceil(packet_length / data_rate) + 1;
}


static netopt_state_t hw_radio_get_opmode() {
    netopt_state_t opmode;

    netdev->driver->get(netdev, NETOPT_STATE, &opmode, sizeof(netopt_state_t));
    return opmode;
}

static void hw_radio_set_opmode(netopt_state_t opmode) {
    netdev->driver->set(netdev, NETOPT_STATE, &opmode, sizeof(netopt_state_t));
}

static void hw_radio_set_center_freq(uint32_t center_freq)
{
    DPRINT("Set center frequency: %d\n", center_freq);

    netdev->driver->set(netdev, NETOPT_CHANNEL, &center_freq, sizeof(uint32_t));
}

static void hw_radio_set_rx_bw_hz(uint32_t bw_hz)
{
    netdev->driver->set(netdev, NETOPT_BANDWIDTH, &bw_hz, sizeof(uint32_t));
}

static void hw_radio_set_bitrate(uint32_t bps)
{
    netdev->driver->set(netdev, NETOPT_BITRATE, &bps, sizeof(uint32_t));
}

static void hw_radio_set_tx_fdev(uint32_t fdev)
{
    netdev->driver->set(netdev, NETOPT_FDEV, &fdev, sizeof(uint32_t));
}

static void hw_radio_set_preamble_size(uint16_t size)
{
    netdev->driver->set(netdev, NETOPT_PREAMBLE_LENGTH, &size, sizeof(uint16_t));
}

/* TODO Make use of the following APIs to setup the xcvr */
/*
static void hw_radio_set_modulation_shaping(uint8_t shaping)
{
    netdev->driver->set(netdev, NETOPT_MOD_SHAPING, &shaping, sizeof(uint8_t));
}

static void hw_radio_set_preamble_polarity(uint8_t polarity)
{
    netdev->driver->set(netdev, NETOPT_PREAMBLE_POLARITY, &polarity, sizeof(uint8_t));
}

static void hw_radio_set_rssi_threshold(uint8_t rssi_thr)
{
    netdev->driver->set(netdev, NETOPT_CCA_THRESHOLD, &rssi_thr, sizeof(uint8_t));
}

static void hw_radio_set_rssi_smoothing(uint8_t rssi_samples)
{
    netdev->driver->set(netdev, NETOPT_RSSI_SMOOTHING, &rssi_samples, sizeof(uint8_t));
}

static void hw_radio_set_sync_word_size(uint8_t sync_size)
{
    netdev->driver->set(netdev, NETOPT_SYNC_LENGTH, &sync_size, sizeof(uint8_t));
}

static void hw_radio_set_sync_on(uint8_t enable)
{
    netdev->driver->set(netdev, NETOPT_SYNC_ON, &enable, sizeof(uint8_t));
}

static void hw_radio_set_preamble_detect_on(uint8_t enable)
{
    netdev->driver->set(netdev, NETOPT_PREAMBLE_DETECT_ON, &enable, sizeof(uint8_t));
}
*/

static void hw_radio_set_dc_free(uint8_t scheme)
{
    netdev->driver->set(netdev, NETOPT_DC_FREE_SCHEME, &scheme, sizeof(uint8_t));
}

static void hw_radio_set_sync_word(uint8_t *sync_word, uint8_t sync_size)
{
    //uint8_t sync_size = read_reg(REG_SYNCCONFIG) & ~RF_SYNCCONFIG_SYNCSIZE_MASK;
    assert(sync_size >= 1);

    netdev->driver->set(netdev, NETOPT_SYNC_WORD, sync_word, sync_size);
}

static void hw_radio_set_crc_on(uint8_t enable)
{
    netdev->driver->set(netdev, NETOPT_INTEGRITY_CHECK, &enable, sizeof(uint8_t));
}

int16_t hw_radio_get_rssi()
{
    int16_t rssi;

    netdev->driver->get(netdev, NETOPT_RSSI_VALUE, (uint16_t *)&rssi, sizeof(uint16_t));
    return rssi;
}

void hw_radio_set_payload_length(uint8_t length)
{
    netdev->driver->set(netdev, NETOPT_MAX_PACKET_SIZE, &length, sizeof(uint8_t));
}

void hw_radio_send_payload(uint8_t * data, uint8_t len)
{
    struct iovec vec[1];
    vec[0].iov_base = data;
    vec[0].iov_len = len;

    if (netdev->driver->send(netdev, vec, 1) == -ENOTSUP)
    {
        DPRINT("Cannot send: radio is still transmitting");
    }
}

static void configure_eirp(eirp_t eirp)
{
    DPRINT("Set Tx power: %d dBm\n", eirp);

    netdev->driver->set(netdev, NETOPT_TX_POWER, &eirp, sizeof(uint8_t));
}

static void configure_channel(const channel_id_t* channel) {
    if(phy_radio_channel_ids_equal(&current_channel_id, channel)) {
        return;
    }

    // configure modulation settings
    if(channel->channel_header.ch_class == PHY_CLASS_LO_RATE)
    {
        hw_radio_set_bitrate(BITRATE_L);
        hw_radio_set_tx_fdev(FDEV_L);
        hw_radio_set_rx_bw_hz(RXBW_L);
        hw_radio_set_preamble_size(PREAMBLE_LOW_RATE_CLASS * 8);
    }
    else if(channel->channel_header.ch_class == PHY_CLASS_NORMAL_RATE)
    {
        hw_radio_set_bitrate(BITRATE_N);
        hw_radio_set_tx_fdev(FDEV_N);
        hw_radio_set_rx_bw_hz(RXBW_N);
        hw_radio_set_preamble_size(PREAMBLE_NORMAL_RATE_CLASS * 8);
    }
    else if(channel->channel_header.ch_class == PHY_CLASS_HI_RATE)
    {
        hw_radio_set_bitrate(BITRATE_H);
        hw_radio_set_tx_fdev(FDEV_H);
        hw_radio_set_rx_bw_hz(RXBW_H);
        hw_radio_set_preamble_size(PREAMBLE_HI_RATE_CLASS * 8);
    }

    // TODO regopmode for LF?

    uint32_t center_freq = 433.06e6;
    if(channel->channel_header.ch_freq_band == PHY_BAND_868)
        center_freq = 863e6;
    else if(channel->channel_header.ch_freq_band == PHY_BAND_915)
        center_freq = 902e6;

    uint32_t channel_spacing_half = 100e3;
    if(channel->channel_header.ch_class == PHY_CLASS_LO_RATE)
        channel_spacing_half = 12500;

    center_freq += 25000 * channel->center_freq_index + channel_spacing_half;
    hw_radio_set_center_freq(center_freq);

    current_channel_id = *channel;
    DPRINT("set channel_header %i, channel_band %i, center_freq_index %i\n",
           current_channel_id.channel_header_raw,
           current_channel_id.channel_header.ch_freq_band,
           current_channel_id.center_freq_index);
}

static void configure_syncword(syncword_class_t syncword_class, const channel_id_t* channel)
{
    if(syncword_class != current_syncword_class || (channel->channel_header.ch_coding != current_channel_id.channel_header.ch_coding))
    {
        current_syncword_class = syncword_class;
        uint16_t sync_word = sync_word_value[syncword_class][channel->channel_header.ch_coding ];

        DPRINT("sync_word = %04x", sync_word);
        hw_radio_set_sync_word((uint8_t *)&sync_word, sizeof(uint16_t));
    }
}

static void start_rx(hw_rx_cfg_t const* rx_cfg) {
    configure_channel(&(rx_cfg->channel_id));
    configure_syncword(rx_cfg->syncword_class, &(rx_cfg->channel_id));

    hw_radio_set_payload_length(0x00); // unlimited length mode

    if(rx_packet_callback != 0) {
        // enable the interrupt after after the preamble is correctly received
        netopt_enable_t enable = NETOPT_ENABLE;
        netdev->driver->set(netdev, NETOPT_RX_START_IRQ, &enable, sizeof(netopt_enable_t));
    } else {
        // when rx callback not set we ignore received packets
        netopt_enable_t enable = NETOPT_DISABLE;
        netdev->driver->set(netdev, NETOPT_RX_START_IRQ, &enable, sizeof(netopt_enable_t));
        netdev->driver->set(netdev, NETOPT_RX_END_IRQ, &enable, sizeof(netopt_enable_t));
    }

    DPRINT("START FG scan @ %i", timer_get_counter_value());
    DEBUG_RX_START();

    state = STATE_RX;
    hw_radio_set_opmode(NETOPT_STATE_RX);

    if(rssi_valid_callback != 0)
    {
        rssi_valid_callback(hw_radio_get_rssi());
    }
}

static void switch_to_standby_mode()
{
    hw_radio_set_opmode(NETOPT_STATE_STANDBY);
    state = STATE_IDLE;
}

static void isr_handler(void *arg) {
    DPRINT("Netdev ISR handler outside the IRQ context");
    netdev->driver->isr(netdev);
}

static void _event_cb(netdev_t *dev, netdev_event_t event)
{
    if (event == NETDEV_EVENT_ISR) {
        sched_post_task_prio(isr_handler, MAX_PRIORITY);
    }
    else {
        int len = 0;
        netdev_radio_rx_info_t packet_info;
        uint8_t *data;
        switch (event) {
            case NETDEV_EVENT_RX_COMPLETE:
                len = dev->driver->recv(netdev, NULL, 0, 0);
                current_packet = alloc_packet_callback(len);

                if (state == STATE_BG_SCAN)
                {
                    current_packet->length = BACKGROUND_FRAME_LENGTH;
                    data = current_packet->data + 1;
                }
                else
                    data = current_packet->data;
                dev->driver->recv(netdev, data, len, &packet_info);
                DPRINT("RX done\n");
                DPRINT("Payload: %d bytes, RSSI: %i, LQI: %i" /*SNR: %i, TOA: %i}\n"*/,
                        len, packet_info.rssi, packet_info.lqi/*(int)packet_info.snr,
                       (int)packet_info.time_on_air*/);
                DPRINT_DATA(data, len);

#ifndef HAL_RADIO_USE_HW_DC_FREE
                pn9_encode(data, len);
#endif
#ifndef HAL_RADIO_USE_HW_FEC
                if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
                    fec_decode_packet(data, len, len);
#endif

                current_packet->rx_meta.timestamp = timer_get_counter_value();
                current_packet->rx_meta.rx_cfg.syncword_class = current_syncword_class;
                current_packet->rx_meta.crc_status = HW_CRC_UNAVAILABLE;
                current_packet->rx_meta.rssi = packet_info.rssi;
                current_packet->rx_meta.lqi = packet_info.lqi;
                memcpy(&(current_packet->rx_meta.rx_cfg.channel_id), &current_channel_id, sizeof(channel_id_t));

                rx_packet_callback(current_packet);

                if (state == STATE_RX)
                {
                    // Restart the reception until upper layer decides to stop it
                    hw_radio_set_opmode(NETOPT_STATE_RX);
                }
                else
                    switch_to_standby_mode();
                break;
            case NETDEV_EVENT_TX_COMPLETE:
                DPRINT("Transmission completed");
                DEBUG_TX_END();
                state = STATE_IDLE;
                if(tx_packet_callback) {
                  current_packet->tx_meta.timestamp = timer_get_counter_value();
                  tx_packet_callback(current_packet);
                }
                break;
            case NETDEV_EVENT_TX_REFILL_NEEDED:
                 DPRINT("New data needed to transmit without discontinuity");
                 fill_in_fifo(dev);
                 break;
            case NETDEV_EVENT_RX_STARTED: {
                uint8_t buffer[4];
                uint8_t packet_len;

                len = dev->driver->recv(netdev, buffer, sizeof(buffer), NULL);
                DPRINT("Packet Header received %i\n", len);
                DPRINT_DATA(buffer, len);

                assert(len == 4);

#ifndef HAL_RADIO_USE_HW_DC_FREE
                pn9_encode(buffer, len);
#endif

                DPRINT("Packet Header received after PN9 decoding %i\n", len);
                DPRINT_DATA(buffer, len);

                if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
                {
#ifndef HAL_RADIO_USE_HW_FEC
                    fec_decode_packet(buffer, len, len);
#endif
                    packet_len = fec_calculated_decoded_length(buffer[0] + 1);
                }
                else
                    packet_len = buffer[0] + 1 ;

                DPRINT("RX Packet Length: %i ", packet_len);
                // set PayloadLength to the length of the expected foreground frame
                hw_radio_set_payload_length(packet_len);
            } break;
            case NETDEV_EVENT_CAD_DONE:
                break;
            case NETDEV_EVENT_TX_TIMEOUT:
                //sx127x_set_sleep(&sx127x);
                break;
            default:
                DPRINT("Unexpected netdev event received: %d\n", event);
                break;
        }
    }
}



error_t hw_radio_init(alloc_packet_callback_t alloc_packet_cb, release_packet_callback_t release_packet_cb) {

    error_t ret;

    alloc_packet_callback = alloc_packet_cb;
    release_packet_callback = release_packet_cb;

    netdev = (netdev_t*) &xcvr;
    ret = netdev->driver->init(netdev);

#ifdef HAL_RADIO_USE_HW_CRC
    hw_radio_set_crc_on(true);
#else
    hw_radio_set_crc_on(false);
#endif

#ifdef HAL_RADIO_USE_HW_DC_FREE
    hw_radio_set_dc_free(NETOPT_DC_FREE_WHITENING);
#else
    hw_radio_set_dc_free(NETOPT_DC_FREE_NONE);
#endif

    configure_syncword(PHY_SYNCWORD_CLASS0, &default_channel_id);
    configure_channel(&default_channel_id);
    configure_eirp(10);

    netdev->event_callback = _event_cb;
    sched_register_task(&isr_handler, netdev);

    //hw_radio_set_opmode(OPMODE_STANDBY); --> done by the netdev driver
    //while(hw_radio_get_opmode() != OPMODE_STANDBY) {}

    return ret;
}

error_t hw_radio_set_idle() {
    // TODO Select the chip mode during Idle state (Standby mode or Sleep mode)

    // For now, select by default the standby mode
    switch_to_standby_mode();
    DEBUG_RX_END();
    DEBUG_TX_END();

    return SUCCESS;
}

bool hw_radio_is_idle() {
    if (hw_radio_get_opmode() == NETOPT_STATE_STANDBY)
        return true;
    else
        return false;
}

error_t hw_radio_set_rx(hw_rx_cfg_t const* rx_cfg, rx_packet_callback_t rx_cb, rssi_valid_callback_t rssi_valid_cb) {
  if(rx_cb) {
    assert(alloc_packet_callback != NULL);
    assert(release_packet_callback != NULL);
  }

   assert(rx_cb != NULL || rssi_valid_cb != NULL); // at least one callback should be valid

  // TODO error handling EINVAL, EOFF

  rx_packet_callback = rx_cb;
  rssi_valid_callback = rssi_valid_cb;

  // if we are currently transmitting wait until TX completed before entering RX
  // we return now and go into RX when TX is completed
  if(state == STATE_TX)
  {
    should_rx_after_tx_completed = true;
    memcpy(&pending_rx_cfg, rx_cfg, sizeof(hw_rx_cfg_t));
    return SUCCESS;
  }

  start_rx(rx_cfg);

  return SUCCESS;
}

bool hw_radio_is_rx() {
    if (hw_radio_get_opmode() == NETOPT_STATE_RX)
        return true;
    else
        return false;
}

static uint8_t encode_packet(hw_radio_packet_t* packet, uint8_t* encoded_packet)
{
    uint8_t encoded_len = packet->length + 1;
    memcpy(encoded_packet, packet->data, packet->length + 1);

#ifndef HAL_RADIO_USE_HW_FEC
    if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
        encoded_len = fec_encode(encoded_packet, packet->length + 1);
#endif

#ifndef HAL_RADIO_USE_HW_DC_FREE
    pn9_encode(encoded_packet, encoded_len);
#endif

    return encoded_len;
}

error_t hw_radio_send_packet(hw_radio_packet_t* packet, tx_packet_callback_t tx_callback, uint16_t eta, uint8_t dll_header_bg_frame[2])
{
    assert(packet->length < PACKET_MAX_SIZE);

    tx_packet_callback = tx_callback;
    current_packet = packet;

    if(state == STATE_RX)
    {
        pending_rx_cfg.channel_id = current_channel_id;
        pending_rx_cfg.syncword_class = current_syncword_class;
        should_rx_after_tx_completed = true;
        switch_to_standby_mode();
    }

    if (eta) {
        uint16_t tx_duration_bg_frame = phy_calculate_tx_duration(current_channel_id.channel_header.ch_class,
                                                                current_channel_id.channel_header.ch_coding,
                                                                BACKGROUND_FRAME_LENGTH);

        return hw_radio_send_packet_with_advertising(dll_header_bg_frame, tx_duration_bg_frame, eta, packet);
    }

    configure_channel((channel_id_t*)&(current_packet->tx_meta.tx_cfg.channel_id));
    configure_eirp(current_packet->tx_meta.tx_cfg.eirp);
    configure_syncword(current_packet->tx_meta.tx_cfg.syncword_class,
                       &(current_packet->tx_meta.tx_cfg.channel_id));

    state = STATE_TX;

    // Encode the packet if not supported by xcvr
    uint8_t encoded_packet[PACKET_MAX_SIZE + 1];
    uint8_t encoded_length = encode_packet(packet, encoded_packet);

    DPRINT("TX len=%i", encoded_length);
    DPRINT_DATA(encoded_packet, encoded_length);

    DEBUG_RX_END();
    DEBUG_TX_START();

    hw_radio_send_payload(encoded_packet, encoded_length);

    return SUCCESS; // TODO other return codes
}

static uint8_t assemble_background_payload()
{
    uint16_t crc, swap_eta;
    uint8_t payload_len;

    /*
     * Build the next advertising frame.
     * In order to flood the channel with advertising frames without discontinuity,
     * the FIFO is refilled with the next frames within the same TX.
     * For that, the preamble and the sync word are explicitly inserted before each
     * subsequent advertising frame.
     */

    memcpy(bg_adv.packet_payload, bg_adv.dll_header, BACKGROUND_DLL_HEADER_LENGTH);

    // add ETA for background frames
    //DPRINT("eta %i", bg_adv.eta);
    swap_eta = __builtin_bswap16(bg_adv.eta);
    memcpy(&bg_adv.packet_payload[BACKGROUND_DLL_HEADER_LENGTH], &swap_eta, sizeof(uint16_t));

    // add CRC
    crc = __builtin_bswap16(crc_calculate(bg_adv.packet_payload, 4));
    memcpy(&bg_adv.packet_payload[BACKGROUND_DLL_HEADER_LENGTH + sizeof(uint16_t)], &crc, 2);

    if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
    {
        payload_len = fec_encode(bg_adv.packet_payload, BACKGROUND_FRAME_LENGTH);
        pn9_encode(bg_adv.packet_payload, payload_len);
    }
    else
    {
        //DPRINT("assemble payload %d", BACKGROUND_FRAME_LENGTH);
        //DPRINT_DATA(bg_adv.packet_payload, BACKGROUND_FRAME_LENGTH);
        pn9_encode(bg_adv.packet_payload, BACKGROUND_FRAME_LENGTH);
        payload_len = BACKGROUND_FRAME_LENGTH;
    }

    return payload_len;
}

/** \brief Send a packet using background advertising
 *
 * Start a background frame flooding until expiration of the advertising period, followed by transmission
 * of the foreground frame.
 * Each background frame contains the Estimated Time of Arrival of the D7ANP Request (ETA).
 * When no more advertising background frames can be fully transmitted before the start of D7ANP,
 * the last background frame is extended by padding preamble symbols after the end of the background
 * packet, in order to guarantee no silence period on the channel between D7AAdvP and D7ANP.
 */
static error_t hw_radio_send_packet_with_advertising(uint8_t dll_header_bg_frame[2], uint16_t tx_duration_bg_frame, uint16_t eta, hw_radio_packet_t* packet)
{
    DPRINT("Start the bg advertising for ad-hoc sync before transmitting the FG frame");

    configure_syncword(PHY_SYNCWORD_CLASS0, (channel_id_t*)&(packet->tx_meta.tx_cfg.channel_id));
    configure_channel((channel_id_t*)&(packet->tx_meta.tx_cfg.channel_id));
    configure_eirp(packet->tx_meta.tx_cfg.eirp);

    // During the advertising flooding, use the infinite packet length mode
    hw_radio_set_payload_length(0x00); // unlimited length mode

    netopt_enable_t enable = NETOPT_ENABLE;
    netdev->driver->set(netdev, NETOPT_TX_REFILL_IRQ, &enable, sizeof(netopt_enable_t));
    netdev->driver->set(netdev, NETOPT_PRELOADING, &enable, sizeof(netopt_enable_t));

    // Prepare the subsequent background frames which include the preamble and the sync word
    uint8_t preamble_len = (current_channel_id.channel_header.ch_class ==  PHY_CLASS_HI_RATE ? PREAMBLE_HI_RATE_CLASS : PREAMBLE_LOW_RATE_CLASS);
    memset(bg_adv.packet, 0xAA, preamble_len); // preamble length is given in number of bytes
    uint16_t sync_word = __builtin_bswap16(sync_word_value[PHY_SYNCWORD_CLASS0][current_channel_id.channel_header.ch_coding]);
    memcpy(&bg_adv.packet[preamble_len], &sync_word, 2);

    if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
        bg_adv.packet_size = preamble_len + 2 + fec_calculated_decoded_length(BACKGROUND_FRAME_LENGTH);
    else
        bg_adv.packet_size = preamble_len + 2 + BACKGROUND_FRAME_LENGTH;

    bg_adv.packet_payload = bg_adv.packet + preamble_len + 2 ;

    // Backup the DLL header
    memcpy(bg_adv.dll_header, dll_header_bg_frame, BACKGROUND_DLL_HEADER_LENGTH);
    DPRINT("DLL header followed by ETA %i", eta);
    DPRINT_DATA(bg_adv.dll_header, BACKGROUND_DLL_HEADER_LENGTH);

    bg_adv.eta = eta;
    bg_adv.tx_duration = tx_duration_bg_frame;

    // prepare the foreground frame, so we can transmit this immediately
    DPRINT("Original payload with ETA %i", eta);
    DPRINT_DATA(packet->data, packet->length + 1);

    fg_frame.bg_adv = true;
    memset(fg_frame.encoded_packet, 0xAA, preamble_len);
    sync_word = __builtin_bswap16(sync_word_value[PHY_SYNCWORD_CLASS1][current_channel_id.channel_header.ch_coding]);
    memcpy(&fg_frame.encoded_packet[preamble_len], &sync_word, 2);
    fg_frame.encoded_length = encode_packet(packet, fg_frame.encoded_packet + preamble_len + 2);
    fg_frame.encoded_length += preamble_len + 2; // add preamble + syncword

    uint8_t payload_len;
    payload_len = assemble_background_payload();

    // For the first advertising frame, transmit directly the payload since the preamble and the sync word are directly managed by the xcv
    DPRINT("Transmit packet: %d", payload_len);
    DPRINT_DATA(bg_adv.packet_payload, payload_len);

    hw_radio_send_payload(bg_adv.packet_payload, payload_len); // in preloading mode

    // prepare the next advertising frame, insert the preamble and the SYNC word
    bg_adv.eta -= bg_adv.tx_duration; // the next ETA is the time remaining after the end transmission time of the D7AAdvP frame
    assemble_background_payload();

    // start Tx
    timer_tick_t start = timer_get_counter_value();
    bg_adv.stop_time = start + bg_adv.eta;
    DPRINT("BG advertising start time @ %i stop time @ %i", start, bg_adv.stop_time);

    state = STATE_TX;
    DEBUG_RX_END();
    DEBUG_TX_START();
    hw_radio_set_opmode(NETOPT_STATE_TX);

    return SUCCESS;
}


static void fill_in_fifo()
{
    if (fg_frame.bg_adv == true)
    {
        timer_tick_t current = timer_get_counter_value();
        if (bg_adv.stop_time > current)
            bg_adv.eta = (bg_adv.stop_time - current) - bg_adv.tx_duration; // ETA is updated according the real current time
        else
            //TODO avoid stop time being elapsed
            bg_adv.eta = 0;

        /*
         * When no more advertising background frames can be fully transmitted before
         * the start of D7ANP, the last background frame is extended by padding preamble
         * symbols after the end of the background packet, in order to guarantee no silence period.
         * The FIFO level allows to write enough padding preamble bytes without overflow
         */

        if (bg_adv.eta > bg_adv.tx_duration)
        {
            // Fill up the TX FIFO with the full packet including the preamble and the SYNC word
            hw_radio_send_payload(bg_adv.packet, bg_adv.packet_size);

            // Prepare the next frame
            assemble_background_payload();
        }
        else
        {
            // not enough time for sending another BG frame, send the FG frame,
            // prepend with preamble bytes if necessary
            uint16_t preamble_len = 0;
            uint8_t preamble[bg_adv.packet_size];

            preamble_len = bg_adv.eta * (bg_adv.packet_size / bg_adv.tx_duration);
            DPRINT("Add preamble_bytes: %d", preamble_len);

            memset(preamble, 0xAA, preamble_len);
            hw_radio_send_payload(preamble, preamble_len);

            bg_adv.eta = 0;
            fg_frame.bg_adv = false;
        }
    }
    else
    {
        // Disable the refill event since this is the last chunk of data to transmit
        netopt_enable_t enable = NETOPT_DISABLE;
        netdev->driver->set(netdev, NETOPT_TX_REFILL_IRQ, &enable, sizeof(netopt_enable_t));
        hw_radio_send_payload(fg_frame.encoded_packet, fg_frame.encoded_length);
    }

}


error_t hw_radio_start_background_scan(hw_rx_cfg_t const* rx_cfg, rx_packet_callback_t rx_cb, int16_t rssi_thr)
{
    uint8_t packet_len;

    //DPRINT("START BG scan @ %i", timer_get_counter_value());

    if(rx_cb != NULL)
    {
        assert(alloc_packet_callback != NULL);
        assert(release_packet_callback != NULL);
    }
    rx_packet_callback = rx_cb;

    // We should not initiate a background scan before TX is completed
    assert(state != STATE_TX);

    state = STATE_BG_SCAN;

    configure_syncword(rx_cfg->syncword_class, &(rx_cfg->channel_id));
    configure_channel(&(rx_cfg->channel_id));

    if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
        packet_len = fec_calculated_decoded_length(BACKGROUND_FRAME_LENGTH);
    else
        packet_len = BACKGROUND_FRAME_LENGTH;

    // set PayloadLength to the length of the expected Background frame (fixed length packet format is used)
    hw_radio_set_payload_length(packet_len);

    DEBUG_RX_START();

    hw_radio_set_opmode(NETOPT_STATE_RX);

    int16_t rssi = hw_radio_get_rssi();
    if (rssi <= rssi_thr)
    {
        //DPRINT("FAST RX termination RSSI %i limit %i", rssi, rssi_thr);
        switch_to_standby_mode();
        DEBUG_RX_END();
        return FAIL;
    }

    DPRINT("rssi %i, waiting for BG frame", rssi);

    // the device has a period of To to successfully detect the sync word
    netdev->driver->set(netdev, NETOPT_RX_TIMEOUT, &bg_timeout[current_channel_id.channel_header.ch_class], sizeof(uint32_t));

    // enable the interrupt after packet reception
    netopt_enable_t enable = NETOPT_ENABLE;
    netdev->driver->set(netdev, NETOPT_RX_END_IRQ, &enable, sizeof(netopt_enable_t));

    return SUCCESS;
}

void hw_radio_continuous_tx(hw_tx_cfg_t const* tx_cfg, bool continuous_wave)
{
    DPRINT("Continuous tx\n");

    assert(!continuous_wave); // TODO CW not supported for now
    configure_eirp(tx_cfg->eirp);
    configure_syncword(tx_cfg->syncword_class, &(tx_cfg->channel_id));
    configure_channel(&(tx_cfg->channel_id));

    netopt_enable_t enable = NETOPT_ENABLE;
    netdev->driver->set(netdev, NETOPT_TX_REFILL_IRQ, &enable, sizeof(netopt_enable_t));

    fg_frame.bg_adv = false;
    fg_frame.encoded_length = 64;
    fg_frame.encoded_packet[0] = fg_frame.encoded_length - 1;

    for(int i = 1; i < fg_frame.encoded_length; i++)
        fg_frame.encoded_packet[i] = i;

    pn9_encode(fg_frame.encoded_packet, fg_frame.encoded_length);

    hw_radio_send_payload(fg_frame.encoded_packet, fg_frame.encoded_length);
}


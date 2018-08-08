/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2018 CORTUS
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

#include "debug.h"
#include "log.h"

#include "net/netdev.h"
#include "net/netopt.h"
#include "hwradio.h"
#include "hwdebug.h"
#include "errors.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(HAL_RADIO_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_FWK, __VA_ARGS__)
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

static alloc_packet_callback_t alloc_packet_callback;
static release_packet_callback_t release_packet_callback;
static rx_packet_callback_t rx_packet_callback;
static tx_packet_callback_t tx_packet_callback;
static rx_packet_header_callback_t rx_packet_header_callback;
static tx_refill_callback_t tx_refill_callback;

static netdev_t *netdev;

static void isr_handler(void *arg) {
    DPRINT("Netdev ISR handler outside the IRQ context");
    netdev->driver->isr(netdev);
}

hw_state_t hw_radio_get_opmode() {
    hw_state_t opmode;

    netdev->driver->get(netdev, NETOPT_STATE, &opmode, sizeof(netopt_state_t));
    return opmode;
}

void hw_radio_set_opmode(hw_state_t opmode) {
    netdev->driver->set(netdev, NETOPT_STATE, &opmode, sizeof(netopt_state_t));
}

void hw_radio_set_center_freq(uint32_t center_freq)
{
    DPRINT("Set center frequency: %d\n", center_freq);

    netdev->driver->set(netdev, NETOPT_CHANNEL_FREQUENCY, &center_freq, sizeof(uint32_t));
}

void hw_radio_set_rx_bw_hz(uint32_t bw_hz)
{
    netdev->driver->set(netdev, NETOPT_BANDWIDTH, &bw_hz, sizeof(uint32_t));
}

void hw_radio_set_bitrate(uint32_t bps)
{
    netdev->driver->set(netdev, NETOPT_BITRATE, &bps, sizeof(uint32_t));
}

void hw_radio_set_tx_fdev(uint32_t fdev)
{
    netdev->driver->set(netdev, NETOPT_FDEV, &fdev, sizeof(uint32_t));
}

void hw_radio_set_preamble_size(uint16_t size)
{
    netdev->driver->set(netdev, NETOPT_PREAMBLE_LENGTH, &size, sizeof(uint16_t));
}

/* TODO Make use of the following APIs to setup the xcvr */
/*
void hw_radio_set_modulation_shaping(uint8_t shaping)
{
    netdev->driver->set(netdev, NETOPT_MOD_SHAPING, &shaping, sizeof(uint8_t));
}

void hw_radio_set_preamble_polarity(uint8_t polarity)
{
    netdev->driver->set(netdev, NETOPT_PREAMBLE_POLARITY, &polarity, sizeof(uint8_t));
}

void hw_radio_set_rssi_threshold(uint8_t rssi_thr)
{
    netdev->driver->set(netdev, NETOPT_CCA_THRESHOLD, &rssi_thr, sizeof(uint8_t));
}

void hw_radio_set_rssi_smoothing(uint8_t rssi_samples)
{
    netdev->driver->set(netdev, NETOPT_RSSI_SMOOTHING, &rssi_samples, sizeof(uint8_t));
}

void hw_radio_set_sync_word_size(uint8_t sync_size)
{
    netdev->driver->set(netdev, NETOPT_SYNC_LENGTH, &sync_size, sizeof(uint8_t));
}

void hw_radio_set_sync_on(uint8_t enable)
{
    netdev->driver->set(netdev, NETOPT_SYNC_ON, &enable, sizeof(uint8_t));
}

void hw_radio_set_preamble_detect_on(uint8_t enable)
{
    netdev->driver->set(netdev, NETOPT_PREAMBLE_DETECT_ON, &enable, sizeof(uint8_t));
}
*/

void hw_radio_set_dc_free(uint8_t scheme)
{
    netdev->driver->set(netdev, NETOPT_DC_FREE_SCHEME, &scheme, sizeof(uint8_t));
}

void hw_radio_set_sync_word(uint8_t *sync_word, uint8_t sync_size)
{
    //uint8_t sync_size = read_reg(REG_SYNCCONFIG) & ~RF_SYNCCONFIG_SYNCSIZE_MASK;
    assert(sync_size >= 1);

    netdev->driver->set(netdev, NETOPT_SYNC_WORD, sync_word, sync_size);
}

void hw_radio_set_crc_on(uint8_t enable)
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
    iolist_t iolist = {
            .iol_base = data,
            .iol_len = len
        };

    DPRINT("TX data");
    DPRINT("Payload: %d bytes", len);
    DPRINT_DATA(data, len);

    if (netdev->driver->send(netdev, &iolist) == -ENOTSUP)
    {
        DPRINT("Cannot send: radio is still transmitting");
    }
}

error_t hw_radio_set_idle() {
    // TODO Select the chip mode during Idle state (Standby mode or Sleep mode)

    // For now, select by default the standby mode
    hw_radio_set_opmode(HW_STATE_STANDBY);
    DEBUG_RX_END();
    DEBUG_TX_END();

    return SUCCESS;
}

bool hw_radio_is_idle() {
    if (hw_radio_get_opmode() == HW_STATE_STANDBY)
        return true;
    else
        return false;
}


static void _event_cb(netdev_t *dev, netdev_event_t event)
{
    if (event == NETDEV_EVENT_ISR) {
        sched_post_task_prio(isr_handler, MAX_PRIORITY, NULL);
    }
    else {
        int len = 0;
        netdev_radio_rx_info_t packet_info;
        switch (event) {
            case NETDEV_EVENT_RX_COMPLETE:
                len = dev->driver->recv(netdev, NULL, 0, 0);
                hw_radio_packet_t* rx_packet = alloc_packet_callback(len);
                rx_packet->length = len;

                dev->driver->recv(netdev, rx_packet->data, len, &packet_info);
                DPRINT("RX done\n");
                DPRINT("Payload: %d bytes, RSSI: %i, LQI: %i" /*SNR: %i, TOA: %i}\n"*/,
                        len, packet_info.rssi, packet_info.lqi/*(int)packet_info.snr,
                       (int)packet_info.time_on_air*/);
                DPRINT_DATA(rx_packet->data, len);

                rx_packet->rx_meta.timestamp = timer_get_counter_value();
                rx_packet->rx_meta.crc_status = HW_CRC_UNAVAILABLE;
                rx_packet->rx_meta.rssi = packet_info.rssi;
                rx_packet->rx_meta.lqi = packet_info.lqi;

                rx_packet_callback(rx_packet);
                break;
            case NETDEV_EVENT_TX_COMPLETE:
                DPRINT("Transmission completed");
                DEBUG_TX_END();
                 if(tx_packet_callback) {
                  tx_packet_callback(timer_get_counter_value());
                }
                break;
            case NETDEV_EVENT_TX_REFILL_NEEDED:
                 DPRINT("New data needed to transmit without discontinuity");
                 tx_refill_callback();
                 break;
            case NETDEV_EVENT_RX_STARTED: {
                uint8_t buffer[4];
                uint8_t len;

                len = dev->driver->recv(netdev, buffer, sizeof(buffer), NULL);
                DPRINT("Packet Header received %i\n", len);
                DPRINT_DATA(buffer, len);

                if(rx_packet_header_callback) {
                    rx_packet_header_callback(buffer, len);
                }

            } break;
            case NETDEV_EVENT_TX_TIMEOUT:
                //sx127x_set_sleep(&sx127x);
                break;
            default:
                DPRINT("Unexpected netdev event received: %d\n", event);
                break;
        }
    }
}

error_t hw_radio_init(hwradio_init_args_t* init_args)
{
    error_t ret;

    alloc_packet_callback = init_args->alloc_packet_cb;
    release_packet_callback = init_args->release_packet_cb;
    rx_packet_callback =  init_args->rx_packet_cb;
    rx_packet_header_callback = init_args->rx_packet_header_cb;
    tx_packet_callback = init_args->tx_packet_cb;
    tx_refill_callback = init_args->tx_refill_cb;

    netdev = (netdev_t*) &xcvr;
    ret = netdev->driver->init(netdev);

    netdev->event_callback = _event_cb;
    sched_register_task(&isr_handler);

    return ret;
}

bool hw_radio_is_rx() {
    if (hw_radio_get_opmode() == HW_STATE_RX)
        return true;
    else
        return false;
}

void hw_radio_enable_refill(bool enable)
{
    netopt_enable_t netopt_enable = enable ? NETOPT_ENABLE : NETOPT_DISABLE;
    netdev->driver->set(netdev, NETOPT_TX_REFILL_IRQ, &netopt_enable, sizeof(netopt_enable_t));
}

void hw_radio_enable_preloading(bool enable)
{
    netopt_enable_t netopt_enable = enable ? NETOPT_ENABLE : NETOPT_DISABLE;
    netdev->driver->set(netdev, NETOPT_TX_REFILL_IRQ, &netopt_enable, sizeof(netopt_enable_t));
}

void hw_radio_set_tx_power(uint8_t eirp)
{
    netdev->driver->set(netdev, NETOPT_TX_POWER, &eirp, sizeof(uint8_t));
}

void hw_radio_set_rx_timeout(uint32_t timeout)
{
    netdev->driver->set(netdev, NETOPT_RX_TIMEOUT, &timeout, sizeof(uint32_t));
}

void hw_radio_enable_rx_interrupt(bool enable)
{
    netopt_enable_t netopt_enable = enable ? NETOPT_ENABLE : NETOPT_DISABLE;
    netdev->driver->set(netdev, NETOPT_RX_END_IRQ, &netopt_enable, sizeof(netopt_enable_t));
    netdev->driver->set(netdev, NETOPT_RX_START_IRQ, &netopt_enable, sizeof(netopt_enable_t));
}

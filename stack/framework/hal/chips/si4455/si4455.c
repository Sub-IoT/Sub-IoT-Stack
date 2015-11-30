/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
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

/* \file
 *
 * Driver for TexasInstruments cc1101 radio. This driver is also used for TI cc430 which a SoC containing a cc1101.
 * Nearly all of the logic is shared, except for the way of communicating with the chip, which is done using SPI and GPIO for
 * an external cc1101 versus using registers for cc430. The specifics parts are contained in cc1101_interface{spi/cc430}.c
 *
 * @author glenn.ergeerts@uantwerpen.be
 *
 *
 * TODOs:
 * - implement all possible channels (+validate)
 * - support packet size > 64 bytes (up to 128 bytes)
 * - RSSI measurement + callback when valid
 * - call release_packet callback when RX interrupted
 * - FEC not supported
 * - CRC
 */

#include "debug.h"
#include "string.h"

#include "log.h"
#include "hwradio.h"
#include "hwsystem.h"
#include "hwdebug.h"

#include "si4455.h"
#include "si4455_interface.h"
//#include "cc1101_constants.h"
//#include "cc1101_registers.h"

// turn on/off the debug prints
#ifdef FRAMEWORK_LOG_ENABLED // TODO more granular (LOG_PHY_ENABLED)
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

#define RSSI_OFFSET 74

#if DEBUG_PIN_NUM >= 2
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

/** \brief The possible states the radio can be in
 */
typedef enum
{
    HW_RADIO_STATE_IDLE,
    HW_RADIO_STATE_TX,
    HW_RADIO_STATE_RX
} hw_radio_state_t;

static alloc_packet_callback_t alloc_packet_callback;
static release_packet_callback_t release_packet_callback;
static rx_packet_callback_t rx_packet_callback;
static tx_packet_callback_t tx_packet_callback;
static rssi_valid_callback_t rssi_valid_callback;

static hw_radio_state_t current_state;
static hw_radio_packet_t* current_packet;
static channel_id_t current_channel_id = {
    .channel_header.ch_coding = PHY_CODING_PN9,
    .channel_header.ch_class = PHY_CLASS_NORMAL_RATE,
    .channel_header.ch_freq_band = PHY_BAND_433,
    .center_freq_index = 0
};

static syncword_class_t current_syncword_class = PHY_SYNCWORD_CLASS0;
static syncword_class_t current_eirp = 0;

static bool should_rx_after_tx_completed = false;
static hw_rx_cfg_t pending_rx_cfg;

static void start_rx(hw_rx_cfg_t const* rx_cfg);


error_t hw_radio_init(alloc_packet_callback_t alloc_packet_cb,
                      release_packet_callback_t release_packet_cb)
{

}

error_t hw_radio_set_rx(hw_rx_cfg_t const* rx_cfg, rx_packet_callback_t rx_cb, rssi_valid_callback_t rssi_valid_cb)
{


    return ERROR;
}


error_t hw_radio_send_packet(hw_radio_packet_t* packet, tx_packet_callback_t tx_cb)
{
	  return ERROR;

}

int16_t hw_radio_get_rssi()
{
	  return ERROR;
}

error_t hw_radio_set_idle()
{
	  return ERROR;
}


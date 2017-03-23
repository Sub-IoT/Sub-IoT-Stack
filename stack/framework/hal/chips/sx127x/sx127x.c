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

/* \file
 *
 * Driver for sx127x
 *
 * @author glenn.ergeerts@uantwerpen.be
 */

#include "string.h"
#include "types.h"

#include "debug.h"
#include "log.h"
#include "hwradio.h"
#include "hwdebug.h"
#include "hwspi.h"
#include "platform.h"

#include "sx1276Regs-Fsk.h"


#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_PHY_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_PHY, __VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_PACKET(...)
#define DPRINT_DATA(...)
#endif

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

typedef enum {
    HW_RADIO_STATE_IDLE,
    HW_RADIO_STATE_TX,
    HW_RADIO_STATE_RX,
    HW_RADIO_STATE_OFF,
    HW_RADIO_STATE_UNKOWN
} state_t;

static spi_slave_handle_t* sx127x_spi = NULL;
static alloc_packet_callback_t alloc_packet_callback;
static release_packet_callback_t release_packet_callback;
static rx_packet_callback_t rx_packet_callback;
static tx_packet_callback_t tx_packet_callback;
//static rssi_valid_callback_t rssi_valid_callback;
//static state_t current_state;
//static hw_radio_packet_t* current_packet;
//static channel_id_t current_channel_id = {
//    .channel_header_raw = 0xFF,
//    .center_freq_index = 0xFF
//};

static uint8_t read_reg(uint8_t addr) {
  spi_select(sx127x_spi);
  spi_exchange_byte(sx127x_spi, addr & 0x7F); // send address with bit 7 low to signal a read operation
  uint8_t value = spi_exchange_byte(sx127x_spi, 0x00); // get the response
  spi_deselect(sx127x_spi);
  return value;
}

error_t hw_radio_init(alloc_packet_callback_t alloc_packet_cb, release_packet_callback_t release_packet_cb) {
  if(sx127x_spi != NULL)
    return EALREADY;

  if(alloc_packet_cb == NULL || release_packet_cb == NULL)
    return EINVAL;

  alloc_packet_callback = alloc_packet_cb;
  release_packet_callback = release_packet_cb;

  spi_handle_t* spi_handle = spi_init(SX127x_SPI_USART, SX127x_SPI_BAUDRATE, 8, true, SX127x_SPI_LOCATION);
  sx127x_spi = spi_init_slave(spi_handle, SX127x_SPI_PIN_CS, true);

  return SUCCESS; // TODO FAIL return code
}

error_t hw_radio_set_idle() {
  // TODO
}

bool hw_radio_is_idle() {
  // TODO
}

error_t hw_radio_set_rx(hw_rx_cfg_t const* rx_cfg, rx_packet_callback_t rx_callback, rssi_valid_callback_t rssi_callback) {
  // TODO
}

bool hw_radio_is_rx() {
  // TODO
}

error_t hw_radio_send_packet(hw_radio_packet_t* packet, tx_packet_callback_t tx_callback) {
  // TODO
}

error_t hw_radio_send_background_packet(hw_radio_packet_t* packet, tx_packet_callback_t tx_callback, uint16_t eta, uint16_t tx_duration) {
  // TODO
}

error_t hw_radio_start_background_scan(hw_rx_cfg_t const* rx_cfg, rx_packet_callback_t rx_cb, int16_t rssi_thr) {

}

int16_t hw_radio_get_rssi() {
  // TODO
}

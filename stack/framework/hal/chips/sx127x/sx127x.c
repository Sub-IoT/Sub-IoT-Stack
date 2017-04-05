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

#include "pn9.h"

#define FREQ_STEP 61.03515625

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
  OPMODE_SLEEP = 0,
  OPMODE_STANDBY = 1,
  OPMODE_FSTX = 2,
  OPMODE_TX = 3,
  OPMODE_FSRX = 4,
  OPMODE_RX = 5,
} opmode_t;

typedef enum {
  STATE_IDLE,
  STATE_TX,
  STATE_RX
} state_t;

static spi_slave_handle_t* sx127x_spi = NULL;
static alloc_packet_callback_t alloc_packet_callback;
static release_packet_callback_t release_packet_callback;
static rx_packet_callback_t rx_packet_callback;
static tx_packet_callback_t tx_packet_callback;
static state_t state = STATE_IDLE;
static hw_radio_packet_t* current_packet;
//static rssi_valid_callback_t rssi_valid_callback;
//static channel_id_t current_channel_id = {
//    .channel_header_raw = 0xFF,
//    .center_freq_index = 0xFF
//};

static uint8_t read_reg(uint8_t addr) {
  spi_select(sx127x_spi);
  spi_exchange_byte(sx127x_spi, addr & 0x7F); // send address with bit 7 low to signal a read operation
  uint8_t value = spi_exchange_byte(sx127x_spi, 0x00); // get the response
  spi_deselect(sx127x_spi);
  DPRINT("READ %02x: %02x\n", addr, value);
  return value;
}

static void write_reg(uint8_t addr, uint8_t value) {
  spi_select(sx127x_spi);
  spi_exchange_byte(sx127x_spi, addr | 0x80); // send address with bit 8 high to signal a write operation
  spi_exchange_byte(sx127x_spi, value);
  spi_deselect(sx127x_spi);
  DPRINT("WRITE %02x: %02x", addr, value);
}

static void write_fifo(uint8_t* buffer, uint8_t size) {
  spi_select(sx127x_spi);
  spi_exchange_byte(sx127x_spi, 0x00 | 0x80); // send address with bit 8 high to signal a write operation
  spi_exchange_bytes(sx127x_spi, buffer, NULL, size);
  spi_deselect(sx127x_spi);
  DPRINT("WRITE FIFO %i", size);
}

static void set_opmode(opmode_t opmode) {
  write_reg(REG_OPMODE, (read_reg(REG_OPMODE) & RF_OPMODE_MASK) | opmode);
}

static void init_regs() {
  write_reg(REG_OPMODE, 0x00); // FSK, hi freq, sleep

  // modulation settings, defaulting to normal channel class
  // BR 0x0240 => 55555.55555 bps
  write_reg(REG_BITRATEMSB, 0x02);
  write_reg(REG_BITRATELSB, 0x40);

  // Fdev => 49.988 kHz
  write_reg(REG_FDEVMSB, 0x03);
  write_reg(REG_FDEVLSB, 0x33);

  // centr freq, default to 868N000: 863.1 MHz
  uint32_t freq = 863100000; // in Hz
  freq = (uint32_t)((double)freq / (double)FREQ_STEP);
  write_reg(REG_FRFMSB, (uint8_t)((freq >> 16) & 0xFF));
  write_reg(REG_FRFMID, (uint8_t)((freq >> 8) & 0xFF));
  write_reg(REG_FRFLSB, (uint8_t)(freq & 0xFF));

  // PA
  write_reg(REG_PACONFIG, 0xF8); // use PA BOOST & ~10 dBm output power // TODO needed for RFM96, for nucleo shield disable PA boost
  write_reg(REG_PARAMP, (2 << 5) | 0x09); // BT=0.5 and PaRamp=40us // TODO

  //  write_reg(REG_OCP, 0); // TODO default for now
  write_reg(REG_LNA, 0x23); // highest gain for now, for 868
  //  write_reg(REG_RXCONFIG, 0); // TODO default for now
  //  write_reg(REG_RSSICONFIG, 0); // TODO default for now
  //  write_reg(REG_RSSICOLLISION, 0); // TODO default for now
  //  write_reg(REG_RSSITHRESH, 0); // TODO default for now

  // RX
  // channel bandwidth 203.125 kHz, data rate 55.542 kBaud
  // Carson's rule: 2 x fm + 2 x fd  = 55.555 + 2 x 50 = 155.555 kHz
  // assuming 10 ppm crystals gives max error of: 2 * 10 ppm * 433.16 = 8.66 kHz
  // => BW > 155.555 + 8.66 kHz => > 164 kHZ. Closest possible value is 166.7
  write_reg(REG_RXBW, (2 << 3) & 1);
  //  write_reg(REG_AFCBW, 0); // TODO default for now
  //  write_reg(REG_AFCFEI, 0); // TODO default for now
  //  write_reg(REG_AFCMSB, 0); // TODO default for now
  //  write_reg(REG_AFCLSB, 0); // TODO default for now
//  write_reg(REG_PREAMBLEDETECT, 0); // TODO
//  write_reg(REG_RXTIMEOUT1, 0); // TODO
//  write_reg(REG_RXTIMEOUT1, 0); // TODO
//  write_reg(REG_RXTIMEOUT1, 0); // TODO
//  write_reg(REG_RXDELAY, 0); // TODO
//  write_reg(REG_OSC, 0x07); // keep as default: off
  write_reg(REG_PREAMBLEMSB, 0x00);
  write_reg(REG_PREAMBLELSB, 0x04); // TODO 6 for hi rate
  write_reg(REG_SYNCCONFIG, 0); // no AutoRestartRx, default PreambePolarity, enable syncword of 2 bytes
  write_reg(REG_SYNCVALUE1, 0x0B);
  write_reg(REG_SYNCVALUE2, 0x67);
  write_reg(REG_PACKETCONFIG1, 0xC8); // var length, whitening, addressFiltering off. TODO CRC
  write_reg(REG_PACKETCONFIG2, 0x40); // packet mode
//  write_reg(REG_FIFOTHRESH, 0); // TODO
//  write_reg(REG_SEQCONFIG1, 0); // TODO
//  write_reg(REG_SEQCONFIG2, 0); // TODO
//  write_reg(REG_TIMERRESOL, 0); // TODO
//  write_reg(REG_TIMER1COEF, 0); // TODO
//  write_reg(REG_TIMER2COEF, 0); // TODO
//  write_reg(REG_IMAGECAL, 0); // TODO
//  write_reg(REG_LOWBAT, 0); // TODO
  write_reg(REG_DIOMAPPING1, 0); // TODO
  write_reg(REG_DIOMAPPING2, 0x30); // ModeReady TODO
//  write_reg(REG_PLLHOP, 0); // TODO
//  write_reg(REG_TCXO, 0); // TODO
//  write_reg(REG_PADAC, 0); // TODO
//  write_reg(REG_BITRATEFRAC, 0); // TODO
//  write_reg(REG_AGCREF, 0); // TODO
//  write_reg(REG_AGCTHRESH1, 0); // TODO
//  write_reg(REG_AGCTHRESH2, 0); // TODO
//  write_reg(REG_AGCTHRESH3, 0); // TODO
//  write_reg(REG_PLL, 0); // TODO


  // TODO validate:
  // bitrate
  // GFSK settings
  // preamble detector
  // packet handler
  // sync word
  // packet len
  // CRC
  // whitening
  // PA

  // TODO burst write reg?
}

static void end_of_packet_isr() {
  log_print_string("end of packet ISR");
  if(state = STATE_TX) {
    set_opmode(OPMODE_STANDBY);
    if(tx_packet_callback) {
      current_packet->tx_meta.timestamp = timer_get_counter_value();
      tx_packet_callback(current_packet);
    }
  } else {
    assert(false); // TODO RX
  }
}

static void dio_isr(pin_id_t pin_id, uint8_t event_mask) {
  log_print_string("DIO IRQ\n");
  if(hw_gpio_pin_matches(pin_id, SX127x_DIO0_PIN)) {
    hw_gpio_disable_interrupt(SX127x_DIO0_PIN);
    end_of_packet_isr();
  }
  else
     assert(false); // other DIO pins not used (yet)

}



//static void reset() {
//  hw_gpio_configure_pin(SX127x_RESET_PIN, false, gpioModePushPull, 0);
//  hw_busy_wait(150);
//  hw_gpio_configure_pin(SX127x_RESET_PIN, false, gpioModeInputPull, 1);
//  hw_busy_wait(6000);
//}

error_t hw_radio_init(alloc_packet_callback_t alloc_packet_cb, release_packet_callback_t release_packet_cb) {
  //reset();

  if(sx127x_spi != NULL)
    return EALREADY;

  if(alloc_packet_cb == NULL || release_packet_cb == NULL)
    return EINVAL;

  alloc_packet_callback = alloc_packet_cb;
  release_packet_callback = release_packet_cb;

  spi_handle_t* spi_handle = spi_init(SX127x_SPI_USART, SX127x_SPI_BAUDRATE, 8, true, SX127x_SPI_LOCATION);
  sx127x_spi = spi_init_slave(spi_handle, SX127x_SPI_PIN_CS, true);

  init_regs();
  set_opmode(OPMODE_STANDBY); // TODO sleep
  // TODO reset ?
  // TODO op mode
  // TODO calibrate rx chain
  error_t e = hw_gpio_configure_interrupt(SX127x_DIO0_PIN, &dio_isr, GPIO_RISING_EDGE); assert(e == SUCCESS);

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

  tx_packet_callback = tx_callback;
  current_packet = packet;
  hw_gpio_enable_interrupt(SX127x_DIO0_PIN);
  log_print_string("TX len=%i", packet->length);
  write_reg(REG_PAYLOADLENGTH, packet->length);
  write_fifo(packet->data, packet->length);
  state = STATE_TX;
  set_opmode(OPMODE_TX);
}

error_t hw_radio_send_background_packet(hw_radio_packet_t* packet, tx_packet_callback_t tx_callback, uint16_t eta, uint16_t tx_duration) {
  // TODO
}

error_t hw_radio_start_background_scan(hw_rx_cfg_t const* rx_cfg, rx_packet_callback_t rx_cb, int16_t rssi_thr) {

}

int16_t hw_radio_get_rssi() {
  // TODO
}

void hw_radio_continuous_tx(hw_tx_cfg_t const* tx_cfg, bool continuous_wave) {
  // TODO tx_cfg
  log_print_string("cont tx\n");
  assert(!continuous_wave); // TODO CW not supported for now
  hw_gpio_disable_interrupt(SX127x_DIO0_PIN);
  set_opmode(OPMODE_TX);

  // chip does not include a PN9 generator so fill fifo manually ...
  while(1) {
    uint8_t payload_len = 63;
    uint8_t data[payload_len + 1];
    data[0]= payload_len;
    pn9_encode(data + 1 , sizeof(data) - 1);
    write_fifo(data, payload_len + 1);
    while(read_reg(REG_IRQFLAGS2) & 0x20) {} // wait until we go under Fifo threshold
  }
}

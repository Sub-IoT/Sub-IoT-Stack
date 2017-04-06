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
static rssi_valid_callback_t rssi_valid_callback;
static bool should_rx_after_tx_completed = false;
static hw_rx_cfg_t pending_rx_cfg;
static channel_id_t current_channel_id = {
  .channel_header.ch_coding = PHY_CODING_PN9,
  .channel_header.ch_class = PHY_CLASS_NORMAL_RATE,
  .channel_header.ch_freq_band = PHY_BAND_868,
  .center_freq_index = 0
};

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
  spi_exchange_byte(sx127x_spi, 0x80); // send address with bit 8 high to signal a write operation
  spi_exchange_bytes(sx127x_spi, buffer, NULL, size);
  spi_deselect(sx127x_spi);
  DPRINT("WRITE FIFO %i", size);
}

static void set_opmode(opmode_t opmode) {
  write_reg(REG_OPMODE, (read_reg(REG_OPMODE) & RF_OPMODE_MASK) | opmode);
}

static void set_eirp(eirp_t eirp) {
#ifdef PLATFORM_SX127X_USE_PA_BOOST
  // Pout = 17-(15-outputpower)
  assert(eirp >= 2); // lower not supported when using PA_BOOST output
  assert(eirp <= 17); // chip supports until +20 dBm but then we need to enable PaDac. Max 17 for now.
  write_reg(REG_PACONFIG, 0x80 | (eirp - 2));
#else
  // Pout = Pmax-(15-outputpower)
  assert(eirp <= 14); // Pmax = 13.8 dBm
  assert(eirp >= -2); // -1.2 dBm is minimum with this Pmax. We can modify Pmax later as well if we need to go lower.
  write_reg(REG_PACONFIG, 0x70 | (uint8_t)(eirp - 13.8 + 15));
#endif
}

static void set_center_freq(const channel_id_t* channel) {
  assert(channel->channel_header.ch_freq_band == PHY_BAND_868); // TODO other bands
  assert(channel->channel_header.ch_class == PHY_CLASS_NORMAL_RATE); // TODO other rates

   // TODO check channel index is allowed

  uint32_t center_freq = 433.06e3;
  if(channel->channel_header.ch_freq_band == PHY_BAND_868)
    center_freq = 863e6;
  else if(channel->channel_header.ch_freq_band == PHY_BAND_915)
    center_freq = 902e6;

  uint32_t channel_spacing_half = 100e3;
  if(channel->channel_header.ch_class == PHY_CLASS_LO_RATE)
    channel_spacing_half = 12500;

  center_freq += 0.025 * channel->center_freq_index + channel_spacing_half;
  DPRINT("center: %d\n", center_freq);
  center_freq = (uint32_t)((double)center_freq/(double)FREQ_STEP);

  write_reg(REG_FRFMSB, (uint8_t)((center_freq >> 16) & 0xFF));
  write_reg(REG_FRFMID, (uint8_t)((center_freq >> 8) & 0xFF));
  write_reg(REG_FRFLSB, (uint8_t)(center_freq & 0xFF));
}

static void set_channel(const channel_id_t* channel) {
// TODO cache
//  if(hw_radio_channel_ids_equal(&current_channel_id, channel)) {
//    return;
//  }
  assert(channel->channel_header.ch_freq_band == PHY_BAND_868); // TODO other bands
  assert(channel->channel_header.ch_class == PHY_CLASS_NORMAL_RATE); // TODO other rates
  assert(channel->channel_header.ch_coding == PHY_CODING_PN9); // TODO FEC

  set_center_freq(channel);
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

  set_channel(&current_channel_id);

  // PA
  set_eirp(10);
  write_reg(REG_PARAMP, (2 << 5) | 0x09); // BT=0.5 and PaRamp=40us // TODO

  // RX
  //  write_reg(REG_OCP, 0); // TODO default for now
  write_reg(REG_LNA, 0x23); // highest gain for now, for 868 // TODO LnaBoostHf consumes 150% current compared to default LNA

  // TODO validate:
  // - RestartRxOnCollision (off for now)
  // - RestartRxWith(out)PllLock flags: set on freq change
  // - AfcAutoOn: default for now
  // - AgcAutoOn: default for now (use AGC)
  // - RxTrigger: default for now
  write_reg(REG_RXCONFIG, 0x0E);

  write_reg(REG_RSSICONFIG, 0x02); // TODO no RSSI offset for now + using 8 samples for smoothing
  //  write_reg(REG_RSSICOLLISION, 0); // TODO not used for now
  write_reg(REG_RSSITHRESH, 0xFF); // TODO using -128 dBm for now

  // channel bandwidth 203.125 kHz, data rate 55.542 kBaud
  // Carson's rule: 2 x fm + 2 x fd  = 55.555 + 2 x 50 = 155.555 kHz
  // assuming 10 ppm crystals gives max error of: 2 * 10 ppm * 433.16 = 8.66 kHz
  // => BW > 155.555 + 8.66 kHz => > 164 kHZ. Closest possible value is 166.7
  // TODO validate sensitivity / xtal accuracy tradeoff
  write_reg(REG_RXBW, (2 << 3) & 1);
  //  write_reg(REG_AFCBW, 0); // TODO not used for now (AfcAutoOn not set)
  //  write_reg(REG_AFCFEI, 0); // TODO not used for now (AfcAutoOn not set)
  //  write_reg(REG_AFCMSB, 0); // TODO not used for now (AfcAutoOn not set)
  //  write_reg(REG_AFCLSB, 0); // TODO not used for now (AfcAutoOn not set)
  //  write_reg(REG_FEIMSB, 0); // TODO freq offset not used for now
  //  write_reg(REG_FEILSB, 0); // TODO freq offset not used for now
  write_reg(REG_PREAMBLEDETECT, 0xCA); // TODO validate PreambleDetectorSize (2 now) and PreambleDetectorTol (10 now)
  // write_reg(REG_RXTIMEOUT1, 0); // not used for now
  // write_reg(REG_RXTIMEOUT2, 0); // not used for now
  // write_reg(REG_RXTIMEOUT3, 0); // not used for now
  // write_reg(REG_RXDELAY, 0); // not used for now
  // write_reg(REG_OSC, 0x07); // keep as default: off
  write_reg(REG_PREAMBLEMSB, 0x00);
  write_reg(REG_PREAMBLELSB, 32); // TODO 48 for hi rate
  write_reg(REG_SYNCCONFIG, 0x11); // no AutoRestartRx, default PreambePolarity, enable syncword of 2 bytes
  write_reg(REG_SYNCVALUE1, 0x0B);
  write_reg(REG_SYNCVALUE2, 0x67);

  write_reg(REG_PACKETCONFIG1, 0x08); // fixed length (unlimited length mode), whitening and CRC disabled (not compatible), addressFiltering off.
  write_reg(REG_PACKETCONFIG2, 0x40); // packet mode
  write_reg(REG_PAYLOADLENGTH, 0x00); // unlimited length mode (in combination with PacketFormat = 0), so we can encode/decode length byte in software
  write_reg(REG_FIFOTHRESH, 0x83); // tx start condition true when there is at least one byte in FIFO (we are in standby/sleep when filling FIFO anyway)
                                   // For RX the threshold is set to 4 since this is the minimum length of a D7 packet.
  write_reg(REG_SEQCONFIG1, 0x40); // force off for now
  //  write_reg(REG_SEQCONFIG2, 0); // not used for now
  //  write_reg(REG_TIMERRESOL, 0); // not used for now
  //  write_reg(REG_TIMER1COEF, 0); // not used for now
  //  write_reg(REG_TIMER2COEF, 0); // not used for now
  //  write_reg(REG_IMAGECAL, 0); // TODO not used for now
  //  write_reg(REG_LOWBAT, 0); // TODO not used for now

  write_reg(REG_DIOMAPPING1, 0x0C); // DIO2 = 0b11 => interrupt on sync detect
  write_reg(REG_DIOMAPPING2, 0x30); // ModeReady TODO configure for RSSI interrupt when doing CCA?
  //  write_reg(REG_PLLHOP, 0); // TODO might be interesting for channel hopping
  //  write_reg(REG_TCXO, 0); // default
  //  write_reg(REG_PADAC, 0); // default
  //  write_reg(REG_FORMERTEMP, 0); // not used for now
  //  write_reg(REG_BITRATEFRAC, 0); // default
  //  write_reg(REG_AGCREF, 0); // default, TODO validate
  //  write_reg(REG_AGCTHRESH1, 0); // not used for now
  //  write_reg(REG_AGCTHRESH2, 0); // not used for now
  //  write_reg(REG_AGCTHRESH3, 0); // not used for now
  //  write_reg(REG_PLL, 0); // not used for now


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

static void packet_transmitted_isr(pin_id_t pin_id, uint8_t event_mask) {
  hw_gpio_disable_interrupt(SX127x_DIO0_PIN);
  DPRINT("packet transmitted ISR\n");
  assert(state == STATE_TX);

  set_opmode(OPMODE_STANDBY);
  if(tx_packet_callback) {
    current_packet->tx_meta.timestamp = timer_get_counter_value();
    tx_packet_callback(current_packet);
  }
}

static void fifo_threshold_isr(pin_id_t pin_id, uint8_t event_mask) {
  hw_gpio_disable_interrupt(SX127x_DIO1_PIN);
  DPRINT("fifo threshold detected ISR\n");
  assert(state == STATE_RX);

  uint8_t packet_len = read_reg(REG_FIFO);
  pn9_encode(&packet_len, 1);
  DPRINT("rx packet len=%i\n", packet_len);
  // TODO receive packet. This has to be handled in software since hardware data whitening support
  // is not compatible with PN9, so packet handler cannot decode length byte.
}

//static void reset() {
//  hw_gpio_configure_pin(SX127x_RESET_PIN, false, gpioModePushPull, 0);
//  hw_busy_wait(150);
//  hw_gpio_configure_pin(SX127x_RESET_PIN, false, gpioModeInputPull, 1);
//  hw_busy_wait(6000);
//}

static inline void flush_fifo() {
  write_reg(REG_IRQFLAGS2, 0x10);
}

static void start_rx(hw_rx_cfg_t const* rx_cfg) {
  state = STATE_RX;

  set_channel(&(rx_cfg->channel_id));
  // TODO configure_syncword(rx_cfg->syncword_class, rx_cfg->channel_id.channel_header.ch_coding);

  DPRINT("START FG scan @ %i", timer_get_counter_value());
  DEBUG_RX_START();

  if(rx_packet_callback != 0) {
    assert(hw_gpio_enable_interrupt(SX127x_DIO1_PIN) == SUCCESS);
  } else {
    assert(false); // TODO tmp
    // when rx callback not set we ignore received packets
    hw_gpio_disable_interrupt(SX127x_DIO1_PIN);
    // TODO disable packet handler completely in this case?
  }

  flush_fifo();
  set_opmode(OPMODE_RX);


  if(rssi_valid_callback != 0)
  {
      assert(false); // TODO
  }
}

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

  error_t e;
  e = hw_gpio_configure_interrupt(SX127x_DIO0_PIN, &packet_transmitted_isr, GPIO_RISING_EDGE); assert(e == SUCCESS);
  e = hw_gpio_configure_interrupt(SX127x_DIO1_PIN, &fifo_threshold_isr, GPIO_RISING_EDGE); assert(e == SUCCESS);

  return SUCCESS; // TODO FAIL return code
}

error_t hw_radio_set_idle() {
  // TODO
}

bool hw_radio_is_idle() {
  // TODO
}

error_t hw_radio_set_rx(hw_rx_cfg_t const* rx_cfg, rx_packet_callback_t rx_cb, rssi_valid_callback_t rssi_valid_cb) {
  if(rx_cb) {
    assert(alloc_packet_callback != NULL);
    assert(release_packet_callback != NULL);
  }

  // assert(rx_cb != NULL || rssi_valid_cb != NULL); // at least one callback should be valid

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
  // TODO
}

error_t hw_radio_send_packet(hw_radio_packet_t* packet, tx_packet_callback_t tx_callback) {
  tx_packet_callback = tx_callback;
  current_packet = packet;
  hw_gpio_enable_interrupt(SX127x_DIO0_PIN);
  DPRINT("TX len=%i", packet->length);
  DPRINT_DATA(packet->data, packet->length + 1);
  pn9_encode(packet->data, packet->length + 1); // sx127x does not support PN9 whitening in hardware ...
  write_fifo(packet->data, packet->length + 1);
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

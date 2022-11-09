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
 */

/* \file
 *
 * Driver for sx127x
 *
 * @author philippe.nunes@cortus.com
 * @author liam.oorts@aloxy.io
 * @author glenn.ergeerts@uantwerpen.be
 */

#include "string.h"
#include "types.h"
#include "stdlib.h"

#include "debug.h"
#include "log.h"
#include "framework_defs.h"
#include "hwradio.h"
#include "hwdebug.h"
#include "hwspi.h"
#include "platform.h"
#include "errors.h"
#include "power_tracking_file.h"
#include "timer.h"

#include "sx1276Regs-Fsk.h"
#include "sx1276Regs-LoRa.h"

#include "crc.h"
#include "pn9.h"
#include "fec.h"

#define SX127X_FXOSC       32000000UL

#define FREQ_STEP 61.03515625

#define FIFO_SIZE   64
#define BYTES_IN_RX_FIFO            32
#define BG_THRESHOLD                5
#define FG_THRESHOLD                32
#define FIFO_AVAILABLE_SPACE        FIFO_SIZE - FG_THRESHOLD

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_PHY_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_PHY, __VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_PACKET(...)
#define DPRINT_DATA(...)
#endif

// #define testing_ADV

#if PLATFORM_NUM_DEBUGPINS >= 2
    #ifndef testing_ADV
        #define DEBUG_TX_START() hw_debug_set(0);
        #define DEBUG_TX_END() hw_debug_clr(0);
        #define DEBUG_RX_START() hw_debug_set(1);
        #define DEBUG_RX_END() hw_debug_clr(1);
        #define DEBUG_FG_START()
        #define DEBUG_FG_END()
        #define DEBUG_BG_START()
        #define DEBUG_BG_END()
    #else
        #define DEBUG_TX_START()
        #define DEBUG_TX_END()
        #define DEBUG_RX_START()
        #define DEBUG_RX_END()
        #define DEBUG_FG_START() hw_debug_set(0);
        #define DEBUG_FG_END() hw_debug_clr(0);
        #define DEBUG_BG_START() hw_debug_set(1);
        #define DEBUG_BG_END() hw_debug_clr(1);
    #endif
#else
    #define DEBUG_TX_START()
    #define DEBUG_TX_END()
    #define DEBUG_RX_START()
    #define DEBUG_RX_END()
    #define DEBUG_FG_START()
    #define DEBUG_FG_END()
    #define DEBUG_BG_START()
    #define DEBUG_BG_END()
#endif

#ifdef PLATFORM_SX127X_USE_DIO3_PIN
  #define CHECK_FIFO_EMPTY() hw_gpio_get_in(SX127x_DIO3_PIN)
#else
  #define CHECK_FIFO_EMPTY() (read_reg(REG_IRQFLAGS2) & 0x40)
#endif

#define CHECK_FIFO_LEVEL() (read_reg(REG_IRQFLAGS2) & 0x20)
#define CHECK_FIFO_FULL()  (read_reg(REG_IRQFLAGS2) & 0x80)

#if defined(PLATFORM_USE_ABZ) && defined(PLATFORM_SX127X_USE_MANUAL_RXTXSW_PIN)
  #error "Invalid configuration"
#endif

#define LORA_MAC_PRIVATE_SYNCWORD                   0x12 // Sync word for Private LoRa networks
#define LORA_MAC_PUBLIC_SYNCWORD                    0x34 // Sync word for Public LoRa networks
#define RF_MID_BAND_THRESH                          525000000

// bits[4-0]: Imax = 45+5*OcpTrim [mA] if OcpTrim <= 15 (120 mA),bit[5]=OcpOn 
#define OCP_TRIM_OFF                                0x00 //=OcpOff
#define OCP_TRIM_PA_BOOST_ON                        0x2A //=OcpOn, 95mA (including other components delivers a max current of 120mA) 
#define OCP_TRIM_PA_BOOST_OFF                       0x20 //=0cpOn, 45mA (lowest possible value, delivers current consumption of ~68mA)

static const uint16_t rx_bw_startup_time[21] = {66, 78, 89, 105, 88, 126, 125, 151, 177, 226, 277, 329, 
  427, 529, 631, 831, 1033, 1239, 1638, 2037, 2447}; //TS_RE + 5% margin

static const uint32_t lora_available_bw[10] = {7800, 10400, 15600, 20800, 31250, 41700, 62500, 125000, 250000, 500000}; // in Hz
static const uint8_t lora_bw_indexes[10] = {15, 14, 12, 11, 9, 8, 6, 3, 0, 0}; // indexes of lora bw's of startup times
uint8_t lora_closest_bw_index;

static uint8_t rx_bw_number = 21;
static uint8_t rx_bw_khz = 0;
static uint8_t rssi_smoothing_full = 0;
static int8_t current_tx_power;

#ifdef FRAMEWORK_POWER_TRACKING_RF
static timer_tick_t tx_start_time = 0;
static timer_tick_t rx_start_time = 0;
static timer_tick_t standby_start_time = 0;
#endif //FRAMEWORK_POWER_TRACKING_RF

static volatile bool fifo_level_irq_triggered = false;

typedef enum {
  OPMODE_SLEEP = 0,
  OPMODE_STANDBY = 1,
  OPMODE_FSTX = 2,
  OPMODE_TX = 3,
  OPMODE_FSRX = 4,
  OPMODE_RX = 5,
  OPMODE_RXSINGLE = 6, //RXSINGLE is only used in LoRa mode, not FSK
} opmode_t;

typedef enum {
  STATE_IDLE,
  STATE_TX,
  STATE_RX,
  STATE_STANDBY
} state_t;

/*
 * FSK packet handler structure
 */
typedef struct
{
    uint16_t Size;
    uint16_t NbBytes;
    uint16_t FifoThresh;
}FskPacketHandler_t;

FskPacketHandler_t FskPacketHandler_sx127x;

/*
 * TODO:
 * - packets > 64 bytes
 * - FEC
 * - background frames
 * - validate RSSI measurement (CCA)
 * - after CCA chip does not seem to go into TX
 * - research if it has advantages to use chip's top level sequencer
 */

static void rx_timeout(void *arg);

static spi_handle_t* spi_handle = NULL;
static spi_slave_handle_t* sx127x_spi = NULL;
static alloc_packet_callback_t alloc_packet_callback;
static release_packet_callback_t release_packet_callback;
static rx_packet_callback_t rx_packet_callback;
static tx_packet_callback_t tx_packet_callback;
static rx_packet_header_callback_t rx_packet_header_callback;
static tx_refill_callback_t tx_refill_callback;

static tx_lora_packet_callback_t tx_lora_packet_callback;
static rx_lora_packet_callback_t rx_lora_packet_callback;
static rx_lora_error_callback_t rx_lora_error_callback;
static rx_lora_timeout_callback_t rx_lora_timeout_callback;

#define RX_BUFFER_SIZE                              256
uint8_t * rx_buffer;

static state_t state = STATE_STANDBY;
static hw_radio_packet_t* current_packet;

static bool is_sx1272 = false;
static bool enable_refill = false;
static bool enable_preloading = false;
static uint16_t remaining_bytes_len = 0;
static uint8_t previous_threshold = 0;
static uint16_t previous_payload_length = 0;
static bool io_inited = false;

static bool lora_mode = false;

static uint32_t current_center_freq = 0;
static bool rx_type_continuous = true; //if true, use RXCONT, if false use RX_SINGLE

void set_opmode(uint8_t opmode);
static void fifo_threshold_isr();
static void update_active_times(hw_radio_state_t opmode);

static void enable_spi_io() {
  if(!io_inited){
    hw_radio_io_init(true);
    io_inited = true;
  }
  spi_enable(spi_handle);
}

static uint8_t read_reg(uint8_t addr) {
  enable_spi_io();
  spi_select(sx127x_spi);
  spi_exchange_byte(sx127x_spi, addr & 0x7F); // send address with bit 7 low to signal a read operation
  uint8_t value = spi_exchange_byte(sx127x_spi, 0x00); // get the response
  spi_deselect(sx127x_spi);
  //DPRINT("READ %02x: %02x\n", addr, value);
  return value;
}

static void write_reg(uint8_t addr, uint8_t value) {
  enable_spi_io();
  spi_select(sx127x_spi);
  spi_exchange_byte(sx127x_spi, addr | 0x80); // send address with bit 8 high to signal a write operation
  spi_exchange_byte(sx127x_spi, value);
  spi_deselect(sx127x_spi);
  //DPRINT("WRITE %02x: %02x", addr, value);
}

void write_reg_16(uint8_t start_reg, uint16_t value) {
  write_reg(start_reg, (uint8_t)((value >> 8) & 0xFF));
  write_reg(start_reg + 1, (uint8_t)(value & 0xFF));
}

static void write_fifo(uint8_t* buffer, uint8_t size) {
  enable_spi_io();
  spi_select(sx127x_spi);
  spi_exchange_byte(sx127x_spi, 0x80); // send address with bit 8 high to signal a write operation
  spi_exchange_bytes(sx127x_spi, buffer, NULL, size);
  spi_deselect(sx127x_spi);
  // DPRINT("WRITE FIFO %i", size);
  // DPRINT_DATA(buffer, size);
}

static void read_fifo(uint8_t* buffer, uint8_t size) {
  enable_spi_io();
  spi_select(sx127x_spi);
  spi_exchange_byte(sx127x_spi, REG_FIFO);
  spi_exchange_bytes(sx127x_spi, NULL, buffer, size);
  spi_deselect(sx127x_spi);
  DPRINT("READ FIFO %i", size);
}

static opmode_t get_opmode() {
  return (read_reg(REG_OPMODE) & ~RF_OPMODE_MASK);
}


//static void dump_register()
//{

//    DPRINT("************************DUMP REGISTER*********************");

//    for (uint8_t add=0; add <= REG_VERSION; add++)
//        DPRINT("ADDR %2X DATA %02X \r\n", add, read_reg(add));

//    // Please note that when reading the first byte of the FIFO register, this
//    // byte is removed so the dump is not recommended before a TX or take care
//    // to fill it after the dump

//    DPRINT("**********************************************************");
//}

static void set_antenna_switch(opmode_t opmode) {
  if(opmode == OPMODE_TX) {
#ifdef PLATFORM_SX127X_USE_MANUAL_RXTXSW_PIN
    hw_gpio_set(SX127x_MANUAL_RXTXSW_PIN);
#endif
#ifdef PLATFORM_USE_ABZ
    hw_gpio_clr(ABZ_ANT_SW_RX_PIN);
    if((read_reg(REG_PACONFIG) & RF_PACONFIG_PASELECT_PABOOST) == RF_PACONFIG_PASELECT_PABOOST) {
      hw_gpio_clr(ABZ_ANT_SW_TX_PIN);
      hw_gpio_set(ABZ_ANT_SW_PA_BOOST_PIN);
    } else {
      hw_gpio_set(ABZ_ANT_SW_TX_PIN);
      hw_gpio_clr(ABZ_ANT_SW_PA_BOOST_PIN);
    }
#endif
  } else {
#ifdef PLATFORM_SX127X_USE_MANUAL_RXTXSW_PIN
    hw_gpio_clr(SX127x_MANUAL_RXTXSW_PIN);
#endif
#ifdef PLATFORM_USE_ABZ
    hw_gpio_set(ABZ_ANT_SW_RX_PIN);
    hw_gpio_clr(ABZ_ANT_SW_TX_PIN);
    hw_gpio_clr(ABZ_ANT_SW_PA_BOOST_PIN);
#endif
  }
}

static inline void flush_fifo() {
  sched_cancel_task(&fifo_threshold_isr);
  DPRINT("Flush fifo @ %i\n", timer_get_counter_value());
  write_reg(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN);
}

static void init_regs() {
  uint8_t gaussian_shape_filter = 2; // 0: no shaping, 1: BT=1.0, 2: BT=0.5, 3: BT=0.3 // TODO benchmark?
  if(is_sx1272) {
    write_reg(REG_OPMODE, RF_OPMODE_SLEEP | gaussian_shape_filter << 3); // FSK; modulation shaping, sleep
  } else {
    write_reg(REG_OPMODE, RF_OPMODE_SLEEP | RF_OPMODE_MODULATIONTYPE_FSK); // FSK, hi freq, sleep
  }

  // PA
  hw_radio_set_tx_power(10);
  if(is_sx1272) {
    write_reg(REG_PARAMP, RF_PARAMP_0040_US | RF_PARAMP_LOWPNTXPLL_OFF); // PaRamp=40us // TODO, use LowPnRxPll?
  } else {
    write_reg(REG_PARAMP, RF_PARAMP_0040_US | (gaussian_shape_filter << 5)); // modulation shaping and PaRamp=40us
  }

  // RX
  write_reg(REG_LNA, RF_LNA_GAIN_G1 | RF_LNA_BOOST_ON); // highest gain for now, for 868 // TODO LnaBoostHf consumes 150% current compared to default LNA

  // TODO validate:
  // - RestartRxOnCollision (off for now)
  // - RestartRxWith(out)PllLock flags: set on freq change
  // - AfcAutoOn: default for now
  // - AgcAutoOn: default for now (use AGC)
  // - RxTrigger: default for now
  write_reg(REG_RXCONFIG, RF_RXCONFIG_RESTARTRXONCOLLISION_OFF | RF_RXCONFIG_AFCAUTO_OFF | 
                                RF_RXCONFIG_AGCAUTO_ON | RF_RXCONFIG_RXTRIGER_PREAMBLEDETECT);

  write_reg(REG_RSSICONFIG, RF_RSSICONFIG_OFFSET_P_00_DB | RF_RSSICONFIG_SMOOTHING_8); // TODO no RSSI offset for now + using 8 samples for smoothing
  rssi_smoothing_full = 8;
  //  write_reg(REG_RSSICOLLISION, 0); // TODO not used for now
  write_reg(REG_RSSITHRESH, RF_RSSITHRESH_THRESHOLD); // TODO using -128 dBm for now

  //  write_reg(REG_AFCBW, 0); // TODO not used for now (AfcAutoOn not set)
  //  write_reg(REG_AFCFEI, 0); // TODO not used for now (AfcAutoOn not set)
  //  write_reg(REG_AFCMSB, 0); // TODO not used for now (AfcAutoOn not set)
  //  write_reg(REG_AFCLSB, 0); // TODO not used for now (AfcAutoOn not set)
  //  write_reg(REG_FEIMSB, 0); // TODO freq offset not used for now
  //  write_reg(REG_FEILSB, 0); // TODO freq offset not used for now
  write_reg(REG_PREAMBLEDETECT, RF_PREAMBLEDETECT_DETECTOR_ON | RF_PREAMBLEDETECT_DETECTORSIZE_3 | RF_PREAMBLEDETECT_DETECTORTOL_15);  
  // TODO validate PreambleDetectorSize (2 now) and PreambleDetectorTol (10 now)
  // write_reg(REG_RXTIMEOUT1, 0); // not used for now
  // write_reg(REG_RXTIMEOUT2, 0); // not used for now
  // write_reg(REG_RXTIMEOUT3, 0); // not used for now
  // write_reg(REG_RXDELAY, 0); // not used for now
  // write_reg(REG_OSC, 0x07); // keep as default: off

  write_reg(REG_SYNCCONFIG, RF_SYNCCONFIG_AUTORESTARTRXMODE_OFF | RF_SYNCCONFIG_PREAMBLEPOLARITY_AA | 
    RF_SYNCCONFIG_SYNC_ON | RF_SYNCCONFIG_SYNCSIZE_2); // no AutoRestartRx, default PreambePolarity, enable syncword of 2 bytes
  write_reg(REG_SYNCVALUE1, 0xE6); // by default, the syncword is set for CS0(PN9) class 0
  write_reg(REG_SYNCVALUE2, 0xD0);

  write_reg(REG_PACKETCONFIG1, RF_PACKETCONFIG1_PACKETFORMAT_FIXED | RF_PACKETCONFIG1_DCFREE_OFF |
    RF_PACKETCONFIG1_CRC_OFF | RF_PACKETCONFIG1_CRCAUTOCLEAR_OFF | RF_PACKETCONFIG1_ADDRSFILTERING_OFF | 
    RF_PACKETCONFIG1_CRCWHITENINGTYPE_CCITT); // fixed length (unlimited length mode), CRC auto clear OFF, whitening and CRC disabled (not compatible), addressFiltering off.
  write_reg(REG_PACKETCONFIG2, RF_PACKETCONFIG2_WMBUS_CRC_DISABLE | RF_PACKETCONFIG2_DATAMODE_PACKET |
    RF_PACKETCONFIG2_IOHOME_OFF | RF_PACKETCONFIG2_BEACON_OFF); // packet mode
  write_reg(REG_PAYLOADLENGTH, 0x00); // unlimited length mode (in combination with PacketFormat = 0), so we can encode/decode length byte in software
  previous_payload_length = 0;
  write_reg(REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTARTCONDITION_FIFONOTEMPTY | 0x03); // tx start condition true when there is at least one byte in FIFO (we are in standby/sleep when filling FIFO anyway)
                                   // For RX the threshold is set to 4 since this is the minimum length of a D7 packet (number of bytes in FIFO >= FifoThreshold + 1).

  write_reg(REG_SEQCONFIG1, RF_SEQCONFIG1_SEQUENCER_STOP | RF_SEQCONFIG1_IDLEMODE_STANDBY |
    RF_SEQCONFIG1_FROMSTART_TOLPS | RF_SEQCONFIG1_LPS_SEQUENCER_OFF | RF_SEQCONFIG1_FROMIDLE_TOTX |
    RF_SEQCONFIG1_FROMTX_TOLPS); // force off for now
  //  write_reg(REG_SEQCONFIG2, 0); // not used for now
  //  write_reg(REG_TIMERRESOL, 0); // not used for now
  //  write_reg(REG_TIMER1COEF, 0); // not used for now
  //  write_reg(REG_TIMER2COEF, 0); // not used for now
  //  write_reg(REG_IMAGECAL, 0); // TODO not used for now
  //  write_reg(REG_LOWBAT, 0); // TODO not used for now

  write_reg(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_00 | RF_DIOMAPPING1_DIO1_00 |
    RF_DIOMAPPING1_DIO2_11 | RF_DIOMAPPING1_DIO3_00); // DIO2 = 0b11 => interrupt on sync detect 
  write_reg(REG_DIOMAPPING2, RF_DIOMAPPING2_DIO4_00 | RF_DIOMAPPING2_DIO5_11 |
    RF_DIOMAPPING2_MAP_RSSI); // ModeReady TODO configure for RSSI interrupt when doing CCA?
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

  // TODO
//  sx127x_set_tx_timeout(dev, SX127X_TX_TIMEOUT_DEFAULT);
//  sx127x_set_modem(dev, SX127X_MODEM_DEFAULT);
//  sx127x_set_channel(dev, SX127X_CHANNEL_DEFAULT);
//  sx127x_set_bandwidth(dev, SX127X_BW_DEFAULT);
//  sx127x_set_payload_length(dev, SX127X_PAYLOAD_LENGTH);
//  sx127x_set_tx_power(dev, SX127X_RADIO_TX_POWER);
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

void hw_radio_set_preamble_detector(uint8_t preamble_detector_size, uint8_t preamble_tol) {
  write_reg(REG_PREAMBLEDETECT, RF_PREAMBLEDETECT_DETECTOR_ON | (preamble_detector_size-1) << 5 | preamble_tol);
}

void hw_radio_set_rssi_config(uint8_t rssi_smoothing, uint8_t rssi_offset) {
  write_reg(REG_RSSICONFIG, rssi_offset << 3 | rssi_smoothing);

  rssi_smoothing_full = 2 << rssi_smoothing;
}

static inline int16_t get_rssi() {
  return (- read_reg(REG_RSSIVALUE) >> 1);
}

static void packet_transmitted_isr() {

  DEBUG_TX_END();
  DEBUG_FG_END();

   
  if(tx_lora_packet_callback) {
    if(lora_mode) {
      tx_lora_packet_callback();
    }
  } else {
    hw_busy_wait(110);
    if(tx_packet_callback)
      tx_packet_callback(timer_get_counter_value());
  }
}

static void bg_scan_rx_done() 
{

  //  assert(current_syncword_class == PHY_SYNCWORD_CLASS0);
   timer_tick_t rx_timestamp = timer_get_counter_value();
   DPRINT("BG packet received!");

   DEBUG_BG_END();

   current_packet = alloc_packet_callback(FskPacketHandler_sx127x.Size);
   assert(current_packet); // TODO handle
   current_packet->length = FskPacketHandler_sx127x.Size;

   read_fifo(current_packet->data, FskPacketHandler_sx127x.Size); //current_packet->data + 1

   current_packet->rx_meta.timestamp = rx_timestamp;
   current_packet->rx_meta.crc_status = HW_CRC_UNAVAILABLE;
   current_packet->rx_meta.rssi = get_rssi();
   current_packet->rx_meta.lqi = 0; // TODO

   rx_packet_callback(current_packet);
   flush_fifo(); 
}

static void lora_rxdone_isr() {
  DPRINT("LoRa RxDone ISR\n");
  assert(state == STATE_RX && lora_mode);
  uint8_t raw_rssi = read_reg(REG_LR_PKTRSSIVALUE);
  int8_t raw_snr = read_reg(REG_LR_PKTSNRVALUE);
  uint8_t irqflags = read_reg(REG_LR_IRQFLAGS);
  assert(irqflags & RFLR_IRQFLAGS_RXDONE_MASK);

  // TODO check ValidHeader?
  if((irqflags & RFLR_IRQFLAGS_PAYLOADCRCERROR_MASK) == RFLR_IRQFLAGS_PAYLOADCRCERROR) { 
    if(rx_lora_error_callback) {
      rx_lora_error_callback();
      hw_gpio_enable_interrupt(SX127x_DIO0_PIN);
      return;
    }
  }

  uint8_t len = read_reg(REG_LR_RXNBBYTES);
  write_reg(REG_LR_FIFOADDRPTR, read_reg(REG_LR_FIFORXCURRENTADDR));
  DPRINT("rx packet len=%i", len);

  int16_t rssi;
  if(raw_snr > 0) { 
      rssi = -157 + 16*raw_rssi/15; // TODO only valid for HF port;
  } else {
      rssi = -157 + raw_rssi + raw_snr / 4;
  }

  if(rx_lora_packet_callback) {
    read_fifo(rx_buffer, len);
    write_reg(REG_LR_IRQFLAGS, 0xFF);
    rx_lora_packet_callback(rx_buffer, len, rssi, raw_snr);
  } else {
    //engineering mode
    current_packet = alloc_packet_callback(len);
    if(current_packet == NULL) {
      log_print_error_string("could not allocate package, discarding.");
      return;
    }
    current_packet->length = len;
    read_fifo(current_packet->data, len);
    write_reg(REG_LR_IRQFLAGS, 0xFF);

    current_packet->rx_meta.timestamp = timer_get_counter_value();
    current_packet->rx_meta.crc_status = HW_CRC_UNAVAILABLE;
    current_packet->rx_meta.rssi = rssi;
    current_packet->rx_meta.lqi = 0; // TODO

    rx_packet_callback(current_packet);
  }
  DPRINT("RX done\n");
  hw_gpio_enable_interrupt(SX127x_DIO0_PIN);
}

static void lora_rxtimeout_isr() {
  uint8_t irqflags = read_reg(REG_LR_IRQFLAGS);
  
  if((irqflags & RFLR_IRQFLAGS_RXTIMEOUT_MASK) == RFLR_IRQFLAGS_RXTIMEOUT) {
      hw_gpio_enable_interrupt(SX127x_DIO1_PIN);
      update_active_times(HW_STATE_STANDBY); //RXTIMEOUT interrupt automatically sets SX1276 back to standby mode
      timer_cancel_task(&rx_timeout); //cancel the timer which would otherwise call the same callback
      rx_lora_timeout_callback();
  } else {
      log_print_error_string("DIO1 ISR should only be handling RxTimeout but a different interrupt was handled, unexpected behaviour.");
      assert(false);
  }
}

static void rx_timeout(void *arg) {
  DEBUG_BG_END();
  DPRINT("RX timeout");
  if(lora_mode) {
    if(rx_lora_timeout_callback) {
      rx_lora_timeout_callback();
    }
  }
  hw_radio_set_idle();
}

static void dio0_isr(void *arg) {
  hw_gpio_disable_interrupt(SX127x_DIO0_PIN);  

  if(state == STATE_RX) {
    if(lora_mode)
      sched_post_task(&lora_rxdone_isr);
    else
      sched_post_task(&bg_scan_rx_done);
  } else {
    sched_post_task(&packet_transmitted_isr);
  }
}

static void set_packet_handler_enabled(bool enable) {
  write_reg(REG_PREAMBLEDETECT, (read_reg(REG_PREAMBLEDETECT) & RF_PREAMBLEDETECT_DETECTOR_MASK) | (enable << 7));
  write_reg(REG_SYNCCONFIG, (read_reg(REG_SYNCCONFIG) & RF_SYNCCONFIG_SYNC_MASK) | (enable << 4));
}

static void fifo_level_isr()
{
    uint8_t flags;

    hw_gpio_disable_interrupt(SX127x_DIO1_PIN);

    flags = read_reg(REG_IRQFLAGS2);
    // detect underflow
    if (flags & 0x08)
    {
        DPRINT("FlagsIRQ2: %x means that packet has been sent! ", flags);
        assert(false);
    }
    tx_refill_callback(remaining_bytes_len);
}

static void wait_for_fifo_level_isr() { 
  // We're holding the MCU from going to sleep as this sometimes takes to long to fill the newer data
  // TODO make this non-blocking without breaking it, for example by telling scheduler it can't go to sleep
  fifo_level_irq_triggered = false;
  while(!fifo_level_irq_triggered) 
  {
      if(state == STATE_IDLE)
          return; //if radio gets put in sleep mode, cancel this action
  }
  fifo_level_isr();
}

static void reinit_rx() {
 FskPacketHandler_sx127x.NbBytes = 0;
 FskPacketHandler_sx127x.Size = 0;
 FskPacketHandler_sx127x.FifoThresh = 4;

 write_reg(REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTARTCONDITION_FIFONOTEMPTY | (FskPacketHandler_sx127x.FifoThresh - 1));
 write_reg(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO2_11);
 previous_payload_length = 0;
 write_reg(REG_PACKETCONFIG2, (read_reg(REG_PACKETCONFIG2) & RF_PACKETCONFIG2_PAYLOADLENGTH_MSB_MASK));
 write_reg(REG_PAYLOADLENGTH, 0);

 // Trigger a manual restart of the Receiver chain (no frequency change)
 write_reg(REG_RXCONFIG, RF_RXCONFIG_RESTARTRXWITHOUTPLLLOCK | RF_RXCONFIG_AFCAUTO_OFF| 
  RF_RXCONFIG_AGCAUTO_ON | RF_RXCONFIG_RXTRIGER_PREAMBLEDETECT);
 flush_fifo();

//  //DPRINT("Before enabling interrupt: FLAGS1 %x FLAGS2 %x\n", read_reg(REG_IRQFLAGS1), read_reg(REG_IRQFLAGS2));
 hw_gpio_set_edge_interrupt(SX127x_DIO1_PIN, GPIO_RISING_EDGE);
 hw_gpio_enable_interrupt(SX127x_DIO1_PIN);
}

// TODO
static void fifo_threshold_isr() {
 // TODO might be optimized. Initial plan was to read length byte and reconfigure threshold
 // based on the expected length so we can wait for next interrupt to read remaining bytes.
 // This doesn't seem to work for now however: the interrupt doesn't fire again for some unclear reason.
 // So now we do it as suggest in the datasheet: reading bytes from FIFO until FifoEmpty flag is set.
 // Reading more bytes at once might be more efficient, however getting the number of bytes in the FIFO seems
 // not possible at least in FSK mode (for LoRa, the register RegRxNbBytes gives the number of received bytes).
   hw_gpio_disable_interrupt(SX127x_DIO1_PIN);
   DPRINT("THR ISR with IRQ %x\n", read_reg(REG_IRQFLAGS2));
   assert(state == STATE_RX);

   if (FskPacketHandler_sx127x.Size == 0 && FskPacketHandler_sx127x.NbBytes == 0)
   {
       // For RX, the threshold is set to 4, so if the DIO1 interrupt occurs, it means that can read at least 4 bytes
       uint8_t rx_bytes = 0;
       uint8_t buffer[4];
       uint8_t backup_buffer[4];
       int16_t rssi = get_rssi();
       while(!(CHECK_FIFO_EMPTY()) && rx_bytes < 4)
       {
           buffer[rx_bytes++] = read_reg(REG_FIFO);
       }

       assert(rx_bytes == 4);

      memcpy(backup_buffer, buffer, rx_bytes);
       rx_packet_header_callback(buffer, rx_bytes);
       if(FskPacketHandler_sx127x.Size == 0) {
         log_print_error_string("Length was too large, discarding packet");
         reinit_rx();
         return;
       }

       current_packet = alloc_packet_callback(FskPacketHandler_sx127x.Size);
       if(current_packet == NULL) {
         log_print_error_string("Could not allocate package, discarding.");
         reinit_rx();
         return;
       }

       current_packet->rx_meta.rssi = rssi;
       memcpy(current_packet->data, backup_buffer, 4);
       current_packet->length = FskPacketHandler_sx127x.Size;

       FskPacketHandler_sx127x.NbBytes = 4;
   }

   while(!(CHECK_FIFO_EMPTY()) && (FskPacketHandler_sx127x.NbBytes < FskPacketHandler_sx127x.Size)) {
     uint8_t flags = read_reg(REG_IRQFLAGS2);
     if((flags & RF_IRQFLAGS2_FIFOLEVEL) && ((FskPacketHandler_sx127x.NbBytes + FskPacketHandler_sx127x.FifoThresh) < FskPacketHandler_sx127x.Size)) {
       read_fifo(&current_packet->data[FskPacketHandler_sx127x.NbBytes], FskPacketHandler_sx127x.FifoThresh);
       FskPacketHandler_sx127x.NbBytes += FskPacketHandler_sx127x.FifoThresh;
     } else if(!(flags & RF_IRQFLAGS2_FIFOEMPTY)) {
       current_packet->data[FskPacketHandler_sx127x.NbBytes++] = read_reg(REG_FIFO); 
     }
   }

   uint16_t remaining_bytes = FskPacketHandler_sx127x.Size - FskPacketHandler_sx127x.NbBytes;

   // blocking to get last data
   while((remaining_bytes < 3) && (remaining_bytes != 0)) {
      if(!CHECK_FIFO_EMPTY()) {
        current_packet->data[FskPacketHandler_sx127x.NbBytes++] = read_reg(REG_FIFO);
        remaining_bytes--;
      }
   }

   if(remaining_bytes == 0) {
    current_packet->rx_meta.timestamp = timer_get_counter_value();
    current_packet->rx_meta.crc_status = HW_CRC_UNAVAILABLE;
    current_packet->rx_meta.lqi = 0; // TODO

    // RSSI is measured during reception of the first part of the packet
    // to make sure we are actually measuring during a TX, instead of after

    // Restart the reception until upper layer decides to stop it
    reinit_rx(); // restart already before doing decoding so we don't miss packets on low clock speeds

    DEBUG_FG_END();

    rx_packet_callback(current_packet);

    return;
   }

   //Trigger FifoLevel interrupt
   if ( remaining_bytes > FIFO_SIZE)
      FskPacketHandler_sx127x.FifoThresh = BYTES_IN_RX_FIFO;
   else
      FskPacketHandler_sx127x.FifoThresh = remaining_bytes - 2;
    write_reg(REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTARTCONDITION_FIFONOTEMPTY | (FskPacketHandler_sx127x.FifoThresh - 1));

   hw_gpio_set_edge_interrupt(SX127x_DIO1_PIN, GPIO_RISING_EDGE);
   hw_gpio_enable_interrupt(SX127x_DIO1_PIN);
   
   DPRINT("read %i bytes, %i remaining, FLAGS2 %x, time: %i \n", FskPacketHandler_sx127x.NbBytes, remaining_bytes, read_reg(REG_IRQFLAGS2), timer_get_counter_value());
}

static void dio1_isr(void *arg) {
  DPRINT("DIO1_irq");
  hw_gpio_disable_interrupt(SX127x_DIO1_PIN);

  if(state == STATE_RX) {
    if(lora_mode && rx_lora_timeout_callback) {
      sched_post_task(&lora_rxtimeout_isr);
    } else {
      sched_post_task(&fifo_threshold_isr);
    }
  } else {
      fifo_level_irq_triggered = true;
  }
}

static void restart_rx_chain() {
  // TODO restarting by triggering RF_RXCONFIG_RESTARTRXWITHPLLLOCK seems not to work
  // for some reason, when already in RX and after a freq change.
  // The chip is unable to receive on the new freq
  // For now the workaround is to go back to standby mode, to be optimized later
  hw_radio_set_opmode(HW_STATE_STANDBY);

  // TODO for now we assume we need a restart with PLL lock.
  // this can be optimized for case where there is no freq change
  // write_reg(REG_RXCONFIG, read_reg(REG_RXCONFIG) | RF_RXCONFIG_RESTARTRXWITHPLLLOCK);
  DPRINT("restart RX chain with PLL lock");
}

static void calibrate_rx_chain() {
  // TODO currently assumes to be called on boot only
  assert(get_opmode() == OPMODE_STANDBY);
  uint8_t reg_pa_config_initial_value = read_reg(REG_PACONFIG);

  // Cut the PA just in case, RFO output, power = -1 dBm
  write_reg(REG_PACONFIG, 0x00);

  // We are not calibrating for LF band for now, this is done at POR already

  hw_radio_set_center_freq(863150000);   // Sets a Frequency in HF band

  write_reg(REG_IMAGECAL, RF_IMAGECAL_TEMPMONITOR_OFF | RF_IMAGECAL_IMAGECAL_START); // TODO temperature monitoring disabled for now
  while((read_reg(REG_IMAGECAL) & RF_IMAGECAL_IMAGECAL_RUNNING) == RF_IMAGECAL_IMAGECAL_RUNNING) { }

  write_reg(REG_PACONFIG, reg_pa_config_initial_value);
}

error_t hw_radio_init(hwradio_init_args_t* init_args) {
  alloc_packet_callback = init_args->alloc_packet_cb;
  release_packet_callback = init_args->release_packet_cb;
  rx_packet_callback = init_args->rx_packet_cb;
  rx_packet_header_callback = init_args->rx_packet_header_cb;
  tx_packet_callback = init_args->tx_packet_cb;
  tx_refill_callback = init_args->tx_refill_cb;

  //link callbacks to LoRaMac-node
  tx_lora_packet_callback = init_args->tx_lora_packet_cb;
  rx_lora_packet_callback = init_args->rx_lora_packet_cb;
  rx_lora_error_callback = init_args->rx_lora_error_cb;
  rx_lora_timeout_callback = init_args->rx_lora_timeout_cb;

  if(sx127x_spi == NULL) {
    spi_handle = spi_init(SX127x_SPI_INDEX, SX127x_SPI_BAUDRATE, 8, true, false, false, false, NULL);
    sx127x_spi = spi_init_slave(spi_handle, SX127x_SPI_PIN_CS, true, false);
  }

  spi_enable(spi_handle);
  hw_radio_io_init(true);
  io_inited = true;
  hw_radio_reset();

  write_reg(REG_OPMODE, ((read_reg(REG_OPMODE) & RF_OPMODE_MASK) & RF_OPMODE_LONGRANGEMODE_MASK) | OPMODE_STANDBY);
  while(get_opmode() != OPMODE_STANDBY) {}

  uint8_t chip_version = read_reg(REG_VERSION);
  if(chip_version == 0x12) {
    is_sx1272 = false;
  } else if(chip_version == 0x22) {
    is_sx1272 = true;
  } else {
    assert(false);
  }


  calibrate_rx_chain();
  init_regs();

#ifdef PLATFORM_SX127X_USE_LOW_BAT_SHUTDOWN
  write_reg(REG_LOWBAT, read_reg(REG_LOWBAT) | (1 << 3) | 0x02);
#endif

  hw_radio_set_idle();

  error_t e;
  e = hw_gpio_configure_interrupt(SX127x_DIO0_PIN, GPIO_RISING_EDGE, &dio0_isr, NULL); assert(e == SUCCESS);
  e = hw_gpio_configure_interrupt(SX127x_DIO1_PIN, GPIO_RISING_EDGE, &dio1_isr, NULL); assert(e == SUCCESS);

  sched_register_task(&rx_timeout);
  sched_register_task(&bg_scan_rx_done);
  sched_register_task(&lora_rxdone_isr);
  sched_register_task(&lora_rxtimeout_isr);
  sched_register_task(&packet_transmitted_isr);
  sched_register_task(&fifo_threshold_isr);
  sched_register_task(&wait_for_fifo_level_isr);

  return SUCCESS; // TODO FAIL return code
}

void hw_radio_stop() {
  // TODO reset chip?
  hw_radio_set_idle();
}

error_t hw_radio_set_idle() {
    if(state == STATE_IDLE && !io_inited)
        return EALREADY;
    hw_radio_set_opmode(HW_STATE_SLEEP);
    if(FskPacketHandler_sx127x.Size - FskPacketHandler_sx127x.NbBytes != 0 && FskPacketHandler_sx127x.NbBytes != 0) {
      DPRINT("going to idle while still %i bytes to read.", FskPacketHandler_sx127x.Size - FskPacketHandler_sx127x.NbBytes);
      FskPacketHandler_sx127x.Size = 0;
      FskPacketHandler_sx127x.NbBytes = 0;
      release_packet_callback(current_packet);
    }
    sched_cancel_task(&fifo_threshold_isr);
    sched_cancel_task(&wait_for_fifo_level_isr);
    sched_cancel_task(&bg_scan_rx_done);
    sched_cancel_task(&packet_transmitted_isr);
    timer_cancel_task(&rx_timeout);
    DEBUG_RX_END();
    DEBUG_TX_END();
    DEBUG_BG_END();
    DEBUG_FG_END();
    return SUCCESS;
}

bool hw_radio_is_idle() {
  if(state != STATE_IDLE)
    return false;
  else
    return true;
}

hw_radio_state_t hw_radio_get_opmode(void) {
  switch(get_opmode()) {
    case OPMODE_TX:
      return HW_STATE_TX;
      break;
    case OPMODE_RX:
      return HW_STATE_RX;
      break;
    case OPMODE_RXSINGLE:
      return HW_STATE_RX; 
      break;
    case OPMODE_SLEEP:
      return HW_STATE_SLEEP;
      break;
    case OPMODE_STANDBY:
      return HW_STATE_STANDBY;
      break;
    default:
      return HW_STATE_IDLE;
      break;
  }
}

void set_opmode(uint8_t opmode) {
  switch(opmode) {
    case OPMODE_SLEEP:
      state = STATE_IDLE;
      break;
    case OPMODE_RX:
      state = STATE_RX;
      break;
    case OPMODE_RXSINGLE:
      state = STATE_RX;
      break;
    case OPMODE_TX:
      state = STATE_TX;
      break;
    case OPMODE_STANDBY:
      state = STATE_STANDBY;
      break;
  }
  #if defined(PLATFORM_SX127X_USE_MANUAL_RXTXSW_PIN) || defined(PLATFORM_USE_ABZ)
  set_antenna_switch(opmode);
  #endif
  write_reg(REG_OPMODE, (read_reg(REG_OPMODE) & RF_OPMODE_MASK) | opmode);

  #ifdef PLATFORM_SX127X_USE_VCC_TXCO
  if(opmode == OPMODE_SLEEP)
  {
    while((read_reg(REG_OPMODE) & (0xFF - RF_OPMODE_MASK)) != OPMODE_SLEEP){} //ensure setting sleep mode is processed
    hw_gpio_clr(SX127x_VCC_TXCO);
  }
  else
    hw_gpio_set(SX127x_VCC_TXCO);
  #endif
}

void set_state_rx() {
  if(lora_mode) {
    hw_radio_set_opmode(HW_STATE_STANDBY);

    //from errata
    write_reg( REG_LR_TEST30, 0x00 ); 
    write_reg(REG_LR_TEST2F, 0x40 ); 
    
    
    write_reg(REG_LR_FIFORXBASEADDR, 0);
    write_reg(REG_LR_FIFOADDRPTR, 0);

    write_reg(REG_LR_IRQFLAGS, 0xFF);
    write_reg(REG_LR_IRQFLAGSMASK, //RFLR_IRQFLAGS_RXTIMEOUT |
                                   // RFLR_IRQFLAGS_RXDONE |
                                   // RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                   RFLR_IRQFLAGS_VALIDHEADER |
                                   RFLR_IRQFLAGS_TXDONE |
                                   RFLR_IRQFLAGS_CADDONE |
                                   RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                   RFLR_IRQFLAGS_CADDETECTED );
    
    write_reg(REG_DIOMAPPING1, RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO1_00); // DIO0 mapped to RXDone, DIO1 mapped to RXTimeout

    hw_gpio_enable_interrupt(SX127x_DIO0_PIN);
    hw_gpio_enable_interrupt(SX127x_DIO1_PIN);

    memset( rx_buffer, 0, ( size_t ) RX_BUFFER_SIZE );

    if(rx_type_continuous) {
      set_opmode(OPMODE_RX); //put into RXCONT mode
    } else {
      set_opmode(OPMODE_RXSINGLE); //put into RXSINGLE mode  
    }
    update_active_times(HW_STATE_RX);

  } else {
    if(get_opmode() >= OPMODE_FSRX || get_opmode() == OPMODE_SLEEP) {
      hw_radio_set_opmode(HW_STATE_STANDBY); //Restart when changing freq/datarate
      while(!(read_reg(REG_IRQFLAGS1) & 0x80));
    }
    flush_fifo();

    FskPacketHandler_sx127x.FifoThresh = 4;
    write_reg(REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTARTCONDITION_FIFONOTEMPTY | (FskPacketHandler_sx127x.FifoThresh - 1));
    write_reg(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO2_11);

    FskPacketHandler_sx127x.NbBytes = 0;

    set_packet_handler_enabled(true);

    if(FskPacketHandler_sx127x.Size == 0) {
      hw_gpio_set_edge_interrupt(SX127x_DIO1_PIN, GPIO_RISING_EDGE);
      hw_gpio_enable_interrupt(SX127x_DIO1_PIN);
    } else {
      hw_gpio_set_edge_interrupt(SX127x_DIO0_PIN, GPIO_RISING_EDGE);
      hw_gpio_enable_interrupt(SX127x_DIO0_PIN);
    }

    set_opmode(OPMODE_RX);
    update_active_times(HW_STATE_RX);
  }
}

static void update_active_times(hw_radio_state_t opmode)
{    
#ifdef FRAMEWORK_POWER_TRACKING_RF
    timer_tick_t current_time = timer_get_counter_value();

    uint8_t curr_mode = lora_mode ? POWER_TRACKING_LORA : POWER_TRACKING_D7;

    // Check all 3 tracked modes to stop and start timers
    if(opmode != HW_STATE_STANDBY)
    {
        if(standby_start_time) {
            DPRINT("registering standby, going into: %u", opmode);
            power_tracking_register_radio_action(curr_mode, POWER_TRACKING_RADIO_STANDBY, timer_calculate_difference(standby_start_time, current_time), NULL);
        }
            
        standby_start_time = 0;
    }
    else
        standby_start_time = standby_start_time ? standby_start_time : current_time;

    if(opmode != HW_STATE_TX)
    {
        if(tx_start_time) {
            DPRINT("registering tx, going into: %u", opmode);
            power_tracking_register_radio_action(curr_mode, POWER_TRACKING_RADIO_TX, timer_calculate_difference(tx_start_time, current_time), (void*)&current_tx_power);
        }
            
        tx_start_time = 0;
    }
    else
        tx_start_time = tx_start_time ? tx_start_time : current_time;

    if((opmode != HW_STATE_RX) && (opmode != HW_STATE_IDLE))
    {
        if(rx_start_time) {
            DPRINT("registering rx, going into: %u", opmode);
            power_tracking_register_radio_action(curr_mode, POWER_TRACKING_RADIO_RX, timer_calculate_difference(rx_start_time, current_time), NULL);
        }
            
        rx_start_time = 0;
    }
    else
        rx_start_time = rx_start_time ? rx_start_time : current_time;
#endif //FRAMEWORK_POWER_TRACKING_RF
}

void hw_radio_set_opmode(hw_radio_state_t opmode) {
  switch(opmode) {
    case HW_STATE_OFF:
    case HW_STATE_SLEEP:
      DEBUG_TX_END();
      DEBUG_RX_END();
      hw_gpio_disable_interrupt(SX127x_DIO0_PIN);
      hw_gpio_disable_interrupt(SX127x_DIO1_PIN);
      set_opmode(OPMODE_SLEEP);
      spi_disable(spi_handle);
      hw_radio_io_deinit();
      io_inited = false;
      break;
    case HW_STATE_STANDBY:
      set_opmode(OPMODE_STANDBY);
      break;
    case HW_STATE_TX:
      DEBUG_RX_END();
      DEBUG_TX_START();
      set_opmode(OPMODE_TX);
      break;
    case HW_STATE_IDLE:
    case HW_STATE_RX:
      DEBUG_RX_START();
      set_state_rx();
      break;
    case HW_STATE_RESET:
      hw_reset();
      break;
  }
  update_active_times(opmode);
}

void hw_radio_set_center_freq(uint32_t center_freq) {
  current_center_freq = center_freq; 
  
  center_freq = (uint32_t)(center_freq / FREQ_STEP);

  write_reg(REG_FRFMSB, (uint8_t)((center_freq >> 16) & 0xFF));
  write_reg(REG_FRFMID, (uint8_t)((center_freq >> 8) & 0xFF));
  write_reg(REG_FRFLSB, (uint8_t)(center_freq & 0xFF));
}

void hw_radio_set_rx_bw_hz(uint32_t bw_hz) {
  uint8_t bw_exp_count, bw_mant_count;
  uint32_t computed_bw;
  uint32_t min_bw_dif = 10e6;
  uint8_t reg_bw;

  for(bw_exp_count = 1; bw_exp_count < 8; bw_exp_count++) {
    for(bw_mant_count = 16; bw_mant_count <= 24; bw_mant_count += 4) {
      computed_bw = SX127X_FXOSC / (bw_mant_count * (1 << (bw_exp_count + 2)));
      if((uint32_t) abs(computed_bw - bw_hz) < min_bw_dif) {
        min_bw_dif = (uint32_t) abs(computed_bw - bw_hz);
        reg_bw = ((((bw_mant_count - 16) / 4) << 3) | bw_exp_count);
        rx_bw_number = (bw_exp_count - 1) * 3 + ((bw_mant_count - 16) >> 2);
        rx_bw_khz = (uint8_t) (computed_bw / 1000);
      }
    }
  }

  write_reg(REG_RXBW, reg_bw);
}

void hw_radio_set_bitrate(uint32_t bps) {
  /* Bitrate(15,0) + (BitrateFrac / 16) = FXOSC / bps */
  uint16_t bps_downscaled = (uint16_t)(SX127X_FXOSC / bps); 
  
  write_reg_16(REG_BITRATEMSB, bps_downscaled);
}

void hw_radio_set_tx_fdev(uint32_t fdev) {
  /* Fdev(13,0) = Fdev / Fstep */
  uint16_t fdev_downscaled = fdev / FREQ_STEP;

  write_reg_16(REG_FDEVMSB, fdev_downscaled);
}

void hw_radio_set_preamble_size(uint16_t size) {
  write_reg_16(REG_PREAMBLEMSB, size);
}

void hw_radio_set_dc_free(uint8_t scheme) {
  write_reg(REG_PACKETCONFIG1, (read_reg(REG_PACKETCONFIG1) & RF_PACKETCONFIG1_DCFREE_MASK) | (scheme << 5));
}

void hw_radio_set_sync_word(uint8_t *sync_word, uint8_t sync_size) {
  //TODO: make sync word dependant on size
  uint16_t full_sync_word = sync_word[0];
  if(sync_size > 1)
    full_sync_word += ((uint16_t)sync_word[1]) << 8;
  write_reg_16(REG_SYNCVALUE1, full_sync_word);
}

void hw_radio_set_crc_on(uint8_t enable) {
  write_reg(REG_PACKETCONFIG1, (read_reg(REG_PACKETCONFIG1) & RF_PACKETCONFIG1_CRC_MASK) | (enable << 4));
}

error_t hw_radio_send_payload(uint8_t * data, uint16_t len) {
  if(rx_lora_packet_callback) { //if in LoRaMAC node i.e. callbacks have been defined. Otherwise a buffer in the d7 layer will be used for rx.
    if(rx_buffer != data) 
      rx_buffer = data; //save this so we can use the same buffer in the rx
  }

  if(len == 0)
    return ESIZE;

#ifdef PLATFORM_SX127X_USE_LOW_BAT_SHUTDOWN
  /*activate low battery detector*/
  if(read_reg(REG_IRQFLAGS2) & 0x01){
    write_reg(REG_IRQFLAGS2, 0x01);
    hw_radio_set_idle();
    return;
  }
#endif

  if(state == STATE_RX) {
    hw_gpio_disable_interrupt(SX127x_DIO0_PIN);
    hw_gpio_disable_interrupt(SX127x_DIO1_PIN);
    hw_radio_set_opmode(HW_STATE_STANDBY);
    if(!lora_mode)
      while(!(read_reg(REG_IRQFLAGS1) & 0x80));
  }

  if(state == STATE_IDLE) { //Sleeping
    hw_radio_set_opmode(HW_STATE_STANDBY);
    if(!lora_mode)
      while(!(read_reg(REG_IRQFLAGS1) & 0x80));
  }

  if(!lora_mode) {
    uint16_t start = 0;
    uint8_t available_size = FIFO_SIZE - previous_threshold;
    if(remaining_bytes_len == 0)
      remaining_bytes_len = len;
    else
      start = len - remaining_bytes_len;

    write_reg(REG_DIOMAPPING1, 0x00); //FIFO LEVEL ISR or Packet Sent ISR

    if(remaining_bytes_len > available_size) {
      previous_threshold = FG_THRESHOLD;
      write_reg(REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTARTCONDITION_FIFONOTEMPTY | FG_THRESHOLD);
      write_fifo(data + start, available_size);
      remaining_bytes_len = remaining_bytes_len - available_size;
      hw_gpio_set_edge_interrupt(SX127x_DIO1_PIN, GPIO_FALLING_EDGE);
      hw_gpio_enable_interrupt(SX127x_DIO1_PIN); 
      sched_post_task(&wait_for_fifo_level_isr);
    } else {
      if(!enable_refill) {
        previous_threshold = 0;
        write_fifo(data + start, remaining_bytes_len);
        remaining_bytes_len = 0;
        hw_gpio_set_edge_interrupt(SX127x_DIO0_PIN, GPIO_RISING_EDGE);
        hw_gpio_enable_interrupt(SX127x_DIO0_PIN); 
      } else {
        previous_threshold = 2;
        write_reg(REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTARTCONDITION_FIFONOTEMPTY | 2);
        write_fifo(data + start, remaining_bytes_len);
        remaining_bytes_len = 0;
        hw_gpio_set_edge_interrupt(SX127x_DIO1_PIN, GPIO_FALLING_EDGE);
        hw_gpio_enable_interrupt(SX127x_DIO1_PIN);
        sched_post_task(&wait_for_fifo_level_isr);
      }
    }

    set_packet_handler_enabled(true);
  } else {
    hw_radio_set_opmode(HW_STATE_STANDBY);
    write_reg(REG_LR_PAYLOADLENGTH, len);
    write_reg(REG_LR_FIFOTXBASEADDR, 0);
    write_reg(REG_LR_FIFOADDRPTR, 0);
    write_fifo(data, len);
    write_reg(REG_LR_IRQFLAGS, 0xFF);
    write_reg(REG_LR_IRQFLAGSMASK,  RFLR_IRQFLAGS_RXTIMEOUT |
                                    RFLR_IRQFLAGS_RXDONE |
                                    RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                    RFLR_IRQFLAGS_VALIDHEADER |
                                    // RFLR_IRQFLAGS_TXDONE |
                                    RFLR_IRQFLAGS_CADDONE |
                                    RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                    RFLR_IRQFLAGS_CADDETECTED);

    write_reg(REG_DIOMAPPING1, RFLR_DIOMAPPING1_DIO0_01 ); // DIO0 mapped to TxDone

    hw_gpio_enable_interrupt(SX127x_DIO0_PIN); 
  }

  if(!enable_preloading)
    hw_radio_set_opmode(HW_STATE_TX);
  else
    enable_preloading = false;

  return SUCCESS;
}

void hw_radio_set_payload_length(uint16_t length) {
  if(previous_payload_length != length) {
    write_reg(REG_PACKETCONFIG2, (read_reg(REG_PACKETCONFIG2) & RF_PACKETCONFIG2_PAYLOADLENGTH_MSB_MASK) | ((length >> 8) & 0x07));
    write_reg(REG_PAYLOADLENGTH, length & 0xFF);
    previous_payload_length = length;
  }
  FskPacketHandler_sx127x.Size = length;
}


bool hw_radio_is_rx(void) {
  return (hw_radio_get_opmode() == HW_STATE_RX);
}

void hw_radio_enable_refill(bool enable) {
  if(lora_mode) {
    write_reg(REG_LR_MODEMCONFIG2, read_reg(REG_LR_MODEMCONFIG2) | (enable * RFLR_MODEMCONFIG2_TXCONTINUOUSMODE_ON));
    hw_radio_set_opmode(HW_STATE_STANDBY);
  } else
    enable_refill = enable;
}

void hw_radio_enable_preloading(bool enable) {
  enable_preloading = enable;
}

void hw_radio_set_tx_power(int8_t eirp) {
  if(eirp < -5) {
    eirp = -5;
    DPRINT("The given eirp is too low, adjusted to %d dBm, offset excluded", eirp);
    // assert(eirp >= -5); // -4.2 dBm is minimum
  } 
#ifdef PLATFORM_SX127X_USE_PA_BOOST
 // Pout = 17-(15-outputpower)
  if(eirp > 20) {
    eirp = 20;
    DPRINT("The given eirp is too high, adjusted to %d dBm, offset excluded", eirp);
    // chip supports until +15 dBm default, +17 dBm with PA_BOOST and +20 dBm with PaDac enabled. 
  }
  current_tx_power = eirp;
  if(eirp <= 5) {
    write_reg(REG_PACONFIG, (uint8_t)(eirp - 10.8 + 15));
    write_reg(REG_PADAC, (read_reg(REG_PADAC) & RF_PADAC_20DBM_MASK) | RF_PADAC_20DBM_OFF); //Default Power
  } else if(eirp <= 15) {
    write_reg(REG_PACONFIG, 0x70 | (uint8_t)(eirp));
    write_reg(REG_PADAC, (read_reg(REG_PADAC) & RF_PADAC_20DBM_MASK) | RF_PADAC_20DBM_OFF); //Default Power
  } else if(eirp <= 17) {
    write_reg(REG_PACONFIG, RF_PACONFIG_PASELECT_PABOOST | (eirp - 2));
    write_reg(REG_PADAC, (read_reg(REG_PADAC) & RF_PADAC_20DBM_MASK) | RF_PADAC_20DBM_OFF); //Default Power
  } else {
    write_reg(REG_PACONFIG, RF_PACONFIG_PASELECT_PABOOST | (eirp - 5));
    write_reg(REG_PADAC, (read_reg(REG_PADAC) & RF_PADAC_20DBM_MASK) | RF_PADAC_20DBM_ON);  //High Power
  }
#else
  // Pout = Pmax-(15-outputpower)
  if(eirp > 15) {
    eirp = 15;
    DPRINT("The given eirp is too high, adjusted to %d dBm, offset excluded", eirp);
    // assert(eirp <= 15); // Pmax = 15 dBm
  }
  current_tx_power = eirp;
  if(eirp <= 5)
    write_reg(REG_PACONFIG, (uint8_t)(eirp - 10.8 + 15));
  else
    write_reg(REG_PACONFIG, 0x70 | (uint8_t)(eirp));
#endif

#ifdef PLATFORM_SX127X_OCP_ENABLED
  if(eirp > 15) 
    write_reg(REG_OCP, OCP_TRIM_PA_BOOST_ON); 
  else
    write_reg(REG_OCP, OCP_TRIM_PA_BOOST_OFF);
#else
  write_reg(REG_OCP, OCP_TRIM_OFF);
#endif
}

void hw_radio_switch_longRangeMode(bool use_lora) {
  if(use_lora != lora_mode) {
      set_opmode(OPMODE_SLEEP);
      update_active_times(HW_STATE_SLEEP);
      write_reg(REG_OPMODE, (read_reg(REG_OPMODE) & RFLR_OPMODE_LONGRANGEMODE_MASK) | (use_lora << 7));
      lora_mode = use_lora;

      if(!use_lora) {
        //swapping back to FSK mode, remove LoRaMac callbacks. If the LoRaMac is reinitialised, these will be reset.
        tx_lora_packet_callback  = NULL;
        rx_lora_packet_callback  = NULL;
        rx_lora_error_callback   = NULL;
        rx_lora_timeout_callback = NULL;
      }
  }
}

void hw_radio_set_lora_mode(uint32_t lora_bw, uint8_t lora_SF) {
  hw_radio_set_opmode(HW_STATE_STANDBY); //device has to be in sleep or standby when configuring
  uint32_t min_diff = UINT32_MAX;

  for(uint8_t bw_cnt = 0; bw_cnt < 10; bw_cnt++) {
    if((uint32_t) abs(lora_bw - lora_available_bw[bw_cnt]) < min_diff) {
      lora_closest_bw_index = bw_cnt;
      min_diff = (uint32_t) abs(lora_bw - lora_available_bw[bw_cnt]);
    }
  }
  write_reg(REG_LR_MODEMCONFIG1, RFLR_MODEMCONFIG1_CODINGRATE_4_5 | RFLR_MODEMCONFIG1_IMPLICITHEADER_OFF | (lora_closest_bw_index << 4));

  DPRINT("set to lora mode with %i Hz bandwidth (corrected to %i Hz) and Spreading Factor %i", lora_bw, lora_available_bw[lora_closest_bw_index], lora_SF);

  assert((lora_SF >= 7) && (lora_SF <= 12));
  write_reg(REG_LR_MODEMCONFIG2, RFLR_MODEMCONFIG2_RXPAYLOADCRC_OFF | RFLR_MODEMCONFIG2_TXCONTINUOUSMODE_OFF | (lora_SF << 4));
}

void hw_radio_set_lora_cont_tx(bool activate) {
  write_reg(REG_LR_MODEMCONFIG2, read_reg(REG_LR_MODEMCONFIG2) | (activate * RFLR_MODEMCONFIG2_TXCONTINUOUSMODE_ON));
}

void hw_radio_set_rx_timeout(uint32_t timeout) {
  timer_post_task_delay(&rx_timeout, timeout);
}

__attribute__((weak)) void hw_radio_reset() {
  // needs to be implemented in platform for now (until we have a public API to configure GPIO pins)
}

__attribute__((weak)) void hw_radio_io_init(bool disable_interrupts) {
  // needs to be implemented in platform for now (until we have a public API to configure GPIO pins)
}

__attribute__((weak)) void hw_radio_io_deinit() {
  // needs to be implemented in platform for now (until we have a public API to configure GPIO pins)
}

/* startup time = TS_RE + TS_RSSI */
/* with TS_RE = rx_bw_startup_time */
/* with TS_RSSI = 2^(rssi_smoothing + 1) / (4 * RXBW[kHz]) [ms] */
int16_t hw_radio_get_rssi() {
    set_opmode(OPMODE_RX); //0.103 ms
    update_active_times(HW_STATE_RX);
    hw_gpio_disable_interrupt(SX127x_DIO0_PIN); //3.7µs
    hw_gpio_disable_interrupt(SX127x_DIO1_PIN);
    if(!lora_mode) {
      hw_busy_wait(rx_bw_startup_time[rx_bw_number] + (rssi_smoothing_full * 1000)/(4 * rx_bw_khz));
      return (- read_reg(REG_RSSIVALUE) >> 1);
    } else {
      hw_busy_wait(rx_bw_startup_time[lora_bw_indexes[lora_closest_bw_index]] + ((rssi_smoothing_full * 1000)/(4 * lora_available_bw[lora_closest_bw_index] / 1000)));
      return ( - 157 + read_reg(REG_LR_RSSIVALUE));
    }
}

/*
   * Radio setup for random number generation
*/
uint32_t hw_lora_random( void ) {
  uint8_t i;
  uint32_t rnd = 0;

  hw_radio_switch_longRangeMode( true ); // Set LoRa modem ON  

  // Disable LoRa modem interrupts
  write_reg( REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT |
              RFLR_IRQFLAGS_RXDONE |
              RFLR_IRQFLAGS_PAYLOADCRCERROR |
              RFLR_IRQFLAGS_VALIDHEADER |
              RFLR_IRQFLAGS_TXDONE |
              RFLR_IRQFLAGS_CADDONE |
              RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
              RFLR_IRQFLAGS_CADDETECTED );

  //from errata
  write_reg( REG_LR_TEST30, 0x00 ); 
  write_reg(REG_LR_TEST2F, 0x40 );   

  write_reg(REG_LR_FIFORXBASEADDR, 0);
  write_reg(REG_LR_FIFOADDRPTR, 0);
  set_opmode(OPMODE_RX);

  for( i = 0; i < 32; i++ )
  {
    hw_busy_wait(1000); //wait for 1ms
    // Unfiltered RSSI value reading. Only takes the LSB value
    rnd |= ( ( uint32_t )read_reg( REG_LR_RSSIWIDEBAND ) & 0x01 ) << i;
  }

  hw_radio_set_idle();

  return rnd;
}

void hw_lora_set_tx_continuous_wave( uint32_t freq, int8_t power, uint16_t time )
{
  //NOTE: not used.
}

void hw_lora_set_public_network( bool enable ) {
  hw_radio_switch_longRangeMode(true);
  if( enable )
  {
    // Change LoRa modem SyncWord
    write_reg( REG_LR_SYNCWORD, LORA_MAC_PUBLIC_SYNCWORD );
  }
  else
  {
    // Change LoRa modem SyncWord
    write_reg( REG_LR_SYNCWORD, LORA_MAC_PRIVATE_SYNCWORD );
  }
}

void hw_lora_set_max_payload_length( uint8_t max ) {
  write_reg( REG_LR_PAYLOADMAXLENGTH, max );
}

void hw_lora_set_rx_config(uint32_t bandwidth,
                         uint32_t datarate, uint8_t coderate,
                         uint16_t preambleLen,
                         uint16_t symbTimeout, bool fixLen,
                         uint8_t payloadLen,
                         bool crcOn, bool freqHopOn, uint8_t hopPeriod,
                         bool iqInverted, bool rxContinuous) {

               
  assert(bandwidth <= 2); // Fatal error: When using LoRa modem only bandwidths 125, 250 and 500 kHz are supported
    
  bandwidth += 7;
    
  if( datarate > 12 )
  {
    datarate = 12;
  }
  else if( datarate < 6 )
  {
    datarate = 6;
  }

  bool lowDatarateOptimize = 0;
  if( ( ( bandwidth == 7 ) && ( ( datarate == 11 ) || ( datarate == 12 ) ) ) || ( ( bandwidth == 8 ) && ( datarate == 12 ) ) )
  {
    lowDatarateOptimize = 0x01;
  }

  write_reg( REG_LR_MODEMCONFIG1,
        ( read_reg( REG_LR_MODEMCONFIG1 ) &
        RFLR_MODEMCONFIG1_BW_MASK &
        RFLR_MODEMCONFIG1_CODINGRATE_MASK &
        RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK ) |
        ( bandwidth << 4 ) | ( coderate << 1 ) |
        fixLen );

  write_reg( REG_LR_MODEMCONFIG2,
        ( read_reg( REG_LR_MODEMCONFIG2 ) &
        RFLR_MODEMCONFIG2_SF_MASK &
        RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK &
        RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK ) |
        ( datarate << 4 ) | ( crcOn << 2 ) |
        ( ( symbTimeout >> 8 ) & ~RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK ) );

  write_reg( REG_LR_MODEMCONFIG3,
        ( read_reg( REG_LR_MODEMCONFIG3 ) &
        RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK ) |
        ( lowDatarateOptimize << 3 ) );

  write_reg( REG_LR_SYMBTIMEOUTLSB, ( uint8_t )( symbTimeout & 0xFF ) );

  write_reg( REG_LR_PREAMBLEMSB, ( uint8_t )( ( preambleLen >> 8 ) & 0xFF ) );
  write_reg( REG_LR_PREAMBLELSB, ( uint8_t )( preambleLen & 0xFF ) );

  if( fixLen ) write_reg( REG_LR_PAYLOADLENGTH, payloadLen );


  if( freqHopOn )
  {
    write_reg( REG_LR_PLLHOP, ( read_reg( REG_LR_PLLHOP ) & RFLR_PLLHOP_FASTHOP_MASK ) | RFLR_PLLHOP_FASTHOP_ON );
    write_reg( REG_LR_HOPPERIOD, hopPeriod );
  }

  if( ( bandwidth == 9 ) && ( current_center_freq > RF_MID_BAND_THRESH ) )
  {
    // ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
    write_reg( REG_LR_HIGHBWOPTIMIZE1, 0x02 );
    write_reg( REG_LR_HIGHBWOPTIMIZE2, 0x64 );
  }
  else if( bandwidth == 9 )
  {
    // ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
    write_reg( REG_LR_HIGHBWOPTIMIZE1, 0x02 );
    write_reg( REG_LR_HIGHBWOPTIMIZE2, 0x7F );
  }
  else
  {
    // ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
    write_reg( REG_LR_HIGHBWOPTIMIZE1, 0x03 );
  }

  if( datarate == 6 )
  {
    write_reg( REG_LR_DETECTOPTIMIZE,
                ( read_reg( REG_LR_DETECTOPTIMIZE ) &
                RFLR_DETECTIONOPTIMIZE_MASK ) |
                RFLR_DETECTIONOPTIMIZE_SF6 );
    write_reg( REG_LR_DETECTIONTHRESHOLD, RFLR_DETECTIONTHRESH_SF6 );
  }
  else
  {
    write_reg( REG_LR_DETECTOPTIMIZE,
                ( read_reg( REG_LR_DETECTOPTIMIZE ) &
                RFLR_DETECTIONOPTIMIZE_MASK ) |
                RFLR_DETECTIONOPTIMIZE_SF7_TO_SF12 );
    write_reg( REG_LR_DETECTIONTHRESHOLD, RFLR_DETECTIONTHRESH_SF7_TO_SF12 );
  }

  if( iqInverted )
  {
    write_reg( REG_LR_INVERTIQ, ( ( read_reg( REG_LR_INVERTIQ ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK ) | RFLR_INVERTIQ_RX_ON | RFLR_INVERTIQ_TX_OFF ) );
    write_reg( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_ON );
  }
  else
  {
    write_reg( REG_LR_INVERTIQ, ( ( read_reg( REG_LR_INVERTIQ ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK ) | RFLR_INVERTIQ_RX_OFF | RFLR_INVERTIQ_TX_OFF ) );
    write_reg( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_OFF );
  }
  rx_type_continuous = rxContinuous;
}

void hw_lora_set_tx_config(uint32_t bandwidth, uint32_t datarate,
                        uint8_t coderate, uint16_t preambleLen,
                        bool fixLen, bool crcOn, bool freqHopOn,
                        uint8_t hopPeriod, bool iqInverted, uint32_t timeout) {
  
  assert(bandwidth <= 2); // Fatal error: When using LoRa modem only bandwidths 125, 250 and 500 kHz are supported

  bandwidth += 7;

            
  if( datarate > 12 )
  {
    datarate = 12;
  }
  else if( datarate < 6 )
  {
    datarate = 6;
  }

  bool lowDatarateOptimize = 0;
  if( ( ( bandwidth == 7 ) && ( ( datarate == 11 ) || ( datarate == 12 ) ) ) || ( ( bandwidth == 8 ) && ( datarate == 12 ) ) )
  {
    lowDatarateOptimize = 0x01;
  }
  else
  {
    lowDatarateOptimize = 0x00;
  }

  if( freqHopOn  )
  {
    write_reg( REG_LR_PLLHOP, ( read_reg( REG_LR_PLLHOP ) & RFLR_PLLHOP_FASTHOP_MASK ) | RFLR_PLLHOP_FASTHOP_ON );
    write_reg( REG_LR_HOPPERIOD, hopPeriod );
  }

  write_reg( REG_LR_MODEMCONFIG1,
        ( read_reg( REG_LR_MODEMCONFIG1 ) &
        RFLR_MODEMCONFIG1_BW_MASK &
        RFLR_MODEMCONFIG1_CODINGRATE_MASK &
        RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK ) |
        ( bandwidth << 4 ) | ( coderate << 1 ) |   fixLen );

  write_reg( REG_LR_MODEMCONFIG2,
        ( read_reg( REG_LR_MODEMCONFIG2 ) &
        RFLR_MODEMCONFIG2_SF_MASK &
        RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK ) | ( datarate << 4 ) | ( crcOn << 2 ) );

  write_reg( REG_LR_MODEMCONFIG3,
        ( read_reg( REG_LR_MODEMCONFIG3 ) &
        RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK ) | ( lowDatarateOptimize << 3 ) );

  write_reg( REG_LR_PREAMBLEMSB, ( preambleLen >> 8 ) & 0x00FF );
  write_reg( REG_LR_PREAMBLELSB, preambleLen & 0xFF );

  if( datarate == 6 )
  {
    write_reg( REG_LR_DETECTOPTIMIZE,
            ( read_reg( REG_LR_DETECTOPTIMIZE ) &
            RFLR_DETECTIONOPTIMIZE_MASK ) |
            RFLR_DETECTIONOPTIMIZE_SF6 );
    write_reg( REG_LR_DETECTIONTHRESHOLD, RFLR_DETECTIONTHRESH_SF6 );
  }
  else
  {
    write_reg( REG_LR_DETECTOPTIMIZE,
            ( read_reg( REG_LR_DETECTOPTIMIZE ) &
            RFLR_DETECTIONOPTIMIZE_MASK ) |
            RFLR_DETECTIONOPTIMIZE_SF7_TO_SF12 );
    write_reg( REG_LR_DETECTIONTHRESHOLD, RFLR_DETECTIONTHRESH_SF7_TO_SF12 );
  }

  if( iqInverted )
  {
    write_reg( REG_LR_INVERTIQ, ( ( read_reg( REG_LR_INVERTIQ ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK ) | RFLR_INVERTIQ_RX_OFF | RFLR_INVERTIQ_TX_ON ) );
    write_reg( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_ON );
  }
  else
  {
    write_reg( REG_LR_INVERTIQ, ( ( read_reg( REG_LR_INVERTIQ ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK ) | RFLR_INVERTIQ_RX_OFF | RFLR_INVERTIQ_TX_OFF ) );
    write_reg( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_OFF );
  }
}

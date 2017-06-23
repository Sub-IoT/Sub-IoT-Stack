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
#include "sx1276Regs-LoRa.h"

#include "crc.h"
#include "pn9.h"
#include "fec.h"

#define FREQ_STEP 61.03515625

#define FIFO_SIZE   64
#define AVAILABLE_BYTES_IN_TX_FIFO  32
#define BYTES_IN_RX_FIFO            32



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

#ifdef PLATFORM_SX127X_USE_DIO3_PIN
  #define CHECK_FIFO_EMPTY() hw_gpio_get_in(SX127x_DIO3_PIN)
#else
  #define CHECK_FIFO_EMPTY() (read_reg(REG_IRQFLAGS2) & 0x40)
#endif

#define CHECK_FIFO_LEVEL() (read_reg(REG_IRQFLAGS2) & 0x20)
#define CHECK_FIFO_FULL()  (read_reg(REG_IRQFLAGS2) & 0x80)


typedef enum {
  OPMODE_SLEEP = 0,
  OPMODE_STANDBY = 1,
  OPMODE_FSTX = 2,
  OPMODE_TX = 3,
  OPMODE_FSRX = 4,
  OPMODE_RX = 5, // RXCONTINUOUS in case of LoRa
} opmode_t;

typedef enum {
  STATE_IDLE,
  STATE_TX,
  STATE_RX
} state_t;

/*
 * TODO:
 * - packets > 64 bytes
 * - FEC
 * - background frames
 * - validate RSSI measurement (CCA)
 * - after CCA chip does not seem to go into TX
 * - research if it has advantages to use chip's top level sequencer
 */

static spi_slave_handle_t* sx127x_spi = NULL;
static alloc_packet_callback_t alloc_packet_callback;
static release_packet_callback_t release_packet_callback;
static rx_packet_callback_t rx_packet_callback;
static tx_packet_callback_t tx_packet_callback;
static state_t state = STATE_IDLE;
static bool lora_mode = false;
static hw_radio_packet_t* current_packet;
static rssi_valid_callback_t rssi_valid_callback;
static bool should_rx_after_tx_completed = false;
static syncword_class_t current_syncword_class = PHY_SYNCWORD_CLASS0;
static hw_rx_cfg_t pending_rx_cfg;
static channel_id_t current_channel_id = {
  .channel_header.ch_coding = PHY_CODING_PN9,
  .channel_header.ch_class = PHY_CLASS_NORMAL_RATE,
  .channel_header.ch_freq_band = PHY_BAND_868,
  .center_freq_index = 0
};

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

static uint8_t read_reg(uint8_t addr) {
  spi_select(sx127x_spi);
  spi_exchange_byte(sx127x_spi, addr & 0x7F); // send address with bit 7 low to signal a read operation
  uint8_t value = spi_exchange_byte(sx127x_spi, 0x00); // get the response
  spi_deselect(sx127x_spi);
  //DPRINT("READ %02x: %02x\n", addr, value);
  return value;
}

static void write_reg(uint8_t addr, uint8_t value) {
  spi_select(sx127x_spi);
  spi_exchange_byte(sx127x_spi, addr | 0x80); // send address with bit 8 high to signal a write operation
  spi_exchange_byte(sx127x_spi, value);
  spi_deselect(sx127x_spi);
  //DPRINT("WRITE %02x: %02x", addr, value);
}

static void write_fifo(uint8_t* buffer, uint8_t size) {
  spi_select(sx127x_spi);
  spi_exchange_byte(sx127x_spi, 0x80); // send address with bit 8 high to signal a write operation
  spi_exchange_bytes(sx127x_spi, buffer, NULL, size);
  spi_deselect(sx127x_spi);
  //DPRINT("WRITE FIFO %i", size);
}

static void read_fifo(uint8_t* buffer, uint8_t size) {
  spi_select(sx127x_spi);
  spi_exchange_byte(sx127x_spi, 0x00);
  spi_exchange_bytes(sx127x_spi, NULL, buffer, size);
  spi_deselect(sx127x_spi);
  DPRINT("READ FIFO %i", size);
}

static opmode_t get_opmode() {
  return (read_reg(REG_OPMODE) & ~RF_OPMODE_MASK);
}

static void set_opmode(opmode_t opmode) {
  write_reg(REG_OPMODE, (read_reg(REG_OPMODE) & RF_OPMODE_MASK) | opmode);
}

static inline void flush_fifo() {
  write_reg(REG_IRQFLAGS2, 0x10);
}

static void set_lora_mode(bool use_lora) {
  set_opmode(OPMODE_SLEEP); // mode changing requires being in sleep
  write_reg(REG_OPMODE, (read_reg(REG_OPMODE) & RFLR_OPMODE_LONGRANGEMODE_MASK) | (use_lora << 7)); // TODO can only be modified in sleep mode
  if(use_lora) {
    DPRINT("Enabling LoRa mode");
    write_reg(REG_LR_MODEMCONFIG1, 0x72); // BW=125 kHz, CR=4/5, explicit header mode
    write_reg(REG_LR_MODEMCONFIG2, 0x90); // SF=9, CRC disabled
    lora_mode = true;
  } else {
    DPRINT("Enabling GFSK mode");
    lora_mode = false;
  }
}

static void configure_eirp(eirp_t eirp) {
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
  assert(channel->channel_header.ch_class == PHY_CLASS_NORMAL_RATE ||
         channel->channel_header.ch_class == PHY_CLASS_LORA); // TODO other rates

  // TODO check channel index is allowed
  // TODO define channel settings for LoRa PHY

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

static void configure_channel(const channel_id_t* channel) {
  assert(channel->channel_header.ch_freq_band == PHY_BAND_868); // TODO other bands
  assert(channel->channel_header.ch_class == PHY_CLASS_NORMAL_RATE ||
         channel->channel_header.ch_class == PHY_CLASS_LORA ); // TODO other rates

  if(hw_radio_channel_ids_equal(&current_channel_id, channel)) {
    return;
  }

  set_lora_mode(channel->channel_header.ch_class == PHY_CLASS_LORA); // TODO only switch when needed

  set_center_freq(channel);
  memcpy(&current_channel_id, channel, sizeof(channel_id_t));
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

  configure_channel(&current_channel_id);

  // PA
  configure_eirp(10);
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
  write_reg(REG_RXBW, (2 << 3) | 1); // 166.7 kHz
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
  write_reg(REG_SYNCVALUE1, 0xE6); // by default, the syncword is set for CS0(PN9) class 0
  write_reg(REG_SYNCVALUE2, 0xD0);

  write_reg(REG_PACKETCONFIG1, 0x08); // fixed length (unlimited length mode), CRC auto clear OFF, whitening and CRC disabled (not compatible), addressFiltering off.
  write_reg(REG_PACKETCONFIG2, 0x40); // packet mode
  write_reg(REG_PAYLOADLENGTH, 0x00); // unlimited length mode (in combination with PacketFormat = 0), so we can encode/decode length byte in software
  write_reg(REG_FIFOTHRESH, 0x83); // tx start condition true when there is at least one byte in FIFO (we are in standby/sleep when filling FIFO anyway)
                                   // For RX the threshold is set to 4 since this is the minimum length of a D7 packet (number of bytes in FIFO >= FifoThreshold + 1).

  write_reg(REG_SEQCONFIG1, 0x40); // force off for now
  //  write_reg(REG_SEQCONFIG2, 0); // not used for now
  //  write_reg(REG_TIMERRESOL, 0); // not used for now
  //  write_reg(REG_TIMER1COEF, 0); // not used for now
  //  write_reg(REG_TIMER2COEF, 0); // not used for now
  //  write_reg(REG_IMAGECAL, 0); // TODO not used for now
  //  write_reg(REG_LOWBAT, 0); // TODO not used for now

  write_reg(REG_DIOMAPPING1, 0x0C); // DIO0 = 00 | DIO1 = 00 | DIO2 = 0b11 => interrupt on sync detect | DIO3 = 00
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

static inline int16_t get_rssi() {
  return - read_reg(REG_RSSIVALUE) / 2;
}

static void packet_transmitted_isr() {
  hw_gpio_disable_interrupt(SX127x_DIO0_PIN);
  DPRINT("packet transmitted ISR\n");
  assert(state == STATE_TX);
  if(current_channel_id.channel_header.ch_class == PHY_CLASS_LORA) {
    // in LoRa mode we have to clear the IRQ register manually
    write_reg(REG_LR_IRQFLAGS, 0xFF);
  }

  DEBUG_TX_END();
  set_opmode(OPMODE_STANDBY);
  state = STATE_IDLE;
  if(tx_packet_callback) {
    current_packet->tx_meta.timestamp = timer_get_counter_value();
    tx_packet_callback(current_packet);
  }
}

static void lora_rxdone_isr() {
  hw_gpio_disable_interrupt(SX127x_DIO0_PIN);
  DPRINT("LoRa RxDone ISR\n");
  assert(state == STATE_RX && lora_mode);
  uint8_t irqflags = read_reg(REG_LR_IRQFLAGS);
  assert(irqflags & RFLR_IRQFLAGS_RXDONE_MASK);
  // TODO check PayloadCRCError and ValidHeader?

  uint8_t len = read_reg(REG_LR_RXNBBYTES);
  write_reg(REG_LR_FIFOADDRPTR, read_reg(REG_LR_FIFORXCURRENTADDR));
  DPRINT("rx packet len=%i\n", len);
  current_packet = alloc_packet_callback(len);
  current_packet->length = len - 1;
  read_fifo(current_packet->data, len);
  write_reg(REG_LR_IRQFLAGS, 0xFF);

  current_packet->rx_meta.timestamp = timer_get_counter_value();
  current_packet->rx_meta.rx_cfg.syncword_class = current_syncword_class;
  current_packet->rx_meta.crc_status = HW_CRC_UNAVAILABLE;
  current_packet->rx_meta.rssi = -157 + read_reg(REG_LR_PKTRSSIVALUE); // TODO only valid for HF port
  current_packet->rx_meta.lqi = 0; // TODO
  memcpy(&(current_packet->rx_meta.rx_cfg.channel_id), &current_channel_id, sizeof(channel_id_t));
  DPRINT_DATA(current_packet->data, current_packet->length + 1);
  DPRINT("RX done\n");

  rx_packet_callback(current_packet);
  hw_gpio_enable_interrupt(SX127x_DIO0_PIN);
}

static void bg_scan_rx_done()
{
    hw_gpio_disable_interrupt(SX127x_DIO0_PIN);

    assert(current_syncword_class == PHY_SYNCWORD_CLASS0);

    DPRINT("BG packet received!");

    current_packet = alloc_packet_callback(FskPacketHandler.Size);
    current_packet->length = BACKGROUND_FRAME_LENGTH;

    read_fifo(current_packet->data + 1, FskPacketHandler.Size);

    current_packet->rx_meta.timestamp = timer_get_counter_value();
    current_packet->rx_meta.rx_cfg.syncword_class = current_syncword_class;
    current_packet->rx_meta.crc_status = HW_CRC_UNAVAILABLE;
    current_packet->rx_meta.rssi = get_rssi();
    current_packet->rx_meta.lqi = 0; // TODO
    memcpy(&(current_packet->rx_meta.rx_cfg.channel_id), &current_channel_id, sizeof(channel_id_t));

    pn9_encode(current_packet->data + 1, FskPacketHandler.Size);
    if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
        fec_decode_packet(current_packet->data + 1, FskPacketHandler.Size, FskPacketHandler.Size);

    DPRINT_DATA(current_packet->data, BACKGROUND_FRAME_LENGTH + 1);
    DPRINT("RX done\n");
    flush_fifo();
    rx_packet_callback(current_packet);
}


static void dio0_isr(pin_id_t pin_id, uint8_t event_mask) {

    if(state == STATE_RX) {
        if(lora_mode)
            lora_rxdone_isr();
        else
            bg_scan_rx_done();
    } else {
      packet_transmitted_isr();
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

    fill_in_fifo();
    //DPRINT("Flag %x", read_reg(REG_IRQFLAGS2));

    if (bg_adv.eta)
    {
        // clear the interrupt
        hw_gpio_set_edge_interrupt(SX127x_DIO1_PIN, GPIO_FALLING_EDGE);
        hw_gpio_enable_interrupt(SX127x_DIO1_PIN);
    }
    else
    {
        DPRINT("Wait end of advertising period!");
        hw_gpio_enable_interrupt(SX127x_DIO0_PIN);
    }
}

static void fifo_threshold_isr() {
  // TODO might be optimized. Initial plan was to read length byte and reconfigure threshold
  // based on the expected length so we can wait for next interrupt to read remaining bytes.
  // This doesn't seem to work for now however: the interrupt doesn't fire again for some unclear reason.
  // So now we do it as suggest in the datasheet: reading bytes from FIFO until FifoEmpty flag is set.
  // Reading more bytes at once might be more efficient, however getting the number of bytes in the FIFO seems
  // not possible at least in FSK mode (for LoRa, the register RegRxNbBytes gives the number of received bytes).
    hw_gpio_disable_interrupt(SX127x_DIO1_PIN);
    //DPRINT("fifo threshold detected ISR with IRQ %x\n", read_reg(REG_IRQFLAGS2));
    assert(state == STATE_RX);

    uint8_t packet_len;

    if (FskPacketHandler.Size == 0 && FskPacketHandler.NbBytes == 0)
    {
        // For RX, the threshold is set to 4, so if the DIO1 interrupt occurs, it means that can read at least 4 bytes
        uint8_t rx_bytes = 0;
        uint8_t buffer[4];
        do {
            buffer[rx_bytes++] = read_reg(REG_FIFO);
        } while(!(CHECK_FIFO_EMPTY()) && rx_bytes < 4);

        assert(rx_bytes == 4);

        if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
        {
            uint8_t decoded_buffer[4];
            memcpy(decoded_buffer, buffer, 4);
            pn9_encode(decoded_buffer, 4);
            fec_decode_packet(decoded_buffer, 4, 4);
            packet_len = fec_calculated_decoded_length(decoded_buffer[0]+1);
        }
        else
        {
            packet_len = buffer[0];
            pn9_encode(&packet_len, 1); // decode only packet_len for now so we know how many bytes to receive.
                                        // the full packet is decoded at once, when completely received
            packet_len++;
        }

        DPRINT("RX Packet Length: %i ", packet_len);

        current_packet = alloc_packet_callback(packet_len);
        memcpy(current_packet->data, buffer, 4);

        FskPacketHandler.Size = packet_len;
        FskPacketHandler.NbBytes = 4;
    }

    if (FskPacketHandler.FifoThresh)
    {
        read_fifo(&current_packet->data[FskPacketHandler.NbBytes], FskPacketHandler.FifoThresh);
        FskPacketHandler.NbBytes += FskPacketHandler.FifoThresh;
    }

    while(!(CHECK_FIFO_EMPTY()) && (FskPacketHandler.NbBytes < FskPacketHandler.Size))
       current_packet->data[FskPacketHandler.NbBytes++] = read_reg(REG_FIFO);

    uint8_t remaining_bytes = FskPacketHandler.Size - FskPacketHandler.NbBytes;
    DPRINT("read %i bytes, %i remaining\n", FskPacketHandler.NbBytes, remaining_bytes);

    if(remaining_bytes == 0) {
        current_packet->rx_meta.timestamp = timer_get_counter_value();
        current_packet->rx_meta.rx_cfg.syncword_class = current_syncword_class;
        current_packet->rx_meta.crc_status = HW_CRC_UNAVAILABLE;
        current_packet->rx_meta.rssi = get_rssi();
        current_packet->rx_meta.lqi = 0; // TODO
        memcpy(&(current_packet->rx_meta.rx_cfg.channel_id), &current_channel_id, sizeof(channel_id_t));

        pn9_encode(current_packet->data, FskPacketHandler.NbBytes);

        if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
            fec_decode_packet(current_packet->data, FskPacketHandler.NbBytes, FskPacketHandler.NbBytes);

        DPRINT_DATA(current_packet->data, current_packet->length + 1);
        DPRINT("RX done\n");
        flush_fifo();
        rx_packet_callback(current_packet);
        FskPacketHandler.Size = 0;
        FskPacketHandler.NbBytes = 0;
        return;
    }

    //Trigger FifoLevel interrupt
    if ( remaining_bytes > FIFO_SIZE)
    {
        write_reg(REG_FIFOTHRESH, 0x80 | (BYTES_IN_RX_FIFO - 1));
        FskPacketHandler.FifoThresh = BYTES_IN_RX_FIFO;
    } else {
        write_reg(REG_FIFOTHRESH, 0x80 | (remaining_bytes - 1));
        FskPacketHandler.FifoThresh = remaining_bytes;
    }

    hw_gpio_set_edge_interrupt(SX127x_DIO1_PIN, GPIO_RISING_EDGE);
    hw_gpio_enable_interrupt(SX127x_DIO1_PIN);
}

static void dio1_isr(pin_id_t pin_id, uint8_t event_mask) {

    if(state == STATE_RX) {
        fifo_threshold_isr();
    } else {
        fifo_level_isr();
    }
}

static void configure_syncword(syncword_class_t syncword_class, const channel_id_t* channel)
{
  if(channel->channel_header.ch_class == PHY_CLASS_LORA) {
    assert(syncword_class == PHY_SYNCWORD_CLASS1); // TODO CLASS0 not implemented for LoRa for now
    current_syncword_class = syncword_class;
    return;
    // TODO
  }

  if(syncword_class != current_syncword_class || (channel->channel_header.ch_coding != current_channel_id.channel_header.ch_coding))
  {
    current_syncword_class = syncword_class;
    uint16_t sync_word = sync_word_value[syncword_class][channel->channel_header.ch_coding ];

    DPRINT("sync_word = %04x", sync_word);
    write_reg(REG_SYNCVALUE2, sync_word & 0xFF);
    write_reg(REG_SYNCVALUE1, sync_word >> 8);
  }
}

static void start_rx(hw_rx_cfg_t const* rx_cfg) {
  state = STATE_RX;

  configure_channel(&(rx_cfg->channel_id));
  configure_syncword(rx_cfg->syncword_class, &(rx_cfg->channel_id));

  if(lora_mode) {
    if(rx_packet_callback != 0) {
      // mask all interrupts except RxDone
      write_reg(REG_LR_IRQFLAGSMASK,  RFLR_IRQFLAGS_RXTIMEOUT |
                                      // RFLR_IRQFLAGS_RXDONE |
                                      RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                      RFLR_IRQFLAGS_VALIDHEADER |
                                      RFLR_IRQFLAGS_TXDONE |
                                      RFLR_IRQFLAGS_CADDONE |
                                      RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                      RFLR_IRQFLAGS_CADDETECTED );

      // DIO0=RxDone
      write_reg(REG_DIOMAPPING1, (read_reg(REG_DIOMAPPING1 ) & RFLR_DIOMAPPING1_DIO0_MASK) | RFLR_DIOMAPPING1_DIO0_00);
      write_reg(REG_LR_FIFORXBASEADDR, 0);
      write_reg(REG_LR_FIFOADDRPTR, 0);

      hw_gpio_enable_interrupt(SX127x_DIO0_PIN);
      DPRINT("START LoRa FG scan @ %i", timer_get_counter_value());
      DEBUG_RX_START();
      write_reg(REG_LR_IRQFLAGS, 0xFF);
      set_opmode(OPMODE_RX);
    } else {
      // when rx callback not set we ignore received packets
      hw_gpio_disable_interrupt(SX127x_DIO0_PIN);
      // TODO disable packet handler completely in this case?
    }

    if(rssi_valid_callback) {
      // TODO decide how to handle CCA for LoRa. RSSI on it's own is not sufficient since LoRa is able to receive
      // below noise level. Using CAD feature only works LoRa transmitters during preambe.
      // For now we assume channel is always clear and return a very low RSSI.
      // Note that we don't go to RX in case of CCA for now.
      rssi_valid_callback(-140);
    }
  } else {
   

    flush_fifo();
    FskPacketHandler.NbBytes = 0;
    FskPacketHandler.Size = 0;
    FskPacketHandler.FifoThresh = 0;

    write_reg(REG_FIFOTHRESH, 0x83);
    write_reg(REG_PAYLOADLENGTH, 0x00); // unlimited length mode

    // TODO not sure why yet, but for some reason we need to write REG_DIOMAPPING1
    // if not, this will result in continuous TX when switching from LoRa mode to FSK RX ...
    write_reg(REG_DIOMAPPING1, 0x0C); // DIO2 = 0b11 => interrupt on sync detect

    if(rx_packet_callback != 0) {
      hw_gpio_set_edge_interrupt(SX127x_DIO1_PIN, GPIO_RISING_EDGE);
      hw_gpio_enable_interrupt(SX127x_DIO1_PIN);
      set_packet_handler_enabled(true);
    } else {
      // when rx callback not set we ignore received packets
      hw_gpio_disable_interrupt(SX127x_DIO1_PIN);
      set_packet_handler_enabled(false);
    }

    DPRINT("START FG scan @ %i", timer_get_counter_value());
    DEBUG_RX_START();
    set_opmode(OPMODE_RX);

    if(rssi_valid_callback != 0)
    {
      while(!(read_reg(REG_IRQFLAGS1) & 0x08)) {} // wait until we have a valid RSSI value

      rssi_valid_callback(get_rssi());
    }
  }
}

static void calibrate_rx_chain() {
  // TODO currently assumes to be called on boot only
  DPRINT("Calibrating RX chain");
  assert(get_opmode() == OPMODE_STANDBY);
  uint8_t reg_pa_config_initial_value = read_reg(REG_PACONFIG);

  // Cut the PA just in case, RFO output, power = -1 dBm
  write_reg(REG_PACONFIG, 0x00);

  // We are not calibrating for LF band for now, this is done at POR already

  set_center_freq(&current_channel_id);   // Sets a Frequency in HF band

  write_reg(REG_IMAGECAL, 0x01 | RF_IMAGECAL_IMAGECAL_START); // TODO temperature monitoring disabled for now
  while((read_reg(REG_IMAGECAL) & RF_IMAGECAL_IMAGECAL_RUNNING) == RF_IMAGECAL_IMAGECAL_RUNNING) { }

  write_reg(REG_PACONFIG, reg_pa_config_initial_value);
}

static void switch_to_standby_mode()
{
    DPRINT("Switching to standby mode");
    //Ensure interrupts are disabled before selecting the chip mode
    hw_gpio_disable_interrupt(SX127x_DIO0_PIN);
    hw_gpio_disable_interrupt(SX127x_DIO1_PIN);

    set_opmode(OPMODE_STANDBY);
    state = STATE_IDLE;
}

error_t hw_radio_init(alloc_packet_callback_t alloc_packet_cb, release_packet_callback_t release_packet_cb) {
  if(sx127x_spi != NULL)
    return EALREADY;

  if(alloc_packet_cb == NULL || release_packet_cb == NULL)
    return EINVAL;

  alloc_packet_callback = alloc_packet_cb;
  release_packet_callback = release_packet_cb;

  spi_handle_t* spi_handle = spi_init(SX127x_SPI_INDEX, SX127x_SPI_BAUDRATE, 8, true, SX127x_SPI_LOCATION);
  sx127x_spi = spi_init_slave(spi_handle, SX127x_SPI_PIN_CS, true);

  set_opmode(OPMODE_STANDBY);
  while(get_opmode() != OPMODE_STANDBY) {}

  calibrate_rx_chain();
  init_regs();

  // TODO sleep

  error_t e;
  e = hw_gpio_configure_interrupt(SX127x_DIO0_PIN, &dio0_isr, GPIO_RISING_EDGE); assert(e == SUCCESS);
  e = hw_gpio_configure_interrupt(SX127x_DIO1_PIN, &dio1_isr, GPIO_RISING_EDGE); assert(e == SUCCESS);

  return SUCCESS; // TODO FAIL return code
}

error_t hw_radio_set_idle() {
    // TODO Select the chip mode during Idle state (Standby mode or Sleep mode)

    // For now, select by default the standby mode
    switch_to_standby_mode();
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

  configure_channel((channel_id_t*)&(current_packet->tx_meta.tx_cfg.channel_id));
  configure_eirp(current_packet->tx_meta.tx_cfg.eirp);
  configure_syncword(current_packet->tx_meta.tx_cfg.syncword_class,
                     &(current_packet->tx_meta.tx_cfg.channel_id));

  state = STATE_TX;

  if(packet->tx_meta.tx_cfg.channel_id.channel_header.ch_class != PHY_CLASS_LORA) {
    // sx127x does not support PN9 whitening in hardware ...
    // copy the packet so we do not encode original packet->data
    uint8_t encoded_packet[256];
    uint8_t encoded_len = packet->length + 1;
    memcpy(encoded_packet, packet->data, packet->length + 1);
    DPRINT("TX len=%i", packet->length);
    DPRINT_DATA(packet->data, packet->length + 1);

    if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
        encoded_len = fec_encode(encoded_packet, packet->length + 1);

    pn9_encode(encoded_packet, encoded_len);

    flush_fifo();

    hw_gpio_set_edge_interrupt(SX127x_DIO0_PIN, GPIO_RISING_EDGE);
    hw_gpio_enable_interrupt(SX127x_DIO0_PIN);
    set_packet_handler_enabled(true);
 
    DEBUG_TX_START();
    set_opmode(OPMODE_TX);
    write_fifo(encoded_packet, encoded_len);
  } else {
    DPRINT("TX LoRa len=%i", packet->length);
    DPRINT_DATA(packet->data, packet->length + 1);
    set_opmode(OPMODE_STANDBY); // LoRa FIFO can only be filled in standby mode
    write_reg(REG_LR_PAYLOADLENGTH, packet->length + 1);
    write_reg(REG_LR_FIFOTXBASEADDR, 0);
    write_reg(REG_LR_FIFOADDRPTR, 0);
    write_fifo(packet->data, packet->length + 1);
    write_reg(REG_LR_IRQFLAGS, 0xFF);
    // mask all interrupts except for TxDone
    write_reg(REG_LR_IRQFLAGSMASK,  RFLR_IRQFLAGS_RXTIMEOUT |
                                    RFLR_IRQFLAGS_RXDONE |
                                    RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                    RFLR_IRQFLAGS_VALIDHEADER |
                                    // RFLR_IRQFLAGS_TXDONE |
                                    RFLR_IRQFLAGS_CADDONE |
                                    RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                    RFLR_IRQFLAGS_CADDETECTED);

    // DIO0 mapped to TxDone
    write_reg(REG_DIOMAPPING1, RFLR_DIOMAPPING1_DIO0_01 );

    DEBUG_TX_START();
    hw_gpio_enable_interrupt(SX127x_DIO0_PIN);
    set_opmode(OPMODE_TX);
  }

  return SUCCESS; // TODO other return codes
}

error_t hw_radio_set_background(hw_radio_packet_t* packet, uint16_t eta, uint16_t tx_duration)
{
    if(state == STATE_TX)
        return EBUSY;

    if(state == STATE_RX)
    {
        pending_rx_cfg.channel_id = current_channel_id;
        pending_rx_cfg.syncword_class = current_syncword_class;
        should_rx_after_tx_completed = true;
        // when in RX state we can directly strobe TX and do not need to wait until in IDLE
    }
    else
    {
        assert(get_opmode() == OPMODE_STANDBY);
        // wait for Standby mode ready
        while(!(read_reg(REG_IRQFLAGS1) & 0x80))
        state = STATE_IDLE;
    }

    flush_fifo();

    configure_syncword(packet->tx_meta.tx_cfg.syncword_class,
                       (channel_id_t*)&(packet->tx_meta.tx_cfg.channel_id));
    configure_channel((channel_id_t*)&(packet->tx_meta.tx_cfg.channel_id));
    configure_eirp(packet->tx_meta.tx_cfg.eirp);

    // During the advertising flooding, use the infinite packet length mode
    write_reg(REG_PAYLOADLENGTH, 0x00);
    // Set the FifoThreshold to fill in the FIFO before the FIFO is empty
    write_reg(REG_FIFOTHRESH, 0x80 | 0x05);

    // Prepare the subsequent background frames which include the preamble and the sync word
    uint8_t preamble_len = (current_channel_id.channel_header.ch_class ==  PHY_CLASS_HI_RATE ? PREAMBLE_HI_RATE_CLASS : PREAMBLE_LOW_RATE_CLASS);
    memset(bg_adv.packet, 0xAA, preamble_len); // preamble length is given in number of bytes
    uint16_t sync_word = __builtin_bswap16(sync_word_value[current_syncword_class][current_channel_id.channel_header.ch_coding]);
    memcpy(&bg_adv.packet[preamble_len], &sync_word, 2);

    if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
        bg_adv.packet_size = preamble_len + 2 + fec_calculated_decoded_length(BACKGROUND_FRAME_LENGTH);
    else
        bg_adv.packet_size = preamble_len + 2 + BACKGROUND_FRAME_LENGTH;

    bg_adv.packet_payload = bg_adv.packet + preamble_len + 2 ;

    // Backup the DLL header
    memcpy(bg_adv.dll_header, packet->data + 1, BACKGROUND_DLL_HEADER_LENGTH);
    DPRINT("DLL header followed by ETA %i", eta);
    DPRINT_DATA(bg_adv.dll_header, BACKGROUND_DLL_HEADER_LENGTH);

    DPRINT("Original payload with ETA %i", eta);
    DPRINT_DATA(packet->data, packet->length + 1);

    bg_adv.eta = eta;
    bg_adv.tx_duration = tx_duration;

    return SUCCESS;
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

static void fill_in_fifo()
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
    if (bg_adv.eta < bg_adv.tx_duration)
    {
        uint16_t preamble_len;
        preamble_len = bg_adv.eta * (bg_adv.packet_size / bg_adv.tx_duration);
        DPRINT("Add preamble_bytes: %d", preamble_len);

        // Normally, we have space enough in the FIFO for the padding preambles
        while(!(CHECK_FIFO_FULL()) && preamble_len)
        {
            write_reg(REG_FIFO, 0xAA);
            preamble_len --;
        }

        bg_adv.eta = 0;
    }
    else
    {
        // Fill up the TX FIFO with the full packet including the preamble and the SYNC word
        write_fifo(bg_adv.packet, bg_adv.packet_size);

        // Prepare the next frame
        assemble_background_payload();
    }
}

error_t hw_radio_start_background_advertising(tx_packet_callback_t tx_callback)
{
    uint8_t payload_len;

    if(state == STATE_TX)
        return EBUSY;

    switch_to_standby_mode(); // to fill the FIFO, we need to be in standby mode

    tx_packet_callback = tx_callback;
    payload_len = assemble_background_payload();

    // For the first advertising frame, transmit directly the payload since the preamble and the sync word are directly managed by the xcv
    DPRINT("Transmit packet: %d", payload_len);
    DPRINT_DATA(bg_adv.packet_payload, payload_len);

    // Fill the FIFO with the first BG frame (preamble and sync word are appended by the transceiver)
    write_fifo(bg_adv.packet_payload, payload_len); // Fill up the TX FIFO

    // prepare the next advertising frame, insert the preamble and the SYNC word
    bg_adv.eta -= bg_adv.tx_duration; // the next ETA is the time remaining after the end transmission time of the D7AAdvP frame
    assemble_background_payload();

    write_reg(REG_DIOMAPPING1, 0x0C); // FIFO LEVEL ISR
    hw_gpio_set_edge_interrupt(SX127x_DIO1_PIN, GPIO_FALLING_EDGE);

    DPRINT("\n REG %x", read_reg(REG_IRQFLAGS2));

    // start Tx
    timer_tick_t start = timer_get_counter_value();
    bg_adv.stop_time = start + bg_adv.eta;
    DPRINT("BG advertising start time @ %i stop time @ %i", start, bg_adv.stop_time);

    state = STATE_TX;
    DEBUG_TX_START();
    set_opmode(OPMODE_TX);
    hw_gpio_enable_interrupt(SX127x_DIO1_PIN);

    return SUCCESS;
}
error_t hw_radio_start_background_scan(hw_rx_cfg_t const* rx_cfg, rx_packet_callback_t rx_cb, int16_t rssi_thr)
{
    uint8_t packet_len;

    DPRINT("START BG scan @ %i", timer_get_counter_value());

    if(rx_cb != NULL)
    {
        assert(alloc_packet_callback != NULL);
        assert(release_packet_callback != NULL);
    }
    rx_packet_callback = rx_cb;

    // We should not initiate a background scan before TX is completed
    assert(state != STATE_TX);

    state = STATE_RX;

    configure_syncword(rx_cfg->syncword_class, &(rx_cfg->channel_id));
    configure_channel(&(rx_cfg->channel_id));

    if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
        packet_len = fec_calculated_decoded_length(BACKGROUND_FRAME_LENGTH);
    else
        packet_len = BACKGROUND_FRAME_LENGTH;

    // set PayloadLength to the length of the expected Background frame (fixed length packet format is used)
    write_reg(REG_PAYLOADLENGTH, packet_len);
    DPRINT("packet length %d", packet_len);

    DEBUG_RX_START();

    flush_fifo();
    FskPacketHandler.NbBytes = 0;
    FskPacketHandler.Size = packet_len;
    FskPacketHandler.FifoThresh = 0;
    write_reg(REG_DIOMAPPING1, 0x0C); // DIO2 interrupt on sync detect and DIO0 interrupt on PayloadReady
    set_opmode(OPMODE_RX);

    // wait for RSSI sample ready
    while(!(read_reg(REG_IRQFLAGS1) & 0x08));

    int16_t rssi = hw_radio_get_rssi();
    if (rssi <= rssi_thr)
    {
        DPRINT("FAST RX termination RSSI %i limit %i", rssi, rssi_thr);
        switch_to_standby_mode();
        DEBUG_RX_END();
        return FAIL;
    }
    else
    {
        //hw_gpio_enable_interrupt(SX127x_DIO2_PIN); // enable the SyncAddress interrupt to stop the sync detection timeout
        DPRINT("rssi %i", rssi);
        hw_gpio_enable_interrupt(SX127x_DIO0_PIN); // enable the PayloadReady interrupt
    }

    // the device has a period of To to successfully detect the sync word
    assert(timer_post_task_delay(&switch_to_standby_mode, bg_timeout[current_channel_id.channel_header.ch_class]) == SUCCESS);

    return SUCCESS;
}

int16_t hw_radio_get_rssi() {
    if(lora_mode)
        return (read_reg(REG_LR_RSSIVALUE) -157); // for HF output port
    else
        return (- read_reg(REG_RSSIVALUE) / 2);
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

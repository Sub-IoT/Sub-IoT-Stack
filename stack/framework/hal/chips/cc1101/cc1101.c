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
#include "hwgpio.h"

#include "cc1101.h"
#include "cc1101_interface.h"
#include "cc1101_constants.h"
#include "cc1101_registers.h"

#include "pn9.h"
#include "fec.h"
#include "crc.h"
#include "phy.h"
#include "MODULE_D7AP_defs.h"

// turn on/off the debug prints
#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_PHY_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_PHY, __VA_ARGS__)
#define DPRINT_PACKET(...) log_print_raw_phy_packet(__VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_PACKET(...)
#define DPRINT_DATA(...)
#endif

#ifdef HAL_RADIO_USE_HW_CRC
static bool has_hardware_crc = true;
#else
static bool has_hardware_crc = false;
#endif

#define RSSI_OFFSET 74

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
static eirp_t current_eirp = 0;

static bool should_rx_after_tx_completed = false;
static hw_rx_cfg_t pending_rx_cfg;

static bool writeRemainingDataFlag = false;
static bool endOfPacket = false;
static uint8_t bytesLeft;
static uint8_t *BufferIndex;
static uint8_t iterations;

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

static void start_rx(hw_rx_cfg_t const* rx_cfg);

static RF_SETTINGS rf_settings = {
   RADIO_GDO2_VALUE,               // IOCFG2    GDO2 output pin configuration.
   RADIO_GDO1_VALUE,               // IOCFG1    GDO1 output pin configuration.
   RADIO_GDO0_VALUE,               // IOCFG0    GDO0 output pin configuration.
   RADIO_FIFOTHR_FIFO_THR_61_4,    // FIFOTHR   RXFIFO and TXFIFO thresholds.
   RADIO_SYNC1_CS0_CLASS0,         // SYNC1     Sync word, high byte
   RADIO_SYNC0_CS0_CLASS0,         // SYNC0     Sync word, low byte
   RADIO_PKTLEN,                   // PKTLEN    Packet length.
   RADIO_PKTCTRL1_PQT(3) | RADIO_PKTCTRL1_APPEND_STATUS,   // PKTCTRL1  Packet automation control.
   RADIO_PKTCTRL0_WHITE_DATA | RADIO_PKTCTRL0_LENGTH_VAR,  // PKTCTRL0  Packet automation control.
   RADIO_ADDR,                      // ADDR      Device address.
   RADIO_CHAN,                      // CHANNR    Channel number.
   RADIO_FREQ_IF,                   // FSCTRL1   Frequency synthesizer control.
   RADIO_FREQOFF,                   // FSCTRL0   Frequency synthesizer control.
   RADIO_FREQ2(RADIO_FREQ_433_NORMAL_RATE),   // FREQ2     Frequency control word, high byte.
   RADIO_FREQ1(RADIO_FREQ_433_NORMAL_RATE),   // FREQ1     Frequency control word, middle byte.
   RADIO_FREQ0(RADIO_FREQ_433_NORMAL_RATE),   // FREQ0     Frequency control word, low byte.
   RADIO_MDMCFG4_NORMAL_RATE,
   RADIO_MDMCFG3_DRATE_M_NORMAL_RATE,         // MDMCFG3   Modem configuration.
   RADIO_MDMCFG2_DEM_DCFILT_ON | RADIO_MDMCFG2_MOD_FORMAT_GFSK | RADIO_MDMCFG2_SYNC_MODE_16in16CS,   // MDMCFG2   Modem configuration.
   RADIO_MDMCFG1_NUM_PREAMBLE_4B | RADIO_MDMCFG1_CHANSPC_E_NORMAL_RATE,   // MDMCFG1   Modem configuration.
   RADIO_MDMCFG0_CHANSPC_M_NORMAL_RATE,       // MDMCFG0   Modem configuration.
   RADIO_DEVIATN_NORMAL_RATE,   // DEVIATN   Modem deviation setting (when FSK modulation is enabled).
   RADIO_MCSM2_RX_TIME(7),      // MCSM2         Main Radio Control State Machine configuration.
   RADIO_MCSM1_CCA_ALWAYS | RADIO_MCSM1_RXOFF_MODE_RX | RADIO_MCSM1_TXOFF_MODE_IDLE,    // MCSM1 Main Radio Control State Machine configuration.
   //RADIO_MCSM0_FS_AUTOCAL_FROMIDLE,// MCSM0     Main Radio Control State Machine configuration.
   RADIO_MCSM0_FS_AUTOCAL_4THIDLE,// MCSM0     Main Radio Control State Machine configuration.
   RADIO_FOCCFG_FOC_PRE_K_3K | RADIO_FOCCFG_FOC_POST_K_HALFK | RADIO_FOCCFG_FOC_LIMIT_4THBW,   // FOCCFG    Frequency Offset Compensation Configuration.
   RADIO_BSCFG_BS_PRE_KI_2KI | RADIO_BSCFG_BS_PRE_KP_3KP | RADIO_BSCFG_BS_POST_KI_1KP | RADIO_BSCFG_BS_POST_KP_1KP | RADIO_BSCFG_BS_LIMIT_0,   // BSCFG     Bit synchronization Configuration.
   RADIO_AGCCTRL2_MAX_DVGA_GAIN_ALL | RADIO_AGCCTRL2_MAX_LNA_GAIN_SUB0 | RADIO_AGCCTRL2_MAX_MAGN_TARGET_33,   // AGCCTRL2  AGC control.
   RADIO_AGCCTRL1_AGC_LNA_PRIORITY | RADIO_AGCCTRL1_CS_REL_THR_DISABLED | RADIO_AGCCTRL1_CS_ABS_THR_FLAT,   // AGCCTRL1  AGC control.
   RADIO_AGCCTRL0_HYST_LEVEL_MED | RADIO_AGCCTRL0_WAIT_ITME_16 | RADIO_AGCCTRL0_AGC_FREEZE_NORMAL | RADIO_AGCCTRL0_FILTER_LENGTH_16,   // AGCCTRL0  AGC control.
   RADIO_WOREVT1_EVENT0_HI(128),  // WOREVT1
   RADIO_WOREVT0_EVENT0_LO(0),    // WOREVT0
   RADIO_WORCTRL_ALCK_PD,         // WORCTRL
   RADIO_FREND1_LNA_CURRENT(1) | RADIO_FREND1_LNA2MIX_CURRENT(1) | RADIO_FREND1_LODIV_BUF_CURRENT_RX(1) | RADIO_FREND1_MIX_CURRENT(2),   // FREND1    Front end RX configuration.
   RADIO_FREND0_LODIV_BUF_CURRENT_TX(1) | RADIO_FREND0_PA_POWER(0),   // FREND0    Front end TX configuration.
   RADIO_FSCAL3_HI(3) | RADIO_FSCAL3_CHP_CURR_CAL_EN(2) | RADIO_FSCAL3_LO(10),   // FSCAL3    Frequency synthesizer calibration.
   RADIO_FSCAL2_FSCAL2(10) | RADIO_FSCAL2_VCO_CORE_H_EN,           // FSCAL2    Frequency synthesizer calibration.
   RADIO_FSCAL1(0),               // FSCAL1    Frequency synthesizer calibration.
   RADIO_FSCAL0(31)               // FSCAL0    Frequency synthesizer calibration.
};

static void switch_to_idle_mode()
{
    DPRINT("Switching to HW_RADIO_STATE_IDLE");
    //Flush FIFOs and go to sleep, ensure interrupts are disabled
    cc1101_interface_set_interrupts_enabled(CC1101_GDO0, false);
    cc1101_interface_set_interrupts_enabled(CC1101_GDO2, false);
    current_state = HW_RADIO_STATE_IDLE;
    cc1101_interface_strobe(RF_SFRX); // TODO cc1101 datasheet : Only issue SFRX in IDLE or RXFIFO_OVERFLOW states
    cc1101_interface_strobe(RF_SFTX); // TODO cc1101 datasheet : Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states.
    cc1101_interface_strobe(RF_SIDLE);
    cc1101_interface_strobe(RF_SPWD);
    DEBUG_TX_END();
    DEBUG_RX_END();
}

static inline int16_t convert_rssi(int8_t rssi_raw)
{
    // CC1101 RSSI is 0.5 dBm units, signed byte
    int16_t rssi = (int16_t)rssi_raw;        //Convert to signed 16 bit
    rssi += 128;                             //Make it positive...
    rssi >>= 1;                              //...So division to 1 dBm units can be a shift...
    rssi -= 64 + RSSI_OFFSET;                // ...and then rescale it, including offset

    return rssi;
}


static void ensure_settling_and_calibration_done()
{
    uint8_t status = (cc1101_interface_strobe(RF_SNOP)) & 0x70;
    uint8_t counter = 0;
    while ((status == 0x40) || (status == 0x50))
    {
        assert(counter++ < 100); // TODO measure in normal case
        hw_busy_wait(10);
        status = cc1101_interface_strobe(RF_SNOP);
    }
}

static void fifo_threshold_isr()
{
    switch(current_state)
    {
        case HW_RADIO_STATE_RX: ;
            // Do not empty the FIFO (See the CC1100 or 2500 Errata Note)
            cc1101_interface_read_burst_reg(RXFIFO, BufferIndex, (BYTES_IN_RX_FIFO - 1));
            bytesLeft -= (BYTES_IN_RX_FIFO - 1);
            BufferIndex += (BYTES_IN_RX_FIFO - 1);

            //c1101_interface_set_edge_interrupt(CC1101_GDO2, GPIO_RISING_EDGE);
            cc1101_interface_set_interrupts_enabled(CC1101_GDO2, true);

            break;
        case HW_RADIO_STATE_TX:
            if (writeRemainingDataFlag)
            {
                // We have space enough in the FIFO to write the remaining data
                cc1101_interface_write_burst_reg(TXFIFO, BufferIndex, bytesLeft);
                // Wait end of packet transmission
                c1101_interface_set_edge_interrupt(CC1101_GDO0, GPIO_FALLING_EDGE);
                cc1101_interface_set_interrupts_enabled(CC1101_GDO0, true);
            }
            else
            {
                cc1101_interface_write_burst_reg(TXFIFO, BufferIndex, AVAILABLE_BYTES_IN_TX_FIFO);

                BufferIndex += AVAILABLE_BYTES_IN_TX_FIFO;
                bytesLeft -= AVAILABLE_BYTES_IN_TX_FIFO;
                if (!(--iterations))
                    writeRemainingDataFlag = true;

                c1101_interface_set_edge_interrupt(CC1101_GDO2, GPIO_FALLING_EDGE);
                cc1101_interface_set_interrupts_enabled(CC1101_GDO2, true);
            }
            break;
        default:
            assert(false);
    }
}

static void end_of_packet_isr()
{
    DPRINT("end of packet ISR");
    switch(current_state)
    {
        case HW_RADIO_STATE_RX: ;
            uint8_t packet_len = 0;

            if (current_syncword_class == PHY_SYNCWORD_CLASS0)
            {
                DPRINT("BG packet received!");
                uint8_t packet_len;
                if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
                    packet_len = fec_calculated_decoded_length(BACKGROUND_FRAME_LENGTH);
                else
                    packet_len = BACKGROUND_FRAME_LENGTH;

                current_packet = alloc_packet_callback(packet_len);
                current_packet->length = BACKGROUND_FRAME_LENGTH;
                bytesLeft = packet_len;
                BufferIndex = current_packet->data + 1;
                endOfPacket = true;
            }

            if (!endOfPacket)
            {
                // After the sync word is received one needs to wait some time before there will be any data
                // in the FIFO. In addition, the FIFO should not be emptied
                // (See the CC1100 or 2500 Errata Note) before the whole packet has been received.
                uint8_t rx_bytes = 0;
                do
                {
                    cc1101_interface_read_burst_reg(RXBYTES, &rx_bytes, 1);
                    rx_bytes = rx_bytes & 0x7F;
                } while (rx_bytes < 4);  // TODO counter to avoid a dead lock

                uint8_t buffer[4];
                cc1101_interface_read_burst_reg(RXFIFO, buffer, 4);

                if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
                {
                    uint8_t fec_buffer[4];
                    memcpy(fec_buffer, buffer, 4);
                    fec_decode_packet(fec_buffer, 4, 4);
                    packet_len = fec_calculated_decoded_length(fec_buffer[0]+1);
                    DPRINT("RX Packet Length: %d / %d", fec_buffer[0], packet_len);
                }
                else
                {
                    packet_len = buffer[0] + 1;
                }

                current_packet = alloc_packet_callback(packet_len);
                memcpy(current_packet->data, buffer, 4);

                bytesLeft = packet_len - 4;
                BufferIndex = current_packet->data + 4;
                endOfPacket = true;

                if (bytesLeft < FIFO_SIZE)
                {
                    // Disable interrupt on threshold since it is room for the whole packet in the FIFO
                    cc1101_interface_set_interrupts_enabled(CC1101_GDO2, false);
                    if (bytesLeft <= rx_bytes)
                        goto end_of_packet;
                }
                else
                {
                    // Asserts when RX FIFO is filled at or above the RX FIFO threshold.
                    c1101_interface_set_edge_interrupt(CC1101_GDO2, GPIO_RISING_EDGE);
                    cc1101_interface_set_interrupts_enabled(CC1101_GDO2, true);
                }

                // Enables external interrupt on falling edge (packet received)
                c1101_interface_set_edge_interrupt(CC1101_GDO0, GPIO_FALLING_EDGE);
                cc1101_interface_set_interrupts_enabled(CC1101_GDO0, true);
            }
            else
            {   // End of Packet
end_of_packet:

                cc1101_interface_read_burst_reg(RXFIFO, BufferIndex, bytesLeft);
                endOfPacket = false;

                // fill rx_meta
                current_packet->rx_meta.rssi = convert_rssi(cc1101_interface_read_single_reg(RXFIFO));
                current_packet->rx_meta.lqi = cc1101_interface_read_single_reg(RXFIFO) & 0x7F;
                 memcpy(&(current_packet->rx_meta.rx_cfg.channel_id), &current_channel_id, sizeof(channel_id_t));
                current_packet->rx_meta.rx_cfg.syncword_class = current_syncword_class;
                current_packet->rx_meta.crc_status = HW_CRC_UNAVAILABLE; // TODO
                current_packet->rx_meta.timestamp = timer_get_counter_value();

                DEBUG_RX_END();
                if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
                {
                    if (current_syncword_class == PHY_SYNCWORD_CLASS0)
                        fec_decode_packet(current_packet->data + 1, packet_len, packet_len);
                    else
                    {
                        packet_len = (BufferIndex + bytesLeft) - current_packet->data;
                        fec_decode_packet(current_packet->data, packet_len, packet_len);
                    }
                }

                if(rx_packet_callback != NULL) // TODO this can happen while doing CCA but we should not be interrupting here (disable packet handler?)
                    rx_packet_callback(current_packet);
                else
                    release_packet_callback(current_packet);

                // check still in RX, could be modified by upper layer while in callback
                if ((current_state == HW_RADIO_STATE_RX) && (current_syncword_class != PHY_SYNCWORD_CLASS0))
                {
                    uint8_t status = (cc1101_interface_strobe(RF_SNOP) & 0xF0);
                    if(status == 0x60) // RX overflow
                    {
                        cc1101_interface_strobe(RF_SFRX);
                        while(cc1101_interface_strobe(RF_SNOP) != 0x0F); // wait until in idle state
                        cc1101_interface_strobe(RF_SRX);
                        while(cc1101_interface_strobe(RF_SNOP) != 0x1F); // wait until in RX state
                    }

                    c1101_interface_set_edge_interrupt(CC1101_GDO0, GPIO_RISING_EDGE);
                    cc1101_interface_set_interrupts_enabled(CC1101_GDO0, true);
                    assert(cc1101_interface_strobe(RF_SNOP) == 0x1F); // expect to be in RX mode
                }
            }
            break;
        case HW_RADIO_STATE_TX:
          DEBUG_TX_END();

          writeRemainingDataFlag = false;

          if(tx_packet_callback != 0)
          {
              current_packet->tx_meta.timestamp = timer_get_counter_value();
              tx_packet_callback(current_packet);
          }

          /* We can't switch back to Rx since the Rx callbacks are modified
           * during CCA, so we systematically go to idle
           */
          switch_to_idle_mode();
          break;
        default:
            assert(false);
    }
}

static inline void wait_for_chip_state(cc1101_chipstate_t expected_state)
{
    uint8_t chipstate;
    do {
        chipstate = cc1101_interface_read_single_reg(MARCSTATE);
    } while (chipstate != expected_state);
}

static void configure_channel(const channel_id_t* channel_id)
{
    //Update the PKTCTRL0 register since this register may be changed by background scan
    // CC1101 supports CRC as well as FEC, but the order of data whitening relative to FEC
    // is the reverse of what is specified by D7A. Therefore, the embedded FEC can't be enabled
    if (channel_id->channel_header.ch_coding == PHY_CODING_FEC_PN9)
    {
        DPRINT("FEC is applied in SW");
        // we use the fixed packet length mode since the first byte is FEC encoded
        cc1101_interface_write_single_reg(PKTCTRL0, RADIO_PKTCTRL0_WHITE_DATA | RADIO_PKTCTRL0_LENGTH_FIXED);
    }
    else if (channel_id->channel_header.ch_coding == PHY_CODING_PN9)
    {
        if (has_hardware_crc)
            cc1101_interface_write_single_reg(PKTCTRL0, RADIO_PKTCTRL0_WHITE_DATA | RADIO_PKTCTRL0_CRC | RADIO_PKTCTRL0_LENGTH_VAR);
        else
            cc1101_interface_write_single_reg(PKTCTRL0, RADIO_PKTCTRL0_WHITE_DATA | RADIO_PKTCTRL0_LENGTH_VAR);

        cc1101_interface_write_single_reg(PKTLEN, RADIO_PKTLEN);
     }
     else
     {
         // Receive the raw data as is.
         cc1101_interface_write_single_reg(PKTCTRL0, RADIO_PKTCTRL0_LENGTH_INF);
         cc1101_interface_write_single_reg(PKTLEN, RADIO_PKTLEN);
         DPRINT("Raw data applied !");
     }

    // only change settings if channel_id changed compared to current config
    if(!phy_radio_channel_ids_equal(channel_id, &current_channel_id))
    {
        // TODO assert valid center freq index

        cc1101_interface_strobe(RF_SIDLE); // we need to be in IDLE state before starting calibration
        wait_for_chip_state(CC1101_CHIPSTATE_IDLE);

        memcpy(&current_channel_id, channel_id, sizeof(channel_id_t)); // cache new settings

        // TODO preamble size depends on channel class

        // set freq band
        DPRINT("Set frequency band index: %d", channel_id->channel_header.ch_freq_band);

        // TODO validate
        switch(channel_id->channel_header.ch_freq_band)
        {
        // TODO calculate depending on rate and channr
        case PHY_BAND_433:
            if(channel_id->channel_header.ch_class == PHY_CLASS_NORMAL_RATE)
            {
                cc1101_interface_write_single_reg(FREQ2, RADIO_FREQ2(RADIO_FREQ_433_NORMAL_RATE));
                cc1101_interface_write_single_reg(FREQ1, RADIO_FREQ1(RADIO_FREQ_433_NORMAL_RATE));
                cc1101_interface_write_single_reg(FREQ0, RADIO_FREQ0(RADIO_FREQ_433_NORMAL_RATE));
                assert(channel_id->center_freq_index % 8 == 0 && channel_id->center_freq_index <= 56);
                DPRINT("Set channel freq index: %d", channel_id->center_freq_index);
                cc1101_interface_write_single_reg(CHANNR, channel_id->center_freq_index / 8);
            }
            else if(channel_id->channel_header.ch_class == PHY_CLASS_LO_RATE)
            {
                cc1101_interface_write_single_reg(FREQ2, RADIO_FREQ2(RADIO_FREQ_433_LO_RATE));
                cc1101_interface_write_single_reg(FREQ1, RADIO_FREQ1(RADIO_FREQ_433_LO_RATE));
                cc1101_interface_write_single_reg(FREQ0, RADIO_FREQ0(RADIO_FREQ_433_LO_RATE));
                assert(channel_id->center_freq_index <= 68);
                DPRINT("Set channel freq index: %d", channel_id->center_freq_index);
                cc1101_interface_write_single_reg(CHANNR, channel_id->center_freq_index);
            }
            else if(channel_id->channel_header.ch_class == PHY_CLASS_HI_RATE)
            {
                assert(false);
            }
            break;
        case PHY_BAND_868:
            if(channel_id->channel_header.ch_class == PHY_CLASS_NORMAL_RATE)
            {
                cc1101_interface_write_single_reg(FREQ2, RADIO_FREQ2(RADIO_FREQ_868_NORMAL_RATE));
                cc1101_interface_write_single_reg(FREQ1, RADIO_FREQ1(RADIO_FREQ_868_NORMAL_RATE));
                cc1101_interface_write_single_reg(FREQ0, RADIO_FREQ0(RADIO_FREQ_868_NORMAL_RATE));
                assert(channel_id->center_freq_index % 8 == 0 && channel_id->center_freq_index <= 272); // TODO should be 270?
                DPRINT("Set channel freq index: %d", channel_id->center_freq_index);
                cc1101_interface_write_single_reg(CHANNR, channel_id->center_freq_index / 8);
            }
            else
                assert(false);
            // TODO lo-rate and hi-rate
            break;
        case PHY_BAND_915:
            assert(false);
//            WriteSingleReg(RADIO_FREQ2, (uint8_t)(RADIO_FREQ_915>>16 & 0xFF));
//            WriteSingleReg(RADIO_FREQ1, (uint8_t)(RADIO_FREQ_915>>8 & 0xFF));
//            WriteSingleReg(RADIO_FREQ0, (uint8_t)(RADIO_FREQ_915 & 0xFF));
            break;
        }

        // set modulation, symbol rate, channel spacing and deviation
        switch(channel_id->channel_header.ch_class)
        {
            case PHY_CLASS_NORMAL_RATE:
                // TODO validate
                cc1101_interface_write_single_reg(MDMCFG3, RADIO_MDMCFG3_DRATE_M_NORMAL_RATE);
                cc1101_interface_write_single_reg(MDMCFG4, RADIO_MDMCFG4_NORMAL_RATE);
                cc1101_interface_write_single_reg(DEVIATN, RADIO_DEVIATN_NORMAL_RATE);
                cc1101_interface_write_single_reg(MDMCFG1, RADIO_MDMCFG1_NUM_PREAMBLE_4B | RADIO_MDMCFG1_CHANSPC_E_NORMAL_RATE);
                cc1101_interface_write_single_reg(MDMCFG0, RADIO_MDMCFG0_CHANSPC_M_NORMAL_RATE);
                break;
            case PHY_CLASS_LO_RATE:
                cc1101_interface_write_single_reg(MDMCFG3, RADIO_MDMCFG3_DRATE_M_LOW_RATE);
                cc1101_interface_write_single_reg(MDMCFG4, RADIO_MDMCFG4_LOW_RATE);
                cc1101_interface_write_single_reg(DEVIATN, RADIO_DEVIATN_LOW_RATE);
                // TODO using channel spacing to switch channels is not accurate here since min value for spacing is 25.39 kHz instead
                // of 25 Khz resulting in a big offset after x channels ... change center freq instead
                cc1101_interface_write_single_reg(MDMCFG1, RADIO_MDMCFG1_NUM_PREAMBLE_4B | RADIO_MDMCFG1_CHANSPC_E_LO_RATE);
                cc1101_interface_write_single_reg(MDMCFG0, RADIO_MDMCFG0_CHANSPC_M_LO_RATE);

            break;
            default:
                assert(false);
                // TODO: other classes
        }

        cc1101_interface_strobe(RF_SCAL); // TODO use autocalibration instead of manual?
        wait_for_chip_state(CC1101_CHIPSTATE_IDLE);
    }

}

static void configure_eirp(const eirp_t eirp)
{
    if(eirp != current_eirp)
    {
        current_eirp = eirp;
        uint8_t register_value = 0xC0;
        switch(eirp) // TODO band dependent!
        {
          case 0:
            register_value = 0x60;
            break;
          case 10:
            register_value = 0xC0;
            break;
          default:
            assert(false);
        }

        cc1101_interface_write_single_patable(register_value); // TODO only 10 dBm supported for now
    }
}

static void configure_syncword(syncword_class_t syncword_class, phy_coding_t ch_coding)
{
    if((syncword_class != current_syncword_class) || (ch_coding != current_channel_id.channel_header.ch_coding))
    {
        current_syncword_class = syncword_class;
        uint16_t sync_word = sync_word_value[syncword_class][ch_coding];

        DPRINT("sync_word = %04x", sync_word);
        cc1101_interface_write_single_reg(SYNC0, sync_word & 0xFF);
        cc1101_interface_write_single_reg(SYNC1, sync_word >> 8);
    }
}

error_t hw_radio_init(alloc_packet_callback_t alloc_packet_cb,
                      release_packet_callback_t release_packet_cb)
{
    alloc_packet_callback = alloc_packet_cb;
    release_packet_callback = release_packet_cb;

    current_state = HW_RADIO_STATE_IDLE;

    sched_register_task(&switch_to_idle_mode);

    cc1101_interface_init(&end_of_packet_isr, &fifo_threshold_isr);
    cc1101_interface_reset_radio_core();
    cc1101_interface_write_rfsettings(&rf_settings);

    DPRINT("RF settings:");
    uint8_t* p = (uint8_t*) &rf_settings;
    uint8_t i;
    for(i = 0; i < sizeof(RF_SETTINGS); i++)
    {
        DPRINT("\t0x%02X", p[i]);
    }

    // configure default channel, eirp and syncword
    configure_channel(&current_channel_id);
    configure_eirp(current_eirp);
    configure_syncword(current_syncword_class, current_channel_id.channel_header.ch_coding);
    cc1101_interface_write_single_reg(FIFOTHR, 0x04);
    // FIFO_THR = 4
    // 45 bytes in TX FIFO (19 available spaces)
    // 20 bytes in the RX FIFO
    // If this threshold is changed, AVAILABLE_BYTES_IN_TX_FIFO
    // and BYTES_RX_FIFO must be updated

    cc1101_interface_strobe(RF_SCAL); // TODO use autocalibration instead of manual?
    wait_for_chip_state(CC1101_CHIPSTATE_IDLE);
}

static void start_rx(hw_rx_cfg_t const* rx_cfg)
{
    current_state = HW_RADIO_STATE_RX;

//    uint8_t status = 0x80;
//
//    while (status == 0x80)
//    {
          // Should not be 0x80 -> means Chip not ready
//        status = cc1101_interface_strobe(RF_SNOP) & 0x80;
//    }

    configure_channel(&(rx_cfg->channel_id));
    configure_syncword(rx_cfg->syncword_class, rx_cfg->channel_id.channel_header.ch_coding);
    cc1101_interface_write_single_reg(PKTLEN, 0xFF);
    // Associated to the RX FIFO: Asserts when RX FIFO is filled at or above the
    // RX FIFO threshold. De-asserts when RX FIFO is drained below the same threshold
    cc1101_interface_write_single_reg(IOCFG2, 0x00);

    // cc1101_interface_strobe(RF_SFRX); TODO only when in idle or overflow state

    DPRINT("START FG scan @ %i", timer_get_counter_value());

    uint8_t status;
    uint8_t counter = 0;
    do
    {
        status = cc1101_interface_strobe(RF_SRX);
        if(status == 0x6F)
        {
            // RX FIFO overflow, flush first
            cc1101_interface_strobe(RF_SFRX);
        }

        assert(counter++ < 100); // TODO measure value in normal case
    } while(status != 0x1F);

    DEBUG_RX_START();
    if(rx_packet_callback != 0) // when rx callback not set we ignore received packets
    {
        c1101_interface_set_edge_interrupt(CC1101_GDO0, GPIO_RISING_EDGE);
        cc1101_interface_set_interrupts_enabled(CC1101_GDO0, true);
    }
    else
        cc1101_interface_set_interrupts_enabled(CC1101_GDO0, false);

    // TODO when only rssi callback set the packet handler is still active and we enter in RXFIFOOVERFLOW, find a way to around this

    if(rssi_valid_callback != 0)
    {
        // TODO calculate/predict rssi response time (see DN505)
        // and wait until valid. For now we wait 200 us.

        hw_busy_wait(200);
        ensure_settling_and_calibration_done();
        rssi_valid_callback(hw_radio_get_rssi());
    }
}

error_t hw_radio_set_rx(hw_rx_cfg_t const* rx_cfg, rx_packet_callback_t rx_cb, rssi_valid_callback_t rssi_valid_cb)
{
    if(rx_cb != NULL)
    {
        assert(alloc_packet_callback != NULL);
        assert(release_packet_callback != NULL);
    }

    // assert(rx_cb != NULL || rssi_valid_cb != NULL); // at least one callback should be valid

    // TODO error handling EINVAL, EOFF
    rx_packet_callback = rx_cb;
    rssi_valid_callback = rssi_valid_cb;

    // if we are currently transmitting wait until TX completed before entering RX
    // we return now and go into RX when TX is completed
    if(current_state == HW_RADIO_STATE_TX)
    {
        should_rx_after_tx_completed = true;
        memcpy(&pending_rx_cfg, rx_cfg, sizeof(hw_rx_cfg_t));
        return SUCCESS;
    }

    start_rx(rx_cfg);

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
    assert(current_state != HW_RADIO_STATE_TX);

    current_state = HW_RADIO_STATE_RX;

    configure_syncword(rx_cfg->syncword_class, rx_cfg->channel_id.channel_header.ch_coding);
    configure_channel(&(rx_cfg->channel_id));

    cc1101_interface_write_single_reg(PKTCTRL0, RADIO_PKTCTRL0_WHITE_DATA | RADIO_PKTCTRL0_LENGTH_FIXED);

    if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
        packet_len = fec_calculated_decoded_length(BACKGROUND_FRAME_LENGTH);
    else
        packet_len = BACKGROUND_FRAME_LENGTH;

    cc1101_interface_write_single_reg(PKTLEN, packet_len);
    DPRINT("packet length %d", packet_len);

    DEBUG_RX_START();

    uint8_t status;
    uint8_t counter = 0;
    do
    {
        status = cc1101_interface_strobe(RF_SRX);
        if(status == 0x6F)
        {
            // RX FIFO overflow, flush first
            DPRINT("RX FIFO overflow");
            cc1101_interface_strobe(RF_SFRX);
        }

        assert(counter++ < 100); // TODO measure value in normal case
    } while(status != 0x1F);

    // Fast RX termination if no carrier is detected
    ensure_settling_and_calibration_done();

    int16_t rssi = hw_radio_get_rssi();
    if (rssi <= rssi_thr)
    {
        DPRINT("FAST RX termination RSSI %i limit %i", rssi, rssi_thr);
        switch_to_idle_mode();
        DEBUG_RX_END();
        return FAIL;
    }
    else
    {
        c1101_interface_set_edge_interrupt(CC1101_GDO0, GPIO_FALLING_EDGE);
        cc1101_interface_set_interrupts_enabled(CC1101_GDO0, true);
    }

    // the device has a period of To to successfully detect the sync word
    assert(timer_post_task_delay(&switch_to_idle_mode, bg_timeout[current_channel_id.channel_header.ch_class]) == SUCCESS);

    return SUCCESS;
}


error_t hw_radio_send_packet(hw_radio_packet_t* packet, tx_packet_callback_t tx_cb, uint16_t eta, uint8_t dll_header_bg_frame[2])
{
    assert(eta == 0); // advertising not implemented on cc1101 for now

    // TODO error handling EINVAL, ESIZE, EOFF
    if(current_state == HW_RADIO_STATE_TX)
        return EBUSY;

    assert(packet->length < MODULE_D7AP_RAW_PACKET_SIZE);

    tx_packet_callback = tx_cb;

    if(current_state == HW_RADIO_STATE_RX)
    {
        pending_rx_cfg.channel_id = current_channel_id;
        pending_rx_cfg.syncword_class = current_syncword_class;
        should_rx_after_tx_completed = true;
        // when in RX state we can directly strobe TX and do not need to wait until in IDLE
    }
    else
    {
        wait_for_chip_state(CC1101_CHIPSTATE_IDLE); // TODO reading state sometimes returns illegal values such as 0x1F.
                                                // polling for this seems to take 50-200us after a quick test, not sure why yet
    }

    current_state = HW_RADIO_STATE_TX;
    current_packet = packet;

    configure_channel((channel_id_t*)&(current_packet->tx_meta.tx_cfg.channel_id));
    configure_eirp(current_packet->tx_meta.tx_cfg.eirp);
    configure_syncword(current_packet->tx_meta.tx_cfg.syncword_class,
                       current_packet->tx_meta.tx_cfg.channel_id.channel_header.ch_coding);

    // Associated to the TX FIFO: Asserts when the TX FIFO is filled above TXFIFO_THR.
    // De-asserts when the TX FIFO is below TXFIFO_THR.
    cc1101_interface_write_single_reg(IOCFG2, 0x02);

    // The entire packet can be written at once
    if (packet->length < FIFO_SIZE)
    {
        cc1101_interface_write_burst_reg(TXFIFO, packet->data, packet->length + 1);

        c1101_interface_set_edge_interrupt(CC1101_GDO0, GPIO_FALLING_EDGE);
        cc1101_interface_set_interrupts_enabled(CC1101_GDO0, true);
        DEBUG_TX_START();
        DEBUG_RX_END();
        cc1101_interface_strobe(RF_STX);
    }
    else
    {   // The TX FIFO needs to be re-filled several times
        DPRINT("Data to TX Fifo (First part of %d bytes):", packet->length + 1);
        DPRINT_DATA(packet->data, FIFO_SIZE);

        cc1101_interface_write_burst_reg(TXFIFO, packet->data, FIFO_SIZE); // Fill up the TX FIFO

        DEBUG_TX_START();
        DEBUG_RX_END();

        bytesLeft = packet->length + 1 - FIFO_SIZE;
        BufferIndex = packet->data + FIFO_SIZE;
        iterations = (bytesLeft / AVAILABLE_BYTES_IN_TX_FIFO);
        if (!iterations)
             writeRemainingDataFlag = true;

        c1101_interface_set_edge_interrupt(CC1101_GDO2, GPIO_FALLING_EDGE);
        cc1101_interface_set_interrupts_enabled(CC1101_GDO2, true);
        cc1101_interface_strobe(RF_STX);
    }

    return SUCCESS;
}

error_t hw_radio_send_background_packet(hw_radio_packet_t* packet, tx_packet_callback_t tx_cb,
                                        uint16_t eta, uint16_t tx_duration)
{
    uint8_t adv_packet[20];  // 6 bytes preamble (PREAMBLE_HI_RATE_CLASS) + 2 bytes SYNC word + 12 bytes max for a background frame FEC encoded
    uint8_t *packet_payload;
    uint8_t payload_len = packet->length;
    uint8_t *tx_packet;
    uint8_t tx_len;
    uint16_t crc, swap_eta;
    uint8_t bytes_in_fifo;
    bool fifo_filled = false;
    timer_tick_t start_time;
    timer_tick_t stop_time = eta;

    // TODO error handling EINVAL, ESIZE, EOFF
    if(current_state == HW_RADIO_STATE_TX)
        return EBUSY;

    assert(packet->length < MODULE_D7AP_RAW_PACKET_SIZE);

    tx_packet_callback = tx_cb;

    if(current_state == HW_RADIO_STATE_RX)
    {
        pending_rx_cfg.channel_id = current_channel_id;
        pending_rx_cfg.syncword_class = current_syncword_class;
        should_rx_after_tx_completed = true;
        // when in RX state we can directly strobe TX and do not need to wait until in IDLE
    }
    else
    {
        wait_for_chip_state(CC1101_CHIPSTATE_IDLE); // TODO reading state sometimes returns illegal values such as 0x1F.
                                                    // polling for this seems to take 50-200us after a quick test, not sure why yet
        current_state = HW_RADIO_STATE_IDLE;
    }

    current_packet = packet;

    configure_syncword(current_packet->tx_meta.tx_cfg.syncword_class,
                       current_packet->tx_meta.tx_cfg.channel_id.channel_header.ch_coding);
    configure_channel((channel_id_t*)&(current_packet->tx_meta.tx_cfg.channel_id));
    configure_eirp(current_packet->tx_meta.tx_cfg.eirp);

    // During the advertising flooding, use the infinite packet length mode and disable the hardware PN9/FEC/CRC
    cc1101_interface_write_single_reg(PKTCTRL0, RADIO_PKTCTRL0_LENGTH_INF);
    cc1101_interface_write_single_reg(PKTLEN, 0xFF);

    // Prepare the subsequent background frames which include the preamble and the sync word
    uint8_t preamble_len = (current_channel_id.channel_header.ch_class ==  PHY_CLASS_HI_RATE ? PREAMBLE_HI_RATE_CLASS : PREAMBLE_LOW_RATE_CLASS);
    memset(adv_packet, 0xAA, preamble_len); // preamble length is given in number of bytes
    uint16_t sync_word = __builtin_bswap16(sync_word_value[current_syncword_class][current_channel_id.channel_header.ch_coding]);
    memcpy(&adv_packet[preamble_len], &sync_word, 2);
    packet_payload = adv_packet + preamble_len + 2;
    memcpy(packet_payload, current_packet->data + 1, payload_len); // The length byte is not included in the background payload

    DPRINT("Original payload: %d", payload_len);
    DPRINT_DATA(packet_payload, payload_len);

    if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
    {
        tx_len = fec_encode(packet_payload, payload_len);
        pn9_encode(packet_payload, tx_len);
    }
    else
    {
        pn9_encode(packet_payload, payload_len);
        tx_len = payload_len;
    }

    // For the first advertising frame, transmit directly the payload since the preamble and the sync word are directly managed by the xcv
    tx_packet = packet_payload;
    DPRINT("Transmit packet: %d", tx_len);
    DPRINT_DATA(tx_packet, tx_len);

    // Fill the FIFO with the first BG frame (preamble and sync word are appended by the transceiver)
    cc1101_interface_write_burst_reg(TXFIFO, tx_packet, tx_len); // Fill up the TX FIFO

    // for the next advertising frame, insert the preamble and the SYNC word
    tx_packet = adv_packet;
    tx_len += preamble_len + 2;

    while (eta > tx_duration)
    {
        // start Tx when the FIFO is almost full
        if ((current_state != HW_RADIO_STATE_TX) && (fifo_filled))
        {
            current_state = HW_RADIO_STATE_TX;
            DEBUG_TX_START();
            DEBUG_RX_END();

            start_time = timer_get_counter_value();
            stop_time += start_time;
            DPRINT("Start transmitting @ %i stop @ %i", start_time, stop_time);
            cc1101_interface_strobe(RF_STX);
        }

        /*
         * Build the next advertising frame.
         * In order to flood the channel with advertising frames without discontinuity,
         * the FIFO is refilled with the next frames within the same TX.
         * For that, the preamble and the sync word are explicitly inserted before each
         * subsequent advertising frame.
         */

        // first recover the initial payload
        memcpy(packet_payload, current_packet->data + 1, payload_len);

        // update eta
        if (fifo_filled)
            eta = stop_time - timer_get_counter_value();
        else
            eta -= tx_duration;

        swap_eta = __builtin_bswap16(eta);
        memcpy(&packet_payload[2], &swap_eta, sizeof(uint16_t));

        // update the CRC
        crc = __builtin_bswap16(crc_calculate(packet_payload, 4));
        memcpy(&packet_payload[4], &crc, 2);

        if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
        {
            pn9_encode(packet_payload, fec_encode(packet_payload, payload_len));
        }
        else
        {
            pn9_encode(packet_payload, payload_len);
        }

        // wait for some space available in the TX FIFO
        do
        {
            cc1101_interface_read_burst_reg(TXBYTES, &bytes_in_fifo, 1);
            if (bytes_in_fifo & 0x80)
                DPRINT("TX FIFO underflow %x", bytes_in_fifo);
            bytes_in_fifo = bytes_in_fifo & 0x7F;
        } while (FIFO_SIZE - bytes_in_fifo < tx_len);

        if ((FIFO_SIZE - bytes_in_fifo) < (tx_len * 2))
            fifo_filled = true;

        //DPRINT("Transmit packet: %d", tx_len);
        //DPRINT_DATA(tx_packet, tx_len);
        //DPRINT("ETA %d", eta);
        cc1101_interface_write_burst_reg(TXFIFO, tx_packet, tx_len); // Fill up the TX FIFO
     }

    /*
     * When no more advertising background frames can be fully transmitted before
     * the start of D7ANP, the last background frame is extended by padding preamble
     * symbols after the end of the background packet, in order to guarantee no silence period.
     */
    if (eta)
    {
        preamble_len = eta / (tx_duration / tx_len);
        DPRINT("Add preamble_bytes: %d", preamble_len);
        memset(tx_packet, 0xAA, preamble_len);

        do
        {
            cc1101_interface_read_burst_reg(TXBYTES, &bytes_in_fifo, 1);
            bytes_in_fifo = bytes_in_fifo & 0x7F;
        } while (FIFO_SIZE - bytes_in_fifo < preamble_len);
        cc1101_interface_write_burst_reg(TXFIFO, tx_packet, preamble_len); // Fill up the TX FIFO
    }

    /* wait FIFO empty before calling the callback */
    if(tx_packet_callback != 0)
    {
        do
        {
            cc1101_interface_read_burst_reg(TXBYTES, &bytes_in_fifo, 1);
            bytes_in_fifo = bytes_in_fifo & 0x7F;
        } while (bytes_in_fifo);
        DPRINT("End AdvP @ %i", timer_get_counter_value());
        switch_to_idle_mode();
        tx_packet_callback(current_packet);
    }

    return SUCCESS;
}

int16_t hw_radio_get_rssi()
{
    return convert_rssi(cc1101_interface_read_single_reg(RSSI));
}

error_t hw_radio_set_idle()
{
    // if we are currently transmitting wait until TX completed before entering IDLE
    // we return now and go into IDLE when TX is completed
    if(current_state == HW_RADIO_STATE_TX)
    {
        should_rx_after_tx_completed = false;
        // Check if it is more safe to remove the callback or to disable the Tx interrupt
        tx_packet_callback = NULL;
        return SUCCESS;
    }

    if (current_state != HW_RADIO_STATE_IDLE)
        switch_to_idle_mode();

    return SUCCESS;
}

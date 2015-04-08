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

#include "assert.h"
#include "string.h"

#include "log.h"
#include "hwradio.h"

#include "cc1101.h"
#include "cc1101_interface.h"
#include "cc1101_constants.h"
#include "cc1101_registers.h"

// turn on/off the debug prints
#ifdef FRAMEWORK_LOG_ENABLED // TODO more granular (LOG_PHY_ENABLED)
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

#define RSSI_OFFSET 74

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
    .ch_coding = PHY_CODING_PN9,
    .ch_class = PHY_CLASS_NORMAL_RATE,
    .ch_freq_band = PHY_BAND_433,
    .center_freq_index = 0
};

static syncword_class_t current_syncword_class = PHY_SYNCWORD_CLASS0;
static syncword_class_t current_eirp = 0;

static bool should_rx_after_tx_completed = false;
static hw_rx_cfg_t pending_rx_cfg;

static void start_rx(hw_rx_cfg_t const* rx_cfg);

static RF_SETTINGS rf_settings = {
   RADIO_GDO2_VALUE,   			// IOCFG2    GDO2 output pin configuration.
   RADIO_GDO1_VALUE,    			// IOCFG1    GDO1 output pin configuration.
   RADIO_GDO0_VALUE,   			// IOCFG0    GDO0 output pin configuration.
   RADIO_FIFOTHR_FIFO_THR_61_4,   	// FIFOTHR   RXFIFO and TXFIFO thresholds.
   RADIO_SYNC1,     				// SYNC1	 Sync word, high byte
   RADIO_SYNC0,     				// SYNC0	 Sync word, low byte
   RADIO_PKTLEN,    				// PKTLEN    Packet length.
   RADIO_PKTCTRL1_PQT(3) | RADIO_PKTCTRL1_APPEND_STATUS,   // PKTCTRL1  Packet automation control.
   RADIO_PKTCTRL0_WHITE_DATA | RADIO_PKTCTRL0_LENGTH_VAR,      // PKTCTRL0  Packet automation control.
   RADIO_ADDR,   					// ADDR      Device address.
   RADIO_CHAN,   					// CHANNR    Channel number.
   RADIO_FREQ_IF,   				// FSCTRL1   Frequency synthesizer control.
   RADIO_FREQOFF,   				// FSCTRL0   Frequency synthesizer control.
   RADIO_FREQ2,   					// FREQ2     Frequency control word, high byte.
   RADIO_FREQ1,   					// FREQ1     Frequency control word, middle byte.
   RADIO_FREQ0,   					// FREQ0     Frequency control word, low byte.
   RADIO_MDMCFG4_CHANBW_E(1) | RADIO_MDMCFG4_CHANBW_M(0) | RADIO_MDMCFG4_DRATE_E(11),   // MDMCFG4   Modem configuration.
   RADIO_MDMCFG3_DRATE_M(24),   	// MDMCFG3   Modem configuration.
   RADIO_MDMCFG2_DEM_DCFILT_ON | RADIO_MDMCFG2_MOD_FORMAT_GFSK | RADIO_MDMCFG2_SYNC_MODE_16in16CS,   // MDMCFG2   Modem configuration.
   RADIO_MDMCFG1_NUM_PREAMBLE_4B | RADIO_MDMCFG1_CHANSPC_E(2),   // MDMCFG1   Modem configuration.
   RADIO_MDMCFG0_CHANSPC_M(16),   	// MDMCFG0   Modem configuration.
   RADIO_DEVIATN_E(5) | RADIO_DEVIATN_M(0),   // DEVIATN   Modem deviation setting (when FSK modulation is enabled).
   RADIO_MCSM2_RX_TIME(7),			// MCSM2		 Main Radio Control State Machine configuration.
   RADIO_MCSM1_CCA_RSSILOWRX | RADIO_MCSM1_RXOFF_MODE_RX | RADIO_MCSM1_TXOFF_MODE_IDLE,	// MCSM1 Main Radio Control State Machine configuration.
   //RADIO_MCSM0_FS_AUTOCAL_FROMIDLE,// MCSM0     Main Radio Control State Machine configuration.
   RADIO_MCSM0_FS_AUTOCAL_4THIDLE,// MCSM0     Main Radio Control State Machine configuration.
   RADIO_FOCCFG_FOC_PRE_K_3K | RADIO_FOCCFG_FOC_POST_K_HALFK | RADIO_FOCCFG_FOC_LIMIT_4THBW,   // FOCCFG    Frequency Offset Compensation Configuration.
   RADIO_BSCFG_BS_PRE_KI_2KI | RADIO_BSCFG_BS_PRE_KP_3KP | RADIO_BSCFG_BS_POST_KI_1KP | RADIO_BSCFG_BS_POST_KP_1KP | RADIO_BSCFG_BS_LIMIT_0,   // BSCFG     Bit synchronization Configuration.
   RADIO_AGCCTRL2_MAX_DVGA_GAIN_ALL | RADIO_AGCCTRL2_MAX_LNA_GAIN_SUB0 | RADIO_AGCCTRL2_MAX_MAGN_TARGET_33,   // AGCCTRL2  AGC control.
   RADIO_AGCCTRL1_AGC_LNA_PRIORITY | RADIO_AGCCTRL1_CS_REL_THR_DISABLED | RADIO_AGCCTRL1_CS_ABS_THR_FLAT,   // AGCCTRL1  AGC control.
   RADIO_AGCCTRL0_HYST_LEVEL_MED | RADIO_AGCCTRL0_WAIT_ITME_16 | RADIO_AGCCTRL0_AGC_FREEZE_NORMAL | RADIO_AGCCTRL0_FILTER_LENGTH_16,   // AGCCTRL0  AGC control.
   RADIO_WOREVT1_EVENT0_HI(128), 	// WOREVT1
   RADIO_WOREVT0_EVENT0_LO(0),		// WOREVT0
   RADIO_WORCTRL_ALCK_PD,			// WORCTRL
   RADIO_FREND1_LNA_CURRENT(1) | RADIO_FREND1_LNA2MIX_CURRENT(1) | RADIO_FREND1_LODIV_BUF_CURRENT_RX(1) | RADIO_FREND1_MIX_CURRENT(2),   // FREND1    Front end RX configuration.
   RADIO_FREND0_LODIV_BUF_CURRENT_TX(1) | RADIO_FREND0_PA_POWER(0),   // FREND0    Front end TX configuration.
   RADIO_FSCAL3_HI(3) | RADIO_FSCAL3_CHP_CURR_CAL_EN(2) | RADIO_FSCAL3_LO(10),   // FSCAL3    Frequency synthesizer calibration.
   RADIO_FSCAL2_FSCAL2(10),   		// FSCAL2    Frequency synthesizer calibration.
   RADIO_FSCAL1(0),   				// FSCAL1    Frequency synthesizer calibration.
   RADIO_FSCAL0(31)   				// FSCAL0    Frequency synthesizer calibration.
};

static void switch_to_idle_mode()
{
    //Flush FIFOs and go to sleep, ensure interrupts are disabled
    cc1101_interface_set_interrupts_enabled(false);
    cc1101_interface_strobe(RF_SFRX); // TODO cc1101 datasheet : Only issue SFRX in IDLE or RXFIFO_OVERFLOW states
    cc1101_interface_strobe(RF_SFTX); // TODO cc1101 datasheet : Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states.
    cc1101_interface_strobe(RF_SIDLE);
    cc1101_interface_strobe(RF_SPWD);
    current_state = HW_RADIO_STATE_IDLE;
}

static inline int16_t convert_rssi(int8_t rssi_raw)
{
    // CC1101 RSSI is 0.5 dBm units, signed byte
    int16_t rssi = (int16_t)rssi_raw;		//Convert to signed 16 bit
    rssi += 128;                      		//Make it positive...
    rssi >>= 1;                        		//...So division to 1 dBm units can be a shift...
    rssi -= 64 + RSSI_OFFSET;     			// ...and then rescale it, including offset

    return rssi;
}

static void end_of_packet_isr()
{
    cc1101_interface_set_interrupts_enabled(false);
    DPRINT("end of packet ISR");
    switch(current_state)
    {
        case HW_RADIO_STATE_RX: ;
            uint8_t packet_len = cc1101_interface_read_single_reg(RXFIFO);
            DPRINT("EOP ISR packetLength: %d", packet_len);
            hw_radio_packet_t* packet = alloc_packet_callback(packet_len);
            packet->length = packet_len;
            assert(packet->length < 63); // long packets not yet supported
            cc1101_interface_read_burst_reg(RXFIFO, packet->data + 1, packet->length);

            // fill rx_meta
            packet->rx_meta.rssi = convert_rssi(cc1101_interface_read_single_reg(RXFIFO));
            packet->rx_meta.lqi = cc1101_interface_read_single_reg(RXFIFO) & 0x7F;
            memcpy(&(packet->rx_meta.rx_cfg.channel_id), &current_channel_id, sizeof(channel_id_t));
            packet->rx_meta.crc_status = HW_CRC_UNAVAILABLE; // TODO
            packet->rx_meta.timestamp = timer_get_counter_value();

            rx_packet_callback(packet);
            cc1101_interface_set_interrupts_enabled(true);
            break;
        case HW_RADIO_STATE_TX:
            switch_to_idle_mode();

            current_packet->tx_meta.timestamp = timer_get_counter_value();
            if(tx_packet_callback != 0)
                tx_packet_callback(current_packet);

            if(should_rx_after_tx_completed)
            {
                // RX requested while still in TX ...
                // TODO this could probably be further optimized by not going into IDLE
                // after RX by setting TXOFF_MODE to RX (if the cfg is the same at least)
                should_rx_after_tx_completed = false;
                start_rx(&pending_rx_cfg);
            }
            break;
        default:
            assert(false);
    }
}

static void configure_channel(const channel_id_t* channel_id)
{
    // only change settings if channel_id changed compared to current config
    if(!hw_radio_channel_ids_equal(channel_id, &current_channel_id))
    {
        assert(channel_id->ch_freq_band == PHY_BAND_433); // TODO implement other bands
        assert(channel_id->ch_class == PHY_CLASS_NORMAL_RATE); // TODO implement other rates
        assert(channel_id->ch_coding == PHY_CODING_PN9); // TODO implement other codings
        // TODO assert valid center freq index

        memcpy(&current_channel_id, channel_id, sizeof(channel_id_t)); // cache new settings

        // TODO preamble size depends on channel class

        // set freq band
        DPRINT("Set frequency band index: %d", channel_id->ch_freq_band);
        // TODO validate
        /*
        switch(frequency_band)
            {
            case 0:
                WriteSingleReg(RADIO_FREQ2, (uint8_t)(RADIO_FREQ_433>>16 & 0xFF));
                WriteSingleReg(RADIO_FREQ1, (uint8_t)(RADIO_FREQ_433>>8 & 0xFF));
                WriteSingleReg(RADIO_FREQ0, (uint8_t)(RADIO_FREQ_433 & 0xFF));
                break;
            case 1:
                WriteSingleReg(RADIO_FREQ2, (uint8_t)(RADIO_FREQ_868>>16 & 0xFF));
                WriteSingleReg(RADIO_FREQ1, (uint8_t)(RADIO_FREQ_868>>8 & 0xFF));
                WriteSingleReg(RADIO_FREQ0, (uint8_t)(RADIO_FREQ_868 & 0xFF));
                break;
            case 2:
                WriteSingleReg(RADIO_FREQ2, (uint8_t)(RADIO_FREQ_915>>16 & 0xFF));
                WriteSingleReg(RADIO_FREQ1, (uint8_t)(RADIO_FREQ_915>>8 & 0xFF));
                WriteSingleReg(RADIO_FREQ0, (uint8_t)(RADIO_FREQ_915 & 0xFF));
                break;
            }
        */

        // set channel center frequency
        DPRINT("Set channel freq index: %d", channel_id->center_freq_index);
        cc1101_interface_write_single_reg(CHANNR, channel_id->center_freq_index); // TODO validate

        // set modulation, symbol rate and deviation
        switch(channel_id->ch_class)
        {
            case PHY_CLASS_NORMAL_RATE:
                // TODO validate
                cc1101_interface_write_single_reg(MDMCFG3, RADIO_MDMCFG3_DRATE_M(24));
                cc1101_interface_write_single_reg(MDMCFG4, (RADIO_MDMCFG4_CHANBW_E(1) | RADIO_MDMCFG4_CHANBW_M(0) | RADIO_MDMCFG4_DRATE_E(11)));
                cc1101_interface_write_single_reg(DEVIATN, (RADIO_DEVIATN_E(5) | RADIO_DEVIATN_M(0)));
                break;
                // TODO: other classes
        }
    }

    cc1101_interface_strobe(RF_SCAL); // TODO is this the right case?
}

static void configure_eirp(const eirp_t eirp)
{
    if(eirp != current_eirp)
    {
        // TODO set EIRP
        current_eirp = eirp;
    }
}

static void configure_syncword_class(syncword_class_t syncword_class)
{
    if(syncword_class != current_syncword_class)
    {
        // TODO set syncword
        current_syncword_class = syncword_class;
    }
}

error_t hw_radio_init(alloc_packet_callback_t alloc_packet_cb,
                      release_packet_callback_t release_packet_cb)
{
    alloc_packet_callback = alloc_packet_cb;
    release_packet_callback = release_packet_cb;

    current_state = HW_RADIO_STATE_IDLE;

    cc1101_interface_init(&end_of_packet_isr);
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
    configure_syncword_class(current_syncword_class);
}

static void start_rx(hw_rx_cfg_t const* rx_cfg)
{
    current_state = HW_RADIO_STATE_RX;

    cc1101_interface_strobe(RF_SFRX);
    configure_channel(&(rx_cfg->channel_id));
    configure_syncword_class(rx_cfg->syncword_class);
    cc1101_interface_write_single_reg(PKTLEN, 0xFF);

    if(rx_packet_callback != 0) // when rx callback not set we ignore received packets
        cc1101_interface_set_interrupts_enabled(true);

    cc1101_interface_strobe(RF_SRX);

    if(rssi_valid_callback != 0)
    {
        // TODO calculate/predict rssi response time (see DN505)
        // and wait until valid. For now we callback immediately
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

    assert(rx_cb != NULL || rssi_valid_cb != NULL); // at least one callback should be valid

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

error_t hw_radio_send_packet(hw_radio_packet_t* packet, tx_packet_callback_t tx_cb)
{
    // TODO error handling EINVAL, ESIZE, EOFF
    if(current_state == HW_RADIO_STATE_TX)
        return EBUSY;

    assert(packet->length < 63); // long packets not yet supported

    tx_packet_callback = tx_cb;

    current_state = HW_RADIO_STATE_TX;
    current_packet = packet;
    cc1101_interface_strobe(RF_SIDLE);
    cc1101_interface_strobe(RF_SFTX);

#ifdef FRAMEWORK_LOG_ENABLED // TODO more granular
    log_print_stack_string(LOG_STACK_PHY, "Data to TX Fifo:");
    log_print_data(packet->data, packet->length + 1);
#endif

    configure_channel((channel_id_t*)&(current_packet->tx_meta.tx_cfg.channel_id));
    configure_eirp(current_packet->tx_meta.tx_cfg.eirp);
    configure_syncword_class(current_packet->tx_meta.tx_cfg.syncword_class);

    cc1101_interface_write_burst_reg(TXFIFO, packet->data, packet->length + 1);
    cc1101_interface_set_interrupts_enabled(true);
    cc1101_interface_strobe(RF_STX);
    return SUCCESS;
}

int16_t hw_radio_get_rssi()
{
    return convert_rssi(cc1101_interface_read_single_reg(RSSI));
}

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

#include "cc1101.h"
#include "cc1101_interface.h"
#include "cc1101_constants.h"
#include "cc1101_registers.h"

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
   RADIO_FREQ2(RADIO_FREQ_433_NORMAL_RATE),                                        // FREQ2     Frequency control word, high byte.
   RADIO_FREQ1(RADIO_FREQ_433_NORMAL_RATE),                                        // FREQ1     Frequency control word, middle byte.
   RADIO_FREQ0(RADIO_FREQ_433_NORMAL_RATE),                                        // FREQ0     Frequency control word, low byte.
   RADIO_MDMCFG4_NORMAL_RATE,
   RADIO_MDMCFG3_DRATE_M_NORMAL_RATE,   	// MDMCFG3   Modem configuration.
   RADIO_MDMCFG2_DEM_DCFILT_ON | RADIO_MDMCFG2_MOD_FORMAT_GFSK | RADIO_MDMCFG2_SYNC_MODE_16in16CS,   // MDMCFG2   Modem configuration.
   RADIO_MDMCFG1_NUM_PREAMBLE_4B | RADIO_MDMCFG1_CHANSPC_E_NORMAL_RATE,   // MDMCFG1   Modem configuration.
   RADIO_MDMCFG0_CHANSPC_M_NORMAL_RATE,   	// MDMCFG0   Modem configuration.
   RADIO_DEVIATN_NORMAL_RATE,   // DEVIATN   Modem deviation setting (when FSK modulation is enabled).
   RADIO_MCSM2_RX_TIME(7),			// MCSM2		 Main Radio Control State Machine configuration.
   RADIO_MCSM1_CCA_ALWAYS | RADIO_MCSM1_RXOFF_MODE_RX | RADIO_MCSM1_TXOFF_MODE_IDLE,	// MCSM1 Main Radio Control State Machine configuration.
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
   RADIO_FSCAL2_FSCAL2(10) | RADIO_FSCAL2_VCO_CORE_H_EN,   		// FSCAL2    Frequency synthesizer calibration.
   RADIO_FSCAL1(0),   				// FSCAL1    Frequency synthesizer calibration.
   RADIO_FSCAL0(31)   				// FSCAL0    Frequency synthesizer calibration.
};

static void switch_to_idle_mode()
{
    DPRINT("Switching to HW_RADIO_STATE_IDLE");
    //Flush FIFOs and go to sleep, ensure interrupts are disabled
    cc1101_interface_set_interrupts_enabled(false);
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
    int16_t rssi = (int16_t)rssi_raw;		//Convert to signed 16 bit
    rssi += 128;                      		//Make it positive...
    rssi >>= 1;                        		//...So division to 1 dBm units can be a shift...
    rssi -= 64 + RSSI_OFFSET;     			// ...and then rescale it, including offset

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

static void end_of_packet_isr()
{
    DPRINT("end of packet ISR");
    switch(current_state)
    {
        case HW_RADIO_STATE_RX: ;
            uint8_t packet_len = cc1101_interface_read_single_reg(RXFIFO);
            DPRINT("EOP ISR packetLength: %d", packet_len);
            if(packet_len >= 63)
            {
            	// long packets not yet supported or bit error in length byte, don't assert but flush rx
                DPRINT("Packet size too big, flushing RX");
                uint8_t status = (cc1101_interface_strobe(RF_SNOP) & 0xF0);
                if(status == 0x60)
                {
                    // RX overflow
                    cc1101_interface_strobe(RF_SFRX);
                }
                else if(status == 0x10)
                {
                    // still in RX, switch to idle first
                    cc1101_interface_strobe(RF_SIDLE);
                    cc1101_interface_strobe(RF_SFRX);
                }

                while(cc1101_interface_strobe(RF_SNOP) != 0x0F); // wait until in idle state
                cc1101_interface_strobe(RF_SRX);
                while(cc1101_interface_strobe(RF_SNOP) != 0x1F); // wait until in RX state
                cc1101_interface_set_interrupts_enabled(true);
                return;
            }

            hw_radio_packet_t* packet = alloc_packet_callback(packet_len);
            packet->length = packet_len;
            cc1101_interface_read_burst_reg(RXFIFO, packet->data + 1, packet->length);

            // fill rx_meta
            packet->rx_meta.rssi = convert_rssi(cc1101_interface_read_single_reg(RXFIFO));
            packet->rx_meta.lqi = cc1101_interface_read_single_reg(RXFIFO) & 0x7F;
            memcpy(&(packet->rx_meta.rx_cfg.channel_id), &current_channel_id, sizeof(channel_id_t));
            packet->rx_meta.crc_status = HW_CRC_UNAVAILABLE; // TODO
            packet->rx_meta.timestamp = timer_get_counter_value();

            DPRINT_PACKET(packet, false);

            DEBUG_RX_END();
            if(rx_packet_callback != NULL) // TODO this can happen while doing CCA but we should not be interrupting here (disable packet handler?)
                rx_packet_callback(packet);
            else
                release_packet_callback(packet);

            if(current_state == HW_RADIO_STATE_RX) // check still in RX, could be modified by upper layer while in callback
            {
                uint8_t status = (cc1101_interface_strobe(RF_SNOP) & 0xF0);
                if(status == 0x60) // RX overflow
                {
                    cc1101_interface_strobe(RF_SFRX);
                    while(cc1101_interface_strobe(RF_SNOP) != 0x0F); // wait until in idle state
                    cc1101_interface_strobe(RF_SRX);
                    while(cc1101_interface_strobe(RF_SNOP) != 0x1F); // wait until in RX state
                }

                cc1101_interface_set_interrupts_enabled(true);
                assert(cc1101_interface_strobe(RF_SNOP) == 0x1F); // expect to be in RX mode
            }
            break;
        case HW_RADIO_STATE_TX:
          DEBUG_TX_END();

          // going to IDLE is the default unless we where in RX when starting transmission or we were put in RX during
          // transmission. We can also be forced to go back to IDLE after starting TX from RX state (for instance during CCA)
          if(!should_rx_after_tx_completed)
          {
            switch_to_idle_mode();
          }

          current_packet->tx_meta.timestamp = timer_get_counter_value();

          DPRINT_PACKET(current_packet, true);

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

static inline void wait_for_chip_state(cc1101_chipstate_t expected_state)
{
    uint8_t chipstate;
    do {
        chipstate = cc1101_interface_read_single_reg(MARCSTATE);
    } while (chipstate != expected_state);
}

static void configure_channel(const channel_id_t* channel_id)
{
    // only change settings if channel_id changed compared to current config
    if(!hw_radio_channel_ids_equal(channel_id, &current_channel_id))
    {
        assert(channel_id->channel_header.ch_coding == PHY_CODING_PN9); // TODO implement other codings
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
//    	status = cc1101_interface_strobe(RF_SNOP) & 0x80;
//    }

    configure_channel(&(rx_cfg->channel_id));
    configure_syncword_class(rx_cfg->syncword_class);
    cc1101_interface_write_single_reg(PKTLEN, 0xFF);

    // cc1101_interface_strobe(RF_SFRX); TODO only when in idle or overflow state

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
      cc1101_interface_set_interrupts_enabled(true);
    else
      cc1101_interface_set_interrupts_enabled(false);

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

error_t hw_radio_send_packet(hw_radio_packet_t* packet, tx_packet_callback_t tx_cb)
{
    // TODO error handling EINVAL, ESIZE, EOFF
    if(current_state == HW_RADIO_STATE_TX)
        return EBUSY;

    assert(packet->length < 63); // long packets not yet supported

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

    DPRINT("Data to TX Fifo:");
    DPRINT_DATA(packet->data, packet->length + 1);
    configure_channel((channel_id_t*)&(current_packet->tx_meta.tx_cfg.channel_id));
    configure_eirp(current_packet->tx_meta.tx_cfg.eirp);
    configure_syncword_class(current_packet->tx_meta.tx_cfg.syncword_class);

    cc1101_interface_write_burst_reg(TXFIFO, packet->data, packet->length + 1);
    cc1101_interface_set_interrupts_enabled(true);
    DEBUG_TX_START();
    DEBUG_RX_END();
    cc1101_interface_strobe(RF_STX);

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
        return SUCCESS;
    }

    switch_to_idle_mode();
    return SUCCESS;
}

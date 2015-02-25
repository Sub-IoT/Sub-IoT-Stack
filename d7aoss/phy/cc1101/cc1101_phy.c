/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 *     	glenn.ergeerts@uantwerpen.be
 *     	maarten.weyn@uantwerpen.be
 *		alexanderhoet@gmail.com
 *		armin@otheruse.nl
 *
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "phy.h"
#include "cc1101_phy.h"
#include "cc1101_registers.h"
#include "cc1101_constants.h"

#include "cc1101_interface.h"
#include "radio_hw.h"
#include "spi.h"
#include "log.h"

#ifdef LOG_PHY_ENABLED
#define DPRINT(...) log_print_stack_string(LOG_PHY, __VA_ARGS__)
#else
#define DPRINT(...)
#endif

extern RF_SETTINGS rfSettings;

static RadioState state;
static uint8_t buffer[255];
static uint8_t packet_length;

static bool fec_enabled;
static uint8_t frequency_band;
static uint8_t channel_center_freq_index;
static uint8_t channel_bandwidth_index;
static uint8_t preamble_size;
static uint16_t sync_word;

static bool init_and_close_radio = true;
static bool phy_initialized = false;

static uint8_t previous_spectrum_id[2] = {0xFF, 0xFF};
static uint8_t previous_sync_word_class = 0xFF;

phy_rx_data_t rx_data;

phy_tx_cfg_t last_tx_cfg;


//for this moment both int8, should be changed to float or double values.
static int8_t eirp_values[103] = {9.9, 9.5, 9.2, 8.8, 8.5, 8.1, 7.8, 7.4, 7.1, 6.8, 6.4, 6.3, 6.1, 6.0, 5.8, 5.5, 5.1, 4.9, 4.8, 4.4, //20
                                  4.0, 3.6, 3.2, 2.8, 2.3, 2.0, 1.9, 1.4, 0.4, 0.1, -0.3, -0.5, -0.8, -0.9, -1.1, -1.4, -1.5, -1.7, -2.1, //40
                                  -2.2, -2.3, -2.5, -2.8, -2.9, -3.0, -3.1, -3.3, -3.5, -3.8, -4.0, -4.1, -4.6, -4.7, -5.3, -5.6, -5.9, //56
                                  -6.0, -6.5, -6.8, -6.9, -7.1, -7.7, -7.8, -8.3, -8.7, -8.9, -9.9, -10.1, -11.4, -12.3, -13.3, -13.7, //72
                                  -14.3, -14.9, -15.5, -15.6, -15.7, -16.2, -17.0, -17.8, -18.8, -19.0, -19.3,-19.8, -20.4, -21.0, -21.3, //87
                                  -21.7, -22.5, -23.3, -24.3, -24.5, -25.3, -26.5, -27.9, -29.5, -29.6, -31.4, -33.8, -36.5,-38.3, -38.4, -62.7}; //103

static int8_t eirp_reg_values[103]={0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0x80, 0xCB, 0x81, 0xCC, 0xCD, 0x84, 0xCE, 0x85, 0x86, //20
                                    0x87, 0x88, 0x89, 0x8A, 0x8B, 0xCF,0x8C, 0x8D, 0x8E, 0x60, 0x51, 0x61, 0x40, 0x52, 0x3, 0x3E, 0x53, 0x3D, 0x3C, 0x54, //40
                                    0x64, 0x3B, 0x55, 0x65, 0x2F, 0x3A, 0x56, 0x66, 0x39, 0x57, 0x67, 0x8F, 0x2C, 0x2B, 0x37, 0x6A, 0x2A, 0x6B, 0x36, 0x29,//60
                                    0x6C, 0x6D, 0x28, 0x35, 0x27, 0x6E, 0x26, 0x34, 0x25, 0x33, 0x24, 0x1F, 0x1E, 0x1D, 0x6F, 0x23, 0x32, 0x1B, 0x1A, 0x19,//80
                                    0x18, 0x22, 0xF, 0x17, 0xD, 0xC, 0x31, 0xB, 0x15, 0x9, 0x14, 0x21, 0x7, 0x13, 0x5, 0x4, 0x12, 0x3, 0x11, 0x1, 0x10, 0x30, 0x0}; //103

static RadioState get_radiostate(void)
{
    uint8_t state = Strobe(RF_SNOP) >> 4;

    if(state > 7)
        return Idle;

    return (RadioState)state;
}

static int16_t calculate_rssi(int8_t rssi_raw)
{
    // CC1101 RSSI is 0.5 dBm units, signed byte
    int16_t rssi = (int16_t)rssi_raw;		//Convert to signed 16 bit
    rssi += 128;                      		//Make it positive...
    rssi >>= 1;                        		//...So division to 1 dBm units can be a shift...
    rssi -= 64 + RSSI_OFFSET;     			// ...and then rescale it, including offset

    return rssi;
}

static void set_preamble_size(uint8_t preamble_size)
{
    uint8_t mdmcfg1 = ReadSingleReg(MDMCFG1) & 0x03;

    if(preamble_size >= 24)
        mdmcfg1 |= 0x70;
    else if(preamble_size >= 16)
        mdmcfg1 |= 0x60;
    else if(preamble_size >= 12)
        mdmcfg1 |= 0x50;
    else if(preamble_size >= 8)
        mdmcfg1 |= 0x40;
    else if(preamble_size >= 6)
        mdmcfg1 |= 0x30;
    else if(preamble_size >= 4)
        mdmcfg1 |= 0x20;
    else if(preamble_size >= 3)
        mdmcfg1 |= 0x10;

    WriteSingleReg(MDMCFG1, mdmcfg1);
}

static void set_timeout(uint16_t timeout)
{
    if (timeout == 0)
    {
        WriteSingleReg(WORCTRL, RADIO_WORCTRL_ALCK_PD);
        WriteSingleReg(MCSM2, RADIO_MCSM2_RX_TIME(7));
    }
    else
    {
        WriteSingleReg(WORCTRL, RADIO_WORCTRL_WOR_RES_32ms);
        WriteSingleReg(MCSM2, RADIO_MCSM2_RX_TIME(0));
        WriteSingleReg(WOREVT0, timeout & 0x00FF);
        WriteSingleReg(WOREVT1, timeout >> 8);
    }
}

static void set_eirp(int8_t eirp)
{
    // TODO compare with CC430 implementation
    uint8_t i;
    for( i = 0; i < sizeof(eirp_values); i++ )
    {
        if (eirp >= eirp_values[i]) //round the given eirp to a lower possible value
        {
            WriteSinglePATable(eirp_reg_values[i]);
            break;
        }
    }
}

static void set_sync_word(uint16_t sync_word)
{
    WriteSingleReg(SYNC1, (uint8_t)(sync_word >> 8));
    WriteSingleReg(SYNC0, (uint8_t)(sync_word & 0x00FF));
}

static void set_channel(uint8_t frequency_band, uint8_t channel_center_freq_index, uint8_t channel_bandwith_index)
{
    //Set channel center frequency
    DPRINT("Set channel freq index: %d", channel_center_freq_index);

    WriteSingleReg(CHANNR, channel_center_freq_index);

    //Set channel bandwidth, modulation and symbol rate
    DPRINT("Set channel bandwidth index: %d", channel_bandwith_index);

    switch(channel_bandwith_index)
    {
    case 0:
        // TODO CC430:
        //		WriteSingleReg(MDMCFG3, RADIO_MDMCFG3_DRATE_M(131));
        //		WriteSingleReg(MDMCFG4, (RADIO_MDMCFG4_CHANBW_E(3) | RADIO_MDMCFG4_CHANBW_M(0) | RADIO_MDMCFG4_DRATE_E(8)));
        //		WriteSingleReg(DEVIATN, (RADIO_DEVIATN_E(0) | RADIO_DEVIATN_M(16)));
        WriteSingleReg(MDMCFG3, RADIO_MDMCFG3_DRATE_M(24));
        WriteSingleReg(MDMCFG4, (RADIO_MDMCFG4_CHANBW_E(1) | RADIO_MDMCFG4_CHANBW_M(0) | RADIO_MDMCFG4_DRATE_E(11)));
        WriteSingleReg(DEVIATN, (RADIO_DEVIATN_E(5) | RADIO_DEVIATN_M(0)));
        break;
    case 1:
        WriteSingleReg(MDMCFG3, RADIO_MDMCFG3_DRATE_M(24));
        WriteSingleReg(MDMCFG4, (RADIO_MDMCFG4_CHANBW_E(2) | RADIO_MDMCFG4_CHANBW_M(0) | RADIO_MDMCFG4_DRATE_E(11)));
        //WriteSingleReg(MDMCFG4, (RADIO_MDMCFG4_CHANBW_E(1) | RADIO_MDMCFG4_CHANBW_M(0) | RADIO_MDMCFG4_DRATE_E(11))); // TODO tmp 400kHz BW
        WriteSingleReg(DEVIATN, (RADIO_DEVIATN_E(5) | RADIO_DEVIATN_M(0)));
        break;
    case 2:
        WriteSingleReg(MDMCFG3, RADIO_MDMCFG3_DRATE_M(248));
        WriteSingleReg(MDMCFG4, (RADIO_MDMCFG4_CHANBW_E(1) | RADIO_MDMCFG4_CHANBW_M(0) | RADIO_MDMCFG4_DRATE_E(12)));
        WriteSingleReg(DEVIATN, (RADIO_DEVIATN_E(5) | RADIO_DEVIATN_M(0)));
        break;
        //	case 3:
        //		WriteSingleReg(MDMCFG3, RADIO_MDMCFG3_DRATE_M(248));
        //		WriteSingleReg(MDMCFG4, (RADIO_MDMCFG4_CHANBW_E(0) | RADIO_MDMCFG4_CHANBW_M(1) | RADIO_MDMCFG4_DRATE_E(12)));
        //		WriteSingleReg(DEVIATN, (RADIO_DEVIATN_E(5) | RADIO_DEVIATN_M(0)));
        //		break;
    }

    DPRINT("Set frequency band: %d", frequency_band);

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

    // is this the right place?
    Strobe(RF_SCAL);
}

static bool translate_and_set_settings(uint8_t spectrum_id[2], uint8_t sync_word_class)
{
    if (!memcmp(previous_spectrum_id, spectrum_id, 2) && previous_sync_word_class == sync_word_class)
        return true;

    Strobe(RF_SIDLE);

    if(!phy_translate_settings(spectrum_id, sync_word_class, &fec_enabled, &frequency_band, &channel_center_freq_index, &channel_bandwidth_index, &preamble_size, &sync_word))
    {
        DPRINT("PHY Cannot translate settings");
        return false;
    }

    set_channel(frequency_band, channel_center_freq_index, channel_bandwidth_index); // TODO freq band
    set_preamble_size(preamble_size);
    set_sync_word(sync_word);

    memcpy(previous_spectrum_id, spectrum_id, 2);
    previous_sync_word_class = sync_word_class;

    return true;
}

static bool set_tx_config(phy_tx_cfg_t* cfg)
{
    translate_and_set_settings(cfg->spectrum_id, cfg->sync_word_class);
    set_eirp(cfg->eirp);

    //TODO Return error if fec not enabled but requested
    return true;
}

static void set_length_infinite(bool infinite)
{
    uint8_t pktctrl0 = ReadSingleReg(PKTCTRL0) & 0xFC;

    if (infinite)
        pktctrl0 |= RADIO_PKTCTRL0_LENGTH_INF;

    WriteSingleReg(PKTCTRL0, pktctrl0);
}

void phy_init(void)
{
    state = Idle;
    cc1101_interface_init();

    ResetRadioCore();

    WriteRfSettings(&rfSettings);

#ifdef LOG_PHY_ENABLED
    DPRINT("RF settings:");
    uint8_t* p = (uint8_t*) &rfSettings;
    for(uint8_t i = 0; i < sizeof(RF_SETTINGS); i++)
    {
        DPRINT("\t0x%02X", p[i]);
    }
#endif

    last_tx_cfg.eirp = 0;
    last_tx_cfg.spectrum_id[0] = 0;
    last_tx_cfg.spectrum_id[1] = 0;
    last_tx_cfg.sync_word_class = 0;
}

void phy_idle(void)
{
    if (state != Idle)
    {
        rxtx_finish_isr();
        state = Idle;
    }
}

extern bool phy_tx_data(phy_tx_cfg_t* cfg)
{
    //TODO Return error if fec not enabled but requested

    set_length_infinite(false);

#ifdef LOG_PHY_ENABLED
    log_print_stack_string(LOG_PHY, "Data to TX Fifo:");
    log_print_data(tx_queue.front, cfg->length);
#endif

    WriteSingleReg(TXFIFO, cfg->length);
    WriteBurstReg(TXFIFO, tx_queue.front, cfg->length);

    cc1101_interface_set_interrupts_enabled(true);

    //Start transmitting
    Strobe(RF_STX);

    return true;
}

bool phy_init_tx()
{
    RadioState current_state = get_radiostate();
    if (current_state != Idle && current_state != Transmit)
    {
        DPRINT("PHY radio not idle");

        return false;
    }

    state = Transmit;

    Strobe(RF_SIDLE);
    Strobe(RF_SFTX);

    return true;
}

bool phy_tx(phy_tx_cfg_t* cfg)
{
    if (init_and_close_radio || !phy_initialized)
    {
        if (!phy_init_tx())
        {
            return false;
        }

        //if (last_tx_cfg.spectrum_id != cfg->spectrum_id || last_tx_cfg.sync_word_class != cfg->sync_word_class || last_tx_cfg.eirp != cfg->eirp)
        if (memcmp(&last_tx_cfg, cfg, 3) != 0)
        {
            if (!set_tx_config(cfg))
                return false;

            memcpy(&last_tx_cfg, cfg, 3);
        }
    }

    return phy_tx_data(cfg);
}

bool phy_rx(phy_rx_cfg_t* cfg)
{
    DPRINT("phy_rx");

    RadioState current_state = get_radiostate();
    if(current_state != Idle && current_state != Receive)
    {
        DPRINT("PHY Cannot RX, PHY not idle");

        return false;
    }

    state = Receive;

    Strobe(RF_SIDLE);
    Strobe(RF_SFRX);

    if (!translate_and_set_settings(cfg->spectrum_id, cfg->sync_word_class))
        return false;

    rx_data.spectrum_id[0] = cfg->spectrum_id[0];
    rx_data.spectrum_id[1] = cfg->spectrum_id[1];
    rx_data.sync_word_class = cfg->sync_word_class;

    set_timeout(cfg->timeout);

    //TODO Return error if fec not enabled but requested

    queue_clear(&rx_queue);

    set_length_infinite(false); // TODO needed?

    // TODO remove cfg->length, length is contained in background frames as well in draft spec so always dynamic

    packet_length = 0;
    WriteSingleReg(PKTLEN, 0xFF);

    //TODO: set minimum sync word rss to scan minimum energy

    cc1101_interface_set_interrupts_enabled(true);

    Strobe(RF_SRX);

    return true;
}

bool phy_is_rx_in_progress(void)
{
    return (state == Receive);
}

bool phy_is_tx_in_progress(void)
{
    return (state == Transmit);
}

extern int16_t phy_get_rssi(uint8_t spectrum_id[2], uint8_t sync_word_class)
{
    uint8_t rssi_raw = 0;

    if (!translate_and_set_settings(spectrum_id, sync_word_class))
        return false;

    Strobe(RF_SRX);

    //FIXME wait for RSSI VALID!!!
    // Is this possible with CC1101?

    rssi_raw = ReadSingleReg(RSSI);
    rxtx_finish_isr();

    return calculate_rssi(rssi_raw);
}

void end_of_packet_isr()
{
    DPRINT("end of packet ISR");
    if (state == Receive)
    {
        packet_length = ReadSingleReg(RXFIFO);
        DPRINT("EOP ISR packetLength: %d", packet_length);
        ReadBurstReg(RXFIFO, buffer, packet_length + 2); // +2 for RSSI and LQI
        rxtx_finish_isr(); // TODO: should this be called by DLL?
        queue_push_u8(&rx_queue, packet_length); // TODO do not put length in buffer only in rx_data->len ?
        queue_push_u8_array(&rx_queue, buffer, packet_length);
        rx_data.rssi = calculate_rssi(buffer[packet_length]);
        rx_data.lqi = buffer[packet_length + 1] & 0x7F;
        rx_data.length = packet_length;
        rx_data.data = rx_queue.front;
        if(phy_rx_callback != NULL)
            phy_rx_callback(&rx_data);    
    }
    else if (state == Transmit)
    {
        rxtx_finish_isr();
        if(phy_tx_callback != NULL)
            phy_tx_callback();
    }
    else
    {
        rxtx_finish_isr();
    }
}


//void rx_data_isr()
//{
//
//	//Read number of bytes in RXFIFO
//	uint8_t rxBytes = ReadSingleReg(RXBYTES);
//#ifdef LOG_PHY_ENABLED
//		log_print_stack_string(LOG_PHY, "rx_data_isr (%d bytes received)", rxBytes);
//#endif
//
//    //If length is not set get the length from RXFIFO and set PKTLEN
//	if (packetLength == 0) {
//		packetLength = ReadSingleReg(RXFIFO);
//		WriteSingleReg(PKTLEN, packetLength);
//		WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_17_48);
//		remainingBytes = packetLength - 1;
//		queue_push_u8(&rx_queue, packetLength); // TODO do not put length in buffer only in rx_data->len ?
//		rxBytes--;
//#ifdef LOG_PHY_ENABLED
////		log_print_stack_string(LOG_PHY, "rx_data_isr getting packetLength (%d)", packetLength);
//	#endif
//	}
//
//	//Never read the entire buffer as long as more data is going to be received
//    if (remainingBytes > rxBytes)
//    {
//    	rxBytes--;
//    }
//    else
//    {
//    	rxBytes = remainingBytes; // we may have received more bytes than packetlength already (probably noise) so limit to packetlength
//    }
//
//    //Read data from buffer
//	#ifdef LOG_PHY_ENABLED
////		log_print_stack_string(LOG_PHY, "Getting %d bytes from RXFifo", rxBytes);
//	#endif
//	ReadBurstReg(RXFIFO, &rx_queue.rear[1], rxBytes);
//
//	rx_queue.length += rxBytes;
//	rx_queue.rear += rxBytes;
//
//	remainingBytes -= rxBytes;
//	#ifdef LOG_PHY_ENABLED
////		log_print_stack_string(LOG_PHY, "Received data:");
////		log_print_data(rx_queue.front, rxBytes);
//	#endif
//#ifdef LOG_PHY_ENABLED
////	log_print_stack_string(LOG_PHY, "%d bytes remaining", remainingBytes);
//#endif
//
//    //When all data has been received read rssi and lqi value and set packetreceived flag
//	// TODO do in end_of_packet_isr()?
//    if(remainingBytes <= 0)
//    {
//    	rx_data.rssi = calculate_rssi(ReadSingleReg(RXFIFO));
//    	rx_data.lqi = ReadSingleReg(RXFIFO) & 0x7F;
//		rx_data.length = *rx_queue.front;
//		rx_data.data = rx_queue.front;
//		#ifdef LOG_PHY_ENABLED
////			log_print_stack_string(LOG_PHY, "rx_data_isr packet received");
////			log_print_data(rx_data.data, rx_data.length);
////			log_phy_rx_res(&rx_data);
//		#endif
//    }
//
//	#ifdef LOG_PHY_ENABLED
////		log_print_stack_string(LOG_PHY, "rx_data_isr 1");
//	#endif
//}

void rx_timeout_isr()
{
    rxtx_finish_isr();
    if(phy_rx_callback != NULL)
        phy_rx_callback(NULL);
}

void rx_fifo_overflow_isr()
{
    DPRINT("rx_fifo_overflow");

    rxtx_finish_isr();
    if(phy_rx_callback != NULL)
        phy_rx_callback(NULL);
}

void rxtx_finish_isr()
{
    cc1101_interface_set_interrupts_enabled(false);

    if (init_and_close_radio)
    {
        //Flush FIFOs and go to sleep
        Strobe(RF_SIDLE);
        Strobe(RF_SFRX);
        Strobe(RF_SFTX);
        Strobe(RF_SPWD);

        state = Idle;
    }
}


//void dissable_autocalibration()
//{
//	WriteSingleReg(MCSM0, RADIO_MCSM0_FS_AUTOCAL_NEVER);
//}
//
//void enable_autocalibration()
//{
//	WriteSingleReg(MCSM0, RADIO_MCSM0_FS_AUTOCAL_4THIDLE);
//}
//
//void manual_calibration()
//{
//	Strobe(RF_SCAL);
//}

void phy_keep_radio_on(bool status)
{
    init_and_close_radio = !status;
    // TODO compare CC430
}

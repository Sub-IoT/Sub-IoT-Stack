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
 * Driver for si4460
 *
 * @author maarten.weyn@uantwerpen.be
 *
 * Currently implemented for 26 MHZ RF Xtal
 * TODO: SYNTH_LPFILTx is dependent on hardware define in platform
 *
 * TODO: PKT_RX_THRESHOLD -> IRQ when Rx fifo almost full
 * TODO: PKT_TX_THRESHOLD
 */

#include "debug.h"
#include "string.h"

#include "log.h"
#include "hwradio.h"
#include "hwsystem.h"
#include "hwdebug.h"
#include "types.h"

#include "si4460.h"
#include "si4460_interface.h"
#include "si4460_registers.h"
#include "ezradio_cmd.h"
#include "ezradio_api_lib.h"

#include "ezradio_hal.h"
#include "fec.h"


#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_PHY_LOG_ENABLED) // TODO more granular (LOG_PHY_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_PHY, __VA_ARGS__)
#define DPRINT_PACKET(...) log_print_raw_phy_packet(__VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_PACKET(...)
#define DPRINT_DATA(...)
#endif

#define RSSI_OFFSET 64 // if this is changed also change radio register 0x20,0x4e

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

#ifdef HAL_RADIO_USE_HW_CRC
static bool has_hardware_crc = true;
#else
static bool has_hardware_crc = false;
#endif

/** \brief The possible states the radio can be in
 */
typedef enum
{
    HW_RADIO_STATE_IDLE,
    HW_RADIO_STATE_TX,
    HW_RADIO_STATE_RX,
    HW_RADIO_STATE_OFF,
    HW_RADIO_STATE_UNKOWN
} hw_radio_state_t;



static alloc_packet_callback_t alloc_packet_callback;
static release_packet_callback_t release_packet_callback;
static rx_packet_callback_t rx_packet_callback;
static tx_packet_callback_t tx_packet_callback;
static rssi_valid_callback_t rssi_valid_callback;
static hw_radio_state_t current_state;
static hw_radio_packet_t* current_packet;
static channel_id_t current_channel_id = {
    .channel_header_raw = 0xFF,
    .center_freq_index = 0xFF
};

static uint8_t ez_channel_id = 0;
static hw_radio_packet_t* rx_packet;

static eirp_t current_eirp = 0;

static bool should_rx_after_tx_completed = false;
static uint16_t tx_fifo_data_length = 0;
static uint16_t rx_fifo_data_lenght = 0;
static uint16_t expected_data_length = 0;

static hw_rx_cfg_t current_rx_cfg = {0x0000, PHY_SYNCWORD_CLASS0};
static syncword_class_t current_syncword_class = PHY_SYNCWORD_CLASS0;

static inline int16_t convert_rssi(uint8_t rssi_raw);
static void start_rx(hw_rx_cfg_t const* rx_cfg);
static void ezradio_int_callback();
static void report_rssi();

static void configure_channel(const channel_id_t* channel_id)
{

	// only change settings if channel_id changed compared to current config
	// TODO: check if only channel number changes

	if((channel_id->channel_header_raw != current_channel_id.channel_header_raw))
	{
		DPRINT("configure_channel: %s", byte_to_binary(channel_id->channel_header_raw));
		if (channel_id->channel_header.ch_coding != current_channel_id.channel_header.ch_coding)
		{
			if (has_hardware_crc && channel_id->channel_header.ch_coding != PHY_CODING_FEC_PN9)
			{
				// use HW CRC
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_PKT_LEN_ADJUST_HW_CRC);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_PKT_FIELD_1_CRC_CONFIG_HW_CRC);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_PKT_FIELD_2_CRC_CONFIG_HW_CRC);
			} else {
				// use SW CRC
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_PKT_LEN_ADJUST_SW_CRC);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_PKT_FIELD_1_CRC_CONFIG_SW_CRC);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_PKT_FIELD_2_CRC_CONFIG_SW_CRC);
			}
		}
		// TODO assert valid center freq index

		memcpy(&current_channel_id, channel_id, sizeof(channel_id_t)); // cache new settings

		// TODO preamble size depends on channel class

		// set freq band
		DPRINT("Set frequency band index: %d", channel_id->channel_header.ch_freq_band);

		switch(channel_id->channel_header.ch_class)
		{
		case PHY_CLASS_LO_RATE:
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNTH_PFDCP_CPFF_LR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNTH_PFDCP_CPINT_LR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT3_LR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT2_LR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT1_LR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_DATA_RATE_LR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_TX_NCO_MODE_LR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_PA_TC_LR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_WAIT_LR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_SPIKE_DET_LR);

			break;
		case PHY_CLASS_NORMAL_RATE:
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNTH_PFDCP_CPFF_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNTH_PFDCP_CPINT_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT3_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT2_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT1_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_DATA_RATE_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_TX_NCO_MODE_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_PA_TC_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_WAIT_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_SPIKE_DET_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_DECIMATION_CFG1_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_OSR_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_GAIN_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_LIMITER_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AGC_RFPD_DECAY_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AGC_IFPD_DECAY_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_RAW_EYE_NR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_DSA_QUAL_NR);

			break;
		case PHY_CLASS_HI_RATE:
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNTH_PFDCP_CPFF_HR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNTH_PFDCP_CPINT_HR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT3_HR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT2_HR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT1_HR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_TX_NCO_MODE_HR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_PA_TC_HR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_WAIT_HR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_SPIKE_DET_HR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_DECIMATION_CFG1_HR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_OSR_HR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_GAIN_HR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_LIMITER_HR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AGC_RFPD_DECAY_HR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AGC_IFPD_DECAY_HR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_RAW_EYE_HR);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_DSA_QUAL_HR);
			//assert(false);
			break;
		default:
			assert(false);
			break;
		}

		// TODO validate
		switch(channel_id->channel_header.ch_freq_band)
		{
		// TODO calculate depending on rate and channr
		case PHY_BAND_433:
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CLKGEN_BAND_433);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_VCOCNT_RX_ADJ_433);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_IF_FREQ_433);

			if(channel_id->channel_header.ch_class == PHY_CLASS_LO_RATE)
			{
				assert(channel_id->center_freq_index <= 68);
				ez_channel_id = channel_id->center_freq_index;
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_FREQ_DEV_433_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_FRAC_433_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_DECIMATION_CFG1_433_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_OSR_433_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_NCO_OFFSET_433_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_GAIN_433_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_GAIN_433_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_LIMITER_433_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AGC_RFPD_DECAY_433_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AGC_IFPD_DECAY_433_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_RAW_EYE_433_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_0_433_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_1_433_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_0_433_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_1_433_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_DSA_QUAL_433_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_CHANNEL_STEP_SIZE_433_LR);

			}
			else if(channel_id->channel_header.ch_class == PHY_CLASS_NORMAL_RATE)
			{
				assert(channel_id->center_freq_index % 8 == 0 && channel_id->center_freq_index <= 270);
				ez_channel_id = channel_id->center_freq_index / 8;

				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_FREQ_DEV_433_NR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_FRAC_433_NR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_NCO_OFFSET_433_NR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_GAIN_433_NR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_0_433_NR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_1_433_NR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_0_433_NR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_1_433_NR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_CHANNEL_STEP_SIZE_433_NR);
			}
			else if(channel_id->channel_header.ch_class == PHY_CLASS_HI_RATE)
			{
				assert(channel_id->center_freq_index % 8 == 0 && channel_id->center_freq_index <= 56);
				ez_channel_id = channel_id->center_freq_index / 8;
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_DATA_RATE_433_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_FREQ_DEV_433_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_FRAC_433_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_NCO_OFFSET_433_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_GAIN_433_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_0_433_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_1_433_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_0_433_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_1_433_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_CHANNEL_STEP_SIZE_433_HR);
			}

			DPRINT("Set channel freq index: %d", channel_id->center_freq_index);
			break;
		case PHY_BAND_868:
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CLKGEN_BAND_868);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_VCOCNT_RX_ADJ_868);
			ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_IF_FREQ_868);
			if(channel_id->channel_header.ch_class == PHY_CLASS_LO_RATE)
			{
				assert(channel_id->center_freq_index <= 279);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_FREQ_DEV_868_LR);

				ez_channel_id = channel_id->center_freq_index % 255;
				if ((uint8_t) (channel_id->center_freq_index / 255) == 0)
				{
					ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_FRAC_868_LR_01);
				} else
				{
					ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_FRAC_868_LR_02);
				}

				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_DECIMATION_CFG1_868_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_OSR_868_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_NCO_OFFSET_868_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_GAIN_868_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_GAIN_868_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_LIMITER_868_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AGC_RFPD_DECAY_868_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AGC_IFPD_DECAY_868_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_RAW_EYE_868_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_0_868_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_1_868_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_0_868_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_1_868_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_DSA_QUAL_868_LR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_CHANNEL_STEP_SIZE_868_LR);
			}
			else if(channel_id->channel_header.ch_class == PHY_CLASS_NORMAL_RATE)
			{
				assert(channel_id->center_freq_index % 8 == 0 && channel_id->center_freq_index <= 270);
				ez_channel_id = channel_id->center_freq_index / 8;

				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_FREQ_DEV_868_NR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_FRAC_868_NR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_NCO_OFFSET_868_NR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_GAIN_868_NR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_0_868_NR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_1_868_NR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_0_868_NR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_1_868_NR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_CHANNEL_STEP_SIZE_868_NR);
			} else {
				assert(channel_id->center_freq_index % 8 == 0 && channel_id->center_freq_index <= 270);
				ez_channel_id = channel_id->center_freq_index / 8;

				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_DATA_RATE_868_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_FREQ_DEV_868_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_FRAC_868_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_NCO_OFFSET_868_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_GAIN_868_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_0_868_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_1_868_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_0_868_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_1_868_HR);
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_CHANNEL_STEP_SIZE_868_HR);
			}

			DPRINT("Set channel freq index: %d", channel_id->center_freq_index);
			break;
		case PHY_BAND_915:
			assert(false);

			break;

		}
	} else 	if (channel_id->center_freq_index != current_channel_id.center_freq_index)
	{
		DPRINT("configure center_freq_index: %d", channel_id->center_freq_index);
		switch(channel_id->channel_header.ch_freq_band)
		{
		// TODO calculate depending on rate and channr
		case PHY_BAND_433:
			if(channel_id->channel_header.ch_class == PHY_CLASS_LO_RATE)
			{
				assert(channel_id->center_freq_index <= 68);
				ez_channel_id = channel_id->center_freq_index;

			}
			else
			{
				assert(channel_id->center_freq_index % 8 == 0 && channel_id->center_freq_index <= 56);
				ez_channel_id = channel_id->center_freq_index / 8;
			}

			DPRINT("Set channel freq index: %d", channel_id->center_freq_index);
			break;
		case PHY_BAND_868:
			if(channel_id->channel_header.ch_class == PHY_CLASS_LO_RATE)
			{
				assert(channel_id->center_freq_index <= 279);
				ez_channel_id = channel_id->center_freq_index % 255;
				if ((uint8_t) (channel_id->center_freq_index / 255) == 0)
				{
					ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_FRAC_868_LR_01);
				} else
				{
					ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_FRAC_868_LR_02);
				}

			}
			else  {
				assert(channel_id->center_freq_index % 8 == 0 && channel_id->center_freq_index <= 272);
				ez_channel_id = channel_id->center_freq_index / 8;
			}

			DPRINT("Set channel freq index: %d", channel_id->center_freq_index);
			break;
		case PHY_BAND_915:
			assert(false);
			break;

		}

		current_channel_id.center_freq_index = channel_id->center_freq_index;
	}
}


static void configure_eirp(const eirp_t eirp)
{
	if (eirp == current_eirp)
		return;


	DPRINT("configure_eirp not implemented using max");
	uint8_t ddac = 0x7F; // max
	ezradio_set_property(0x22, 0x01, 0x01, ddac);
	current_eirp = eirp;
}

static void configure_syncword_class(syncword_class_t syncword_class, phy_coding_t coding)
{
	if(syncword_class == current_syncword_class)
		return;

	current_syncword_class = syncword_class;
	switch (coding)
	{
		case PHY_CODING_PN9:
			switch (syncword_class)
			{
				case PHY_SYNCWORD_CLASS0:
					ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNC_BITS_CS0_0);
					break;
				case PHY_SYNCWORD_CLASS1:
					ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNC_BITS_CS0_1);
					break;
			}
			break;
		case PHY_CODING_FEC_PN9:
			switch (syncword_class)
			{
				case PHY_SYNCWORD_CLASS0:
					ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNC_BITS_CS1_0);
					break;
				case PHY_SYNCWORD_CLASS1:
					ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNC_BITS_CS1_1);
					break;
			}
			break;
	}
}

static void switch_to_idle_mode()
{
	if (current_state == HW_RADIO_STATE_IDLE)
		return;

	ezradio_change_state(EZRADIO_CMD_CHANGE_STATE_ARG_NEXT_STATE1_NEW_STATE_ENUM_SLEEP);
	current_state = HW_RADIO_STATE_IDLE;
}

error_t hw_radio_init(alloc_packet_callback_t alloc_packet_cb,
                      release_packet_callback_t release_packet_cb)
{
	/* EZRadio response structure union */
	ezradio_cmd_reply_t ezradioReply;

	alloc_packet_callback = alloc_packet_cb;
	release_packet_callback = release_packet_cb;

	current_state = HW_RADIO_STATE_UNKOWN;


	/* Initialize EZRadio device. */
	DPRINT("INIT ezradioInit");
	ezradioInit(&ezradio_int_callback);

	/* Print EZRadio device number. */
	DPRINT("INIT ezradio_part_info");
	ezradio_part_info(&ezradioReply);
	DPRINT("   Device: Si%04x\n\n", ezradioReply.PART_INFO.PART);

	/* Enable Packet Trace Interface */
	//ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_PTI_ENABLE);

	/* Fix registers not correct set by radio configurator */
	// latch on sync word detect - currently no threshold comparison (todo: optimize)
	//ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_RSSI_CONTROL);

	// disable jump detection (todo: optimize)
	//ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_MODEM_RSSI_CONTROL2);

	/* Reset radio fifos. */
	DPRINT("INIT ezradioResetTRxFifo");
	ezradioResetTRxFifo();



	// configure default channel, eirp and syncword
	configure_channel(&current_channel_id);
	configure_eirp(current_eirp);
	configure_syncword_class(current_rx_cfg.syncword_class, current_channel_id.channel_header.ch_coding);

	switch_to_idle_mode();
}

error_t hw_radio_set_rx(hw_rx_cfg_t const* rx_cfg, rx_packet_callback_t rx_cb, rssi_valid_callback_t rssi_valid_cb)
{
	DPRINT("hw_radio_set_rx rx_cb %p rssi_valid_cb %p", rx_cb, rssi_valid_cb);
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

		return SUCCESS;
	} else {

	}

	if (rx_cfg != NULL)
	{

		start_rx(rx_cfg);
		memcpy(&current_rx_cfg, rx_cfg, sizeof(hw_rx_cfg_t));
	}
	else
	{
		start_rx(&current_rx_cfg);
	}


	return SUCCESS;
}


error_t hw_radio_send_packet(hw_radio_packet_t* packet, tx_packet_callback_t tx_cb)
{
	// TODO error handling EINVAL, ESIZE, EOFF
	if(current_state == HW_RADIO_STATE_TX)
		return EBUSY;

	uint8_t data_length = packet->length + 1;

	if (packet->tx_meta.tx_cfg.channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
	{

		DPRINT("Original packet: %d", data_length);
		DPRINT_DATA(packet->data, data_length);

		data_length = fec_encode(packet->data, data_length);
		DPRINT("Encoded packet: %d", data_length);
		DPRINT_DATA(packet->data, data_length);

	} else {
		if (has_hardware_crc)
			data_length -= 2;
	}

	tx_packet_callback = tx_cb;

	if(current_state == HW_RADIO_STATE_RX)
	{
		//pending_rx_cfg.channel_id = current_channel_id;
		//pending_rx_cfg.syncword_class = current_syncword_class;
		should_rx_after_tx_completed = true;
	}

	current_state = HW_RADIO_STATE_TX;
	current_packet = packet;


	DPRINT("Data to TX Fifo:");
	DPRINT_DATA(packet->data, data_length);


	configure_channel((channel_id_t*)&(packet->tx_meta.tx_cfg.channel_id));
	configure_eirp(packet->tx_meta.tx_cfg.eirp);
	configure_syncword_class(current_packet->tx_meta.tx_cfg.syncword_class, current_packet->tx_meta.tx_cfg.channel_id.channel_header.ch_coding);

	DEBUG_TX_START();
	DEBUG_RX_END();

	ezradioStartTx(packet, ez_channel_id, should_rx_after_tx_completed, data_length);
	return SUCCESS;
}

int16_t hw_radio_get_rssi()
{
	ezradio_cmd_reply_t ezradioReply;
	ezradio_get_modem_status(0, &ezradioReply);

	//return -120;

//	DPRINT("CURR_RSSI    %d", convert_rssi(ezradioReply.GET_MODEM_STATUS.CURR_RSSI));
//	DPRINT("LATCH_RSSI    %d", convert_rssi(ezradioReply.GET_MODEM_STATUS.LATCH_RSSI));
//	DPRINT("ANT1_RSSI    %d", convert_rssi(ezradioReply.GET_MODEM_STATUS.ANT1_RSSI));
//	DPRINT("ANT2_RSSI    %d", convert_rssi(ezradioReply.GET_MODEM_STATUS.ANT2_RSSI));
//	ezradio_frr_d_read(1, &ezradioReply);
//	DPRINT("FRR_D_VALUE    %d", convert_rssi(ezradioReply.FRR_D_READ.FRR_D_VALUE));

	uint8_t rss = ezradioReply.GET_MODEM_STATUS.CURR_RSSI > ezradioReply.GET_MODEM_STATUS.LATCH_RSSI ? ezradioReply.GET_MODEM_STATUS.CURR_RSSI : ezradioReply.GET_MODEM_STATUS.LATCH_RSSI;
	return convert_rssi(rss);
}

int16_t hw_radio_get_latched_rssi()
{
	ezradio_cmd_reply_t ezradioReply;
	//ezradio_get_modem_status(0, &ezradioReply);
	//return convert_rssi(ezradioReply.GET_MODEM_STATUS.LATCH_RSSI);

	ezradio_frr_d_read(1, &ezradioReply);
	return convert_rssi(ezradioReply.FRR_D_READ.FRR_D_VALUE);
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

error_t hw_radio_poweroff()
{
	ezradio_hal_AssertShutdown();
	current_state = HW_RADIO_STATE_OFF;
	return SUCCESS;
}

static void start_rx(hw_rx_cfg_t const* rx_cfg)
{
	DPRINT("start_rx");

	if (current_state == HW_RADIO_STATE_OFF)
		ezradio_hal_DeassertShutdown();

    current_state = HW_RADIO_STATE_RX;

    configure_channel(&(rx_cfg->channel_id));
    configure_syncword_class(rx_cfg->syncword_class, rx_cfg->channel_id.channel_header.ch_coding);

    rx_fifo_data_lenght = 0;
    if (rx_cfg->channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
    {
    	ezradioStartRx(ez_channel_id, false);
    } else {

    	ezradioStartRx(ez_channel_id, true);
    }

    DEBUG_RX_START();


    if(rssi_valid_callback != 0)
    {
      // TODO calculate/predict rssi response time and wait until valid. For now we wait 200 us.

      hw_busy_wait(200);
      rssi_valid_callback(hw_radio_get_rssi());
    }
}

static inline int16_t convert_rssi(uint8_t rssi_raw)
{
	return ((int16_t)(rssi_raw >> 1)) - (70 + RSSI_OFFSET);
}

static void ezradio_handle_end_of_packet()
{
	// fill rx_meta
	rx_packet->rx_meta.rssi = hw_radio_get_latched_rssi();
	rx_packet->rx_meta.lqi = 0;
	memcpy(&(rx_packet->rx_meta.rx_cfg.channel_id), &current_channel_id, sizeof(channel_id_t));
	if (has_hardware_crc && current_rx_cfg.channel_id.channel_header.ch_coding != PHY_CODING_FEC_PN9)
		rx_packet->rx_meta.crc_status = HW_CRC_VALID;
	else
		rx_packet->rx_meta.crc_status = HW_CRC_UNAVAILABLE;

	rx_packet->rx_meta.timestamp = timer_get_counter_value();
	//memcpy((void*)rx_packet->rx_meta.rx_cfg, (void*)&current_rx_cfg, sizeof(hw_rx_metadata_t));

	ezradio_fifo_info(EZRADIO_CMD_FIFO_INFO_ARG_FIFO_RX_BIT, NULL);

	DPRINT_DATA(rx_packet->data, expected_data_length);

	if (current_rx_cfg.channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
	{
		fec_decode_packet(rx_packet->data, expected_data_length, expected_data_length);
		//assert length and data[0] can only differ 1
	}

	DPRINT_PACKET(rx_packet, false);


	DEBUG_RX_END();

//					if(rx_packet_callback != NULL) // TODO this can happen while doing CCA but we should not be interrupting here (disable packet handler?)
	rx_packet_callback(rx_packet);
//					else
//						release_packet_callback(packet);

	if(current_state == HW_RADIO_STATE_RX)
	{
		start_rx(&current_rx_cfg);
	}
}

static void ezradio_int_callback()
{
	//DPRINT("ezradio ISR");

	ezradio_cmd_reply_t ezradioReply;
	ezradio_cmd_reply_t radioReplyLocal;
	ezradio_get_int_status(0x0, 0x0, 0x0, &ezradioReply);
	//ezradio_frr_a_read(3, &ezradioReply);

	//DPRINT(" - INT_PEND     %s", byte_to_binary(ezradioReply.FRR_A_READ.FRR_A_VALUE));
//	DPRINT(" - INT_PEND     %s", byte_to_binary(ezradioReply.GET_INT_STATUS.INT_PEND));
//	DPRINT(" - INT_STATUS   %s", byte_to_binary(ezradioReply.GET_INT_STATUS.INT_STATUS));
//	DPRINT(" - PH_PEND      %s", byte_to_binary(ezradioReply.GET_INT_STATUS.PH_PEND));
	//DPRINT(" - PH_STATUS    %s", byte_to_binary(ezradioReply.GET_INT_STATUS.PH_STATUS));
	//DPRINT(" - PH_STATUS    %s", byte_to_binary(ezradioReply.FRR_A_READ.FRR_B_VALUE));
//	DPRINT(" - MODEM_PEND   %s", byte_to_binary(ezradioReply.GET_INT_STATUS.MODEM_PEND));
//	DPRINT(" - MODEM_STATUS %s", byte_to_binary(ezradioReply.GET_INT_STATUS.MODEM_STATUS));
//	DPRINT(" - CHIP_PEND    %s", byte_to_binary(ezradioReply.GET_INT_STATUS.CHIP_PEND));
//	DPRINT(" - CHIP_STATUS  %s", byte_to_binary(ezradioReply.GET_INT_STATUS.CHIP_STATUS));

	//if (ezradioReply.FRR_A_READ.FRR_A_VALUE & EZRADIO_CMD_GET_INT_STATUS_REP_INT_PEND_PH_INT_PEND_BIT)
	if (ezradioReply.GET_INT_STATUS.INT_PEND & EZRADIO_CMD_GET_INT_STATUS_REP_INT_PEND_PH_INT_PEND_BIT)
	{

		//DPRINT("PH ISR");
		switch(current_state)
		{
			case HW_RADIO_STATE_RX:
				if ((ezradioReply.GET_INT_STATUS.PH_STATUS & EZRADIO_CMD_GET_INT_STATUS_REP_PH_STATUS_PACKET_RX_BIT) ||
						(ezradioReply.GET_INT_STATUS.PH_STATUS & EZRADIO_CMD_GET_INT_STATUS_REP_PH_STATUS_RX_FIFO_ALMOST_FULL_BIT))
				{
					//DPRINT("PACKET_RX IRQ");

					if(rx_packet_callback != NULL)
					{
						/* Check how many bytes we received. */
						ezradio_fifo_info(0, &radioReplyLocal);

						DPRINT("RX ISR packetLength: %d", radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT);

						if (rx_fifo_data_lenght == 0)
						{
							DPRINT("RX FIFO: %d", radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT);
							uint8_t buffer[4];
							ezradio_read_rx_fifo(4, buffer);
							if (current_rx_cfg.channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
							{
								uint8_t fec_buffer[4];
								memcpy(fec_buffer, buffer, 4);
								fec_decode_packet(fec_buffer, 4, 4);
								expected_data_length = fec_calculated_decoded_length(fec_buffer[0]+1);
								DPRINT("RX Packet Length: %d / %d", fec_buffer[0], expected_data_length);
							} else {
								expected_data_length = buffer[0] + 1;
							}
							rx_packet = alloc_packet_callback(expected_data_length);
							memcpy(rx_packet->data, buffer, 4);
							rx_fifo_data_lenght += 4;
							radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT-=4;
						}

						if (ezradioReply.GET_INT_STATUS.PH_STATUS & EZRADIO_CMD_GET_INT_STATUS_REP_PH_STATUS_PACKET_RX_BIT)
						{

							/* Read out the RX FIFO content. */
							ezradio_read_rx_fifo(radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT, &(rx_packet->data[rx_fifo_data_lenght]));
							//ezradio_read_rx_fifo(radioReplyLocal2.PACKET_INFO.LENGTH, packet->data);

							ezradio_handle_end_of_packet();
							return;
						}

						if (ezradioReply.GET_INT_STATUS.PH_STATUS & EZRADIO_CMD_GET_INT_STATUS_REP_PH_STATUS_RX_FIFO_ALMOST_FULL_BIT)
						{
							while (radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT > 0)
							{
								DPRINT("RX FIFO: %d", radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT);
								/* Read out the FIFO Count bytes of RX FIFO */
								ezradio_read_rx_fifo(radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT, &(rx_packet->data[rx_fifo_data_lenght]));
								rx_fifo_data_lenght += radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT;
								//DPRINT("%d of %d bytes collected", rx_fifo_data_lenght, rx_packet->data[0]+1);
								ezradio_fifo_info(0, &radioReplyLocal);

								if (rx_fifo_data_lenght >= expected_data_length)
								{
									ezradio_handle_end_of_packet();
									return;
								}
							}

							ezradio_int_callback();

							return;
						}

					}
				} else if (ezradioReply.GET_INT_STATUS.PH_STATUS & EZRADIO_CMD_GET_INT_STATUS_REP_PH_STATUS_CRC_ERROR_BIT)

				//} else if ( ezradioReply.FRR_A_READ.FRR_B_VALUE & EZRADIO_CMD_GET_INT_STATUS_REP_PH_STATUS_CRC_ERROR_BIT)
				{
					DPRINT("- PACKET_RX CRC_ERROR IRQ");
					/* Check how many bytes we received. */
					ezradio_fifo_info(0, &radioReplyLocal);
					DPRINT("RX ISR packetLength: %d", radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT);

					//ezradio_cmd_reply_t radioReplyLocal2;
					//ezradio_get_packet_info(0, 0, 0, &radioReplyLocal2);

					if (rx_fifo_data_lenght == 0)
					{
						rx_packet = alloc_packet_callback(radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT);
					}

					/* Read out the RX FIFO content. */
					ezradio_read_rx_fifo(radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT, &(rx_packet->data[rx_fifo_data_lenght]));

					rx_packet->rx_meta.crc_status = HW_CRC_INVALID;
					rx_packet->length = rx_packet->data[0] + 1;


					DPRINT_PACKET(rx_packet, false);


					DEBUG_RX_END();

					if(rx_packet_callback != NULL)
						rx_packet_callback(rx_packet);
					else
						release_packet_callback(rx_packet);

					if(current_state == HW_RADIO_STATE_RX)
					{
						start_rx(&current_rx_cfg);
					}

				} else {
					DPRINT((" - OTHER RX IRQ"));
				}



				break;
			case HW_RADIO_STATE_TX:
				if (ezradioReply.GET_INT_STATUS.PH_PEND & EZRADIO_CMD_GET_INT_STATUS_REP_PH_PEND_PACKET_SENT_PEND_BIT)
				//if (ezradioReply.FRR_A_READ.FRR_C_VALUE & EZRADIO_CMD_GET_INT_STATUS_REP_PH_PEND_PACKET_SENT_PEND_BIT)
				{
					DPRINT("PACKET_SENT IRQ");

					DEBUG_TX_END();
					if(!should_rx_after_tx_completed)
						switch_to_idle_mode();

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
						start_rx(&current_rx_cfg);
					}
//				} else if (ezradioReply.FRR_A_READ.FRR_C_VALUE & EZRADIO_CMD_GET_INT_STATUS_REP_PH_PEND_TX_FIFO_ALMOST_EMPTY_PEND_BIT)
//				{
//					DPRINT(" - TX FIFO Almost empty IRQ ");
//
//					ezradio_fifo_info(0, &radioReplyLocal);
//					DPRINT("TX FIFO Space: %d", radioReplyLocal.FIFO_INFO.TX_FIFO_SPACE);
//
//					//Fill fifo
//					#ifdef HAL_RADIO_USE_HW_CRC
//						int16_t new_length = current_packet->length-1 - tx_fifo_data_length;
//					#else
//						int16_t new_length = current_packet->length+1 - tx_fifo_data_length;
//					#endif
//					if (new_length > radioReplyLocal.FIFO_INFO.TX_FIFO_SPACE) new_length = radioReplyLocal.FIFO_INFO.TX_FIFO_SPACE;
//
//					while (new_length > 0)
//					{
//						ezradio_write_tx_fifo(new_length, &(current_packet->data[tx_fifo_data_length]));
//						tx_fifo_data_length += new_length;
//						DPRINT ("%d added -> %d", new_length, tx_fifo_data_length);
//
//					#ifdef HAL_RADIO_USE_HW_CRC
//						new_length = current_packet->length-1 - tx_fifo_data_length;
//					#else
//						new_length = current_packet->length+1 - tx_fifo_data_length;
//					#endif
//						if (new_length > radioReplyLocal.FIFO_INFO.TX_FIFO_SPACE) new_length = radioReplyLocal.FIFO_INFO.TX_FIFO_SPACE;
//					}
//
//					if (new_length == 0)
//					{
//						DPRINT("reprocess callback");
//						ezradio_int_callback();
//						return;
//					}
//
//					DPRINT ("%d added -> %d", new_length, tx_fifo_data_length);
				} else {
					DPRINT(" - OTHER IRQ");
				}
				break;
			default:
				//assert(false);
				DPRINT("State: %d", current_state);
		}
	}

//	if (ezradioReply.FRR_A_READ.FRR_A_VALUE & EZRADIO_CMD_GET_INT_STATUS_REP_INT_PEND_MODEM_INT_PEND_BIT)
//	{
//		DPRINT("MODEM ISR");
//	}

//	if (ezradioReply.FRR_A_READ.FRR_A_VALUE & EZRADIO_CMD_GET_INT_STATUS_REP_INT_PEND_CHIP_INT_PEND_BIT)
//	{
//		DPRINT("CHIP ISR");
//
//		if (current_state != HW_RADIO_STATE_IDLE)
//		{
//			if (ezradioReply.GET_INT_STATUS.CHIP_STATUS & EZRADIO_CMD_GET_INT_STATUS_REP_CHIP_STATUS_STATE_CHANGE_BIT)
//			{
//				ezradio_request_device_state(&radioReplyLocal);
//				DPRINT(" - Current State %d", radioReplyLocal.REQUEST_DEVICE_STATE.CURR_STATE);
//				DPRINT(" - Current channel %d", radioReplyLocal.REQUEST_DEVICE_STATE.CURRENT_CHANNEL);
//			}else {
//				DPRINT(" - OTHER IRQ");
//			}
//		}
//	}


}


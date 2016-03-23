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
 *
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
    .channel_header.ch_coding = PHY_CODING_PN9,
    .channel_header.ch_class = PHY_CLASS_HI_RATE,
    .channel_header.ch_freq_band = PHY_BAND_868,
    .center_freq_index = 0
};

static uint8_t ez_channel_id = 0;

static eirp_t current_eirp = 0;

static bool should_rx_after_tx_completed = false;

static hw_rx_cfg_t current_rx_cfg = {0x0000, PHY_SYNCWORD_CLASS0};

static inline int16_t convert_rssi(uint8_t rssi_raw);
static void start_rx(hw_rx_cfg_t const* rx_cfg);
static void ezradio_int_callback();
static void report_rssi();

static bool si4460_channel_ids_equal(const channel_id_t* a, const channel_id_t* b)
{
	if (a->channel_header_raw != b->channel_header_raw)
		return false;

	if (a->channel_header.ch_freq_band == PHY_BAND_868)
	{
		if ((uint8_t) (a->center_freq_index / 255) != (uint8_t) (b->center_freq_index / 255))
			return false;
	}

	return true;
}


static void configure_channel(const channel_id_t* channel_id)
{
	DPRINT("configure_channel: %s", byte_to_binary(channel_id->channel_header_raw));
	// only change settings if channel_id changed compared to current config
	// TODO: check if only channel number changes
	if(!si4460_channel_ids_equal(channel_id, &current_channel_id))
	{
		assert(channel_id->channel_header.ch_coding == PHY_CODING_PN9); // TODO implement other codings
		// TODO assert valid center freq index

		memcpy(&current_channel_id, channel_id, sizeof(channel_id_t)); // cache new settings

		// TODO preamble size depends on channel class

		// set freq band
		DPRINT("Set frequency band index: %d", channel_id->channel_header.ch_freq_band);

		// TODO validate
		switch(channel_id->channel_header.ch_freq_band)
		{
		// TODO calculate depending on rate and channr
		case PHY_BAND_433:
			ezradio_set_property(RF_MODEM_RAW_SEARCH2_2_433);

			if(channel_id->channel_header.ch_class == PHY_CLASS_LO_RATE)
			{
				assert(channel_id->center_freq_index <= 68);

				ezradio_set_property(RF_MODEM_MOD_TYPE_12_433_LR);
				ezradio_set_property(RF_MODEM_FREQ_DEV_0_1_433_LR);
				ezradio_set_property(RF_MODEM_TX_RAMP_DELAY_12_433_LR);
				ezradio_set_property(RF_MODEM_BCR_NCO_OFFSET_2_12_433_LR);
				ezradio_set_property(RF_MODEM_AFC_LIMITER_1_3_433_LR);
				ezradio_set_property(RF_MODEM_AGC_WINDOW_SIZE_12_433_LR);
				ezradio_set_property(RF_MODEM_RAW_CONTROL_8_433_LR);
				ezradio_set_property(RF_MODEM_SPIKE_DET_2_433_LR);
				ezradio_set_property(RF_MODEM_DSA_CTRL1_5_433_LR);
				ezradio_set_property(RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12_433_LR);
				ezradio_set_property(RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12_433_LR);
				ezradio_set_property(RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12_433_LR);
				ezradio_set_property(RF_PA_MODE_4_433_LR);
				ezradio_set_property(RF_SYNTH_PFDCP_CPFF_7_433_LR);
				ezradio_set_property(RF_FREQ_CONTROL_INTE_8_433_LR);
			}
			else if(channel_id->channel_header.ch_class == PHY_CLASS_NORMAL_RATE)
			{
				assert(channel_id->center_freq_index % 8 == 0 && channel_id->center_freq_index <= 270);
				ez_channel_id = channel_id->center_freq_index / 8;

				ezradio_set_property(RF_MODEM_MOD_TYPE_12_433_NR);
				ezradio_set_property(RF_MODEM_FREQ_DEV_0_1_433_NR);
				ezradio_set_property(RF_MODEM_TX_RAMP_DELAY_12_433_NR);
				ezradio_set_property(RF_MODEM_BCR_NCO_OFFSET_2_12_433_NR);
				ezradio_set_property(RF_MODEM_AFC_LIMITER_1_3_433_NR);
				ezradio_set_property(RF_MODEM_AGC_WINDOW_SIZE_12_433_NR);
				ezradio_set_property(RF_MODEM_RAW_CONTROL_8_433_NR);
				ezradio_set_property(RF_MODEM_SPIKE_DET_2_433_NR);
				ezradio_set_property(RF_MODEM_DSA_CTRL1_5_433_NR);
				ezradio_set_property(RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12_433_NR);
				ezradio_set_property(RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12_433_NR);
				ezradio_set_property(RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12_433_NR);
				ezradio_set_property(RF_PA_MODE_4_433_NR);
				ezradio_set_property(RF_SYNTH_PFDCP_CPFF_7_433_NR);
				ezradio_set_property(RF_FREQ_CONTROL_INTE_8_433_NR);
			}
			else if(channel_id->channel_header.ch_class == PHY_CLASS_HI_RATE)
			{
				assert(channel_id->center_freq_index % 8 == 0 && channel_id->center_freq_index <= 56);

				ezradio_set_property(RF_MODEM_MOD_TYPE_12_433_HR);
				ezradio_set_property(RF_MODEM_FREQ_DEV_0_1_433_HR);
				ezradio_set_property(RF_MODEM_TX_RAMP_DELAY_12_433_HR);
				ezradio_set_property(RF_MODEM_BCR_NCO_OFFSET_2_12_433_HR);
				ezradio_set_property(RF_MODEM_AFC_LIMITER_1_3_433_HR);
				ezradio_set_property(RF_MODEM_AGC_WINDOW_SIZE_12_433_HR);
				ezradio_set_property(RF_MODEM_RAW_CONTROL_8_433_HR);
				ezradio_set_property(RF_MODEM_SPIKE_DET_2_433_HR);
				ezradio_set_property(RF_MODEM_DSA_CTRL1_5_433_HR);
				ezradio_set_property(RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12_433_HR);
				ezradio_set_property(RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12_433_HR);
				ezradio_set_property(RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12_433_HR);
				ezradio_set_property(RF_PA_MODE_4_433_HR);
				ezradio_set_property(RF_SYNTH_PFDCP_CPFF_7_433_HR);
				ezradio_set_property(RF_FREQ_CONTROL_INTE_8_433_HR);
			}

			DPRINT("Set channel freq index: %d", channel_id->center_freq_index);
			break;
		case PHY_BAND_868:
			ezradio_set_property(RF_MODEM_RAW_SEARCH2_2_868);

			if(channel_id->channel_header.ch_class == PHY_CLASS_LO_RATE)
			{
				assert(channel_id->center_freq_index <= 279);

				ezradio_set_property(RF_MODEM_MOD_TYPE_12_868_LR);
				ezradio_set_property(RF_MODEM_FREQ_DEV_0_1_868_LR);
				ezradio_set_property(RF_MODEM_TX_RAMP_DELAY_12_868_LR);
				ezradio_set_property(RF_MODEM_BCR_NCO_OFFSET_2_12_868_LR);
				ezradio_set_property(RF_MODEM_AFC_LIMITER_1_3_868_LR);
				ezradio_set_property(RF_MODEM_AGC_WINDOW_SIZE_12_868_LR);
				ezradio_set_property(RF_MODEM_RAW_CONTROL_8_868_LR);
				ezradio_set_property(RF_MODEM_SPIKE_DET_2_868_LR);
				ezradio_set_property(RF_MODEM_DSA_CTRL1_5_868_LR);
				ezradio_set_property(RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12_868_LR);
				ezradio_set_property(RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12_868_LR);
				ezradio_set_property(RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12_868_LR);
				ezradio_set_property(RF_PA_MODE_4_868_LR);
				ezradio_set_property(RF_SYNTH_PFDCP_CPFF_7_868_LR);

				ez_channel_id = channel_id->center_freq_index % 255;
				if ((uint8_t) (channel_id->center_freq_index / 255) == 0)
				{
					ezradio_set_property(RF_FREQ_CONTROL_INTE_8_868_LR1);
				} else
				{
					ezradio_set_property(RF_FREQ_CONTROL_INTE_8_868_LR2);
				}
			}
			else if(channel_id->channel_header.ch_class == PHY_CLASS_NORMAL_RATE)
			{
				assert(channel_id->center_freq_index % 8 == 0 && channel_id->center_freq_index <= 270);
				ez_channel_id = channel_id->center_freq_index / 8;

				ezradio_set_property(RF_MODEM_MOD_TYPE_12_868_NR);
				ezradio_set_property(RF_MODEM_FREQ_DEV_0_1_868_NR);
				ezradio_set_property(RF_MODEM_TX_RAMP_DELAY_12_868_NR);
				ezradio_set_property(RF_MODEM_BCR_NCO_OFFSET_2_12_868_NR);
				ezradio_set_property(RF_MODEM_AFC_LIMITER_1_3_868_NR);
				ezradio_set_property(RF_MODEM_AGC_WINDOW_SIZE_12_868_NR);
				ezradio_set_property(RF_MODEM_RAW_CONTROL_8_868_NR);
				ezradio_set_property(RF_MODEM_SPIKE_DET_2_868_NR);
				ezradio_set_property(RF_MODEM_DSA_CTRL1_5_868_NR);
				ezradio_set_property(RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12_868_NR);
				ezradio_set_property(RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12_868_NR);
				ezradio_set_property(RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12_868_NR);
				ezradio_set_property(RF_PA_MODE_4_868_NR);
				ezradio_set_property(RF_SYNTH_PFDCP_CPFF_7_868_NR);
				ezradio_set_property(RF_FREQ_CONTROL_INTE_8_868_NR);
			} else {
				assert(channel_id->center_freq_index % 8 == 0 && channel_id->center_freq_index <= 270);
				ez_channel_id = channel_id->center_freq_index / 8;

				ezradio_set_property(RF_MODEM_MOD_TYPE_12_868_HR);
				ezradio_set_property(RF_MODEM_FREQ_DEV_0_1_868_HR);
				ezradio_set_property(RF_MODEM_TX_RAMP_DELAY_12_868_HR);
				ezradio_set_property(RF_MODEM_BCR_NCO_OFFSET_2_12_868_HR);
				ezradio_set_property(RF_MODEM_AFC_LIMITER_1_3_868_HR);
				ezradio_set_property(RF_MODEM_AGC_WINDOW_SIZE_12_868_HR);
				ezradio_set_property(RF_MODEM_RAW_CONTROL_8_868_HR);
				ezradio_set_property(RF_MODEM_SPIKE_DET_2_868_HR);
				ezradio_set_property(RF_MODEM_DSA_CTRL1_5_868_HR);
				ezradio_set_property(RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12_868_HR);
				ezradio_set_property(RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12_868_HR);
				ezradio_set_property(RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12_868_HR);
				ezradio_set_property(RF_PA_MODE_4_868_HR);
				ezradio_set_property(RF_SYNTH_PFDCP_CPFF_7_868_HR);
				ezradio_set_property(RF_FREQ_CONTROL_INTE_8_868_HR);
			}

			DPRINT("Set channel freq index: %d", channel_id->center_freq_index);
			break;
		case PHY_BAND_915:
			assert(false);

			break;

		}
	}

}

static void configure_eirp(const eirp_t eirp)
{
	DPRINT("configure_eirp not implemented using max");
	uint8_t ddac = 0x7F; // max
	ezradio_set_property(0x22, 0x01, 0x01, ddac);
}

static void configure_syncword_class(syncword_class_t syncword_class)
{
	if(syncword_class != current_rx_cfg.syncword_class)
	{
		current_rx_cfg.syncword_class = syncword_class;
		switch (syncword_class)
		{
			case PHY_SYNCWORD_CLASS0:
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNC_BITS_CS0_0);
				break;
			case PHY_SYNCWORD_CLASS1:
				ezradio_set_property(RADIO_CONFIG_SET_PROPERTY_SYNC_BITS_CS0_1);
				break;
		}
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
	configure_syncword_class(current_rx_cfg.syncword_class);

	sched_register_task((&report_rssi));

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

	assert(packet->length < 63); // long packets not yet supported

	tx_packet_callback = tx_cb;

	if(current_state == HW_RADIO_STATE_RX)
	{
		//pending_rx_cfg.channel_id = current_channel_id;
		//pending_rx_cfg.syncword_class = current_syncword_class;
		should_rx_after_tx_completed = true;
	}

	current_state = HW_RADIO_STATE_TX;
	current_packet = packet;

#ifdef FRAMEWORK_LOG_ENABLED // TODO more granular
	log_print_stack_string(LOG_STACK_PHY, "Data to TX Fifo:");
#ifdef HAL_RADIO_USE_HW_CRC
	log_print_data(packet->data, packet->length - 1 );
#else
	log_print_data(packet->data, packet->length + 1 );
#endif
#endif

	configure_channel((channel_id_t*)&(current_packet->tx_meta.tx_cfg.channel_id));
	configure_eirp(current_packet->tx_meta.tx_cfg.eirp);
	configure_syncword_class(current_packet->tx_meta.tx_cfg.syncword_class);

	DEBUG_TX_START();
	DEBUG_RX_END();

	ezradioStartTx(packet, ez_channel_id, should_rx_after_tx_completed);
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

static void report_rssi()
{
	int16_t rss = hw_radio_get_rssi();
	rssi_valid_callback(rss);
}

static void start_rx(hw_rx_cfg_t const* rx_cfg)
{
	DPRINT("start_rx");

	if (current_state == HW_RADIO_STATE_OFF)
		ezradio_hal_DeassertShutdown();

    current_state = HW_RADIO_STATE_RX;

    configure_channel(&(rx_cfg->channel_id));
    configure_syncword_class(rx_cfg->syncword_class);

    ezradioStartRx((uint8_t) (current_channel_id.center_freq_index));

    DEBUG_RX_START();


    if(rssi_valid_callback != 0)
    {
    	timer_post_task_delay(&report_rssi, TIMER_TICKS_PER_SEC / 5000);
    }
}

static inline int16_t convert_rssi(uint8_t rssi_raw)
{
	return ((int16_t)(rssi_raw >> 1)) - (70 + RSSI_OFFSET);
}

static void ezradio_int_callback()
{
	DPRINT("ezradio ISR");

	ezradio_cmd_reply_t ezradioReply;
	ezradio_cmd_reply_t radioReplyLocal;
	ezradio_get_int_status(0x0, 0x0, 0x0, &ezradioReply);

	DPRINT(" - INT_PEND     %s", byte_to_binary(ezradioReply.GET_INT_STATUS.INT_PEND));
	DPRINT(" - INT_STATUS   %s", byte_to_binary(ezradioReply.GET_INT_STATUS.INT_STATUS));
	DPRINT(" - PH_PEND      %s", byte_to_binary(ezradioReply.GET_INT_STATUS.PH_PEND));
	DPRINT(" - PH_STATUS    %s", byte_to_binary(ezradioReply.GET_INT_STATUS.PH_STATUS));
	DPRINT(" - MODEM_PEND   %s", byte_to_binary(ezradioReply.GET_INT_STATUS.MODEM_PEND));
	DPRINT(" - MODEM_STATUS %s", byte_to_binary(ezradioReply.GET_INT_STATUS.MODEM_STATUS));
	DPRINT(" - CHIP_PEND    %s", byte_to_binary(ezradioReply.GET_INT_STATUS.CHIP_PEND));
	DPRINT(" - CHIP_STATUS  %s", byte_to_binary(ezradioReply.GET_INT_STATUS.CHIP_STATUS));

	if (ezradioReply.GET_INT_STATUS.INT_PEND & EZRADIO_CMD_GET_INT_STATUS_REP_INT_PEND_MODEM_INT_PEND_BIT)
	{
		DPRINT("MODEM ISR");
	}

	if (ezradioReply.GET_INT_STATUS.INT_PEND & EZRADIO_CMD_GET_INT_STATUS_REP_INT_PEND_PH_INT_PEND_BIT)
	{
		DPRINT("PH ISR");
		switch(current_state)
		{
			case HW_RADIO_STATE_RX:
				//if (ezradioReply.GET_INT_STATUS.PH_PEND & EZRADIO_CMD_GET_INT_STATUS_REP_PH_PEND_PACKET_RX_PEND_BIT)
				if (ezradioReply.GET_INT_STATUS.PH_STATUS & EZRADIO_CMD_GET_INT_STATUS_REP_PH_STATUS_PACKET_RX_BIT)
				{
					DPRINT("PACKET_RX IRQ");

					if(rx_packet_callback != NULL)
					{
						/* Check how many bytes we received. */
						ezradio_fifo_info(0, &radioReplyLocal);

						DPRINT("RX ISR packetLength: %d", radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT);

						//ezradio_cmd_reply_t radioReplyLocal2;
						//ezradio_get_packet_info(0, 0, 0, &radioReplyLocal2);

						hw_radio_packet_t* packet = alloc_packet_callback(radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT);
						packet->length = radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT;

			//            hw_radio_packet_t* packet = alloc_packet_callback(radioReplyLocal2.PACKET_INFO.LENGTH);
			//            packet->length = radioReplyLocal2.PACKET_INFO.LENGTH;

						/* Read out the RX FIFO content. */
						ezradio_read_rx_fifo(radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT, packet->data);
						//ezradio_read_rx_fifo(radioReplyLocal2.PACKET_INFO.LENGTH, packet->data);

						// fill rx_meta
						packet->rx_meta.rssi = hw_radio_get_latched_rssi();
						packet->rx_meta.lqi = 0;
						memcpy(&(packet->rx_meta.rx_cfg.channel_id), &current_channel_id, sizeof(channel_id_t));
#ifdef HAL_RADIO_USE_HW_CRC
						packet->rx_meta.crc_status = HW_CRC_VALID;
#else
						packet->rx_meta.crc_status = HW_CRC_UNAVAILABLE;
#endif
						packet->rx_meta.timestamp = timer_get_counter_value();

						ezradio_fifo_info(EZRADIO_CMD_FIFO_INFO_ARG_FIFO_RX_BIT, NULL);

			#ifdef FRAMEWORK_LOG_ENABLED
						log_print_raw_phy_packet(packet, false);
			#endif

						DEBUG_RX_END();

//					if(rx_packet_callback != NULL) // TODO this can happen while doing CCA but we should not be interrupting here (disable packet handler?)
						rx_packet_callback(packet);
//					else
//						release_packet_callback(packet);
					}
				}

				if ( ezradioReply.GET_INT_STATUS.PH_PEND & EZRADIO_CMD_GET_INT_STATUS_REP_PH_PEND_CRC_ERROR_PEND_BIT)
				{
					DPRINT("PACKET_RX CRC_ERROR");
					/* Check how many bytes we received. */
					ezradio_fifo_info(0, &radioReplyLocal);
					DPRINT("RX ISR packetLength: %d", radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT);

					//ezradio_cmd_reply_t radioReplyLocal2;
					//ezradio_get_packet_info(0, 0, 0, &radioReplyLocal2);

					hw_radio_packet_t* packet = alloc_packet_callback(radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT);
					packet->length = radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT;

					/* Read out the RX FIFO content. */
					ezradio_read_rx_fifo(radioReplyLocal.FIFO_INFO.RX_FIFO_COUNT, packet->data);

					packet->rx_meta.crc_status = HW_CRC_INVALID;

					#ifdef FRAMEWORK_LOG_ENABLED
						log_print_raw_phy_packet(packet, false);
					#endif

					DEBUG_RX_END();

					if(rx_packet_callback != NULL)
						rx_packet_callback(packet);
									else
										release_packet_callback(packet);

				}

				if(current_state == HW_RADIO_STATE_RX)
				{
					start_rx(&current_rx_cfg);
				}

				break;
			case HW_RADIO_STATE_TX:
				if (ezradioReply.GET_INT_STATUS.PH_PEND & EZRADIO_CMD_GET_INT_STATUS_REP_PH_PEND_PACKET_SENT_PEND_BIT)
				{
					DPRINT("PACKET_SENT IRQ");

					DEBUG_TX_END();
					if(!should_rx_after_tx_completed)
						switch_to_idle_mode();

					current_packet->tx_meta.timestamp = timer_get_counter_value();

		#ifdef FRAMEWORK_LOG_ENABLED
					log_print_raw_phy_packet(current_packet, true);
		#endif
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
				} else {
					DPRINT(" - OTHER IRQ");
				}
				break;
			default:
				//assert(false);
				DPRINT("State: %d", current_state);
		}
	}

	if (ezradioReply.GET_INT_STATUS.INT_PEND & EZRADIO_CMD_GET_INT_STATUS_REP_INT_PEND_CHIP_INT_PEND_BIT)
	{
		DPRINT("CHIP ISR");

		if (current_state != HW_RADIO_STATE_IDLE)
		{
			if (ezradioReply.GET_INT_STATUS.CHIP_STATUS & EZRADIO_CMD_GET_INT_STATUS_REP_CHIP_STATUS_STATE_CHANGE_BIT)
			{
				ezradio_request_device_state(&radioReplyLocal);
				DPRINT(" - Current State %d", radioReplyLocal.REQUEST_DEVICE_STATE.CURR_STATE);
				DPRINT(" - Current channel %d", radioReplyLocal.REQUEST_DEVICE_STATE.CURRENT_CHANNEL);
			}else {
				DPRINT(" - OTHER IRQ");
			}
		}
	}


}


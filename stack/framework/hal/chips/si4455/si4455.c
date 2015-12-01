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
 * @author maarten.weyn@uantwerpen.be
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

#include "si4455.h"
#include "ezradio_api_lib.h"
#include "ezradio_plugin_manager.h"

#ifdef FRAMEWORK_LOG_ENABLED // TODO more granular
    #define DPRINT(...) log_print_stack_string(LOG_STACK_PHY, __VA_ARGS__)
#else
    #define DPRINT(...)
#endif


//#include "si4455_interface.h"
//#include "cc1101_constants.h"
//#include "cc1101_registers.h"


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


/* Rx packet data array */
static uint8_t radioRxPkt[EZRADIO_FIFO_SIZE];

static void appPacketReceivedCallback ( EZRADIODRV_Handle_t handle, Ecode_t status )
{
  //Silent warning.
  (void)handle;

  if ( status == ECODE_EMDRV_EZRADIODRV_OK )
  {
    /* Read out and print received packet data:
     *  - print 'ACK' in case of ACK was received
     *  - print the data if some other data was received. */
//    if ( (radioRxPkt[APP_PKT_DATA_START] == 'A') &&
//         (radioRxPkt[APP_PKT_DATA_START + 1] == 'C') &&
//         (radioRxPkt[APP_PKT_DATA_START + 2] == 'K') )
//    {
//      printf("-->Data RX: ACK\n");
//    }
//    else
//    {
//      uint16_t rxData = *(uint16_t *)(radioRxPkt + APP_PKT_DATA_START);
//      printf("-->Data RX: %05d\n", rxData);
//    }
  }
}

static void appPacketTransmittedCallback ( EZRADIODRV_Handle_t handle, Ecode_t status )
{
  if ( status == ECODE_EMDRV_EZRADIODRV_OK )
  {
    /* Sign tx passive state */
    //appTxActive = false;

    /* Change to RX state */
    ezradioStartRx( handle );
  }
}

static void appPacketCrcErrorCallback ( EZRADIODRV_Handle_t handle, Ecode_t status )
{
  if ( status == ECODE_EMDRV_EZRADIODRV_OK )
  {
	  DPRINT("-->Pkt  RX: CRC Error\n");

    /* Change to RX state */
    ezradioStartRx( handle );
  }
}

static void configure_channel(const channel_id_t* channel_id)
{

}

static void configure_eirp(const eirp_t eirp)
{

}

static void configure_syncword_class(syncword_class_t syncword_class)
{

}

error_t hw_radio_init(alloc_packet_callback_t alloc_packet_cb,
                      release_packet_callback_t release_packet_cb)
{
	alloc_packet_callback = alloc_packet_cb;
	release_packet_callback = release_packet_cb;

	current_state = HW_RADIO_STATE_IDLE;

	/* EZRadio driver init data and handler */
	EZRADIODRV_HandleData_t appRadioInitData = EZRADIODRV_INIT_DEFAULT;
	EZRADIODRV_Handle_t appRadioHandle = &appRadioInitData;

	/* EZRadio response structure union */
	ezradio_cmd_reply_t ezradioReply;

	/* Configure packet transmitted callback. */
	appRadioInitData.packetTx.userCallback = &appPacketTransmittedCallback;

	/* Configure packet received buffer and callback. */
	appRadioInitData.packetRx.userCallback = &appPacketReceivedCallback;
	appRadioInitData.packetRx.pktBuf = radioRxPkt;

	/* Configure packet received with CRC error callback. */
	appRadioInitData.packetCrcError.userCallback = &appPacketCrcErrorCallback;

	/* Initialize EZRadio device. */
	ezradioInit( appRadioHandle );

	/* Reset radio fifos and start reception. */
	ezradioResetTRxFifo();

	//cc1101_interface_init(&end_of_packet_isr);
	//cc1101_interface_reset_radio_core();
	//cc1101_interface_write_rfsettings(&rf_settings);



	// configure default channel, eirp and syncword
	configure_channel(&current_channel_id);
	configure_eirp(current_eirp);
	configure_syncword_class(current_syncword_class);
}

error_t hw_radio_set_rx(hw_rx_cfg_t const* rx_cfg, rx_packet_callback_t rx_cb, rssi_valid_callback_t rssi_valid_cb)
{


    return ERROR;
}


error_t hw_radio_send_packet(hw_radio_packet_t* packet, tx_packet_callback_t tx_cb)
{
	  return ERROR;

}

int16_t hw_radio_get_rssi()
{
	  return ERROR;
}

error_t hw_radio_set_idle()
{
	  return ERROR;
}


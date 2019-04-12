/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2018 University of Antwerp
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

/*! \file phy.h
 * \addtogroup PHY
 * \ingroup D7AP
 * @{
 * \brief This module implements the phy layer of the D7AP.
 *
 * \author philippe.nunes@cortus.com
 *
 */
#ifndef __PHY_H_
#define __PHY_H_

#include "string.h"

#include "link_c.h"
#include "errors.h"
#include "hal_defs.h"
#include "timer.h"
#include "hwradio.h"
#include "platform_defs.h"
#include "dae.h"

#define HW_RSSI_INVALID 0x7FFF

#define PACKET_MAX_SIZE 256

#define BACKGROUND_FRAME_LENGTH 6
#define BACKGROUND_DLL_HEADER_LENGTH 2

/* \brief The syncword classes as defined in D7AP
 *
 */
typedef enum
{
    PHY_SYNCWORD_CLASS0 = 0x00,
    PHY_SYNCWORD_CLASS1 = 0x01
} phy_syncword_class_t;

typedef enum
{
    PREAMBLE_LOW_RATE_CLASS = 4, //(4 bytes, 32 bits)
    PREAMBLE_NORMAL_RATE_CLASS = 4, //(4 bytes, 32 bits)
    PREAMBLE_HI_RATE_CLASS = 6, //(6 bytes, 48 bits)
} phy_preamble_min_length_t;

/** \brief type of the 'syncword class'
 *
 */
typedef uint8_t    syncword_class_t;

/** \brief type of the 'eirp' used to transmit packets
 *
 */
typedef int8_t    eirp_t;

/** \brief The 'RX Configuration' used when receiving a packet. */
typedef struct
{
    channel_id_t channel_id;         /**< The channel_id of the D7A 'channel' to which the radio is tuned */
    syncword_class_t syncword_class; /**< The 'syncword' class used */
    int16_t rssi_thr;                /**< The rssi threshold to trigger the demodulation */
} phy_rx_config_t;


/** \brief The 'TX Configuration' to use when sending a packet.
 *
 * This struct contains settings to be applied before transmitting a packet. These settings are applied
 * on a per-packet basis and must be supplied as a parameter to hw_radio_send_packet(). The settings used are also stored in the
 * hw_tx_metadata attached to the packet upon completion of the transmission
 *
 */
typedef struct
{
    channel_id_t channel_id;           /**< The channel_id of the D7A 'channel' on which to send the packet */
    syncword_class_t syncword_class;   /**< The 'syncword' class used */
    eirp_t eirp;                       /**< The transmission power level measured in dBm [-39,+10].
                                        *   If the value specified is not supported by the driver,
                                        *   the nearest supported value is used instead */
} phy_tx_config_t;


typedef struct
{
    union
    {
        phy_rx_config_t rx;    /**< The RX configuration of the received packet */
        phy_tx_config_t tx;    /**< The TX configuration to transmit the packet */
    };
} phy_config_t;


/** \brief Type definition for the rssi_valid callback function.
 *
 * The rssi_valid callback is called by the PHY driver every time the RSSI measurements of the 
 * radio become valid after the radio is put in RX mode (see hw_radio_set_rx).
 *
 * A callback to the rssi_valid function is not only triggered by a call to hw_radio_set_rx, but also
 * but also by a call to hw_radio_send_packet. In that case, rssi_valid is called when the radio enters RX 
 * mode after the packet has been transmitted. (Unless of course the radio was put in IDLE mode during tx).
 *
 * This function is called from an interrupt context and should therefore do as little processing as possible.
 *
 */
typedef void (*rssi_valid_callback_t)(int16_t cur_rssi);


/** \brief Initiate a packet transmission over the air with the specified TX settings.
 *
 * This function sends the packet pointed to by the packet parameter over the air. Packets are always 
 * transmitted using the tx_cfg settings in the tx_meta field of the supplied packet. If these settings are 
 * not valid, EINVAL is returned and the packet is not transmitted. Moreover the 'length' field of the packet 
 * must also be properly configured. More specifically, the radio will send the first 'length' bytes of the 
 * packet's 'data' buffer over the air. It is the responsibility of the caller to ensure that the 'length' of 
 * the packet has been properly configured.
 *
 * Packet transmission is always done *asynchronously*, that is: a packet transmission is initiated by a call 
 * to this function, but is not completed until the tx_packet_callback_t function supplied to the 
 * hw_radio_init function is invoked by the radio driver. If this function is 0x0, no callback will occur. 
 * In that case the user must check that the transmission has finished by querying the hw_radio_tx_busy 
 * function. The packet buffer supplied to this function *MUST* remain available until the transmission has 
 * been completed. It should be noted that the tx_callback function will ONLY be called if this function 
 * pointer != 0x0 and the hw_radio_send_packet function returns SUCCESS.
 *
 * If a transmission is initiated while the radio is in IDLE mode, the radio switches directly from IDLE mode to 
 * TX mode to transmit the packet. After the packet has been sent, it switches back to IDLE mode (unless 
 * hw_radio_set_rx() is called while the TX is in progress).
 *
 * If a transmission is initiated while the radio is in RX mode, the radio switches immediately to TX mode to 
 * transmit the packet. If a packet reception is in progress while this function is called, this packet is 
 * dropped. Once the packet has been sent, the radio switches back to IDLE mode , unless hw_radio_set_rx() is
 * called while the TX is still in progress.
 *
 * \param packet  A pointer to the start of the Foreground frame to be transmitted
 *
 * \return error_t	SUCCESS if the packet transmission has been successfully initiated.
 *          EINVAL if the tx_cfg parameter contains invalid settings
 *          EBUSY if another TX operation is already in progress
 *          ESIZE if the packet is either too long or too small
 *          EOFF if the radio has not yet been initialised
 */
__LINK_C error_t phy_send_packet(hw_radio_packet_t* packet, phy_tx_config_t* config, tx_packet_callback_t tx_callback);


/** \brief Initiate a packet transmission with a preliminary advertising period for ad-hoc
 *         synchronization with the responder.
 *
 * \param packet A pointer to the start of the Foreground frame to be transmitted
 * If the ETA parameter is set, the packet transmission requires a preliminary advertising period for ad-hoc
 * synchronization with the responder. In this case the dll_header_bg_frame parameter should be not NULL.
 */

error_t phy_send_packet_with_advertising(hw_radio_packet_t* packet, phy_tx_config_t* config, uint8_t dll_header_bg_frame[2], uint16_t eta, tx_packet_callback_t tx_callback);


/** \brief Start a background scan.
 *
 * During scan automation, the receiver is waiting for a valid D7AAdvP frame.
 *
 * \param channel_id      The channel_id of the D7A 'channel' to which the radio should be tuned
 * \param syncword_class  The 'syncword' class to use
 *
 * \param rssi_thr        The scan shall stop immediately upon failure to detect a modulated signal on the channel
 *                        rssi_thr gives the signal strength threshold to detect a modulated signal
 *
 * \return error_t SUCCESS if the radio was put in background scan mode
 *                 EINVAL if the supplied rx_cfg contains invalid parameters.
 *                 EOFF if the radio is not yet initialised.
 */
error_t phy_start_background_scan(phy_rx_config_t* config, rx_packet_callback_t rx_cb);

/* \brief Utility function to check whether two channel_id_t are equal
 *
 * \param a      The first channel_id
 * \param b      The second channel_id
 * \return bool  true if the two channel_id are equal, false otherwise.
 */
bool phy_radio_channel_ids_equal(const channel_id_t* a, const channel_id_t* b);

uint16_t phy_calculate_tx_duration(phy_channel_class_t channel_class, phy_coding_t ch_coding, uint8_t packet_length, bool payload_only);

void phy_continuous_tx(phy_tx_config_t const* tx_cfg, uint8_t time_period, tx_packet_callback_t tx_cb);

error_t phy_init();

error_t phy_start_rx(channel_id_t *channel, syncword_class_t syncword_class, rx_packet_callback_t rx_cb);

/** \brief Start the energy scan sequence on the radio.
 *
 * \param channel_id   The channel to perform the energy scan on.
 * \param rssi_cb      The rssi_valid_callback_t function to call whenever the energy scan is complete.
 * \param cca_duration The duration, in milliseconds, for the channel to be scanned.
 *
 * \return error_t SUCCESS Successfully started scanning the channel.
  *                EOFF if the radio is not yet initialised.
 */
error_t phy_start_energy_scan(channel_id_t *channel, rssi_valid_callback_t rssi_cb, int16_t scan_duration);


#endif //__HW_RADIO_H_

/** @}*/

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

/*! \file hwradio.h
 * \addtogroup radio
 * \ingroup HAL
 * @{
 * \brief The interface specification for accessing the radio interface.
 *
 * \author Daniel van den Akker
 * \author Glenn Ergeerts
 * \author Philippe Nunes
 *
 */
#ifndef __HW_RADIO_H_
#define __HW_RADIO_H_

#include "string.h"

#include "link_c.h"
#include "hal_defs.h"
#include "timer.h"
#include "platform_defs.h"

#define HW_RSSI_INVALID 0x7FFF

typedef struct xcvr_handle xcvr_handle_t;

/**
 * @brief   Reference to the transceiver handle struct
 */
extern xcvr_handle_t xcvr;

/**
 * @brief   used  to set or get the state of a radio device
 */
typedef enum {
    HW_STATE_OFF = 0,       /**< powered off */
    HW_STATE_SLEEP,         /**< sleep mode */
    HW_STATE_IDLE,          /**< idle mode,
                             *   the device listens to receive packets */
    HW_STATE_RX,            /**< receive mode,
                             *   the device currently receives a packet */
    HW_STATE_TX,            /**< transmit mode,
                             *   set: triggers transmission of a preloaded packet
                             *   The resulting state of the network device is @ref HW_STATE_IDLE
                             *   get: the network device is in the process of
                             *   transmitting a packet */
    HW_STATE_RESET,         /**< triggers a hardware reset. The resulting
                             *   state of the network device is @ref HW_STATE_IDLE */
    HW_STATE_STANDBY,       /**< standby mode. The devices is awake but
                             *   not listening to packets. */
    /* add other states if needed */
} hw_radio_state_t;

/** \brief The type for the result of a 'hardware' crc check
 *
 */
typedef enum
{
    HW_CRC_VALID = 0,
    HW_CRC_INVALID = 1,
    HW_CRC_UNAVAILABLE = 2
} hw_crc_t;

typedef enum {
    HW_DC_FREE_NONE = 0,         /**< No DC-FREE encoding/decoding */
    HW_DC_FREE_MANCHESTER,       /**< Manchester */
    HW_DC_FREE_WHITENING,        /**< PN9 whitening */
} hw_dc_free_t;

/** \brief The metadata attached to a received packet.
 *
 */
typedef struct
{
    timer_tick_t timestamp;	/**< The clock_tick of the framework timer at which the whole frame was received. */
    uint8_t lqi;			/**< The link quality indicator (LQI) reported by the radio for the received packet*/
    int16_t rssi;			/**< The Received signal strength (RSSI) reported by the radio for the received packet. */
    hw_crc_t crc_status;	/**< The crc status of the packet
                             *
                             * HW_CRC_UNAVAILABLE 	if the driver does not support hardware crc checking
                             * HW_CRC_INVALID 	if the CRC was not valid
                             * HW_CRC_VALID	if the CRC was valid
                             */

// TODO optimize struct for size. This was packed but resulted in alignment issues on Cortex-M0 so removed for now.
} hw_rx_metadata_t;

/** \brief The metadata an TX settings attached to a packet ready to be transmitted / that has been 
 *  transmitted.
 */
typedef struct
{
    timer_tick_t timestamp;	/**< The clock_tick of the framework timer at which the whole frame is transmitted. */

    // TODO optimize struct for size. This was packed but resulted in alignment issues on Cortex-M0 so removed for now.
} hw_tx_metadata_t;

/** \brief A radio layer packet that can be sent / received over the air using the HW radio interface.
 *
 * A hw_radio_packet_t consists of:
 *   - rx_meta / tx_meta: 		The metadata attached to the packet that is either collected by the radio driver
 *					upon reception of the packet or needed/returned by the radio driver upon 
 *					transmission of the packet.
 *   - the packet data itself:		a uint8[] of undetermined length that contains the actual packet data.
 *					for convenience, the first byte of this data array overlaps with the 
 *					'length' field containing the length of the data array.
 *
 * It should be noted that the rx_meta and tx_meta structs occupy the same memory space (unioned)
 * And that they have been defined so the fields of these structs that have the same meaning overlap.
 * Moreover, the fields of the hw_radio_packet_t structure are aligned such that the 'data' of a packet
 * is always in the same place, regardless of whether it is transmitted or received.
 *
 */
typedef struct
{
    union
    {
        hw_rx_metadata_t rx_meta;		/**< The TX Metadata of the packet */
        hw_tx_metadata_t tx_meta;		/**< The RX Metadata of the packet */
    };

    uint16_t    length;			/**< The length of the packet*/
    struct
    {
        uint8_t	__resv[0];		//this is ONLY here to keep the compiler happy: DO NOT USE
        uint8_t	data[];			/**< The packet data */
    };
    // TODO optimize struct for size. This was packed but resulted in alignment issues on Cortex-M0 so removed for now.
} hw_radio_packet_t;

/** \brief A convenience MACRO that calculates the minimum size of a buffer large enough to hold a single
 * hw_radio_packet_t of the specified length
 *
 * This functionality is defined as a MACRO rather than a static inline function to
 * allow it to be used for compile-time calculations.
 *
 * \param length		The length of the packet to be allocated (max 255)
 */
#define HW_PACKET_BUF_SIZE(length) (sizeof(hw_radio_packet_t) + length)

/** \brief Type definition for the 'new_packet' callback function
 *
 * The new_packet_callback_t function is called by the radio driver each time a (new) buffer is needed to store a
 * packet. This function is supplied with the *length* of the packet to be stored and is expected to 
 * return a pointer to a 'hw_radio_packet_t' that is sufficiently large to contain a packet that is at least
 * length bytes long. If no sufficiently large buffer can be allocated, this function MUST return NULL 
 * (0x0).
 *
 * It should be noted that this function is typically called while the packet is being received and as a 
 * result it is imperative that this function does at little processing as possible. Also, this function may 
 * be called both from a 'thread' and from an interrupt context. Implementors are advised to use atomic 
 * sections (where needed) to protect against concurrency issues.
 *
 * Once a packet has been allocated, it remains under the control of the radio driver until it is `released' 
 * by a call to either the release_packet_callback or the rx_packet_callback function.
 *
 * \param length		The length of the packet for which a buffer must be allocated
 * \return hw_radio_packet_t*	The allocated packet buffer. The buffer MUST be large enough for the
 *				data field to contain at least length bytes. If no sufficiently large 
 *				buffer can be allocated, 0x0 is returned.
 */
typedef hw_radio_packet_t* (*alloc_packet_callback_t)(uint16_t length);

/** \brief definition of the callback used by the radio driver to 'release' control of a previously allocated 
 *	   packet buffer.
 *
 * This callback is called by the radio driver when it determines that a previously allocated packet buffer is 
 * no longer needed (for instance because the RX was interrupted). If the packet WAS received correctly, 
 * control of the buffer is released through a call to rx_callback_packet_t.
 *
 * As with new_packet_callback_t, this function is called from an interrupt context during 
 * time-critical radio layer processing. As a result, this function should do as little processing as possible.
 *
 * \param packet	A pointer to the hw_radio_packet to release
 * 
 */
typedef void (*release_packet_callback_t)(hw_radio_packet_t* packet);

/** \brief Type definition for the rx callback function
 *
 * The rx_packet_callback_t function is called by the radio driver every time a new packet is received. This 
 * function is supplied with a pointer the hw_radio_packet containing the received packet.
 *
 * It should be noted that the `packet' pointer supplied to this callback function is *ALWAYS* a pointer that 
 * was obtained through a call to the new_packet_callback (new_packet_callback_t) function. By calling this 
 * function, the radio driver explicitly releases control of the previously allocated buffer back to the radio stack.
 * This means that, from the perspective of the radio driver, calling rx_packet_callback_t has the same 
 * effect as calling release_packet_callback_t.
 *
 * As with new_packet_callback_t, this function is called from an interrupt context.
 * While this function is being executed, no other interrupts can fire and no other packets can be 
 * received. As a result, this function should do as little processing as possible.
 *
 * \param	packet		A pointer to the received packet
 *
 */
typedef void (*rx_packet_callback_t)(hw_radio_packet_t* packet);


/** \brief Type definition for the rx header callback function
 *
 * The rx_packet_header_callback_t function is called by the radio driver every time a new packet header is received. This
 * function is supplied with a pointer the buffer containing the received packet header.
 *
 * \param    data    A pointer to the received packet header
 * \param    len     The length of the received packet header
 *
 */
typedef void (*rx_packet_header_callback_t)(uint8_t* data, uint8_t len);


/** \brief Type definition for the tx callback function
 *
 * The tx_packet_callback_t function is called by the radio driver upon completion of a packet transmission. 
 * This function is supplied with a pointer to the transmitted packet (the packet supplied to the 
 * hw_radio_send_packet function)
 *
 * As with new_packet_callback_t, this function is called from an interrupt context and should therefore do 
 * as little processing as possible.
 *
 * \param timestamp    The timestamp when the packet has been transmitted
 *
 */
typedef void (*tx_packet_callback_t)(timer_tick_t timestamp);


/** \brief Type definition for the tx refill callback function
 *
 * The tx_refill_callback_t function is called by the radio driver when the TX FIFO needs to be refilled before
 * an underrun occur.
 *
 * \param    len     The length of the remaining bytes before refill
 *
 */
typedef void (*tx_refill_callback_t)(uint16_t remaining_bytes_len);


/** \brief Type definition for the rssi_valid callback function.
 *
 * The rssi_valid callback is called by the radio driver every time the RSSI measurements of the 
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

/**
 * @brief Type definition for the tx_lora_packet callback function.
 * 
 * The tx_lora_packet callback is called by the radio driver upon the successful completion of a LoRa transmission, which is indicated through the TxDone interrupt on DIO0.
 * 
 * The callback triggers the equivalent LoRaMac function to enable the handling of this case on the MAC layer.
 */
typedef void (*tx_lora_packet_callback_t)( void );

/**
 * @brief  Type definition for the rx_lora_packet callback function.
 * 
 * The rx_lora_packet callback is called by the radio driver upon the successful reception of a LoRa transmission, which is indicated through the RxDone interrupt on DIO0.
 * 
 * The callback triggers the equivalent LoRaMac function to enable the handling of this case on the MAC layer.
 * 
 */
typedef void (*rx_lora_packet_callback_t)( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/**
 * @brief  Type definition for the rx_lora_error callback function.
 * 
 * The rx_lora_error callback is called by the radio driver after the reception of a LoRa transmission, but when the CRC check of the transmission has failed. The reception of the frame is indicated through the RxDone interrupt on DIO0.
 * The validity of the CRC is then checked through the PAYLOADCRCERROR interrupt flag. If the flag is set, this callback is called.
 * 
 * The callback triggers the equivalent LoRaMac function to enable the handling of this case on the MAC layer
 * 
 */
typedef void (*rx_lora_error_callback_t)( void );

/**
 * @brief Type definition for the rx_lora_timeout callback function.
 * 
 * The rx_lora_timeout callback is called by the radio driver in the cases where the chip's time in receive mode has exceeded an expected threshold, without the detection of any frame.
 * 
 * This occurs in two cases: 
 * 
 * 1) where the chip has been set into RXSINGLE mode, and no preamble is detected during the time in receive mode. This is indicated through the RxTimeout interrupt on DIO1.
 * 
 * 2) where a timeout has been set just prior to going into receive mode. This is typically used with RXCONTINUOUS mode, as the timeout would have no effect when used with RXSINGLE.
 * 
 */
typedef void (*rx_lora_timeout_callback_t)( void );

/*!
 * hwradio callbacks structure
 * Used to notify upper layers of radio events
 */
typedef struct hwradio_init_args
{
    alloc_packet_callback_t alloc_packet_cb;
    release_packet_callback_t release_packet_cb;
    rx_packet_callback_t rx_packet_cb;
    rx_packet_header_callback_t rx_packet_header_cb;
    tx_packet_callback_t tx_packet_cb;
    tx_refill_callback_t tx_refill_cb;
    tx_lora_packet_callback_t tx_lora_packet_cb;
    rx_lora_packet_callback_t rx_lora_packet_cb;
    rx_lora_error_callback_t rx_lora_error_cb;
    rx_lora_timeout_callback_t rx_lora_timeout_cb; 
} hwradio_init_args_t;

/** \brief Initialize the radio driver.
 *
 * After initialization, the radio is in IDLE state. The RX must be explicitly enabled by a call to
 * hw_radio_set_rx(...) before any packets can be received.
 *
 * \param init_args specifies the callback function pointers
 *
 * \return	error_t			SUCCESS  if the radio driver was initialised successfully
 * 							EINVAL	 if any of the callback functions is 0x0
 * 							EALREADY if the radio driver was already initialised
 * 							FAIL	 if the radio driver could not be initialised
 */
__LINK_C error_t hw_radio_init(hwradio_init_args_t* init_args);

/** \brief Stop the radio driver, and free the hardware resources (SPI, GPIO interrupts, ...)
 */
__LINK_C void hw_radio_stop(void);

/** \brief Initializes all GPIO pins required by the radio.
 * This is a weak symbol which needs to be implemented in the platform if you want to use this
 */
__LINK_C __attribute__((weak)) void hw_radio_io_init(bool disable_interrupts);

/** \brief Reset the radio.
 * This is a weak symbol which can be implemented in the platform if you want to use this
 */
__LINK_C __attribute__((weak)) void hw_radio_reset(void);


/** \brief Deinitializes all GPIO pins required by the radio.
 * This is a weak symbol which needs to be implemented in the platform if you want to use this
 */
__LINK_C __attribute__((weak)) void hw_radio_io_deinit(void);

/** \brief Set the radio in the IDLE mode.
 *
 * When the radio is IDLE, the tranceiver is disabled to reduce energy consumption. 
 * While in this state, it is not possible to receive any packets. The tranceiver can 
 * be re-enabled by a call to hw_radio_set_rx().
 *
 * It should be noted that although packet reception is not possible while in IDLE mode,
 * it is possible to transmit packets (by calling hw_radio_send_packet). In that case the 
 * radio will go directly from IDLE to TX mode to transmit the packet and then go back to 
 * IDLE mode once the packet has been sent.
 *
 * If the radio is placed in IDLE mode while a packet is being transmitted, the transmisison
 * is completed before the radio is placed in IDLE mode. If the radio is placed in IDLE mode 
 * while a packet is being received, the reception of the current packet is interrupted and the 
 * radio is placed in IDLE mode IMMEDIATELY.
 *
 * \return error_t	SUCCESS if the radio was put in IDLE mode (or will be after the current TX has finished)
 *			EALREADY if the radio was already in IDLE mode
 *			EOFF if the radio is not yet initialised.
 *
 */
__LINK_C error_t hw_radio_set_idle(void);

/** \brief Initiate a packet transmission over the air.
 *
 * This function sends the packet pointed to by the data parameter over the air. The 'length' field of the packet
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
 * pointer != 0x0 and the hw_radio_transmit function returns SUCCESS.
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
 * \param data                  A pointer to the start of the frame to be transmitted
 * \param len                   length bytes of the packet's data buffer
 *
 * \return error_t	SUCCESS if the packet transmission has been successfully initiated.
 *          EBUSY if another TX operation is already in progress
 *          ESIZE if the packet is either too long or too small
 *          EOFF if the radio has not yet been initialised
 */
error_t hw_radio_send_payload(uint8_t * data, uint16_t len);

/** \brief Check whether or not the radio is currently transmitting a packet
 *
 *  \return bool	true if the radio is currently transmitting a packet, false if it is not.
 */
__LINK_C bool hw_radio_tx_busy(void);

/** \brief Check whether or not the radio is currently receiving a packet
 *
 *  \return bool	true if the radio is currently receiving a packet, false if it is not.
 */
__LINK_C bool hw_radio_rx_busy(void);

/** \brief Check whether the RSSI value measured by the radio driver is valid or not.
 *
 * The RSSI will only be valid if the radio has been initialized and has been in RX mode long enough
 * for the RSSI to become valid (this is signaled by a callback to the rssi_valid_callback_t function)
 *
 * In general the following rule applies:
 *  - hw_radio_rssi_valid() == (hw_radio_get_rssi() == HW_RSSI_INVALID)
 *
 */
bool hw_radio_rssi_valid(void);

/** \brief Measure the current RSSI on the channel.
 *
 * If the RSSI is not valid (hw_radio_rssi_valied() returns false), the special value 'HW_RSSI_INVALID' is 
 * returned. Otherwise the current RSSI is returned. The RSSI is measured in dBm and must be rounded to the 
 * nearest 16-bit signed integer value.
 *
 * If HW_RSSI_INVALID is returned, the caller shoud ensure that the radio is actually in RX mode and,
 * if so, either schedule the RSSI to be read at a later time or wait for the rssi_valid callback to be 
 * invoked.
 * 
 * Measuring the RSSI is done using the 'current' RX settings of the radio and does NOT interrupt any receive
 * operation currently in progress. 
 *
 * Reading the RSSI for a different RX configuration can be done by: 
 *   -# Setting the required settings by calling hw_radio_set_rx
 *   -# Waiting for the rssi_valid callback to be invoked.
 *
 */
__LINK_C int16_t hw_radio_get_rssi(void);

hw_radio_state_t hw_radio_get_opmode(void);
void hw_radio_set_opmode(hw_radio_state_t opmode);

void hw_radio_set_center_freq(uint32_t center_freq);
void hw_radio_set_rx_bw_hz(uint32_t bw_hz);
void hw_radio_set_bitrate(uint32_t bps);
void hw_radio_set_tx_fdev(uint32_t fdev);
void hw_radio_set_preamble_size(uint16_t size);
void hw_radio_set_preamble_detector(uint8_t preamble_detector_size, uint8_t preamble_tol);
void hw_radio_set_rssi_config(uint8_t rssi_smoothing, uint8_t rssi_offset);

#if 0
void hw_radio_set_modulation_shaping(uint8_t shaping);
void hw_radio_set_preamble_polarity(uint8_t polarity);
void hw_radio_set_rssi_threshold(uint8_t rssi_thr);
void hw_radio_set_rssi_smoothing(uint8_t rssi_samples);
void hw_radio_set_sync_word_size(uint8_t sync_size);
void hw_radio_set_sync_on(uint8_t enable);
void hw_radio_set_preamble_detect_on(uint8_t enable);
#endif

void hw_radio_set_dc_free(uint8_t scheme);
void hw_radio_set_sync_word(uint8_t *sync_word, uint8_t sync_size);
void hw_radio_set_crc_on(uint8_t enable);

void hw_radio_set_payload_length(uint16_t length);

error_t hw_radio_set_idle(void);
bool hw_radio_is_idle(void);
bool hw_radio_is_rx(void);

void hw_radio_enable_refill(bool enable);
void hw_radio_enable_preloading(bool enable);

#ifdef USE_SX127X
/**
 * @brief swaps the chip between LoRa and FSK modes
 * 
 * @param use_lora: if true, use LoRa. If false, use FSK
 */
void hw_radio_switch_longRangeMode(bool use_lora);

/**
 * @brief sets the bandwidth and spreading factor of the chip in LoRa mode
 * 
 * @param lora_bw: the bandwidth of the channel
 * @param lora_SF: the spreading factor to send/receive using
 */
void hw_radio_set_lora_mode(uint32_t lora_bw, uint8_t lora_SF);

/**
 * @brief puts the device into a continous tx mode (in FSK mode)
 * 
 * NOTE: not implemented, as not used in regular use of LoRaMac-node
 * 
 * @param freq: the frequency to transmit at continuously
 * @param power: the tx power to transmit at 
 * @param time: the amount of time to transmit for 
 */
void hw_lora_set_tx_continuous_wave( uint32_t freq, int8_t power, uint16_t time );

/**
 * @brief changes the syncword in the LoRa preamble to indicate a "public" or "private" network
 * 
 * @param enable: if true, use public network. If false, use private network
 * 
 * Note that Semtech's intention behind future use of syncwords is unclear, but effectively at the moment if a device is using LoRaWAN it should use the 
 * public syncword set here (which comes from the LoRaWAN regional parameters document). If a device is transmitting using LoRa, but not LoRaWAN, then the private
 * syncword should be used.
 */
void hw_lora_set_public_network( bool enable );

/**
 * @brief sets the maximum payload length of the LoRa transmission. In LoRaWAN, the maximum payload length is dependent on the used data rate.
 * 
 * @param max: the max payload length 
 */
void hw_lora_set_max_payload_length( uint8_t max );

/**
 * @brief sets all of the LoRa TX-related parameters. This function is required by LoRaMac-node, to set all the required parameters all at once.
 */
void hw_lora_set_tx_config(uint32_t bandwidth, uint32_t datarate,
                        uint8_t coderate, uint16_t preambleLen,
                        bool fixLen, bool crcOn, bool freqHopOn,
                        uint8_t hopPeriod, bool iqInverted, uint32_t timeout);

/**
 * @brief sets all of the LoRa RX-related parameters. This function is required by LoRaMac-node, to set all the required parameters all at once.
 *
 * @param rxContinuous: whether the reception should use RXCONTINUOUS or RXSINGLE mode
 */
void hw_lora_set_rx_config(uint32_t bandwidth,
                         uint32_t datarate, uint8_t coderate,
                         uint16_t preambleLen,
                         uint16_t symbTimeout, bool fixLen,
                         uint8_t payloadLen,
                         bool crcOn, bool freqHopOn, uint8_t hopPeriod,
                         bool iqInverted, bool rxContinuous);


void hw_radio_set_lora_cont_tx(bool activate);

/**
 * @brief generates a random number using RSSI value readings
 * 
 * @return uint32_t the random number
 */
uint32_t hw_lora_random( void ); 
#endif

void hw_radio_set_tx_power(int8_t eirp);

void hw_radio_set_rx_timeout(uint32_t timeout);

#endif //__HW_RADIO_H_

/** @}*/

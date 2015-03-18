/* \file
 *
 * The interface specification for accessing the radio interface.
 * Please note: the currently defined functions are nothing more than
 * placeholders to test out the simulator. These functions WILL change
 * when we start working with the PHY
 *
 */
#ifndef __HW_RADIO_H_
#define __HW_RADIO_H_

#include "link_c.h"
#include "errors.h"

#define HW_RSSI_INVALID 0x7FFF

/** \brief spectrum id used to identify the spectrum settings
 *
 * This struct adheres to the 'Channel ID' format the Dash7 PHY layer. (@17/03/2015)
 */
typedef struct
{
    union
    {	
	uint8_t channel_header; //the raw (8-bit) channel header
	struct
	{
	    uint8_t ch_coding: 2; // the 'coding' field in the channel header
    	    uint8_t ch_class: 2;  // the 'class' field in the channel header
    	    uint8_t ch_freq_band: 4;   // the frequency 'band' field in the channel header
	};
    };
    uint8_t center_freq_index;    // the center frequency index of the channel id
} channel_id_t;

/** \brief type of the 'syncword class'
 *
 */
typedef uint8_t    syncword_class_t;

/** \brief type of the 'eirp' used to transmit packets
 *
 */
typedef int8_t	   eirp_t;



/** \brief The 'RX Configuration' for the radio. 
 *
 * This struct contains various settings that are used to configure the radio (such as the spectrum_id and 
 * the syncword). It must be passed as a parameter to ... and is also a part of the hw_rx_metadata attached 
 * to the received packets.
 *
 **/
typedef struct
{
    /** \brief The channel_id of the Dash 7 'channel' to which the radio is tuned
     *
     */
    channel_id_t channel_id;
    
    /** \brief The 'threshold' RSSI value for the radio to determine the channel to be 'busy'
     *
     */
    int16_t cca_rssi_threshold;

    /** \brief The 'syncword' class used to designate the SFD of the frame
     *
     */
    syncword_class_t syncword_class;
    
    /** \brief Whether or not the channel is deemed to be 'BUSY' if a packet is currently being received
     *
     */
    bool busy_on_rx;    

    /** \brief Whether or not the channel is deemed to be 'BUSY' if a packet is currently being transmitted
     *
     */
    bool busy_on_tx;    
    
    /** \brief Whether or not to attach a hw_rx_metadata struct to the received packet before it is passed to 
     * the higher layers of the stack.
     */
    bool report_metadata;
    
} hw_rx_cfg_t;

/** \brief The 'TX Configuration' to use when sending a packet.
 *
 * This struct contains settings to be applied before transmitting a packet. These settings are applied
 * on a per-packet basis and must be supplied as a parameter to ... The settings used are also stored in the
 * hw_tx_metadata attached to the packet upon completion of the transmission
 *
 */
typedef struct
{
    /** \brief The channel_id of the Dash 7 'channel' to which the radio is tuned
     *
     */
    channel_id_t channel_id;
    
    /** \brief The 'syncword' class used to designate the SFD of the frame
     *
     */
    syncword_class_t syncword_class;
    /** \brief The transmission power level measured in dBm [-39,+10]. 
     *
     * If the value specified is not supported by the driver, the nearest supported value is used instead.
     *
     */
    eirp_t eirp;
} hw_tx_cfg_t;

/** \brief A struct defining the 'metadata' that can be attached to a received packet by the PHY driver
 *
 */
typedef struct
{
    /** \brief The clock_tick of the framework timer at which the first bit of the SFD was received
     * 
     */
    timer_tick_t timestamp;
    /** \brief The crc status of the packet
     *
     * EOFF 	if the driver does not support hardware crc checking
     * FAIL 	if the CRC was not valid
     * SUCCESS	if the CRC was valid
     */
    error_t crc_status;
    /** \brief The 'RX Configuration' used to receive the packet
     *
     */
    hw_rx_cfg_t rx_cfg;

    /** \brief The link quality indicator (LQI) reported by the radio for the received packet
     *
     */
    uint8_t lqi;
    /** \brief The Received signal strength (RSSI) reported by the radio for the received packet
     *
     */
    int16_t rssi;
    //TODO: add additional fields here as needed
} hw_rx_metadata_t;

/** \brief A struct defining the 'metadata' that can be attached to a transmitted packet by the PHY driver
 *
 */
typedef struct
{
    /** \brief The clock_tick of the framework timer at which the first bit of the SFD was transmitted
     * 
     */
    timer_tick_t timestamp;
    /** \brief The 'TX Configuration' used to transmit the packet
     *
     */
    hw_tx_cfg_t	 tx_cfg;
    //TODO: add additional fields here as needed
} hw_tx_metadata_t;

/** \brief Type definition for the 'new_packet' callback function
 *
 * The new_packet_callback_t function is called by the PHY driver each time a (new) buffer is needed to store a 
 * packet. This function is supplied with the *minimum* length this buffer is required to have (the buffer 
 * may be larger) and is expected to return a pointer to the beginning of a packet buffer that is at least 
 * <length> bytes long. If such a buffer is not available, this function MUST return NULL (0x0). 
 *
 * It should be noted that this function is typically called while the packet is being received and as a 
 * result it is imperative that this function does at little processing as possible. Also, this function may 
 * be called both from a 'thread' and from an interrupt context. Implementors are advised to use atomic 
 * sections (where needed) to protect against concurrency issues.
 *
 * Once the buffer has been allocated it remains under the control of the PHY driver until it is `released' 
 * by a call to the rx_packet_callback function (see rx_packet_callback_t below).
 *
 * \param length	The length of the buffer (in bytes) required
 * \return uint8_t*	A pointer to the beginning of the byte buffer. 0x0 (NULL) if no buffer of the 
 *			required size is available
 */
typedef uint8_t* (*new_packet_callback_t)(uint8_t length);

/** \brief definition of the callback used by the PHY driver to 'release' control of a previously allocated 
 *	   packet buffer.
 *
 * This callback is called by the PHY driver when it determines that a previously allocated packet buffer is 
 * no longer needed (for instance because the RX was interrupted). If the packet WAS received correctly, 
 * control of the buffer is released through a call to rx_callback_packet_t.
 *
 * \param packet	A pointer to the buffer to release.
 * 
 */
typedef void (*release_packet_callback_t)(uint8_t* packet);

/** \brief Type definition for the rx callback function
 *
 * The rx_packet_callback_t function is called by the PHY driver every time a new packet is received. This 
 * function is supplied with a pointer to where the packet is stored and (optionally) a pointer to where the 
 * hw_rx_metadata_t of the received packet it stored.
 *
 * The first byte of the returned packet buffer contains the length of the received packet INCLUDING the 
 * length byte itself. 
 *
 * It should be noted that the `packet' pointer supplied to this callback function is *ALWAYS* a pointer that 
 * was obtained through a call to the new_packet_callback (new_packet_callback_t) function. By calling this 
 * function, the PHY driver explicitly releases control of the previously allocated buffer back to the radio stack.
 * This means that, from the perspective of the radio driver, calling rx_packet_callback_t has the same 
 * effect as calling release_packet_callback_t.
 *
 * If an hw_rx_metatadata_t struct is attached to the received packet (metadata != 0x0), this metadata struct 
 * is *ALWAYS* stored in the packet buffer, after the packet data itself. As a result, the metadata of the 
 * packet remains available as long as the packet buffer is not 'recycled' for a new packet. Whether or not a
 * metadata struct is attached to the packet can be controlled through the report_metadata field in the 
 * hw_rx_cfg_t struct.
 *
 * The first byte of the returned packet buffer contains the length of the received packet INCLUDING the 
 * length byte itself.
 *
 * \param	packet		A pointer to where the packet is stored
 * \param	metadata	A pointer to the rx_metadata of the packet. 0x0 if this data is not available
 *
 */
typedef uint8_t* (*rx_packet_callback_t)(uint8_t* packet, hw_rx_metadata_t* metadata);

/** \brief Initialise the radio driver. 
 *
 * After initialisation, the radio is in IDLE state. The RX must be explicitly enabled by a call to
 * hw_radio_set_rx(...) before any packets can be received.
 *
 * \param p_alloc			The new_packet_callback_t function to call whenever a buffer is 
 *					needed to store a new packet. Please note that this function is 
 *					called from an *interrupt* context and therefore can only do minimal 
 *					processing.
 *
 * \param p_free			The release_packet_callback_t function to call whenever an allocated 
 *					buffer is not needed any more. (A buffer can also be release by a 
 *					call to p_callback) Please note that this function is called from an 
 *					*interrupt* context and therefore can only do minimal processing.
 *
 * \param p_callback			The rx_packet_callback_t function to call whenever a packet is received
 * 					please note that this function is called from an *interrupt* context and therefore
 * 					can only do minimal processing.
 *
 * \return	error_t			SUCCESS  if the radio driver was initialised successfully
 * 							EINVAL	 if p_alloc == 0x0 or p_callback == 0x0
 * 							EALREADY if the radio driver was already initialised
 * 							FAIL	 if the radio driver could not be initialised
 */
__LINK_C error_t hw_radio_init(new_packet_callback_t p_alloc, release_packet_callback_t p_free, rx_packet_callback_t p_callback);

/** \brief Set the radio in the IDLE mode.
 *
 * When the radio is IDLE, the tranceiver is disabled to reduce energy consumption. 
 * While in this state, it is not possible to receive any packets. The tranceiver can 
 * be re-enabled by a call to hw_radio_set_rx().
 *
 * It should be noted that althoug packet reception is not possible while in IDLE mode,
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
__LINK_C error_t hw_radio_set_idle();

/** \brief Check whether or not the radio is in IDLE mode.
 *
 * Please note that if hw_radio_set_idle() was called while a transmission was still in progress, 
 * hw_radio_is_idle() will return TRUE even if the transmission is still in progress.
 *
 * \return bool		true if the radio is in idle mode, false if it is not.
 */
__LINK_C bool hw_radio_is_idle();

/** \brief Set the radio in RX mode.
 *
 * When the radio is placed in RX mode, the tranceiver is configured according to the settings in the 
 * supplied hw_tx_cfg_t struct after which it starts scanning the channel for possible packets.
 *
 * Packets that can be correctly received are passed back to the user by calling the rx_packet_callback_t
 * function supplied to the hw_radio_init function. 
 *
 * If the radio is already in RX mode when this function is called, any current packet receptions
 * are interrupted, the new settings are applied and the radio restarts the channel scanning process.
 *
 * If the radio is in TX mode when this function is called, the current packet transmission is completed 
 * before the radio is placed in RX mode.
 *
 * \param rx_cfg	A pointer the the rx settings to apply before entering RX mode
 *
 * \return error_t	SUCCESS if the radio was put in RX mode (or will be after the current TX has finished)
 *			EALREADY if the radio was already in RX mode.
 *			EINVAL if the supplied rx_cfg contains invalid parameters.
 *			EOFF if the radio is not yet initialised.
 */
__LINK_C error_t hw_radio_set_rx(hw_rx_cfg_t const* rx_cfg);

/** \brief Check whether or not the radio is in RX mode.
 *
 * Please note that if hw_radio_set_rx() was called while a transmission was still in progress, 
 * hw_radio_is_rx() will return TRUE even if the transmission is still in progress.
 *
 *  \return bool	true if the radio is in RX mode, false if not.
 *
 */
__LINK_C bool hw_radio_is_rx();

/** \brief Send a packet over the air with the specified TX settings.
 *
 * This function sends the packet pointed to by the <packet> parameter over the air using the 
 * TX settings specified by <tx_cfg>. The length of the specified packet MUST be stored in the 
 * FIRST byte of the buffer pointed to by <packet>.  Moreover the 'length' byte is regarded as 
 * an integral part of the packet which means that the specified  'length' MUST include the 
 * length byte itself.
 *
 * Optionally, a pointer to a hw_tx_metadata_t struct may be specified. If <metadata> is not 0x0, 
 * additional metadata about the transmission will be stored in pointed to hw_tx_metadata_t struct.
 *
 * If a transmission is initiated while the radio is in IDLE mode, the radio switches directly from IDLE mode to 
 * TX mode to transmit the packet. After the packet has been sent, it switches back to IDLE mode (unless 
 * hw_radio_set_rx() is called while the TX is in progress).
 *
 * If a transmission is initiated while the radio is in RX mode, the radio switches immediately to TX mode to 
 * transmit the packet. If a packet reception is in progress while this function is called, this packet is 
 * dropped. Once the packet has been sent, the radio switches back to RX mode with the original hw_rx_cfg_t 
 * settings used (unless either hw_radio_set_idle() or hw_radio_set_rx() with different rx parameters) is 
 * called while the TX is still in progress.)
 *
 * At this point, packet transmission is done synchronously. As a result, this function does not return until
 * the entire packet has been transmitted. Also, depending on the PHY driver implementation, sending a packet 
 * may be an 'atomic' operation (meaning that other interrupt events will be delayed until the packet has 
 * been transmitted).
 *
 * \param packet	A pointer to the start of the packet to be transmitted
 * \param tx_cfg	A pointer to the transmission settings to be used
 * \param metadata	A pointer to the location where to store the metadata. 0x0 if no metadata should be 
 *			kept.
 *
 * \return error_t	SUCCESS if the packet has been sent successfully
 *			EINVAL if the tx_cfg parameter contains invalid settings
 *			ESIZE if the packet is either too long or too small
 *			EOFF if the radio has not yet been initialised
 */
__LINK_C error_t hw_radio_send_packet(uint8_t* packet, hw_tx_cfg_t const* tx_cfg, hw_tx_metadata_t* metadata);

/** \brief Check whether or not the radio is currently transmitting a packet
 *
 *  \return bool	true if the radio is currently transmitting a packet, false if it is not.
 */
__LINK_C bool hw_radio_tx_busy();

/** \brief Check whether or not the radio is currently receiving a packet
 *
 *  \return bool	true if the radio is currently receiving a packet, false if it is not.
 */
__LINK_C bool hw_radio_rx_busy();


/** \brief Measure the current RSSI on the channel.
 *
 * In order to read the channel RSSI, the radio must be initialised and must already be in RX mode for a
 * certain time. (How much time depends on the specific radio used). If the radio is not in RX mode, or 
 * has not yet been in RX mode for the required amount of time the special value 'HW_RSSI_INVALID' is 
 * returned. When this value is received the caller shoud ensure that the radio is actually in RX mode and,
 * if so, schedule the RSSI to be read at a later time.
 * 
 * Measuring the RSSI is done using the 'current' RX settings of the radio and does NOT interrupt any receive
 * operation currently in progress. 
 *
 * Reading the RSSI for a different RX configuration can be done by: 
 *   -# Setting the required settings by calling hw_radio_set_rx
 *   -# Scheduling the RSSI to be read after a sufficient timeout.
 *
 * //NOTE: a function to set the rx_settings and then query the rssi in a busy wait until the rssi is valid 
 * is NOT included here *ON PURPOSE* because this would cause an infinite loop with the *SIM* environment
 *
 */
__LINK_C int16_t hw_radio_get_rssi();

/** \brief Perform a Clear Channel Assessment (CCA) and return the result.
 *
 * The radio must be initialised and must be in RX mode before a CCA can be performed. 
 * How a clear channel assessment is performed depends largely on the settings in the 
 * <rx_cfg> struct passed to the most recent call to hw_radio_rx. More specifically, the 
 * channel is deemed to be BUSY if at least one of the following conditions is met:
 *
 *   - <rx_cfg>.busy_on_rx && hw_radio_rx_busy()
 *   - <rx_cfg>.busy_on_tx && hw_radio_tx_busy()
 *   - <rx_cfg>.cca_rssi_threshold < hw_radio_get_rssi()
 *
 * If the channel is deemed to be clear 'SUCCESS' is returned. If the channel is deemed top be busy
 * 'EBUSY' is returned. If the CCA could not be performed, an appropriate error value is returned
 *
 * \return	SUCCESS if the CCA could be performed and the channel is clear
 *		EBUSY	if the CCA could be performed and the channel is not clear
 *		EOFF	if the radio is not in RX mode
 *		ERETRY	if the radio is in RX mode, but the RSSI value is not (yet) valid.
 *			in this case the CCA check should be rescheduled for a later time.
 */
__LINK_C error_t hw_radio_cca();

#endif //__HW_RADIO_H_

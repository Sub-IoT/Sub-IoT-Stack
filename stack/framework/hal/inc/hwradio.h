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
/* \brief Type definition for the rx callback function
 *
 * The rx_packet_callback_t function is supplied with a pointer to where
 * the received packet is stored (and it's length) when this call returns it is expected to
 * return a pointer to the locatio where the next packet may be stored. This may be the same pointer
 * that was originally provided to the function or another pointer alltogether. The only requirement is that the
 * returned pointer points to a data area that is large enough to store a maximum-sized packet
 *
 * \param	packet		A pointer to where the packet is stored
 * \param	length 		The length of the packet
 *
 * \return	uint8_t*	A pointer to where the next packet may be stored
 *
 */
typedef uint8_t* (*rx_packet_callback_t)(uint8_t* packet, uint8_t length);

/* \brief Initialise the radio driver
 *
 * \param initial_rx_buffer	A pointer to where the first received packet may be stored
 * \param callback			The rx_packet_callback_t function to call whenever a packet is received
 * 							please note that this function is called from an *interrupt* context and therefore
 * 							can only do minimal processing
 *
 * \return	error_t			SUCCESS  if the radio driver was initialised successfully
 * 							EINVAL	 if initial_rx_buffer == 0x0 or callback == 0x0
 * 							EALREADY if the radio driver was already initialised
 * 							FAIL	 if the radio driver could not be initialised
 */
__LINK_C error_t hw_radio_init(uint8_t* initial_rx_buffer, rx_packet_callback_t callback);

/* \brief Enable or disable the radio
 *
 * If the radio is enabled, it will continuously scan the wireless channel for new packets.
 * No packets can be received if the radio is disabled.
 *
 * \param enabled	true if the radio is to be enabled, false if it must be disabled
 * \return	error_t	SUCCESS  if the radio driver was successfully enabled or disabled
 * 							EALREADY if the radio was already in the requested state
 */
__LINK_C error_t hw_radio_setenabled(bool enabled);

/* \brief Send a packet over the wireless channel
 *
 * Please note: for now the length of the packet needs to be specified.
 * In future this will be derived from the supplied buffer, but for the
 * sim interface this suffices for now
 *
 * \param buffer	pointer to the location of the packet to send
 * \param length 	length of the packet to send
 *
 * \return	error_t	SUCCESS  if the radio driver sent successfully
 *
 */
__LINK_C error_t hw_radio_send_packet(uint8_t* buffer, uint8_t length);


#endif //__HW_RADIO_H_

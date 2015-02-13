#include "hwradio.h"
#include "HalModule.h"

__LINK_C error_t hw_radio_init(uint8_t* initial_rx_buffer, rx_packet_callback_t callback)
{
	return HalModule::getActiveModule()->hw_radio_init(initial_rx_buffer, callback);
}

/* \brief Enable or disable the radio
 *
 * If the radio is enabled, it will continuously scan the wireless channel for new packets.
 * No packets can be received if the radio is disabled.
 *
 * \param enabled	true if the radio is to be enabled, false if it must be disabled
 * \return	error_t	SUCCESS  if the radio driver was successfully enabled or disabled
 * 							EALREADY if the radio was already in the requested state
 */
__LINK_C error_t hw_radio_setenabled(bool enabled)
{
	return HalModule::getActiveModule()->hw_radio_setenabled(enabled);
}

__LINK_C error_t hw_radio_send_packet(uint8_t* buffer, uint8_t length)
{
	return HalModule::getActiveModule()->hw_radio_send_packet(buffer, length);
}

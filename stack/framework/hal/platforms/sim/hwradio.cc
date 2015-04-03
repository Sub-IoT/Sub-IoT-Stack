//
// OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
// lowpower wireless sensor communication
//
// Copyright 2015 University of Antwerp
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

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

/*
 *  Created on: Dec 06, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 *  	armin@otheruse.nl
 */

#include "cc1101_phy.h"
#include "cc1101_core.h"

/*
0x00	Associated to the RX FIFO: Asserts when RX FIFO is filled at or above the RX FIFO threshold. De-asserts when RX FIFO is drained below the same threshold.
0x01	Associated to the RX FIFO: Asserts when RX FIFO is filled at or above the RX FIFO threshold or the end of packet is reached. De-asserts when the RX FIFO is empty.
0x02	Associated to the TX FIFO: Asserts when the TX FIFO is filled at or above the TX FIFO threshold. De-asserts when the TX FIFO is below the same threshold.
0x03	Associated to the TX FIFO: Asserts when TX FIFO is full. De-asserts when the TX FIFO is drained below the TX FIFO threshold.
0x04	Asserts when the RX FIFO has overflowed. De-asserts when the FIFO has been flushed.
0x05	Asserts when the TX FIFO has underflowed. De-asserts when the FIFO is flushed.
0x06	Asserts when sync word has been sent / received, and de-asserts at the end of the packet. In RX, the pin will also de-assert when a packet is discarded due to address or maximum length filtering or when the radio enters RXFIFO_OVERFLOW state. In TX the pin will de-assert if the TX FIFO underflows.
0x07	Asserts when a packet has been received with CRC OK. De-asserts when the first byte is read from the RX FIFO.
0x08	Preamble Quality Reached. Asserts when the PQI is above the programmed PQT value. De-asserted when the chip re- enters RX state (MARCSTATE=0x0D) or the PQI gets below the programmed PQT value.
0x09	Clear channel assessment. High when RSSI level is below threshold (dependent on the current CCA_MODE setting).
0x0E	Carrier sense. High if RSSI level is above threshold. Cleared when entering IDLE mode.
0x24	WOR_EVNT0.
0x25	WOR_EVNT1.
0x29	CHIP_RDYn.
0x2B	XOSC_STABLE
 */
InterruptHandlerDescriptor interrupt_table[6] = {
		{.gdoSetting = 0x00, .edge = GDOEdgeRising, .handler = rx_data_isr},
		{.gdoSetting = 0x02, .edge = GDOEdgeFalling, .handler = tx_data_isr},
		{.gdoSetting = 0x05, .edge = GDOEdgeRising, .handler = end_of_packet_isr},
		{.gdoSetting = 0x06, .edge = GDOEdgeFalling, .handler = end_of_packet_isr},
		{.gdoSetting = 0x04, .edge = GDOEdgeRising, .handler = rx_fifo_overflow_isr},
		{.handler = 0},
};

void CC1101_ISR(GDOLine gdo)
{
	uint8_t gdo_setting = ReadSingleReg(gdo);
	uint8_t index = 0;
	InterruptHandlerDescriptor descriptor;
	do {
		descriptor = interrupt_table[index];
		if ((gdo_setting & 0x7f) == (descriptor.gdoSetting | descriptor.edge)) {
			descriptor.handler();
			break;
		}
		index++;
	}
	while (descriptor.handler != 0);
}

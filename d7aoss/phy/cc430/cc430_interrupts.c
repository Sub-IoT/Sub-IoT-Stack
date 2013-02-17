/*
 *  Created on: Dec 06, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 */

#include "cc430_phy.h"

// Interrupt branch table
InterruptHandler interrupt_table[34] = {
	//Rising Edges
	no_interrupt_isr,        	// No RF core interrupt pending
	rxtx_finish_isr,			// RFIFG0 - Timeout
	no_interrupt_isr,           // RFIFG1 - RSSI_VALID
	no_interrupt_isr,           // RFIFG2
	rx_data_isr,				// RFIFG3 - RX FIFO filled or above the RX FIFO threshold
	no_interrupt_isr,           // RFIFG4 - RX FIFO filled or above the RX FIFO threshold or end of packet is reached (Do not use for eop! Will not reset until rxfifo emptied)
	no_interrupt_isr,		    // RFIFG5 - TX FIFO filled or above the TX FIFO threshold
	no_interrupt_isr,			// RFIFG6 - TX FIFO full
	rxtx_finish_isr, 			// RFIFG7 - RX FIFO overflowed
	rxtx_finish_isr,			// RFIFG8 - TX FIFO underflowed
	no_interrupt_isr,			// RFIFG9 - Sync word sent or received
	no_interrupt_isr,           // RFIFG10 - Packet received with CRC OK
	no_interrupt_isr,           // RFIFG11 - Preamble quality reached (PQI) is above programmed PQT value
	no_interrupt_isr,           // RFIFG12 - Clear channel assessment when RSSI level is below threshold
	no_interrupt_isr,           // RFIFG13 - Carrier sense. RSSI level is above threshold
	no_interrupt_isr,           // RFIFG14 - WOR event 0
	no_interrupt_isr,       	// RFIFG15 - WOR event 1

	//Falling Edges
	no_interrupt_isr,        	// No RF core interrupt pending
	no_interrupt_isr,			// RFIFG0 TODO: timeout not implemented yet
	no_interrupt_isr,           // RFIFG1 - RSSI_VALID
	no_interrupt_isr,           // RFIFG2 -
	no_interrupt_isr,			// RFIFG3 - RX FIFO drained below RX FIFO threshold
	no_interrupt_isr,           // RFIFG4 - RX FIFO empty
	tx_data_isr,				// RFIFG5 - TX FIFO below TX FIFO threshold
	no_interrupt_isr,			// RFIFG6 - TX FIFO below TX FIFO threshold
	no_interrupt_isr,           // RFIFG7 - RX FIFO flushed
	no_interrupt_isr,			// RFIFG8 - TX FIFO flushed
	end_of_packet_isr,			// RFIFG9 - End of packet or in RX when optional address check fails or RX FIFO overflows or in TX when TX FIFO underflows
	no_interrupt_isr,           // RFIFG10 - First byte read from RX FIFO
	no_interrupt_isr,           // RFIFG11 - (LPW)
	no_interrupt_isr,           // RFIFG12 - RSSI level is above threshold
	no_interrupt_isr,           // RFIFG13 - RSSI level is below threshold
	no_interrupt_isr,           // RFIFG14 - WOR event 0 + 1 ACLK
	no_interrupt_isr,       	// RFIFG15 - RF oscillator stable or next WOR event0 triggered
};

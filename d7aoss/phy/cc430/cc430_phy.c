/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 *     	glenn.ergeerts@uantwerpen.be
 *     	maarten.weyn@uantwerpen.be
 *		alexanderhoet@gmail.com
 *
 */

#include <msp430.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../../hal/system.h"
#include "../../phy/phy.h"
#include "../../phy/fec.h"
#include "cc430_phy.h"
#include "cc430_registers.h"
#include "../../framework/log.h"
#include "../../framework/queue.h"
#include "../../framework/timer.h"

#include "rf1a.h"

/*
 * Variables
 */
extern RF_SETTINGS rfSettings;
extern InterruptHandler interrupt_table[34];

static RadioState state;

static uint8_t packetLength;
//uint8_t* bufferPosition;
static int16_t remainingBytes;

static bool fec;
static uint8_t frequency_band;
static uint8_t channel_center_freq_index;
static uint8_t channel_bandwidth_index;
static uint8_t preamble_size;
static uint16_t sync_word;

static bool init_and_close_radio = true;
static bool phy_initialized = false;

static uint8_t previous_spectrum_id[2] = {0xFF, 0xFF};
static uint8_t previous_sync_word_class = 0xFF;

static phy_rx_data_t rx_data;

static phy_tx_cfg_t last_tx_cfg;

// from -35 to 10 dbm in steps of +- 1
static const int8_t eirp_values[46] = {0x1,0x2,0x2,0x3,0x3,0x4,0x4,0x5,0x6,0x6,0x7,0x8,0x9,0xA,0xC,0xE,0xF,0x19,0x1A,0x1B,0x1D,0x1E,0x24,0x33,0x25,0x34,0x26,0x28,0x29,0x2A,0x2B,0x2D,0x2F,0x3C,0x3F,0x60,0x8D,0xCF,0x8A,0x87,0x84,0x81,0xC8,0xC5,0xC2,0xC0};


// *****************************************************************************
// @fn          ReadBurstRegToQueue
// @brief       Read multiple bytes of  the radio registers and put it in a queu
// @param       unsigned char addr      Beginning address of burst read
// @param       queue_t *buffer   		The target queue
// @param       unsigned char count     Number of bytes to be read
// @return      none
// *****************************************************************************
/*static void ReadBurstRegToQueue(uint8_t addr, queue_t* buffer, uint8_t count)
{
	uint8_t i;
	uint16_t int_state;
	ENTER_CRITICAL_SECTION(int_state);

	RADIO_INST_READY_WAIT();
	RF1AINSTR1B = RF_REGRD | addr;

	for (i = 0; i < (count-1); i++) {
		RADIO_DOUT_READY_WAIT();
		buffer->rear[1+i] = RF1ADOUT1B;
	}

	buffer->rear[1+i] = RF1ADOUT1B;

	buffer->length += count;
	buffer->rear += count;

	EXIT_CRITICAL_SECTION(int_state);
}
*/

// *****************************************************************************
// @fn          WriteBurstRegFromQueue
// @brief       Write multiple bytes to the radio registers from a queue
// @param       unsigned char addr      Beginning address of burst write
// @param       unsigned char *buffer   Pointer to data table
// @param       unsigned char count     Number of bytes to be written
// @return      none
// *****************************************************************************
void WriteBurstRegFromQueue(uint8_t addr, queue_t* buffer, uint8_t count)
{
	uint8_t i;
	uint16_t int_state;

	ENTER_CRITICAL_SECTION(int_state);

	RADIO_INST_READY_WAIT();
	RF1AINSTRW = ((RF_REGWR | addr) << 8) + buffer->front[0];//queue_pop_u8(buffer);

	for (i = 1; i < count; i++) {
		RADIO_DIN_READY_WAIT();
		RF1ADINB = buffer->front[i]; //queue_pop_u8(buffer);
	}

	buffer->front += count;
	buffer->length -= count;

	EXIT_CRITICAL_SECTION(int_state);
}

 /*
 * Phy implementation functions
 */
void phy_init(void)
{
	//Set radio state
	state = Idle;

	//Reset the radio core
	ResetRadioCore();

	//Write configuration
	WriteRfSettings(&rfSettings);

	last_tx_cfg.eirp=0;
	last_tx_cfg.spectrum_id[0] = 0;
	last_tx_cfg.spectrum_id[1] = 0;
	last_tx_cfg.sync_word_class = 0;

}

void phy_idle(void)
{
	if (state != Idle)
	{
		rxtx_finish_isr();
		state = Idle;
	}
}

bool phy_translate_and_set_settings(uint8_t spectrum_id[2], uint8_t sync_word_class)
{
	if (!memcmp(previous_spectrum_id, spectrum_id, 2) && previous_sync_word_class == sync_word_class)
		return true;

	Strobe(RF_SIDLE);

	if(!phy_translate_settings(spectrum_id, sync_word_class, &fec, &frequency_band, &channel_center_freq_index, &channel_bandwidth_index, &preamble_size, &sync_word))
	{
		#ifdef LOG_PHY_ENABLED
		log_print_stack_string(LOG_PHY, "PHY Cannot translate settings");
		#endif

		return false;
	}

	set_channel(frequency_band, channel_center_freq_index, channel_bandwidth_index);
	set_preamble_size(preamble_size);
	set_sync_word(sync_word);

	memcpy(previous_spectrum_id, spectrum_id, 2);
	previous_sync_word_class = sync_word_class;

	return true;
}


// configure using cfg
extern bool phy_set_tx(phy_tx_cfg_t* cfg)
{
	//General configuration
	phy_translate_and_set_settings(cfg->spectrum_id, cfg->sync_word_class);

	set_eirp(cfg->eirp);

	//TODO Return error if fec not enabled but requested

	//TODO: only enable if it was dissabled previously
	//set_data_whitening(true);

	return true;
}
// send data
extern bool phy_tx_data(phy_tx_cfg_t* cfg)
{

	//TODO Return error if fec not enabled but requested
	#ifdef D7_PHY_USE_FEC
	if (fec) {
		//Initialize fec encoding
		fec_init_encode(cfg->data);

		//Configure length settings
		set_length_infinite(true);
		fec_set_length(cfg->length);
		remainingBytes = ((cfg->length & 0xFE) + 2) << 1;
		WriteSingleReg(PKTLEN, (uint8_t)(remainingBytes & 0x00FF));
	} else {
	#endif

		//Set buffer position
		//bufferPosition = cfg->data;

		//Configure length settings
		set_length_infinite(false);
		remainingBytes = tx_queue.length;
		WriteSingleReg(PKTLEN, (uint8_t)remainingBytes);
	#ifdef D7_PHY_USE_FEC
	}
	#endif

	//Write initial data to txfifo
	tx_data_isr();

	//Configure txfifo threshold
	WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_17_48);

	//Enable interrupts
	RF1AIES = RFIFG_FLANK_TXBelowThresh | RFIFG_FLANK_TXUnderflow | RFIFG_FLANK_EndOfPacket;
	RF1AIFG = 0;
	RF1AIE  = RFIFG_FLAG_TXBelowThresh | RFIFG_FLAG_TXUnderflow | RFIFG_FLAG_EndOfPacket;

	//Start transmitting
	Strobe(RF_STX);

	return true;
}

bool phy_init_tx()
{
	RadioState current_state = get_radiostate();
	if (current_state != Idle && current_state != Transmit)
	{
		#ifdef LOG_PHY_ENABLED
		log_print_stack_string(LOG_PHY, "PHY radio not idle");
		#endif

		return false;
	}

	//Set radio state
	state = Transmit;

	//Go to idle and flush the txfifo
	Strobe(RF_SIDLE);
	Strobe(RF_SFTX);

	return true;
}

bool phy_tx(phy_tx_cfg_t* cfg)
{
	if (init_and_close_radio || !phy_initialized)
	{
		if (!phy_init_tx())
		{
			return false;
		}

		//if (last_tx_cfg.spectrum_id != cfg->spectrum_id || last_tx_cfg.sync_word_class != cfg->sync_word_class || last_tx_cfg.eirp != cfg->eirp)
		if (memcmp(&last_tx_cfg, cfg, 3) != 0)
		{
			if (!phy_set_tx(cfg))
				return false;
			memcpy(&last_tx_cfg, cfg, 3);
		}

		phy_initialized = true;
	}

	return phy_tx_data(cfg);
}

bool phy_rx(phy_rx_cfg_t* cfg)
{
	#ifdef LOG_PHY_ENABLED
	log_print_stack_string(LOG_PHY, "phy_rx");
	#endif

	RadioState current_state = get_radiostate();
	if(current_state != Idle && current_state != Receive)
	{
		#ifdef LOG_PHY_ENABLED
		log_print_stack_string(LOG_PHY, "PHY Cannot RX, PHY not idle, retrying");
		#endif

//		timer_wait_ms(100);
//
//		if(current_state != Idle && current_state != Receive)
//		{
//			//#ifdef LOG_PHY_ENABLED
//			log_print_stack_string(LOG_PHY, "PHY Cannot RX, PHY not idle, stopping");
//			//#endif
			return false;
//		}

		//#ifdef LOG_PHY_ENABLED
		//log_print_stack_string(LOG_PHY, "ok PHY idle");
		//#endif
	}

	//Set radio state
	state = Receive;

	//Flush the txfifo
	Strobe(RF_SIDLE);
	Strobe(RF_SFRX);

	//Set configuration

	if (!phy_translate_and_set_settings(cfg->spectrum_id, cfg->sync_word_class))
		return false;

	rx_data.spectrum_id[0] = cfg->spectrum_id[0];
	rx_data.spectrum_id[1] = cfg->spectrum_id[1];
	rx_data.sync_word_class = cfg->sync_word_class;

	set_timeout(cfg->timeout);

//TODO Return error if fec not enabled but requested
#ifdef D7_PHY_USE_FEC
	if (fec) {
		//Initialize fec encoding
		fec_init_decode(buffer);

		//Configure length settings
		set_length_infinite(true);

		if(cfg->length == 0)
		{
			packetLength = 0;
			remainingBytes = 0;
			WriteSingleReg(PKTLEN, 0xFF);
			WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_61_4);
		} else {
			fec_set_length(cfg->length);
			packetLength = ((cfg->length & 0xFE) + 2) << 1;
			remainingBytes = packetLength;
			WriteSingleReg(PKTLEN, (uint8_t)(packetLength & 0x00FF));
			WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_17_48);
		}
	} else {
#endif
		//Enable hardware data whitening
		//todo: only enable when dissabled previously
		//set_data_whitening(true);

		//Set buffer position
		//bufferPosition = buffer;
		queue_clear(&rx_queue);

		//Configure length settings and txfifo threshold
		set_length_infinite(false);

		if(cfg->length == 0)
		{
			packetLength = 0;
			remainingBytes = 0;
			WriteSingleReg(PKTLEN, 0xFF);
			WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_61_4);
		} else {
			packetLength = cfg->length;
			remainingBytes = packetLength;
			WriteSingleReg(PKTLEN, packetLength);
			WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_17_48);
		}
#ifdef D7_PHY_USE_FEC
	}
#endif

	//TODO: set minimum sync word rss to scan minimum energy
	//Enable interrupts
	RF1AIES = RFIFG_FLANK_IOCFG0 | RFIFG_FLANK_RXFilled | RFIFG_FLANK_RXOverflow | RFIFG_FLANK_EndOfPacket;
	RF1AIFG = 0;
	RF1AIE  = RFIFG_FLAG_IOCFG0 | RFIFG_FLAG_RXFilled | RFIFG_FLAG_RXOverflow | RFIFG_FLAG_EndOfPacket;

	//Start receiving
	Strobe(RF_SRX);

	return true;
}

bool phy_is_rx_in_progress(void)
{
	return (state == Receive);
}

bool phy_is_tx_in_progress(void)
{
	return (state == Transmit);
}

extern int16_t phy_get_rssi(uint8_t spectrum_id[2], uint8_t sync_word_class)
{
	uint8_t rssi_raw = 0;

	if (!phy_translate_and_set_settings(spectrum_id, sync_word_class))
		return false;


    //Start receiving
    Strobe(RF_SRX);

    //Wait until RSSI valid
    while((RF1AIN & RFIFG_FLAG_IOCFG1) == 0);

    rssi_raw = ReadSingleReg(RSSI);
    rxtx_finish_isr();

    return calculate_rssi(rssi_raw);
}

/*
 * Interrupt functions
 */
#pragma vector=CC1101_VECTOR
__interrupt void CC1101_ISR (void)
{
	uint16_t isr_vector = RF1AIV >> 1;
	uint16_t edge = (1 << (isr_vector - 1)) & RF1AIES;
	if(edge) isr_vector += 0x11;

//	#ifdef LOG_PHY_ENABLED
//		uart_transmit_data(0xDD);
//		uart_transmit_data(LOG_TYPE_STRING);
//		uart_transmit_data(13);
//		uart_transmit_message("CC1101_ISR ", 11);
//		uart_transmit_data(48 + (isr_vector / 10));
//		uart_transmit_data(48 + (isr_vector - (10 * (isr_vector / 10))));
//	#endif

	interrupt_table[isr_vector]();
	LPM4_EXIT;  //Todo should system call be made here?
}

void no_interrupt_isr() { }

void end_of_packet_isr()
{
	if (state == Receive) {
		rx_data_isr();
		rxtx_finish_isr(); // TODO: should this be called by DLL?
		state = Idle;

		#ifdef LOG_PHY_ENABLED
		log_print_stack_string(LOG_PHY, "PHY RX End of Packet");
		#endif

		if(phy_rx_callback != NULL)
			phy_rx_callback(&rx_data);
	} else if (state == Transmit) {
		rxtx_finish_isr();
		state = Idle;

		#ifdef LOG_PHY_ENABLED
		log_print_stack_string(LOG_PHY, "PHY TX End of Packet");
		#endif
		if(phy_tx_callback != NULL)
			phy_tx_callback();
	} else {
		rxtx_finish_isr();
		state = Idle;
	}
}

void tx_data_isr()
{
	if (remainingBytes == 0)
		return;

	//Calculate number of free bytes in TXFIFO
	uint8_t txBytes = 64 - ReadSingleReg(TXBYTES);

#ifdef D7_PHY_USE_FEC
	if(fec)
	{
		uint8_t fecbuffer[4];

		//If remaining bytes is equal or less than 255 - fifo size, set length to fixed
		if(remainingBytes < 192)
			set_length_infinite(false);

		while (txBytes >= 4) {
			//Get encoded data, stop when no more data available
			if(fec_encode(fecbuffer) == false)
				break;

			//Write data to tx fifo
			WriteBurstReg(RF_TXFIFOWR, fecbuffer, 4);
			remainingBytes -= 4;
			txBytes -= 4;
		}
	} else {
#endif
		//Limit number of bytes to remaining bytes
		if(txBytes > remainingBytes)
			txBytes = remainingBytes;

		//Write data to tx fifo
		#ifdef LOG_PHY_ENABLED
		log_print_stack_string(LOG_PHY, "Data to TX Fifo:");
		log_print_data(tx_queue.front, txBytes);
		#endif

		WriteBurstRegFromQueue(RF_TXFIFOWR, &tx_queue, txBytes);

		remainingBytes -= txBytes;
#ifdef D7_PHY_USE_FEC
	}
#endif
}

void rx_data_isr()
{
	#ifdef LOG_PHY_ENABLED
	log_print_stack_string(LOG_PHY, "rx_data_isr 0");
	#endif

	//Read number of bytes in RXFIFO
	volatile uint8_t rxBytes = ReadSingleReg(RXBYTES);

#ifdef D7_PHY_USE_FEC
	if(fec)
	{
		uint8_t fecbuffer[4];

	    //If length is not set get the length from RXFIFO and set PKTLEN
		if (packetLength == 0) {
			ReadBurstReg(RXFIFO, fecbuffer, 4);
			fec_decode(fecbuffer);
			fec_set_length(*buffer);
			packetLength = ((*buffer & 0xFE) + 2) << 1;
			remainingBytes = packetLength - 4;
			WriteSingleReg(PKTLEN, (uint8_t)(packetLength & 0x00FF));
			WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_17_48);
			rxBytes -= 4;
		}

		//If remaining bytes is equal or less than 255 - fifo size, set length to fixed
		if(remainingBytes < 192)
			set_length_infinite(false);

		//Read data from buffer and decode
		while (rxBytes >= 4) {
			ReadBurstReg(RXFIFO, fecbuffer, 4);

			if(fec_decode(fecbuffer) == false)
				break;

			remainingBytes -= 4;
			rxBytes -= 4;
		}
	} else {
#endif
    //If length is not set get the length from RXFIFO and set PKTLEN
	if (packetLength == 0) {
		packetLength = ReadSingleReg(RXFIFO);
		WriteSingleReg(PKTLEN, packetLength);
		WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_17_48);
		remainingBytes = packetLength - 1;
		queue_push_u8(&rx_queue, packetLength);
		rxBytes--;
	#ifdef LOG_PHY_ENABLED
		log_print_stack_string(LOG_PHY, "rx_data_isr getting packetLength");
	#endif
	}

	//Never read the entire buffer as long as more data is going to be received
    if (remainingBytes > rxBytes) {
    	rxBytes--;
    } else {
    	rxBytes = remainingBytes;
    }


//	uint16_t int_state;
//    ENTER_CRITICAL_SECTION(int_state);
    //Read data from buffer
	#ifdef LOG_PHY_ENABLED
		log_print_stack_string(LOG_PHY, "Getting %d byte from RXFifo", rxBytes);
	#endif
	//ReadBurstRegToQueue(RXFIFO, &rx_queue, rxBytes);
	ReadBurstReg(RXFIFO, &rx_queue.rear[1], rxBytes);

	rx_queue.length += rxBytes;
	rx_queue.rear += rxBytes;

	remainingBytes -= rxBytes;

#ifdef LOG_PHY_ENABLED
	log_print_stack_string(LOG_PHY, "%d bytes remaining", remainingBytes);
#endif
#ifdef D7_PHY_USE_FEC
	}
#endif

    //When all data has been received read rssi and lqi value and set packetreceived flag
    if(remainingBytes <= 0)
    {
    	rx_data.rssi = calculate_rssi(ReadSingleReg(RSSI));
    	rx_data.lqi = ReadSingleReg(LQI);
		rx_data.length = *rx_queue.front;
		rx_data.data = rx_queue.front;
		#ifdef LOG_PHY_ENABLED
		log_print_stack_string(LOG_PHY, "rx_data_isr packet received");
		log_print_data(rx_data.data, rx_data.length);
		log_phy_rx_res(&rx_data);
		#endif
    }

    //EXIT_CRITICAL_SECTION(int_state);

	#ifdef LOG_PHY_ENABLED
    log_print_stack_string(LOG_PHY, "rx_data_isr 1");
	#endif
}

void rx_timeout_isr()
{
	rxtx_finish_isr();
	if(phy_rx_callback != NULL)
		phy_rx_callback(NULL);
}

void rx_fifo_overflow_isr()
{
	#ifdef LOG_PHY_ENABLED
	log_print_stack_string(LOG_PHY, "rx_fifo_overflow");
	#endif

	rxtx_finish_isr();
	if(phy_rx_callback != NULL)
		phy_rx_callback(NULL);
}

void rxtx_finish_isr()
{
	//Disable interrupts
	RF1AIE  = 0;
	RF1AIFG = 0;

	if (init_and_close_radio)
	{
		//Flush FIFOs and go to sleep
		Strobe(RF_SIDLE);
		Strobe(RF_SFRX);
		Strobe(RF_SFTX);
		Strobe(RF_SPWD);
	}
}

/*
 * Private functions
 */
RadioState get_radiostate(void)
{
	uint8_t state = Strobe(RF_SNOP) >> 4;

	if(state > 7)
		return Idle;

	return (RadioState)state;
}

void set_channel(uint8_t frequency_band, uint8_t channel_center_freq_index, uint8_t channel_bandwith_index)
{
	//Set channel center frequency
	#ifdef LOG_PHY_ENABLED
	log_print_stack_string(LOG_PHY, "Set channel freq index: %d", channel_center_freq_index);
	#endif

	WriteSingleReg(CHANNR, channel_center_freq_index);

	//Set channel bandwidth, modulation and symbol rate

	#ifdef LOG_PHY_ENABLED
	log_print_stack_string(LOG_PHY, "Set channel bandwidth index: %d", channel_bandwith_index);
	#endif
	switch(channel_bandwith_index)
	{
	case 0:
		WriteSingleReg(MDMCFG3, RADIO_MDMCFG3_DRATE_M(131));
		WriteSingleReg(MDMCFG4, (RADIO_MDMCFG4_CHANBW_E(3) | RADIO_MDMCFG4_CHANBW_M(0) | RADIO_MDMCFG4_DRATE_E(8)));
		WriteSingleReg(DEVIATN, (RADIO_DEVIATN_E(0) | RADIO_DEVIATN_M(16)));
		break;
	case 1:
		WriteSingleReg(MDMCFG3, RADIO_MDMCFG3_DRATE_M(24));
		WriteSingleReg(MDMCFG4, (RADIO_MDMCFG4_CHANBW_E(2) | RADIO_MDMCFG4_CHANBW_M(0) | RADIO_MDMCFG4_DRATE_E(11)));
		WriteSingleReg(DEVIATN, (RADIO_DEVIATN_E(5) | RADIO_DEVIATN_M(0)));
		break;
	case 2:
		WriteSingleReg(MDMCFG3, RADIO_MDMCFG3_DRATE_M(248));
		WriteSingleReg(MDMCFG4, (RADIO_MDMCFG4_CHANBW_E(1) | RADIO_MDMCFG4_CHANBW_M(0) | RADIO_MDMCFG4_DRATE_E(12)));
		WriteSingleReg(DEVIATN, (RADIO_DEVIATN_E(5) | RADIO_DEVIATN_M(0)));
		break;
//	case 3:
//		WriteSingleReg(MDMCFG3, RADIO_MDMCFG3_DRATE_M(248));
//		WriteSingleReg(MDMCFG4, (RADIO_MDMCFG4_CHANBW_E(0) | RADIO_MDMCFG4_CHANBW_M(1) | RADIO_MDMCFG4_DRATE_E(12)));
//		WriteSingleReg(DEVIATN, (RADIO_DEVIATN_E(5) | RADIO_DEVIATN_M(0)));
//		break;
	}

	#ifdef LOG_PHY_ENABLED
	log_print_stack_string(LOG_PHY, "Set frequency band: %d", frequency_band);
	#endif

	/*
	switch(frequency_band)
		{
		case 0:
			WriteSingleReg(RADIO_FREQ2, (uint8_t)(RADIO_FREQ_433>>16 & 0xFF));
			WriteSingleReg(RADIO_FREQ1, (uint8_t)(RADIO_FREQ_433>>8 & 0xFF));
			WriteSingleReg(RADIO_FREQ0, (uint8_t)(RADIO_FREQ_433 & 0xFF));
			break;
		case 1:
			WriteSingleReg(RADIO_FREQ2, (uint8_t)(RADIO_FREQ_868>>16 & 0xFF));
			WriteSingleReg(RADIO_FREQ1, (uint8_t)(RADIO_FREQ_868>>8 & 0xFF));
			WriteSingleReg(RADIO_FREQ0, (uint8_t)(RADIO_FREQ_868 & 0xFF));
			break;
		case 2:
			WriteSingleReg(RADIO_FREQ2, (uint8_t)(RADIO_FREQ_915>>16 & 0xFF));
			WriteSingleReg(RADIO_FREQ1, (uint8_t)(RADIO_FREQ_915>>8 & 0xFF));
			WriteSingleReg(RADIO_FREQ0, (uint8_t)(RADIO_FREQ_915 & 0xFF));
			break;
		}
	*/

	// is this the right place?
	Strobe(RF_SCAL);
}

void set_sync_word(uint16_t sync_word)
{
	WriteSingleReg(SYNC1, (uint8_t)(sync_word >> 8));
	WriteSingleReg(SYNC0, (uint8_t)(sync_word & 0x00FF));
}

void set_preamble_size(uint8_t preamble_size)
{
	uint8_t mdmcfg1 = ReadSingleReg(MDMCFG1) & 0x03;

	if(preamble_size >= 24)
		mdmcfg1 |= 0x70;
	else if(preamble_size >= 16)
		mdmcfg1 |= 0x60;
	else if(preamble_size >= 12)
		mdmcfg1 |= 0x50;
	else if(preamble_size >= 8)
		mdmcfg1 |= 0x40;
	else if(preamble_size >= 6)
		mdmcfg1 |= 0x30;
	else if(preamble_size >= 4)
		mdmcfg1 |= 0x20;
	else if(preamble_size >= 3)
		mdmcfg1 |= 0x10;

	WriteSingleReg(MDMCFG1, mdmcfg1);
}

void set_data_whitening(bool white_data)
{
	uint8_t pktctrl0 = ReadSingleReg(PKTCTRL0) & 0xBF;

	if (white_data)
		pktctrl0 |= RADIO_PKTCTRL0_WHITE_DATA;

	WriteSingleReg(PKTCTRL0, pktctrl0);
}

void set_length_infinite(bool infinite)
{
	uint8_t pktctrl0 = ReadSingleReg(PKTCTRL0) & 0xFC;

	if (infinite)
		pktctrl0 |= RADIO_PKTCTRL0_LENGTH_INF;

	WriteSingleReg(PKTCTRL0, pktctrl0);
}

void set_timeout(uint16_t timeout)
{
	if (timeout == 0) {
		WriteSingleReg(WORCTRL, RADIO_WORCTRL_ALCK_PD);
		WriteSingleReg(MCSM2, RADIO_MCSM2_RX_TIME(7));
	} else {
		WriteSingleReg(WORCTRL, RADIO_WORCTRL_WOR_RES_32ms);
		WriteSingleReg(MCSM2, RADIO_MCSM2_RX_TIME(0));
		WriteSingleReg(WOREVT0, timeout & 0x00FF);
		WriteSingleReg(WOREVT1, timeout >> 8);
	}
}

void set_eirp(int8_t eirp)
{
	int8_t eirp_index = 0;
	uint8_t i;
	uint8_t  pa_table[8];

	if (eirp >= 10)
	{
		eirp_index = 45;
	} else if (eirp > -35) {
		eirp_index = eirp + 35;
	}

	 for(i=7; (i<8) && (eirp_index<46); i--, eirp_index-=3) {
		 pa_table[i] = eirp_values[eirp_index];
	 }

	 WriteBurstPATable(&pa_table[i], (uint8_t)(8-i));

	 WriteSingleReg(FREND0, RADIO_FREND0_LODIV_BUF_CURRENT_TX(1) | RADIO_FREND0_PA_POWER((uint8_t)(7-i)));

}

int16_t calculate_rssi(int8_t rssi_raw)
{
	// CC430 RSSI is 0.5 dBm units, signed byte
    int16_t rssi = (int16_t)rssi_raw;		//Convert to signed 16 bit
    rssi += 128;                      		//Make it positive...
    rssi >>= 1;                        		//...So division to 1 dBm units can be a shift...
    rssi -= 64 + RSSI_OFFSET;     			// ...and then rescale it, including offset

    return rssi;
}

//void dissable_autocalibration()
//{
//	WriteSingleReg(MCSM0, RADIO_MCSM0_FS_AUTOCAL_NEVER);
//}
//
//void enable_autocalibration()
//{
//	WriteSingleReg(MCSM0, RADIO_MCSM0_FS_AUTOCAL_4THIDLE);
//}
//
//void manual_calibration()
//{
//	Strobe(RF_SCAL);
//}

void phy_keep_radio_on(bool status)
{
	if (status)
	{
		if (!init_and_close_radio)
			return;

		WriteSingleReg(MCSM1, RADIO_MCSM1_CCA_RSSILOWRX | RADIO_MCSM1_RXOFF_MODE_RX | RADIO_MCSM1_TXOFF_MODE_TX);
		phy_initialized = false;
		init_and_close_radio = false;
	}
	else
	{
		if (init_and_close_radio)
			return;

		WriteSingleReg(MCSM1, RADIO_MCSM1_CCA_RSSILOWRX | RADIO_MCSM1_RXOFF_MODE_IDLE | RADIO_MCSM1_TXOFF_MODE_IDLE);
		init_and_close_radio = true;
	}

}

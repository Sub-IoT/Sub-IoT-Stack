/*
 *  Created on: Jan 23, 2013
 *  Authors:
 * 		maarten.weyn@uantwerpen.be
 */


#include <string.h>

#include <d7aoss.h>
#include <framework/log.h>
#include <hal/rtc.h>
#include <hal/crc.h>
#include <hal/leds.h>
#include <framework/timer.h>

#include <phy/cc430/cc430_phy.h>
#include <phy/cc430/rf1a.h>

#define SEND_INTERVAL_MS	2000
#define SYNC_PERIOD_MS		500
#define SYNC_LENGTH			50

#define USE_LEDS


#define PREAMBLE_SIZE_MIN	46 // 55.542- 10
#define PREAMBLE_SYMBOL		0xAA

#define INITIAL_PN9				0x01FF


#define CC430_STATE_TX                   (0x20)
#define CC430_STATE_IDLE                 (0x00)
#define CC430_STATE_TX_UNDERFLOW         (0x70)
#define CC430_STATE_MASK                 (0x70)
#define CC430_FIFO_BYTES_AVAILABLE_MASK  (0x0F)
#define CC430_STATE_RX                   (0x10)
#define CC430_STATE_RX_OVERFLOW          (0x60)

#define MIN(n,m) (((n) < (m)) ? (n) : (m))

#define INTERRUPT_RTC 		(1 << 3)
#define BATTERY_OFFSET	-10
#define CLOCKS_PER_1us	20

#define LED_RED	1
#define LED_ORANGE 2
#define LED_GREEN 3

//static u8 interrupt_flags = 0;
//static u8 tx = 0;
#define BUFFER_LENGTH 512


uint8_t buffer[BUFFER_LENGTH];
uint8_t dataLength = 0;
uint16_t battery_voltage;
uint8_t sync_position = 0;
uint32_t volatile targetTimeStamp;

unsigned char transmitting = 0;
unsigned char receiving = 0;
unsigned char packetTransmit;

timer_event event;
timer_event sync_event;
timer_event queue_event;


//uint8_t data[512];

static uint8_t spectrum_id[2] = { 0x00, 0x04};
static uint8_t sync_word[2] = {0xD0, 0xE6}; //0xE6D0

static bool first_packet = true;

static uint16_t eta = 500;

static uint16_t pn9;
static uint8_t pn9buffer;

//
//
//void start_tx()
//{
//	log_print_string("tx_event removed");
//
//	#ifdef USE_LEDS
//	led_on(LED_GREEN);
//	#endif
//
//	sync_position = SYNC_LENGTH;
//
//	targetTimeStamp = timer_get_counter_value() + SYNC_PERIOD_MS;
//
//	eta = 1000;
//
//	nwl_build_advertising_protocol_data(eta, spectrum_id, 10, 0xFF);
//	dll_tx_frame();
//
//	timer_add_event(&sync_event);
//}
//
void start_tx_sync()
{
//
//	//log_print_string("sync_event removed");
	eta = targetTimeStamp - timer_get_counter_value();
	log_print_string("ADVP eta: %d", eta);

	if (eta < 5 || eta > SYNC_PERIOD_MS)
	{
//		#ifdef USE_LEDS
//		led_off(LED_GREEN);
//		led_on(LED_ORANGE);
//		#endif
//
//		//trans_tx_foreground_frame(data, dataLength, 0xFF, SEND_CHANNEL, 10);
//
//		//log_print_string("tx_event added");
		log_print_string("FF");

		packetTransmit = 1;
		transmitting = 0;

	} else {
//		//log_print_string("sync_event added");
//		#ifdef USE_LEDS
//		led_on(LED_GREEN);
//		#endif
//		//log_print_string("BF");
//
//		//log_print_data((uint8_t*) &sync_position, 1);
//		//trans_tx_background_frame((uint8_t*) &counter, 0xFF, SEND_CHANNEL, 10);
//

		#ifdef USE_LEDS
		led_on(LED_ORANGE);
		#endif


		nwl_build_advertising_protocol_data(eta, spectrum_id, 10, 0xFF);
		dll_tx_frame();
	}
}

void tx_callback(Dll_Tx_Result result)
{
#ifdef USE_LEDS
	if(result == DLLTxResultOK)
	{

		led_off(LED_ORANGE);
		log_print_string("TX OK");
	}
	else
	{
		led_toggle(LED_RED);
		log_print_string("TX CCA FAIL");
	}

	transmitting = 1;
#endif
}

uint8_t calculate_pn9(uint8_t input)
{
//	uint16_t tmppn9;
//
//	//Pn9 data whitening
//	pn9buffer = input ^ (uint8_t)pn9;
//
//	//Rotate pn9 code
//	tmppn9 = ((pn9 << 5) ^ pn9) & 0x01E0;
//	pn9 = tmppn9 | (pn9 >> 4);
//	tmppn9 = ((pn9 << 5) ^ pn9) & 0x01E0;
//	pn9 = tmppn9 | (pn9 >> 4);
//
//	return pn9buffer;
	return input;
}

void fill_buffer()
{
	while (tx_queue.length < (BUFFER_LENGTH-(30+14)) && eta > 0)
	{
		if (!first_packet)
		{
			uint8_t i = 0;

			for (;i<PREAMBLE_SIZE_MIN;i++)
				queue_push_u8(&tx_queue, PREAMBLE_SYMBOL);

			queue_push_u8(&tx_queue, sync_word[1]);
			queue_push_u8(&tx_queue, sync_word[0]);
		} else {
			first_packet = false;
		}


		// re_init PN9
		pn9 = INITIAL_PN9;

		uint8_t frame[8];
		frame[0] = calculate_pn9(8); // LENGTH
//		frame[1] = calculate_pn9(0xFF); // SUBNET
//		frame[2] = calculate_pn9(42); // Control
//		frame[3] = calculate_pn9(0xF0); // BPID
//		frame[4] = calculate_pn9(eta >> 8); // eta
//		frame[5] = calculate_pn9((uint8_t) eta); // eta
//
//		uint16_t crc = crc_calculate(frame, 6);
//
//		frame[6] = calculate_pn9(crc >> 8);
//		frame[7] = calculate_pn9((uint8_t) crc);

		uint8_t i = 1;
		for (;i<8;i++)
			frame[i] = i;

		queue_push_u8_array(&tx_queue, frame, 8);

		eta--;
	}
}

void queue_data()
{

	led_toggle(LED_ORANGE);

	fill_buffer();

	uint8_t TxStatus = Strobe(RF_SNOP);
	uint8_t freeSpaceInFifo;

	switch (TxStatus & CC430_STATE_MASK) {
		case CC430_STATE_TX:
			// If there's anything to transfer..
			if (freeSpaceInFifo = MIN(tx_queue.length, TxStatus & CC430_FIFO_BYTES_AVAILABLE_MASK))
			//if (freeSpaceInFifo = TxStatus & CC430_FIFO_BYTES_AVAILABLE_MASK)
			{
			 // txBytesLeft -= freeSpaceInFifo;

			  while(freeSpaceInFifo--)
			  {
				WriteSingleReg(TXFIFO, queue_pop_u8(&tx_queue));
				//txPosition++;
			  }

			  if(tx_queue.length == 0)
			  {
				RF1AIES |= BIT9;      // End-of-packet TX interrupt
				RF1AIFG &= ~BIT9;     // clear RFIFG9
				//while(!(RF1AIFG & BIT9)); // poll RFIFG9 for TX end-of-packet
				//RF1AIES &= ~BIT9;      // End-of-packet TX interrupt
				//RF1AIFG &= ~BIT9;     // clear RFIFG9
				transmitting = 0;
				packetTransmit = 1;
			  }
			}

			timer_add_event(&queue_event);

			break;

		case CC430_STATE_TX_UNDERFLOW:
			Strobe(RF_SFTX);  // Flush the TX FIFO

			//__no_operation(void);
			// No break here!

			if(!packetTransmit)
				packetTransmit = 1;

			transmitting = 0;
			led_off(LED_GREEN);
			led_on(LED_RED);
			timer_add_event(&event);;
		default:
			if(!packetTransmit)
			  packetTransmit = 1;

			if (transmitting) {
				if ((TxStatus & CC430_STATE_MASK) == CC430_STATE_IDLE) {
				  transmitting = 0;
					led_off(LED_GREEN);
					timer_add_event(&event);
				} else {
					timer_add_event(&queue_event);
				}
			}
		break;
	}
}

void start_tx()
{
	led_on(LED_GREEN);

	led_off(LED_RED);
	led_off(LED_ORANGE);

	//fill_buffer();

//	Strobe(RF_SIDLE);
//
//	set_length_infinite(true);
//	WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_61_4);
//	set_channel(spectrum_id[0], (spectrum_id[1] >> 2) & 0x03);
//	set_preamble_size(4);
//	set_sync_word(0xE6D0);
//
//	set_data_whitening(false);
//
//	Strobe( RF_STX );                         // Strobe STX

	//set_data_whitening(false);

	phy_keep_radio_on(true);


	packetTransmit = 0;

	targetTimeStamp = timer_get_counter_value() + SYNC_PERIOD_MS;
	//eta = 5;

	//timer_add_event(&queue_event);



	timer_add_event(&event);
	transmitting = 1;

}

int main(void) {
	d7aoss_init(buffer, BUFFER_LENGTH, buffer, BUFFER_LENGTH);



	nwl_set_tx_callback(tx_callback);

	event.next_event = SEND_INTERVAL_MS;
	event.f = &start_tx;

	//sync_event.next_event = 5;
	//sync_event.f = &start_tx_sync;

	queue_event.next_event = 1;
	queue_event.f = &queue_data;

	log_print_string("sync node 1 started");

	timer_add_event(&event);

	while(1)
	{
		if (transmitting)
		{
			transmitting = 0;
			start_tx_sync();
		}

		system_lowpower_mode(0,1);
	}
}

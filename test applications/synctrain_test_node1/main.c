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
#define CLOCKS_PER_1us	4

#define LED_RED	1
#define LED_ORANGE 2
#define LED_GREEN 3

//static u8 interrupt_flags = 0;
//static u8 tx = 0;
#define BUFFER_LENGTH 512

#define TX_EIRP 10


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

static uint16_t eta = 500;

static uint16_t counter = 0;

static ALP_File_Data_Template data_template;
static ALP_Template alp_template;

static uint8_t data[4];

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
		if (eta > SYNC_PERIOD_MS)
			eta = 4;

//		while (eta > 0)
//		{
//			__delay_cycles(4);
//			eta--;
//		}

		#ifdef USE_LEDS
		led_off(LED_GREEN);
		led_on(LED_ORANGE);
		#endif

		data[0] = counter >> 8;
		data[1] = counter & 0xFF;
		counter++;

		alp_create_structure_for_tx(ALP_REC_FLG_TYPE_UNSOLICITED, 0, 1, &alp_template);
		trans_tx_query(NULL, 0xFF, spectrum_id, TX_EIRP);

		log_print_string("tx_event added");
		log_print_string("FF");

		packetTransmit = 1;
		transmitting = 0;

	} else {
		#ifdef USE_LEDS
		led_on(LED_GREEN);
		#endif
		log_print_string("BF");


		nwl_build_advertising_protocol_data(eta, spectrum_id, TX_EIRP, 0xFF);
		dll_tx_frame();
	}
}

void tx_callback(Dll_Tx_Result result)
{
#ifdef USE_LEDS
	if(result == DLLTxResultOK)
	{

		led_off(LED_ORANGE);
		led_off(LED_GREEN);
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

		alp_template.op = ALP_OP_RESP_DATA;
	alp_template.data = (uint8_t*) &data_template;

	data_template.file_id = 32;
	data_template.start_byte_offset = 2;
	data_template.bytes_accessing = 4;
	data_template.data = data;

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

#include "em_device.h"
#include "em_system.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_rtc.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_i2c.h"
#include <assert.h>
#include "hwgpio.h"
#include "MicroOLED.h"


#include "hwi2c.h"
#include "platform.h"


#define I2C_SDA D6 // PE12
#define I2C_SCL D7 // PE13
#define	I2C_LOC	1


#define I2C_PORT    gpioPortE // PORTE - I2C0 Location #6
#define I2C_SDA_PIN 12 // PE12
#define I2C_SCL_PIN 13 // PE13

#define LEUART_PORT   gpioPortF // PORTF - LEUART0 Location #3
#define LEUART_TX_PIN 0 // PF0
#define LEUART_RX_PIN 1 // PF1

#define SENSOR_SLAVE_ADDR 0xAC // see TSL2569 datasheet for details

void print_byte_setup(uint8_t byte);

/* Global variables */
volatile uint8_t print_byte_array[4] = { 0x30, 0x78, 0x20, 0x20 }; // array for displaying bytes one nibble at a time
const char header[] = "\n\rEFM32 Zero Gecko I2C Example\n\r";
const char id_string[] = "Part# RevID: ";
const char nack_error_string[] = "NACK Received\n\r";

void i2c_init() {

//  CHIP_Init();


  /* Setup clock tree */
  /* Use 14MHz RC oscillator as HF clock source */
  /* Bootloader Clock Correction */
  // If using uart bootloader to program, the system boots with 21MHz internal RC as
  // the HF clock source. Change to 14MHz, but don't overwrite reset tuning value.
  CMU_HFRCOBandSet(cmuHFRCOBand_14MHz); // set HFCLK to 14MHz
  /* End Bootloader Clock Correction */

  CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_CORELEDIV2);           // select HFCORECLK/2 as clock source to LFB
  CMU_ClockEnable(cmuClock_CORELE, true);                           // enable the Low Energy Peripheral Interface clock

  /* LEUART reset (if using bootloader to program)
  LEUART0->ROUTE = 0;                                               // reset the LEUART route register*/
  CMU_ClockEnable(cmuClock_LEUART0, false);                         // disable LEUART peripheral clock
  /*for(i=0; i<2000; i++);                                            // brief delay to keep TX line low at least one frame period
  /* End LEUART reset */

  //CMU_ClockEnable(cmuClock_LEUART0, true);                          // enable LEUART peripheral clock
  //CMU_ClockDivSet(cmuClock_LFB, 1);                                 // set LFB prescaler to /2: LEUART clock freq = 14MHz/2/2 = 3.5MHz
  CMU_ClockEnable(cmuClock_HFPER, true);                            // enable HF peripheral clock  -->> enables fast mode
  CMU_ClockEnable(cmuClock_GPIO, true);                             // enable GPIO peripheral clock*/
  CMU_ClockEnable(cmuClock_I2C0, true);                             // enable I2C0 peripheral clock

  /* Configure GPIO */
  error_t err;
  //err = hw_gpio_configure_pin(LEUART_PORT, LEUART_TX_PIN, gpioModePushPull, 1); // configure UART TX pin as output, initialize high
  //err = hw_gpio_configure_pin(LEUART_PORT, LEUART_RX_PIN, gpioModeInput, 0);    // configure UART RX pin as input, no filter
 // err = hw_gpio_configure_pin(I2C_SDA, false, gpioModeWiredAnd, 0); assert(err == SUCCESS);     // configure SDA pin as open drain output
 // err = hw_gpio_configure_pin(I2C_SCL, false, gpioModeWiredAnd, 0); assert(err == SUCCESS);     // configure SCL pin as open drain outputµ

  err = hw_gpio_configure_pin(I2C_SDA, false, gpioModeWiredAnd, 0); assert(err == SUCCESS);     // configure SDA pin as open drain output
  err = hw_gpio_configure_pin(I2C_SCL, false, gpioModeWiredAnd, 0); assert(err == SUCCESS);     // configure SCL pin as open drain output

  err = hw_gpio_configure_pin(D5, false, gpioModePushPull, 0);

  /* Configure COM Port (LEUART0 asynch)
  LEUART_Init_TypeDef leuart_init =
  {
    .enable   = leuartEnable,                    // enable Receiver and Transmitter
    .refFreq  = 0,                               // measure reference clock
    .baudrate = 115200,                          // 115200 bps
    .databits = leuartDatabits8,                 // 8 data bits
    .parity   = leuartNoParity,                  // no parity bits
    .stopbits = leuartStopbits1,                 // 1 stop bit
  };
  LEUART_IntClear(LEUART0, (0xFF << 3) | 0x1);   // clear interrupt flags (optional)
  LEUART_Init(LEUART0, &leuart_init);            // apply configuration to LEUART0
  LEUART0->ROUTE = (3 << 8) | 0x3;               // use Location #3 (PF0, PF1) enable TX/RX pins

  /* Configure I2C - Fast mode (400kHz) Master */
  I2C_Init_TypeDef i2c_init =
  {
    .enable = true,                              // enable I2C
    .master = true,                              // I2C master
    .refFreq = 0,                                // measure reference clock
    .freq = 400000,                              // 400kbps fast mode
    .clhr = i2cClockHLRAsymetric,                // use 6:3 bit ratio
  };
  I2C_Init(I2C0, &i2c_init);                     // apply configuration to I2C0
  I2C0->CTRL |= (1 << 2);                        // enable AUTO-ACK feature
  I2C0->ROUTE = (I2C_LOC << 8) | 0x3;                  // use location #6 (PE11, PE12), enable SDA and SCL

  /* Print Header */
  //for(i=0; i<sizeof(header); i++) {
    //LEUART_Tx(LEUART0, header[i]);
  //}


  //while(1);
}

void i2c_write(uint8_t address,uint8_t* tx_buffer,int length)
{
	//uint8_t tx_buffer[17]= {0x03,0x00,0xA4,0x00,0x07,0xD2,0x76,0x00,0x00,0x85,0x01,0x01,0x00,0xDF,0xBE};           // software tx buffer
	//uint8_t rx_buffer[7];           // software rx buffer
	I2C_TransferReturn_TypeDef ret; // I2C state tracker
	/* Send power-up command to sensor (see TSL2569 datasheet for details) */
	//tx_buffer[] = {0xAC,0x02,0x00,0xA4,0x00,0x07,0xD2,0x76,0x00,0x00,0x85,0x01,0x01,0x00,0x35,0xC0};
	I2C_TransferSeq_TypeDef sensor_message =
	{
	    .addr = address,//SENSOR_SLAVE_ADDR,//(SENSOR_SLAVE_ADDR << 1),            // set sensor slave address
	    .flags = I2C_FLAG_WRITE,//I2C_FLAG_WRITE,                     // indicate basic write
	    .buf[0].data = tx_buffer,                    // point to tx_buffer
	    .buf[0].len = length,//15,                             // specify number of bytes
	};

	ret = I2C_TransferInit(I2C0, &sensor_message); // start I2C write transaction with sensor
	while(ret == i2cTransferInProgress) {          // continue until all data has been sent
	    ret = I2C_Transfer(I2C0);
	}
}

void i2c_read(uint8_t address,uint8_t* rx_buffer, int length)
{
	//static uint8_t rx_buffer[20]; //fixed size for now
	I2C_TransferReturn_TypeDef ret; // I2C state tracker
	I2C_TransferSeq_TypeDef sensor_message =
	{
		.addr = SENSOR_SLAVE_ADDR,//(SENSOR_SLAVE_ADDR << 1),            // set sensor slave address
		.flags = I2C_FLAG_READ,//I2C_FLAG_WRITE,                     // indicate basic write
		.buf[1].data = rx_buffer,                    // point to tx_buffer
		.buf[1].len = length,//15,                             // specify number of bytes
	};
	ret = I2C_TransferInit(I2C0, &sensor_message); // start I2C write transaction with sensor
	while(ret == i2cTransferInProgress) {          // continue until all data has been sent
		ret = I2C_Transfer(I2C0);
	}
	//return rx_buffer;
}


void i2c_write_read(uint8_t address,uint8_t* tx_buffer,int lengthtx,uint8_t* rx_buffer,int lengthrx)
{
	//uint8_t tx_buffer[17]= {0x03,0x00,0xA4,0x00,0x07,0xD2,0x76,0x00,0x00,0x85,0x01,0x01,0x00,0xDF,0xBE};           // software tx buffer
	//uint8_t rx_buffer[7];           // software rx buffer
	I2C_TransferReturn_TypeDef ret; // I2C state tracker
	/* Send power-up command to sensor (see TSL2569 datasheet for details) */
	//tx_buffer[] = {0xAC,0x02,0x00,0xA4,0x00,0x07,0xD2,0x76,0x00,0x00,0x85,0x01,0x01,0x00,0x35,0xC0};
	I2C_TransferSeq_TypeDef sensor_message =
	{
	    .addr = address,//SENSOR_SLAVE_ADDR,//(SENSOR_SLAVE_ADDR << 1),            // set sensor slave address
	    .flags = I2C_FLAG_WRITE_READ,//I2C_FLAG_WRITE,                     // indicate basic write
	    .buf[0].data = tx_buffer,                    // point to tx_buffer
	    .buf[0].len = lengthtx,//15,                             // specify number of bytes
		.buf[1].data = rx_buffer,
		.buf[1].len = lengthrx,
	};

	ret = I2C_TransferInit(I2C0, &sensor_message); // start I2C write transaction with sensor
	while(ret == i2cTransferInProgress) {          // continue until all data has been sent
	    ret = I2C_Transfer(I2C0);
	}
}





void read_sensor()
{
	  uint16_t i;
	  uint8_t tx_buffer[17]= {0x03,0x00,0xA4,0x00,0x07,0xD2,0x76,0x00,0x00,0x85,0x01,0x01,0x00,0xDF,0xBE};           // software tx buffer
	  uint8_t rx_buffer[7];           // software rx buffer
	  I2C_TransferReturn_TypeDef ret; // I2C state tracker
	/* Send power-up command to sensor (see TSL2569 datasheet for details) */
	  //tx_buffer[] = {0xAC,0x02,0x00,0xA4,0x00,0x07,0xD2,0x76,0x00,0x00,0x85,0x01,0x01,0x00,0x35,0xC0};
	  I2C_TransferSeq_TypeDef sensor_message =
	  	  {
	  	    .addr = SENSOR_SLAVE_ADDR,//(SENSOR_SLAVE_ADDR << 1),            // set sensor slave address
	  	    .flags = I2C_FLAG_WRITE,//I2C_FLAG_WRITE,                     // indicate basic write
	  	    .buf[0].data = tx_buffer,                    // point to tx_buffer
	  	    .buf[0].len = 15,                             // specify number of bytes
	  	  };

	  ret = I2C_TransferInit(I2C0, &sensor_message); // start I2C write transaction with sensor
	  	  while(ret == i2cTransferInProgress) {          // continue until all data has been sent
	  	    ret = I2C_Transfer(I2C0);
	  	  }

	  tx_buffer[0] = 0x02;//0xD0;                           // acces register
	  tx_buffer[1] = 0x90;//0x3;                            //
	  tx_buffer[2] = 0x00;//0x3;                            // offset = 8
	  tx_buffer[3] = 0xF1;//0x3;
	  tx_buffer[4] = 0x09;//0x3;

	  sensor_message.flags = I2C_FLAG_READ;//I2C_FLAG_WRITE,                     // indicate basic write
	  sensor_message.buf[0].data = tx_buffer;                    // point to tx_buffer
	  sensor_message.buf[0].len = 5;                             // specify number of bytes
	  sensor_message.buf[1].len = 5;                 // specify # bytes to be read
	  sensor_message.buf[1].data = rx_buffer;

	  ret = I2C_TransferInit(I2C0, &sensor_message); // start I2C write transaction with sensor
	  while(ret == i2cTransferInProgress) {          // continue until all data has been sent
	    ret = I2C_Transfer(I2C0);
	  }

	  /* Read part# and rev ID from sensor (see TSL2569 datasheet for details) */
	  tx_buffer[0] = 0x00;//0xD0;                           // acces register
	  tx_buffer[1] = 0x0B;//0x3;                            //
	  tx_buffer[2] = 0x00;//0x3;                            // offset = 8
	  tx_buffer[3] = 0x08;//0x3;                            // offset = 8
	  tx_buffer[4] = 0x07;

	  sensor_message.flags = I2C_FLAG_WRITE;         //I2C_FLAG_READ;//I2C_FLAG_WRITE_READ;    // indicate combined write/read
	  sensor_message.buf[0].len = 5;                 // specify # bytes to be written
	  sensor_message.buf[0].data = tx_buffer;        // point to tx_buffer
	  sensor_message.buf[1].len = 7;                 // specify # bytes to be read
	  sensor_message.buf[1].data = rx_buffer;        // point to rx_buffer

	  ret = I2C_TransferInit(I2C0, &sensor_message); // start I2C write/read transaction with sensor
	  while(ret == i2cTransferInProgress) {          // continue until all data has been received
	    ret = I2C_Transfer(I2C0);
	  }

	  sensor_message.flags = I2C_FLAG_READ;
	  ret = I2C_TransferInit(I2C0, &sensor_message); // start I2C write/read transaction with sensor
	  	  while(ret == i2cTransferInProgress) {          // continue until all data has been received
	  	    ret = I2C_Transfer(I2C0);
	  	  }

	  print_byte_setup(rx_buffer[0]);                // break result up into individual characters

	  /* Print Part# RevID */
	  for(i=0; i<sizeof(id_string); i++) {
	    //LEUART_Tx(LEUART0, id_string[i]);
	  }
	  for(i=0; i<sizeof(print_byte_array); i++) {
	    //LEUART_Tx(LEUART0, print_byte_array[i]);
	  }
}
// This function breaks up 'byte' into high and low nibbles (ASCII characters) to be transmitted over UART
void print_byte_setup(uint8_t byte) {

  if (((byte & 0xF0) >> 4) <= 0x09) {                      // if high nibble is less than 0xA
    print_byte_array[2] = ((byte & 0xF0) >> 4) + 0x30;     // store ASCII char
  }
  if ((((byte & 0xF0) >> 4) >= 0x0A) && (((byte & 0xF0) >> 4)<= 0x0F)) { // if high nibble is between 0xA and 0xF
    print_byte_array[2] = ((byte & 0xF0) >> 4) + 0x37;     // store ASCII char
  }
  if ((byte & 0x0F) <= 0x09) {                             // if low nibble is less than 0xA
    print_byte_array[3] = (byte & 0x0F) + 0x30;            // store ASCII char
  }
  if (((byte & 0x0F) >= 0x0A) && ((byte & 0x0F)<= 0x0F)) { // if low nibble is between 0xA and 0xF
    print_byte_array[3] = (byte & 0x0F) + 0x37;            // store ASCII char
  }
}

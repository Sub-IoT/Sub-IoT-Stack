// *************************************************************************************************
//
// Actual revision: $Revision: $
// Revision label:  $Name: $
// Revision state:  $State: $
//
// *************************************************************************************************
// Radio core access functions. Taken from TI reference code for CC430.
// *************************************************************************************************

#include <stdbool.h>
#include <stdint.h>

#include "cc1101_constants.h"
//#include "radio_spi_hw.h" // TODO provide HAL SPI API
#include "cc1101_core.h"
#include "spi.h"
#include "timer.h"
#include "log.h"

// turn on/off the debug prints
#if 1
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)  
#endif

// *************************************************************************************************
// @fn          Strobe
// @brief       Send command to radio.
// @param       none
// @return      none
// *************************************************************************************************
uint8_t Strobe(uint8_t strobe) {

    DPRINT("STROBE 0x%02X", strobe);

    uint8_t statusByte = 0;
    uint8_t strobe_tmp = strobe & 0x7F;
//	uint16_t int_state;

    // Check for valid strobe command
    if ((strobe_tmp >= RF_SRES) && (strobe_tmp <= RF_SNOP)) {
            statusByte = spi_byte(strobe & 0x3f);
    }

    return statusByte;
}

// *****************************************************************************
// @fn          ResetRadioCore
// @brief       Reset the radio core using RF_SRES command
// @param       none
// @return      none
// ***********************************;******************************************
void ResetRadioCore(void) {

    DPRINT("RESET RADIO");

    spi_deselect_chip();
    //delayuS(30);
    timer_wait_ms(1);
    spi_select_chip();
    //delayuS(30);
    timer_wait_ms(1);
    spi_deselect_chip();
    //delayuS(45);
    timer_wait_ms(1);
    Strobe(RF_SRES);                          // Reset the Radio Core
    Strobe(RF_SNOP);                          // Get Radio Status
}

// *****************************************************************************
// @fn          WriteRfSettings
// @brief       Write the RF configuration register settings
// @param       RF_SETTINGS* rfsettings  Pointer to the structure that holds the rf settings
// @return      none
// *****************************************************************************
void WriteRfSettings(RF_SETTINGS *rfsettings) {
	WriteBurstReg(IOCFG2, (unsigned char*) rfsettings, sizeof(RF_SETTINGS));
}


static uint8_t readreg(uint8_t addr) {



    /*
    radioSelect();
    spiSendByte((addr & 0x3F) | READ_SINGLE);
    uint8_t val = spiSendByte(0); // send dummy byte to receive reply
    radioDeselect();
    */

    unsigned char buf[2] = { ((addr & 0x3F) | READ_SINGLE), 0 };

    spi_string( buf, buf, 2 );

    DPRINT("READ REG 0x%02X @0x%02X", buf[1], addr);

    return buf[1];
}

static uint8_t readstatus(uint8_t addr)
{


    uint8_t ret, retCheck, data, data2;
    /*
    uint8_t addr = (addr & 0x3F) | READ_BURST;
    radioSelect();
    ret = spiSendByte(addr);
    data = spiSendByte(0); // send dummy byte to receive reply

    See CC1101's Errata for SPI read errors
    while (true) {
    	retCheck = spiSendByte(addr);
        data2 = spiSendByte(0);
    	if (ret == retCheck && data == data2)
    		break;
    	else {
    		ret = retCheck;
    		data = data2;
    	}
    }
    radioDeselect();
    return data;
    */

    uint8_t txBuf[2] = { ((addr & 0x3F) | READ_BURST), 0 };
    uint8_t rxBuf[2];

    // turn off the auto Chip Select for polling purposes
    spi_auto_cs_off();
    // manually select the chip
    spi_select_chip();
    // read register
    spi_string( txBuf, rxBuf, 2 );
    ret = rxBuf[0];
    data = rxBuf[1];
    
    //See CC1101's Errata for SPI read errors
    while (true)
    {
    	spi_string( txBuf, rxBuf, 2 );
        retCheck = rxBuf[0];
        data2 = rxBuf[1];
    	if (ret == retCheck && data == data2)
        {
            break;
        }
        else
        {
    	    ret = retCheck;
            data = data2;
    	}
    }
    // manually deselect the chip
    spi_deselect_chip();
    // Enable auto Chip Select
    spi_auto_cs_on();

    DPRINT("READ STATUS 0x%02X @0x%02X", data, addr);

    return data;
}


// *****************************************************************************
// @fn          ReadSingleReg
// @brief       Read a single byte from the radio register
// @param       unsigned char addr      Target radio register address
// @return      unsigned char data_out  Value of byte that was read
// *****************************************************************************
uint8_t ReadSingleReg(uint8_t addr) {

    uint8_t value;

    // Check for valid configuration register address, PATABLE or FIFO
    if ((addr<= 0x2E) || (addr>= 0x3E)) {
            value = readreg(addr);
    }
    else {
            value = readstatus(addr);
    }

    return value;
}

// *****************************************************************************
// @fn          WriteSingleReg
// @brief       Write a single byte to a radio register
// @param       unsigned char addr      Target radio register address
// @param       unsigned char value     Value to be written
// @return      none
// *****************************************************************************
void WriteSingleReg(uint8_t addr, uint8_t value) {



    //radioSelect();
    //spiSendByte((addr & 0x3F));
    //spiSendByte(value);
    //radioDeselect();

    uint8_t buf[2] = { (addr & 0x3F), value };

    spi_string( buf, NULL, 2 );

    DPRINT("WRITE SREG 0x%02X @0x%02X", value, addr);
}

// *****************************************************************************
// @fn          ReadBurstReg
// @brief       Read multiple bytes to the radio registers
// @param       unsigned char addr      Beginning address of burst read
// @param       unsigned char *buffer   Pointer to data table
// @param       unsigned char count     Number of bytes to be read
// @return      none
// *****************************************************************************
void ReadBurstReg(uint8_t addr, uint8_t* buffer, uint8_t count) {

    DPRINT("READ BREG @0x%02X", addr);

    //uint8_t _addr = (addr & 0x3F) | READ_BURST;
    //radioSelect();
    //spiSendByte(_addr);
    //uint8_t i;
    //for (i = 0; i < count; i++) {
    //        buffer[i] = spiSendByte(0); // send dummy byte to receive reply
    //}
    //radioDeselect();
    
    // fill tx buffer
    uint8_t buf[count+1];
    buf[0] = (addr & 0x3F) | READ_BURST;

    // read registers
    spi_string( buf, buf, count+1 );

    // retreive register values
    uint8_t i;
    for (i = 0; i < count; i++)
    {
        buffer[i] = buf[i+1];
    }
}

// *****************************************************************************
// @fn          WriteBurstReg
// @brief       Write multiple bytes to the radio registers
// @param       unsigned char addr      Beginning address of burst write
// @param       unsigned char *buffer   Pointer to data table
// @param       unsigned char count     Number of bytes to be written
// @return      none
// *****************************************************************************
void WriteBurstReg(uint8_t addr, uint8_t* buffer, uint8_t count) {

    DPRINT("WRITE BREG @0x%02X", addr);

    //uint8_t _addr = (addr & 0x3F) | WRITE_BURST;
    //radioSelect();
    //spiSendByte(_addr);
    //uint8_t i;
    //for (i = 0; i < count; i++) {
    //	spiSendByte(buffer[i]);
    //}
    //radioDeselect();

    // fill transmission buffer
    uint8_t buf[count+1];
    buf[0] = (addr & 0x3F) | WRITE_BURST;

    uint8_t i;
    for (i = 0; i < count; i++)
    {
        buf[i+1] = buffer[i];
    }

    // write register values
    spi_string( buf, buf, count+1 );
}

// *****************************************************************************
// @fn          WritePATable
// @brief       Write data to power table
// @param       unsigned char value		Value to write
// @return      none
// *****************************************************************************
void WriteSinglePATable(uint8_t value) {
	WriteSingleReg(PATABLE, value);
}

// *****************************************************************************
// @fn          WritePATable
// @brief       Write to multiple locations in power table 
// @param       unsigned char *buffer	Pointer to the table of values to be written 
// @param       unsigned char count	Number of values to be written
// @return      none
// *****************************************************************************
void WriteBurstPATable(uint8_t* buffer, uint8_t count) {
	WriteBurstReg(PATABLE, buffer, count);
}

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
#ifdef LOG_PHY_ENABLED
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

    uint8_t statusByte = 0;
    uint8_t strobe_tmp = strobe & 0x7F;
//	uint16_t int_state;

    // Check for valid strobe command
    if ((strobe_tmp >= RF_SRES) && (strobe_tmp <= RF_SNOP))
    {
        spi_select_chip();
        statusByte = spi_byte(strobe & 0x3F);
        spi_deselect_chip();
    }

    DPRINT("STROBE 0x%02X STATUS: 0x%02X", strobe, statusByte);

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

    spi_select_chip();
    spi_byte((addr & 0x3F) | READ_SINGLE);
    uint8_t val = spi_byte(0); // send dummy byte to receive reply
    spi_deselect_chip();

    DPRINT("READ REG 0x%02X @0x%02X", val, addr);

    return val;
}

static uint8_t readstatus(uint8_t addr)
{
    uint8_t ret, retCheck, data, data2;
    uint8_t _addr = (addr & 0x3F) | READ_BURST;
    spi_select_chip();
    ret = spi_byte(_addr);
    data = spi_byte(0); // send dummy byte to receive reply

    // See CC1101's Errata for SPI read errors // TODO needed?
    while (true) {
    	retCheck = spi_byte(_addr);
        data2 = spi_byte(0);
    	if (ret == retCheck && data == data2)
    		break;
    	else {
    		ret = retCheck;
    		data = data2;
    	}
    }

    spi_deselect_chip();

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

    spi_select_chip();
    spi_byte((addr & 0x3F));
    spi_byte(value);
    spi_deselect_chip();

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

    uint8_t _addr = (addr & 0x3F) | READ_BURST;
    spi_select_chip();
    spi_byte(_addr);
    spi_string( NULL, buffer, count );
    spi_deselect_chip();
    
    DPRINT("READ BREG %u Byte(s) @0x%02X", count, addr);
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

    uint8_t _addr = (addr & 0x3F) | WRITE_BURST;
    spi_select_chip();
    spi_byte(_addr);
    spi_string( buffer, NULL, count );
    spi_deselect_chip();

    DPRINT("WRITE BREG %u Byte(s) @0x%02X", count, addr);
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


uint8_t ReadPartNum( void )
{
    uint8_t reg;
    ReadBurstReg( 0x30, &reg, 1 );
    return reg;
}

uint8_t ReadVersion( void )
{
    uint8_t reg;
    ReadBurstReg( 0x31, &reg, 1 );
    return reg;
}

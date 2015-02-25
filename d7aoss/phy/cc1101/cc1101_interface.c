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

#include "cc1101_interface.h"
#include "log.h"

// turn on/off the debug prints
#ifdef LOG_PHY_ENABLED
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)  
#endif

// functions to be defined which contain CC1101 or CC430 specific implementation
// implementation is in cc1101_interface_spi.c for cc1101 or cc1101_interface_cc430.c for cc430.
extern void _cc1101_interface_init();
extern void _c1101_set_interrupt_enabled(bool);
extern uint8_t _strobe(uint8_t);
extern void _reset_radio_core();
extern uint8_t _read_single_reg(uint8_t);
extern void _write_single_reg(uint8_t, uint8_t);
extern void _read_burst_reg(uint8_t, uint8_t*, uint8_t);
extern void _write_burst_reg(uint8_t, uint8_t*, uint8_t);
extern void _write_single_patable(uint8_t);
extern void _write_burst_patable(uint8_t*, uint8_t);


void cc1101_interface_init()
{
    _cc1101_interface_init();
}

void cc1101_interface_set_interrupts_enabled(bool enabled)
{
    _c1101_interface_set_interrupts_enabled(enabled);
}

// *************************************************************************************************
// @fn          Strobe
// @brief       Send command to radio.
// @param       none
// @return      none
// *************************************************************************************************
uint8_t Strobe(uint8_t strobe_command)
{
    uint8_t statusByte = 0;
    uint8_t strobe_tmp = strobe_command & 0x7F;
//	uint16_t int_state;

    // Check for valid strobe command
    if ((strobe_tmp >= RF_SRES) && (strobe_tmp <= RF_SNOP))
    {
        statusByte = _strobe(strobe_command);
    }

    DPRINT("STROBE 0x%02X STATUS: 0x%02X", strobe_command, statusByte);

    return statusByte;
}

// *****************************************************************************
// @fn          ResetRadioCore
// @brief       Reset the radio core using RF_SRES command
// @param       none
// @return      none
// ***********************************;******************************************
void ResetRadioCore(void)
{
    DPRINT("RESET RADIO");
    _reset_radio_core();
}

// *****************************************************************************
// @fn          WriteRfSettings
// @brief       Write the RF configuration register settings
// @param       RF_SETTINGS* rfsettings  Pointer to the structure that holds the rf settings
// @return      none
// *****************************************************************************
void WriteRfSettings(RF_SETTINGS *rfsettings)
{
	WriteBurstReg(IOCFG2, (unsigned char*) rfsettings, sizeof(RF_SETTINGS));
}


// *****************************************************************************
// @fn          ReadSingleReg
// @brief       Read a single byte from the radio register
// @param       unsigned char addr      Target radio register address
// @return      unsigned char data_out  Value of byte that was read
// *****************************************************************************
uint8_t ReadSingleReg(uint8_t addr)
{
    return _read_single_reg(addr);
}

// *****************************************************************************
// @fn          WriteSingleReg
// @brief       Write a single byte to a radio register
// @param       unsigned char addr      Target radio register address
// @param       unsigned char value     Value to be written
// @return      none
// *****************************************************************************
void WriteSingleReg(uint8_t addr, uint8_t value)
{
    _write_single_reg(addr, value);
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
void ReadBurstReg(uint8_t addr, uint8_t* buffer, uint8_t count)
{
    _read_burst_reg(addr, buffer, count);

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
void WriteBurstReg(uint8_t addr, uint8_t* buffer, uint8_t count)
{
    _write_burst_reg(addr, buffer, count);

    DPRINT("WRITE BREG %u Byte(s) @0x%02X", count, addr);
}

// *****************************************************************************
// @fn          WritePATable
// @brief       Write data to power table
// @param       unsigned char value		Value to write
// @return      none
// *****************************************************************************
void WriteSinglePATable(uint8_t value)
{
    _write_single_patable(value);
}

// *****************************************************************************
// @fn          WritePATable
// @brief       Write to multiple locations in power table 
// @param       unsigned char *buffer	Pointer to the table of values to be written 
// @param       unsigned char count	Number of values to be written
// @return      none
// *****************************************************************************
void WriteBurstPATable(uint8_t* buffer, uint8_t count)
{
    _write_burst_patable(buffer, count);
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

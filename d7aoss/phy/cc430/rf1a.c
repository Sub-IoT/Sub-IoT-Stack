// *************************************************************************************************
//
// Actual revision: $Revision: $
// Revision label:  $Name: $
// Revision state:  $State: $
//
// *************************************************************************************************
// Radio core access functions. Taken from TI reference code for CC430.
// *************************************************************************************************

#include <stdint.h>
#include <msp430.h>

#include "rf1a.h"

#define ENTER_CRITICAL_SECTION(x)  	x = __get_interrupt_state(); __disable_interrupt()
#define EXIT_CRITICAL_SECTION(x)    __set_interrupt_state(x)

// *************************************************************************************************
// @fn          Strobe
// @brief       Send command to radio.
// @param       none
// @return      none
// *************************************************************************************************
uint8_t Strobe(uint8_t strobe)
{
	uint8_t statusByte = 0;
	uint8_t strobe_tmp = strobe & 0x7F;
	uint16_t int_state;

	// Check for valid strobe command
	if ((strobe_tmp >= RF_SRES) && (strobe_tmp <= RF_SNOP)) {
		ENTER_CRITICAL_SECTION(int_state);

		// Clear the Status read flag
		RF1AIFCTL1 &= ~(RFSTATIFG);

		// Wait for the Radio to be ready for the next instruction
		RADIO_INST_READY_WAIT();

		// Write the strobe instruction
		RF1AINSTRB = strobe;

		if (strobe_tmp != RF_SRES) {
			if(RF1AIN & 0x04) {
				if ((strobe_tmp != RF_SXOFF) && (strobe_tmp != RF_SWOR) && (strobe_tmp != RF_SPWD)  && (strobe_tmp != RF_SNOP)) {
					while (RF1AIN & 0x04);	// Is chip ready?
					//__delay_cycles(810);	// Delay for ~810usec at 1MHz CPU clock, see erratum RF1A7
					__delay_cycles(12960);	// Delay for ~810usec at 16MHz CPU clock, see erratum RF1A7
				}
			}

			RADIO_STAT_READY_WAIT();
		}

		statusByte = RF1ASTATB;
		EXIT_CRITICAL_SECTION(int_state);
	}

	return statusByte;
}

// *****************************************************************************
// @fn          ResetRadioCore
// @brief       Reset the radio core using RF_SRES command
// @param       none
// @return      none
// *****************************************************************************
void ResetRadioCore (void)
{
  Strobe(RF_SRES);                          // Reset the Radio Core
  Strobe(RF_SNOP);                          // Get Radio Status
}

// *****************************************************************************
// @fn          WriteRfSettings
// @brief       Write the RF configuration register settings
// @param       RF_SETTINGS* rfsettings  Pointer to the structure that holds the rf settings
// @return      none
// *****************************************************************************
void WriteRfSettings(RF_SETTINGS *rfsettings)
{
	WriteBurstReg(IOCFG2, (unsigned char*)rfsettings, sizeof(RF_SETTINGS));
}

// *****************************************************************************
// @fn          ReadSingleReg
// @brief       Read a single byte from the radio register
// @param       unsigned char addr      Target radio register address
// @return      unsigned char data_out  Value of byte that was read
// *****************************************************************************
uint8_t ReadSingleReg(uint8_t addr)
{
	uint8_t value;
	uint16_t int_state;
  
	ENTER_CRITICAL_SECTION(int_state);

	// Check for valid configuration register address, 0x3E refers to PATABLE
	if ((addr <= 0x2E) || (addr == 0x3E))
		RF1AINSTR1B = RF_SNGLREGRD | addr;
	else
		RF1AINSTR1B = RF_STATREGRD | addr;

	RADIO_DOUT_READY_WAIT();
	value = RF1ADOUTB;

	EXIT_CRITICAL_SECTION(int_state);

	return value;
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
	uint16_t int_state;

	ENTER_CRITICAL_SECTION(int_state);

	RADIO_INST_READY_WAIT();
	RF1AINSTRW = ((RF_REGWR | addr) << 8) + value;

	EXIT_CRITICAL_SECTION(int_state);
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
	uint8_t i;
	uint16_t int_state;

	ENTER_CRITICAL_SECTION(int_state);

	RADIO_INST_READY_WAIT();
	RF1AINSTR1B = RF_REGRD | addr;

	for (i = 0; i < (count-1); i++) {
		RADIO_DOUT_READY_WAIT();
		buffer[i] = RF1ADOUT1B;
	}
	buffer[count-1] = RF1ADOUT0B;

	EXIT_CRITICAL_SECTION(int_state);
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
	uint8_t i;
	uint16_t int_state;

	ENTER_CRITICAL_SECTION(int_state);

	RADIO_INST_READY_WAIT();
	RF1AINSTRW = ((RF_REGWR | addr) << 8) + buffer[0];

	for (i = 1; i < count; i++) {
		RADIO_DIN_READY_WAIT();
		RF1ADINB = buffer[i];
	}

	EXIT_CRITICAL_SECTION(int_state);
}

// *****************************************************************************
// @fn          WritePATable
// @brief       Write data to power table
// @param       unsigned char value		Value to write
// @return      none
// *****************************************************************************
void WriteSinglePATable(uint8_t value)
{
	uint16_t int_state;

	ENTER_CRITICAL_SECTION(int_state);

	RADIO_INST_READY_WAIT();
	RF1AINSTRW = (RF_SNGLPATABWR << 8) + value;

	RADIO_INST_READY_WAIT();
	RF1AINSTRB = RF_SNOP; // Reset PA_Table pointer

	EXIT_CRITICAL_SECTION(int_state);
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
	uint8_t i;
	uint16_t int_state;

	ENTER_CRITICAL_SECTION(int_state);

	RADIO_INST_READY_WAIT();
	RF1AINSTRW = (RF_PATABWR << 8) + buffer[0];

	for (i = 1; i < count; i++) {
		RADIO_DIN_READY_WAIT();
		RF1ADINB = buffer[i];
	}

	RADIO_INST_READY_WAIT();
	RF1AINSTRB = RF_SNOP; // Reset PA Table pointer

	EXIT_CRITICAL_SECTION(int_state);
}

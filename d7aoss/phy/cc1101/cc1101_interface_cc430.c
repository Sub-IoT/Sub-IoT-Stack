#ifndef CC1101_INTERFACE_CC430_H
#define CC1101_INTERFACE_CC430_H


#include <stdint.h>
#include <msp430.h>

#define RADIO_INST_READY_WAIT()		while(!(RF1AIFCTL1 & RFINSTRIFG))
#define RADIO_DIN_READY_WAIT()		while(!(RF1AIFCTL1 & RFDINIFG))
#define RADIO_STAT_READY_WAIT()		while(!(RF1AIFCTL1 & RFSTATIFG))
#define RADIO_DOUT_READY_WAIT()		while(!(RF1AIFCTL1 & RFDOUTIFG))

#define ENTER_CRITICAL_SECTION(x)  	x = __get_interrupt_state(); __disable_interrupt()
#define EXIT_CRITICAL_SECTION(x)    __set_interrupt_state(x)

// functions used by cc1101_interface.c when accessing the CC1101 of a CC430 SoC.

void _cc1101_interface_init()
{

}

uint8_t _strobe(uint8_t strobe)
{
    uint16_t int_state;
    uint8_t strobe_tmp = strobe & 0x7F;
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

    uint8_t statusByte = RF1ASTATB;
    EXIT_CRITICAL_SECTION(int_state);

    return statusByte;
}

uint8_t _reset_radio_core()
{
    _strobe(RF_SRES);                          // Reset the Radio Core
    return _strobe(RF_SNOP);                          // Get Radio Status
}

uint8_t _read_single_reg(uint8_t addr)
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

void _write_single_reg(uint8_t addr, uint8_t value)
{
    uint16_t int_state;

    ENTER_CRITICAL_SECTION(int_state);

    RADIO_INST_READY_WAIT();
    RF1AINSTRW = ((RF_REGWR | addr) << 8) + value;

    EXIT_CRITICAL_SECTION(int_state);
}

void _read_burst_reg(uint8_t addr, uint8_t* buffer, uint8_t count)
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


void _write_burst_reg(uint8_t addr, uint8_t* buffer, uint8_t count)
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

void _write_single_patable(uint8_t value)
{
    uint16_t int_state;

    ENTER_CRITICAL_SECTION(int_state);

    RADIO_INST_READY_WAIT();
    RF1AINSTRW = (RF_SNGLPATABWR << 8) + value;

    RADIO_INST_READY_WAIT();
    RF1AINSTRB = RF_SNOP; // Reset PA_Table pointer

    EXIT_CRITICAL_SECTION(int_state);
}

void _write_burst_patable(uint8_t* buffer, uint8_t count)
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

#endif // CC1101_INTERFACE_CC430_H

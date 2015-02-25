
// functions used by cc1101_interface.c when accessing a CC1101 over SPI.
// when using CCS instead of cmake make sure to exclude this file from the build

#include <stdint.h>

#include "spi.h"
#include "timer.h"

#include "cc1101_constants.h"
#include "cc1101_interface.h"

#include "radio_hw.h" // TODO replace by GPIO HAL API later

// turn on/off the debug prints
#ifdef LOG_PHY_ENABLED
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

void _cc1101_interface_init()
{
    spi_init();
    radioConfigureInterrupt();
}

void _c1101_interface_set_interrupts_enabled(bool enable)
{
    // TODO replace by GPIO HAL API later
    if(enable)
    {
        radioClearInterruptPendingLines();
        radioEnableGDO0Interrupt();
        //radioEnableGDO2Interrupt();
    }
    else
    {
        radioDisableGDO0Interrupt();
        //radioDisableGDO2Interrupt();
    }

}

uint8_t _strobe(uint8_t strobe)
{
    spi_select_chip();
    uint8_t statusByte = spi_byte(strobe & 0x3F);
    spi_deselect_chip();

    return statusByte;
}

uint8_t _reset_radio_core()
{
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
    return Strobe(RF_SNOP);                          // Get Radio Status
}

static uint8_t readreg(uint8_t addr)
{

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
//    while (true) {
//    	retCheck = spi_byte(_addr);
//        data2 = spi_byte(0);
//    	if (ret == retCheck && data == data2)
//    		break;
//    	else {
//    		ret = retCheck;
//    		data = data2;
//    	}
//    }

    spi_deselect_chip();

    DPRINT("READ STATUS 0x%02X @0x%02X", data, addr);

    return data;
}

uint8_t _read_single_reg(uint8_t addr)
{
    // Check for valid configuration register address, PATABLE or FIFO
    if ((addr<= 0x2E) || (addr>= 0x3E))
        return readreg(addr);
    else
        return readstatus(addr);
}

void _write_single_reg(uint8_t addr, uint8_t value)
{
    spi_select_chip();
    spi_byte((addr & 0x3F));
    spi_byte(value);
    spi_deselect_chip();
}

void _read_burst_reg(uint8_t addr, uint8_t* buffer, uint8_t count)
{
    uint8_t _addr = (addr & 0x3F) | READ_BURST;
    spi_select_chip();
    spi_byte(_addr);
    spi_string( NULL, buffer, count );
    spi_deselect_chip();
}

void _write_burst_reg(uint8_t addr, uint8_t* buffer, uint8_t count)
{
    uint8_t _addr = (addr & 0x3F) | WRITE_BURST;
    spi_select_chip();
    spi_byte(_addr);
    spi_string( buffer, NULL, count );
    spi_deselect_chip();
}

void _write_single_patable(uint8_t value)
{
    WriteSingleReg(PATABLE, value);
}

void _write_burst_patable(uint8_t* buffer, uint8_t count)
{
    WriteBurstReg(PATABLE, buffer, count);
}



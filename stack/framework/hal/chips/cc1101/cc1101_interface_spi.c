
// functions used by cc1101_interface.c when accessing a CC1101 over SPI.
// when using CCS instead of cmake make sure to exclude this file from the build

#include "stdint.h"
#include "assert.h"

#include "hwspi.h"
#include "hwgpio.h"
#include "timer.h"

#include "cc1101_constants.h"
#include "cc1101_interface.h"


// turn on/off the debug prints
#ifdef LOG_PHY_ENABLED
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)
#endif


static void wait_ms(uint32_t ms) // TODO refactor: add busy waiting in timer api or use cb?
{
    /*
    timer_wait.next_event = (int32_t)((ms*1024)/1000);
    timer_add_event( &timer_wait );
    while(waiting);
    waiting = true;
    */

    uint32_t i;
    for( i=0 ; i<ms ; i++ )
    {
        volatile uint32_t n = 3200;
        while(n--);
    }
}

void _cc1101_gdo_isr(pin_id_t pin_id, uint8_t event_mask)
{
    assert(hw_gpio_pin_matches(pin_id, CC1101_GDO0_PIN));
}

void _cc1101_interface_init()
{
    spi_init();

    error_t err;
    err = hw_gpio_configure_interrupt(CC1101_GDO0_PIN, &_cc1101_gdo_isr, GPIO_FALLING_EDGE); assert(err == SUCCESS);
}

void _c1101_interface_set_interrupts_enabled(bool enable)
{
    if(enable)
    {
        //radioClearInterruptPendingLines(); // TODO clearing int needed?

        hw_gpio_enable_interrupt(CC1101_GDO0_PIN);
        // TODO GD02 not used for now
    }
    else
    {
        hw_gpio_disable_interrupt(CC1101_GDO0_PIN);
        // TODO GD02 not used for now
    }
}

uint8_t _c1101_interface_strobe(uint8_t strobe)
{
    spi_select_chip();
    uint8_t statusByte = spi_byte(strobe & 0x3F);
    spi_deselect_chip();

    return statusByte;
}

uint8_t _c1101_interface_reset_radio_core()
{
    spi_deselect_chip();
    //delayuS(30);
    wait_ms(1);
    spi_select_chip();
    //delayuS(30);
    wait_ms(1);
    spi_deselect_chip();
    //delayuS(45);
    wait_ms(1);

    cc1101_interface_strobe(RF_SRES);          // Reset the Radio Core
    return cc1101_interface_strobe(RF_SNOP);   // Get Radio Status
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

uint8_t _c1101_interface_read_single_reg(uint8_t addr)
{
    // Check for valid configuration register address, PATABLE or FIFO
    if ((addr<= 0x2E) || (addr>= 0x3E))
        return readreg(addr);
    else
        return readstatus(addr);
}

void _c1101_interface_write_single_reg(uint8_t addr, uint8_t value)
{
    spi_select_chip();
    spi_byte((addr & 0x3F));
    spi_byte(value);
    spi_deselect_chip();
}

void _c1101_interface_read_burst_reg(uint8_t addr, uint8_t* buffer, uint8_t count)
{
    uint8_t _addr = (addr & 0x3F) | READ_BURST;
    spi_select_chip();
    spi_byte(_addr);
    spi_string( NULL, buffer, count );
    spi_deselect_chip();
}

void _c1101_interface_write_burst_reg(uint8_t addr, uint8_t* buffer, uint8_t count)
{
    uint8_t _addr = (addr & 0x3F) | WRITE_BURST;
    spi_select_chip();
    spi_byte(_addr);
    spi_string( buffer, NULL, count );
    spi_deselect_chip();
}

void _c1101_interface_write_single_patable(uint8_t value)
{
    cc1101_interface_write_single_reg(PATABLE, value);
}

void _c1101_interface_write_burst_patable(uint8_t* buffer, uint8_t count)
{
    cc1101_interface_write_burst_reg(PATABLE, buffer, count);
}



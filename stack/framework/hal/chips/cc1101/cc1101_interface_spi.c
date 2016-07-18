/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


// functions used by cc1101_interface.c when accessing a CC1101 over SPI.
// when using CCS instead of cmake make sure to exclude this file from the build

#include "stdint.h"
#include "debug.h"

#include "hwspi.h"
#include "hwgpio.h"
#include "hwsystem.h"
#include "timer.h"
#include "scheduler.h"

#include "cc1101_constants.h"
#include "cc1101_interface.h"

 #include "platform.h"

// turn on/off the debug prints
#ifdef FRAMEWORK_PHY_LOG_ENABLED
#include "log.h"
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

static end_of_packet_isr_t end_of_packet_isr_callback;
void _c1101_interface_set_interrupts_enabled(bool enable);

void _cc1101_gdo_isr(pin_id_t pin_id, uint8_t event_mask)
{
    assert(hw_gpio_pin_matches(pin_id, CC1101_GDO0_PIN));
    //assert(event_mask == GPIO_FALLING_EDGE); // only using falling edge for now // TODO flank detection not supported on efm32gg for now

    _c1101_interface_set_interrupts_enabled(false);
    end_of_packet_isr_callback();
}

static spi_handle_t* spi;
static spi_slave_handle_t* spi_slave;


void _cc1101_interface_init(end_of_packet_isr_t end_of_packet_isr_cb)
{
    end_of_packet_isr_callback = end_of_packet_isr_cb;

    spi = spi_init(CC1101_SPI_USART, CC1101_SPI_BAUDRATE, 8, true, CC1101_SPI_LOCATION);
    spi_enable(spi);
    spi_slave = spi_init_slave(spi, CC1101_SPI_PIN_CS, true);

    error_t err;
    err = hw_gpio_configure_interrupt(CC1101_GDO0_PIN, &_cc1101_gdo_isr, GPIO_FALLING_EDGE); assert(err == SUCCESS);

    DPRINT("CC1101 SPI initialised\r\n");
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
    spi_select(spi_slave);
    uint8_t statusByte = spi_exchange_byte(spi_slave, strobe & 0x3F);
    spi_deselect(spi_slave);

    return statusByte;
}

uint8_t _c1101_interface_reset_radio_core()
{
    spi_deselect(spi_slave);
    hw_busy_wait(30);
    spi_select(spi_slave);
    hw_busy_wait(30);
    spi_deselect(spi_slave);
    hw_busy_wait(45);

    cc1101_interface_strobe(RF_SRES);          // Reset the Radio Core
    // TODO wait until completed
    return cc1101_interface_strobe(RF_SNOP);   // Get Radio Status
}

static uint8_t readreg(uint8_t addr)
{

    spi_select(spi_slave);
    spi_exchange_byte(spi_slave, (addr & 0x3F) | READ_SINGLE);
    // send dummy byte to receive reply
    uint8_t val = spi_exchange_byte(spi_slave, 0);
    spi_deselect(spi_slave);

    DPRINT("READ REG 0x%02X @0x%02X", val, addr);

    return val;
}

static uint8_t readstatus(uint8_t addr)
{
    uint8_t ret, ret2, data, data2;
    uint8_t _addr = (addr & 0x3F) | READ_BURST;
    spi_select(spi_slave);
    ret = spi_exchange_byte(spi_slave, _addr);
    data = spi_exchange_byte(spi_slave, 0); // send dummy byte to receive reply
    // Read value again until this matches previous reead (See CC1101's Errata for SPI read errors)
    while (true) {
        ret2 = spi_exchange_byte(spi_slave, _addr);
        // send dummy byte to receive reply
        data2 = spi_exchange_byte(spi_slave, 0);
        if (ret == ret2 && data == data2)
            break;
        else {
            ret = ret2;
            data = data2;
        }
    }

    spi_deselect(spi_slave);

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
    spi_select(spi_slave);
    spi_exchange_byte(spi_slave, (addr & 0x3F));
    spi_exchange_byte(spi_slave, value);
    spi_deselect(spi_slave);
}

void _c1101_interface_read_burst_reg(uint8_t addr, uint8_t* buffer, uint8_t count)
{
    uint8_t _addr = (addr & 0x3F) | READ_BURST;
    spi_select(spi_slave);
    spi_exchange_byte(spi_slave, _addr);
    spi_exchange_bytes(spi_slave,  NULL, buffer, count );
    spi_deselect(spi_slave);
}

void _c1101_interface_write_burst_reg(uint8_t addr, uint8_t* buffer, uint8_t count)
{
    uint8_t _addr = (addr & 0x3F) | WRITE_BURST;
    spi_select(spi_slave);
    spi_exchange_byte(spi_slave, _addr);
    spi_exchange_bytes(spi_slave,  buffer, NULL, count );
    spi_deselect(spi_slave);
}

void _c1101_interface_write_single_patable(uint8_t value)
{
    cc1101_interface_write_single_reg(PATABLE, value);
}

void _c1101_interface_write_burst_patable(uint8_t* buffer, uint8_t count)
{
    cc1101_interface_write_burst_reg(PATABLE, buffer, count);
}



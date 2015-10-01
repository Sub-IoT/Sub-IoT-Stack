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

#include "hwgpio.h"
#include "gpio.h"

#include "msp430.h"
#include "hw_regaccess.h"

#include "debug.h"


static uint16_t port_to_base_address(uint16_t port)
{
    switch(port)
    {
    case GPIO_PORT_P1:
        return __MSP430_BASEADDRESS_PORT1_R__;
    case GPIO_PORT_P2:
        return __MSP430_BASEADDRESS_PORT2_R__;
    case GPIO_PORT_P3:
        return __MSP430_BASEADDRESS_PORT3_R__;
    case GPIO_PORT_P5:
        return __MSP430_BASEADDRESS_PORT5_R__;
    case GPIO_PORT_PJ:
        return __MSP430_BASEADDRESS_PORTJ_R__;
    default:
        assert(0);
    }
}

__LINK_C void __gpio_init()
{
    // init all pins to output direction and drive low, to prevent floating
    PADIR = 0xFF; // note: portA == port1 + port2
    PAOUT = 0x00;
    P3DIR = 0xFF;
    P3OUT = 0x00;
    P5DIR = 0xFF;
    P5OUT = 0x00; // TODO used for 32kHz crystal
    PJDIR = 0xFF;
    PJOUT = 0x00;
}

__LINK_C error_t hw_gpio_configure_pin(pin_id_t pin_id, cc430_gpio_mode_t mode)
{
    // TODO check if already configured (return EALREADY)

    if(mode == GPIO_MODE_OUTPUT || mode == GPIO_MODE_OUTPUT_FULL_DRIVE_STRENGTH)
    {
        GPIO_setAsOutputPin(pin_id.port, pin_id.pin);
        if(mode == GPIO_MODE_OUTPUT_FULL_DRIVE_STRENGTH)
            GPIO_setDriveStrength(pin_id.port, pin_id.pin, GPIO_FULL_OUTPUT_DRIVE_STRENGTH);
    }
    else if(mode ==GPIO_MODE_INPUT)
        GPIO_setAsInputPin(pin_id.port, pin_id.pin);
    else if(mode == GPIO_MODE_INPUT_PULL_UP)
        GPIO_setAsInputPinWithPullUpResistor(pin_id.port, pin_id.pin);
    else if(mode == GPIO_MODE_INPUT_PULL_DOWN)
        GPIO_setAsInputPinWithPullDownResistor(pin_id.port, pin_id.pin);

    return SUCCESS;
}

__LINK_C error_t hw_gpio_set(pin_id_t pin_id)
{
    // TODO check if pin configured?
    GPIO_setOutputHighOnPin(pin_id.port, pin_id.pin);
    return SUCCESS;
}

__LINK_C error_t hw_gpio_clr(pin_id_t pin_id)
{
    // TODO check if pin configured?
    GPIO_setOutputLowOnPin(pin_id.port, pin_id.pin);
    return SUCCESS;
}

__LINK_C error_t hw_gpio_toggle(pin_id_t pin_id)
{
    // TODO check if pin configured?
    GPIO_toggleOutputOnPin(pin_id.port, pin_id.pin);
    return SUCCESS;
}

__LINK_C bool hw_gpio_get_out(pin_id_t pin_id)
{
    // TODO check if pin configured?

    // no function for this in driverlib
    uint16_t baseAddress = port_to_base_address(pin_id.port);
    uint16_t pin = pin_id.pin;

    // Shift by 8 if port is even (upper 8-bits)
    if((pin_id.port & 1) ^ 1)
    {
        pin <<= 8;
    }

    uint16_t reg = HWREG16(baseAddress + OFS_PAIN);
    return ((reg & (pin)) != 0);
}

__LINK_C bool hw_gpio_get_in(pin_id_t pin_id)
{
    // TODO check if pin configured?
    // TODO not tested yet
    return GPIO_getInputPinValue(pin_id.port, pin_id.pin) == GPIO_INPUT_PIN_HIGH;
}

static void gpio_int_callback(uint8_t pin)
{
    // TODO
}

__LINK_C error_t hw_gpio_configure_interrupt(pin_id_t pin_id, gpio_inthandler_t callback, uint8_t event_mask)
{
    // TODO
}
__LINK_C error_t hw_gpio_enable_interrupt(pin_id_t pin_id)
{
    // TODO
}

__LINK_C error_t hw_gpio_disable_interrupt(pin_id_t pin_id)
{
    // TODO
}

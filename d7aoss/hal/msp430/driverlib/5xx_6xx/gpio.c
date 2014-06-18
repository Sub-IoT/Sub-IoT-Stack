/* --COPYRIGHT--,BSD
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
//*****************************************************************************
//
//gpio.c - Driver for the DIgital I/O Module.
//
//*****************************************************************************
#include "../inc/hw_types.h"
#include "debug.h"
#include "gpio.h"
#ifdef  __IAR_SYSTEMS_ICC__
#include "../deprecated/IAR/msp430xgeneric.h"
#else
#include "../deprecated/CCS/msp430xgeneric.h"
#endif

//*****************************************************************************
//
//! This function configures the selected Pin as output pin
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are
//!             \b GPIO_PORT_P1,
//!             \b GPIO_PORT_P2,
//!             \b GPIO_PORT_P3,
//!             \b GPIO_PORT_P4,
//!             \b GPIO_PORT_P5,
//!             \b GPIO_PORT_P6,
//!             \b GPIO_PORT_P7,
//!             \b GPIO_PORT_P8,
//!             \b GPIO_PORT_P9,
//!             \b GPIO_PORT_P10,
//!             \b GPIO_PORT_P11,
//!             \b GPIO_PORT_PA,
//!             \b GPIO_PORT_PB,
//!             \b GPIO_PORT_PC,
//!             \b GPIO_PORT_PD,
//!             \b GPIO_PORT_PE,
//!             \b GPIO_PORT_PF,
//!             \b GPIO_PORT_PJ
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are
//!             \b GPIO_PIN0,
//!             \b GPIO_PIN1,
//!             \b GPIO_PIN2,
//!             \b GPIO_PIN3,
//!             \b GPIO_PIN4,
//!             \b GPIO_PIN5,
//!             \b GPIO_PIN6,
//!             \b GPIO_PIN7,
//!             \b GPIO_PIN8,
//!             \b GPIO_PIN9,
//!             \b GPIO_PIN10,
//!             \b GPIO_PIN11,
//!             \b GPIO_PIN12,
//!             \b GPIO_PIN13,
//!             \b GPIO_PIN14,
//!             \b GPIO_PIN15
//! Modified registers are \b PxSEL and \b PxDIR.
//!
//! \return None
//
//*****************************************************************************
void GPIO_setAsOutputPin ( unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    )
{
    ASSERT((GPIO_PORT_P1 == selectedPort) || (GPIO_PORT_P2 == selectedPort) ||
        (GPIO_PORT_P3 == selectedPort) || (GPIO_PORT_P4 == selectedPort) ||
        (GPIO_PORT_P5 == selectedPort) || (GPIO_PORT_P6 == selectedPort) ||
        (GPIO_PORT_P7 == selectedPort) || (GPIO_PORT_P8 == selectedPort) ||
        (GPIO_PORT_P9 == selectedPort) || (GPIO_PORT_P10 == selectedPort) ||
        (GPIO_PORT_P11 == selectedPort) || (GPIO_PORT_PA == selectedPort) ||
        (GPIO_PORT_PB == selectedPort) || (GPIO_PORT_PC == selectedPort) ||
        (GPIO_PORT_PD == selectedPort) || (GPIO_PORT_PE == selectedPort) ||
        (GPIO_PORT_PF == selectedPort) || (GPIO_PORT_PJ == selectedPort)
        );
  
     ASSERT(0x00 != (selectedPins & (GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + 
                                     GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN5 + 
                                     GPIO_PIN6 + GPIO_PIN7 + GPIO_PIN8 + 
                                     GPIO_PIN9 + GPIO_PIN10 + GPIO_PIN11 +
                                     GPIO_PIN12 + GPIO_PIN13 + GPIO_PIN14 +  
                                     GPIO_PIN15
             )));

    switch (selectedPort){
        case GPIO_PORT_P1:
            HWREGB(baseAddress + OFS_P1SEL) &= ~selectedPins;
            HWREGB(baseAddress + OFS_P1DIR) |= selectedPins;
            break;
        case GPIO_PORT_P2:
            HWREGB(baseAddress + OFS_P2SEL) &= ~selectedPins;
            HWREGB(baseAddress + OFS_P2DIR) |= selectedPins;
            break;
        case GPIO_PORT_PA:
            HWREG(baseAddress + OFS_PASEL) &= ~selectedPins;
            HWREG(baseAddress + OFS_PADIR) |= selectedPins;
            break;
    }
}

//*****************************************************************************
//
//! This function configures the selected Pin as input pin
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are
//!             \b GPIO_PORT_P1,
//!             \b GPIO_PORT_P2,
//!             \b GPIO_PORT_P3,
//!             \b GPIO_PORT_P4,
//!             \b GPIO_PORT_P5,
//!             \b GPIO_PORT_P6,
//!             \b GPIO_PORT_P7,
//!             \b GPIO_PORT_P8,
//!             \b GPIO_PORT_P9,
//!             \b GPIO_PORT_P10,
//!             \b GPIO_PORT_P11,
//!             \b GPIO_PORT_PA,
//!             \b GPIO_PORT_PB,
//!             \b GPIO_PORT_PC,
//!             \b GPIO_PORT_PD,
//!             \b GPIO_PORT_PE,
//!             \b GPIO_PORT_PF,
//!             \b GPIO_PORT_PJ
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are
//!             \b GPIO_PIN0,
//!             \b GPIO_PIN1,
//!             \b GPIO_PIN2,
//!             \b GPIO_PIN3,
//!             \b GPIO_PIN4,
//!             \b GPIO_PIN5,
//!             \b GPIO_PIN6,
//!             \b GPIO_PIN7,
//!             \b GPIO_PIN8,
//!             \b GPIO_PIN9,
//!             \b GPIO_PIN10,
//!             \b GPIO_PIN11,
//!             \b GPIO_PIN12,
//!             \b GPIO_PIN13,
//!             \b GPIO_PIN14,
//!             \b GPIO_PIN15
//! Modified registers are \b PxSEL, \b PxREN and \b PxDIR.
//! \return None
//
//*****************************************************************************
void GPIO_setAsInputPin (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    )
{
    ASSERT((GPIO_PORT_P1 == selectedPort) || (GPIO_PORT_P2 == selectedPort) ||
        (GPIO_PORT_P3 == selectedPort) || (GPIO_PORT_P4 == selectedPort) ||
        (GPIO_PORT_P5 == selectedPort) || (GPIO_PORT_P6 == selectedPort) ||
        (GPIO_PORT_P7 == selectedPort) || (GPIO_PORT_P8 == selectedPort) ||
        (GPIO_PORT_P9 == selectedPort) || (GPIO_PORT_P10 == selectedPort) ||
        (GPIO_PORT_P11 == selectedPort) || (GPIO_PORT_PA == selectedPort) ||
        (GPIO_PORT_PB == selectedPort) || (GPIO_PORT_PC == selectedPort) ||
        (GPIO_PORT_PD == selectedPort) || (GPIO_PORT_PE == selectedPort) ||
        (GPIO_PORT_PF == selectedPort) || (GPIO_PORT_PJ == selectedPort)
        );

    ASSERT(0x00 != (selectedPins & (GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + 
                                     GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN5 + 
                                     GPIO_PIN6 + GPIO_PIN7 + GPIO_PIN8 + 
                                     GPIO_PIN9 + GPIO_PIN10 + GPIO_PIN11 +
                                     GPIO_PIN12 + GPIO_PIN13 + GPIO_PIN14 +  
                                     GPIO_PIN15
             )));

    switch (selectedPort){
        case GPIO_PORT_P1:
            HWREGB(baseAddress + OFS_P1SEL) &= ~selectedPins;
            HWREGB(baseAddress + OFS_P1DIR) &= ~selectedPins;
            HWREGB(baseAddress + OFS_P1REN) &= ~selectedPins;
            break;
        case GPIO_PORT_P2:
            HWREGB(baseAddress + OFS_P2SEL) &= ~selectedPins;
            HWREGB(baseAddress + OFS_P2DIR) &= ~selectedPins;
            HWREGB(baseAddress + OFS_P2REN) &= ~selectedPins;
            break;
        case GPIO_PORT_PA:
            HWREG(baseAddress + OFS_PASEL) &= ~selectedPins;
            HWREG(baseAddress + OFS_PADIR) &= ~selectedPins;
            HWREG(baseAddress + OFS_PAREN) &= ~selectedPins;
            break;
    }
}

//*****************************************************************************
//
//! This function configures the peripheral module function in output direction
//! for the selected pin.
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are \b GPIO_PORT_P1, \b GPIO_PORT_P2,
//!             \b GPIO_PORT_P3, \b GPIO_PORT_P4,\b GPIO_PORT_P5,
//!             \b GPIO_PORT_P6, \b GPIO_PORT_P7,\b GPIO_PORT_P8,
//!             \b GPIO_PORT_P9, \b GPIO_PORT_P10,
//!             \b GPIO_PORT_P11, \b GPIO_PORT_PA,
//!             \b GPIO_PORT_PB, \b GPIO_PORT_PC,
//!             \b GPIO_PORT_PD, \b GPIO_PORT_PE,
//!             \b GPIO_PORT_PF, \b GPIO_PORT_PJ
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are \b GPIO_PIN0, \b GPIO_PIN1, \b GPIO_PIN2,
//!             \b GPIO_PIN3, \b GPIO_PIN4, \b GPIO_PIN5, \b GPIO_PIN6,
//!             \b GPIO_PIN7,\b GPIO_PIN8,\b GPIO_PIN9,\b GPIO_PIN10,
//!             \b GPIO_PIN11,\b GPIO_PIN12,\b GPIO_PIN13,\b GPIO_PIN14,
//!             \b GPIO_PIN15
///! Modified registers are \b PxSEL and \b PxDIR.
//!
//! \return None
//
//*****************************************************************************
void GPIO_setAsPeripheralModuleFunctionOutputPin ( unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    )
{
    ASSERT((GPIO_PORT_P1 == selectedPort) || (GPIO_PORT_P2 == selectedPort) ||
        (GPIO_PORT_P3 == selectedPort) || (GPIO_PORT_P4 == selectedPort) ||
        (GPIO_PORT_P5 == selectedPort) || (GPIO_PORT_P6 == selectedPort) ||
        (GPIO_PORT_P7 == selectedPort) || (GPIO_PORT_P8 == selectedPort) ||
        (GPIO_PORT_P9 == selectedPort) || (GPIO_PORT_P10 == selectedPort) ||
        (GPIO_PORT_P11 == selectedPort) || (GPIO_PORT_PA == selectedPort) ||
        (GPIO_PORT_PB == selectedPort) || (GPIO_PORT_PC == selectedPort) ||
        (GPIO_PORT_PD == selectedPort) || (GPIO_PORT_PE == selectedPort) ||
        (GPIO_PORT_PF == selectedPort) || (GPIO_PORT_PJ == selectedPort)
        );

    ASSERT(0x00 != (selectedPins & (GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + 
                                     GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN5 + 
                                     GPIO_PIN6 + GPIO_PIN7 + GPIO_PIN8 + 
                                     GPIO_PIN9 + GPIO_PIN10 + GPIO_PIN11 +
                                     GPIO_PIN12 + GPIO_PIN13 + GPIO_PIN14 +  
                                     GPIO_PIN15
             )));

    switch (selectedPort){
        case GPIO_PORT_P1:
            HWREGB(baseAddress + OFS_P1SEL) |= selectedPins;
            HWREGB(baseAddress + OFS_P1DIR) |= selectedPins;
            break;
        case GPIO_PORT_P2:
            HWREGB(baseAddress + OFS_P2SEL) |= selectedPins;
            HWREGB(baseAddress + OFS_P2DIR) |= selectedPins;
            break;
        case GPIO_PORT_PA:
            HWREG(baseAddress + OFS_PASEL) |= selectedPins;
            HWREG(baseAddress + OFS_PADIR) |= selectedPins;
            break;
    }
}

//*****************************************************************************
//
//! This function configures the peripheral module function in input direction
//! for the selected pin.
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are \b GPIO_PORT_P1, \b GPIO_PORT_P2,
//!             \b GPIO_PORT_P3, \b GPIO_PORT_P4,\b GPIO_PORT_P5,
//!             \b GPIO_PORT_P6, \b GPIO_PORT_P7,\b GPIO_PORT_P8,
//!             \b GPIO_PORT_P9, \b GPIO_PORT_P10,
//!             \b GPIO_PORT_P11, \b GPIO_PORT_PA,
//!             \b GPIO_PORT_PB, \b GPIO_PORT_PC,
//!             \b GPIO_PORT_PD, \b GPIO_PORT_PE,
//!             \b GPIO_PORT_PF, \b GPIO_PORT_PJ
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are \b GPIO_PIN0, \b GPIO_PIN1, \b GPIO_PIN2,
//!             \b GPIO_PIN3, \b GPIO_PIN4, \b GPIO_PIN5, \b GPIO_PIN6,
//!             \b GPIO_PIN7,\b GPIO_PIN8,\b GPIO_PIN9,\b GPIO_PIN10,
//!             \b GPIO_PIN11,\b GPIO_PIN12,\b GPIO_PIN13,\b GPIO_PIN14,
//!             \b GPIO_PIN15
//! Modified registers are \b PxSEL and \b PxDIR.
//!
//! \return None
//
//*****************************************************************************
void GPIO_setAsPeripheralModuleFunctionInputPin ( unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    )
{
    ASSERT((GPIO_PORT_P1 == selectedPort) || (GPIO_PORT_P2 == selectedPort) ||
        (GPIO_PORT_P3 == selectedPort) || (GPIO_PORT_P4 == selectedPort) ||
        (GPIO_PORT_P5 == selectedPort) || (GPIO_PORT_P6 == selectedPort) ||
        (GPIO_PORT_P7 == selectedPort) || (GPIO_PORT_P8 == selectedPort) ||
        (GPIO_PORT_P9 == selectedPort) || (GPIO_PORT_P10 == selectedPort) ||
        (GPIO_PORT_P11 == selectedPort) || (GPIO_PORT_PA == selectedPort) ||
        (GPIO_PORT_PB == selectedPort) || (GPIO_PORT_PC == selectedPort) ||
        (GPIO_PORT_PD == selectedPort) || (GPIO_PORT_PE == selectedPort) ||
        (GPIO_PORT_PF == selectedPort) || (GPIO_PORT_PJ == selectedPort)
        );

    ASSERT(0x00 != (selectedPins & (GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + 
                                     GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN5 + 
                                     GPIO_PIN6 + GPIO_PIN7 + GPIO_PIN8 + 
                                     GPIO_PIN9 + GPIO_PIN10 + GPIO_PIN11 +
                                     GPIO_PIN12 + GPIO_PIN13 + GPIO_PIN14 +  
                                     GPIO_PIN15
             )));
    

    switch (selectedPort){
        case GPIO_PORT_P1:
            HWREGB(baseAddress + OFS_P1SEL) |= selectedPins;
            HWREGB(baseAddress + OFS_P1DIR) &= ~selectedPins;
            break;
        case GPIO_PORT_P2:
            HWREGB(baseAddress + OFS_P2SEL) |= selectedPins;
            HWREGB(baseAddress + OFS_P2DIR) &= ~selectedPins;
            break;
        case GPIO_PORT_PA:
            HWREG(baseAddress + OFS_PASEL) |= selectedPins;
            HWREG(baseAddress + OFS_PADIR) &= ~selectedPins;
            break;
    }
}

//*****************************************************************************
//
//! This function sets output HIGH on the selected Pin
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are \b GPIO_PORT_P1, \b GPIO_PORT_P2,
//!             \b GPIO_PORT_P3, \b GPIO_PORT_P4,\b GPIO_PORT_P5,
//!             \b GPIO_PORT_P6, \b GPIO_PORT_P7,\b GPIO_PORT_P8,
//!             \b GPIO_PORT_P9, \b GPIO_PORT_P10,
//!             \b GPIO_PORT_P11, \b GPIO_PORT_PA,
//!             \b GPIO_PORT_PB, \b GPIO_PORT_PC,
//!             \b GPIO_PORT_PD, \b GPIO_PORT_PE,
//!             \b GPIO_PORT_PF, \b GPIO_PORT_PJ
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are \b GPIO_PIN0, \b GPIO_PIN1, \b GPIO_PIN2,
//!             \b GPIO_PIN3, \b GPIO_PIN4, \b GPIO_PIN5, \b GPIO_PIN6,
//!             \b GPIO_PIN7,\b GPIO_PIN8,\b GPIO_PIN9,\b GPIO_PIN10,
//!             \b GPIO_PIN11,\b GPIO_PIN12,\b GPIO_PIN13,\b GPIO_PIN14,
//!             \b GPIO_PIN15
//! Modified registers are \b PxOUT.
//!
//! \return None
//
//*****************************************************************************
void GPIO_setOutputHighOnPin (  unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    )
{
    ASSERT((GPIO_PORT_P1 == selectedPort) || (GPIO_PORT_P2 == selectedPort) ||
        (GPIO_PORT_P3 == selectedPort) || (GPIO_PORT_P4 == selectedPort) ||
        (GPIO_PORT_P5 == selectedPort) || (GPIO_PORT_P6 == selectedPort) ||
        (GPIO_PORT_P7 == selectedPort) || (GPIO_PORT_P8 == selectedPort) ||
        (GPIO_PORT_P9 == selectedPort) || (GPIO_PORT_P10 == selectedPort) ||
        (GPIO_PORT_P11 == selectedPort) || (GPIO_PORT_PA == selectedPort) ||
        (GPIO_PORT_PB == selectedPort) || (GPIO_PORT_PC == selectedPort) ||
        (GPIO_PORT_PD == selectedPort) || (GPIO_PORT_PE == selectedPort) ||
        (GPIO_PORT_PF == selectedPort) || (GPIO_PORT_PJ == selectedPort)
        );

    ASSERT(0x00 != (selectedPins & (GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + 
                                     GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN5 + 
                                     GPIO_PIN6 + GPIO_PIN7 + GPIO_PIN8 + 
                                     GPIO_PIN9 + GPIO_PIN10 + GPIO_PIN11 +
                                     GPIO_PIN12 + GPIO_PIN13 + GPIO_PIN14 +  
                                     GPIO_PIN15
             )));

    switch (selectedPort){
        case GPIO_PORT_P1:
            HWREGB(baseAddress + OFS_P1OUT) |= selectedPins;
            break;
        case GPIO_PORT_P2:
            HWREGB(baseAddress + OFS_P2OUT) |= selectedPins;
            break;
        case GPIO_PORT_PA:
            HWREG(baseAddress + OFS_PAOUT) |= selectedPins;
            break;
    }
}

//*****************************************************************************
//
//! This function sets output LOW on the selected Pin
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are \b GPIO_PORT_P1, \b GPIO_PORT_P2,
//!             \b GPIO_PORT_P3, \b GPIO_PORT_P4,\b GPIO_PORT_P5,
//!             \b GPIO_PORT_P6, \b GPIO_PORT_P7,\b GPIO_PORT_P8,
//!             \b GPIO_PORT_P9, \b GPIO_PORT_P10,
//!             \b GPIO_PORT_P11, \b GPIO_PORT_PA,
//!             \b GPIO_PORT_PB, \b GPIO_PORT_PC,
//!             \b GPIO_PORT_PD, \b GPIO_PORT_PE,
//!             \b GPIO_PORT_PF, \b GPIO_PORT_PJ
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are \b GPIO_PIN0, \b GPIO_PIN1, \b GPIO_PIN2,
//!             \b GPIO_PIN3, \b GPIO_PIN4, \b GPIO_PIN5, \b GPIO_PIN6,
//!             \b GPIO_PIN7,\b GPIO_PIN8,\b GPIO_PIN9,\b GPIO_PIN10,
//!             \b GPIO_PIN11,\b GPIO_PIN12,\b GPIO_PIN13,\b GPIO_PIN14,
//!             \b GPIO_PIN15
//! Modified registers are \b PxOUT.
//!
//! \return None
//
//*****************************************************************************
void GPIO_setOutputLowOnPin (  unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    )
{
    ASSERT((GPIO_PORT_P1 == selectedPort) ||
        (GPIO_PORT_P2 == selectedPort) ||
        (GPIO_PORT_P3 == selectedPort) ||
        (GPIO_PORT_P4 == selectedPort) ||
        (GPIO_PORT_P5 == selectedPort) ||
        (GPIO_PORT_P6 == selectedPort) ||
        (GPIO_PORT_P7 == selectedPort) ||
        (GPIO_PORT_P8 == selectedPort) ||
        (GPIO_PORT_P9 == selectedPort) ||
        (GPIO_PORT_P10 == selectedPort) ||
        (GPIO_PORT_P11 == selectedPort) ||
        (GPIO_PORT_PA == selectedPort) ||
        (GPIO_PORT_PB == selectedPort) ||
        (GPIO_PORT_PC == selectedPort) ||
        (GPIO_PORT_PD == selectedPort) ||
        (GPIO_PORT_PE == selectedPort) ||
        (GPIO_PORT_PF == selectedPort) ||
        (GPIO_PORT_PJ == selectedPort)
        );

    ASSERT(0x00 != (selectedPins & (GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + 
                                     GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN5 + 
                                     GPIO_PIN6 + GPIO_PIN7 + GPIO_PIN8 + 
                                     GPIO_PIN9 + GPIO_PIN10 + GPIO_PIN11 +
                                     GPIO_PIN12 + GPIO_PIN13 + GPIO_PIN14 +  
                                     GPIO_PIN15
             )));

    switch (selectedPort){
        case GPIO_PORT_P1:
            HWREGB(baseAddress + OFS_P1OUT) &= ~selectedPins;
            break;
        case GPIO_PORT_P2:
            HWREGB(baseAddress + OFS_P2OUT) &= ~selectedPins;
            break;
        case GPIO_PORT_PA:
            HWREG(baseAddress + OFS_PAOUT) &= ~selectedPins;
            break;
    }
}

//*****************************************************************************
//
//! This function toggles the output on the selected Pin
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are \b GPIO_PORT_P1, \b GPIO_PORT_P2,
//!             \b GPIO_PORT_P3, \b GPIO_PORT_P4,\b GPIO_PORT_P5,
//!             \b GPIO_PORT_P6, \b GPIO_PORT_P7,\b GPIO_PORT_P8,
//!             \b GPIO_PORT_P9, \b GPIO_PORT_P10,
//!             \b GPIO_PORT_P11, \b GPIO_PORT_PA,
//!             \b GPIO_PORT_PB, \b GPIO_PORT_PC,
//!             \b GPIO_PORT_PD, \b GPIO_PORT_PE,
//!             \b GPIO_PORT_PF, \b GPIO_PORT_PJ
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are \b GPIO_PIN0, \b GPIO_PIN1, \b GPIO_PIN2,
//!             \b GPIO_PIN3, \b GPIO_PIN4, \b GPIO_PIN5, \b GPIO_PIN6,
//!             \b GPIO_PIN7,\b GPIO_PIN8,\b GPIO_PIN9,\b GPIO_PIN10,
//!             \b GPIO_PIN11,\b GPIO_PIN12,\b GPIO_PIN13,\b GPIO_PIN14,
//!             \b GPIO_PIN15
//! Modified registers are \b PxOUT.
//!
//! \return None
//
//*****************************************************************************
void GPIO_toggleOutputOnPin (  unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    )
{
    ASSERT((GPIO_PORT_P1 == selectedPort) ||
        (GPIO_PORT_P2 == selectedPort) ||
        (GPIO_PORT_P3 == selectedPort) ||
        (GPIO_PORT_P4 == selectedPort) ||
        (GPIO_PORT_P5 == selectedPort) ||
        (GPIO_PORT_P6 == selectedPort) ||
        (GPIO_PORT_P7 == selectedPort) ||
        (GPIO_PORT_P8 == selectedPort) ||
        (GPIO_PORT_P9 == selectedPort) ||
        (GPIO_PORT_P10 == selectedPort) ||
        (GPIO_PORT_P11 == selectedPort) ||
        (GPIO_PORT_PA == selectedPort) ||
        (GPIO_PORT_PB == selectedPort) ||
        (GPIO_PORT_PC == selectedPort) ||
        (GPIO_PORT_PD == selectedPort) ||
        (GPIO_PORT_PE == selectedPort) ||
        (GPIO_PORT_PF == selectedPort) ||
        (GPIO_PORT_PJ == selectedPort)
        );

    ASSERT(0x00 != (selectedPins & (GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + 
                                     GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN5 + 
                                     GPIO_PIN6 + GPIO_PIN7 + GPIO_PIN8 + 
                                     GPIO_PIN9 + GPIO_PIN10 + GPIO_PIN11 +
                                     GPIO_PIN12 + GPIO_PIN13 + GPIO_PIN14 +  
                                     GPIO_PIN15
             )));

    switch (selectedPort){
        case GPIO_PORT_P1:
            HWREGB(baseAddress + OFS_P1OUT) ^= selectedPins;
            break;
        case GPIO_PORT_P2:
            HWREGB(baseAddress + OFS_P2OUT) ^= selectedPins;
            break;
        case GPIO_PORT_PA:
            HWREG(baseAddress + OFS_PAOUT) ^= selectedPins;
            break;
    }
}

//*****************************************************************************
//
//! This function sets the selected Pin in input Mode wuth Pull Down resistor
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are \b GPIO_PORT_P1, \b GPIO_PORT_P2,
//!             \b GPIO_PORT_P3, \b GPIO_PORT_P4,\b GPIO_PORT_P5,
//!             \b GPIO_PORT_P6, \b GPIO_PORT_P7,\b GPIO_PORT_P8,
//!             \b GPIO_PORT_P9, \b GPIO_PORT_P10,
//!             \b GPIO_PORT_P11, \b GPIO_PORT_PA,
//!             \b GPIO_PORT_PB, \b GPIO_PORT_PC,
//!             \b GPIO_PORT_PD, \b GPIO_PORT_PE,
//!             \b GPIO_PORT_PF, \b GPIO_PORT_PJ
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are \b GPIO_PIN0, \b GPIO_PIN1, \b GPIO_PIN2,
//!             \b GPIO_PIN3, \b GPIO_PIN4, \b GPIO_PIN5, \b GPIO_PIN6,
//!             \b GPIO_PIN7,\b GPIO_PIN8,\b GPIO_PIN9,\b GPIO_PIN10,
//!             \b GPIO_PIN11,\b GPIO_PIN12,\b GPIO_PIN13,\b GPIO_PIN14,
//!             \b GPIO_PIN15
//! Modified registers are \b PxREN, \b PxOUT and \b PxDIR.
//!
//! \return None
//
//*****************************************************************************
void GPIO_setAsInputPinWithPullDownresistor (  unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    )
{
    ASSERT((GPIO_PORT_P1 == selectedPort) ||
        (GPIO_PORT_P2 == selectedPort) ||
        (GPIO_PORT_P3 == selectedPort) ||
        (GPIO_PORT_P4 == selectedPort) ||
        (GPIO_PORT_P5 == selectedPort) ||
        (GPIO_PORT_P6 == selectedPort) ||
        (GPIO_PORT_P7 == selectedPort) ||
        (GPIO_PORT_P8 == selectedPort) ||
        (GPIO_PORT_P9 == selectedPort) ||
        (GPIO_PORT_P10 == selectedPort) ||
        (GPIO_PORT_P11 == selectedPort) ||
        (GPIO_PORT_PA == selectedPort) ||
        (GPIO_PORT_PB == selectedPort) ||
        (GPIO_PORT_PC == selectedPort) ||
        (GPIO_PORT_PD == selectedPort) ||
        (GPIO_PORT_PE == selectedPort) ||
        (GPIO_PORT_PF == selectedPort) ||
        (GPIO_PORT_PJ == selectedPort)
        );

    ASSERT(0x00 != (selectedPins & (GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + 
                                     GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN5 + 
                                     GPIO_PIN6 + GPIO_PIN7 + GPIO_PIN8 + 
                                     GPIO_PIN9 + GPIO_PIN10 + GPIO_PIN11 +
                                     GPIO_PIN12 + GPIO_PIN13 + GPIO_PIN14 +  
                                     GPIO_PIN15
             )));
    

    switch (selectedPort){
        case GPIO_PORT_P1:
            HWREGB(baseAddress + OFS_P1SEL) &= ~selectedPins;
            HWREGB(baseAddress + OFS_P1DIR) &= ~selectedPins;
            HWREGB(baseAddress + OFS_P1REN) |= selectedPins;
            HWREGB(baseAddress + OFS_P1OUT) &= ~selectedPins;
            break;
        case GPIO_PORT_P2:
            HWREGB(baseAddress + OFS_P2SEL) &= ~selectedPins;
            HWREGB(baseAddress + OFS_P2DIR) &= ~selectedPins;
            HWREGB(baseAddress + OFS_P2REN) |= selectedPins;
            HWREGB(baseAddress + OFS_P2OUT) &= ~selectedPins;
            break;
        case GPIO_PORT_PA:
            HWREG(baseAddress + OFS_PASEL) &= ~selectedPins;
            HWREG(baseAddress + OFS_PADIR) &= ~selectedPins;
            HWREG(baseAddress + OFS_PAREN) |= selectedPins;
            HWREG(baseAddress + OFS_PAOUT) &= ~selectedPins;
            break;
    }
}

//*****************************************************************************
//
//! This function sets the selected Pin in input Mode wuth Pull Up resistor
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are \b GPIO_PORT_P1, \b GPIO_PORT_P2,
//!             \b GPIO_PORT_P3, \b GPIO_PORT_P4,\b GPIO_PORT_P5,
//!             \b GPIO_PORT_P6, \b GPIO_PORT_P7,\b GPIO_PORT_P8,
//!             \b GPIO_PORT_P9, \b GPIO_PORT_P10,
//!             \b GPIO_PORT_P11, \b GPIO_PORT_PA,
//!             \b GPIO_PORT_PB, \b GPIO_PORT_PC,
//!             \b GPIO_PORT_PD, \b GPIO_PORT_PE,
//!             \b GPIO_PORT_PF, \b GPIO_PORT_PJ
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are \b GPIO_PIN0, \b GPIO_PIN1, \b GPIO_PIN2,
//!             \b GPIO_PIN3, \b GPIO_PIN4, \b GPIO_PIN5, \b GPIO_PIN6,
//!             \b GPIO_PIN7,\b GPIO_PIN8,\b GPIO_PIN9,\b GPIO_PIN10,
//!             \b GPIO_PIN11,\b GPIO_PIN12,\b GPIO_PIN13,\b GPIO_PIN14,
//!             \b GPIO_PIN15
//! Modified registers are \b PxREN, \b PxOUT and \b PxDIR.
//!
//! \return None
//
//*****************************************************************************
void GPIO_setAsInputPinWithPullUpresistor (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    )
{
    ASSERT((GPIO_PORT_P1 == selectedPort) ||
        (GPIO_PORT_P2 == selectedPort) ||
        (GPIO_PORT_P3 == selectedPort) ||
        (GPIO_PORT_P4 == selectedPort) ||
        (GPIO_PORT_P5 == selectedPort) ||
        (GPIO_PORT_P6 == selectedPort) ||
        (GPIO_PORT_P7 == selectedPort) ||
        (GPIO_PORT_P8 == selectedPort) ||
        (GPIO_PORT_P9 == selectedPort) ||
        (GPIO_PORT_P10 == selectedPort) ||
        (GPIO_PORT_P11 == selectedPort) ||
        (GPIO_PORT_PA == selectedPort) ||
        (GPIO_PORT_PB == selectedPort) ||
        (GPIO_PORT_PC == selectedPort) ||
        (GPIO_PORT_PD == selectedPort) ||
        (GPIO_PORT_PE == selectedPort) ||
        (GPIO_PORT_PF == selectedPort) ||
        (GPIO_PORT_PJ == selectedPort)
        );

    ASSERT(0x00 != (selectedPins & (GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + 
                                     GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN5 + 
                                     GPIO_PIN6 + GPIO_PIN7 + GPIO_PIN8 + 
                                     GPIO_PIN9 + GPIO_PIN10 + GPIO_PIN11 +
                                     GPIO_PIN12 + GPIO_PIN13 + GPIO_PIN14 +  
                                     GPIO_PIN15
             )));

    switch (selectedPort){
        case GPIO_PORT_P1:
            HWREGB(baseAddress + OFS_P1SEL) &= ~selectedPins;
            HWREGB(baseAddress + OFS_P1DIR) &= ~selectedPins;
            HWREGB(baseAddress + OFS_P1REN) |= selectedPins;
            HWREGB(baseAddress + OFS_P1OUT) |= selectedPins;
            break;
        case GPIO_PORT_P2:
            HWREGB(baseAddress + OFS_P2SEL) &= ~selectedPins;
            HWREGB(baseAddress + OFS_P2DIR) &= ~selectedPins;
            HWREGB(baseAddress + OFS_P2REN) |= selectedPins;
            HWREGB(baseAddress + OFS_P2OUT) |= selectedPins;
            break;
        case GPIO_PORT_PA:
            HWREG(baseAddress + OFS_PASEL) &= ~selectedPins;
            HWREG(baseAddress + OFS_PADIR) &= ~selectedPins;
            HWREG(baseAddress + OFS_PAREN) |= selectedPins;
            HWREG(baseAddress + OFS_PAOUT) |= selectedPins;
            break;
    }
}

//*****************************************************************************
//
//! This function gets the input value on the selected pin
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are \b GPIO_PORT_P1, \b GPIO_PORT_P2,
//!             \b GPIO_PORT_P3, \b GPIO_PORT_P4,\b GPIO_PORT_P5,
//!             \b GPIO_PORT_P6, \b GPIO_PORT_P7,\b GPIO_PORT_P8,
//!             \b GPIO_PORT_P9, \b GPIO_PORT_P10,
//!             \b GPIO_PORT_P11, \b GPIO_PORT_PA,
//!             \b GPIO_PORT_PB, \b GPIO_PORT_PC,
//!             \b GPIO_PORT_PD, \b GPIO_PORT_PE,
//!             \b GPIO_PORT_PF, \b GPIO_PORT_PJ
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are \b GPIO_PIN0, \b GPIO_PIN1, \b GPIO_PIN2,
//!             \b GPIO_PIN3, \b GPIO_PIN4, \b GPIO_PIN5, \b GPIO_PIN6,
//!             \b GPIO_PIN7,\b GPIO_PIN8,\b GPIO_PIN9,\b GPIO_PIN10,
//!             \b GPIO_PIN11,\b GPIO_PIN12,\b GPIO_PIN13,\b GPIO_PIN14,
//!             \b GPIO_PIN15
//! Modified registers are \b PxIN.
//!
//! \return Input value on Pin - \b GPIO_INPUT_PIN_HIGH,
//!                              \b GPIO_INPUT_PIN_LOW
//
//*****************************************************************************
unsigned short GPIO_getInputPinValue (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    )
{
    ASSERT((GPIO_PORT_P1 == selectedPort) || (GPIO_PORT_P2 == selectedPort) ||
        (GPIO_PORT_P3 == selectedPort) || (GPIO_PORT_P4 == selectedPort) ||
        (GPIO_PORT_P5 == selectedPort) || (GPIO_PORT_P6 == selectedPort) ||
        (GPIO_PORT_P7 == selectedPort) || (GPIO_PORT_P8 == selectedPort) ||
        (GPIO_PORT_P9 == selectedPort) || (GPIO_PORT_P10 == selectedPort) ||
        (GPIO_PORT_P11 == selectedPort) || (GPIO_PORT_PA == selectedPort) ||
        (GPIO_PORT_PB == selectedPort) || (GPIO_PORT_PC == selectedPort) ||
        (GPIO_PORT_PD == selectedPort) || (GPIO_PORT_PE == selectedPort) ||
        (GPIO_PORT_PF == selectedPort) || (GPIO_PORT_PJ == selectedPort)
        );

    ASSERT(0x00 != (selectedPins & (GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + 
                                     GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN5 + 
                                     GPIO_PIN6 + GPIO_PIN7 + GPIO_PIN8 + 
                                     GPIO_PIN9 + GPIO_PIN10 + GPIO_PIN11 +
                                     GPIO_PIN12 + GPIO_PIN13 + GPIO_PIN14 +  
                                     GPIO_PIN15
             )));

    unsigned int inputPinValue = 0;

    switch (selectedPort){
        case GPIO_PORT_P1:
            inputPinValue = HWREGB(baseAddress + OFS_P1IN) & selectedPins;
            break;
        case GPIO_PORT_P2:
            inputPinValue = HWREGB(baseAddress + OFS_P2IN) & selectedPins;
            break;
        case GPIO_PORT_PA:
            inputPinValue = HWREG(baseAddress + OFS_PAIN) & selectedPins;
            break;
    }

    if (inputPinValue > 0){
        return ( GPIO_INPUT_PIN_HIGH) ;
    }
    return ( GPIO_INPUT_PIN_LOW) ;
}

//*****************************************************************************
//
//! This function enables the port interrupt on the selected pin.
//!     Note that only Port 1,2, A have this capability
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are \b GPIO_PORT_P1, \b GPIO_PORT_P2,
//!             \b GPIO_PORT_PA
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are \b GPIO_PIN0, \b GPIO_PIN1, \b GPIO_PIN2,
//!             \b GPIO_PIN3, \b GPIO_PIN4, \b GPIO_PIN5, \b GPIO_PIN6,
//!             \b GPIO_PIN7,\b GPIO_PIN8,\b GPIO_PIN9,\b GPIO_PIN10,
//!             \b GPIO_PIN11,\b GPIO_PIN12,\b GPIO_PIN13,\b GPIO_PIN14,
//!             \b GPIO_PIN15
//! Modified registers are \b PxIE.
//!
//! \return None
//
//*****************************************************************************
void GPIO_enableInterrupt (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    )
{
    ASSERT((GPIO_PORT_P1 == selectedPort) ||
        (GPIO_PORT_P2 == selectedPort) ||
        (GPIO_PORT_PA == selectedPort)
        );

    ASSERT(0x00 != (selectedPins & (GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + 
                                     GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN5 + 
                                     GPIO_PIN6 + GPIO_PIN7 + GPIO_PIN8 + 
                                     GPIO_PIN9 + GPIO_PIN10 + GPIO_PIN11 +
                                     GPIO_PIN12 + GPIO_PIN13 + GPIO_PIN14 +  
                                     GPIO_PIN15
             )));

    switch (selectedPort){
        case GPIO_PORT_P1:
        HWREGB(baseAddress + OFS_P1IFG) &= ~selectedPins;
            HWREGB(baseAddress + OFS_P1IE) |= selectedPins;
            break;
        case GPIO_PORT_P2:
        HWREGB(baseAddress + OFS_P2IFG) &= ~selectedPins;
            HWREGB(baseAddress + OFS_P2IE) |= selectedPins;
            break;
        case GPIO_PORT_PA:
        HWREG(baseAddress + OFS_PAIFG) &= ~selectedPins;
            HWREG(baseAddress + OFS_PAIE) |= selectedPins;
            break;
    }
}

//*****************************************************************************
//
//! This function disables the port interrupt on the selected pin.
//!     Note that only Port 1,2, A have this capability
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are \b GPIO_PORT_P1, \b GPIO_PORT_P2,
//!             \b GPIO_PORT_PA
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are \b GPIO_PIN0, \b GPIO_PIN1, \b GPIO_PIN2,
//!             \b GPIO_PIN3, \b GPIO_PIN4, \b GPIO_PIN5, \b GPIO_PIN6,
//!             \b GPIO_PIN7,\b GPIO_PIN8,\b GPIO_PIN9,\b GPIO_PIN10,
//!             \b GPIO_PIN11,\b GPIO_PIN12,\b GPIO_PIN13,\b GPIO_PIN14,
//!             \b GPIO_PIN15
//! Modified registers are \b PxIE.
//!
//! \return None
//
//*****************************************************************************
void GPIO_disableInterrupt (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    )
{
    ASSERT((GPIO_PORT_P1 == selectedPort) ||
        (GPIO_PORT_P2 == selectedPort) ||
        (GPIO_PORT_PA == selectedPort)
        );

    ASSERT(0x00 != (selectedPins & (GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + 
                                     GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN5 + 
                                     GPIO_PIN6 + GPIO_PIN7 + GPIO_PIN8 + 
                                     GPIO_PIN9 + GPIO_PIN10 + GPIO_PIN11 +
                                     GPIO_PIN12 + GPIO_PIN13 + GPIO_PIN14 +  
                                     GPIO_PIN15
             )));

    switch (selectedPort){
        case GPIO_PORT_P1:
            HWREGB(baseAddress + OFS_P1IE) &= ~selectedPins;
            break;
        case GPIO_PORT_P2:
            HWREGB(baseAddress + OFS_P2IE) &= ~selectedPins;
            break;
        case GPIO_PORT_PA:
            HWREG(baseAddress + OFS_PAIE) &= ~selectedPins;
            break;
    }
}

//*****************************************************************************
//
//! DEPRECATED - Spelling Error Fixed
//! This function disables the port interrupt on the selected pin.
//!     Note that only Port 1,2, A have this capability
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are \b GPIO_PORT_P1, \b GPIO_PORT_P2,
//!             \b GPIO_PORT_PA
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are \b GPIO_PIN0, \b GPIO_PIN1, \b GPIO_PIN2,
//!             \b GPIO_PIN3, \b GPIO_PIN4, \b GPIO_PIN5, \b GPIO_PIN6,
//!             \b GPIO_PIN7,\b GPIO_PIN8,\b GPIO_PIN9,\b GPIO_PIN10,
//!             \b GPIO_PIN11,\b GPIO_PIN12,\b GPIO_PIN13,\b GPIO_PIN14,
//!             \b GPIO_PIN15
//! Modified registers are \b PxIE.
//!
//! \return None
//
//*****************************************************************************
void GPIO_disbleInterrupt (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    )
{
    GPIO_disableInterrupt (baseAddress,
        selectedPort,
    	selectedPins
    );
}

//*****************************************************************************
//
//! This function gets the interrupt status of the selected pin.
//!     Note that only Port 1,2, A have this capability
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are \b GPIO_PORT_P1, \b GPIO_PORT_P2,
//!             \b GPIO_PORT_PA
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are \b GPIO_PIN0, \b GPIO_PIN1, \b GPIO_PIN2,
//!             \b GPIO_PIN3, \b GPIO_PIN4, \b GPIO_PIN5, \b GPIO_PIN6,
//!             \b GPIO_PIN7,\b GPIO_PIN8,\b GPIO_PIN9,\b GPIO_PIN10,
//!             \b GPIO_PIN11,\b GPIO_PIN12,\b GPIO_PIN13,\b GPIO_PIN14,
//!             \b GPIO_PIN15
//! Modified registers are \b PxIFG.
//!
//! \return Masked state of the interupt
//
//*****************************************************************************
unsigned int GPIO_getInterruptStatus (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    )
{
    ASSERT((GPIO_PORT_P1 == selectedPort) ||
        (GPIO_PORT_P2 == selectedPort) ||
        (GPIO_PORT_PA == selectedPort)
        );

    ASSERT(0x00 != (selectedPins & (GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + 
                                     GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN5 + 
                                     GPIO_PIN6 + GPIO_PIN7 + GPIO_PIN8 + 
                                     GPIO_PIN9 + GPIO_PIN10 + GPIO_PIN11 +
                                     GPIO_PIN12 + GPIO_PIN13 + GPIO_PIN14 +  
                                     GPIO_PIN15
             )));
    
    unsigned char returnValue = 0;

    switch (selectedPort){
        case GPIO_PORT_P1:
            returnValue = (HWREGB(baseAddress + OFS_P1IFG) & selectedPins);
            break;
        case GPIO_PORT_P2:
            returnValue = (HWREGB(baseAddress + OFS_P2IFG) & selectedPins);
            break;
        case GPIO_PORT_PA:
            returnValue = (HWREG(baseAddress + OFS_PAIFG) & selectedPins);
            break;
    }

    return ( returnValue) ;
}

//*****************************************************************************
//
//! This function clears the interrupt flag on the selected pin.
//!     Note that only Port 1,2, A have this capability
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are \b GPIO_PORT_P1, \b GPIO_PORT_P2,
//!             \b GPIO_PORT_PA
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are \b GPIO_PIN0, \b GPIO_PIN1, \b GPIO_PIN2,
//!             \b GPIO_PIN3, \b GPIO_PIN4, \b GPIO_PIN5, \b GPIO_PIN6,
//!             \b GPIO_PIN7,\b GPIO_PIN8,\b GPIO_PIN9,\b GPIO_PIN10,
//!             \b GPIO_PIN11,\b GPIO_PIN12,\b GPIO_PIN13,\b GPIO_PIN14,
//!             \b GPIO_PIN15
//! Modified registers are \b PxIFG.
//!
//! \return None
//
//*****************************************************************************
void GPIO_clearInterruptFlag (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    )
{
    ASSERT((GPIO_PORT_P1 == selectedPort) ||
        (GPIO_PORT_P2 == selectedPort) ||
        (GPIO_PORT_PA == selectedPort)
        );


    ASSERT(0x00 != (selectedPins & (GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + 
                                     GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN5 + 
                                     GPIO_PIN6 + GPIO_PIN7 + GPIO_PIN8 + 
                                     GPIO_PIN9 + GPIO_PIN10 + GPIO_PIN11 +
                                     GPIO_PIN12 + GPIO_PIN13 + GPIO_PIN14 +  
                                     GPIO_PIN15
             )));
    

    switch (selectedPort){
        case GPIO_PORT_P1:
            HWREGB(baseAddress + OFS_P1IFG) &= ~selectedPins;
            break;
        case GPIO_PORT_P2:
            HWREGB(baseAddress + OFS_P2IFG) &= ~selectedPins;
            break;
        case GPIO_PORT_PA:
            HWREG(baseAddress + OFS_PAIFG) &= ~selectedPins;
            break;
    }
}

//*****************************************************************************
//
//! This function selects on what edge the port interrupt flag should be set
//! for a transition
//!     Note that only Port 1,2, A have this capability
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are
//!             \b GPIO_PORT_P1,
//!             \b GPIO_PORT_P2,
//!             \b GPIO_PORT_PA,
//!             \b GPIO_PORT_PB
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are
//!             \b GPIO_PIN0,
//!             \b GPIO_PIN1,
//!             \b GPIO_PIN2,
//!             \b GPIO_PIN3,
//!             \b GPIO_PIN4,
//!             \b GPIO_PIN5,
//!             \b GPIO_PIN6,
//!             \b GPIO_PIN7
//! \param edgeSelect specifies what tranistion sets the interrupt flag
//!             Valid values are
//!             \b GPIO_HIGH_TO_LOW_TRANSITION,
//!             \b GPIO_LOW_TO_HIGH_TRANSITION
//! Modified registers are \b PxIES.
//!
//! \return None
//
//*****************************************************************************
void GPIO_interruptEdgeSelect (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins,
    unsigned char edgeSelect
    )
{
    ASSERT((GPIO_PORT_P1 == selectedPort) ||
        (GPIO_PORT_P2 == selectedPort)
        );

    ASSERT(0x00 != (selectedPins & (GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + 
                                     GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN5 + 
                                     GPIO_PIN6 + GPIO_PIN7
             )));

    ASSERT((edgeSelect == GPIO_HIGH_TO_LOW_TRANSITION) ||
        (edgeSelect == GPIO_LOW_TO_HIGH_TRANSITION)
        );

    switch (selectedPort){
        case GPIO_PORT_P1:
            if (GPIO_LOW_TO_HIGH_TRANSITION == edgeSelect){
                HWREGB(baseAddress + OFS_P1IES) &= ~selectedPins;
            } else   {
                HWREGB(baseAddress + OFS_P1IES) |= selectedPins;
            }
            break;

        case GPIO_PORT_P2:
            if (GPIO_LOW_TO_HIGH_TRANSITION == edgeSelect){
                HWREGB(baseAddress + OFS_P2IES) &= ~selectedPins;
            } else  {
                HWREGB(baseAddress + OFS_P1IES) |= selectedPins;
            }
            break;
    }
}

//*****************************************************************************
//
//! This function sets the drive strength for the selected port pin.
//!
//! \param baseAddress is the base address of the GPIO Port Register
//! \param selectedPort is the selected port.
//!             Valid values are
//!             \b GPIO_PORT_P1,
//!             \b GPIO_PORT_P2,
//!             \b GPIO_PORT_P3,
//!             \b GPIO_PORT_P4,
//!             \b GPIO_PORT_P5,
//!             \b GPIO_PORT_P6,
//!             \b GPIO_PORT_P7,
//!             \b GPIO_PORT_P8,
//!             \b GPIO_PORT_P9,
//!             \b GPIO_PORT_P10,
//!             \b GPIO_PORT_P11,
//!             \b GPIO_PORT_PA,
//!             \b GPIO_PORT_PB,
//!             \b GPIO_PORT_PC,
//!             \b GPIO_PORT_PD,
//!             \b GPIO_PORT_PE,
//!             \b GPIO_PORT_PF,
//!             \b GPIO_PORT_PJ
//! \param selectedPins is the specified pin in the selected port.
//!             Valid values are
//!             \b GPIO_PIN0,
//!             \b GPIO_PIN1,
//!             \b GPIO_PIN2,
//!             \b GPIO_PIN3,
//!             \b GPIO_PIN4,
//!             \b GPIO_PIN5,
//!             \b GPIO_PIN6,
//!             \b GPIO_PIN7,
//!             \b GPIO_PIN8,
//!             \b GPIO_PIN9,
//!             \b GPIO_PIN10,
//!             \b GPIO_PIN11,
//!             \b GPIO_PIN12,
//!             \b GPIO_PIN13,
//!             \b GPIO_PIN14,
//!             \b GPIO_PIN15
//! \param driveStrength specifies what tranistion sets the interrupt flag
//!             Valid values are
//!             \b GPIO_REDUCED_OUTPUT_DRIVE_STRENGTH,
//!             \b GPIO_FULL_OUTPUT_DRIVE_STRENGTH
//! Modified registers are \b PxIES.
//!
//! \return None
//
//*****************************************************************************
void GPIO_setDriveStrength (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins,
    unsigned char driveStrength
    )
{
    ASSERT((GPIO_PORT_P1 == selectedPort) || (GPIO_PORT_P2 == selectedPort) ||
        (GPIO_PORT_P3 == selectedPort) || (GPIO_PORT_P4 == selectedPort) ||
        (GPIO_PORT_P5 == selectedPort) || (GPIO_PORT_P6 == selectedPort) ||
        (GPIO_PORT_P7 == selectedPort) || (GPIO_PORT_P8 == selectedPort) ||
        (GPIO_PORT_P9 == selectedPort) || (GPIO_PORT_P10 == selectedPort) ||
        (GPIO_PORT_P11 == selectedPort) || (GPIO_PORT_PA == selectedPort) ||
        (GPIO_PORT_PB == selectedPort) || (GPIO_PORT_PC == selectedPort) ||
        (GPIO_PORT_PD == selectedPort) || (GPIO_PORT_PE == selectedPort) ||
        (GPIO_PORT_PF == selectedPort) || (GPIO_PORT_PJ == selectedPort)
        );

    ASSERT(0x00 != (selectedPins & (GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + 
                                     GPIO_PIN3 + GPIO_PIN4 + GPIO_PIN5 + 
                                     GPIO_PIN6 + GPIO_PIN7 + GPIO_PIN8 + 
                                     GPIO_PIN9 + GPIO_PIN10 + GPIO_PIN11 +
                                     GPIO_PIN12 + GPIO_PIN13 + GPIO_PIN14 +  
                                     GPIO_PIN15
             )));

    ASSERT((driveStrength == GPIO_REDUCED_OUTPUT_DRIVE_STRENGTH) ||
        (driveStrength == GPIO_FULL_OUTPUT_DRIVE_STRENGTH)
        );

    switch (selectedPort){
        case GPIO_PORT_P1:
            if (GPIO_REDUCED_OUTPUT_DRIVE_STRENGTH == driveStrength){
                HWREGB(baseAddress + OFS_P1IES) &= ~selectedPins;
            } else   {
                HWREGB(baseAddress + OFS_P1IES) |= selectedPins;
            }
            break;

        case GPIO_PORT_P2:
            if (GPIO_REDUCED_OUTPUT_DRIVE_STRENGTH == driveStrength){
                HWREGB(baseAddress + OFS_P2IES) &= ~selectedPins;
            } else  {
                HWREGB(baseAddress + OFS_P2IES) |= selectedPins;
            }
            break;
    }
}

//*****************************************************************************
//
//Close the Doxygen group.
//! @}
//
//*****************************************************************************

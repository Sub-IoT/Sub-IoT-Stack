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
#ifndef __MSP430WARE_GPIO_H__
#define __MSP430WARE_GPIO_H__
//*****************************************************************************
//
//The following are the defines to include the required modules for this
//peripheral in msp430xgeneric.h file
//
//*****************************************************************************
#define __MSP430_HAS_PORT1_R__
#define __MSP430_HAS_PORT2_R__
#define __MSP430_HAS_PORTA_R__

//*****************************************************************************
//
//The following are values that can be passed to the all GPIO module API
//as the selectedPort parameter.
//
//*****************************************************************************
#define GPIO_PORT_P1     0x00
#define GPIO_PORT_P2     0x01
#define GPIO_PORT_P3     0x00
#define GPIO_PORT_P4     0x01
#define GPIO_PORT_P5     0x00
#define GPIO_PORT_P6     0x01
#define GPIO_PORT_P7     0x00
#define GPIO_PORT_P8     0x01
#define GPIO_PORT_P9     0x00
#define GPIO_PORT_P10    0x01
#define GPIO_PORT_P11    0x00
#define GPIO_PORT_PA     0x02
#define GPIO_PORT_PB     0x02
#define GPIO_PORT_PC     0x02
#define GPIO_PORT_PD     0x02
#define GPIO_PORT_PE     0x02
#define GPIO_PORT_PF     0x02
#define GPIO_PORT_PJ     0x02

//*****************************************************************************
//
//The following are values that can be passed to the all GPIO module API
//as the selectedPin parameter.
//
//*****************************************************************************
#define GPIO_PIN0   0x01
#define GPIO_PIN1   0x02
#define GPIO_PIN2   0x04
#define GPIO_PIN3   0x08
#define GPIO_PIN4   0x10
#define GPIO_PIN5   0x20
#define GPIO_PIN6   0x40
#define GPIO_PIN7   0x80
#define GPIO_PIN8   0x100
#define GPIO_PIN9   0x200
#define GPIO_PIN10   0x400
#define GPIO_PIN11   0x800
#define GPIO_PIN12   0x1000
#define GPIO_PIN13   0x2000
#define GPIO_PIN14   0x4000
#define GPIO_PIN15   0x8000

//*****************************************************************************
//
//The following are values that may be returned by the GPIO_getInputPinValue()
//API.
//
//*****************************************************************************
#define GPIO_INPUT_PIN_HIGH 0x01
#define GPIO_INPUT_PIN_LOW  0x00

//*****************************************************************************
//
//The following are values that can be passed to the GPIO_interruptEdgeSelect()
//API as the edgeSelect parameter.
//
//*****************************************************************************
#define GPIO_HIGH_TO_LOW_TRANSITION 0x01
#define GPIO_LOW_TO_HIGH_TRANSITION 0x00

//*****************************************************************************
//
//The following are values that can be passed to the GPIO_setDriveStrength()
//API as the driveStrength parameter.
//
//*****************************************************************************
#define GPIO_FULL_OUTPUT_DRIVE_STRENGTH     0x01
#define GPIO_REDUCED_OUTPUT_DRIVE_STRENGTH  0x00


//*****************************************************************************
//
//Prototypes for the APIs.
//
//*****************************************************************************
extern
void GPIO_setAsOutputPin (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPin
    );
extern
void GPIO_setAsInputPin (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPin
    );

extern
void GPIO_setOutputHighOnPin ( unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPin
    );

extern
void GPIO_setOutputLowOnPin (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPin
    );

extern
void GPIO_toggleOutputOnPin (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPin
    );
extern
void GPIO_setAsInputPinWithPullDownresistor (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPin
    );
extern
void GPIO_setAsInputPinWithPullUpresistor (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPin
    );


extern
void GPIO_enableInterrupt (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    );

extern
void GPIO_disableInterrupt (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    );
extern
void GPIO_disbleInterrupt (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    );
extern
void GPIO_clearInterruptFlag (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    );
extern
unsigned int GPIO_getInterruptStatus (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins
    );

extern
void GPIO_interruptEdgeSelect (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins,
    unsigned char edgeSelect
    );

extern
unsigned short GPIO_getInputPinValue (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPin
    );

extern
void GPIO_setDriveStrength (unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPins,
    unsigned char driveStrength
    );
extern
void GPIO_setAsPeripheralModuleFunctionOutputPin ( unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPin
    );

extern
void GPIO_setAsPeripheralModuleFunctionInputPin ( unsigned int baseAddress,
    unsigned char selectedPort,
    unsigned int selectedPin
    );

#endif

/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/HAL/DigIoL1.c $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
* $LastChangedBy: geert $
*
* \par Company:
*	E.D.&A.\n
*/

/*-- Includes --*/
#include "stm32_device.h"
#include "TimersL1.h"
#include "stdbool.h"
#include "debug.h"

#include "DigIoL1.h"

/*-- MISRA rules --*/
/*
 * Description: "Integral type 'unsigned int' should not be converted to pointer to object type"
 */
/* parasoft suppress item MISRA2012-RULE-11_4 reason "Conversion in external library (STM8 Peripheral library)"*/


/*-- Local definitions --*/
/*! \cond *//* Local definitions shouldn't be documented */

/*! \endcond *//* End of local definitions */

/*-- Local types --*/
/*! \cond *//* Local types shouldn't be documented */

typedef struct
{
    GPIO_TypeDef *port;
    uint32_t pin;
    uint32_t pull;
} DigIn_t;

typedef struct
{
    GPIO_TypeDef *port;
    uint32_t pin;
    GPIO_PinState activeValue;
} DigOut_t;

typedef struct
{
    GPIO_TypeDef *port;
    uint32_t pin;
} Unused_t;

/*! \endcond *//* End of local types */


/*-- Local data --*/

static DigIn_t const s_inputPins[E_DIGIN_COUNT] =
{
    { GPIOA, GPIO_PIN_10, GPIO_PULLUP },    /* E_DIGIN_CAN_ID_BIT0 */
    { GPIOA, GPIO_PIN_9, GPIO_PULLUP },     /* E_DIGIN_CAN_ID_BIT1 */
};

static DigOut_t const s_outputPins[E_DIGOUT_COUNT] =
{
    { GPIOA, GPIO_PIN_5, GPIO_PIN_SET },    /* E_DIGOUT_RL1 */
    { GPIOA, GPIO_PIN_6, GPIO_PIN_SET },    /* E_DIGOUT_RL2 */
    { GPIOA, GPIO_PIN_7, GPIO_PIN_SET },    /* E_DIGOUT_RL3 */
    { GPIOB, GPIO_PIN_0, GPIO_PIN_SET },    /* E_DIGOUT_RL4 */
    { GPIOB, GPIO_PIN_1, GPIO_PIN_SET },    /* E_DIGOUT_RL5 */
    { GPIOB, GPIO_PIN_2, GPIO_PIN_SET },    /* E_DIGOUT_RL6 */
    { GPIOB, GPIO_PIN_10, GPIO_PIN_SET },   /* E_DIGOUT_RL7 */
    { GPIOB, GPIO_PIN_11, GPIO_PIN_SET },   /* E_DIGOUT_RL8 */
    { GPIOA, GPIO_PIN_4, GPIO_PIN_SET },    /* E_DIGOUT_RL9 */
    { GPIOA, GPIO_PIN_1, GPIO_PIN_SET },    /* E_DIGOUT_RL10 */
};

static Unused_t const s_unusedPins[E_UNUSED_COUNT] =  /* parasoft-suppress MISRA2012-RULE-8_9 "Variable declared in th beginning for readability." */
{
    { GPIOA, GPIO_PIN_0 },      /* E_UNUSED_PA0 */
	{ GPIOA, GPIO_PIN_8 },      /* E_UNUSED_PA8 */
	{ GPIOA, GPIO_PIN_15 },     /* E_UNUSED_PA15 */
	{ GPIOB, GPIO_PIN_3 },      /* E_UNUSED_PB3 */
	{ GPIOB, GPIO_PIN_4 },      /* E_UNUSED_PB4 */
	{ GPIOB, GPIO_PIN_5 },      /* E_UNUSED_PB5 */
	{ GPIOB, GPIO_PIN_6 },      /* E_UNUSED_PB6 */
	{ GPIOB, GPIO_PIN_7 },      /* E_UNUSED_PB7 */
	{ GPIOB, GPIO_PIN_8 },      /* E_UNUSED_PB8 */
	{ GPIOB, GPIO_PIN_9 },      /* E_UNUSED_PB9 */
	{ GPIOB, GPIO_PIN_12 },     /* E_UNUSED_PB12 */
	{ GPIOB, GPIO_PIN_13 },     /* E_UNUSED_PB13 */
	{ GPIOB, GPIO_PIN_14 },     /* E_UNUSED_PB14 */
	{ GPIOB, GPIO_PIN_15 },     /* E_UNUSED_PB15 */
	{ GPIOC, GPIO_PIN_13 },     /* E_UNUSED_PC13 */
	{ GPIOC, GPIO_PIN_14 },     /* E_UNUSED_PC14 */
	{ GPIOC, GPIO_PIN_15 },     /* E_UNUSED_PC15 */
};

/*-- Private prototypes --*/

/*-- Public functions --*/

/*!
*    Init DigIoL1 module
*/
void DigIoL1_Init(void)
{
    GPIO_InitTypeDef initStruct;
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Configure input pins */
    initStruct.Speed = GPIO_SPEED_FREQ_LOW;
    for (uint8_t inputId = 0u; inputId < E_DIGIN_COUNT; ++inputId)
    {
        initStruct.Pin = s_inputPins[inputId].pin;
        initStruct.Pull = s_inputPins[inputId].pull;
        HAL_GPIO_Init( s_inputPins[inputId].port, &initStruct );
    }
    
    /* Configure output pins */
    initStruct.Mode = GPIO_MODE_OUTPUT_PP;
    initStruct.Pull = GPIO_NOPULL;
    initStruct.Speed = GPIO_SPEED_FREQ_LOW;
    for (uint8_t outputId = 0u; outputId < E_DIGOUT_COUNT; ++outputId)
    {
        initStruct.Pin = s_outputPins[outputId].pin;
        HAL_GPIO_Init( s_outputPins[outputId].port, &initStruct );
        DigIoL1_SetOutput(outputId, false);     /* Set output low */
    }
    
    /* Configure unused pins */
	initStruct.Mode = GPIO_MODE_ANALOG;
    initStruct.Pull = GPIO_NOPULL;
    for (uint8_t unusedId = 0u; unusedId < E_UNUSED_COUNT; ++unusedId)
    {
        initStruct.Pin = s_unusedPins[unusedId].pin;
        HAL_GPIO_Init( s_unusedPins[unusedId].port, &initStruct );
    }
}


/*!
*    Make DigIoL1 module work
*/
void DigIoL1_DoWork(void)
{
    static bool s_initialised = false;
    
    /* Disable SWD pins after 10 seconds. */
    if ( (TimersL1_GetSystemTime() > 10000u) && (s_initialised == false) )
    {
        GPIO_InitTypeDef initStruct;
        
        /*##-2- Configure peripheral GPIO ##########################################*/
        initStruct.Pin = GPIO_PIN_13;
        initStruct.Mode = GPIO_MODE_ANALOG;
        initStruct.Speed = GPIO_SPEED_FREQ_LOW;
        initStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOA, &initStruct); /* parasoft-suppress MISRA2012-RULE-11_4 "Use of library specific macro." */
        
        initStruct.Pin = GPIO_PIN_14;
        HAL_GPIO_Init(GPIOA, &initStruct); /* parasoft-suppress MISRA2012-RULE-11_4 "Use of library specific macro." */
        
        s_initialised = true;
    }
}


/*!
*    Read a digital input
*    
*    \param[in]     index       index of digital input
*
*    \retval        true        input is active
*    \retval        false       input is inactive
*/
bool DigIoL1_GetInput(uint8_t inputId)
{
    bool retVal = false;
    assert(inputId < E_DIGIN_COUNT);
    
    /* Read input pin */
    if ( HAL_GPIO_ReadPin(s_inputPins[inputId].port, s_inputPins[inputId].pin) == GPIO_PIN_SET )
    {
        retVal = true;
    }
    
    return retVal;
}


/*!
*	Set a digital output
*	
*	\param[in]	index       index of digital input
*	\param[in]	state       the value to write to the output
*/
void DigIoL1_SetOutput(uint8_t outputId, bool state)
{
    bool outputVal = state;
	assert(outputId < E_DIGOUT_COUNT);
    
    /* Check whether this is an inverting output */
    if (s_outputPins[outputId].activeValue == GPIO_PIN_RESET)
    {
        outputVal = !outputVal;
    }

    /* Set the output */
    HAL_GPIO_WritePin( s_outputPins[outputId].port, s_outputPins[outputId].pin, (GPIO_PinState)outputVal );
}


/*!
*	Set all digital outputs
*	
*	\param[in]	outputs     the value to write to the outputs
*/
void DigIoL1_SetOutputs(uint16_t outputs)
{    
    for(uint_fast8_t i = E_DIGOUT_RL1; i <= E_DIGOUT_RL10; ++i)
    {
        GPIO_PinState pinState;
        
        /* Check whether this is an inverting output */
        if (( outputs & ((uint16_t)1u<<i) ) != 0u) /* parasoft-suppress MISRA2012-RULE-12_2 "Shifting with a enum value doesn't need checking." */
        {
            pinState = s_outputPins[i].activeValue;
        }
        else
        {
            pinState = (GPIO_PinState)!s_outputPins[i].activeValue;
        }
        
        /* Set the output */
        HAL_GPIO_WritePin( s_outputPins[i + E_DIGOUT_RL1].port, s_outputPins[i + E_DIGOUT_RL1].pin, pinState );
    }
}


/*!
*	Set all digital outputs
*	
*	\retval	    the values of the outputs
*/
uint16_t DigIoL1_GetOutputs(void)
{    
    uint16_t retVal = 0u;
    
    for(uint_fast8_t i = E_DIGOUT_RL1; i <= E_DIGOUT_RL10; ++i)
    {
        GPIO_PinState pinState;
        
        /* Get the output */
        pinState = HAL_GPIO_ReadPin( s_outputPins[i + E_DIGOUT_RL1].port, s_outputPins[i + E_DIGOUT_RL1].pin );
        
        /* Check whether this is an inverting output */
        if (s_outputPins[i].activeValue == 0u)
        {
            pinState = (GPIO_PinState)!pinState;
        }
        
        retVal |= pinState << i;
    }
    
    return retVal;
}

/*-- Private functions --*/






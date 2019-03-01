/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/HAL/DigIoL1.h $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
* $LastChangedBy: geert $
*
* \par Company:
*	E.D.&A.\n
*
* \par Description:
*   HAL for configuring, driving & reading IO.\n
*/

#ifndef DIGIOL1_H__
#define DIGIOL1_H__

#ifdef __cplusplus
extern "C" {
#endif

/*-- Includes --*/

/*-- Public definitions --*/

/* Don't change order: used in pin definition array */
    
enum DigIn
{
    E_DIGIN_CAN_ID_BIT0 = 0u,
    E_DIGIN_CAN_ID_BIT1,

    E_DIGIN_COUNT           /*!< Count of inputs */
};

enum DigOut
{
    E_DIGOUT_RL1 = 0u,
    E_DIGOUT_RL2,
    E_DIGOUT_RL3,
    E_DIGOUT_RL4,
    E_DIGOUT_RL5,
    E_DIGOUT_RL6,
    E_DIGOUT_RL7,
    E_DIGOUT_RL8,
    E_DIGOUT_RL9,
    E_DIGOUT_RL10,
    
    E_DIGOUT_COUNT           /*!< Count of outputs */
};

enum Unused
{
    E_UNUSED_PA0 = 0u,
	E_UNUSED_PA8,
	E_UNUSED_PA15,
	E_UNUSED_PB3,
	E_UNUSED_PB4,
	E_UNUSED_PB5,
	E_UNUSED_PB6,
	E_UNUSED_PB7,
	E_UNUSED_PB8,
	E_UNUSED_PB9,
	E_UNUSED_PB12,
	E_UNUSED_PB13,
	E_UNUSED_PB14,
	E_UNUSED_PB15,
	E_UNUSED_PC13,
	E_UNUSED_PC14,
	E_UNUSED_PC15,
	E_UNUSED_PF11_BOOT0,
        
    E_UNUSED_COUNT           /*!< Count of unused pins */
};

/*-- Public types --*/

/*-- Public functions --*/

void DigIoL1_Init(void);
void DigIoL1_DoWork(void);

bool DigIoL1_GetInput(uint8_t inputId);
void DigIoL1_SetOutput(uint8_t outputId, bool state);
void DigIoL1_SetOutputs(uint16_t outputs);
uint16_t DigIoL1_GetOutputs(void);

#ifdef __cplusplus
}
#endif

#endif /* !DIGIOL1_H__ */






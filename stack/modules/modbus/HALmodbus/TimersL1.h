/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/HAL/TimersL1.h $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
* $LastChangedBy: geert $
*
* \par Company:
*	E.D.&A.\n
*
* \par Description:
*	Headers for lower level function for timer \n
*/

#ifndef TIMERSL1_H__
#define TIMERSL1_H__


/*-- Includes --*/
#include <stdint.h>
/*-- Public definitions --*/

/*-- Public types --*/

/*-- Public functions --*/
void TimersL1_Init(void);

uint32_t TimersL1_GetSystemTime(void);

void TimersL1_Enable(void);
void TimersL1_Disable(void);

void TimersL1_AddCallBack(void (*pCallBackFn)(void), uint32_t callPeriod);

#endif /* !TIMERSL1_H__ */



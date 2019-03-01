/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/HAL/OsIntfl1.h $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
* $LastChangedBy: geert $
*
* \par Company:
*	E.D.&A.\n
*
* \par Description:
*	Low level function for STM32L\n
*/

#ifndef OSINTFL1_H__
#define OSINTFL1_H__

/*-- Includes --*/
//#include "intrinsics.h"
#include <stdint.h>
#include <stdbool.h>
#include "stm32_device.h"
#include "stm32_common_mcu.h"

/*-- Disable MISRA rules --*/

/*
 * Description: "Do not define function-like macro: *"
 */
/* parasoft suppress item MISRA2004-19_7 reason "Used for speed optimizations (inline functions defined in header are also not allowed)" */

/*
 * Description: "Macro '*' was defined improperly"
 */
/* parasoft suppress item MISRA2004-19_4 reason "Used for speed optimizations (inline functions defined in header are also not allowed)" */

/*-- Public definitions --*/
/* Enable/Disable interrupts */
#define OSDisableInterrupts()       __disable_irq() /* parasoft-suppress EDnA-218 "Legacy code" */
#define OSEnableInterrupts()        __enable_irq() /* parasoft-suppress EDnA-218 "Legacy code" */

#define	OSWaitForInterrupt()		__WFI() /* parasoft-suppress EDnA-218 "Legacy code" */
#define	OSWaitForEvent()			__WFE() /* parasoft-suppress EDnA-218 "Legacy code" */

#define OSSaveInterruptState()     __get_PRIMASK() /* parasoft-suppress EDnA-218 "Legacy code" */
#define OSRestoreInterruptState(x)  __set_PRIMASK(x) /* parasoft-suppress EDnA-218 "Legacy code" */


/*-- Public types --*/
typedef uint32_t saveInterruptState_t;

/*-- Public functions --*/
void OSIntfL1_Init(void);
void OSIntfL1_KickWatchDog(void);

uint32_t OSIntfL1_GetResetCause(void);
void OSIntfL1_ClearResetCause(void);

void OSIntfL1_ForceReset(void);

uint32_t OSIntfL1_GetMyHashedAddress(void);


/*-- Enable MISRA rules --*/

/* parasoft unsuppress item MISRA2004-19_7 */

/* parasoft unsuppress item MISRA2004-19_4 */


#endif /* !OSINTFL1_H__ */


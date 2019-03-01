/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/HAL/CpuL1.c $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
*
* \par Company:
*	E.D.&A.\n
*
* \par Description:
*	Handler for fatal CPU Exceptions\n
*
*/

/*-- Includes --*/
#include "stm32_device.h"
#include "OsIntfL1.h"

#include "CpuL1.h"
#include "debug.h"

/*-- Local types --*/
typedef struct
{
	uint32_t m_IRQ_LR;			/* Link register in exception handler (EXC_RETURN) */
	uint32_t m_LR;				/* Link register value when exception occurred */
	uint32_t m_PC;				/* Program Counter */
	uint32_t m_SP;				/* Current stack pointer (MSP or PSP) at the time of exception */
} savedRegisters_t;

/*-- Local data --*/
static savedRegisters_t s_savedRegisters; /* parasoft-suppress MISRA2004-8_10 "Needed to prevent compiler from optimizing variable" */ /* parasoft-suppress MISRA2012-RULE-8_9 "Needed to prevent compiler from optimizing variable" */

/*-- Private prototype --*/
//void HardFault_Handler(void); /* Interrupt handler */

/*-- Public functions --*/

/*!
*	Initialize cpu exceptions
*/
void CpuL1_Init(void)
{ 
	/*
	Cortex M0 core doesn't support unaligned access, Hard fault is always generated
	Cortex M0+ doesn't generate HardFault on div by zero (no hardware div, software routine)
	*/
	
#if 0
	/* Unaligned access */
	uint32_t  * address;
	address = (uint32_t *)1;
	*address = (uint32_t)address;
#endif
}

// /*!
// *	Hard Fault handler
// */
// void HardFault_Handler(void) /* parasoft-suppress MISRA2004-8_10 "Interrupt handler is declared in assembly file" */
// {
// 	OSDisableInterrupts();
	
// 	/* Get correct stack pointer */
// 	//s_savedRegisters.m_IRQ_LR = (uint32_t)__get_LR();

// 	if ((s_savedRegisters.m_IRQ_LR & 0x04U) == 0x04U)
// 	{
// 		/* Process stack (application) */
// 		s_savedRegisters.m_SP = (uint32_t)__get_PSP();
// 	}
// 	else
// 	{
// 		/* Main stack (OS or handler) */
// 		s_savedRegisters.m_SP = (uint32_t)__get_MSP();
// 	}
		
// 	/* Save useful register from SP */
// 	s_savedRegisters.m_LR = *((uint32_t *)(s_savedRegisters.m_SP) + 7U); /* parasoft-suppress MISRA2004-11_3_b "Cast from integer to pointer is needed to access stack" */ /* parasoft-suppress MISRA2012-RULE-11_4 "Cast from integer to pointer is needed to access stack" */
// 	s_savedRegisters.m_PC = *((uint32_t *)(s_savedRegisters.m_SP) + 8U); /* parasoft-suppress MISRA2004-11_3_b "Cast from integer to pointer is needed to access stack" */ /* parasoft-suppress MISRA2012-RULE-11_4 "Cast from integer to pointer is needed to access stack" */

// 	assert(false);
// }


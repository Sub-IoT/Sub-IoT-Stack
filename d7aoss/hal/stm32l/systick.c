/*
 * systick.c
 *
 *  Created on: Mar 4, 2010
 *      Author: armin
 */
#include <stdint.h>
#include <stm32l1xx.h>

#include "systick.h"

volatile uint32_t systickValue;

//volatile uint32_t *DWT_CYCCNT = (volatile uint32_t *) 0xE0001004; //address of the register
//volatile uint32_t *DWT_CTRL = (volatile uint32_t *) 0xE0001000; //address of the register

void systick_init() {
	SystemCoreClockUpdate();
	/* Setup SysTick Timer for 10 msec interrupts  */
	systickValue = 0;
	if (SysTick_Config(SystemCoreClock / 100)) {
		/* Capture error */
		while (1)
			;
	}
//	CoreDebug ->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // enable DWT
//	*DWT_CTRL |= 1; // enable cycle counter
//	*DWT_CYCCNT = 0; // reset cycle counter
}

void delaymS(uint32_t msec) {
	if (msec < 0x800) {
		delayuS(msec * 1000);
	}
	else {
		uint32_t exitValue = systickValue + msec/10;
		// wait until we're there
		while (systickValue < exitValue)
			;
	}
}
/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void) {
	systickValue += 1;
}

void delayuS(uint32_t uS) {
	uint32_t cyclestoLoops;

	cyclestoLoops = SystemCoreClock;
	if (cyclestoLoops >= 2000000) {
		cyclestoLoops /= 1000000;
		cyclestoLoops *= uS;
	} else {
		cyclestoLoops *= uS;
		cyclestoLoops /= 1000000;
	}

	if (cyclestoLoops <= 100)
		return;
	cyclestoLoops -= 50; // cycle count for entry/exit
	cyclestoLoops /= 4; // cycle count per iteration - should be 4 on Cortex M0/M3

	if (!cyclestoLoops)
		return;

	// Delay loop for Cortex M3 thumb2
    __asm volatile (
			// Load loop count to register
			" mov r3, %[loops]\n"

			// loop start- subtract 1 from r3
			"loop: subs r3, #1\n"
			// test for zero, loop if not
			" bne loop\n\n"

			:// No output registers
			: [loops] "r" (cyclestoLoops)// Input registers
			: "r3"// clobbered registers
	);
}

//void delayuS(uint32_t uS) {
//	uint32_t timeEntry, timeExit;
//	timeEntry = *DWT_CYCCNT;
//	_delayuS(uS);
//	timeExit = *DWT_CYCCNT;
//	printf("Delay of %d uS took %d cycles.\n", uS, (timeExit - timeEntry));
//}

#ifndef __EFM32GG_MCU_H_
#define __EFM32GG_MCU_H_

#include "em_int.h"

//General definition for EFM32 interrupt handlers
#define INT_HANDLER(handler)	static inline void __ ## handler();\
    void handler(){INT_Disable();__ ## handler();INT_Enable();}\
    static inline void __ ## handler()

void __efm32gg_mcu_init();

#endif

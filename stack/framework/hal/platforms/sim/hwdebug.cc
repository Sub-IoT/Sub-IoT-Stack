#include "hwdebug.h"

extern "C" void __hw_debug_init(){}
extern "C" void hw_debug_set(uint8_t){}
extern "C" void hw_debug_clr(uint8_t){}
extern "C" void hw_debug_toggle(uint8_t){}
extern "C" void hw_debug_mask(uint32_t){}

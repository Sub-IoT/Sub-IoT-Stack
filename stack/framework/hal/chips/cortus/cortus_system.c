
#include "hwsystem.h"


void hw_enter_lowpower_mode(uint8_t mode)
{
   // TODO
}

uint64_t hw_get_unique_id()
{
   // TODO
   return 0;
}

void hw_busy_wait(int16_t microseconds)
{
   volatile int i = 0;
#if 0
   uint32_t comp = (uint32_t) microseconds * 12500000 / 1000000; // core clock = 12.5MHz
#else
   uint32_t comp = (uint32_t) microseconds * 125 / 10;
#endif

   while(i!=comp)
      i++;
}

void hw_reset()
{
   // TODO
   start();
}

float hw_get_internal_temperature()
{
   // TODO
   return 0;
}

uint32_t hw_get_battery(void)
{
   // TODO
   return 0;
}

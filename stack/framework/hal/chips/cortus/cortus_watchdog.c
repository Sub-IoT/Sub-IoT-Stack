#include "hwwatchdog.h"
#include "machine/wdt.h"



void __watchdog_init()
{
   wdt->key = 0x700edc33;
   //wdt->value = 1000;
   //wdt->value = 0x64000000; // 1600M cycles @ 6.25MHz (efm32gg: 256K cycles @ 1kHz)
   wdt->value = 0xc8000000; // 3200M cycles @ 12.5MHz (efm32gg: 256K cycles @ 1kHz)

   wdt->key = 0x700edc33;
   wdt->status = 1;

   wdt->key = 0x700edc33;
   wdt->restart = 1;

   wdt->key = 0x700edc33;
   wdt->sel_clk = 2; // 12.5MHz

   wdt->key = 0x700edc33;
   wdt->enable = 1;
}



void hw_watchdog_feed()
{
   wdt->key = 0x700edc33;
   wdt->restart = 1;
}

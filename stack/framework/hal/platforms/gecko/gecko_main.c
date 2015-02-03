#include "scheduler.h"
#include "hwsystem.h"

int main()
{
    __system_init();
    scheduler_init();
    __framework_bootstrap();
    scheduler_run();
    return 0;
}
/*

int main(void)
{
  CHIP_Init();
  initPrintf();

  BSP_TraceProfilerSetup();

  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1) ;

    CMU_ClockEnable(cmuClock_GPIO, true);

    GPIOINT_Init();

    GPIO_PinModeSet(gpioPortB, 9, gpioModeInput, 0);
    GPIO_PinModeSet(gpioPortB, 10, gpioModeInput, 0);

    GPIOINT_CallbackRegister(9, gpioCallback);
    GPIOINT_CallbackRegister(10, gpioCallback);

    GPIO_IntConfig(gpioPortB, 9, false, true, true);
    GPIO_IntConfig(gpioPortB, 10, false, true, true);


  scheduler_init();
  sched_register_task(&bootstrap);
  sched_post_task(&bootstrap);
  scheduler_run();
//
//
//  while (1)
//  {
//    BSP_LedToggle(0);
//    if(BSP_LedGet(0))
//    	BSP_LedClear(1);
//    else
//    	BSP_LedSet(1);
//    printf("Toggled...%d\n", BSP_LedGet(0));
//    Delay(1000);
//  }
}

void _exit()
{
	while(1){}
}

*/
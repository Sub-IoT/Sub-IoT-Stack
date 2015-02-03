#include <stdio.h>

#define AS_STR(V)	#V
#define APP_CONFIG_STR AS_STR(APP_CONFIG)

extern char const * atomic_str;
extern char const * log_str;
extern char const * random_str;
extern char const * scheduler_str;
extern char const * timer_str;


void bootstrap()
{
    printf("Bootstrap called. Provided APP Config: %s\n", APP_CONFIG_STR);
    printf("\tAtomic: %s\n", atomic_str);
    printf("\tLog: %s\n", log_str);
    printf("\tRandom: %s\n", random_str);
    printf("\tScheduler: %s\n", scheduler_str);
    printf("\tTimer: %s\n", timer_str);
}
#include "scheduler.h"

void bootstrap();
void __framework_bootstrap()
{
    //do framework initialisation here
    //register the user bootstrap function();
    sched_register_task(&bootstrap);
    sched_post_task(&bootstrap);
}
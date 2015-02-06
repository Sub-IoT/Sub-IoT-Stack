#include "scheduler.h"
#include "hwsystem.h"
#include "random.h"
#include "log.h"
void bootstrap();
void __framework_bootstrap()
{
    //initialise the scheduler
    scheduler_init();
    //initialise libc RNG with the unique device id
    set_rng_seed(hw_get_unique_id());
    //reset the log counter
    log_counter_reset();

    //register the user bootstrap function();
    sched_register_task(&bootstrap);
    sched_post_task(&bootstrap);
}

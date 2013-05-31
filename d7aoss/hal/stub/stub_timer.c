/*
 *  Created on: May 9, 2013
 *  Authors:
 *  	glenn.ergeerts@artesis.be
 */

#include "../timer.h"

#include <signal.h>
#include <time.h>

timer_t timerid;

static void
sig_handler(int sig, siginfo_t *si, void *uc)
{
    signal(sig, SIG_IGN);
    timer_completed();
}

void hal_timer_init()
{
    hal_timer_enable_interrupt();
}

void hal_timer_enable_interrupt()
{

}

void hal_timer_disable_interrupt()
{

}

int16_t hal_timer_getvalue()
{
    return 0;
}

void hal_timer_setvalue(uint16_t next_event)
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sig_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGRTMIN, &sa, NULL);

    struct sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = &timerid;
    timer_create(CLOCK_REALTIME, &sev, &timerid);

    struct itimerspec its;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = next_event * 1000000;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    timer_settime(timerid, 0, &its, NULL);

}


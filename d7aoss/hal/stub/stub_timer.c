/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 *     	glenn.ergeerts@uantwerpen.be
 *
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

uint16_t hal_timer_getvalue()
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

void hal_benchmarking_timer_init()
{

}

void hal_benchmarking_timer_start()
{

}

void hal_benchmarking_timer_stop()
{

}

uint32_t hal_benchmarking_timer_getvalue()
{

}

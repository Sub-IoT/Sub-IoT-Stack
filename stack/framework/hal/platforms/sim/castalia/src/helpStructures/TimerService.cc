/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2010                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Yuriy Tselishchev                                            *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *  
 ****************************************************************************/

#include "TimerService.h"

void TimerService::setTimerDrift(double new_drift)
{
	timerDrift = new_drift;
}

void TimerService::timerFiredCallback(int timerIndex)
{
}

void TimerService::cancelTimer(int timerIndex)
{
	if (timerIndex < 0)
		opp_error("cancelTimer(): timerIndex=%i negative index is not allowed",timerIndex);
	if (timerIndex >= TIMER_MAX_SIZE)
		opp_error("cancelTimer(): timerIndex=%i is too large",timerIndex);
	if (timerIndex >= timerMessages.size())
		return;
	TimerServiceMessage* tmp = timerMessages[timerIndex];
	if (tmp != NULL && tmp->isScheduled())
		cancelAndDelete(tmp);
	timerMessages[timerIndex] = NULL;
}

void TimerService::setTimer(int timerIndex, simtime_t time)
{
	if (timerIndex < 0)
		opp_error("setTimer(): timerIndex=%i negative index is not allowed",timerIndex);
	if (timerIndex >= TIMER_MAX_SIZE)
		opp_error("setTimer(): timerIndex=%i is too large",timerIndex);
	cancelTimer(timerIndex);
	if (timerIndex >= timerMessages.size()) {
		int newSize = timerMessages.size() + TIMER_MIN_SIZE;
		if (newSize > TIMER_MAX_SIZE)
			newSize = TIMER_MAX_SIZE;
		else if (timerIndex >= newSize)
			newSize = timerIndex + 1;
		timerMessages.resize(newSize,NULL);
	}
	timerMessages[timerIndex] = new TimerServiceMessage("Timer message", TIMER_SERVICE);
	timerMessages[timerIndex]->setTimerIndex(timerIndex);

	// Edit by Daniel van den Akker @ 12/2/2015
	// Critical bugfix in the implementation of Timer Drift:
	//
	//  timerDrift tells us how much faster/slower we are running than the 'real' time
	//  (with timerDrift a little bit more or less than 1)
	//
	//  When scheduling a timer with a delay of 'time' (according to the node clock)
	//	The following equation however MUST hold regardless of timer drift:
	//		clock_new = clock_old + time
	//  with clock_old being the value of getClock() when the timer is scheduled and
	//	clock_new being the value of getClock() when the timer fires.
	//
	//	Given that:
	//		clock_old = actualTime_old*clockDrift
	//		clock_new = actualTime_new*clockDrift
	//  It follows that:
	//
	//	actualTime_new*clockDrift = actualTime_old*clockDrift + time
	//  or
	//  actualTime_new = actualTime_old + time/clockDrift
	//
	//  This means that, unlike the default implementation we need to DIVIDE instead
	//  of multiplying 'time' by 'timerDrift' in order to get the timer to fire at the
	//	right time
	//
	//  Meaning that this is false:
	//scheduleAt(simTime() + timerDrift * time, timerMessages[timerIndex]);
	//  And this is correct:
	scheduleAt(simTime() + time/timerDrift, timerMessages[timerIndex]);
}

void TimerService::handleTimerMessage(cMessage * msg)
{
	int msgKind = (int)msg->getKind();
	if (msgKind == TIMER_SERVICE) {
		TimerServiceMessage *timerMsg = check_and_cast<TimerServiceMessage*>(msg);
		int timerIndex = timerMsg->getTimerIndex();
		if (timerIndex < 0 || timerIndex >= timerMessages.size()) 
			return;
		if (timerMessages[timerIndex] != NULL) {
			timerMessages[timerIndex] = NULL;
			timerFiredCallback(timerIndex);
		} 
	}
}

simtime_t TimerService::getTimer(int timerIndex) 
{
	if (timerIndex < 0)
		opp_error("getTimer(): timerIndex=%i negative index is not allowed",timerIndex);
	if (timerIndex >= TIMER_MAX_SIZE)
		opp_error("getTimer(): timerIndex=%i is too large",timerIndex);
	if (timerIndex >= timerMessages.size())
		return -1;
	if (timerMessages[timerIndex] == NULL)
		return -1;
	else
		return timerMessages[timerIndex]->getArrivalTime() * timerDrift;
}

simtime_t TimerService::getClock()
{
	return simTime() * timerDrift;
}


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
	scheduleAt(simTime() + timerDrift * time, timerMessages[timerIndex]);
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


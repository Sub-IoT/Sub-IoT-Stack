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

#include "NoMobilityManager.h"

Define_Module(NoMobilityManager);

void NoMobilityManager::handleMessage(cMessage * msg)
{
	int msgKind = msg->getKind();
	trace() << "Unexpected message: " << msgKind;
	delete msg;
	msg = NULL;
}

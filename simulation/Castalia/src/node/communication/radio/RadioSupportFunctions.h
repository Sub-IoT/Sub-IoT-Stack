/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2010                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Athanassios Boulis, Dimos Pediaditakis, Yuriy Tselishchev	*
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *  
 ****************************************************************************/

#ifndef SUPPORTFUNCTIONS_H_
#define SUPPORTFUNCTIONS_H_

#include <math.h>
#include "RadioControlMessage_m.h"
#include "CastaliaMessages.h"

float addPower_dBm(float a, float b);

float subtractPower_dBm(float a, float b);

float ratioTodB(float a);

float dBToRatio(float a);

float erfInv(float y);

float erfcInv(float y);

float diffQPSK_SNR2BER(float SNR);

double probabilityOfExactly_N_Errors(double BER, int errors, int numOfBits);

int bitErrors(double BER, int numOfBits, int maxBitErrorsAllowed);

RadioControlCommand *createRadioCommand(RadioControlCommand_type, double);
RadioControlCommand *createRadioCommand(RadioControlCommand_type, const char *);
RadioControlCommand *createRadioCommand(RadioControlCommand_type, BasicState_type);
RadioControlCommand *createRadioCommand(RadioControlCommand_type);

#endif				/*SUPPORTFUNCTIONS_H_ */

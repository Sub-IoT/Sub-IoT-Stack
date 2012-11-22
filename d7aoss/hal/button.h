/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef __BUTTON_H__
#define __BUTTON_H__



void Buttons_Init();

void Buttons_EnableInterrupts();

void Buttons_DisableInterrupts();

void Buttons_ClearInterruptFlag();

unsigned char Button_IsActive(unsigned char);

#endif //__BUTTON_H__

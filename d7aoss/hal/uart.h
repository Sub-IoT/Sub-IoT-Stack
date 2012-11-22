/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef __UART_H__
#define _UART_H__

void Uart_Init();

void Uart_EnableInterrupt();

void Uart_TransmitData(unsigned char data);
void Uart_TransmitMessage(unsigned char *data, unsigned char length);

unsigned char Uart_TxReady();

unsigned char Uart_ReceiveData();

#endif // __UART_H__

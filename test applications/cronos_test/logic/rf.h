/*
 * rf.h
 *
 *  Created on: 7-dec.-2012
 *      Author: Maarten Weyn
 */

#ifndef RF_H_
#define RF_H_

#include "../project.h"

#include <trans/trans.h>

extern int8_t txPower;
extern uint8_t beacon_channel;
extern uint16_t beacon_interval;

void tx_callback(Trans_Tx_Result result);

void blink_init();
void blink_start();
void blink_stop();

void display_sync(uint8_t line, uint8_t update);
void display_beacon(uint8_t line, uint8_t update);

void mx_beacon(uint8_t line);

void sx_beacon(uint8_t line);
void sx_sync(uint8_t line);

void start_d7_sync();

#endif /* RF_H_ */

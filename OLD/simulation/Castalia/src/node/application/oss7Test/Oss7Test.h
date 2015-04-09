/*******************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2011                            *
 *  Developed at the ATP lab, Networked Systems research theme                 *
 *  Author(s): Athanassios Boulis, Dimosthenis Pediaditakis, Yuriy Tselishchev *
 *  This file is distributed under the terms in the attached LICENSE file.     *
 *  If you do not find this file, copies can be found by writing to:           *
 *                                                                             *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia                *
 *      Attention:  License Inquiry.                                           *
 *                                                                             *
 *******************************************************************************/

#ifndef _OSS7TEST_H_
#define _OSS7TEST_H_

#include "VirtualApplication.h"

extern "C" {
    #include "trans.h"
    #include "phy.h"
}

#include <stdint.h>
#include "SimPhyInterface.h"

using namespace std;

class Oss7Test : public VirtualApplication {
 private:
	int packetHeaderOverhead;
	int constantDataPayload;
    bool isBlinker;
    // time in ms needed before valid RSSI is reported after switching to RX
    // TODO measure this, this should be: symbolsForRSSI x bitsPerSymbol / dataRate
    simtime_t phyDelayForValidRssi;
    // time in ms needed for changing the radio state from sleep to RX
    // TODO measure this
    simtime_t phyDelayForSleep2Rx;

 protected:
	void startup();
	void fromNetworkLayer(ApplicationPacket *, const char *, double, double);
    void timerFiredCallback(int);

public:
    void oss7_api_setTimer(int ticks);
    void oss7_api_timerEventCallback();
    void oss7_api_datastreamRxCallback();
    void oss7_api_trace(char* msg, uint8_t len);
    bool oss7_api_phy_rx(phy_rx_cfg_t*);
    bool oss7_api_phy_tx(phy_tx_cfg_t*);
//    void oss7_api_phy_get_rssi();
//    void oss7_api_phy_set_rssi_measurement_completed_callback(phy_rssi_measurement_completed_callback_t);
};

enum Oss7TestTimers {
    TIMER_RX,
    TIMER_RX_TIMEOUT,
    TIMER_TX,
    TIMER_TX_COMPLETED,
    TIMER_OSS7
    //TIMER_RSSI_MEASUREMENT_COMPLETED
};

Oss7Test* oss7_node();

extern "C" void timer_completed(); // prototype for callback implemented in oss7 sim_timer.c


#endif				// _OSS7TEST_H_

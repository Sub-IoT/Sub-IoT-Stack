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

#include "Oss7Test.h"

extern "C" {
    #include "SimHalInterface.h"
    #include "SimPhyInterface.h"

	#include "system.h"
    #include "nwl.h"
    #include "log.h"
    #include "timer.h"
}

Define_Module(Oss7Test);

Oss7Test* oss7_node()
{
    cSimulation* sim = cSimulation::getActiveSimulation();
    cModule* module = sim->getModuleByPath("SN.node[0].Application"); // TODO only 1 node supported for now
    if(module != NULL)
    {
        Oss7Test* oss7Node = dynamic_cast<Oss7Test*>(module);
        return oss7Node;
    }

    return NULL;
}

void rx_datastream_callback(Trans_Rx_Datastream_Result res)
{
    oss7_node()->oss7_api_datastreamRxCallback();
}

void sim_tx_callback(Dll_Tx_Result res)
{
    log_print_trace("DLL TX callback result: %i", res);
}

dll_channel_scan_t scan_cfg1 = {
        0x1C,
        FrameTypeForegroundFrame,
        1000,
        500
};

dll_channel_scan_series_t scan_series;

void nwl_tx_callback(Dll_Tx_Result res)
{
}

void nwl_rx_callback(nwl_rx_res_t* rx_res)
{
    //log_print_string("RX CB");
}

void tx()
{
    uint8_t data[32] = { 0xEF };
    log_print_string("TX...");
    trans_tx_foreground_frame(data, 32, 0xFF, 0x01, 10);
}


void Oss7Test::startup()
{
    this->isBlinker = (bool)par("isBlinker");
    this->phyDelayForValidRssi = (double)par("phyDelayForValidRssi") / 1000.0;
    this->phyDelayForSleep2Rx = (double)par("phyDelayForSleep2Rx") / 1000.0;

    dll_init();
    dll_set_tx_callback(&sim_tx_callback);

    if(this->isBlinker)
    {
       setTimer(TIMER_TX, 0);
    }

//    nwl_init();
//    nwl_set_tx_callback(&nwl_tx_callback);
//    nwl_set_rx_callback(&nwl_rx_callback);
//    nwl_build_advertising_protocol_data(0x00, 50, 10, 0);
//    dll_tx_frame();
//
//    dll_init();
//    dll_set_tx_callback(&sim_tx_callback);
//    uint8_t data[10] = { 0x00 };
//    dll_ff_tx_cfg_t cfg;
//    dll_create_foreground_frame(data, 10, &cfg);
//    dll_tx_frame();
//
//    scan_series.length = 1;
//    scan_series.values = &scan_cfg1;
//    dll_channel_scan_series(&scan_series);
//    setTimer(TIMER_RX, 0);

//    timer_init();
//    timer_event te;
//    te.f = timer_elapsed_callback;
//    te.next_event = 1;
//    timer_add_event(&te);
}

void Oss7Test::fromNetworkLayer(ApplicationPacket * rcvPacket, const char *source, double rssi, double lqi)
{
}

void Oss7Test::timerFiredCallback(int timerIndex)
{
    switch(timerIndex)
    {
        case TIMER_RX:
            //trans_rx_datastream_start(0xFF, 0x1E);
            break;
        case TIMER_TX:
            tx();
            break;
        case TIMER_RX_TIMEOUT:
            toNetworkLayer(createRadioCommand(SET_STATE, SLEEP));
            trace() << "< RX";
            phy_rx_callback(NULL);
            break;
        case TIMER_TX_COMPLETED:
            toNetworkLayer(createRadioCommand(SET_STATE, SLEEP));
            trace() << "< TX";
            phy_tx_callback();
            break;
        case TIMER_OSS7:
            timer_completed();
            break;
//        case TIMER_RSSI_MEASUREMENT_COMPLETED:
//            trace() << "RSSI measurement completed";
//            break;
    }
}

void Oss7Test::oss7_api_setTimer(int ticks)
{
    setTimer(TIMER_OSS7, ticks/1024.0);
}

void Oss7Test::oss7_api_timerEventCallback()
{
    trace() << "timer callback called";
}

void Oss7Test::oss7_api_datastreamRxCallback()
{
    setTimer(TIMER_RX, 0); // go back to RX
}

bool Oss7Test::oss7_api_phy_rx(phy_rx_cfg_t* rx_cfg)
{
    trace() << "> RX";
    toNetworkLayer(createRadioCommand(SET_STATE, RX));
    setTimer(TIMER_RX_TIMEOUT, rx_cfg->timeout/1024.0);
    return true;
}

bool Oss7Test::oss7_api_phy_tx(phy_tx_cfg_t* tx_cfg)
{
    trace() << "> TX";
    toNetworkLayer(createGenericDataPacket(0, 0, tx_cfg->length), "0");
    toNetworkLayer(createRadioCommand(SET_STATE, TX));
    setTimer(TIMER_TX_COMPLETED, 1); // TODO determine TX time
    return true;
}

//void Oss7Test::oss7_api_phy_get_rssi()
//{
//    toNetworkLayer(createRadioCommand(SET_STATE, RX));
//    setTimer(TIMER_RSSI_MEASUREMENT_COMPLETED, this->phyDelayForSleep2Rx + this->phyDelayForValidRssi);
//}

void Oss7Test::oss7_api_trace(char* msg, uint8_t len)
{
    trace() << std::string(msg, len);
}


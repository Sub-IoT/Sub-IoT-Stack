/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "d7ap_internal.h"
#include "log.h"
#include "crc.h"
#include "debug.h"
#include "d7ap_fs.h"
#include "ng.h"
#include "random.h"
#include "compress.h"
#include "fec.h"
#include "errors.h"
#include "timer.h"
#include "engineering_mode.h"

#include "phy.h"
#include "packet_queue.h"
#include "packet.h"
#include "dll.h"

#include "hwdebug.h"
#include "hwatomic.h"

#include "MODULE_D7AP_defs.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_DLL_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_DLL, __VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_DATA(...)
#endif



typedef enum
{
    DLL_STATE_STOPPED,
    DLL_STATE_IDLE,
    DLL_STATE_SCAN_AUTOMATION,
    DLL_STATE_CSMA_CA_STARTED,
    DLL_STATE_CSMA_CA_RETRY,
    DLL_STATE_CCA1,
    DLL_STATE_CCA2,
    DLL_STATE_CCA_FAIL,
    DLL_STATE_FOREGROUND_SCAN,
    DLL_STATE_TX_FOREGROUND,
    DLL_STATE_TX_FOREGROUND_COMPLETED,
    DLL_STATE_TX_DISCARDED
} dll_state_t;

static dae_access_profile_t NGDEF(_current_access_profile);
#define current_access_profile NG(_current_access_profile)

static dae_access_profile_t NGDEF(_remote_access_profile);
#define remote_access_profile NG(_remote_access_profile)

#define NO_ACTIVE_ACCESS_CLASS 0xFF
static uint8_t NGDEF(_active_access_class);
#define active_access_class NG(_active_access_class)

static dll_state_t NGDEF(_dll_state) = DLL_STATE_STOPPED;
#define dll_state NG(_dll_state)

static packet_t* NGDEF(_current_packet);
#define current_packet NG(_current_packet)

// CSMA-CA parameters
static int16_t NGDEF(_dll_tca);
#define dll_tca NG(_dll_tca)

static int16_t NGDEF(_dll_to);
#define dll_to NG(_dll_to)

static uint16_t NGDEF(_dll_slot_duration);
#define dll_slot_duration NG(_dll_slot_duration)

static uint32_t NGDEF(_dll_cca_started);
#define dll_cca_started NG(_dll_cca_started)

static bool NGDEF(_process_received_packets_after_tx);
#define process_received_packets_after_tx NG(_process_received_packets_after_tx)

static bool NGDEF(_resume_fg_scan);
#define resume_fg_scan NG(_resume_fg_scan)

static eirp_t NGDEF(_current_eirp);
#define current_eirp NG(_current_eirp)

static channel_id_t NGDEF(_current_channel_id);
#define current_channel_id NG(_current_channel_id)

static csma_ca_mode_t NGDEF(_csma_ca_mode);
#define csma_ca_mode NG(_csma_ca_mode)

static uint8_t NGDEF(_tx_nf_method);
#define tx_nf_method NG(_tx_nf_method)

static uint8_t NGDEF(_rx_nf_method);
#define rx_nf_method NG(_rx_nf_method)

static int16_t NGDEF(_E_CCA);
#define E_CCA NG(_E_CCA)

static uint16_t NGDEF(_tsched);
#define tsched NG(_tsched)

static bool NGDEF(_guarded_channel);
#define guarded_channel NG(_guarded_channel)

static timer_tick_t guarded_channel_time_stop;

static uint8_t noisefl_last_measurements[PHY_STATUS_MAX_CHANNELS][NOISEFL_NUMBER_MEASUREMENTS]; //3 measurement per channel
static channel_status_t channels[PHY_STATUS_MAX_CHANNELS];
static uint8_t phy_status_channel_counter = 0;
static bool reset_noisefl_last_measurements = false;
static bool phy_status_file_inited = false;

static void execute_cca(void *arg);
static void execute_csma_ca(void *arg);
static void start_foreground_scan();
static void save_noise_floor(uint8_t position);
static uint8_t get_position_channel();

/*!
 * D7A timer used to perform a CCA
 */
static timer_event dll_cca_timer;

/*!
 * D7A timer used to perform a CSMA-CA
 */
static timer_event dll_csma_timer;

/*!
 * D7A timer used to start the automation scan (foreground)
 */
static timer_event dll_scan_automation_timer;

/*!
 * D7A timer used to start a background scan
 */
static timer_event dll_background_scan_timer;

/*!
 * D7A timer used to delay the processing of a received packet
 */
static timer_event dll_process_received_packet_timer;

static void switch_state(dll_state_t next_state)
{
    switch(next_state)
    {
    case DLL_STATE_CSMA_CA_STARTED:
        /*
         * In case of extension, a request can follow the response, so the
         * current state can be DLL_STATE_TX_FOREGROUND_COMPLETED
         */
        assert(dll_state == DLL_STATE_IDLE || dll_state == DLL_STATE_SCAN_AUTOMATION
               || dll_state == DLL_STATE_FOREGROUND_SCAN
               || dll_state == DLL_STATE_TX_FOREGROUND_COMPLETED);
        dll_state = next_state;
        DPRINT("Switched to DLL_STATE_CSMA_CA_STARTED");
        break;
    case DLL_STATE_CSMA_CA_RETRY:
        assert(dll_state == DLL_STATE_CCA1 || dll_state == DLL_STATE_CCA2);
        dll_state = next_state;
        DPRINT("Switched to DLL_STATE_CSMA_CA_RETRY");
        break;
    case DLL_STATE_CCA1:
        assert(dll_state == DLL_STATE_CSMA_CA_STARTED || dll_state == DLL_STATE_CSMA_CA_RETRY);
        dll_state = next_state;
        DPRINT("Switched to DLL_STATE_CCA1");
        break;
    case DLL_STATE_CCA2:
        assert(dll_state == DLL_STATE_CCA1);
        dll_state = next_state;
        DPRINT("Switched to DLL_STATE_CCA2");
        break;
    case DLL_STATE_FOREGROUND_SCAN:
        assert(dll_state == DLL_STATE_IDLE || dll_state == DLL_STATE_SCAN_AUTOMATION
               || dll_state == DLL_STATE_TX_FOREGROUND_COMPLETED);
        dll_state = next_state;
        DPRINT("Switched to DLL_STATE_FOREGROUND_SCAN");
        break;
    case DLL_STATE_IDLE:
        assert(dll_state == DLL_STATE_FOREGROUND_SCAN
               || dll_state == DLL_STATE_CCA_FAIL
               || dll_state == DLL_STATE_TX_DISCARDED
               || dll_state == DLL_STATE_TX_FOREGROUND_COMPLETED
               || dll_state == DLL_STATE_CSMA_CA_STARTED
               || dll_state == DLL_STATE_CCA1
               || dll_state == DLL_STATE_CCA2
               || dll_state == DLL_STATE_CSMA_CA_RETRY
               || dll_state == DLL_STATE_SCAN_AUTOMATION
               || dll_state == DLL_STATE_IDLE);
        dll_state = next_state;
        DPRINT("Switched to DLL_STATE_IDLE");
        break;
    case DLL_STATE_SCAN_AUTOMATION:
        assert(dll_state == DLL_STATE_FOREGROUND_SCAN
               || dll_state == DLL_STATE_IDLE
               || dll_state == DLL_STATE_TX_FOREGROUND_COMPLETED
               || dll_state == DLL_STATE_SCAN_AUTOMATION);
        dll_state = next_state;
        DPRINT("Switched to DLL_STATE_SCAN_AUTOMATION");
        break;
    case DLL_STATE_TX_FOREGROUND:
        assert(dll_state == DLL_STATE_CCA2 || dll_state == DLL_STATE_CSMA_CA_STARTED);
        dll_state = next_state;
        DPRINT("Switched to DLL_STATE_TX_FOREGROUND");
        break;
    case DLL_STATE_TX_FOREGROUND_COMPLETED:
        assert(dll_state == DLL_STATE_TX_FOREGROUND);
        dll_state = next_state;
        DPRINT("Switched to DLL_STATE_TX_FOREGROUND_COMPLETED");
        break;
    case DLL_STATE_TX_DISCARDED:
        assert(dll_state == DLL_STATE_TX_FOREGROUND);
        dll_state = next_state;
        DPRINT("Switched to DLL_STATE_TX_DISCARDED");
        break;
    case DLL_STATE_CCA_FAIL:
        assert(dll_state == DLL_STATE_CCA1 || dll_state == DLL_STATE_CCA2
                || dll_state == DLL_STATE_CSMA_CA_STARTED || dll_state == DLL_STATE_CSMA_CA_RETRY);
        dll_state = next_state;
        DPRINT("Switched to DLL_STATE_CCA_FAIL");
        break;
    default:
        assert(false);
    }

    // output state on debug pins
    switch(dll_state)
    {
        case DLL_STATE_CSMA_CA_STARTED:
        case DLL_STATE_CCA1:
        case DLL_STATE_CCA2:
        case DLL_STATE_CSMA_CA_RETRY:
        case DLL_STATE_CCA_FAIL:
          DEBUG_PIN_SET(2);
          break;
        default:
          DEBUG_PIN_CLR(2);
    }
}

static bool is_tx_busy()
{
    switch(dll_state)
    {
        case DLL_STATE_CSMA_CA_STARTED:
        case DLL_STATE_CSMA_CA_RETRY:
        case DLL_STATE_CCA1:
        case DLL_STATE_CCA2:
        case DLL_STATE_CCA_FAIL:
        case DLL_STATE_TX_FOREGROUND:
        case DLL_STATE_TX_FOREGROUND_COMPLETED:
            return true;
        default:
            return false;
    }
}

void median_measured_noisefloor(uint8_t position) {
    if(position == UINT8_MAX) {
        E_CCA = - current_access_profile.subbands[0].cca;
        return;
    }
    if(reset_noisefl_last_measurements) {
        memset(noisefl_last_measurements[position], 0, 3);
        reset_noisefl_last_measurements = false;
        DPRINT("reset CCA");
    }
    if(noisefl_last_measurements[position][0] && noisefl_last_measurements[position][1] && noisefl_last_measurements[position][2]) { //If not default 0 values
        uint8_t median = noisefl_last_measurements[position][0]>noisefl_last_measurements[position][1]?  ( noisefl_last_measurements[position][2]>noisefl_last_measurements[position][0]? noisefl_last_measurements[position][0] : (noisefl_last_measurements[position][1]>noisefl_last_measurements[position][2]? noisefl_last_measurements[position][1]:noisefl_last_measurements[position][2]) )  :  ( noisefl_last_measurements[position][2]>noisefl_last_measurements[position][1]? noisefl_last_measurements[position][1] : (noisefl_last_measurements[position][0]>noisefl_last_measurements[position][2]? noisefl_last_measurements[position][0]:noisefl_last_measurements[position][2]) );
        E_CCA = - median + 6; //Min of last 3 with 6dB offset
    } else
        E_CCA = - current_access_profile.subbands[0].cca;
}

void start_background_scan()
{
    assert(dll_state == DLL_STATE_SCAN_AUTOMATION);

    // Start a new tsched timer
    dll_background_scan_timer.next_event = tsched;
    timer_add_event(&dll_background_scan_timer);

    phy_rx_config_t config = {
        .channel_id = current_channel_id,
        .rssi_thr = E_CCA,
    };
    error_t err = phy_start_background_scan(&config, &dll_signal_packet_received);
    if(rx_nf_method == D7ADLL_MEDIAN_OF_THREE) { 
        uint8_t position = get_position_channel();
        //if current_channel in array of channels AND gotten rssi_thr smaller than pre-programmed Ecca
        if(position != UINT8_MAX && (config.rssi_thr <= - current_access_profile.subbands[0].cca)) {
            //rotate measurements and add new at the end
            memcpy(noisefl_last_measurements[position], &noisefl_last_measurements[position][1], 2);
            noisefl_last_measurements[position][2] = - config.rssi_thr;
        }

        median_measured_noisefloor(position);
        save_noise_floor(position);
    }
}

void dll_stop_background_scan()
{
    assert(dll_state == DLL_STATE_SCAN_AUTOMATION);

    timer_cancel_event(&dll_background_scan_timer);
    hw_radio_set_idle();
}

void dll_signal_packet_received(packet_t* packet)
{
    assert((dll_state == DLL_STATE_IDLE && process_received_packets_after_tx) || dll_state == DLL_STATE_FOREGROUND_SCAN || dll_state == DLL_STATE_SCAN_AUTOMATION);
    assert(packet != NULL);
    DPRINT("Processing received packet");

    if (packet->type != BACKGROUND_ADV)
    {
        uint16_t tx_duration = phy_calculate_tx_duration(current_channel_id.channel_header.ch_class,
                                                         current_channel_id.channel_header.ch_coding,
                                                         packet->hw_radio_packet.length + 1, false);
        // If the first transmission duration is greater than or equal to the Guard Interval TG,
        // the channel guard period is extended by TG following the transmission.
        guarded_channel_time_stop = packet->hw_radio_packet.rx_meta.timestamp + ((tx_duration >= t_g) ? t_g : t_g - tx_duration);
        guarded_channel = true;
    }

    if (is_tx_busy())
    {
        // this notification might be received while a TX is busy (for example after scheduling an execute_cca()).
        // make sure we don't start processing this packet before the TX is completed.
        // will be invoked again by packet_transmitted() or an CSMA failed.
        DPRINT("Postpone the processing of the received packet after Tx is completed");
        process_received_packets_after_tx = true;
        dll_process_received_packet_timer.arg = packet;
        return;
    }

    packet_queue_mark_processing(packet);
    packet_disassemble(packet);

    // TODO check if more received packets are pending
}

static void packet_received(void *arg)
{
    dll_signal_packet_received((packet_t*)arg);

}

void dll_signal_packet_transmitted(packet_t* packet)
{
    assert(dll_state == DLL_STATE_TX_FOREGROUND);
    switch_state(DLL_STATE_TX_FOREGROUND_COMPLETED);
    DPRINT("Transmitted packet @ %i with length = %i", packet->hw_radio_packet.tx_meta.timestamp, packet->hw_radio_packet.length);
  
    guarded_channel_time_stop = timer_get_counter_value() + ((packet->tx_duration >= t_g) ? t_g : t_g - packet->tx_duration);

    switch_state(DLL_STATE_IDLE);
    d7anp_signal_packet_transmitted(packet);

    if (process_received_packets_after_tx)
    {
        dll_process_received_packet_timer.next_event = 0;
        error_t rtc = timer_add_event(&dll_process_received_packet_timer);
        assert(rtc == SUCCESS);
        process_received_packets_after_tx = false;
    }

    if (resume_fg_scan)
    {
        start_foreground_scan();
        resume_fg_scan = false;
    }
}

static void discard_tx()
{
    start_atomic();
    if (dll_state == DLL_STATE_TX_FOREGROUND)
    {
        /* wait until TX completed but Tx callback is removed */
        hw_radio_set_idle();
        switch_state(DLL_STATE_TX_DISCARDED);
    }
    end_atomic();

    if ((dll_state == DLL_STATE_CCA1) || (dll_state == DLL_STATE_CCA2))
    {
        timer_cancel_event(&dll_cca_timer);
    }
    else if ((dll_state == DLL_STATE_CCA_FAIL) || (dll_state == DLL_STATE_CSMA_CA_RETRY))
    {
        timer_cancel_event(&dll_csma_timer);
    }
    else if (dll_state == DLL_STATE_TX_FOREGROUND_COMPLETED)
    {
        guarded_channel = false;
    }

    switch_state(DLL_STATE_IDLE);
}

static void cca_rssi_valid(int16_t cur_rssi)
{
    DPRINT("cca_rssi_valid @%i", timer_get_counter_value());

    // When the radio goes back to Rx state, the rssi_valid callback may be still set. Skip it in this case
    if (dll_state != DLL_STATE_CCA1 && dll_state != DLL_STATE_CCA2)
        return;

    if (cur_rssi <= E_CCA)
    {
        if((tx_nf_method == D7ADLL_MEDIAN_OF_THREE || rx_nf_method == D7ADLL_MEDIAN_OF_THREE))
        {
            uint8_t position = get_position_channel();
            memcpy(noisefl_last_measurements[position], &noisefl_last_measurements[position][1], 2);
            noisefl_last_measurements[position][2] = - cur_rssi;
            median_measured_noisefloor(position);
        }
        if (dll_state == DLL_STATE_CCA1)
        {
            phy_switch_to_standby_mode();
            DPRINT("CCA1 RSSI: %d", cur_rssi);
            switch_state(DLL_STATE_CCA2);

            // execute CCA2 directly after busy wait instead of scheduling this, to prevent another long running
            // scheduled task to interfer with this timer (for instance d7asp_received_unsollicited_data_cb() )
            hw_busy_wait(5000);
            execute_cca(NULL);
            return;
        }
        else if (dll_state == DLL_STATE_CCA2)
        {
            // OK, send packet
            error_t err;
            DPRINT("CCA2 RSSI: %d", cur_rssi);
            DPRINT("CCA2 succeeded, transmitting ...");
            // log_print_data(current_packet->hw_radio_packet.data, current_packet->hw_radio_packet.length + 1); // TODO tmp

            switch_state(DLL_STATE_TX_FOREGROUND);
            guarded_channel = true;

            if (current_packet->ETA)
            {
                DPRINT("Start background advertising @ %i", timer_get_counter_value());
                uint8_t dll_header_bg_frame[2];
                dll_assemble_packet_header_bg(current_packet, dll_header_bg_frame);

                err = phy_send_packet_with_advertising(&current_packet->hw_radio_packet,
                                                       &current_packet->phy_config.tx,
                                                       dll_header_bg_frame, current_packet->ETA,
                                                       &dll_signal_packet_transmitted);
            }
            else
            {
                err = phy_send_packet(&current_packet->hw_radio_packet, &current_packet->phy_config.tx, &dll_signal_packet_transmitted);
            }

            assert(err == SUCCESS);
            return;
        }
    }
    else
    {
        DPRINT("Channel not clear, RSSI: %i vs CCA: %i", cur_rssi, E_CCA);
        switch_state(DLL_STATE_CSMA_CA_RETRY);
        execute_csma_ca(NULL);
    }
}

static void execute_cca(void *arg)
{
    (void)arg;
    DPRINT("execute_cca @%i", timer_get_counter_value());

    assert(dll_state == DLL_STATE_CCA1 || dll_state == DLL_STATE_CCA2);

    phy_start_energy_scan(&current_channel_id, cca_rssi_valid, 160);
}

static void execute_csma_ca(void *arg)
{
    (void)arg;
    // TODO select Channel at front of the channel queue

    // update guarded channel to check if it's actually still guarded
    guarded_channel = (guarded_channel
        && (timer_calculate_difference(timer_get_counter_value(), guarded_channel_time_stop) <= t_g));
    /*
     * During the period when the channel is guarded by the Requester, the transmission
     * of a subsequent requests, or a single response to a unicast request on the
     * guarded channel, is not conditioned by CSMA-CA
     */
    if ((current_packet->type == SUBSEQUENT_REQUEST || current_packet->type == RESPONSE_TO_UNICAST
            || current_packet->type == REQUEST_IN_DIALOG_EXTENSION)
        && guarded_channel) {
        DPRINT("Guarded channel, UNC CSMA-CA");
        switch_state(DLL_STATE_TX_FOREGROUND);
        error_t rtc = phy_send_packet(&current_packet->hw_radio_packet, &current_packet->phy_config.tx, &dll_signal_packet_transmitted);
        assert(rtc == SUCCESS);
        return;
    }

    switch (dll_state)
    {
        case DLL_STATE_CSMA_CA_STARTED:
        {
            uint16_t dll_tc;

            // in case of response, use the Tc parameter provided in the request
            if (current_packet->type == RESPONSE_TO_UNICAST || current_packet->type == RESPONSE_TO_BROADCAST)
                dll_tc = CT_DECOMPRESS(current_packet->d7atp_tc);
            else
                dll_tc = ceil((SFc + 1) * current_packet->tx_duration + t_g);

            /*
             * Tca = Tc - Ttx - Tg
             * we substract also 1 tick to take into account also the switching
             * time required in the transceiver.
             */
            dll_tca = dll_tc - current_packet->tx_duration - t_g - 1;
            dll_cca_started = timer_get_counter_value();
            DPRINT("Tca= %i with Tc %i and Ttx %i", dll_tca, dll_tc, current_packet->tx_duration);

            // Adjust TCA value according the time already elapsed since the reception time in case of response
            if (current_packet->request_received_timestamp)
            {
                dll_tca -= dll_cca_started - current_packet->request_received_timestamp;
                DPRINT("Adjusted Tca= %i = %i - %i", dll_tca, dll_cca_started, current_packet->request_received_timestamp);
            }

            if (dll_tca <= 0)
            {
                log_print_error_string("Tca negative, CCA failed");
                // Let the upper layer decide eventually to change the channel in order to get a chance a send this frame
                switch_state(DLL_STATE_IDLE); // TODO in this case we should return to scan automation
                resume_fg_scan = false;
                d7anp_signal_transmission_failure();
                break;
            }

            uint16_t t_offset = 0;

            if (current_packet->type == RESPONSE_TO_BROADCAST)
                csma_ca_mode = CSMA_CA_MODE_RAIND;
            else
                csma_ca_mode = CSMA_CA_MODE_AIND;

            // TODO determine how to enable CSMA_CA_MODE_RIGD

            switch(csma_ca_mode)
            {
                case CSMA_CA_MODE_AIND: // TODO implement AIND
                {
                    // no initial delay, t_offset = 0
                    dll_slot_duration = current_packet->tx_duration;
                    break;
                }
                case CSMA_CA_MODE_RAIND:
                {
                    dll_slot_duration = current_packet->tx_duration;
                    uint16_t max_nr_slots = dll_tca / dll_slot_duration;

                    if (max_nr_slots)
                    {
                        uint16_t slots_wait = get_rnd() % max_nr_slots;
                        t_offset = slots_wait * dll_slot_duration;
                        DPRINT("RAIND: slot %i of %i", slots_wait, max_nr_slots);
                    }
                    break;
                }
                case CSMA_CA_MODE_RIGD:
                {
                    dll_slot_duration = (uint16_t) ((double)dll_tca / 2 );
                    t_offset = get_rnd() % dll_slot_duration;
                    break;
                }
            }

            DPRINT("slot duration: %i t_offset: %i csma ca mode: %i", dll_slot_duration, t_offset, csma_ca_mode);

            dll_to = dll_tca;

            if (t_offset)
            {
                switch_state(DLL_STATE_CCA1);
                dll_cca_timer.next_event = t_offset;
                error_t rtc = timer_add_event(&dll_cca_timer);
                assert(rtc == SUCCESS);
            }
            else
            {
                switch_state(DLL_STATE_CCA1);
                dll_cca_timer.next_event = 0;
                error_t rtc = timer_add_event(&dll_cca_timer);
                assert(rtc == SUCCESS);
            }

            break;
        }
        case DLL_STATE_CSMA_CA_RETRY:
        {
            int32_t cca_duration = timer_get_counter_value() - dll_cca_started;
            dll_to -= cca_duration;

            if (dll_to <= 0)
            {
                log_print_error_string("CCA fail because dll_to = %i", dll_to);
                switch_state(DLL_STATE_CCA_FAIL);
                dll_csma_timer.next_event = 0;
                error_t rtc = timer_add_event(&dll_csma_timer);
                assert(rtc == SUCCESS);
                break;
            }

            DPRINT("RETRY with dll_to = %i", dll_to);

            // TODO shift channel queue

            dll_tca = dll_to;
            dll_cca_started = timer_get_counter_value();
            uint16_t t_offset = 0;

            switch(csma_ca_mode)
            {
                case CSMA_CA_MODE_AIND:
                case CSMA_CA_MODE_RAIND:
                {
                    uint16_t max_nr_slots = dll_tca / dll_slot_duration;

                    if (max_nr_slots)
                    {
                        uint16_t slots_wait = get_rnd() % max_nr_slots;
                        t_offset = slots_wait * dll_slot_duration;
                        DPRINT("RAIND: wait %i slots of %i", slots_wait, max_nr_slots);
                    }
                    break;
                }
                case CSMA_CA_MODE_RIGD:
                {
                    dll_slot_duration >>= 1;
                    if (dll_slot_duration != 0) // TODO can be 0, validate
                        t_offset = get_rnd() % dll_slot_duration;

                    DPRINT("RIGD: slot duration: %i", dll_slot_duration);
                    break;
                }
            }

            DPRINT("t_offset: %i", t_offset);

            if (t_offset)
            {
                dll_cca_timer.next_event = t_offset;
            }
            else
            {
                dll_cca_timer.next_event = 0;
            }

            switch_state(DLL_STATE_CCA1);
            error_t rtc = timer_add_event(&dll_cca_timer);
            assert(rtc == SUCCESS);
            break;
        }
        case DLL_STATE_CCA_FAIL:
        {
            // TODO hw_radio_set_idle();
            switch_state(DLL_STATE_IDLE);
            d7anp_signal_transmission_failure();
            if (process_received_packets_after_tx)
            {
                dll_process_received_packet_timer.next_event = 0;
                error_t rtc = timer_add_event(&dll_process_received_packet_timer);
                assert(rtc == SUCCESS);
                process_received_packets_after_tx = false;
            }

            if (resume_fg_scan)
            {
                start_foreground_scan();
                resume_fg_scan = false;
            }

            reset_noisefl_last_measurements = true;
            break;
        }
    }
}

/* returns the position of the current_channel_id in the channel array. If not found and no more room, returns Max value */
static uint8_t get_position_channel() {
    uint8_t position;
    channel_status_t local_channel = {
        .ch_freq_band = current_channel_id.channel_header.ch_freq_band,
        .bandwidth_25kHz = (current_channel_id.channel_header.ch_class == PHY_CLASS_LO_RATE),
        .channel_index_lsb = (current_channel_id.center_freq_index & 0xFF),
        .channel_index_msb = (uint8_t)((current_channel_id.center_freq_index >> 8) & 0x07),
        .noise_floor = - E_CCA
    };
    for(position = 0; position < PHY_STATUS_MAX_CHANNELS; position++) {
        if(((channels[position].raw_channel_status_identifier == 0) && (channels[position].channel_index_lsb == 0)) || 
           ((channels[position].raw_channel_status_identifier == local_channel.raw_channel_status_identifier) && (channels[position].channel_index_lsb == local_channel.channel_index_lsb)))
            return position;
    }
    log_print_error_string("position of channel out of bound. Increase channels size or delete previous");
    return UINT8_MAX;
}

static void save_noise_floor(uint8_t position) {
    if(position == UINT8_MAX)
        return;
    if((channels[position].raw_channel_status_identifier == 0) && (channels[position].channel_index_lsb == 0)) { // new channel
        channels[position] = (channel_status_t) {
            .ch_freq_band = current_channel_id.channel_header.ch_freq_band,
            .bandwidth_25kHz = (current_channel_id.channel_header.ch_class == PHY_CLASS_LO_RATE),
            .channel_index_lsb = (current_channel_id.center_freq_index & 0xFF),
            .channel_index_msb = (uint8_t)((current_channel_id.center_freq_index >> 8) & 0x07),
            .noise_floor = - E_CCA
        };
        phy_status_channel_counter++;
    } else
        channels[position].noise_floor = - E_CCA;

    d7ap_fs_write_file(D7A_FILE_PHY_STATUS_FILE_ID, D7A_FILE_PHY_STATUS_MINIMUM_SIZE - 1, &phy_status_channel_counter, sizeof(uint8_t), ROOT_AUTH);
    d7ap_fs_write_file(D7A_FILE_PHY_STATUS_FILE_ID, D7A_FILE_PHY_STATUS_MINIMUM_SIZE, (uint8_t*) channels, phy_status_channel_counter * sizeof(channel_status_t), ROOT_AUTH);
}

void dll_execute_scan_automation()
{
    if (!(dll_state == DLL_STATE_IDLE || dll_state == DLL_STATE_SCAN_AUTOMATION))
        return;

    // first make sure the background scan timer is stopped and the pending task canceled
    // since they might not be necessary for current active class anymore
    timer_cancel_event(&dll_background_scan_timer);

    DPRINT("DLL execute scan autom AC=0x%02x", active_access_class);

    /*
     * The Scan Automation Parameters are uniquely defined based on the Active
     * Access Class of the device.
     * For now, the access mask and the subband bitmap are not used.
     * By default, subprofile[0] is selected and subband[0] is used.
     */

    if(current_access_profile.subprofiles[0].subband_bitmap == 0)
    {
        DPRINT("Scan autom ch list is void, not entering scan\n");
        hw_radio_set_idle();
        if(dll_state != DLL_STATE_IDLE)
          switch_state(DLL_STATE_IDLE);

        return;
    }

    switch_state(DLL_STATE_SCAN_AUTOMATION);
    phy_rx_config_t rx_cfg = {
        .channel_id.channel_header_raw = current_access_profile.channel_header_raw,
        .channel_id.center_freq_index = current_access_profile.subbands[0].channel_index_start,
    };

    // The Scan Automation TSCHED is obtained as the minimum of all selected subprofiles' TSCHED.
    uint16_t scan_period;
    tsched = (uint16_t)~0;
    for(uint8_t i = 0; i < SUBPROFILES_NB; i++)
    {
        // Only consider the selectable subprofiles (having their Access Mask bits set to 1 and having non-void subband bitmaps)
        if ((ACCESS_MASK(active_access_class) & (0x01 << i)) && current_access_profile.subprofiles[i].subband_bitmap)
        {
            scan_period = CT_DECOMPRESS(current_access_profile.subprofiles[i].scan_automation_period);
            if ((tsched == (uint16_t)~0) || (scan_period < tsched))
                tsched = scan_period;
        }
    }

    // tsched is necessarily set because at least one selectable subprofile should be found
    assert(tsched != (uint16_t)~0);

    /*
     * If the scan automation period (To) is set to 0, the scan type is set to
     * foreground,
     */
    if (tsched == 0)
    {
        rx_cfg.syncword_class = PHY_SYNCWORD_CLASS1;
        current_channel_id = rx_cfg.channel_id;
        phy_start_rx(&current_channel_id, PHY_SYNCWORD_CLASS1, &dll_signal_packet_received);
    }
    else
    {
        rx_cfg.syncword_class = PHY_SYNCWORD_CLASS0;
        current_channel_id = rx_cfg.channel_id;

        // compute Ecca = NF + Eccao
        if (rx_nf_method == D7ADLL_FIXED_NOISE_FLOOR)
        {
            //Use the default channel CCA threshold
            E_CCA = - current_access_profile.subbands[0].cca; // Eccao is set to 0 dB
        } 
        else if(rx_nf_method == D7ADLL_MEDIAN_OF_THREE) 
        {
            uint8_t position = get_position_channel();
            median_measured_noisefloor(position);
            save_noise_floor(position);
        }
        else
        {
          // TODO support the Slow RSSI Variation computation method" and possibly add other methods
          assert(false);
        }
        DPRINT("E_CCA %i", E_CCA);

        // If TSCHED > 0, an independent scheduler is set to generate regular scan start events at TSCHED rate.
        DPRINT("Perform a dll background scan at the end of TSCHED (%d ticks)", tsched);
        dll_background_scan_timer.next_event = tsched;
        error_t rtc = timer_add_event(&dll_background_scan_timer);
        assert(rtc == SUCCESS);
    }

    // Set by default the eirp in case we need to respond to an incoming request
    current_eirp = current_access_profile.subbands[0].eirp;
}

static void execute_scan_automation(void *arg)
{
    (void)arg;
    dll_execute_scan_automation();
}

static void conf_file_changed_callback(uint8_t file_id)
{
    (void)file_id;
    DPRINT("DLL config file changed");
    uint8_t scan_access_class = d7ap_fs_read_dll_conf_active_access_class();

    // if the access class has been modified, update the current access profile
    if (active_access_class != scan_access_class)
    {
        d7ap_fs_read_access_class(ACCESS_SPECIFIER(scan_access_class), &current_access_profile);
        active_access_class = scan_access_class;

        // when doing scan automation restart this
        if (dll_state == DLL_STATE_IDLE || dll_state == DLL_STATE_SCAN_AUTOMATION)
        {
            DPRINT("Re-start the scan automation to apply this change");
            dll_scan_automation_timer.next_event = 0;
            int rtc = timer_add_event(&dll_scan_automation_timer);
            assert(rtc == SUCCESS);
        }
    }
}

void dll_notify_access_profile_file_changed(uint8_t file_id)
{
    DPRINT("Access Profile changed");

    // update only the current access profile if this access profile has been changed
    if (file_id == D7A_FILE_ACCESS_PROFILE_ID + ACCESS_SPECIFIER(active_access_class))
    {
        d7ap_fs_read_access_class(ACCESS_SPECIFIER(active_access_class), &current_access_profile);

        // when we are idle switch to scan automation now as well, in case the new AP enables scanning
        if (dll_state == DLL_STATE_IDLE || dll_state == DLL_STATE_SCAN_AUTOMATION)
        {
            DPRINT("Re-start the scan automation to apply this change");
            dll_scan_automation_timer.next_event = 0;
            int rtc = timer_add_event(&dll_scan_automation_timer);
            assert(rtc == SUCCESS);
        }
    }
}

void dll_notify_dialog_terminated()
{
    DPRINT("Since the dialog is terminated, we can resume the automation scan");
    dll_execute_scan_automation();
}

void dll_init()
{
    assert(dll_state == DLL_STATE_STOPPED);

    uint8_t nf_ctrl;

    // Initialize timers
    timer_init_event(&dll_cca_timer, &execute_cca);
    timer_init_event(&dll_csma_timer, &execute_csma_ca);
    timer_init_event(&dll_scan_automation_timer, &execute_scan_automation);
    timer_init_event(&dll_background_scan_timer, &start_background_scan);
    timer_init_event(&dll_process_received_packet_timer, &packet_received);

    phy_init();


    d7ap_fs_file_header_t volatile_file_header = { 
        .file_permissions = (file_permission_t){ .guest_read = true, .user_read = true },
        .file_properties.storage_class = FS_STORAGE_VOLATILE,
        .length = D7A_FILE_PHY_STATUS_SIZE,
        .allocated_length = D7A_FILE_PHY_STATUS_SIZE }; // TODO length for multiple channels

    if(!phy_status_file_inited)
        assert(d7ap_fs_init_file(D7A_FILE_PHY_STATUS_FILE_ID, &volatile_file_header, NULL) == SUCCESS); // TODO error handling
    phy_status_file_inited = true;

    uint32_t length = D7A_FILE_DLL_CONF_NF_CTRL_SIZE;
    if (d7ap_fs_read_file(D7A_FILE_DLL_CONF_FILE_ID, D7A_FILE_DLL_CONF_NF_CTRL_OFFSET, &nf_ctrl, &length, ROOT_AUTH) != 0)
        nf_ctrl = (D7ADLL_FIXED_NOISE_FLOOR << 4) & 0x0F; // set default NF computation method if the setting is not present

    tx_nf_method = (nf_ctrl >> 4) & 0x0F;
    rx_nf_method = nf_ctrl & 0x0F;

    dll_state = DLL_STATE_IDLE;

    // caching of the active class and the selected access profile
    active_access_class = d7ap_fs_read_dll_conf_active_access_class();
    d7ap_fs_read_access_class(ACCESS_SPECIFIER(active_access_class), &current_access_profile);

    process_received_packets_after_tx = false;
    resume_fg_scan = false;

    d7ap_fs_register_file_modified_callback(D7A_FILE_DLL_CONF_FILE_ID, &conf_file_changed_callback);

#ifdef MODULE_D7AP_EM_ENABLED
    engineering_mode_init();
#endif
    length = D7A_FILE_PHY_STATUS_CHANNEL_COUNT_SIZE;
    d7ap_fs_read_file(D7A_FILE_PHY_STATUS_FILE_ID, D7A_FILE_PHY_STATUS_MINIMUM_SIZE - 1, &phy_status_channel_counter, &length, ROOT_AUTH);
    if(phy_status_channel_counter && (phy_status_channel_counter < PHY_STATUS_MAX_CHANNELS))
    {
        length = phy_status_channel_counter * D7A_FILE_PHY_STATUS_CHANNEL_SIZE;
        d7ap_fs_read_file(D7A_FILE_PHY_STATUS_FILE_ID, D7A_FILE_PHY_STATUS_MINIMUM_SIZE, (uint8_t*) channels, &length, ROOT_AUTH);
    }

    // Start immediately the scan automation
    guarded_channel = false;
    dll_execute_scan_automation();
}

void dll_stop()
{
    dll_state = DLL_STATE_STOPPED;
    timer_cancel_event(&dll_cca_timer);
    timer_cancel_event(&dll_csma_timer);
    timer_cancel_event(&dll_scan_automation_timer);
    timer_cancel_event(&dll_background_scan_timer);
    timer_cancel_event(&dll_process_received_packet_timer);

    d7ap_fs_unregister_file_modified_callback(D7A_FILE_DLL_CONF_FILE_ID);

#ifdef MODULE_D7AP_EM_ENABLED
    engineering_mode_stop();
#endif
    phy_stop();
}

void dll_tx_frame(packet_t* packet)
{
    timer_cancel_event(&dll_scan_automation_timer); //enable this code if this costly operation proves to be necessary

    if (dll_state == DLL_STATE_SCAN_AUTOMATION)
    {
        timer_cancel_event(&dll_background_scan_timer);
    }

    if (dll_state != DLL_STATE_FOREGROUND_SCAN)
    {
        hw_radio_set_idle();
        resume_fg_scan = false;
    }
    else
        resume_fg_scan = true;

    dll_header_t* dll_header = &(packet->dll_header);
    dll_header->subnet = packet->d7anp_addressee->access_class;
    DPRINT("TX with subnet=0x%02x", dll_header->subnet);

    packet->origin_access_class = active_access_class;  // strictly speaking this is a D7ANP field,
                                                        // but we set it here to prevent rereading/caching in D7ANP

    if (packet->d7atp_ctrl.ctrl_is_start && packet->d7anp_addressee != NULL) // when responding in a transaction we MAY skip targetID
        dll_header->control_target_id_type = packet->d7anp_addressee->ctrl.id_type;
    else
        dll_header->control_target_id_type = ID_TYPE_NOID;

    // if the channel is locked, we shall use the channel of the initial request
    if (packet->type == SUBSEQUENT_REQUEST || packet->type == REQUEST_IN_DIALOG_EXTENSION) // TODO MISO conditions not supported
    {
        dll_header->control_eirp_index = current_eirp + 32;

         // We need to check if the channel has really been confirmed through a previous transaction
         // otherwise we need to set packet->ETA to enable the ad-hoc sync.

        packet->phy_config.tx = (phy_tx_config_t){
            .channel_id = current_channel_id,
            .syncword_class = PHY_SYNCWORD_CLASS1,
            .eirp = current_eirp
        };
    }
    else if (packet->type == RESPONSE_TO_UNICAST || packet->type == RESPONSE_TO_BROADCAST)
    {
        dll_header->control_eirp_index = current_eirp + 32;

        packet->phy_config.tx = (phy_tx_config_t){
                .channel_id = packet->phy_config.rx.channel_id,
                .syncword_class = packet->phy_config.rx.syncword_class,
                .eirp = current_eirp
            };
    }
    else
    {
        d7ap_fs_read_access_class(packet->d7anp_addressee->access_specifier, &remote_access_profile);
        /*
         * For now the access mask and the subband bitmap are not used
         * By default, subprofile[0] is selected and subband[0] is used
         */

        // TODO generate random channel queue
        // TODO assert if no selectable subprofile can be found

        /* EIRP (dBm) = (EIRP_I – 32) dBm */

        DPRINT("AC specifier=%i channel=%i",
                         packet->d7anp_addressee->access_specifier,
                         remote_access_profile.subbands[0].channel_index_start);
        dll_header->control_eirp_index = remote_access_profile.subbands[0].eirp + 32;

        packet->phy_config.tx = (phy_tx_config_t){
            .channel_id.channel_header_raw = remote_access_profile.channel_header_raw,
            .channel_id.center_freq_index = remote_access_profile.subbands[0].channel_index_start,
            .eirp = remote_access_profile.subbands[0].eirp
        };

        // The Access TSCHED is obtained as the maximum of all selected subprofiles' TSCHED.
        uint16_t scan_period;
        tsched = 0;
        for(uint8_t i = 0; i < SUBPROFILES_NB; i++)
        {
            // Only consider the selectable subprofiles (having their Access Mask bits set to 1 and having non-void subband bitmaps)
            if ((packet->d7anp_addressee->access_mask & (0x01 << i)) && remote_access_profile.subprofiles[i].subband_bitmap)
            {
                scan_period = CT_DECOMPRESS(remote_access_profile.subprofiles[i].scan_automation_period);
                if (scan_period > tsched)
                    tsched = scan_period;
            }
        }

        /* use D7AAdvP if the receiver is engaged in ultra low power scan */
        if (tsched)
        {
            packet->ETA = tsched;
            DPRINT("This request requires ad-hoc sync with access scheduling period (Tsched) <%d> ti", packet->ETA);
        }

        packet->phy_config.tx.syncword_class = PHY_SYNCWORD_CLASS1;

        // store the channel id and eirp
        current_eirp = packet->phy_config.tx.eirp;
        current_channel_id = packet->phy_config.tx.channel_id;

        // compute Ecca = NF + Eccao
        if (tx_nf_method == D7ADLL_FIXED_NOISE_FLOOR)
        {
            //Use the default channel CCA threshold
            E_CCA = - remote_access_profile.subbands[0].cca; // Eccao is set to 0 dB
            DPRINT("fixed floor: E_CCA %i", E_CCA);
        }
        else if(tx_nf_method == D7ADLL_MEDIAN_OF_THREE)
        {
            uint8_t position = get_position_channel();
            median_measured_noisefloor(position);
        }
        else
        {
          // TODO support the Slow RSSI Variation computation method" and possibly add other methods
          assert(false);
        }
    }

    packet_assemble(packet);

    packet->tx_duration = phy_calculate_tx_duration(current_channel_id.channel_header.ch_class,
                                                    current_channel_id.channel_header.ch_coding,
                                                    packet->hw_radio_packet.length + 1, false);
    DPRINT("Packet LENGTH %d, TX DURATION %d", packet->hw_radio_packet.length, packet->tx_duration);

    current_packet = packet;

    switch_state(DLL_STATE_CSMA_CA_STARTED);

    if ((packet->type == RESPONSE_TO_UNICAST) || (packet->type == RESPONSE_TO_BROADCAST))
    {
        // If the Requester provides an Execution Delay Timeout, the Responders delay their responses
        if (packet->d7atp_ctrl.ctrl_te)
        {
            timer_tick_t Te = CT_DECOMPRESS(packet->d7atp_te);
            timer_tick_t Trpd = timer_get_counter_value() - current_packet->request_received_timestamp; //response processing delay

            // the DLL foreground scan duration TC is adjusted to start after the Execution Delay period
            current_packet->request_received_timestamp += Te;

            if (Te > Trpd)
            {
                Te -= Trpd;
                dll_csma_timer.next_event = Te;
                timer_add_event(&dll_csma_timer);
                return;
            }
            // If the response processing delay TRPD is bigger than TE,
            // Responders subtract the residue TRPD - TE from the Congestion Timeout
            // and start the DLL CSMA-CA routine immediately
        }
    }
    execute_csma_ca(NULL);
}

static void start_foreground_scan()
{
    //timer_cancel_event(&dll_scan_automation_timer);  //enable this code if this costly operation proves to be necessary

    if (dll_state == DLL_STATE_SCAN_AUTOMATION)
    {
        timer_cancel_event(&dll_background_scan_timer);
        hw_radio_set_idle();
    }

    if (dll_state == DLL_STATE_FOREGROUND_SCAN)
        return;

    switch_state(DLL_STATE_FOREGROUND_SCAN);

    // TODO, if the Requester is MISO and the Request is broadcast, the responses
    // are expected on the channel list of the Requester's Access Class and not
    // necessarily on the current channel used to send the initial request

    phy_start_rx(&current_channel_id, PHY_SYNCWORD_CLASS1, &dll_signal_packet_received);
}


void dll_start_foreground_scan()
{
    if (is_tx_busy())
        discard_tx();

    start_foreground_scan();
}

void dll_stop_foreground_scan()
{
    DPRINT("Stop FG scan @ %i\n", timer_get_counter_value());
    if(dll_state == DLL_STATE_IDLE)
        return;

    if (is_tx_busy())
        return; // will go to IDLE after TX

    DPRINT("Set the radio to idle state");
    phy_stop_rx();

    if((dll_state == DLL_STATE_SCAN_AUTOMATION) || (dll_state == DLL_STATE_IDLE))
        return; // stay in scan automation

    switch_state(DLL_STATE_IDLE);
}

uint8_t dll_assemble_packet_header_bg(packet_t* packet, uint8_t* data_ptr)
{
    *data_ptr = packet->dll_header.subnet; data_ptr += sizeof(packet->dll_header.subnet);

    uint16_t crc;
    uint8_t addr_len = packet->dll_header.control_target_id_type == ID_TYPE_VID? 2 : 8;
    crc = crc_calculate(packet->d7anp_addressee->id, addr_len);
    DPRINT("crc %04x ", crc);

    packet->dll_header.control_identifier_tag = (uint8_t)crc & 0x3F;
    DPRINT("dll_header.control_identifier_tag %x ", packet->dll_header.control_identifier_tag);

    *data_ptr = (packet->dll_header.control_target_id_type << 6) | (packet->dll_header.control_identifier_tag & 0x3F);
    DPRINT("dll_header.control %x ", *data_ptr);
    data_ptr ++;

    return 0;
}

uint8_t dll_assemble_packet_header(packet_t* packet, uint8_t* data_ptr)
{
    uint8_t* dll_header_start = data_ptr;
    *data_ptr = packet->dll_header.subnet; data_ptr += sizeof(packet->dll_header.subnet);

    if (!ID_TYPE_IS_BROADCAST(packet->dll_header.control_target_id_type))
    {
        uint8_t addr_len = packet->dll_header.control_target_id_type == ID_TYPE_VID? 2 : 8;
        *data_ptr = (packet->dll_header.control_target_id_type << 6) | (packet->dll_header.control_eirp_index & 0x3F);
        data_ptr ++;
        memcpy(data_ptr, packet->d7anp_addressee->id, addr_len); data_ptr += addr_len;
    }
    else
    {
        if (packet->dll_header.control_target_id_type == ID_TYPE_NBID)
            packet->dll_header.control_target_id_type = ID_TYPE_NOID;
        *data_ptr = (packet->dll_header.control_target_id_type << 6) | (packet->dll_header.control_eirp_index & 0x3F);
        data_ptr ++;
    }

    return data_ptr - dll_header_start;
}

bool dll_disassemble_packet_header(packet_t* packet, uint8_t* data_idx)
{
    if(packet->hw_radio_packet.length < (*data_idx + 2))
    {
        log_print_error_string("%s:%s Packet too small %d < %d",__FILE__, __FUNCTION__, packet->hw_radio_packet.length, *data_idx + 2);
        return false;
    }
    packet->dll_header.subnet = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    uint8_t FSS = ACCESS_SPECIFIER(packet->dll_header.subnet);
    uint8_t FSM = ACCESS_MASK(packet->dll_header.subnet);
    uint8_t address_len;
    uint8_t id[8];

    if ((FSS != 0x0F) && (FSS != ACCESS_SPECIFIER(active_access_class))) // check that the active access class is always set to the scan access class
    {
        DPRINT("Subnet 0x%02x does not match current access class 0x%02x, skipping packet", packet->dll_header.subnet, active_access_class);
        return false;
    }

    if ((FSM & ACCESS_MASK(active_access_class)) == 0) // check that the active access class is always set to the scan access class
    {
        DPRINT("Subnet does not match current access class, skipping packet");
        return false;
    }

    packet->dll_header.control_target_id_type  = packet->hw_radio_packet.data[(*data_idx)] >> 6 ;

    if (packet->type == BACKGROUND_ADV)
    {
        packet->dll_header.control_identifier_tag = packet->hw_radio_packet.data[(*data_idx)] & 0x3F;
        DPRINT("control_target_id_type 0x%02x Identifier Tag 0x%02x", packet->dll_header.control_target_id_type, packet->dll_header.control_identifier_tag);
    }
    else
    {
        packet->dll_header.control_eirp_index = packet->hw_radio_packet.data[(*data_idx)] & 0x3F;
        DPRINT("control_target_id_type 0x%02x EIRP index %d", packet->dll_header.control_target_id_type, packet->dll_header.control_eirp_index);
    }

    (*data_idx)++;

    if (!ID_TYPE_IS_BROADCAST(packet->dll_header.control_target_id_type))
    {
        if (packet->dll_header.control_target_id_type == ID_TYPE_UID)
        {
            d7ap_fs_read_uid(id);
            address_len = 8;
        }
        else
        {
            d7ap_read_vid(id);
            address_len = 2;
        }

        if (packet->type == BACKGROUND_ADV)
        {
            uint16_t crc;
            crc = crc_calculate(id, address_len);
            DPRINT("Identifier Tag crc %04x, tag %x", crc, packet->dll_header.control_identifier_tag);
            /* Check that the tag corresponds to the 6 least significant bits of the CRC16 */
            if (packet->dll_header.control_identifier_tag != ((uint8_t)crc & 0x3F))
            {
                log_print_error_string("Identifier Tag filtering failed, skipping packet");
                return false;
            }
        }
        else
        {
            if(packet->hw_radio_packet.length < (*data_idx + address_len))
            {
                log_print_error_string("%s:%s Packet too small %d < %d", __FILE__, __FUNCTION__, packet->hw_radio_packet.length, *data_idx + address_len);
                return false;
            }
            if (memcmp(packet->hw_radio_packet.data + (*data_idx), id, address_len) != 0)
            {
                DPRINT("Device ID filtering failed, skipping packet");
                DPRINT("OUR DEVICE ID");
                DPRINT_DATA(id, address_len);
                DPRINT("TARGET DEVICE ID");
                DPRINT_DATA(packet->hw_radio_packet.data + (*data_idx), address_len);
                return false;
            }
            (*data_idx) += address_len;
        }
    }
    // TODO filter LQ
    // TODO pass to upper layer
    // TODO Tscan -= Trx
    return true;
}


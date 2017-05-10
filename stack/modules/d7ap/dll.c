/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "log.h"
#include "dll.h"
#include "hwradio.h"
#include "packet_queue.h"
#include "packet.h"
#include "crc.h"
#include "debug.h"
#include "fs.h"
#include "ng.h"
#include "hwdebug.h"
#include "random.h"
#include "MODULE_D7AP_defs.h"
#include "hwatomic.h"
#include "compress.h"
#include "fec.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_DLL_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_DLL, __VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_DATA(...)
#endif



typedef enum
{
    DLL_STATE_IDLE,
    DLL_STATE_SCAN_AUTOMATION,
    DLL_STATE_CSMA_CA_STARTED,
    DLL_STATE_CSMA_CA_RETRY,
    DLL_STATE_CCA1,
    DLL_STATE_CCA2,
    DLL_STATE_CCA_FAIL,
    DLL_STATE_FOREGROUND_SCAN,
    DLL_STATE_TX_FOREGROUND,
    DLL_STATE_TX_BACKGROUND,
    DLL_STATE_TX_FOREGROUND_COMPLETED,
    DLL_STATE_TX_DISCARDED
} dll_state_t;

static dae_access_profile_t NGDEF(_current_access_profile);
#define current_access_profile NG(_current_access_profile)

#define NO_ACTIVE_ACCESS_CLASS 0xFF
static uint8_t NGDEF(_active_access_class);
#define active_access_class NG(_active_access_class)

static dll_state_t NGDEF(_dll_state);
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

static int16_t NGDEF(_E_CCA);
#define E_CCA NG(_E_CCA)

static uint16_t NGDEF(_tsched);
#define tsched NG(_tsched)

static bool NGDEF(_guarded_channel);
#define guarded_channel NG(_guarded_channel)

static void execute_cca();
static void execute_csma_ca();
static void start_foreground_scan();
static void packet_received(hw_radio_packet_t* hw_radio_packet);

static hw_radio_packet_t* alloc_new_packet(uint8_t length)
{
    // note we don't use length because in the current implementation the packets in the queue are of
    // fixed (maximum) size
    return &(packet_queue_alloc_packet()->hw_radio_packet);
}

static void release_packet(hw_radio_packet_t* hw_radio_packet)
{
    packet_queue_free_packet(packet_queue_find_packet(hw_radio_packet));
}

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
               || dll_state == DLL_STATE_SCAN_AUTOMATION);
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
        assert(dll_state == DLL_STATE_CCA2 || dll_state == DLL_STATE_TX_BACKGROUND);
        dll_state = next_state;
        DPRINT("Switched to DLL_STATE_TX_FOREGROUND");
        break;
    case DLL_STATE_TX_BACKGROUND:
        assert(dll_state == DLL_STATE_CCA2);
        dll_state = next_state;
        DPRINT("Switched to DLL_STATE_TX_BACKGROUND");
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

void guard_period_expiration()
{
    guarded_channel = false;
    DPRINT("Channel guarding period is terminated, Tx is now conditioned by CSMA");
}

void start_background_scan()
{
    // Start a new tsched timer
    assert(timer_post_task_delay(&start_background_scan, tsched) == SUCCESS);

    hw_rx_cfg_t rx_cfg = {
        .channel_id = current_channel_id,
        .syncword_class = PHY_SYNCWORD_CLASS0,
       };

    hw_radio_start_background_scan(&rx_cfg, &packet_received, E_CCA);
}

void dll_stop_background_scan()
{
    assert(dll_state == DLL_STATE_SCAN_AUTOMATION);

    timer_cancel_task(&start_background_scan);
    sched_cancel_task(&start_background_scan);
    hw_radio_set_idle();
}

static void process_received_packets()
{
    if (is_tx_busy())
    {
        // this task might be scheduled while a TX is busy (for example after scheduling an execute_cca()).
        // make sure we don't start processing this packet before the TX is completed.
        // will be rescheduled by packet_transmitted() or an CSMA failed.
        process_received_packets_after_tx = true;
        return;
    }

    packet_t* packet = packet_queue_get_received_packet();
    assert(packet != NULL);
    DPRINT("Processing received packet");

    if (packet->hw_radio_packet.rx_meta.rx_cfg.syncword_class == PHY_SYNCWORD_CLASS0)
        packet->type = BACKGROUND_ADV;

    packet_queue_mark_processing(packet);
    packet_disassemble(packet);

    // TODO check if more received packets are pending
}

static void packet_received(hw_radio_packet_t* hw_radio_packet)
{
    assert(dll_state == DLL_STATE_FOREGROUND_SCAN || dll_state == DLL_STATE_SCAN_AUTOMATION);

    if (hw_radio_packet->rx_meta.rx_cfg.syncword_class == PHY_SYNCWORD_CLASS1)
    {
        uint16_t tx_duration = dll_calculate_tx_duration(current_access_profile.channel_header.ch_class,
                                                         current_access_profile.channel_header.ch_coding,
                                                         hw_radio_packet->length + 1);
        // If the first transmission duration is greater than or equal to the Guard Interval TG,
        // the channel guard period is extended by TG following the transmission.
        if (tx_duration >= t_g)
            timer_post_task_prio_delay(&guard_period_expiration, t_g, MAX_PRIORITY);
        else
            timer_post_task_prio_delay(&guard_period_expiration, t_g - tx_duration, MAX_PRIORITY);

        guarded_channel = true;
    }

    // we are in interrupt context here, so mark packet for further processing,
    // schedule it and return
    DPRINT("packet received @ %i , RSSI = %i", hw_radio_packet->rx_meta.timestamp, hw_radio_packet->rx_meta.rssi);
    packet_queue_mark_received(hw_radio_packet);

    /* the received packet needs to be handled in priority */
    sched_post_task_prio(&process_received_packets, MAX_PRIORITY);
}

static void notify_transmitted_packet()
{
    packet_t* packet = packet_queue_get_transmitted_packet();
    assert(packet != NULL);

    switch_state(DLL_STATE_IDLE);
    d7anp_signal_packet_transmitted(packet);

    if (process_received_packets_after_tx)
    {
        sched_post_task_prio(&process_received_packets, MAX_PRIORITY);
        process_received_packets_after_tx = false;
    }

    if (resume_fg_scan)
    {
        start_foreground_scan();
        resume_fg_scan = false;
    }
}

void packet_transmitted(hw_radio_packet_t* hw_radio_packet)
{
    assert(dll_state == DLL_STATE_TX_FOREGROUND);
    switch_state(DLL_STATE_TX_FOREGROUND_COMPLETED);
    DPRINT("Transmitted packet @ %i with length = %i", hw_radio_packet->tx_meta.timestamp, hw_radio_packet->length);

    packet_t* packet = packet_queue_mark_transmitted(hw_radio_packet);

    if (packet->tx_duration >= t_g )
    {
        timer_post_task_prio_delay(&guard_period_expiration, t_g, MAX_PRIORITY);
    }
    else
        timer_post_task_prio_delay(&guard_period_expiration, t_g - packet->tx_duration, MAX_PRIORITY);

    /* the notification task needs to be handled in priority */
    sched_post_task_prio(&notify_transmitted_packet, MAX_PRIORITY);
}

void background_advertising_terminated(hw_radio_packet_t* hw_radio_packet)
{
    assert(dll_state == DLL_STATE_TX_BACKGROUND);
    switch_state(DLL_STATE_TX_FOREGROUND);

    current_packet->hw_radio_packet.tx_meta.tx_cfg.syncword_class = PHY_SYNCWORD_CLASS1;
    current_packet->type = INITIAL_REQUEST;
    packet_assemble(current_packet);

    current_packet->tx_duration = dll_calculate_tx_duration(current_access_profile.channel_header.ch_class,
                                                            current_access_profile.channel_header.ch_coding,
                                                            current_packet->hw_radio_packet.length + 1);

    DPRINT("Transmit packet @ %i", timer_get_counter_value()); // ensure that we are sending the foreground request within the guarded time
    hw_radio_send_packet(&current_packet->hw_radio_packet, &packet_transmitted);
    guarded_channel = true;
}

static void discard_tx()
{
    start_atomic();
    if (dll_state == DLL_STATE_TX_FOREGROUND || dll_state == DLL_STATE_TX_BACKGROUND)
    {
        /* wait until TX completed but Tx callback is removed */
        hw_radio_set_idle();
        switch_state(DLL_STATE_TX_DISCARDED);
    }
    end_atomic();

    if ((dll_state == DLL_STATE_CCA1) || (dll_state == DLL_STATE_CCA2))
    {
        timer_cancel_task(&execute_cca);
        sched_cancel_task(&execute_cca);
    }
    else if ((dll_state == DLL_STATE_CCA_FAIL) || (dll_state == DLL_STATE_CSMA_CA_RETRY))
    {
        timer_cancel_task(&execute_csma_ca);
        sched_cancel_task(&execute_csma_ca);
    }
    else if (dll_state == DLL_STATE_TX_FOREGROUND_COMPLETED)
    {
        timer_cancel_task(&guard_period_expiration);
        sched_cancel_task(&guard_period_expiration);
        guarded_channel = false;
        sched_cancel_task(&notify_transmitted_packet);
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
        if (dll_state == DLL_STATE_CCA1)
        {
            DPRINT("CCA1 RSSI: %d", cur_rssi);
            switch_state(DLL_STATE_CCA2);

            // execute CCA2 directly after busy wait instead of scheduling this, to prevent another long running
            // scheduled task to interfer with this timer (for instance d7asp_received_unsollicited_data_cb() )
            hw_busy_wait(5000);
            execute_cca();
            return;
        }
        else if (dll_state == DLL_STATE_CCA2)
        {
            // OK, send packet
            error_t err;
            DPRINT("CCA2 RSSI: %d", cur_rssi);
            DPRINT("CCA2 succeeded, transmitting ...");
            // log_print_data(current_packet->hw_radio_packet.data, current_packet->hw_radio_packet.length + 1); // TODO tmp

            if (current_packet->type == BACKGROUND_ADV)
            {
                switch_state(DLL_STATE_TX_BACKGROUND);
                DPRINT("Start background advertising @ %i", timer_get_counter_value());
                err = hw_radio_send_background_packet(&current_packet->hw_radio_packet,
                                                      &background_advertising_terminated,
                                                      current_packet->ETA, current_packet->tx_duration);

                // To save time and guarantee to send the foreground frame within the guarded time,
                // it should be optimal to prepare the foreground packet before the end of the
                // advertising period, but for now, the sending of background packet is a none interrupted
                // sequence

                //current_packet->hw_radio_packet.tx_meta.tx_cfg.syncword_class = PHY_SYNCWORD_CLASS1;
                //current_packet->type = INITIAL_REQUEST;
                //packet_assemble(current_packet);
            }
            else
            {
                switch_state(DLL_STATE_TX_FOREGROUND);
                err = hw_radio_send_packet(&current_packet->hw_radio_packet, &packet_transmitted);
                guarded_channel = true;
            }

            assert(err == SUCCESS);
            return;
        }
    }
    else
    {
        DPRINT("Channel not clear, RSSI: %i", cur_rssi);
        switch_state(DLL_STATE_CSMA_CA_RETRY);
        execute_csma_ca();
    }
}

static void execute_cca()
{
    DPRINT("execute_cca @%i", timer_get_counter_value());

    assert(dll_state == DLL_STATE_CCA1 || dll_state == DLL_STATE_CCA2);

    hw_rx_cfg_t rx_cfg =(hw_rx_cfg_t){
        .channel_id = current_channel_id,
        .syncword_class = PHY_SYNCWORD_CLASS1,
    };

    hw_radio_set_rx(&rx_cfg, NULL, &cca_rssi_valid);
}

uint16_t dll_calculate_tx_duration(phy_channel_class_t channel_class, phy_coding_t ch_coding, uint8_t packet_length)
{
    double data_rate = 6.0; // Normal rate: 6.9 bytes/tick

    if (ch_coding == PHY_CODING_FEC_PN9)
        packet_length = fec_calculated_decoded_length(packet_length);

    packet_length += sizeof(uint16_t); // Sync word

    switch (channel_class)
    {
    case PHY_CLASS_LO_RATE:
        packet_length += PREAMBLE_LOW_RATE_CLASS;
        data_rate = 1.0; // Lo Rate 9.6 kbps: 1.2 bytes/tick
        break;
    case PHY_CLASS_NORMAL_RATE:
        packet_length += PREAMBLE_NORMAL_RATE_CLASS;
        data_rate = 6.0; // Normal Rate 55.555 kbps: 6.94 bytes/tick
        break;
    case PHY_CLASS_HI_RATE:
        packet_length += PREAMBLE_HI_RATE_CLASS;
        data_rate = 20.0; // High rate 166.667 kbps: 20.83 byte/tick
    }

    // TODO Add the power ramp-up/ramp-down symbols in the packet length?

    uint16_t duration = ceil(packet_length / data_rate) + 1;
    DPRINT("Transmission duration  %i", duration);
    return duration;
}

static void execute_csma_ca()
{
    // TODO select Channel at front of the channel queue

    /*
     * During the period when the channel is guarded by the Requester, the transmission
     * of a subsequent requests, or a single response to a unicast request on the
     * guarded channel, is not conditioned by CSMA-CA
     */
    if ((current_packet->type == SUBSEQUENT_REQUEST ||
        current_packet->type == RESPONSE_TO_UNICAST) && guarded_channel)
    {
        switch_state(DLL_STATE_TX_FOREGROUND);
        assert(hw_radio_send_packet(&current_packet->hw_radio_packet, &packet_transmitted) == SUCCESS);
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
                DPRINT("Tca negative, CCA failed");
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
                timer_post_task_prio_delay(&execute_cca, t_offset, MAX_PRIORITY);
            }
            else
            {
                switch_state(DLL_STATE_CCA1);
                sched_post_task_prio(&execute_cca, MAX_PRIORITY);
            }

            break;
        }
        case DLL_STATE_CSMA_CA_RETRY:
        {
            int32_t cca_duration = timer_get_counter_value() - dll_cca_started;
            dll_to -= cca_duration;

            if (dll_to <= 0)
            {
                DPRINT("CCA fail because dll_to = %i", dll_to);
                switch_state(DLL_STATE_CCA_FAIL);
                sched_post_task_prio(&execute_csma_ca, MAX_PRIORITY);
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
                switch_state(DLL_STATE_CCA1);
                timer_post_task_prio_delay(&execute_cca, t_offset, MAX_PRIORITY);
            }
            else
            {
                switch_state(DLL_STATE_CCA1);
                sched_post_task_prio(&execute_cca, MAX_PRIORITY);
            }

            break;
        }
        case DLL_STATE_CCA_FAIL:
        {
            // TODO hw_radio_set_idle();
            switch_state(DLL_STATE_IDLE);
            d7anp_signal_transmission_failure();
            if (process_received_packets_after_tx)
            {
                sched_post_task_prio(&process_received_packets, MAX_PRIORITY);
                process_received_packets_after_tx = false;
            }

            if (resume_fg_scan)
            {
                start_foreground_scan();
                resume_fg_scan = false;
            }
            break;
        }
    }
}

void dll_execute_scan_automation()
{
    uint8_t scan_access_class = fs_read_dll_conf_active_access_class();
    if (active_access_class != scan_access_class)
    {
        fs_read_access_class(ACCESS_SPECIFIER(scan_access_class), &current_access_profile);
        active_access_class = scan_access_class;
    }

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
    hw_rx_cfg_t rx_cfg = {
        .channel_id = {
            .channel_header = current_access_profile.channel_header,
            .center_freq_index = current_access_profile.subbands[0].channel_index_start
        },
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
        hw_radio_set_rx(&rx_cfg, &packet_received, NULL);
    }
    else
    {
        rx_cfg.syncword_class = PHY_SYNCWORD_CLASS0;

        // compute Ecca = NF + Eccao
        if (tx_nf_method == D7ADLL_FIXED_NOISE_FLOOR)
        {
            //Use the default channel CCA threshold
            E_CCA = current_access_profile.subbands[0].cca; // Eccao is set to 0 dB
            DPRINT("E_CCA %i", E_CCA);
        }

        // If TSCHED > 0, an independent scheduler is set to generate regular scan start events at TSCHED rate.
        DPRINT("Perform a dll background scan at the end of TSCHED (%d ticks)", tsched);
        assert(timer_post_task_delay(&start_background_scan, tsched) == SUCCESS);
    }

    current_channel_id = rx_cfg.channel_id;
    // Set by default the eirp in case we need to respond to an incoming request
    current_eirp = current_access_profile.subbands[0].eirp;
}

void dll_notify_dll_conf_file_changed()
{
    // when doing scan automation restart this
    if (dll_state == DLL_STATE_SCAN_AUTOMATION)
    {
        dll_execute_scan_automation();
    }
}

void dll_notify_access_profile_file_changed()
{
    DPRINT("AP file changed");

    // make sure access class is re-read when entering scan automation
    active_access_class = NO_ACTIVE_ACCESS_CLASS;

    // when doing scan automation restart this
    if (dll_state == DLL_STATE_SCAN_AUTOMATION)
    {
        dll_execute_scan_automation();
    }
}

void dll_init()
{
    uint8_t nf_ctrl;

    sched_register_task(&process_received_packets);
    sched_register_task(&notify_transmitted_packet);
    sched_register_task(&execute_cca);
    sched_register_task(&execute_csma_ca);
    sched_register_task(&dll_execute_scan_automation);
    sched_register_task(&start_background_scan);
    sched_register_task(&guard_period_expiration);

    hw_radio_init(&alloc_new_packet, &release_packet);

    fs_read_file(D7A_FILE_DLL_CONF_FILE_ID, 4, &nf_ctrl, 1);
    tx_nf_method = (nf_ctrl >> 4) & 0x0F;

    dll_state = DLL_STATE_IDLE;
    active_access_class = NO_ACTIVE_ACCESS_CLASS;
    process_received_packets_after_tx = false;
    resume_fg_scan = false;
    sched_post_task(&dll_execute_scan_automation);
    guarded_channel = false;
}

void dll_tx_frame(packet_t* packet)
{
    if (dll_state == DLL_STATE_SCAN_AUTOMATION)
    {
        timer_cancel_task(&start_background_scan);
        sched_cancel_task(&start_background_scan);
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
    if (packet->type == SUBSEQUENT_REQUEST) // TODO MISO conditions not supported
    {
        dll_header->control_eirp_index = current_eirp + 32;

        packet->hw_radio_packet.tx_meta.tx_cfg = (hw_tx_cfg_t){
            .channel_id = current_channel_id,
            .syncword_class = PHY_SYNCWORD_CLASS1,
            .eirp = current_eirp
        };
    }
    else if (packet->type == RESPONSE_TO_UNICAST || packet->type == RESPONSE_TO_BROADCAST)
    {
        dll_header->control_eirp_index = current_eirp + 32;

        packet->hw_radio_packet.tx_meta.tx_cfg = (hw_tx_cfg_t){
                .channel_id = packet->hw_radio_packet.rx_meta.rx_cfg.channel_id,
                .syncword_class = packet->hw_radio_packet.rx_meta.rx_cfg.syncword_class,
                .eirp = current_eirp
            };
    }
    else
    {
        fs_read_access_class(packet->d7anp_addressee->access_specifier, &current_access_profile);
        /*
         * For now the access mask and the subband bitmap are not used
         * By default, subprofile[0] is selected and subband[0] is used
         */

        // TODO generate random channel queue
        // TODO assert if no selectable subprofile can be found

        /* EIRP (dBm) = (EIRP_I – 32) dBm */

        log_print_string("AC specifier=%i channel=%i",
                         packet->d7anp_addressee->access_specifier,
                         current_access_profile.subbands[0].channel_index_start);
        dll_header->control_eirp_index = current_access_profile.subbands[0].eirp + 32;

        packet->hw_radio_packet.tx_meta.tx_cfg = (hw_tx_cfg_t){
            .channel_id.channel_header = current_access_profile.channel_header,
            .channel_id.center_freq_index = current_access_profile.subbands[0].channel_index_start,
            .eirp = current_access_profile.subbands[0].eirp
        };

        // The Access TSCHED is obtained as the maximum of all selected subprofiles' TSCHED.
        uint16_t scan_period;
        tsched = 0;
        for(uint8_t i = 0; i < SUBPROFILES_NB; i++)
        {
            // Only consider the selectable subprofiles (having their Access Mask bits set to 1 and having non-void subband bitmaps)
            if ((packet->d7anp_addressee->access_mask & (0x01 << i)) && current_access_profile.subprofiles[i].subband_bitmap)
            {
                scan_period = CT_DECOMPRESS(current_access_profile.subprofiles[i].scan_automation_period);
                if (scan_period > tsched)
                    tsched = scan_period;
            }
        }

        /* use D7AAdvP if the receiver is engaged in ultra low power scan */
        if (tsched)
        {
            packet->type = BACKGROUND_ADV;
            packet->hw_radio_packet.tx_meta.tx_cfg.syncword_class = PHY_SYNCWORD_CLASS0;

            // TADV = TSCHED + TTX (TTX is the duration for transmitting a single D7AAdvP frame)
            packet->tx_duration = dll_calculate_tx_duration(current_access_profile.channel_header.ch_class,
                                                            current_access_profile.channel_header.ch_coding,
                                                            BACKGROUND_FRAME_LENGTH);

            packet->ETA = tsched + packet->tx_duration;
            DPRINT("First background frame contains ETA <%d> ttx %d", packet->ETA, packet->tx_duration);
        }
        else
            packet->hw_radio_packet.tx_meta.tx_cfg.syncword_class = PHY_SYNCWORD_CLASS1;

        // store the channel id and eirp
        current_eirp = current_access_profile.subbands[0].eirp;
        current_channel_id = packet->hw_radio_packet.tx_meta.tx_cfg.channel_id;

        // compute Ecca = NF + Eccao
        if (tx_nf_method == D7ADLL_FIXED_NOISE_FLOOR)
        {
            //Use the default channel CCA threshold
            E_CCA = current_access_profile.subbands[0].cca; // Eccao is set to 0 dB
            DPRINT("E_CCA %i", E_CCA);
        }
        else
        {
            //TODO support the Slow RSSI Variation computation method
            assert(false);
        }
    }

    packet_assemble(packet);

    if (packet->type != BACKGROUND_ADV)
    {
        packet->tx_duration = dll_calculate_tx_duration(current_access_profile.channel_header.ch_class,
                                                        current_access_profile.channel_header.ch_coding,
                                                        packet->hw_radio_packet.length + 1);
    }

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
                timer_post_task_prio_delay(&execute_csma_ca, Te, MAX_PRIORITY);
                return;
            }
            // If the response processing delay TRPD is bigger than TE,
            // Responders subtract the residue TRPD - TE from the Congestion Timeout
            // and start the DLL CSMA-CA routine immediately
        }
    }
    execute_csma_ca();
}

static void start_foreground_scan()
{
    if (dll_state == DLL_STATE_SCAN_AUTOMATION)
    {
        timer_cancel_task(&start_background_scan);
        sched_cancel_task(&start_background_scan);
        hw_radio_set_idle();
    }

    switch_state(DLL_STATE_FOREGROUND_SCAN);

    // TODO, if the Requester is MISO and the Request is broadcast, the responses
    // are expected on the channel list of the Requester's Access Class and not
    // necessarily on the current channel used to send the initial request

    hw_rx_cfg_t rx_cfg = (hw_rx_cfg_t){
            .channel_id = current_channel_id,
            .syncword_class = PHY_SYNCWORD_CLASS1,
    };

    hw_radio_set_rx(&rx_cfg, &packet_received, NULL);
}


void dll_start_foreground_scan()
{
    if (is_tx_busy())
        discard_tx();

    start_foreground_scan();
}

void dll_stop_foreground_scan(bool auto_scan)
{
    if (is_tx_busy())
        discard_tx();

    if (auto_scan && dll_state == DLL_STATE_SCAN_AUTOMATION)
        return;

    if (auto_scan)
        dll_execute_scan_automation();
    else
    {
        DPRINT("Set the radio to idle state");
        hw_radio_set_idle();
    }
}

uint8_t dll_assemble_packet_header(packet_t* packet, uint8_t* data_ptr)
{
    uint8_t* dll_header_start = data_ptr;
    *data_ptr = packet->dll_header.subnet; data_ptr += sizeof(packet->dll_header.subnet);

    if (!ID_TYPE_IS_BROADCAST(packet->dll_header.control_target_id_type))
    {
        uint8_t addr_len = packet->dll_header.control_target_id_type == ID_TYPE_VID? 2 : 8;

        if (packet->type == BACKGROUND_ADV)
        {
            uint16_t crc;
            crc = crc_calculate(packet->d7anp_addressee->id, addr_len);
            DPRINT("crc %04x ", crc);

            packet->dll_header.control_identifier_tag = (uint8_t)crc & 0x3F;
            DPRINT("dll_header.control_identifier_tag %x ", packet->dll_header.control_identifier_tag);

            *data_ptr = (packet->dll_header.control_target_id_type << 6) | (packet->dll_header.control_identifier_tag & 0x3F);
            DPRINT("dll_header.control %x ", *data_ptr);
            data_ptr ++;
        }
        else
        {
            *data_ptr = (packet->dll_header.control_target_id_type << 6) | (packet->dll_header.control_eirp_index & 0x3F);
            data_ptr ++;
            memcpy(data_ptr, packet->d7anp_addressee->id, addr_len); data_ptr += addr_len;
        }
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
            fs_read_uid(id);
            address_len = 8;
        }
        else
        {
            fs_read_vid(id);
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
                DPRINT("Identifier Tag filtering failed, skipping packet");
                return false;
            }
        }
        else
        {
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


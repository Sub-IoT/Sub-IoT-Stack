/*
 * Copyright (C) 2016 Fundación Inria Chile
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_sx127x
 * @{
 * @file
 * @brief       Netdev adaptation for the sx127x driver
 *
 * @author      Eugene P. <ep@unwds.com>
 * @author      José Ignacio Alamos <jose.alamos@inria.cl>
 * @}
 */

#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include "net/netopt.h"
#include "net/netdev.h"
#include "net/iolist.h"
#include "net/lora.h"

#include "sx127x_registers.h"
#include "sx127x_internal.h"
#include "sx127x_netdev.h"
#include "sx127x_params.h"
#include "sx127x.h"
#include "hwradio.h"
#include "hwsystem.h"
#include "debug.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(HAL_RADIO_LOG_ENABLED)
#include "log.h"
    #define DEBUG(...) log_print_string(__VA_ARGS__)
    #define DEBUG_DATA(...) log_print_data(__VA_ARGS__)
#else
    #define DEBUG(...)
    #define DEBUG_DATA(...)
#endif

/* Internal helper functions */
static int _set_state(sx127x_t *dev, netopt_state_t state);
static int _get_state(sx127x_t *dev, void *val);
void _on_dio0_irq(void *arg);
void _on_dio1_irq(void *arg);
void _on_dio2_irq(void *arg);
void _on_dio3_irq(void *arg);

bool init_done = false;

/* Netdev driver api functions */
static int _send(netdev_t *netdev, const iolist_t *iolist);
static int _recv(netdev_t *netdev, void *buf, size_t len, void *info);
static int _init(netdev_t *netdev);
static void _isr(netdev_t *netdev);
static int _get(netdev_t *netdev, netopt_t opt, void *val, size_t max_len);
static int _set(netdev_t *netdev, netopt_t opt, const void *val, size_t len);

struct xcvr_handle {
    sx127x_t sx127x;
};

const netdev_driver_t sx127x_driver = {
    .send = _send,
    .recv = _recv,
    .init = _init,
    .isr = _isr,
    .get = _get,
    .set = _set,
};

xcvr_handle_t xcvr = { .sx127x.netdev.driver = &sx127x_driver};

static void dump_register(sx127x_t *dev)
{

    DEBUG("************************DUMP REGISTER*********************");

    for (uint8_t add=0; add <= SX127X_REG_VERSION; add++)
    	DEBUG("ADDR %2X DATA %02X \r\n", add, sx127x_reg_read(dev, add));

    // Please note that when reading the first byte of the FIFO register, this
    // byte is removed so the dump is not recommended before a TX or take care
    // to fill it after the dump

    DEBUG("**********************************************************");
}

static int _send(netdev_t *netdev, const iolist_t *iolist)
{
    sx127x_t *dev = (sx127x_t*) netdev;

    // assume only one item in this list
    uint16_t size = iolist->iol_len; //iolist_size(iolist);

    if (sx127x_get_state(dev) == SX127X_RF_TX_RUNNING)
    {
        // Check if refill is expected
        if (dev->options & SX127X_OPT_TELL_TX_REFILL)
        {
            /* refill the payload buffer */
            memcpy((void*)dev->packet.buf, iolist->iol_base, size);
            dev->packet.pos = 0;
            dev->packet.length = size;
            return 0;
        }
        else
        {
            DEBUG("[WARNING] Cannot send packet: radio already in transmitting "
                  "state.\n");
            return -ENOTSUP;
        }
    }

    /* FIFO operations can not take place in Sleep mode
     * So wake up the chip */
    if (sx127x_get_op_mode(dev) == SX127X_RF_OPMODE_SLEEP) {
        sx127x_set_standby(dev);
        hw_busy_wait(SX127X_RADIO_WAKEUP_TIME); /* wait for chip wake up */
    }

    sx127x_reg_write(dev, SX127X_REG_DIOMAPPING1, 0x00); // FIFO LEVEL ISR or Packet Sent ISR
    sx127x_reg_write(dev, SX127X_REG_FIFOTHRESH, 0x80 | (SX127X_FIFO_MID_SIZE - 1));
    sx127x_flush_fifo(dev);

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:

            dev->packet.length = size;

            if (size > SX127X_FIFO_MAX_SIZE)
            {
                sx127x_reg_write(dev, SX127X_REG_FIFOTHRESH, 0x80 | (SX127X_FIFO_MID_SIZE - 1));
                dev->packet.fifothresh = SX127X_FIFO_MID_SIZE;

                memcpy((void*)dev->packet.buf, iolist->iol_base, size);
                /* Write payload buffer */
                sx127x_write_fifo(dev, dev->packet.buf, SX127X_FIFO_MAX_SIZE);

                dev->packet.pos = SX127X_FIFO_MAX_SIZE;
                hw_gpio_set_edge_interrupt(dev->params.dio1_pin, GPIO_FALLING_EDGE);
                hw_gpio_enable_interrupt(dev->params.dio1_pin);
            }
            else
            {
                dev->packet.pos = size;
                if (dev->options & SX127X_OPT_TELL_TX_REFILL) // we expect to refill the FIFO with subsequent data
                {
                    sx127x_reg_write(dev, SX127X_REG_FIFOTHRESH, 0x81); // FIFO level interrupt if under 2 bytes
                    dev->packet.fifothresh = 2;
                    sx127x_write_fifo(dev, iolist->iol_base, size);
                    hw_gpio_set_edge_interrupt(dev->params.dio1_pin, GPIO_FALLING_EDGE);
                    hw_gpio_enable_interrupt(dev->params.dio1_pin);
                }
                else
                {
                    sx127x_write_fifo(dev, iolist->iol_base, size);
                    hw_gpio_set_edge_interrupt(dev->params.dio0_pin, GPIO_RISING_EDGE);
                    hw_gpio_enable_interrupt(dev->params.dio0_pin);
                }
            }

            sx127x_set_packet_handler_enabled(dev, true);
            break;
        case SX127X_MODEM_LORA:
            /* Initializes the payload size */
            sx127x_set_payload_length(dev, size);

            /* Full buffer used for Tx */
            sx127x_reg_write(dev, SX127X_REG_LR_FIFOTXBASEADDR, 0x00);
            sx127x_reg_write(dev, SX127X_REG_LR_FIFOADDRPTR, 0x00);

            /* FIFO operations can not take place in Sleep mode
             * So wake up the chip */
            if (sx127x_get_op_mode(dev) == SX127X_RF_OPMODE_SLEEP) {
                sx127x_set_standby(dev);
                hw_busy_wait(SX127X_RADIO_WAKEUP_TIME); /* wait for chip wake up */
            }

            /* Write payload buffer */
            for (const iolist_t *iol = iolist; iol; iol = iol->iol_next) {
                sx127x_write_fifo(dev, iol->iol_base, iol->iol_len);
            }

            /* Enable TXDONE interrupt */
            sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGSMASK,
                             SX127X_RF_LORA_IRQFLAGS_RXTIMEOUT |
                             SX127X_RF_LORA_IRQFLAGS_RXDONE |
                             SX127X_RF_LORA_IRQFLAGS_PAYLOADCRCERROR |
                             SX127X_RF_LORA_IRQFLAGS_VALIDHEADER |
                             /* SX127X_RF_LORA_IRQFLAGS_TXDONE | */
                             SX127X_RF_LORA_IRQFLAGS_CADDONE |
                             SX127X_RF_LORA_IRQFLAGS_FHSSCHANGEDCHANNEL |
                             SX127X_RF_LORA_IRQFLAGS_CADDETECTED);

            /* Set TXDONE interrupt to the DIO0 line */
            sx127x_reg_write(dev, SX127X_REG_DIOMAPPING1,
                             (sx127x_reg_read(dev, SX127X_REG_DIOMAPPING1) &
                              SX127X_RF_LORA_DIOMAPPING1_DIO0_MASK) |
                             SX127X_RF_LORA_DIOMAPPING1_DIO0_01);
            break;
        default:
            DEBUG("sx127x_netdev, Unsupported modem");
            break;
    }

    if (!(dev->options & SX127X_OPT_PRELOADING))
    {
        /* Start TX timeout timer */
        if (dev->settings.modem == SX127X_MODEM_LORA)
        {
            dev->_internal.tx_timeout_timer.next_event = dev->settings.lora.tx_timeout; // TODO convert timeout in timer_tick
            timer_add_event(&dev->_internal.tx_timeout_timer);
        }

        /* Put chip into transfer mode */
        sx127x_set_state(dev, SX127X_RF_TX_RUNNING);
        sx127x_set_op_mode(dev, SX127X_RF_OPMODE_TRANSMITTER);
    }

    return 0;
}

static int _recv(netdev_t *netdev, void *buf, size_t len, void *info)
{
    sx127x_t *dev = (sx127x_t*) netdev;
    volatile uint8_t irq_flags = 0;
    uint8_t size = 0;
    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            /* just return length when buf == NULL */
            if (buf == NULL) {
                return dev->packet.length;
            }

            if (dev->packet.length > len) {
                return -ENOSPC;
            }

            memcpy(buf, (void*)dev->packet.buf, dev->packet.length);
            if (info != NULL) {
                netdev_sx127x_packet_info_t *packet_info = info;

                packet_info->rssi = sx127x_read_rssi(dev);
                packet_info->lqi = 0;
            }
            return dev->packet.length;
            break;
        case SX127X_MODEM_LORA:
            /* Clear IRQ */
            sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGS, SX127X_RF_LORA_IRQFLAGS_RXDONE);

            irq_flags = sx127x_reg_read(dev, SX127X_REG_LR_IRQFLAGS);
            if ( (irq_flags & SX127X_RF_LORA_IRQFLAGS_PAYLOADCRCERROR_MASK) ==
                 SX127X_RF_LORA_IRQFLAGS_PAYLOADCRCERROR) {
                /* Clear IRQ */
                sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGS,
                                 SX127X_RF_LORA_IRQFLAGS_PAYLOADCRCERROR);

                if (!(dev->settings.lora.flags & SX127X_RX_CONTINUOUS_FLAG)) {
                    sx127x_set_state(dev, SX127X_RF_IDLE);
                }

                timer_cancel_event(&dev->_internal.rx_timeout_timer);
                netdev->event_callback(netdev, NETDEV_EVENT_CRC_ERROR);
                return -EBADMSG;
            }

            netdev_sx127x_packet_info_t *packet_info = info;
            if (packet_info) {
                /* there is no LQI for LoRa */
                packet_info->lqi = 0;
                uint8_t snr_value = sx127x_reg_read(dev, SX127X_REG_LR_PKTSNRVALUE);
                if (snr_value & 0x80) { /* The SNR is negative */
                    /* Invert and divide by 4 */
                    packet_info->snr = -1 * ((~snr_value + 1) & 0xFF) >> 2;
                }
                else {
                    /* Divide by 4 */
                    packet_info->snr = (snr_value & 0xFF) >> 2;
                }

                int16_t rssi = sx127x_reg_read(dev, SX127X_REG_LR_PKTRSSIVALUE);

                if (packet_info->snr < 0) {
#if defined(MODULE_SX1272)
                    packet_info->rssi = SX127X_RSSI_OFFSET + rssi + (rssi >> 4) + packet_info->snr;
#else /* MODULE_SX1276 */
                    if (dev->settings.channel > SX127X_RF_MID_BAND_THRESH) {
                        packet_info->rssi = SX127X_RSSI_OFFSET_HF + rssi + (rssi >> 4) + packet_info->snr;
                    }
                    else {
                        packet_info->rssi = SX127X_RSSI_OFFSET_LF + rssi + (rssi >> 4) + packet_info->snr;
                    }
#endif
                }
                else {
#if defined(MODULE_SX1272)
                    packet_info->rssi = SX127X_RSSI_OFFSET + rssi + (rssi >> 4);
#else /* MODULE_SX1276 */
                    if (dev->settings.channel > SX127X_RF_MID_BAND_THRESH) {
                        packet_info->rssi = SX127X_RSSI_OFFSET_HF + rssi + (rssi >> 4);
                    }
                    else {
                        packet_info->rssi = SX127X_RSSI_OFFSET_LF + rssi + (rssi >> 4);
                    }
#endif
                }
                packet_info->time_on_air = sx127x_get_time_on_air(dev, len);
            }

            size = sx127x_reg_read(dev, SX127X_REG_LR_RXNBBYTES);
            if (buf == NULL) {
                return size;
            }

            if (size > len) {
                return -ENOBUFS;
            }

            if (!(dev->settings.lora.flags & SX127X_RX_CONTINUOUS_FLAG)) {
                sx127x_set_state(dev, SX127X_RF_IDLE);
            }

            timer_cancel_event(&dev->_internal.rx_timeout_timer);

            /* Read the last packet from FIFO */
            uint8_t last_rx_addr = sx127x_reg_read(dev, SX127X_REG_LR_FIFORXCURRENTADDR);
            sx127x_reg_write(dev, SX127X_REG_LR_FIFOADDRPTR, last_rx_addr);
            sx127x_read_fifo(dev, (uint8_t*)buf, size);
            break;
        default:
            break;
    }

    return size;
}

static int _init(netdev_t *netdev)
{
    sx127x_t *sx127x = (sx127x_t*) netdev;

    if (init_done)
        return 0;

    sx127x->irq = 0;
    sx127x_radio_settings_t settings;
    settings.channel = SX127X_CHANNEL_DEFAULT;
    settings.modem = SX127X_MODEM_DEFAULT;
    settings.state = SX127X_RF_IDLE;

    sx127x->settings = settings;
    memcpy(&sx127x->params, sx127x_params, sizeof(sx127x_params_t));

    /* Launch initialization of driver and device */
    DEBUG("[sx127x] netdev: initializing driver...\n");
    if (sx127x_init(sx127x) != SX127X_INIT_OK) {
        DEBUG("[sx127x] netdev: initialization failed\n");
        return -1;
    }

    sx127x_init_radio_settings(sx127x);
    /* Put chip into sleep */
    sx127x_set_sleep(sx127x);

    DEBUG("[sx127x] init_radio: sx127x initialization done\n");
    init_done = true;

    return 0;
}

static void _isr(netdev_t *netdev)
{
    sx127x_t *dev = (sx127x_t *) netdev;

    uint8_t irq = dev->irq;
    dev->irq = 0;

    switch (irq) {
        case SX127X_IRQ_DIO0:
            _on_dio0_irq(dev);
            break;

        case SX127X_IRQ_DIO1:
            _on_dio1_irq(dev);
            break;

        case SX127X_IRQ_DIO2:
            _on_dio2_irq(dev);
            break;

        case SX127X_IRQ_DIO3:
            _on_dio3_irq(dev);
            break;

        default:
            break;
    }
}

static int _get(netdev_t *netdev, netopt_t opt, void *val, size_t max_len)
{
    sx127x_t *dev = (sx127x_t*) netdev;

    if (dev == NULL) {
        return -ENODEV;
    }

    switch(opt) {
        case NETOPT_STATE:
            assert(max_len >= sizeof(netopt_state_t));
            return _get_state(dev, val);

        case NETOPT_DEVICE_TYPE:
            assert(max_len >= sizeof(uint16_t));
            *((uint16_t*) val) = NETDEV_TYPE_LORA; // FIXME
            return sizeof(uint16_t);

        case NETOPT_CHANNEL_FREQUENCY:
            assert(max_len >= sizeof(uint32_t));
            *((uint32_t*) val) = sx127x_get_channel(dev);
            return sizeof(uint32_t);

        case NETOPT_BANDWIDTH:
            assert(max_len >= sizeof(uint32_t));
            *((uint32_t*) val) = sx127x_get_bandwidth(dev);
            return sizeof(uint32_t);

        case NETOPT_SPREADING_FACTOR:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = sx127x_get_spreading_factor(dev);
            return sizeof(uint8_t);

        case NETOPT_CODING_RATE:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = sx127x_get_coding_rate(dev);
            return sizeof(uint8_t);

        case NETOPT_BITRATE:
            assert(max_len >= sizeof(uint32_t));
            *((uint32_t*) val) = sx127x_get_bitrate(dev);
            return sizeof(uint32_t);

        case NETOPT_MAX_PACKET_SIZE:
            assert(max_len >= sizeof(uint16_t));
            *((uint16_t*) val) = sx127x_get_max_payload_len(dev);
            return sizeof(uint16_t);

        case NETOPT_INTEGRITY_CHECK:
            assert(max_len >= sizeof(netopt_enable_t));
            *((netopt_enable_t*) val) = sx127x_get_crc(dev) ? NETOPT_ENABLE : NETOPT_DISABLE;
            return sizeof(netopt_enable_t);

        case NETOPT_CHANNEL_HOP:
            assert(max_len >= sizeof(netopt_enable_t));
            *((netopt_enable_t*) val) = (dev->settings.lora.flags & SX127X_CHANNEL_HOPPING_FLAG) ? NETOPT_ENABLE : NETOPT_DISABLE;
            return sizeof(netopt_enable_t);

        case NETOPT_CHANNEL_HOP_PERIOD:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = sx127x_get_hop_period(dev);
            return sizeof(uint8_t);

        case NETOPT_SINGLE_RECEIVE:
            assert(max_len >= sizeof(uint8_t));
            *((netopt_enable_t*) val) = sx127x_get_rx_single(dev) ? NETOPT_ENABLE : NETOPT_DISABLE;
            return sizeof(netopt_enable_t);

        case NETOPT_TX_POWER:
            assert(max_len >= sizeof(int16_t));
            *((int16_t*) val) = (int16_t)sx127x_get_tx_power(dev);
            return sizeof(int16_t);

        case NETOPT_IQ_INVERT:
            assert(max_len >= sizeof(uint8_t));
            *((netopt_enable_t*) val) = sx127x_get_iq_invert(dev) ? NETOPT_ENABLE : NETOPT_DISABLE;
            return sizeof(netopt_enable_t);

        case NETOPT_SYNC_LENGTH:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = sx127x_get_syncword_length(dev);
            return sizeof(uint8_t);

        case NETOPT_SYNC_WORD:
            return sx127x_get_syncword(dev, (uint8_t*)val, max_len);

        case NETOPT_FDEV:
            assert(max_len >= sizeof(uint32_t));
            *((uint32_t*) val) = sx127x_get_tx_fdev(dev);
            return sizeof(uint32_t);

        case NETOPT_MOD_SHAPING:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = sx127x_get_modulation_shaping(dev);
            return sizeof(uint8_t);

        case NETOPT_CCA_THRESHOLD:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = sx127x_get_rssi_threshold(dev);
            return sizeof(uint8_t);

        case NETOPT_RSSI_SMOOTHING:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = sx127x_get_rssi_smoothing(dev);
            return sizeof(uint8_t);

        case NETOPT_RSSI_OFFSET:
            assert(max_len >= sizeof(int8_t));
            *((int8_t*) val) = sx127x_get_rssi_offset(dev);
            return sizeof(int8_t);

        case NETOPT_SYNC_ON:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = sx127x_get_sync_on(dev);
            return sizeof(uint8_t);

        case NETOPT_PREAMBLE_DETECT_ON:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = sx127x_get_preamble_detect_on(dev);
            return sizeof(uint8_t);

        case NETOPT_PREAMBLE_LENGTH:
            assert(max_len >= sizeof(uint16_t));
            *((uint16_t*) val) = sx127x_get_preamble_length(dev);
            return sizeof(uint16_t);

        case NETOPT_PREAMBLE_POLARITY:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = sx127x_get_preamble_polarity(dev);
            return sizeof(uint8_t);

        case NETOPT_PREAMBLE_DETECT_SIZE:
             assert(max_len >= sizeof(uint8_t));
             *((uint8_t*) val) = sx127x_get_preamble_detect_size(dev);
             return sizeof(uint8_t);

        case NETOPT_PREAMBLE_DETECT_TOLERANCE:
             assert(max_len >= sizeof(uint8_t));
             *((uint8_t*) val) = sx127x_get_preamble_detect_tol(dev);
             return sizeof(uint8_t);

        case NETOPT_DC_FREE_SCHEME:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = sx127x_get_dc_free(dev);
            return sizeof(uint8_t);

        case NETOPT_RSSI_VALUE:
            assert(max_len >= sizeof(int16_t));
            *((int16_t*) val) = sx127x_read_rssi(dev);
            return sizeof(int16_t);

        case NETOPT_PRELOADING:
            if (dev->options & SX127X_OPT_PRELOADING) {
                *((netopt_enable_t *)val) = NETOPT_ENABLE;
            }
            else {
                *((netopt_enable_t *)val) = NETOPT_DISABLE;
            }
            return sizeof(netopt_enable_t);

        case NETOPT_FIFOTHRESHOLD:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t*) val) = dev->packet.fifothresh;
            return sizeof(uint8_t);

        default:
            break;
    }

    return -ENOTSUP;
}

static int _set(netdev_t *netdev, netopt_t opt, const void *val, size_t len)
{
    sx127x_t *dev = (sx127x_t*) netdev;
    int res = -ENOTSUP;

    if (dev == NULL) {
        return -ENODEV;
    }

    switch(opt) {
        case NETOPT_STATE:
            assert(len <= sizeof(netopt_state_t));
            return _set_state(dev, *((const netopt_state_t*) val));

        case NETOPT_DEVICE_TYPE:
             assert(len <= sizeof(uint16_t));
             if ((*(const uint16_t*) val == NETDEV_TYPE_LORA) ||
                 (*(const uint16_t*) val == NETDEV_TYPE_FSK)) {
                 sx127x_set_modem(dev, *(const uint16_t*) val);
                 return sizeof(uint16_t);
             }
             else {
                 return -EINVAL;
             }

        case NETOPT_CHANNEL_FREQUENCY:
            assert(len <= sizeof(uint32_t));
            sx127x_set_channel(dev, *((const uint32_t*) val));
            return sizeof(uint32_t);

        case NETOPT_BANDWIDTH:
            assert(len <= sizeof(uint32_t));
            uint32_t bw = *((const uint32_t *)val);
            if ((dev->settings.modem == SX127X_MODEM_LORA) && (bw > LORA_BW_500_KHZ)) {
                res = -EINVAL;
                break;
            }

            sx127x_set_bandwidth(dev, bw);
            return sizeof(uint8_t);

        case NETOPT_SPREADING_FACTOR:
            assert(len <= sizeof(uint8_t));
            uint8_t sf = *((const uint8_t *)val);
            if ((sf < LORA_SF6) || (sf > LORA_SF12)) {
                res = -EINVAL;
                break;
            }
            sx127x_set_spreading_factor(dev, sf);
            return sizeof(uint8_t);

        case NETOPT_CODING_RATE:
            assert(len <= sizeof(uint8_t));
            uint8_t cr = *((const uint8_t *)val);
            if ((cr < LORA_CR_4_5) || (cr > LORA_CR_4_8)) {
                res = -EINVAL;
                break;
            }
            sx127x_set_coding_rate(dev, cr);
            return sizeof(uint8_t);

        case NETOPT_BITRATE:
            assert(len <= sizeof(uint32_t));
            sx127x_set_bitrate(dev, *((const uint32_t*) val));
            return sizeof(uint32_t);

        case NETOPT_MAX_PACKET_SIZE:
            assert(len <= sizeof(uint16_t));
            sx127x_set_max_payload_len(dev, *((const uint16_t*) val));
            return sizeof(uint16_t);

        case NETOPT_INTEGRITY_CHECK:
            assert(len <= sizeof(netopt_enable_t));
            sx127x_set_crc(dev, *((const netopt_enable_t*) val) ? true : false);
            return sizeof(netopt_enable_t);

        case NETOPT_CHANNEL_HOP:
            assert(len <= sizeof(netopt_enable_t));
            sx127x_set_freq_hop(dev, *((const netopt_enable_t*) val) ? true : false);
            return sizeof(netopt_enable_t);

        case NETOPT_CHANNEL_HOP_PERIOD:
            assert(len <= sizeof(uint8_t));
            sx127x_set_hop_period(dev, *((const uint8_t*) val));
            return sizeof(uint8_t);

        case NETOPT_SINGLE_RECEIVE:
            assert(len <= sizeof(netopt_enable_t));
            sx127x_set_rx_single(dev, *((const netopt_enable_t*) val) ? true : false);
            return sizeof(netopt_enable_t);

        case NETOPT_RX_TIMEOUT:
            assert(len <= sizeof(uint32_t));
            sx127x_set_rx_timeout(dev, *((const uint32_t*) val));
            return sizeof(uint32_t);

        case NETOPT_TX_TIMEOUT:
            assert(len <= sizeof(uint32_t));
            sx127x_set_tx_timeout(dev, *((const uint32_t*) val));
            return sizeof(uint32_t);

        case NETOPT_TX_POWER:
            assert(len <= sizeof(int8_t));
            int8_t power = *((const int8_t *)val);
            sx127x_set_tx_power(dev, (int8_t)power);
            return sizeof(int16_t);

        case NETOPT_FIXED_HEADER:
            assert(len <= sizeof(netopt_enable_t));
            sx127x_set_fixed_header_len_mode(dev, *((const netopt_enable_t*) val) ? true : false);
            return sizeof(netopt_enable_t);

        case NETOPT_PREAMBLE_LENGTH:
            assert(len <= sizeof(uint16_t));
            sx127x_set_preamble_length(dev, *((const uint16_t*) val));
            return sizeof(uint16_t);

        case NETOPT_PREAMBLE_POLARITY:
            assert(len <= sizeof(uint8_t));
            sx127x_set_preamble_polarity(dev, *((const uint8_t*) val));
            return sizeof(uint8_t);

        case NETOPT_PREAMBLE_DETECT_ON:
            assert(len <= sizeof(uint8_t));
            sx127x_set_preamble_detect_on(dev, *((const uint8_t*) val));
            return sizeof(uint8_t);

        case NETOPT_PREAMBLE_DETECT_SIZE:
            assert(len <= sizeof(uint8_t));
            sx127x_set_preamble_detect_size(dev, *((const uint8_t*) val));
            return sizeof(uint8_t);

        case NETOPT_PREAMBLE_DETECT_TOLERANCE:
            assert(len <= sizeof(uint8_t));
            sx127x_set_preamble_detect_tol(dev, *((const uint8_t*) val));
            return sizeof(uint8_t);

        case NETOPT_IQ_INVERT:
            assert(len <= sizeof(netopt_enable_t));
            sx127x_set_iq_invert(dev, *((const netopt_enable_t*) val) ? true : false);
            return sizeof(bool);

        case NETOPT_SYNC_ON:
            assert(len <= sizeof(uint8_t));
            sx127x_set_sync_on(dev, *((const uint8_t*) val));
            return sizeof(uint8_t);

        case NETOPT_SYNC_LENGTH:
            assert(len <= sizeof(uint8_t));
            sx127x_set_syncword_length(dev, *((const uint8_t*) val));
            return sizeof(uint8_t);

        case NETOPT_SYNC_WORD:
            sx127x_set_syncword(dev, (uint8_t*)val, len);
            return len;

        case NETOPT_FDEV:
            assert(len <= sizeof(uint32_t));
            sx127x_set_tx_fdev(dev, *((const uint32_t*) val));
            return sizeof(uint32_t);

        case NETOPT_MOD_SHAPING:
            assert(len <= sizeof(uint8_t));
            sx127x_set_modulation_shaping(dev, *((const uint8_t*) val));
            return sizeof(uint8_t);

        case NETOPT_CCA_THRESHOLD:
            assert(len <= sizeof(uint8_t));
            sx127x_set_rssi_threshold(dev, *((const uint8_t*) val));
            return sizeof(uint8_t);

        case NETOPT_RSSI_SMOOTHING:
            assert(len <= sizeof(uint8_t));
            sx127x_set_rssi_smoothing(dev, *((const uint8_t*) val));
            return sizeof(uint8_t);

        case NETOPT_RSSI_OFFSET:
            assert(len <= sizeof(int8_t));
            sx127x_set_rssi_offset(dev, *((const int8_t*) val));
            return sizeof(int8_t);

        case NETOPT_DC_FREE_SCHEME:
            assert(len <= sizeof(uint8_t));
            sx127x_set_dc_free(dev, *((const uint8_t*) val));
            return sizeof(uint8_t);

        case NETOPT_RX_START_IRQ:
            return sx127x_set_option(dev, SX127X_OPT_TELL_RX_START, *((const bool *)val));

        case NETOPT_RX_END_IRQ:
            return sx127x_set_option(dev, SX127X_OPT_TELL_RX_END, *((const bool *)val));

        case NETOPT_TX_START_IRQ:
            return sx127x_set_option(dev, SX127X_OPT_TELL_TX_START, *((const bool *)val));

        case NETOPT_TX_END_IRQ:
            return sx127x_set_option(dev, SX127X_OPT_TELL_TX_END, *((const bool *)val));

        case NETOPT_TX_REFILL_IRQ:
            return sx127x_set_option(dev, SX127X_OPT_TELL_TX_REFILL, *((const bool *)val));

        case NETOPT_PRELOADING:
            return sx127x_set_option(dev, SX127X_OPT_PRELOADING, *((const bool *)val));
        default:
            break;
    }

    return res;
}

static int _set_state(sx127x_t *dev, netopt_state_t state)
{
    /* Some operations can not take place in Sleep mode, so wake up the chip */
    if ((state != NETOPT_STATE_SLEEP) && (state !=NETOPT_STATE_RESET))
    {
        if (sx127x_get_op_mode(dev) == SX127X_RF_OPMODE_SLEEP) {
            sx127x_set_standby(dev);
            hw_busy_wait(SX127X_RADIO_WAKEUP_TIME); /* wait for chip wake up */
        }
    }

    switch (state) {
        case NETOPT_STATE_SLEEP:
            sx127x_set_sleep(dev);
            break;

        case NETOPT_STATE_STANDBY:
            sx127x_set_standby(dev);
            break;

        case NETOPT_STATE_IDLE:
            /* set permanent listening */
            sx127x_set_rx_timeout(dev, 0);
            sx127x_set_rx(dev);
            break;

        case NETOPT_STATE_RX:
            sx127x_set_rx(dev);
            break;

        case NETOPT_STATE_TX:
            sx127x_set_tx(dev);
            break;

        case NETOPT_STATE_RESET:
            sx127x_reset(dev);
            break;

        default:
            return -ENOTSUP;
    }
    return sizeof(netopt_state_t);
}

static int _get_state(sx127x_t *dev, void *val)
{
    uint8_t op_mode;
    op_mode = sx127x_get_op_mode(dev);
    netopt_state_t state = NETOPT_STATE_OFF;
    switch(op_mode) {
        case SX127X_RF_OPMODE_SLEEP:
            state = NETOPT_STATE_SLEEP;
            break;

        case SX127X_RF_OPMODE_STANDBY:
            state = NETOPT_STATE_STANDBY;
            break;

        case SX127X_RF_OPMODE_TRANSMITTER:
            state = NETOPT_STATE_TX;
            break;

        case SX127X_RF_OPMODE_RECEIVER:
        case SX127X_RF_LORA_OPMODE_RECEIVER_SINGLE:
            state = NETOPT_STATE_IDLE;
            break;

        default:
            break;
    }
    memcpy(val, &state, sizeof(netopt_state_t));
    return sizeof(netopt_state_t);
}

static void fill_in_fifo(sx127x_t *dev)
{
    uint16_t remaining_bytes = dev->packet.length - dev->packet.pos;
    uint8_t space_left = SX127X_FIFO_MAX_SIZE - dev->packet.fifothresh;

    if (remaining_bytes == 0) // means that we need to ask the upper layer to refill the fifo
    {
        netdev_t *netdev = (netdev_t*) &dev->netdev;
        netdev->event_callback(netdev, NETDEV_EVENT_TX_REFILL_NEEDED);

        //update remaining_bytes
        remaining_bytes = dev->packet.length - dev->packet.pos;
        if (remaining_bytes == 0) // no new data, end of transmission
        {
            hw_gpio_set_edge_interrupt(dev->params.dio0_pin, GPIO_RISING_EDGE);
            hw_gpio_enable_interrupt(dev->params.dio0_pin);
            return;
        }
    }

    if (remaining_bytes > space_left)
    {
        sx127x_reg_write(dev, SX127X_REG_FIFOTHRESH, 0x80 | (SX127X_FIFO_MID_SIZE - 1));
        dev->packet.fifothresh = SX127X_FIFO_MID_SIZE;
        sx127x_write_fifo(dev, &dev->packet.buf[dev->packet.pos], space_left);

        dev->packet.pos += space_left;

        // clear the interrupt
        hw_gpio_set_edge_interrupt(dev->params.dio1_pin, GPIO_FALLING_EDGE);
        hw_gpio_enable_interrupt(dev->params.dio1_pin);
    }
    else
    {
        sx127x_write_fifo(dev, &dev->packet.buf[dev->packet.pos], remaining_bytes);
        dev->packet.pos += remaining_bytes;

        if (dev->options & SX127X_OPT_TELL_TX_REFILL)
        {
            hw_gpio_set_edge_interrupt(dev->params.dio1_pin, GPIO_FALLING_EDGE);
            hw_gpio_enable_interrupt(dev->params.dio1_pin);
        }
        else
        {
            hw_gpio_set_edge_interrupt(dev->params.dio0_pin, GPIO_RISING_EDGE);
            hw_gpio_enable_interrupt(dev->params.dio0_pin);
        }
    }
}

static void restart_rx(sx127x_t *dev)
{
    // Restart the reception until upper layer decides to stop it
    dev->packet.fifothresh = 0;
    dev->packet.length = 0;
    dev->packet.pos = 0;

    sx127x_reg_write(dev, SX127X_REG_FIFOTHRESH, 0x83);
    sx127x_reg_write(dev, SX127X_REG_DIOMAPPING1, 0x0C);
    sx127x_reg_write(dev, SX127X_REG_PAYLOADLENGTH, 0x00);

    // Trigger a manual restart of the Receiver chain (no frequency change)
    sx127x_reg_write(dev, SX127X_REG_RXCONFIG, 0x4E);
    sx127x_flush_fifo(dev);

    //DPRINT("Before enabling interrupt: FLAGS1 %x FLAGS2 %x\n", hw_radio_read_reg(REG_IRQFLAGS1), hw_radio_read_reg(REG_IRQFLAGS2));
    hw_gpio_set_edge_interrupt(dev->params.dio1_pin, GPIO_RISING_EDGE);
    hw_gpio_enable_interrupt(dev->params.dio1_pin);
}


void _on_dio0_irq(void *arg)
{
    sx127x_t *dev = (sx127x_t *) arg;
    netdev_t *netdev = (netdev_t*) &dev->netdev;

    switch (dev->settings.state) {
        case SX127X_RF_RX_RUNNING:
            sx127x_read_fifo(dev, dev->packet.buf, dev->packet.length);
            netdev->event_callback(netdev, NETDEV_EVENT_RX_COMPLETE);
            break;
        case SX127X_RF_TX_RUNNING:
            timer_cancel_event(&dev->_internal.tx_timeout_timer);
            switch (dev->settings.modem) {
                case SX127X_MODEM_LORA:
                    /* Clear IRQ */
                    sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGS,
                                     SX127X_RF_LORA_IRQFLAGS_TXDONE);
                    break;
                /* Intentional fall-through */
                case SX127X_MODEM_FSK:
                default:
                    hw_gpio_disable_interrupt(dev->params.dio0_pin);
                    sx127x_set_standby(dev);
                    netdev->event_callback(netdev, NETDEV_EVENT_TX_COMPLETE);
                    break;
            }
            break;
        case SX127X_RF_IDLE:
            DEBUG("[sx127x] netdev: sx127x_on_dio0: IDLE state\n");
            break;
        default:
            DEBUG("[sx127x] netdev: sx127x_on_dio0: unknown state [%d]\n",
                  dev->settings.state);
            break;
    }
}

void _on_dio1_irq(void *arg)
{
    /* Get interrupt context */
    sx127x_t *dev = (sx127x_t *) arg;
    netdev_t *netdev = (netdev_t*) &dev->netdev;

    switch (dev->settings.state) {
        case SX127X_RF_RX_RUNNING:
            switch (dev->settings.modem) {
                case SX127X_MODEM_FSK:
                    hw_gpio_disable_interrupt(dev->params.dio1_pin);
                    timer_cancel_event(&dev->_internal.rx_timeout_timer);

                    if (dev->packet.length == 0 && dev->packet.pos == 0)
                    {
                        // For RX, the threshold is set to 4, so if the DIO1 interrupt occurs, it means that can read at least 4 bytes
                        uint8_t rx_bytes = 0;
                         while(!(sx127x_is_fifo_empty(dev)) && rx_bytes < 4)
                        {
                            dev->packet.buf[rx_bytes++] = sx127x_reg_read(dev, SX127X_REG_FIFO);
                        }
                        DEBUG("packet buf %i", rx_bytes);
                        DEBUG_DATA(dev->packet.buf, rx_bytes);

                        assert(rx_bytes == 4);

                        //In unlimited length packet format, let the upper layer determine the length
                        dev->packet.length = rx_bytes;
                        netdev->event_callback(netdev, NETDEV_EVENT_RX_STARTED);

                        DEBUG("RX Packet Length: %i ", dev->packet.length);

                        dev->packet.pos = rx_bytes;
                    }

                    if (dev->packet.fifothresh)
                    {
                        sx127x_read_fifo(dev, &dev->packet.buf[dev->packet.pos], dev->packet.fifothresh);
                        dev->packet.pos += dev->packet.fifothresh;
                    }

                    while(!(sx127x_is_fifo_empty(dev)) && (dev->packet.pos < dev->packet.length))
                         dev->packet.buf[dev->packet.pos++] = sx127x_reg_read(dev, SX127X_REG_FIFO);

                    uint16_t remaining_bytes = dev->packet.length - dev->packet.pos;
                    DEBUG("read %i bytes, %i remaining, FLAGS2 %x \n", dev->packet.pos,
                            remaining_bytes, sx127x_reg_read(dev, SX127X_REG_IRQFLAGS2));

                    if (remaining_bytes == 0) {
                        /*if (!(dev->settings.fsk.flags & SX127X_RX_CONTINUOUS_FLAG)) {
                            sx127x_set_state(dev, SX127X_RF_IDLE);
                        }*/
                        netdev->event_callback(netdev, NETDEV_EVENT_RX_COMPLETE);

                        // restart RX in unlimited length mode.
                        restart_rx(dev);
                        return;
                    }

                    //Trigger FifoLevel interrupt
                    if (remaining_bytes > SX127X_FIFO_MAX_SIZE)
                    {
                        sx127x_reg_write(dev, SX127X_REG_FIFOTHRESH, 0x80 | (SX127X_FIFO_MID_SIZE - 1));
                        dev->packet.fifothresh = SX127X_FIFO_MID_SIZE;
                    } else {
                        sx127x_reg_write(dev, SX127X_REG_FIFOTHRESH, 0x80 | (remaining_bytes - 1));
                        dev->packet.fifothresh = remaining_bytes;
                    }

                    hw_gpio_set_edge_interrupt(dev->params.dio1_pin, GPIO_RISING_EDGE);
                    hw_gpio_enable_interrupt(dev->params.dio1_pin);
                    break;
                case SX127X_MODEM_LORA:
                    timer_cancel_event(&dev->_internal.rx_timeout_timer);
                    /*  Clear Irq */
                    sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGS, SX127X_RF_LORA_IRQFLAGS_RXTIMEOUT);
                    sx127x_set_state(dev, SX127X_RF_IDLE);
                    netdev->event_callback(netdev, NETDEV_EVENT_RX_TIMEOUT);
                    break;
                default:
                    break;
            }
            break;
        case SX127X_RF_TX_RUNNING:
            switch (dev->settings.modem) {
                case SX127X_MODEM_FSK: {
                    uint8_t flags;

                    //hw_gpio_disable_interrupt(SX127x_DIO1_PIN);

                    flags = sx127x_reg_read(dev, SX127X_REG_IRQFLAGS2);
                    // detect underflow
                    if (flags & 0x08)
                    {
                        DEBUG("FlagsIRQ2: %x means that packet has been sent! ", flags);
                        assert(false);
                    }

                    fill_in_fifo(dev);
                } break;
                case SX127X_MODEM_LORA:
                    break;
                default:
                    break;
            }
            break;
        default:
            puts("[sx127x] netdev: sx127x_on_dio1: unknown state");
            break;
    }
}

void _on_dio2_irq(void *arg)
{
    /* Get interrupt context */
    sx127x_t *dev = (sx127x_t *) arg;
    netdev_t *netdev = (netdev_t*) dev;

    switch (dev->settings.state) {
        case SX127X_RF_RX_RUNNING:
            switch (dev->settings.modem) {
                case SX127X_MODEM_FSK:
                    /* todo */
                    break;
                case SX127X_MODEM_LORA:
                    if (dev->settings.lora.flags & SX127X_CHANNEL_HOPPING_FLAG) {
                        /* Clear IRQ */
                        sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGS,
                                         SX127X_RF_LORA_IRQFLAGS_FHSSCHANGEDCHANNEL);

                        dev->_internal.last_channel = (sx127x_reg_read(dev, SX127X_REG_LR_HOPCHANNEL) &
                                                       SX127X_RF_LORA_HOPCHANNEL_CHANNEL_MASK);
                        netdev->event_callback(netdev, NETDEV_EVENT_FHSS_CHANGE_CHANNEL);
                    }

                    break;
                default:
                    break;
            }
            break;
        case SX127X_RF_TX_RUNNING:
            switch (dev->settings.modem) {
                case SX127X_MODEM_FSK:
                    break;
                case SX127X_MODEM_LORA:
                    if (dev->settings.lora.flags & SX127X_CHANNEL_HOPPING_FLAG) {
                        /* Clear IRQ */
                        sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGS,
                                         SX127X_RF_LORA_IRQFLAGS_FHSSCHANGEDCHANNEL);

                        dev->_internal.last_channel = (sx127x_reg_read(dev, SX127X_REG_LR_HOPCHANNEL) &
                                                       SX127X_RF_LORA_HOPCHANNEL_CHANNEL_MASK);
                        netdev->event_callback(netdev, NETDEV_EVENT_FHSS_CHANGE_CHANNEL);
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            puts("[sx127x] netdev: sx127x_on_dio2: unknown state");
            break;
    }
}

void _on_dio3_irq(void *arg)
{
    /* Get interrupt context */
    sx127x_t *dev = (sx127x_t *) arg;
    netdev_t *netdev = (netdev_t *) dev;

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            break;
        case SX127X_MODEM_LORA:
            /* Clear IRQ */
            sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGS,
                             SX127X_RF_LORA_IRQFLAGS_CADDETECTED |
                             SX127X_RF_LORA_IRQFLAGS_CADDONE);

            /* Send event message */
            dev->_internal.is_last_cad_success = ((sx127x_reg_read(dev, SX127X_REG_LR_IRQFLAGS) &
                                                   SX127X_RF_LORA_IRQFLAGS_CADDETECTED) ==
                                                  SX127X_RF_LORA_IRQFLAGS_CADDETECTED);
            netdev->event_callback(netdev, NETDEV_EVENT_CAD_DONE);
            break;
        default:
            puts("[sx127x] netdev: sx127x_on_dio3: unknown modem");
            break;
    }
}

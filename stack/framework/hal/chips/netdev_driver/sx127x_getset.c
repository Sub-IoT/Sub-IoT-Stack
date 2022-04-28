/*
 * Copyright (C) 2016 Unwired Devices <info@unwds.com>
 *               2017 Inria Chile
 *               2017 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_sx127x
 * @{
 * @file
 * @brief       Implementation of get and set functions for SX127X
 *
 * @author      Eugene P. <ep@unwds.com>
 * @author      José Ignacio Alamos <jose.alamos@inria.cl>
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 * @}
 */

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

#include "net/lora.h"

#include "sx127x.h"
#include "sx127x_registers.h"
#include "sx127x_internal.h"
#include "platform.h"
#include "debug.h"
#include "hal_defs.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(HAL_RADIO_LOG_ENABLED)
#include "log.h"
    #define DEBUG(...) log_print_string(__VA_ARGS__)
#else
    #define DEBUG(...)
#endif

typedef enum {
  OPMODE_SLEEP = 0,
  OPMODE_STANDBY = 1,
  OPMODE_FSTX = 2,
  OPMODE_TX = 3,
  OPMODE_FSRX = 4,
  OPMODE_RX = 5,
} opmode_t;

uint8_t sx127x_get_state(const sx127x_t *dev)
{
    return dev->settings.state;
}

void sx127x_set_state(sx127x_t *dev, uint8_t state)
{
#if ENABLE_DEBUG
    switch (state) {
    case SX127X_RF_IDLE:
        DEBUG("[sx127x] Change state: IDLE\n");
        break;
    case SX127X_RF_RX_RUNNING:
        DEBUG("[sx127x] Change state: RX\n");
        break;
    case SX127X_RF_TX_RUNNING:
        DEBUG("[sx127x] Change state: TX\n");
        break;
    default:
        DEBUG("[sx127x] Change state: UNKNOWN\n");
        break;
    }
#endif

    dev->settings.state = state;
}

void sx127x_set_modem(sx127x_t *dev, uint8_t modem)
{
    DEBUG("[sx127x] set modem: %d\n", modem);

    //FIXME mapping between SX127X_MODEM_XXX and NETDEV_TYPE_XXX

    if ((sx127x_reg_read(dev, SX127X_REG_OPMODE) & SX127X_RF_LORA_OPMODE_LONGRANGEMODE_ON) != 0) {
        dev->settings.modem = SX127X_MODEM_LORA;
    }
    else {
        dev->settings.modem = SX127X_MODEM_FSK;
    }

    /* Skip if unchanged to avoid resetting the transceiver below (may end up
     * in crashes) */
    if (dev->settings.modem == modem) {
        DEBUG("[sx127x] already using modem: %d\n", modem);
        return;
    }

    dev->settings.modem = modem;

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            sx127x_set_op_mode(dev, SX127X_RF_OPMODE_SLEEP);
            sx127x_reg_write(dev, SX127X_REG_OPMODE,
                                         (sx127x_reg_read(dev, SX127X_REG_OPMODE) &
                                          SX127X_RF_LORA_OPMODE_LONGRANGEMODE_MASK) |
                                          SX127X_RF_LORA_OPMODE_LONGRANGEMODE_OFF);
            /* Todo */
            break;
        case SX127X_MODEM_LORA:
            sx127x_set_op_mode(dev, SX127X_RF_OPMODE_SLEEP);
            sx127x_reg_write(dev, SX127X_REG_OPMODE,
                             (sx127x_reg_read(dev, SX127X_REG_OPMODE) &
                              SX127X_RF_LORA_OPMODE_LONGRANGEMODE_MASK) |
                             SX127X_RF_LORA_OPMODE_LONGRANGEMODE_ON);

            sx127x_reg_write(dev, SX127X_REG_DIOMAPPING1, 0x00);
            sx127x_reg_write(dev, SX127X_REG_DIOMAPPING2, 0x00);
            break;
        default:
            break;
    }
}

uint8_t sx127x_get_syncword_length(const sx127x_t *dev)
{
    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK: {
            uint8_t sync_size = sx127x_reg_read(dev, SX127X_REG_SYNCCONFIG) &
                                ~SX127X_RF_SYNCCONFIG_SYNCSIZE_MASK;
            sync_size++;

            DEBUG("Synx word size <%d>", sync_size);
            return sync_size;
        }
        case SX127X_MODEM_LORA:
            return dev->settings.lora.preamble_len;
    }

    return 0;
}

void sx127x_set_syncword_length(sx127x_t *dev, uint8_t len)
{
    DEBUG("[sx127x] Set sync word length: %d\n", len);

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            assert(len >= 1);
            sx127x_reg_write(dev, SX127X_REG_SYNCCONFIG,
                             (sx127x_reg_read(dev, SX127X_REG_SYNCCONFIG) &
                             SX127X_RF_SYNCCONFIG_SYNCSIZE_MASK) | (len - 1));
            break;
        case SX127X_MODEM_LORA:
            // sync word is necessarily 1 byte
            break;
        default:
            break;
    }
}

uint8_t sx127x_get_syncword(const sx127x_t *dev, uint8_t *syncword, uint8_t sync_size)
{
    uint8_t size;

    assert(sync_size >= 1);

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            size = (sx127x_reg_read(dev, SX127X_REG_SYNCCONFIG) &
                    ~SX127X_RF_SYNCCONFIG_SYNCSIZE_MASK) + 1;
            assert(size <= sync_size);
            sx127x_reg_read_burst(dev, SX127X_REG_SYNCVALUE1, syncword, size);
            return size;
        case SX127X_MODEM_LORA:
            *syncword = sx127x_reg_read(dev, SX127X_REG_LR_SYNCWORD);
            return sizeof(uint8_t);
    }

    return 0;
}

void sx127x_set_syncword(sx127x_t *dev, uint8_t *syncword, uint8_t sync_size)
{
    assert(sync_size >= 1);
    DEBUG("[sx127x] set syncword: len <%d> :", sync_size);

    for( uint32_t i = 0 ; i < sync_size ; i++ )
    {
        DEBUG(" %02X", syncword[i]);
    }

    // Reverse the order since the least significant byte is stored at the lowest address (CPU is little endian)
    // but we write into the SYNCVALUE registers MSB first
    uint8_t reverse_syncword[8];
    for (int i = 0; i < sync_size; i++)
    {
        reverse_syncword[sync_size -1 - i] = syncword[i];
    }

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            sx127x_reg_write(dev, SX127X_REG_SYNCCONFIG,
                             (sx127x_reg_read(dev , SX127X_REG_SYNCCONFIG) &
                              SX127X_RF_SYNCCONFIG_SYNCSIZE_MASK) | (sync_size - 1));
            sx127x_reg_write_burst(dev, SX127X_REG_SYNCVALUE1, reverse_syncword, sync_size);
            break;
        case SX127X_MODEM_LORA:
            sx127x_reg_write(dev, SX127X_REG_LR_SYNCWORD, *syncword);
            break;
    }
}

uint32_t sx127x_get_channel(const sx127x_t *dev)
{
    return (((uint32_t)sx127x_reg_read(dev, SX127X_REG_FRFMSB) << 16) |
            (sx127x_reg_read(dev, SX127X_REG_FRFMID) << 8) |
            (sx127x_reg_read(dev, SX127X_REG_FRFLSB))) * SX127X_FREQUENCY_RESOLUTION;
}

void sx127x_set_channel(sx127x_t *dev, uint32_t channel)
{
    DEBUG("[sx127x] Set channel: %lu\n", channel);

    /* Save current operating mode */
    dev->settings.channel = channel;

    channel = (uint32_t)((double) channel / (double) SX127X_FREQUENCY_RESOLUTION);

    /* Write frequency settings into chip */
    sx127x_reg_write(dev, SX127X_REG_FRFMSB, (uint8_t)((channel >> 16) & 0xFF));
    sx127x_reg_write(dev, SX127X_REG_FRFMID, (uint8_t)((channel >> 8) & 0xFF));
    sx127x_reg_write(dev, SX127X_REG_FRFLSB, (uint8_t)(channel & 0xFF));
}

uint32_t sx127x_get_time_on_air(const sx127x_t *dev, uint8_t pkt_len)
{
    uint32_t air_time = 0;

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            /* todo */
            break;
        case SX127X_MODEM_LORA:
        {
            double bw = 0.0;

            /* Note: When using LoRa modem only bandwidths 125, 250 and 500 kHz are supported. */
            switch (dev->settings.lora.bandwidth) {
                case LORA_BW_125_KHZ:
                    bw = 125e3;
                    break;
                case LORA_BW_250_KHZ:
                    bw = 250e3;
                    break;
                case LORA_BW_500_KHZ:
                    bw = 500e3;
                    break;
                default:
                    DEBUG("Invalid bandwith: %d\n", dev->settings.lora.bandwidth);
                    break;
            }

            /* Symbol rate : time for one symbol [secs] */
            double rs = bw / (1 << dev->settings.lora.datarate);
            double ts = 1 / rs;

            /* time of preamble */
            double t_preamble = (dev->settings.lora.preamble_len + 4.25) * ts;

            /* Symbol length of payload and time */
            double tmp =
                ceil(
                    (8 * pkt_len - 4 * dev->settings.lora.datarate + 28
                     + 16 * (dev->settings.lora.flags & SX127X_ENABLE_CRC_FLAG)
                     - (!(dev->settings.lora.flags & SX127X_ENABLE_FIXED_HEADER_LENGTH_FLAG) ? 20 : 0))
                    / (double) (4 * dev->settings.lora.datarate
                                - (((dev->settings.lora.flags & SX127X_LOW_DATARATE_OPTIMIZE_FLAG)
                                    > 0) ? 2 : 0)))
                * (dev->settings.lora.coderate + 4);
            double n_payload = 8 + ((tmp > 0) ? tmp : 0);
            double t_payload = n_payload * ts;

            /* Time on air */
            double t_on_air = t_preamble + t_payload;

            /* return milli seconds */
            air_time = floor(t_on_air * 1e3 + 0.999);
        }
        break;
    }

    return air_time;
}

void sx127x_set_sleep(sx127x_t *dev)
{
    DEBUG("[sx127x] Set sleep\n");

    /* Disable running timers */
    timer_cancel_event(&dev->_internal.tx_timeout_timer);
    timer_cancel_event(&dev->_internal.rx_timeout_timer);

    dev->options = 0; // clear the options
     /* Disable the interrupts */
    hw_gpio_disable_interrupt(dev->params.dio0_pin);
    hw_gpio_disable_interrupt(dev->params.dio1_pin);

    /* Put chip into sleep */
    sx127x_set_op_mode(dev, SX127X_RF_OPMODE_SLEEP);
    sx127x_set_state(dev,  SX127X_RF_IDLE);
}

void sx127x_set_standby(sx127x_t *dev)
{
    DEBUG("[sx127x] Set standby\n");

    /* Disable running timers */
    timer_cancel_event(&dev->_internal.tx_timeout_timer);
    timer_cancel_event(&dev->_internal.rx_timeout_timer);

    dev->options = 0; // clear the options
     /* Disable the interrupts */
    hw_gpio_disable_interrupt(dev->params.dio0_pin);
    hw_gpio_disable_interrupt(dev->params.dio1_pin);

    sx127x_set_op_mode(dev, SX127X_RF_OPMODE_STANDBY);
    while(!(sx127x_reg_read(dev, SX127X_REG_IRQFLAGS1) & SX127X_RF_IRQFLAGS1_MODEREADY)); // wait for Standby mode ready

    sx127x_set_state(dev,  SX127X_RF_IDLE);
}

void sx127x_set_packet_handler_enabled(sx127x_t *dev, bool enable) {
    sx127x_reg_write(dev, SX127X_REG_PREAMBLEDETECT,
                     (sx127x_reg_read(dev, SX127X_REG_PREAMBLEDETECT) &
                      SX127X_RF_PREAMBLEDETECT_DETECTOR_MASK) |
                     (enable << 7));
    sx127x_reg_write(dev, SX127X_REG_SYNCCONFIG,
                     (sx127x_reg_read(dev, SX127X_REG_SYNCCONFIG) &
                      SX127X_RF_SYNCCONFIG_SYNC_MASK) |
                     (enable << 4));
}

static void restart_rx_chain(sx127x_t *dev)
{
    // TODO restarting by triggering RF_RXCONFIG_RESTARTRXWITHPLLLOCK seems not to work
    // for some reason, when already in RX and after a freq change.
    // The chip is unable to receive on the new freq
    // For now the workaround is to go back to standby mode, to be optimized later
    sx127x_set_op_mode(dev, SX127X_RF_OPMODE_STANDBY);
    while(!(sx127x_reg_read(dev, SX127X_REG_IRQFLAGS1) & SX127X_RF_IRQFLAGS1_MODEREADY)); // wait for Standby mode ready

    // TODO for now we assume we need a restart with PLL lock.
    // this can be optimized for case where there is no freq change
    // write_reg(REG_RXCONFIG, read_reg(REG_RXCONFIG) | RF_RXCONFIG_RESTARTRXWITHPLLLOCK);
    DEBUG("restart RX chain with PLL lock");
}

void sx127x_set_rx(sx127x_t *dev)
{
    DEBUG("[sx127x] Set RX\n");

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            sx127x_flush_fifo(dev);

            dev->packet.pos = 0;
            dev->packet.fifothresh = 0;

            /* Setup interrupts */
            sx127x_reg_write(dev, SX127X_REG_DIOMAPPING1, 0x0C); // DIO2 interrupt on sync detect and DIO0 interrupt on PayloadReady
            sx127x_reg_write(dev, SX127X_REG_FIFOTHRESH, 0x83);

            uint16_t expected_payload_len = sx127x_get_max_payload_len(dev);
            sx127x_set_packet_handler_enabled(dev, true);

            // In case of unlimited length packet format, interrupt is generated once the header is received
            if (expected_payload_len == 0)
            {
                dev->packet.length = 0;
                hw_gpio_set_edge_interrupt(dev->params.dio1_pin, GPIO_RISING_EDGE);
                hw_gpio_enable_interrupt(dev->params.dio1_pin);
                sx127x_set_packet_handler_enabled(dev, true);
            }
            else
            {
                // Don't overrride the payload length which must have been set by the upper layer
                dev->packet.length = expected_payload_len;
                hw_gpio_set_edge_interrupt(dev->params.dio0_pin, GPIO_RISING_EDGE);
                hw_gpio_enable_interrupt(dev->params.dio0_pin); // enable the PayloadReady interrupt
            }

            if(dev->settings.state  == SX127X_RF_RX_RUNNING)
                restart_rx_chain(dev); // restart when already in RX so PLL can lock when there is a freq change

            break;
        case SX127X_MODEM_LORA:
        {
            sx127x_reg_write(dev, SX127X_REG_LR_INVERTIQ,
                             ((sx127x_reg_read(dev, SX127X_REG_LR_INVERTIQ) &
                               SX127X_RF_LORA_INVERTIQ_TX_MASK &
                               SX127X_RF_LORA_INVERTIQ_RX_MASK) |
                              ((dev->settings.lora.flags & SX127X_IQ_INVERTED_FLAG) ? SX127X_RF_LORA_INVERTIQ_RX_ON :SX127X_RF_LORA_INVERTIQ_RX_OFF) |
                              SX127X_RF_LORA_INVERTIQ_TX_OFF));
            sx127x_reg_write(dev, SX127X_REG_LR_INVERTIQ2,
                             ((dev->settings.lora.flags & SX127X_IQ_INVERTED_FLAG) ? SX127X_RF_LORA_INVERTIQ2_ON : SX127X_RF_LORA_INVERTIQ2_OFF));

#if defined(MODULE_SX1276)
            /* ERRATA 2.3 - Receiver Spurious Reception of a LoRa Signal */
            if (dev->settings.lora.bandwidth < 9) {
                sx127x_reg_write(dev, SX127X_REG_LR_DETECTOPTIMIZE,
                                 sx127x_reg_read(dev, SX127X_REG_LR_DETECTOPTIMIZE) & 0x7F);
                sx127x_reg_write(dev, SX127X_REG_LR_TEST30, 0x00);
                switch (dev->settings.lora.bandwidth) {
                    case LORA_BW_125_KHZ: /* 125 kHz */
                        sx127x_reg_write(dev, SX127X_REG_LR_TEST2F, 0x40);
                        break;
                    case LORA_BW_250_KHZ: /* 250 kHz */
                        sx127x_reg_write(dev, SX127X_REG_LR_TEST2F, 0x40);
                        break;

                    default:
                        break;
                }
            }
            else {
                sx127x_reg_write(dev, SX127X_REG_LR_DETECTOPTIMIZE,
                                 sx127x_reg_read(dev, SX127X_REG_LR_DETECTOPTIMIZE) | 0x80);
            }
#endif

            /* Setup interrupts */
            if (dev->settings.lora.flags & SX127X_CHANNEL_HOPPING_FLAG) {
                sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGSMASK,
                                 /* SX127X_RF_LORA_IRQFLAGS_RXTIMEOUT |
                                    SX127X_RF_LORA_IRQFLAGS_RXDONE |
                                    SX127X_RF_LORA_IRQFLAGS_PAYLOADCRCERROR | */
                                 SX127X_RF_LORA_IRQFLAGS_VALIDHEADER |
                                 SX127X_RF_LORA_IRQFLAGS_TXDONE |
                                 SX127X_RF_LORA_IRQFLAGS_CADDONE |
                                 /* SX127X_RF_LORA_IRQFLAGS_FHSSCHANGEDCHANNEL | */
                                 SX127X_RF_LORA_IRQFLAGS_CADDETECTED);

                /* DIO0=RxDone, DIO2=FhssChangeChannel */
                sx127x_reg_write(dev, SX127X_REG_DIOMAPPING1,
                                 (sx127x_reg_read(dev, SX127X_REG_DIOMAPPING1) &
                                  SX127X_RF_LORA_DIOMAPPING1_DIO0_MASK &
                                  SX127X_RF_LORA_DIOMAPPING1_DIO2_MASK) |
                                 SX127X_RF_LORA_DIOMAPPING1_DIO0_00 |
                                 SX127X_RF_LORA_DIOMAPPING1_DIO2_00);
            }
            else {
                sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGSMASK,
                                 /* SX127X_RF_LORA_IRQFLAGS_RXTIMEOUT |
                                    SX127X_RF_LORA_IRQFLAGS_RXDONE |
                                    SX127X_RF_LORA_IRQFLAGS_PAYLOADCRCERROR | */
                                 SX127X_RF_LORA_IRQFLAGS_VALIDHEADER |
                                 SX127X_RF_LORA_IRQFLAGS_TXDONE |
                                 SX127X_RF_LORA_IRQFLAGS_CADDONE |
                                 SX127X_RF_LORA_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                 SX127X_RF_LORA_IRQFLAGS_CADDETECTED);

                /* DIO0=RxDone */
                sx127x_reg_write(dev, SX127X_REG_DIOMAPPING1,
                                 (sx127x_reg_read(dev, SX127X_REG_DIOMAPPING1) &
                                  SX127X_RF_LORA_DIOMAPPING1_DIO0_MASK) |
                                 SX127X_RF_LORA_DIOMAPPING1_DIO0_00);
            }

            sx127x_reg_write(dev, SX127X_REG_LR_FIFORXBASEADDR, 0);
            sx127x_reg_write(dev, SX127X_REG_LR_FIFOADDRPTR, 0);
        }
        break;
    }

    sx127x_set_state(dev, SX127X_RF_RX_RUNNING);

    if (dev->settings.modem == SX127X_MODEM_FSK)
    {
        if (dev->settings.fsk.rx_timeout != 0) {
            dev->_internal.rx_timeout_timer.next_event = dev->settings.fsk.rx_timeout; // TODO convert timeout in timer_tick
            timer_add_event(&dev->_internal.rx_timeout_timer);
        }

        sx127x_set_op_mode(dev, SX127X_RF_OPMODE_RECEIVER);
    }
    else
    {
        if (dev->settings.lora.rx_timeout != 0) {
            dev->_internal.rx_timeout_timer.next_event = dev->settings.lora.rx_timeout; // TODO convert timeout in timer_tick
            timer_add_event(&dev->_internal.rx_timeout_timer);
        }

        if (dev->settings.lora.flags & SX127X_RX_CONTINUOUS_FLAG) {
           sx127x_set_op_mode(dev, SX127X_RF_LORA_OPMODE_RECEIVER);
        }
        else {
            sx127x_set_op_mode(dev, SX127X_RF_LORA_OPMODE_RECEIVER_SINGLE);
        }
    }
}

void sx127x_set_tx(sx127x_t *dev)
{
     switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            sx127x_reg_write(dev, SX127X_REG_DIOMAPPING1, 0x00); // FIFO LEVEL ISR or Packet Sent ISR
            break;
        case SX127X_MODEM_LORA:
        {
            if (dev->settings.lora.flags & SX127X_CHANNEL_HOPPING_FLAG) {
                sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGSMASK,
                                 SX127X_RF_LORA_IRQFLAGS_RXTIMEOUT |
                                 SX127X_RF_LORA_IRQFLAGS_RXDONE |
                                 SX127X_RF_LORA_IRQFLAGS_PAYLOADCRCERROR |
                                 SX127X_RF_LORA_IRQFLAGS_VALIDHEADER |
                                 /* RFLR_IRQFLAGS_TXDONE | */
                                 SX127X_RF_LORA_IRQFLAGS_CADDONE |
                                 /* RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL | */
                                 SX127X_RF_LORA_IRQFLAGS_CADDETECTED);

                /* DIO0=TxDone, DIO2=FhssChangeChannel */
                sx127x_reg_write(dev, SX127X_REG_DIOMAPPING1,
                                 (sx127x_reg_read(dev, SX127X_REG_DIOMAPPING1 ) &
                                  SX127X_RF_LORA_DIOMAPPING1_DIO0_MASK &
                                  SX127X_RF_LORA_DIOMAPPING1_DIO2_MASK) |
                                 SX127X_RF_LORA_DIOMAPPING1_DIO0_01 |
                                 SX127X_RF_LORA_DIOMAPPING1_DIO2_00);
            }
            else
            {
                sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGSMASK,
                                 SX127X_RF_LORA_IRQFLAGS_RXTIMEOUT |
                                 SX127X_RF_LORA_IRQFLAGS_RXDONE |
                                 SX127X_RF_LORA_IRQFLAGS_PAYLOADCRCERROR |
                                 SX127X_RF_LORA_IRQFLAGS_VALIDHEADER |
                                 /* RFLR_IRQFLAGS_TXDONE | */
                                 SX127X_RF_LORA_IRQFLAGS_CADDONE |
                                 SX127X_RF_LORA_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                 SX127X_RF_LORA_IRQFLAGS_CADDETECTED);

                /* DIO0=TxDone */
                sx127x_reg_write(dev, SX127X_REG_DIOMAPPING1,
                                 (sx127x_reg_read(dev, SX127X_REG_DIOMAPPING1) &
                                  SX127X_RF_LORA_DIOMAPPING1_DIO0_MASK) |
                                  SX127X_RF_LORA_DIOMAPPING1_DIO0_01);
            }
        }
        break;
    }

    sx127x_set_state(dev, SX127X_RF_TX_RUNNING);
    if (dev->settings.modem == SX127X_MODEM_LORA)
    {
        if (dev->settings.lora.tx_timeout != 0) {
            dev->_internal.tx_timeout_timer.next_event = dev->settings.lora.tx_timeout; // TODO convert timeout in timer_tick
            timer_add_event(&dev->_internal.tx_timeout_timer);
        }
    }
    sx127x_set_op_mode(dev, SX127X_RF_OPMODE_TRANSMITTER );
}

uint16_t sx127x_get_max_payload_len(const sx127x_t *dev)
{
    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            return sx127x_reg_read(dev, SX127X_REG_PAYLOADLENGTH);

        case SX127X_MODEM_LORA:
            return sx127x_reg_read(dev, SX127X_REG_LR_PAYLOADMAXLENGTH);
    }

    /* should never be reached */
    return 0;
}

void sx127x_set_max_payload_len(sx127x_t *dev, uint16_t maxlen)
{
    DEBUG("[sx127x] Set max payload len: %d\n", maxlen);

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            sx127x_reg_write(dev, SX127X_REG_PAYLOADLENGTH, maxlen);
            if (dev->settings.state == SX127X_RF_RX_RUNNING)
                dev->packet.length = maxlen;
            break;

        case SX127X_MODEM_LORA:
            sx127x_reg_write(dev, SX127X_REG_LR_PAYLOADMAXLENGTH, maxlen);
            break;
    }
}

#if defined(PLATFORM_SX127X_USE_MANUAL_RXTXSW_PIN) || defined(PLATFORM_USE_ABZ)
static void set_antenna_switch(const sx127x_t *dev, opmode_t opmode) {
  if(opmode == OPMODE_TX) {
#ifdef PLATFORM_SX127X_USE_MANUAL_RXTXSW_PIN
    hw_gpio_set(SX127x_MANUAL_RXTXSW_PIN);
#endif
#ifdef PLATFORM_USE_ABZ
    hw_gpio_clr(ABZ_ANT_SW_RX_PIN);
    if((sx127x_reg_read(dev, SX127X_REG_PACONFIG) & SX127X_RF_PACONFIG_PASELECT_PABOOST) == SX127X_RF_PACONFIG_PASELECT_PABOOST) {
      hw_gpio_clr(ABZ_ANT_SW_TX_PIN);
      hw_gpio_set(ABZ_ANT_SW_PA_BOOST_PIN);
    } else {
      hw_gpio_set(ABZ_ANT_SW_TX_PIN);
      hw_gpio_clr(ABZ_ANT_SW_PA_BOOST_PIN);
    }
#endif
  } else {
#ifdef PLATFORM_SX127X_USE_MANUAL_RXTXSW_PIN
    hw_gpio_clr(SX127x_MANUAL_RXTXSW_PIN);
#endif
#ifdef PLATFORM_USE_ABZ
    hw_gpio_set(ABZ_ANT_SW_RX_PIN);
    hw_gpio_clr(ABZ_ANT_SW_TX_PIN);
    hw_gpio_clr(ABZ_ANT_SW_PA_BOOST_PIN);
#endif
  }
}
#endif

uint8_t sx127x_get_op_mode(const sx127x_t *dev)
{
    return sx127x_reg_read(dev, SX127X_REG_OPMODE) & ~SX127X_RF_OPMODE_MASK;
}

void sx127x_set_op_mode(const sx127x_t *dev, uint8_t op_mode)
{
#if ENABLE_DEBUG
    switch(op_mode) {
    case SX127X_RF_OPMODE_SLEEP:
        DEBUG("[sx127x] Set op mode: SLEEP\n");
        break;
    case SX127X_RF_OPMODE_STANDBY:
        DEBUG("[sx127x] Set op mode: STANDBY\n");
        break;
    case SX127X_RF_OPMODE_RECEIVER_SINGLE:
        DEBUG("[sx127x] Set op mode: RECEIVER SINGLE\n");
        break;
    case SX127X_RF_OPMODE_RECEIVER:
        DEBUG("[sx127x] Set op mode: RECEIVER\n");
        break;
    case SX127X_RF_OPMODE_TRANSMITTER:
        DEBUG("[sx127x] Set op mode: TRANSMITTER\n");
        break;
    default:
        DEBUG("[sx127x] Set op mode: UNKNOWN (%d)\n", op_mode);
        break;
    }
#endif

#if defined(PLATFORM_SX127X_USE_MANUAL_RXTXSW_PIN) || defined(PLATFORM_USE_ABZ)
  set_antenna_switch(dev, op_mode);
#endif

    /* Replace previous mode value and setup new mode value */
    sx127x_reg_write(dev, SX127X_REG_OPMODE,
                     (sx127x_reg_read(dev, SX127X_REG_OPMODE) &
                      SX127X_RF_OPMODE_MASK) | op_mode);
}

uint32_t computeRxBw( uint8_t mantisse, uint8_t exponent )
{
    return SX127X_XTAL_FREQ / (mantisse * (1 << (exponent+2)));
}

void computeRxBwMantExp( uint32_t rxBwValue, uint8_t* mantisse, uint8_t* exponent )
{
    uint8_t tmpExp, tmpMant;
    uint32_t tmpRxBw;
    int32_t rxBwMin = 10e6;

    for( tmpExp = 0; tmpExp < 8; tmpExp++ ) {
        for( tmpMant = 16; tmpMant <= 24; tmpMant += 4 ) {
            tmpRxBw = computeRxBw(tmpMant, tmpExp);
            if( abs( tmpRxBw - rxBwValue ) < rxBwMin ) {
                rxBwMin = abs( tmpRxBw - rxBwValue );
                *mantisse = tmpMant;
                *exponent = tmpExp;
            }
        }
    }
}

uint32_t sx127x_get_bandwidth(const sx127x_t *dev)
{
    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK: {
            uint8_t reg_bw = sx127x_reg_read(dev, SX127X_REG_RXBW);
            uint8_t mantissa = reg_bw & ~SX127X_RF_RXBW_MANT_MASK;
            uint8_t exponent = reg_bw & ~SX127X_RF_RXBW_EXP_MASK;

            switch (mantissa) {
                case SX127X_RF_RXBW_MANT_16: mantissa = 16; break;
                case SX127X_RF_RXBW_MANT_20: mantissa = 20; break;
                case SX127X_RF_RXBW_MANT_24: mantissa = 24; break;
                default: mantissa = 0; break;
            }

            return computeRxBw(mantissa , exponent);
        }

        case SX127X_MODEM_LORA:
            return dev->settings.lora.bandwidth;
    }

    return 0;
}

static void _low_datarate_optimize(sx127x_t *dev)
{
    if ( ((dev->settings.lora.bandwidth == LORA_BW_125_KHZ) &&
          ((dev->settings.lora.datarate == LORA_SF11) ||
           (dev->settings.lora.datarate == LORA_SF12))) ||
         ((dev->settings.lora.bandwidth == LORA_BW_250_KHZ) &&
          (dev->settings.lora.datarate == LORA_SF12))) {
        dev->settings.lora.flags |= SX127X_LOW_DATARATE_OPTIMIZE_FLAG;
    } else {
        dev->settings.lora.flags &= ~SX127X_LOW_DATARATE_OPTIMIZE_FLAG;
    }

#if defined(MODULE_SX1272)
    sx127x_reg_write(dev, SX127X_REG_LR_MODEMCONFIG1,
                     (sx127x_reg_read(dev, SX127X_REG_LR_MODEMCONFIG1) &
                      SX127X_RF_LORA_MODEMCONFIG1_LOWDATARATEOPTIMIZE_MASK) |
                     ((dev->settings.lora.flags & SX127X_LOW_DATARATE_OPTIMIZE_FLAG)));
#else /* MODULE_SX1276 */
    sx127x_reg_write(dev, SX127X_REG_LR_MODEMCONFIG3,
                     (sx127x_reg_read(dev, SX127X_REG_LR_MODEMCONFIG3) &
                      SX127X_RF_LORA_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK) |
                     ((dev->settings.lora.flags & SX127X_LOW_DATARATE_OPTIMIZE_FLAG) << 3));
#endif
}

static void _update_bandwidth(const sx127x_t *dev)
{
    uint8_t config1_reg = sx127x_reg_read(dev, SX127X_REG_LR_MODEMCONFIG1);
#if defined(MODULE_SX1272)
    config1_reg &= SX1272_RF_LORA_MODEMCONFIG1_BW_MASK;
    switch (dev->settings.lora.bandwidth) {
    case LORA_BW_125_KHZ:
        config1_reg |=  SX1272_RF_LORA_MODEMCONFIG1_BW_125_KHZ;
        break;
    case LORA_BW_250_KHZ:
        config1_reg |=  SX1272_RF_LORA_MODEMCONFIG1_BW_250_KHZ;
        break;
    case LORA_BW_500_KHZ:
        config1_reg |=  SX1272_RF_LORA_MODEMCONFIG1_BW_500_KHZ;
        break;
    default:
        DEBUG("Unsupported bandwidth, %d", dev->settings.lora.bandwidth);
        break;
    }
#else /* MODULE_SX1276 */
    config1_reg &= SX1276_RF_LORA_MODEMCONFIG1_BW_MASK;
    switch (dev->settings.lora.bandwidth) {
    case LORA_BW_125_KHZ:
        config1_reg |= SX1276_RF_LORA_MODEMCONFIG1_BW_125_KHZ;
        break;
    case LORA_BW_250_KHZ:
        config1_reg |=  SX1276_RF_LORA_MODEMCONFIG1_BW_250_KHZ;
        break;
    case LORA_BW_500_KHZ:
        config1_reg |=  SX1276_RF_LORA_MODEMCONFIG1_BW_500_KHZ;
        break;
    default:
        DEBUG("[sx127x] Unsupported bandwidth, %d\n",
              dev->settings.lora.bandwidth);
        break;
    }
#endif
    sx127x_reg_write(dev, SX127X_REG_LR_MODEMCONFIG1, config1_reg);
}

void sx127x_set_bandwidth(sx127x_t *dev, uint32_t bandwidth)
{
    DEBUG("[sx127x] Set bandwidth: %d\n", bandwidth);

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK: {
            uint8_t mantisse = 0;
            uint8_t exponent = 0;
            uint8_t reg_bw;

            computeRxBwMantExp(bandwidth, &mantisse, &exponent);
            switch( mantisse ) {
                case 16:
                    reg_bw = SX127X_RF_RXBW_MANT_16 | exponent;
                    break;
                case 20:
                    reg_bw = SX127X_RF_RXBW_MANT_20 | exponent;
                    break;
                case 24:
                    reg_bw = SX127X_RF_RXBW_MANT_24 | exponent;
                    break;
                default:
                    DEBUG("mantisse:%d\n", mantisse);
                    assert(false);
                    break;
            }

            sx127x_reg_write(dev, SX127X_REG_RXBW, reg_bw);
        } break;

        case SX127X_MODEM_LORA:

            dev->settings.lora.bandwidth = bandwidth;

            _update_bandwidth((const sx127x_t *)dev);

            _low_datarate_optimize(dev);

            /* ERRATA sensitivity tweaks */
            if ((dev->settings.lora.bandwidth == LORA_BW_500_KHZ) &&
                (dev->settings.channel > SX127X_RF_MID_BAND_THRESH)) {
                /* ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth */
                sx127x_reg_write(dev, SX127X_REG_LR_TEST36, 0x02);
                sx127x_reg_write(dev, SX127X_REG_LR_TEST3A, 0x64);
            }
            else if (dev->settings.lora.bandwidth == LORA_BW_500_KHZ) {
                /* ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth */
                sx127x_reg_write(dev, SX127X_REG_LR_TEST36, 0x02);
                sx127x_reg_write(dev, SX127X_REG_LR_TEST3A, 0x7F);
            }
            else {
                /* ERRATA 2.1 - Sensitivity Optimization with another Bandwidth */
                sx127x_reg_write(dev, SX127X_REG_LR_TEST36, 0x03);
            }
            break;
    }
}

uint8_t sx127x_get_spreading_factor(const sx127x_t *dev)
{
    return dev->settings.lora.datarate;
}

void sx127x_set_spreading_factor(sx127x_t *dev, uint8_t datarate)
{
    DEBUG("[sx127x] Set spreading factor: %d\n", datarate);

    if (datarate == LORA_SF6 &&
        !(dev->settings.lora.flags & SX127X_ENABLE_FIXED_HEADER_LENGTH_FLAG)) {
        /* SF 6 is only valid when using explicit header mode */
        DEBUG("Spreading Factor 6 can only be used when explicit header "
              "mode is set, this mode is not supported by this driver."
              "Ignoring.\n");
        return;
    }

    dev->settings.lora.datarate = datarate;

    uint8_t config2_reg = sx127x_reg_read(dev, SX127X_REG_LR_MODEMCONFIG2);
    config2_reg &= SX127X_RF_LORA_MODEMCONFIG2_SF_MASK;
    config2_reg |= datarate << 4;
    sx127x_reg_write(dev, SX127X_REG_LR_MODEMCONFIG2, config2_reg);

    _low_datarate_optimize(dev);

    switch(dev->settings.lora.datarate) {
    case LORA_SF6:
        sx127x_reg_write(dev, SX127X_REG_LR_DETECTOPTIMIZE,
                         SX127X_RF_LORA_DETECTIONOPTIMIZE_SF6);
        sx127x_reg_write(dev, SX127X_REG_LR_DETECTIONTHRESHOLD,
                         SX127X_RF_LORA_DETECTIONTHRESH_SF6);
        break;
    default:
        sx127x_reg_write(dev, SX127X_REG_LR_DETECTOPTIMIZE,
                         SX127X_RF_LORA_DETECTIONOPTIMIZE_SF7_TO_SF12);
        sx127x_reg_write(dev, SX127X_REG_LR_DETECTIONTHRESHOLD,
                         SX127X_RF_LORA_DETECTIONTHRESH_SF7_TO_SF12);
        break;
    }
}

uint8_t sx127x_get_coding_rate(const sx127x_t *dev)
{
    return dev->settings.lora.coderate;
}

void sx127x_set_coding_rate(sx127x_t *dev, uint8_t coderate)
{
    DEBUG("[sx127x] Set coding rate: %d\n", coderate);

    dev->settings.lora.coderate = coderate;
    uint8_t config1_reg = sx127x_reg_read(dev, SX127X_REG_LR_MODEMCONFIG1);

#if defined(MODULE_SX1272)
    config1_reg &= SX1272_RF_LORA_MODEMCONFIG1_CODINGRATE_MASK;
    config1_reg |= coderate << 3;
#else /* MODULE_SX1276 */
    config1_reg &= SX1276_RF_LORA_MODEMCONFIG1_CODINGRATE_MASK;
    config1_reg |= coderate << 1;
#endif

    sx127x_reg_write(dev, SX127X_REG_LR_MODEMCONFIG1, config1_reg);
}

static inline void _set_flag(sx127x_t *dev, uint8_t flag, bool value)
{
    if (value) {
        dev->settings.lora.flags |= flag;
    }
    else {
        dev->settings.lora.flags &= ~flag;
    }
}

bool sx127x_get_rx_single(const sx127x_t *dev)
{
    return !(dev->settings.lora.flags & SX127X_RX_CONTINUOUS_FLAG);
}

void sx127x_set_rx_single(sx127x_t *dev, bool single)
{
    DEBUG("[sx127x] Set RX single: %d\n", single);
    _set_flag(dev, SX127X_RX_CONTINUOUS_FLAG, !single);
}

bool sx127x_get_crc(const sx127x_t *dev)
{
    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            return ((sx127x_reg_read(dev, SX127X_REG_PACKETCONFIG1) &
                    ~SX127X_RF_PACKETCONFIG1_CRC_MASK) >> 4);
        case SX127X_MODEM_LORA:
#if defined(MODULE_SX1272)
            return (sx127x_reg_read(dev, SX127X_REG_LR_MODEMCONFIG1) &
                    SX1272_RF_LORA_MODEMCONFIG1_RXPAYLOADCRC_MASK);
#else /* MODULE_SX1276 */
            return (sx127x_reg_read(dev, SX127X_REG_LR_MODEMCONFIG2) &
                    SX1276_RF_LORA_MODEMCONFIG2_RXPAYLOADCRC_MASK);
#endif
        default:
            break;
    }

    return false;
}

void sx127x_set_crc(sx127x_t *dev, bool crc)
{
    DEBUG("[sx127x] Set CRC: %d\n", crc);

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            sx127x_reg_write(dev, SX127X_REG_PACKETCONFIG1,
                             (sx127x_reg_read(dev, SX127X_REG_PACKETCONFIG1) &
                              SX127X_RF_PACKETCONFIG1_CRC_MASK) | (crc << 4));
            break;
        case SX127X_MODEM_LORA:
            _set_flag(dev, SX127X_ENABLE_CRC_FLAG, crc);

#if defined(MODULE_SX1272)
            uint8_t config2_reg = sx127x_reg_read(dev, SX127X_REG_LR_MODEMCONFIG1);
            config2_reg &= SX1272_RF_LORA_MODEMCONFIG1_RXPAYLOADCRC_MASK;
            config2_reg |= crc << 1;
            sx127x_reg_write(dev, SX127X_REG_LR_MODEMCONFIG1, config2_reg);
#else /* MODULE_SX1276 */
            uint8_t config2_reg = sx127x_reg_read(dev, SX127X_REG_LR_MODEMCONFIG2);
            config2_reg &= SX1276_RF_LORA_MODEMCONFIG2_RXPAYLOADCRC_MASK;
            config2_reg |= crc << 2;
            sx127x_reg_write(dev, SX127X_REG_LR_MODEMCONFIG2, config2_reg);
#endif
            break;
        default:
            break;
    }
}

uint8_t sx127x_get_hop_period(const sx127x_t *dev)
{
    return sx127x_reg_read(dev, SX127X_REG_LR_HOPPERIOD);
}

void sx127x_set_hop_period(sx127x_t *dev, uint8_t hop_period)
{
    DEBUG("[sx127x] Set Hop period: %d\n", hop_period);

    dev->settings.lora.freq_hop_period = hop_period;

    uint8_t tmp = sx127x_reg_read(dev, SX127X_REG_LR_PLLHOP);
    if (dev->settings.lora.flags & SX127X_CHANNEL_HOPPING_FLAG) {
        tmp |= SX127X_RF_LORA_PLLHOP_FASTHOP_ON;
        sx127x_reg_write(dev, SX127X_REG_LR_PLLHOP, tmp);
        sx127x_reg_write(dev, SX127X_REG_LR_HOPPERIOD, hop_period);
    }
}

bool  sx127x_get_fixed_header_len_mode(const sx127x_t *dev)
{
    return dev->settings.lora.flags & SX127X_ENABLE_FIXED_HEADER_LENGTH_FLAG;
}

void sx127x_set_fixed_header_len_mode(sx127x_t *dev, bool fixed_len)
{
    DEBUG("[sx127x] Set fixed header length: %d\n", fixed_len);

    _set_flag(dev, SX127X_ENABLE_FIXED_HEADER_LENGTH_FLAG, fixed_len);

    uint8_t config1_reg = sx127x_reg_read(dev, SX127X_REG_LR_MODEMCONFIG1);
#if defined(MODULE_SX1272)
    config1_reg &= SX1272_RF_LORA_MODEMCONFIG1_IMPLICITHEADER_MASK;
    config1_reg |= fixed_len << 2;
#else /* MODULE_SX1276 */
    config1_reg &= SX1276_RF_LORA_MODEMCONFIG1_IMPLICITHEADER_MASK;
    config1_reg |= fixed_len;
#endif
    sx127x_reg_write(dev, SX127X_REG_LR_MODEMCONFIG1, config1_reg);
}

uint16_t sx127x_get_payload_length(const sx127x_t *dev)
{
    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            return sx127x_reg_read(dev, SX127X_REG_PAYLOADLENGTH);
        case SX127X_MODEM_LORA:
            return sx127x_reg_read(dev, SX127X_REG_LR_PAYLOADLENGTH);
        default:
            break;
    }

    return 0;
}

void sx127x_set_payload_length(sx127x_t *dev, uint16_t len)
{
    DEBUG("[sx127x] Set payload len: %d\n", len);

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            sx127x_reg_write(dev, SX127X_REG_PAYLOADLENGTH, len);
            break;
        case SX127X_MODEM_LORA:
            sx127x_reg_write(dev, SX127X_REG_LR_PAYLOADLENGTH, len);
            break;
        default:
            break;
    }
}

static inline uint8_t sx127x_get_pa_select(const sx127x_t *dev)
{
    if (dev->params.paselect == SX127X_PA_BOOST) {
        return SX127X_RF_PACONFIG_PASELECT_PABOOST;
    }

    return SX127X_RF_PACONFIG_PASELECT_RFO;
}

uint8_t sx127x_get_tx_power(const sx127x_t *dev)
{
    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            return dev->settings.fsk.power;
        case SX127X_MODEM_LORA:
            return dev->settings.lora.power;
    }

    return 0;
}

void sx127x_set_tx_power(sx127x_t *dev, int8_t power)
{
    DEBUG("[sx127x] Set power: %d\n", power);

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            dev->settings.fsk.power = power;
#ifdef PLATFORM_SX127X_USE_PA_BOOST
            // Pout = 17-(15-outputpower)
            assert(power >= 2); // lower not supported when using PA_BOOST output
            assert(power <= 17); // chip supports until +20 dBm but then we need to enable PaDac. Max 17 for now.
            sx127x_reg_write(dev, SX127X_REG_PACONFIG, 0x80 | (power - 2));
#else
            // Pout = Pmax-(15-outputpower)
            assert(power <= 14); // Pmax = 13.8 dBm
            assert(power >= -2); // -1.2 dBm is minimum with this Pmax. We can modify Pmax later as well if we need to go lower.
            sx127x_reg_write(dev, SX127X_REG_PACONFIG, 0x70 | (uint8_t)(power - 13.8 + 15));
#endif
            return;
            break;
        case SX127X_MODEM_LORA:
            dev->settings.lora.power = power;
            break;
        default:
            break;
    }

    uint8_t pa_config = sx127x_reg_read(dev, SX127X_REG_PACONFIG);
#if defined(MODULE_SX1272)
    uint8_t pa_dac = sx127x_reg_read(dev, SX1272_REG_PADAC);
#else /* MODULE_SX1276 */
    uint8_t pa_dac = sx127x_reg_read(dev, SX1276_REG_PADAC);
#endif

    pa_config = ((pa_config & SX127X_RF_PACONFIG_PASELECT_MASK) |
                 sx127x_get_pa_select(dev));

#if defined(MODULE_SX1276)
    /* max power is 14dBm */
    pa_config = (pa_config & SX127X_RF_PACONFIG_MAX_POWER_MASK) | 0x70;
#endif

    sx127x_reg_write(dev, SX127X_REG_PARAMP, SX127X_RF_PARAMP_0050_US);

    if ((pa_config & SX127X_RF_PACONFIG_PASELECT_PABOOST)
        == SX127X_RF_PACONFIG_PASELECT_PABOOST) {
        if (power > 17) {
            pa_dac = ((pa_dac & SX127X_RF_PADAC_20DBM_MASK) |
                      SX127X_RF_PADAC_20DBM_ON);
        } else {
            pa_dac = ((pa_dac & SX127X_RF_PADAC_20DBM_MASK) |
                      SX127X_RF_PADAC_20DBM_OFF);
        }
        if ((pa_dac & SX127X_RF_PADAC_20DBM_ON) == SX127X_RF_PADAC_20DBM_ON) {
            if (power < 5) {
                power = 5;
            }
            if (power > 20) {
                power = 20;
            }

            pa_config = ((pa_config & SX127X_RF_PACONFIG_OUTPUTPOWER_MASK) |
                         (uint8_t)((uint16_t)(power - 5) & 0x0F));
        } else {
            if (power < 2) {
                power = 2;
            }
            if (power > 17) {
                power = 17;
            }

            pa_config = ((pa_config & SX127X_RF_PACONFIG_OUTPUTPOWER_MASK) |
                         (uint8_t)((uint16_t)(power - 2) & 0x0F));
        }
    } else {
        if (power < -1) {
            power = -1;
        }
        if (power > 14) {
            power = 14;
        }

        pa_config = ((pa_config & SX127X_RF_PACONFIG_OUTPUTPOWER_MASK) |
                     (uint8_t)((uint16_t)(power + 1) & 0x0F));
    }

    sx127x_reg_write(dev, SX127X_REG_PACONFIG, pa_config);
#if defined(MODULE_SX1272)
    sx127x_reg_write(dev, SX1272_REG_PADAC, pa_dac);
#else /* MODULE_SX1276 */
    sx127x_reg_write(dev, SX1276_REG_PADAC, pa_dac);
#endif
}

uint16_t sx127x_get_preamble_length(const sx127x_t *dev)
{
    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            return dev->settings.fsk.preamble_len;
        case SX127X_MODEM_LORA:
            return dev->settings.lora.preamble_len;
    }

    return 0;
}

void sx127x_set_preamble_length(sx127x_t *dev, uint16_t preamble)
{
    DEBUG("[sx127x] Set preamble length: %d\n", preamble);

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            dev->settings.fsk.preamble_len = preamble;
            sx127x_reg_write_u16(dev, SX127X_REG_PREAMBLEMSB, preamble);
            break;
        case SX127X_MODEM_LORA:
            dev->settings.lora.preamble_len = preamble;
            sx127x_reg_write_u16(dev, SX127X_REG_LR_PREAMBLEMSB, preamble);
            break;
        default:
            break;
    }
}

void sx127x_set_rx_timeout(sx127x_t *dev, uint32_t timeout)
{
    DEBUG("[sx127x] Set RX timeout: %lu\n", timeout);

    dev->settings.lora.rx_timeout = timeout;
}

void sx127x_set_tx_timeout(sx127x_t *dev, uint32_t timeout)
{
    DEBUG("[sx127x] Set TX timeout: %lu\n", timeout);

    dev->settings.lora.tx_timeout = timeout;
}

void sx127x_set_symbol_timeout(sx127x_t *dev, uint16_t timeout)
{
    DEBUG("[sx127x] Set symbol timeout: %d\n", timeout);

    dev->settings.lora.rx_timeout = timeout;

    uint8_t config2_reg = sx127x_reg_read(dev, SX127X_REG_LR_MODEMCONFIG2);
    config2_reg &= SX127X_RF_LORA_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK;
    config2_reg |= (timeout >> 8) & ~SX127X_RF_LORA_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK;
    sx127x_reg_write(dev, SX127X_REG_LR_MODEMCONFIG2, config2_reg);
    sx127x_reg_write(dev, SX127X_REG_LR_SYMBTIMEOUTLSB, timeout & 0xFF);
}

bool sx127x_get_iq_invert(const sx127x_t *dev)
{
    return dev->settings.lora.flags & SX127X_IQ_INVERTED_FLAG;
}

void sx127x_set_iq_invert(sx127x_t *dev, bool iq_invert)
{
    DEBUG("[sx127x] Set IQ invert: %d\n", iq_invert);

    _set_flag(dev, SX127X_IQ_INVERTED_FLAG, iq_invert);

    sx127x_reg_write(dev, SX127X_REG_LR_INVERTIQ,
                     (sx127x_reg_read(dev, SX127X_REG_LR_INVERTIQ) &
                      SX127X_RF_LORA_INVERTIQ_RX_MASK &
                      SX127X_RF_LORA_INVERTIQ_TX_MASK) |
                      SX127X_RF_LORA_INVERTIQ_RX_OFF |
                     (iq_invert ? SX127X_RF_LORA_INVERTIQ_TX_ON : SX127X_RF_LORA_INVERTIQ_TX_OFF));

    sx127x_reg_write(dev, SX127X_REG_LR_INVERTIQ2,
                     (iq_invert ? SX127X_RF_LORA_INVERTIQ2_ON : SX127X_RF_LORA_INVERTIQ2_OFF));
}

void sx127x_set_freq_hop(sx127x_t *dev, bool freq_hop_on)
{
    DEBUG("[sx127x] Set freq hop: %d\n", freq_hop_on);

     _set_flag(dev, SX127X_CHANNEL_HOPPING_FLAG, freq_hop_on);
}

uint32_t sx127x_get_bitrate(const sx127x_t *dev)
{
    uint16_t br = sx127x_reg_read_u16(dev, SX127X_REG_BITRATEMSB);

    if (br == 0)
        return 0;
    else {
        uint32_t bps = SX127X_XTAL_FREQ / br;
        //bit_period_us = bps / 1000;
        return bps;
    }
}

void sx127x_set_bitrate(sx127x_t *dev, uint32_t bps)
{
    uint16_t tmpBitrate = SX127X_XTAL_FREQ / bps;
    //bit_period_us = bps / 1000;

    DEBUG("[sx127x] Set bitrate register to %i", tmpBitrate);
    sx127x_reg_write_u16(dev, SX127X_REG_BITRATEMSB, tmpBitrate);
}

uint8_t sx127x_get_preamble_polarity(const sx127x_t *dev)
{
    uint8_t polarity = sx127x_reg_read(dev, SX127X_REG_SYNCCONFIG) &
                       ~SX127X_RF_SYNCCONFIG_PREAMBLEPOLARITY_MASK;

    if (polarity == SX127X_RF_SYNCCONFIG_PREAMBLEPOLARITY_MASK) {
        DEBUG("[sx127x] Polarity set to 0x55");
    }
    else {
        DEBUG("[sx127x] Polarity set to default 0xAA");
    }
    return (polarity >> 5);
}

void sx127x_set_preamble_polarity(sx127x_t *dev, uint8_t polarity)
{
    sx127x_reg_write(dev, SX127X_REG_SYNCCONFIG,
                     (sx127x_reg_read(dev, SX127X_REG_SYNCCONFIG) &
                      SX127X_RF_SYNCCONFIG_PREAMBLEPOLARITY_MASK) |
                     (polarity << 5));
}

void sx127x_set_rssi_threshold(sx127x_t *dev, uint8_t rssi_thr)
{
    /*
     * RSSI trigger level for the Rssi interrupt:
     *  - RssiThreshold / 2 [dBm]
     */
    sx127x_reg_write(dev, SX127X_REG_RSSITHRESH, rssi_thr * 2);
}

uint8_t sx127x_get_rssi_threshold(const sx127x_t *dev)
{
    uint8_t rssi_thr = sx127x_reg_read(dev, SX127X_REG_RSSITHRESH);

    return (rssi_thr / 2);
}

void sx127x_set_rssi_smoothing(sx127x_t *dev, uint8_t rssi_samples)
{
    sx127x_reg_write(dev, SX127X_REG_RSSICONFIG,
                         (sx127x_reg_read(dev, SX127X_REG_RSSICONFIG) &
                          SX127X_RF_RSSICONFIG_SMOOTHING_MASK) |
                         (rssi_samples/2 -1));
}

uint8_t sx127x_get_rssi_smoothing(const sx127x_t *dev)
{
    uint8_t rssi_samples = sx127x_reg_read(dev, SX127X_REG_RSSICONFIG) &
                           ~SX127X_RF_RSSICONFIG_SMOOTHING_MASK;
    return (1 << (1 + rssi_samples));
}

void sx127x_set_rssi_offset(sx127x_t *dev, int8_t rssi_offset)
{
    sx127x_reg_write(dev, SX127X_REG_RSSICONFIG,
                         (sx127x_reg_read(dev, SX127X_REG_RSSICONFIG) &
                          SX127X_RF_RSSICONFIG_OFFSET_MASK) | rssi_offset);
}

int8_t sx127x_get_rssi_offset(const sx127x_t *dev)
{
    int8_t rssi_offset = sx127x_reg_read(dev, SX127X_REG_RSSICONFIG) &
                           ~SX127X_RF_RSSICONFIG_OFFSET_MASK;
    return rssi_offset;
}


void sx127x_set_sync_on(sx127x_t *dev, uint8_t enable)
{
    sx127x_reg_write(dev, SX127X_REG_SYNCCONFIG,
                     (sx127x_reg_read(dev, SX127X_REG_SYNCCONFIG) &
                      SX127X_RF_SYNCCONFIG_SYNC_MASK) |
                     (enable << 4));
}

uint8_t sx127x_get_sync_on(const sx127x_t *dev)
{
    return ((sx127x_reg_read(dev, SX127X_REG_SYNCCONFIG) &
             ~SX127X_RF_SYNCCONFIG_SYNC_MASK) >> 4);
}

void sx127x_set_preamble_detect_on(sx127x_t *dev, uint8_t enable)
{
     sx127x_reg_write(dev, SX127X_REG_PREAMBLEDETECT,
                     (sx127x_reg_read(dev, SX127X_REG_PREAMBLEDETECT) &
                      SX127X_RF_PREAMBLEDETECT_DETECTOR_MASK) |
                     (enable << 7));
}

uint8_t sx127x_get_preamble_detect_on(const sx127x_t *dev)
{
    return ((sx127x_reg_read(dev, SX127X_REG_PREAMBLEDETECT) &
            ~SX127X_RF_PREAMBLEDETECT_DETECTOR_MASK) >> 7);
}

void sx127x_set_preamble_detect_tol(sx127x_t *dev, uint8_t tol)
{
    sx127x_reg_write(dev, SX127X_REG_PREAMBLEDETECT,
                     (sx127x_reg_read(dev, SX127X_REG_PREAMBLEDETECT) &
                      SX127X_RF_PREAMBLEDETECT_DETECTORTOL_MASK) | tol);
}

uint8_t sx127x_get_preamble_detect_tol(const sx127x_t *dev)
{
    return (sx127x_reg_read(dev, SX127X_REG_PREAMBLEDETECT) &
            ~SX127X_RF_PREAMBLEDETECT_DETECTORTOL_MASK);
}

void sx127x_set_preamble_detect_size(sx127x_t *dev, uint8_t size)
{
    sx127x_reg_write(dev, SX127X_REG_PREAMBLEDETECT,
                    (sx127x_reg_read(dev, SX127X_REG_PREAMBLEDETECT) &
                     SX127X_RF_PREAMBLEDETECT_DETECTORSIZE_MASK) |
                     ((size -1) << 5));
}

uint8_t sx127x_get_preamble_detect_size(const sx127x_t *dev)
{
    return (((sx127x_reg_read(dev, SX127X_REG_PREAMBLEDETECT) &
            ~SX127X_RF_PREAMBLEDETECT_DETECTORSIZE_MASK) >> 5) + 1);
}

void sx127x_set_dc_free(sx127x_t *dev, uint8_t encoding_scheme)
{
    sx127x_reg_write(dev, SX127X_REG_PACKETCONFIG1,
                         (sx127x_reg_read(dev, SX127X_REG_PACKETCONFIG1) &
                          SX127X_RF_PACKETCONFIG1_DCFREE_MASK) |
                         (encoding_scheme << 5));
}

uint8_t sx127x_get_dc_free(const sx127x_t *dev)
{
    return ((sx127x_reg_read(dev, SX127X_REG_PACKETCONFIG1) & ~SX127X_RF_PACKETCONFIG1_DCFREE_MASK) >> 5);
}

void sx127x_set_tx_fdev(sx127x_t *dev, uint32_t fdev)
{
    uint16_t tmpFedev = (uint16_t)(fdev / SX127X_FREQUENCY_RESOLUTION);

    DEBUG("[sx127x] Set Frequency deviation to %i", tmpFedev);
    sx127x_reg_write_u16(dev, SX127X_REG_FDEVMSB, tmpFedev);
}

uint32_t sx127x_get_tx_fdev(const sx127x_t *dev)
{
    uint16_t fdev = sx127x_reg_read_u16(dev, SX127X_REG_FDEVMSB);
    return fdev * SX127X_FREQUENCY_RESOLUTION;
}

void sx127x_set_modulation_shaping(sx127x_t *dev, uint8_t shaping)
{
    sx127x_reg_write(dev, SX127X_REG_OPMODE,
                     (sx127x_reg_read(dev, SX127X_REG_OPMODE) &
                      SX127X_RF_OPMODE_MODULATIONSHAPING_MASK) |
                     (shaping << 3));
}

uint8_t sx127x_get_modulation_shaping(const sx127x_t *dev)
{
    uint8_t modulation_shaping = sx127x_reg_read(dev, SX127X_REG_OPMODE) &
                                 ~SX127X_RF_OPMODE_MODULATIONSHAPING_MASK;

    switch (modulation_shaping) {
        case SX127X_RF_OPMODE_MODULATIONSHAPING_00: DEBUG("No Shaping"); break;
        case SX127X_RF_OPMODE_MODULATIONSHAPING_01: DEBUG("Gaussian filter BT = 1.0"); break;
        case SX127X_RF_OPMODE_MODULATIONSHAPING_10: DEBUG("Gaussian filter BT = 0.5"); break;
        case SX127X_RF_OPMODE_MODULATIONSHAPING_11: DEBUG("Gaussian filter BT = 0.3"); break;
    }

    return (modulation_shaping >> 3);
}

int sx127x_set_option(sx127x_t *dev, uint8_t option, bool state)
{
    switch(option) {
        case SX127X_OPT_TELL_TX_START:
            DEBUG("[sx127x] set_option: inform when TX START (%s)\n", state ? "Enable" : "Disable");
            break;
        case SX127X_OPT_TELL_TX_END:
            DEBUG("[sx127x] set_option: inform when TX is terminated (%s)\n", state ? "Enable" : "Disable");
            break;
        case SX127X_OPT_TELL_RX_START:
            DEBUG("[sx127x] set_option: inform when a packet header is received (%s)\n", state ? "Enable" : "Disable");
            break;
        case SX127X_OPT_TELL_RX_END:
            DEBUG("[sx127x] set_option: inform when a packet is received (%s)\n", state ? "Enable" : "Disable");
            break;
        case SX127X_OPT_TELL_TX_REFILL:
            DEBUG("[sx127x] set_option: inform when TX fifo needs to be refilled (%s)\n", state ? "Enable" : "Disable");
            break;
        case SX127X_OPT_PRELOADING:
            DEBUG("[sx127x] set_option: TX FIFO is just filled for preloading (No TX mode) (%s)\n", state ? "Enable" : "Disable");
            break;
    }

    /* set option field */
    if (state) {
        dev->options |= option;
    }
    else {
        dev->options &= ~(option);
    }

    return sizeof(netopt_enable_t);
}

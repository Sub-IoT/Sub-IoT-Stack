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
 * @brief       Basic functionality of sx127x driver
 *
 * @author      Eugene P. <ep@unwds.com>
 * @author      José Ignacio Alamos <jose.alamos@inria.cl>
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 * @}
 */
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "timer.h"
#include "errors.h"
#include "debug.h"

#include "hwgpio.h"
#include "hwspi.h"
#include "hwsystem.h"

#include "net/lora.h"

#include "sx127x.h"
#include "sx127x_internal.h"
#include "sx127x_registers.h"
#include "sx127x_netdev.h"
#include "platform.h"

#include "hal_defs.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(HAL_RADIO_LOG_ENABLED)
#include "log.h"
    #define DEBUG(...) log_print_string(__VA_ARGS__)
#else
    #define DEBUG(...)
#endif

/* Internal functions */
static int _init_spi(sx127x_t *dev);
static int _init_gpios(sx127x_t *dev);
static void _init_timers(sx127x_t *dev);
static void _on_tx_timeout(void *arg);
static void _on_rx_timeout(void *arg);

/* SX127X DIO interrupt handlers initialization */
static void sx127x_on_dio0_isr(void *arg);
static void sx127x_on_dio1_isr(void *arg);
static void sx127x_on_dio2_isr(void *arg);
static void sx127x_on_dio3_isr(void *arg);


void sx127x_setup(sx127x_t *dev, const sx127x_params_t *params)
{
    netdev_t *netdev = (netdev_t*) dev;
    netdev->driver = &sx127x_driver;
    memcpy(&dev->params, params, sizeof(sx127x_params_t));
}

void sx127x_reset(const sx127x_t *dev)
{
    // this function is implemented in platform_main.c
    // TODO expose and invoke the platform API
}

int sx127x_init(sx127x_t *dev)
{
    /* Do internal initialization routines */
    if (_init_spi(dev) < 0) {
        DEBUG("[sx127x] error: failed to initialize SPI\n");
        return -SX127X_ERR_SPI;
    }

    /* reset options */
    dev->options = 0;

    /* set default options */
    sx127x_set_option(dev, SX127X_OPT_TELL_RX_END, true);
    sx127x_set_option(dev, SX127X_OPT_TELL_TX_END, true);

    /* Check presence of SX127X */
    if (sx127x_check_version(dev) < 0) {
        DEBUG("[sx127x] error: no valid device found\n");
        return -SX127X_ERR_NODEV;
    }

    _init_timers(dev);
     hw_busy_wait(1000); /* wait 1 millisecond */

#ifdef PLATFORM_SX127X_USE_RESET_PIN
    sx127x_reset(dev);
#endif
    sx127x_set_op_mode(dev, SX127X_RF_OPMODE_STANDBY);
    sx127x_rx_chain_calibration(dev);
    sx127x_set_op_mode(dev, SX127X_RF_OPMODE_SLEEP);

    if (_init_gpios(dev) < 0) {
        DEBUG("[sx127x] error: failed to initialize GPIOs\n");
        return -SX127X_ERR_GPIOS;
    }

    return SX127X_INIT_OK;
}

void sx127x_init_radio_settings(sx127x_t *dev)
{
    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:

            sx127x_reg_write(dev, SX127X_REG_OPMODE, 0x00); // FSK, hi freq, sleep
            sx127x_reg_write(dev, SX127X_REG_PARAMP, (2 << 5) | 0x09); // BT=0.5 and PaRamp=40us
            sx127x_reg_write(dev, SX127X_REG_LNA, 0x23); // highest gain for now, for 868 // TODO LnaBoostHf consumes 150% current compared to default LNA

            // TODO validate:
            // - RestartRxOnCollision (off for now)
            // - RestartRxWith(out)PllLock flags: set on freq change
            // - AfcAutoOn: default for now
            // - AgcAutoOn: default for now (use AGC)
            // - RxTrigger: default for now
            sx127x_reg_write(dev, SX127X_REG_RXCONFIG, 0x0E);

            sx127x_reg_write(dev, SX127X_REG_RSSICONFIG, 0x02); // TODO no RSSI offset for now + using 8 samples for smoothing
            //  sx127x_reg_write(dev, SX127X_REG_RSSICOLLISION, 0); // TODO not used for now
            sx127x_reg_write(dev, SX127X_REG_RSSITHRESH, 0xFF); // TODO using -128 dBm for now

            //  write_reg(REG_AFCBW, 0); // TODO not used for now (AfcAutoOn not set)
            //  write_reg(REG_AFCFEI, 0); // TODO not used for now (AfcAutoOn not set)
            //  write_reg(REG_AFCMSB, 0); // TODO not used for now (AfcAutoOn not set)
            //  write_reg(REG_AFCLSB, 0); // TODO not used for now (AfcAutoOn not set)
            //  write_reg(REG_FEIMSB, 0); // TODO freq offset not used for now
            //  write_reg(REG_FEILSB, 0); // TODO freq offset not used for now
            sx127x_reg_write(dev, SX127X_REG_PREAMBLEDETECT, 0xCA); // TODO validate PreambleDetectorSize (2 now) and PreambleDetectorTol (10 now)

            sx127x_reg_write(dev, SX127X_REG_SYNCCONFIG, 0x11); // no AutoRestartRx, default PreambePolarity, enable syncword of 2 bytes

            sx127x_reg_write(dev, SX127X_REG_PACKETCONFIG1, 0x08); // fixed length (unlimited length mode), CRC auto clear OFF, whitening and CRC disabled (not compatible), addressFiltering off.
            sx127x_reg_write(dev, SX127X_REG_PACKETCONFIG2, 0x40); // packet mode
            sx127x_reg_write(dev, SX127X_REG_PAYLOADLENGTH, 0x00); // unlimited length mode (in combination with PacketFormat = 0), so we can encode/decode length byte in software
            sx127x_reg_write(dev, SX127X_REG_FIFOTHRESH, 0x83); // tx start condition true when there is at least one byte in FIFO (we are in standby/sleep when filling FIFO anyway)
                                                                // For RX the threshold is set to 4 since this is the minimum length of a D7 packet (number of bytes in FIFO >= FifoThreshold + 1).

            sx127x_reg_write(dev, SX127X_REG_SEQCONFIG1, 0x40); // force off for now
            //  write_reg(REG_SEQCONFIG2, 0); // not used for now
            //  write_reg(REG_TIMERRESOL, 0); // not used for now
            //  write_reg(REG_TIMER1COEF, 0); // not used for now
            //  write_reg(REG_TIMER2COEF, 0); // not used for now
            //  write_reg(REG_IMAGECAL, 0); // TODO not used for now
            //  write_reg(REG_LOWBAT, 0); // TODO not used for now

            sx127x_reg_write(dev, SX127X_REG_DIOMAPPING1, 0x0C); // DIO0 = 00 | DIO1 = 00 | DIO2 = 0b11 => interrupt on sync detect | DIO3 = 00
            sx127x_reg_write(dev, SX127X_REG_DIOMAPPING2, 0x30); // ModeReady TODO configure for RSSI interrupt when doing CCA?
            //  write_reg(REG_PLLHOP, 0); // TODO might be interesting for channel hopping
            //  write_reg(REG_TCXO, 0); // default
            //  write_reg(REG_PADAC, 0); // default
            //  write_reg(REG_FORMERTEMP, 0); // not used for now
            //  write_reg(REG_BITRATEFRAC, 0); // default
            //  write_reg(REG_AGCREF, 0); // default, TODO validate
            //  write_reg(REG_AGCTHRESH1, 0); // not used for now
            //  write_reg(REG_AGCTHRESH2, 0); // not used for now
            //  write_reg(REG_AGCTHRESH3, 0); // not used for now
            //  write_reg(REG_PLL, 0); // not used for now

            sx127x_set_tx_timeout(dev, SX127X_TX_TIMEOUT_DEFAULT);
            sx127x_set_modem(dev, SX127X_MODEM_DEFAULT);
            sx127x_set_channel(dev, SX127X_CHANNEL_DEFAULT);
            sx127x_set_bandwidth(dev, SX127X_BW_DEFAULT);
            sx127x_set_payload_length(dev, SX127X_PAYLOAD_LENGTH);
            sx127x_set_tx_power(dev, SX127X_RADIO_TX_POWER);
            break;
        case SX127X_MODEM_LORA:
            DEBUG("[sx127x] initializing radio settings\n");
            sx127x_set_channel(dev, SX127X_CHANNEL_DEFAULT);
            sx127x_set_modem(dev, SX127X_MODEM_DEFAULT);
            sx127x_set_tx_power(dev, SX127X_RADIO_TX_POWER);
            sx127x_set_bandwidth(dev, LORA_BW_DEFAULT);
            sx127x_set_spreading_factor(dev, LORA_SF_DEFAULT);
            sx127x_set_coding_rate(dev, LORA_CR_DEFAULT);
            sx127x_set_crc(dev, LORA_PAYLOAD_CRC_ON_DEFAULT);
            sx127x_set_freq_hop(dev, LORA_FREQUENCY_HOPPING_DEFAULT);
            sx127x_set_hop_period(dev, LORA_FREQUENCY_HOPPING_PERIOD_DEFAULT);
            sx127x_set_fixed_header_len_mode(dev, LORA_FIXED_HEADER_LEN_MODE_DEFAULT);
            sx127x_set_iq_invert(dev, LORA_IQ_INVERTED_DEFAULT);
            sx127x_set_payload_length(dev, LORA_PAYLOAD_LENGTH_DEFAULT);
            sx127x_set_preamble_length(dev, LORA_PREAMBLE_LENGTH_DEFAULT);
            sx127x_set_symbol_timeout(dev, LORA_SYMBOL_TIMEOUT_DEFAULT);
            sx127x_set_rx_single(dev, SX127X_RX_SINGLE);
            sx127x_set_tx_timeout(dev, SX127X_TX_TIMEOUT_DEFAULT);
            break;
        default:
            assert(false);//not implemented
            break;
    }
}

uint32_t sx127x_random(sx127x_t *dev)
{
    uint32_t rnd = 0;

    sx127x_set_modem(dev, SX127X_MODEM_LORA); /* Set LoRa modem ON */

    /* Disable LoRa modem interrupts */
    sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGSMASK, SX127X_RF_LORA_IRQFLAGS_RXTIMEOUT |
                     SX127X_RF_LORA_IRQFLAGS_RXDONE |
                     SX127X_RF_LORA_IRQFLAGS_PAYLOADCRCERROR |
                     SX127X_RF_LORA_IRQFLAGS_VALIDHEADER |
                     SX127X_RF_LORA_IRQFLAGS_TXDONE |
                     SX127X_RF_LORA_IRQFLAGS_CADDONE |
                     SX127X_RF_LORA_IRQFLAGS_FHSSCHANGEDCHANNEL |
                     SX127X_RF_LORA_IRQFLAGS_CADDETECTED);

    /* Set radio in continuous reception */
    sx127x_set_op_mode(dev, SX127X_RF_OPMODE_RECEIVER);

    for (unsigned i = 0; i < 32; i++) {
        hw_busy_wait(1000); /* wait for the chaos */

        /* Non-filtered RSSI value reading. Only takes the LSB value */
        rnd |= ((uint32_t) sx127x_reg_read(dev, SX127X_REG_LR_RSSIWIDEBAND) & 0x01) << i;
    }

    sx127x_set_sleep(dev);

    return rnd;
}

/**
 * IRQ handlers
 */
void sx127x_isr(netdev_t *dev)
{
    if (dev->event_callback) {
        dev->event_callback(dev, NETDEV_EVENT_ISR);
    }
}

static void sx127x_on_dio_isr(sx127x_t *dev, sx127x_flags_t flag)
{
    dev->irq |= flag;
    sx127x_isr((netdev_t *)dev);
}

static void sx127x_on_dio0_isr(void *arg)
{
    sx127x_t *dev = (sx127x_t*) arg;
    hw_gpio_disable_interrupt(dev->params.dio0_pin);
    sx127x_on_dio_isr((sx127x_t*) arg, SX127X_IRQ_DIO0);
}

static void sx127x_on_dio1_isr(void *arg)
{
    sx127x_t *dev = (sx127x_t*) arg;
    hw_gpio_disable_interrupt(dev->params.dio1_pin);
    sx127x_on_dio_isr((sx127x_t*) arg, SX127X_IRQ_DIO1);
}

static void sx127x_on_dio2_isr(void *arg)
{
    sx127x_on_dio_isr((sx127x_t*) arg, SX127X_IRQ_DIO2);
}

static void sx127x_on_dio3_isr(void *arg)
{
    sx127x_on_dio_isr((sx127x_t*) arg, SX127X_IRQ_DIO3);
}

/* Internal event handlers */
static int _init_gpios(sx127x_t *dev)
{
    error_t res = SUCCESS;

    res = hw_gpio_configure_interrupt(dev->params.dio0_pin, GPIO_RISING_EDGE, &sx127x_on_dio0_isr, dev);
    if (res != SUCCESS){
        DEBUG("[sx127x] error: failed to initialize DIO0 pin\n");
        return res;
    }

    res = hw_gpio_configure_interrupt(dev->params.dio1_pin, GPIO_RISING_EDGE, &sx127x_on_dio1_isr, dev);
    if (res != SUCCESS){
        DEBUG("[sx127x] error: failed to initialize DIO1 pin\n");
        return res;
    }

    res = hw_gpio_configure_interrupt(dev->params.dio2_pin, GPIO_RISING_EDGE, &sx127x_on_dio2_isr, dev);
    if (res != SUCCESS){
        DEBUG("[sx127x] error: failed to initialize DIO2 pin\n");
        return res;
    }

    res = hw_gpio_configure_interrupt(dev->params.dio3_pin, GPIO_RISING_EDGE, &sx127x_on_dio3_isr, dev);
    if (res != SUCCESS){
        DEBUG("[sx127x] error: failed to initialize DIO3 pin\n");
        return res;
    }

    return res;
}

static void _on_tx_timeout(void *arg)
{
    netdev_t *dev = (netdev_t *) arg;

    dev->event_callback(dev, NETDEV_EVENT_TX_TIMEOUT);
}

static void _on_rx_timeout(void *arg)
{
    netdev_t *dev = (netdev_t *) arg;

    dev->event_callback(dev, NETDEV_EVENT_RX_TIMEOUT);
}

static void _init_timers(sx127x_t *dev)
{
    dev->_internal.tx_timeout_timer.f = _on_tx_timeout;
    dev->_internal.tx_timeout_timer.priority = MAX_PRIORITY;
    timer_init_event(&dev->_internal.tx_timeout_timer, &_on_tx_timeout);

    dev->_internal.rx_timeout_timer.f = _on_rx_timeout;
    dev->_internal.rx_timeout_timer.priority = MAX_PRIORITY;
    timer_init_event(&dev->_internal.rx_timeout_timer, &_on_rx_timeout);
}

static int _init_spi(sx127x_t *dev)
{
    /* Setup SPI for SX127X */
    spi_handle_t* spi_handle = spi_init(dev->params.spi, SX127x_SPI_BAUDRATE, 8, true, false, NULL);

    if (spi_handle == NULL) {
        DEBUG("sx127x: error initializing SPI_%i device\n", dev->params.spi);
        return -1;
    }

    spi_slave_handle_t* sx127x_spi = spi_init_slave(spi_handle, dev->params.nss_pin, true);
    if (sx127x_spi == NULL) {
        DEBUG("sx127x: error initializing GPIO_%ld as CS line \n",
                  (long)dev->params.nss_pin);
        return -1;
    }
    dev->_internal.spi_slave = sx127x_spi;

    DEBUG("sx127x: peripherals initialized with success\n");
    return 0;
}

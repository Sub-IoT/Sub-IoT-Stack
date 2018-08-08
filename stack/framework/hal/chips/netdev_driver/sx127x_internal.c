/*
 * Copyright (c) 2016 Unwired Devices <info@unwds.com>
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
 * @brief       implementation of internal functions for sx127x
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

#include "hal_defs.h"

#include "hwatomic.h"
#include "hwsystem.h"

#include "net/lora.h"

#include "sx127x.h"
#include "sx127x_registers.h"
#include "sx127x_internal.h"
#include "sx127x_params.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(HAL_RADIO_LOG_ENABLED)
#include "log.h"
    #define DEBUG(...) log_print_string(__VA_ARGS__)
#else
    #define DEBUG(...)
#endif


#define SX127X_SPI_SPEED    (SPI_CLK_1MHZ)
#define SX127X_SPI_MODE     (SPI_MODE_0)


int sx127x_check_version(const sx127x_t *dev)
{
    /* Read version number and compare with sx127x assigned revision */
    uint8_t version = sx127x_reg_read(dev, SX127X_REG_VERSION);

#if defined(MODULE_SX1272)
    if (version != VERSION_SX1272) {
        DEBUG("[sx127x] sx1272 test failed, invalid version number: %d\n",
              version);
        return -1;
    }
    DEBUG("[sx127x] SX1272 transceiver detected\n");
#else /* MODULE_SX1276) */
    if (version != VERSION_SX1276) {
        DEBUG("[sx127x] sx1276 test failed, invalid version number: %d\n",
              version);
        return -1;
    }
    DEBUG("[sx127x] SX1276 transceiver detected\n");
#endif

    return 0;
}

void sx127x_reg_write(const sx127x_t *dev, uint8_t addr, uint8_t data)
{
    sx127x_reg_write_burst(dev, addr, &data, 1);
}

void sx127x_reg_write_u16(const sx127x_t *dev, uint8_t addr, uint16_t value)
{
    void *sx127x_spi = dev->_internal.spi_slave;

    start_atomic();

    spi_select(sx127x_spi);
    spi_exchange_byte(sx127x_spi, addr | 0x80); // send address with bit 8 high to signal a write operation
    spi_exchange_byte(sx127x_spi, (value >> 8) & 0xff);
    spi_exchange_byte(sx127x_spi, value & 0xff);
    spi_deselect(sx127x_spi);

    end_atomic();
}


uint8_t sx127x_reg_read(const sx127x_t *dev, uint8_t addr)
{
    uint8_t data;

    sx127x_reg_read_burst(dev, addr, &data, 1);

    return data;
}

uint16_t sx127x_reg_read_u16(const sx127x_t *dev, uint8_t addr)
{
    void *sx127x_spi = dev->_internal.spi_slave;

    start_atomic();

    spi_select(sx127x_spi);
    spi_exchange_byte(sx127x_spi, addr & 0x7F); // send address with bit 7 low to signal a read operation
    uint16_t value = spi_exchange_byte(sx127x_spi, 0x00); // get the response
    value <<= 8;
    value += spi_exchange_byte(sx127x_spi, 0x00);
    spi_deselect(sx127x_spi);

    end_atomic();
    return value;
}

void sx127x_reg_write_burst(const sx127x_t *dev, uint8_t addr, uint8_t *buffer,
                            uint8_t size)
{
    void *sx127x_spi = dev->_internal.spi_slave;

    start_atomic();

    spi_select(sx127x_spi);
    spi_exchange_byte(sx127x_spi, addr | 0x80);
    spi_exchange_bytes(sx127x_spi, buffer, NULL, size);
    spi_deselect(sx127x_spi);

    end_atomic();
}

void sx127x_reg_read_burst(const sx127x_t *dev, uint8_t addr, uint8_t *buffer,
                           uint8_t size)
{
	void *sx127x_spi = dev->_internal.spi_slave;

    start_atomic();

    spi_select(sx127x_spi);
    spi_exchange_byte(sx127x_spi, addr);
    spi_exchange_bytes(sx127x_spi, NULL, buffer, size);
    spi_deselect(sx127x_spi);

    end_atomic();
}

void sx127x_write_fifo(const sx127x_t *dev, uint8_t *buffer, uint8_t size)
{
    sx127x_reg_write_burst(dev, 0, buffer, size);
}

void sx127x_read_fifo(const sx127x_t *dev, uint8_t *buffer, uint8_t size)
{
    sx127x_reg_read_burst(dev, 0, buffer, size);
}

void sx127x_rx_chain_calibration(sx127x_t *dev)
{
    uint8_t reg_pa_config_init_val;
    uint32_t initial_freq;

    DEBUG("[sx127x] RX calibration");

    /* Save context */
    reg_pa_config_init_val = sx127x_reg_read(dev, SX127X_REG_PACONFIG);
    initial_freq = (double) (((uint32_t) sx127x_reg_read(dev, SX127X_REG_FRFMSB) << 16)
                             | ((uint32_t) sx127x_reg_read(dev, SX127X_REG_FRFMID) << 8)
                             | ((uint32_t) sx127x_reg_read(dev, SX127X_REG_FRFLSB))) * (double) SX127X_FREQUENCY_RESOLUTION;

    /* Cut the PA just in case, RFO output, power = -1 dBm */
    sx127x_reg_write(dev, SX127X_REG_PACONFIG, 0x00);

    /* Launch Rx chain calibration for LF band */
    sx127x_reg_write(dev,
                     SX127X_REG_IMAGECAL,
                     (sx127x_reg_read(dev, SX127X_REG_IMAGECAL) & SX127X_RF_IMAGECAL_IMAGECAL_MASK)
                     | SX127X_RF_IMAGECAL_IMAGECAL_START);

    while ((sx127x_reg_read(dev, SX127X_REG_IMAGECAL) & SX127X_RF_IMAGECAL_IMAGECAL_RUNNING)
           == SX127X_RF_IMAGECAL_IMAGECAL_RUNNING) {
    }

    /* Set a frequency in HF band */
    sx127x_set_channel(dev, SX127X_HF_CHANNEL_DEFAULT);

    /* Launch Rx chain calibration for HF band */
    sx127x_reg_write(dev,
                     SX127X_REG_IMAGECAL,
                     (sx127x_reg_read(dev, SX127X_REG_IMAGECAL) & SX127X_RF_IMAGECAL_IMAGECAL_MASK)
                     | SX127X_RF_IMAGECAL_IMAGECAL_START);
    while ((sx127x_reg_read(dev, SX127X_REG_IMAGECAL) & SX127X_RF_IMAGECAL_IMAGECAL_RUNNING)
           == SX127X_RF_IMAGECAL_IMAGECAL_RUNNING) {
    }

    /* Restore context */
    sx127x_reg_write(dev, SX127X_REG_PACONFIG, reg_pa_config_init_val);
    sx127x_set_channel(dev, initial_freq);
}

int16_t sx127x_read_rssi(sx127x_t *dev)
{
    int16_t rssi = 0;

    if (dev->settings.state != SX127X_RF_RX_RUNNING)
    {
        sx127x_set_state(dev, SX127X_RF_RX_RUNNING);
        /* Set radio in continuous reception */
        sx127x_set_op_mode(dev, SX127X_RF_OPMODE_RECEIVER);
        hw_busy_wait(700); // wait settling time and RSSI smoothing time
    }

    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            rssi = -(sx127x_reg_read(dev, SX127X_REG_RSSIVALUE) >> 1);
            break;
        case SX127X_MODEM_LORA:
#if defined(MODULE_SX1272)
            rssi = SX127X_RSSI_OFFSET + sx127x_reg_read(dev, SX127X_REG_LR_RSSIVALUE);
#else /* MODULE_SX1276 */
            if (dev->settings.channel > SX127X_RF_MID_BAND_THRESH) {
                rssi = SX127X_RSSI_OFFSET_HF + sx127x_reg_read(dev, SX127X_REG_LR_RSSIVALUE);
            }
            else {
                rssi = SX127X_RSSI_OFFSET_LF + sx127x_reg_read(dev, SX127X_REG_LR_RSSIVALUE);
            }
#endif
            break;
        default:
            rssi = -1;
            break;
    }

    return rssi;
}

void sx127x_start_cad(sx127x_t *dev)
{
    switch (dev->settings.modem) {
        case SX127X_MODEM_FSK:
            break;
        case SX127X_MODEM_LORA:
            /* Disable all interrupts except CAD-related */
            sx127x_reg_write(dev, SX127X_REG_LR_IRQFLAGSMASK,
                             SX127X_RF_LORA_IRQFLAGS_RXTIMEOUT |
                             SX127X_RF_LORA_IRQFLAGS_RXDONE |
                             SX127X_RF_LORA_IRQFLAGS_PAYLOADCRCERROR |
                             SX127X_RF_LORA_IRQFLAGS_VALIDHEADER |
                             SX127X_RF_LORA_IRQFLAGS_TXDONE |
                             /*SX127X_RF_LORA_IRQFLAGS_CADDONE |*/
                             SX127X_RF_LORA_IRQFLAGS_FHSSCHANGEDCHANNEL
                             /* | SX127X_RF_LORA_IRQFLAGS_CADDETECTED*/
                             );

            /* DIO3 = CADDone */
            sx127x_reg_write(dev, SX127X_REG_DIOMAPPING1,
                             (sx127x_reg_read(dev, SX127X_REG_DIOMAPPING1) &
                              SX127X_RF_LORA_DIOMAPPING1_DIO3_MASK) |
                             SX127X_RF_LORA_DIOMAPPING1_DIO3_00);

            sx127x_set_state(dev,  SX127X_RF_CAD);
            sx127x_set_op_mode(dev, SX127X_RF_LORA_OPMODE_CAD);
            break;
        default:
            break;
    }
}

bool sx127x_is_channel_free(sx127x_t *dev, uint32_t freq, int16_t rssi_threshold)
{
    int16_t rssi = 0;

    sx127x_set_channel(dev, freq);
    sx127x_set_op_mode(dev, SX127X_RF_OPMODE_RECEIVER);

    hw_busy_wait(1000); /* wait 1 millisecond */

    rssi = sx127x_read_rssi(dev);
    sx127x_set_sleep(dev);

    return (rssi <= rssi_threshold);
}


void sx127x_flush_fifo(const sx127x_t *dev)
{
    sx127x_reg_write(dev, SX127X_REG_IRQFLAGS2, 0x10);
}

bool sx127x_is_fifo_empty(const sx127x_t *dev)
{
#ifdef PLATFORM_SX127X_USE_DIO3_PIN
    return hw_gpio_get_in(dev->params.dio3_pin);
#else
    return (sx127x_reg_read(dev, SX127X_REG_IRQFLAGS2) & 0x40);
#endif
}

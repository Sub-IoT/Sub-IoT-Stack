/*
 * Copyright (C) 2017 Inria
 *               2017 Inria Chile
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_sx127x
 * @{
 * @file
 * @brief       Default configuration for SX127X driver
 *
 * @author      José Ignacio Alamos <jose.alamos@inria.cl>
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#ifndef SX127X_PARAMS_H
#define SX127X_PARAMS_H

#include "platform.h"
#include "sx127x.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PLATFORM_SX127X_USE_DIO3_PIN
#define SX127X_PARAM_DIO3 SX127x_DIO3_PIN
#endif
#ifdef PLATFORM_SX127X_USE_RESET_PIN
#define SX127X_PARAM_RESET SX127x_RESET_PIN
#endif


/**
 * @name    Set default configuration parameters for the SX127X driver
 *          Pins are adapted to STM32 Nucleo-64 boards.
 * @{
 */
#ifndef SX127X_PARAM_SPI
#define SX127X_PARAM_SPI                    (SPI_DEV(0))
#endif

#ifndef SX127X_PARAM_SPI_NSS
#define SX127X_PARAM_SPI_NSS                PIN(1, 6)       /* D10 */
#endif

#ifndef SX127X_PARAM_RESET
#define SX127X_PARAM_RESET                  PIN(0, 0)       /* A0 */
#endif

#ifndef SX127X_PARAM_DIO0
#define SX127X_PARAM_DIO0                   PIN(0, 10)      /* D2 */
#endif

#ifndef SX127X_PARAM_DIO1
#define SX127X_PARAM_DIO1                   PIN(1, 3)       /* D3 */
#endif

#ifndef SX127X_PARAM_DIO2
#define SX127X_PARAM_DIO2                   PIN(1, 5)       /* D4 */
#endif

#ifndef SX127X_PARAM_DIO3
#define SX127X_PARAM_DIO3                   PIN(1, 4)       /* D5 */
#endif

#ifndef SX127X_PARAM_PASELECT
#define SX127X_PARAM_PASELECT               (SX127X_PA_RFO)
#endif

#define SX127X_PARAMS_DEFAULT               { .spi       = SX127X_PARAM_SPI,     \
                                              .nss_pin   = SX127X_PARAM_SPI_NSS, \
                                              .reset_pin = SX127X_PARAM_RESET,   \
                                              .dio0_pin  = SX127X_PARAM_DIO0,    \
                                              .dio1_pin  = SX127X_PARAM_DIO1,    \
                                              .dio2_pin  = SX127X_PARAM_DIO2,    \
                                              .dio3_pin  = SX127X_PARAM_DIO3,    \
                                              .paselect  = SX127X_PARAM_PASELECT }


#define SX127X_PARAMS_BOARD                 { .spi       = SX127x_SPI_INDEX,   \
                                              .nss_pin   = SX127x_SPI_PIN_CS,  \
                                              .reset_pin = SX127X_PARAM_RESET, \
                                              .dio0_pin  = SX127x_DIO0_PIN,    \
                                              .dio1_pin  = SX127x_DIO1_PIN,    \
                                              .dio2_pin  = SX127X_PARAM_DIO2,  \
                                              .dio3_pin  = SX127X_PARAM_DIO3,  \
                                              .paselect  = SX127X_PARAM_PASELECT }
/**@}*/

/**
 * @brief   SX127X configuration
 */
static const sx127x_params_t sx127x_params[] =
{
#ifdef SX127X_PARAMS_BOARD
    SX127X_PARAMS_BOARD,
#else
    SX127X_PARAMS_DEFAULT,
#endif
};

#ifdef __cplusplus
}
#endif

#endif /* SX127X_PARAMS_H */
/** @} */

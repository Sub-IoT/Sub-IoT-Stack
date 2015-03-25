/*! \file
 *
 *
 */

#ifndef CC1101_H
#define CC1101_H

/* \brief Callback called by cc1101_interface_{spi/cc430} when end_of_packet interrupt occurs.
 * Note: this is called from an interrupt context so should contain minimal processing.
 *
 */
typedef void(*end_of_packet_isr_t)(void);

#endif // CC1101_H

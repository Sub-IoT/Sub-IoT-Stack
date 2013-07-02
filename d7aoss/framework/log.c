/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#include "log.h"
#include "string.h"

#include "../hal/uart.h"

#include <stdio.h>

// TODO only in debug mode?
#define BUFFER_SIZE 50
static char buffer[BUFFER_SIZE];


// TODO: can be removed now log_print_string accepts a printf style format string?
///* Custom reverse() function for which the length of the string is
// * known in advance (avoids a call to strlen() and including
// * string.h).
// *
// * @args: a null-terminated string
// * @return: nothing
// * @result: the characters in str are reversed
// */
//void reverse(char* str, uint8_t length){
//	uint8_t i = 0, j = length-1;
//	uint8_t tmp;
//    while (i < j) {
//        tmp = str[i];
//        str[i] = str[j];
//        str[j] = tmp;
//        i++; j--;
//    }
//}
//
///* Returns the string representation of integer n. Assumes 32-bit
// * int, and 8-bit bytes (i.e. sizeof(char) = 1, sizeof(int) = 4).
// * Assumes char *out is big enough to hold the string
// * representation of n.
// *
// * @args: int n to convert, char* out for the result
// * @result the string representation of n is stored in out
// * @return 0 on success, -1 on error
// */
//bool itoa(int32_t n, char* out)
//{
//    // if negative, need 1 char for the sign
//	uint8_t sign = n < 0? 1: 0;
//	uint8_t i = 0;
//    if (n == 0) {
//        out[i++] = '0';
//    } else if (n < 0) {
//        out[i++] = '-';
//        n = -n;
//    }
//    while (n > 0) {
//        out[i++] = '0' + n % 10;
//        n /= 10;
//    }
//    out[i] = '\0';
//    reverse(out + sign, i - sign);
//    return 0;
//}

void log_print_string(char* format, ...)
{
    va_list args;
    va_start(args, format);
    uint8_t len = vsnprintf(buffer, BUFFER_SIZE, format, args);
    va_end(args);

	uart_transmit_data(0xDD);
	uart_transmit_data(LOG_TYPE_STRING);
	uart_transmit_data(len);
	uart_transmit_message((unsigned char*) buffer, len);
}

void log_print_data(uint8_t* message, uint8_t length)
{
	uart_transmit_data(0xDD);
	uart_transmit_data(LOG_TYPE_DATA);
	uart_transmit_data(length);
	uart_transmit_message((unsigned char*) message, length);
}

void log_phy_rx_res(phy_rx_data_t* res)
{
	// transmit the log header
	uart_transmit_data(0xDD);
	uart_transmit_data(LOG_TYPE_PHY_RX_RES);
	uart_transmit_data(LOG_TYPE_PHY_RX_RES_SIZE + res->length);

	// transmit struct member per member, so we are not dependent on packing
	uart_transmit_data(res->rssi);
	uart_transmit_data(res->lqi);
	uart_transmit_data(res->length);

	// transmit the packet
	uart_transmit_message(res->data, res->length);
}

void log_dll_rx_res(dll_rx_res_t* res)
{
	uart_transmit_data(0xDD);
	uart_transmit_data(LOG_TYPE_DLL_RX_RES);
	uart_transmit_data(LOG_TYPE_DLL_RX_RES_SIZE);
	uart_transmit_data(res->frame_type);
	uart_transmit_data(res->spectrum_id);
}

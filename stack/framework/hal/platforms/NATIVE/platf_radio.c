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

/*! \file dummy_radio.c
 *
 *  \author Liam Wickins <liamw9534@gmail.com>
 *
 */

#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>

#include "fifo.h"
#include "timer.h"
#include "hwradio.h"
#include "errors.h"
#include "log.h"


// RSSI levels
#define RSSI_BELOW_NOISE_LEVEL 	-120
#define RSSI_ABOVE_NOISE_LEVEL 	0

#define US_PER_SEC     1000000
#define NS_PER_SEC     1000000000UL

#define RX_QUEUE_SIZE  4
#define MCAST_GROUP		"239.0.0.1"
#define PORT 			5000

#ifdef HAL_RADIO_LOG_ENABLED

#define DPRINT(...) 	  log_print_stack_string(LOG_STACK_RADIO, __VA_ARGS__)
#define DPRINT_DATA(p, n) log_print_data(p, n)

#else

#define DPRINT(...)
#define DPRINT_DATA(p, n)

#endif

static hwradio_init_args_t dummy_radio;
static uint32_t own_addr = 0;

static uint8_t send_data[1024];
static unsigned int send_length = 0;
static bool refill_enable = false;

static char stack_top[8192];
static int recv_fd = -1;
static int send_fd = -1;
static char rx_buffer[65536];
static volatile hw_radio_state_t radio_opmode = HW_STATE_OFF;
static hw_radio_packet_t *rx_packets[4];
static fifo_t rx_packet_fifo;
static bool radio_idle = false;
static volatile bool radio_low_power_wakeup = false;
static volatile int16_t last_rssi = RSSI_BELOW_NOISE_LEVEL;

static uint32_t radio_sync_size = 0;
static uint32_t radio_center_freq = 0;
static uint32_t radio_bitrate = 0;
static uint32_t radio_bw = 0;

static void execute_send();

// Internal radio packet structure -- it contains additional meta data that
// is required for filtering of received packets over UDP multi-cast.
// The source_addr is set through the additional API call hwradio_set_addr().
// This should be unique on the network of applications but if running on
// a single local machine, one can use the PID to set the address since PID
// is unique on a single machine.

struct radio_packet_header
{
	uint32_t source_addr;
	uint64_t timestamp;
	uint32_t center_freq;
	uint32_t bw;
	uint32_t bitrate;
};


// This is used purely for performance related timestamping to assess any delays
// of network packets when debugging problems -- it has no functional purpose

static uint64_t timestamp()
{
	uint64_t val;
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	val = (tp.tv_sec * NS_PER_SEC) + tp.tv_nsec;
	return val;
}

// The main handling of transmitting and receiving UDP packets happens here.  The main
// consideration is to:
// i)   Filter out any packets that we shouldn't process i.e., packets
//      send by oneself or packets received when the radio is not configured
//      on the same channel or indeed if the radio is switched off or idle
// ii)  Strip out preamble and SYNC bytes
// iii) Allocating of packet structure and queueing
// iv)  Notification to stack with suitable radio delays based on bitrate

static int radio_handler(void *arg)
{
	int no_frame_received_count = 0;

	while (1)
	{
		// Notify any FIFOd radio RX packets
		while (fifo_get_size(&rx_packet_fifo))
		{
			// Do not notify unless radio is in RX state and not idle
			if (radio_opmode != HW_STATE_RX || radio_idle)
				break;

			// Pop next packet off the queue and RX notify
			hw_radio_packet_t *packet;
			fifo_pop(&rx_packet_fifo, (uint8_t *)&packet, sizeof(hw_radio_packet_t *));
			DPRINT("radio_handler: notify packet size=%u", packet->length);
			dummy_radio.rx_packet_cb(packet);
		}

		// Check for a pending TX packet to schedule -- this will also handle TX complete notifications
		if (send_length > 0 && radio_opmode == HW_STATE_TX)
			execute_send();

		// Try to receive any RX radio packets
		//DPRINT("radio_handler: recv...");
		int rc = recv(recv_fd, rx_buffer, sizeof(rx_buffer), 0);

		// Check in case no packet was received
		if (rc == -1)
		{
			// If no packets have been received within a certain timeframe and the RSSI has not been
			// consumed already (i.e., by calling get_rssi) then clear down the RSSI here
			no_frame_received_count++;
			if (no_frame_received_count >= 50)
				last_rssi = RSSI_BELOW_NOISE_LEVEL;  // Set this value to below the noise level
			continue;
		} else
			no_frame_received_count = 0;  // Reset RSSI clear down counter

		// Ignore packets that are not of the required size (should never happen in theory)
		if (rc >= sizeof(struct radio_packet_header))
		{
			uint32_t recv_addr;
			struct radio_packet_header *packet = (struct radio_packet_header *)rx_buffer;
			uint64_t trx = timestamp();  // Timestamp for debugging RX timing

			DPRINT("radio_handler: received UDP packet of %d bytes", rc);

			// Force low power mode wakeup, if the main scheduler process is sleeping
			radio_low_power_wakeup = true;

			// Set a nominal RSSI level for receive level
			last_rssi = RSSI_ABOVE_NOISE_LEVEL;

			// Filter received UDP packets in accordance with the source
			// address (i.e., ignore packets from self) and radio properties
			// such as frequency, bw and bitrate and the radio operating state
			if (packet->source_addr != own_addr &&
				packet->bw == radio_bw &&
				packet->center_freq == radio_center_freq &&
				packet->bitrate == radio_bitrate &&
				radio_opmode == HW_STATE_RX &&
				!radio_idle)
			{
				int preamble_length = 0;
				uint8_t *p = rx_buffer + sizeof(struct radio_packet_header);
				rc += sizeof(struct radio_packet_header);  // Skip over radio packet header now since we've filtered now

				// Skip preamble bytes -- it is assumed that AA bytes are stripped from the beginning of each frame
				// where present
				while (*p == 0xAA && rc)
				{
					p++;
					rc--;
					preamble_length++;
				}

				DPRINT("latency=%lu ns preamble_length=%d detected, remaining bytes=%d", (trx-packet->timestamp), preamble_length, rc);

				// Skip the sync word if configured and there was a preamble detected
				if (preamble_length && rc >= radio_sync_size)
				{
					p += radio_sync_size;
					rc -= radio_sync_size;
				}

				// Proceed if we have a payload remaining
				if (rc > 0)
				{
					hw_radio_packet_t *packet = dummy_radio.alloc_packet_cb(rc);
					if (!packet)
					{
						DPRINT("radio_handler: packet dropped as unable to allocate packet buffer");
						continue;
					}
					packet->length = rc;
					memcpy(&packet->data, p, rc);
					packet->rx_meta.crc_status = HW_CRC_VALID;  // No CRC errors in this implementation
					packet->rx_meta.rssi = RSSI_ABOVE_NOISE_LEVEL;
					packet->rx_meta.timestamp = timer_get_counter_value();
					DPRINT("radio_handler: queued radio packet %p of %d bytes", packet, packet->length);
					if (fifo_put(&rx_packet_fifo, (uint8_t *)&packet, sizeof(hw_radio_packet_t *)) != SUCCESS)
						DPRINT("radio_handler: packet dropped as RX FIFO is full");
				}
				else
				{
					DPRINT("radio_handler: discarding empty data payload");
				}
			}
			else
			{
				DPRINT("radio_handler: packet dropped as does not meet RX criteria");
			}
		}
	}
}

// This function will send a UDP radio packet that has already been constructed by a
// hw_radio_send_payload API call.  It will also handle notifications back to the stack and
// differentiate between when the radio is in refill mode (i.e., BG ETA flooding) or
// normal transmission mode.

static void execute_send()
{
	static uint64_t delay_send_tstart = 0;
	static unsigned int len = 0;

	if (len == 0)
	{
		// Initiate transmission start procedure
		len = send_length;
		delay_send_tstart = timestamp();

		DPRINT("execute_send: new packet len=%i scheduled for transmission", len);
	}

    // Compute time until packet "transmitted" (i.e., radio TX time)
    uint64_t delay_ns = ((NS_PER_SEC * (len * 8)) / radio_bitrate);
	if ((timestamp() - delay_send_tstart) < delay_ns)
		return;

	// Schedule transmission of packet
	struct sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(MCAST_GROUP);
    servaddr.sin_port = htons(PORT);

	DPRINT("execute_send: packet len=%i scheduled for transmission after delay_ns=%lu", len, delay_ns);

    if (sendto(send_fd, send_data, len, 0,
               (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
    	DPRINT("execute_send: sendto failed");
    }

    // This marks the packet as "consumed"
    send_length = 0;
    len = 0;

    // Notify based on refill mode
    if (refill_enable)
    {
    	DPRINT("execute_send: tx_refill_cb");
    	dummy_radio.tx_refill_cb(len);
    }
    else
    {
    	DPRINT("execute_send: tx_packet_cb");
    	dummy_radio.tx_packet_cb(timer_get_counter_value());
    }
}

// This routine will initialise our dummy radio -- it includes creating a cloned process for
// handling the received packets which is done async to the main scheduler process.

error_t hw_radio_init(hwradio_init_args_t* init_args)
{
	DPRINT("hw_radio_init");
	dummy_radio = *init_args;

	// A FIFO is used for queueing received UDP packets since these typically get received in a burst and not
	// in accordance with normal radio timing
	fifo_init(&rx_packet_fifo, (uint8_t *)rx_packets, sizeof(hw_radio_packet_t *) * RX_QUEUE_SIZE);

	// Global send_fd used for transmitting UDP radio packets
    send_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_fd < 0)
    {
		perror("socket");
    	return -1;
    }

	// Global recv_fd used for transmitting UDP radio packets
	recv_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (recv_fd < 0) {
		perror("socket");
		return -1;
	}

	// This allows our receive UDP port number to be reused by multiple processes which is only ever
	// allowed for multicast packets
	int yes = 1;
	if (setsockopt(recv_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
	{
		perror("setsockopt SO_REUSEADDR");
		close(recv_fd);
		exit(-1);
	}

	// Binds our receive UDP port to a reusable port
	struct sockaddr_in addr;
	bzero((char *)&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PORT);

	if (bind(recv_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("bind");
		close(recv_fd);
		exit(-1);
	}

	// Configures our receive UDP socket for multicast traffic on the
	// designated multicast group
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr(MCAST_GROUP);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(recv_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			     &mreq, sizeof(mreq)) < 0) {
		perror("setsockopt IP_ADD_MEMBERSHIP");
		close(recv_fd);
		exit(-1);
	}

	// To allow notification of received packets, the recv_fd has a small timeout on
	// rather than blocking forever.
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 10;
	if (setsockopt (recv_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
	                sizeof(timeout)) < 0)
	{
		perror("setsockopt timeout");
		close(recv_fd);
		return -1;
	}

	// We create a new process that runs the radio_handler -- it must be created with
	// shared memory so that the main scheduler can interact with the stored radio state
	clone(radio_handler, &stack_top[8192], CLONE_VM, NULL);

	DPRINT("own_addr = %08X", own_addr);

	return SUCCESS;
}

/** \brief Stop the radio driver, and free the hardware resources (SPI, GPIO interrupts, ...)
 */
void hw_radio_stop(void)
{
	DPRINT("hw_radio_stop");
	radio_idle = true;
}

error_t hw_radio_set_idle(void)
{
	DPRINT("hw_radio_set_idle");
	radio_idle = true;

	return SUCCESS;
}

// This will construct a radio packet by prepending a radio packet header
// to the supplied data payload.  The actual sending happens inside the
// radio_handler process.

error_t hw_radio_send_payload(uint8_t * data, uint16_t len)
{
	assert(send_length == 0);

	struct radio_packet_header *packet = (struct radio_packet_header *)send_data;
	packet->timestamp = timestamp();

	DPRINT("hw_radio_send_payload(%u) ttx=%lu", len, packet->timestamp);

	packet->source_addr = own_addr;
	packet->center_freq = radio_center_freq;
	packet->bw = radio_bw;
	packet->bitrate = radio_bitrate;
	memcpy(send_data + sizeof(struct radio_packet_header), data, len);
	send_length = len + sizeof(struct radio_packet_header);

	return SUCCESS;
}

bool hw_radio_tx_busy(void)
{
	DPRINT("hw_radio_tx_busy");
	return false;
}

bool hw_radio_rx_busy(void)
{
	DPRINT("hw_radio_rx_busy");
	return false;
}

bool hw_radio_rssi_valid(void)
{
	DPRINT("hw_radio_rssi_valid");
	return true;
}

int16_t hw_radio_get_rssi(void)
{
	int16_t rssi = last_rssi;
	DPRINT("hw_radio_get_rssi");

	last_rssi = RSSI_BELOW_NOISE_LEVEL;

	return rssi;
}

hw_radio_state_t hw_radio_get_opmode(void)
{
	DPRINT("hw_radio_get_opmode = %u", radio_opmode);
	return radio_opmode;
}

void hw_radio_set_opmode(hw_radio_state_t opmode)
{
	DPRINT("hw_radio_set_opmode(%u)", opmode);

	radio_opmode = opmode;
	radio_idle = false;   // We clear the idle state here since the global state change overrides it

	// This only switches the opmode it doesn't schedule send or receive which is done by the radio_handler process

	if (opmode == HW_STATE_TX)
	{
		DPRINT("HW_STATE_TX");
	}
	else if (opmode == HW_STATE_RX)
	{
		DPRINT("HW_STATE_RX");
	}
	else if (opmode == HW_STATE_OFF)
	{
		DPRINT("HW_STATE_OFF");
	}
	else if (opmode == HW_STATE_SLEEP)
	{
		DPRINT("HW_STATE_SLEEP");
	}
	else if (opmode == HW_STATE_IDLE)
	{
		DPRINT("HW_STATE_IDLE");
	}
	else if (opmode == HW_STATE_RESET)
	{
		DPRINT("HW_STATE_RESET");
	}
	else if (opmode == HW_STATE_STANDBY)
	{
		DPRINT("HW_STATE_STANDBY");
	}
}

void hw_radio_set_center_freq(uint32_t center_freq)
{
	DPRINT("hw_radio_set_center_freq(%u)", center_freq);
	radio_center_freq = center_freq;  // Used for packet filtering
}

void hw_radio_set_rx_bw_hz(uint32_t bw_hz)
{
	DPRINT("hw_radio_set_rx_bw_hz(%u)", bw_hz);
	radio_bw = bw_hz;  // Used for packet filtering
}

void hw_radio_set_bitrate(uint32_t bps)
{
	DPRINT("hw_radio_set_bitrate(%u)", bps);
	radio_bitrate = bps;  // Used for packet filtering
}

void hw_radio_set_tx_fdev(uint32_t fdev)
{
	DPRINT("hw_radio_set_tx_fdev(%u)", fdev);
}

void hw_radio_set_preamble_size(uint16_t size)
{
	DPRINT("hw_radio_set_preamble_size(%u)", size);
}

void hw_radio_set_preamble_detector(uint8_t preamble_detector_size, uint8_t preamble_tol)
{
	DPRINT("hw_radio_set_preamble_detector(%u,%u)", preamble_detector_size, preamble_tol);
}

void hw_radio_set_rssi_config(uint8_t rssi_smoothing, uint8_t rssi_offset)
{
	DPRINT("hw_radio_set_rssi_config(%u,%u)", rssi_smoothing, rssi_offset);
}

void hw_radio_set_dc_free(uint8_t scheme)
{
	DPRINT("hw_radio_set_dc_free(%u)", scheme);
}

void hw_radio_set_sync_word(uint8_t *sync_word, uint8_t sync_size)
{
	DPRINT("hw_radio_set_sync_word(%02x,%u)", *sync_word, sync_size);
	radio_sync_size = sync_size;  // Used for removal of SYNC word
	// TODO: Sync word is not presently validated
}

void hw_radio_set_crc_on(uint8_t enable)
{
	DPRINT("hw_radio_set_crc_on(%u)", enable);
	// Not used -- CRC always assumed to be correct
}

void hw_radio_set_payload_length(uint16_t length)
{
	DPRINT("hw_radio_set_payload_length(%u)", length);
}

bool hw_radio_is_idle(void)
{
	DPRINT("hw_radio_is_idle");
	return radio_idle;
}

bool hw_radio_is_rx(void)
{
	DPRINT("hw_radio_is_rx");
	return radio_opmode == HW_STATE_RX;
}

void hw_radio_enable_refill(bool enable)
{
	DPRINT("hw_radio_enable_refill(%u)", enable);
	refill_enable = enable;  // Used to control TX notification behaviour
}

void hw_radio_enable_preloading(bool enable)
{
	DPRINT("hw_radio_enable_preloading(%u)", enable);
}

void hw_radio_set_tx_power(int8_t eirp)
{
	DPRINT("hw_radio_set_tx_power(%d)", eirp);
}

void hw_radio_set_rx_timeout(uint32_t timeout)
{
	DPRINT("hw_radio_set_rx_timeout(%u)", timeout);
}

void hwradio_set_addr(int addr)
{
	// Own address is used as the source_addr field in the radio packet header for UDP transmissions
	own_addr = addr;
}

void hwradio_enter_low_power_mode()
{
	// Allows the radio module to know when the CPU (scheduler) has entered low power mode
	radio_low_power_wakeup = false;
}

bool hwradio_wakeup_from_lowpower_mode()
{
	return radio_low_power_wakeup;
}

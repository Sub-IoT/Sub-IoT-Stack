/*********************************************************************************
 * This confidential and proprietary software may be used only as authorized 
 *                      by a licensing agreement from                           
 *                           Cortus S.A.
 *
 *             (C) Copyright 2004, 2005, 2006 Cortus S.A.
 *                           ALL RIGHTS RESERVED
 *
 * The entire notice above must be reproduced on all authorized copies
 * and any such reproduction must be pursuant to a licensing agreement 
 * from Cortus S.A. (http://www.cortus.com)
 *
 * $CortusRelease$
 * $FileName$
 *
 *********************************************************************************/

#ifndef _USB_H
#define _USB_H
#include <machine/sfradr.h>

typedef struct USB
{
    volatile unsigned status;       // 0x00  set to 1, when USB reset occurs
    volatile unsigned enable;       // 0x04  enables USB device (i.e. enables pullup on DP)
    volatile unsigned mask;         // 0x08  mask for interrupt
} USB;

typedef struct USB_Endpoint
{
    volatile void*    buf0;         // 0x00  Pointer to buffer 0
    volatile void*    buf1;         // 0x04  Pointer to buffer 1
    volatile unsigned buf_done;     // 0x08  (wo cmd) Firmware writes 1 when done with current buffer
    volatile unsigned buf_status;   // 0x0c  (ro) Endpoint buffer status
    volatile unsigned stall;        // 0x10  (r/w cmd) Make endpoint stall
    volatile unsigned mask;         // 0x14  (r/w cmd) Interrupt mask
    volatile unsigned next;         // 0x18  (ro) next buffer to be processed by firmware
    volatile unsigned device_addr;  // 0x1c  usb global device address
    volatile unsigned ready;        // 0x20  (r/w cmd) Firmware is ready
    volatile unsigned toggle_rst;   // 0x24  (wo cmd) Set next packet sequence/toggle bit to 0
} USB_Endpoint;

/*
 * For a host OUT endpoint, the buffer status indicates the number of buffers which are full.
 * The next field, indicates which buffer neads to be read next.
 * Once the buffer is read, buf_done needs to be set.
 *
 * For a host IN endpoint, the buffer status indicates the number of buffers which are empty.
 * The next field, indicates the buext buffer which should be filled for tranmsission.
 * Once the buffer has been filled, buf_done needs to be set.
 *
 *
 * for and host OUT endpoint
 * i.e. the number of buffers we need to deal with.
 * For a HOST in endpoint, the buffer status indicates the number of empty buffers. 
 */

#ifdef __APS__
#define usb_ep0_out     ((USB_Endpoint*) SFRADR_USB_EP0_OUT)
#define usb_ep0_in      ((USB_Endpoint*) SFRADR_USB_EP0_IN)
#define usb             ((USB*) SFRADR_USB)
#define usb_ep1_out     ((USB_Endpoint*) SFRADR_USB_EP1_OUT)
#define usb_ep2_in      ((USB_Endpoint*) SFRADR_USB_EP2_IN)
#else
extern USB_Endpoint __usb_ep0_out;
extern USB_Endpoint __usb_ep0_in;
extern USB_Endpoint __usb_ep1_out;
extern USB_Endpoint __usb_ep2_in;
#define usb_ep0_out (&__usb_ep0_out)
#define usb_ep0_in  (&__usb_ep0_in)
#define usb_ep1_out (&__usb_ep1_out)
#define usb_ep2_in  (&__usb_ep2_in)
#endif

#endif

/***************************************************************************//**
 * @file descriptors.h
 * @brief USB descriptor prototypes for CDC Serial Port adapter example project.
 * @version 3.20.13
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/
#ifndef __SILICON_LABS_DESCRIPTORS_H__
#define __SILICON_LABS_DESCRIPTORS_H__

#include "em_usb.h"
#include "cdc.h"


EFM32_ALIGN(4)
static const USB_DeviceDescriptor_TypeDef USBDESC_deviceDesc __attribute__ ((aligned(4)))=
{
  .bLength            = USB_DEVICE_DESCSIZE,
  .bDescriptorType    = USB_DEVICE_DESCRIPTOR,
  .bcdUSB             = 0x0200,
  .bDeviceClass       = USB_CLASS_CDC,
  .bDeviceSubClass    = 0,
  .bDeviceProtocol    = 0,
  .bMaxPacketSize0    = USB_FS_CTRL_EP_MAXSIZE,
  .idVendor           = 0x10C4,
  .idProduct          = 0x0003,
  .bcdDevice          = 0x0000,
  .iManufacturer      = 1,
  .iProduct           = 2,
  .iSerialNumber      = 0,
  .bNumConfigurations = 1
};

#define CONFIG_DESCSIZE ( USB_CONFIG_DESCSIZE                   + \
                          (USB_INTERFACE_DESCSIZE * 2)          + \
                          (USB_ENDPOINT_DESCSIZE * NUM_EP_USED) + \
                          USB_CDC_HEADER_FND_DESCSIZE           + \
                          USB_CDC_CALLMNG_FND_DESCSIZE          + \
                          USB_CDC_ACM_FND_DESCSIZE              + \
                          5 )

EFM32_ALIGN(4)
static const uint8_t USBDESC_configDesc[] __attribute__ ((aligned(4)))=
{
  /*** Configuration descriptor ***/
  USB_CONFIG_DESCSIZE,    /* bLength                                   */
  USB_CONFIG_DESCRIPTOR,  /* bDescriptorType                           */
  CONFIG_DESCSIZE,        /* wTotalLength (LSB)                        */
  USB_CONFIG_DESCSIZE>>8, /* wTotalLength (MSB)                        */
  2,                      /* bNumInterfaces                            */
  1,                      /* bConfigurationValue                       */
  0,                      /* iConfiguration                            */
  CONFIG_DESC_BM_RESERVED_D7 |   /* bmAttrib: Self powered             */
  CONFIG_DESC_BM_SELFPOWERED,
  CONFIG_DESC_MAXPOWER_mA( 100 ),/* bMaxPower: 100 mA                  */

  /*** Communication Class Interface descriptor (interface no. 0)    ***/
  USB_INTERFACE_DESCSIZE, /* bLength               */
  USB_INTERFACE_DESCRIPTOR,/* bDescriptorType      */
  0,                      /* bInterfaceNumber      */
  0,                      /* bAlternateSetting     */
  1,                      /* bNumEndpoints         */
  USB_CLASS_CDC,          /* bInterfaceClass       */
  USB_CLASS_CDC_ACM,      /* bInterfaceSubClass    */
  0,                      /* bInterfaceProtocol    */
  0,                      /* iInterface            */

  /*** CDC Header Functional descriptor ***/
  USB_CDC_HEADER_FND_DESCSIZE, /* bFunctionLength  */
  USB_CS_INTERFACE_DESCRIPTOR, /* bDescriptorType  */
  USB_CLASS_CDC_HFN,      /* bDescriptorSubtype    */
  0x20,                   /* bcdCDC spec.no LSB    */
  0x01,                   /* bcdCDC spec.no MSB    */

  /*** CDC Call Management Functional descriptor ***/
  USB_CDC_CALLMNG_FND_DESCSIZE, /* bFunctionLength */
  USB_CS_INTERFACE_DESCRIPTOR,  /* bDescriptorType */
  USB_CLASS_CDC_CMNGFN,   /* bDescriptorSubtype    */
  0,                      /* bmCapabilities        */
  1,                      /* bDataInterface        */

  /*** CDC Abstract Control Management Functional descriptor ***/
  USB_CDC_ACM_FND_DESCSIZE, /* bFunctionLength     */
  USB_CS_INTERFACE_DESCRIPTOR, /* bDescriptorType  */
  USB_CLASS_CDC_ACMFN,    /* bDescriptorSubtype    */
  0x02,                   /* bmCapabilities        */
  /* The capabilities that this configuration supports:                   */
  /* D7..D4: RESERVED (Reset to zero)                                     */
  /* D3: 1 - Device supports the notification Network_Connection.         */
  /* D2: 1 - Device supports the request Send_Break                       */
  /* D1: 1 - Device supports the request combination of Set_Line_Coding,  */
  /*         Set_Control_Line_State, Get_Line_Coding, and the             */
  /*         notification Serial_State.                                   */
  /* D0: 1 - Device supports the request combination of Set_Comm_Feature, */
  /*         Clear_Comm_Feature, and Get_Comm_Feature.                    */

  /*** CDC Union Functional descriptor ***/
  5,                      /* bFunctionLength       */
  USB_CS_INTERFACE_DESCRIPTOR, /* bDescriptorType  */
  USB_CLASS_CDC_UNIONFN,  /* bDescriptorSubtype    */
  0,                      /* bControlInterface,      itf. no. 0 */
  1,                      /* bSubordinateInterface0, itf. no. 1 */

  /*** CDC Notification endpoint descriptor ***/
  USB_ENDPOINT_DESCSIZE,  /* bLength               */
  USB_ENDPOINT_DESCRIPTOR,/* bDescriptorType       */
  EP_NOTIFY,              /* bEndpointAddress (IN) */
  USB_EPTYPE_INTR,        /* bmAttributes          */
  USB_FS_INTR_EP_MAXSIZE, /* wMaxPacketSize (LSB)  */
  0,                      /* wMaxPacketSize (MSB)  */
  0xFF,                   /* bInterval             */

  /*** Data Class Interface descriptor (interface no. 1)                ***/
  USB_INTERFACE_DESCSIZE, /* bLength               */
  USB_INTERFACE_DESCRIPTOR,/* bDescriptorType      */
  1,                      /* bInterfaceNumber      */
  0,                      /* bAlternateSetting     */
  2,                      /* bNumEndpoints         */
  USB_CLASS_CDC_DATA,     /* bInterfaceClass       */
  0,                      /* bInterfaceSubClass    */
  0,                      /* bInterfaceProtocol    */
  0,                      /* iInterface            */

  /*** CDC Data interface endpoint descriptors ***/
  USB_ENDPOINT_DESCSIZE,  /* bLength               */
  USB_ENDPOINT_DESCRIPTOR,/* bDescriptorType       */
  EP_DATA_IN,             /* bEndpointAddress (IN) */
  USB_EPTYPE_BULK,        /* bmAttributes          */
  USB_FS_BULK_EP_MAXSIZE, /* wMaxPacketSize (LSB)  */
  0,                      /* wMaxPacketSize (MSB)  */
  0,                      /* bInterval             */

  USB_ENDPOINT_DESCSIZE,  /* bLength               */
  USB_ENDPOINT_DESCRIPTOR,/* bDescriptorType       */
  EP_DATA_OUT,            /* bEndpointAddress (OUT)*/
  USB_EPTYPE_BULK,        /* bmAttributes          */
  USB_FS_BULK_EP_MAXSIZE, /* wMaxPacketSize (LSB)  */
  0,                      /* wMaxPacketSize (MSB)  */
  0                       /* bInterval             */
};

STATIC_CONST_STRING_DESC_LANGID( langID, 0x04, 0x09 );
STATIC_CONST_STRING_DESC( iManufacturer, 'S','i','l','i','c','o','n',' ','L', \
                                         'a','b','o','r','a','t','o','r','i', \
                                         'e','s',' ','I','n','c','.' );
#if defined( _EZR_DEVICE )
STATIC_CONST_STRING_DESC( iProduct     , 'E','Z','R','3','2',' ','U','S','B', \
                                         ' ','C','D','C',' ','s','e','r','i', \
                                         'a','l',' ','p','o','r','t',' ','d', \
                                         'e','v','i','c','e' );
#else
STATIC_CONST_STRING_DESC( iProduct     , 'E','F','M','3','2',' ','U','S','B', \
                                         ' ','C','D','C',' ','s','e','r','i', \
                                         'a','l',' ','p','o','r','t',' ','d', \
                                         'e','v','i','c','e' );
#endif

static const void * const USBDESC_strings[] =
{
  &langID,
  &iManufacturer,
  &iProduct,
};

/* Endpoint buffer sizes */
/* 1 = single buffer, 2 = double buffering, 3 = triple buffering ...  */
/* Use double buffering on the BULK endpoints.                        */
static uint8_t USBDESC_bufferingMultiplier[ NUM_EP_USED + 1 ] = { 1, 1, 2, 2 };



#endif /* __SILICON_LABS_DESCRIPTORS_H__ */

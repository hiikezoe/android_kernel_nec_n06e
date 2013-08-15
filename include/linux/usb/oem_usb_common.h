
/*
 * include/linux/usb/oem_usb_commmon.h
 *
 * (C) NEC CASIO Mobile Communications, Ltd. 2012
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */












#if !defined(_OEM_USB_COMMON_H_)
#define _OEM_USB_COMMON_H_


#define DVE021_USB_FUNCTION_DVE021_SERIAL



#define DVE021_USB_PRODUCT_NAME       "N-06E"
#define DVE021_USB_PID_DEFAULT 0x0458 







#define DVE021_USB_MANUFACTURER_NAME  "NEC Corporation"


#define DVE021_USB_IF_DESC_NAME_ADB         DVE021_USB_PRODUCT_NAME " Android ADB Interface"
#define DVE021_USB_IF_DESC_NAME_DIAG        DVE021_USB_PRODUCT_NAME " Virtual Serial Port"
#define DVE021_USB_IF_DESC_NAME_MSC         DVE021_USB_PRODUCT_NAME " Mass Storage"
#define DVE021_USB_IF_DESC_NAME_NCM         DVE021_USB_PRODUCT_NAME " USB Networking Interface"
#define DVE021_USB_IF_DESC_NAME_MTP         DVE021_USB_PRODUCT_NAME
#define DVE021_USB_IF_DESC_NAME_PTP         DVE021_USB_PRODUCT_NAME
#if defined(DVE021_USB_FUNCTION_DVE021_SERIAL)
#define DVE021_USB_IF_DESC_NAME_DVE021_SERIAL "DOCOMO " DVE021_USB_PRODUCT_NAME
#endif


#define DVE021_USB_VENDOR_NAME    "NEC"
#define DVE021_USB_STORAGE_NAME   DVE021_USB_PRODUCT_NAME " Storage"

#endif 


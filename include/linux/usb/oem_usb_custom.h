
#if defined(DVE021_USB_FUNCTION_DVE021_SERIAL)
/*
 * include/linux/usb/oem_usb_custom.h
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










#if !defined(__LINUX_USB_OEM_USB_CUSTOM_H)
#define __LINUX_USB_OEM_USB_CUSTOM_H

#include <linux/usb/oem_usb_common.h>


enum usb_DVE022_serial_mode_status {
    USB_DVE021_SERIAL_MODE_OFF = 0,            
    USB_DVE021_SERIAL_MODE_ON,                 
};

enum usb_register_result {
    REGISTER_RESULT_OK = 0,            
    REGISTER_RESULT_NG,                
    REGISTER_RESULT_ALREADY,           
};














extern enum  usb_register_result oem_usb_regsiter_DVE022_serial_notify( void* cb );

#endif 
#endif 


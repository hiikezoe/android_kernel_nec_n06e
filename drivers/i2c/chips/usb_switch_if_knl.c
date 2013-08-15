/*
 * include/linux/i2c/usb_switch_if_knl.c
 *
 * (C) NEC CASIO Mobile Communications, Ltd. 2011
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














#include <linux/kernel.h>


#include "linux/i2c/bd91401gw.h"


#include "linux/i2c/usb_switch_if_knl.h"

extern int bd91401gw_get_current_voltage_state( unsigned char* current_voltage_state_p );       
extern int bd91401gw_get_device_state( usb_sw_device_state_enum* device_state_p );     

extern void bd91401gw_path_initialize_again_wrapper(void);















int usb_sw_get_current_voltage_state( unsigned char* current_voltage_state_p )
{





    
    int retVal = USB_SW_NG;
    
    
    if( current_voltage_state_p == NULL )
    {
        
        return USB_SW_NG;
    }

    retVal = bd91401gw_get_current_voltage_state( current_voltage_state_p );

    
    if( retVal == BD91401GW_NG )
    {
        
        return USB_SW_NG;
    }
    
    
    return USB_SW_OK;


}














int usb_sw_get_device_state( usb_sw_device_state_enum* device_state_p )
{





    
    int retVal = USB_SW_NG;
    printk(KERN_DEBUG "[USBSW]LINE:%u %s start\n",__LINE__,__func__);

    
    if( device_state_p == NULL )
    {
        
        return USB_SW_NG;
    }
    retVal = bd91401gw_get_device_state( device_state_p );
    printk(KERN_DEBUG "[USBSW]LINE:%u %s usb_sw_get_device_state(0x%x) called\n",__LINE__,__func__,*device_state_p);
    printk(KERN_DEBUG "[USBSW]LINE:%u %s retVal:%d\n",__LINE__,__func__,retVal);

    
    if( retVal == BD91401GW_NG )
    {
        printk(KERN_DEBUG "[USBSW]LINE:%u %s return value error\n",__LINE__,__func__);
        
        return USB_SW_NG;
    }
    
    printk(KERN_DEBUG "[USBSW]LINE:%u %s end\n",__LINE__,__func__);
    
    return USB_SW_OK;


}










void usb_sw_path_initialize(void)
{





    
    bd91401gw_path_initialize_again_wrapper();
    
    
    return;


}

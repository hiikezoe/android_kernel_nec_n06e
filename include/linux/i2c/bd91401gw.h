

#if !defined(BD91401GW_H)
#define BD91401GW_H
/*
 * include/linux/i2c/bd91401gw.h
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


















#include <linux/i2c.h>
#include <linux/ioctl.h>
#include <linux/irqreturn.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <mach/msm_hsusb.h>


#define BD91401GW_I2C_DEVICE_NAME  ("bd91401gw i2c drv")       
#define BD91401GW_I2C_SLAVE_ADDRESS (0x6A)                     


#define BD91401GW_I2C_PM_GPIO_INTB 37                          



#define BD91401GW_OK 0    
#define BD91401GW_NG -1   


#define BD91401GW_I2C_TRANSFER_OK 0     
#define BD91401GW_I2C_TRANSFER_NG -1    


#define BD91401GW_UCDCNT_ADDRESS            (0x02)    
#define BD91401GW_SWCONTROL_ADDRESS         (0x03)    
#define BD91401GW_INTERRUPT_ADDRESS         (0x05)    
#define BD91401GW_STATUS_ADDRESS            (0x06)    
#define BD91401GW_IDSTATUS_ADDRESS          (0x07)    
#define BD91401GW_SOFTRESET_ADDRESS         (0x08)    


#define BD91401GW_UCDCNT_RESET_VALUE        (0x50)    
#define BD91401GW_SOFTRESET_RESET_VALUE     (0x01)    
                          

#define BD91401GW_SWCONTROL_MASKB           (0xbf)    
#define BD91401GW_INTERRUPT_MASKB           (0xf0)    
#define BD91401GW_STATUS_MASKB              (0xef)    
#define BD91401GW_STATUS_CHGPORT_MASKB      (0xe0)    
#define BD91401GW_STATUS_OVPOCP_MASKB       (0x0f)    
#define BD91401GW_STATUS_OVPOCP_VB_MASKB    (0x0A)    
#define BD91401GW_IDSTATUS_INDO_MASKB       (0x0f)    
#define BD91401GW_IDSTATUS_MASKB            (0x1f)    

                                                    
#define BD91401GW_INTERRUPT_TYPE_DEFAULT    (0x00)    
#define BD91401GW_INTERRUPT_TYPE_VBUSDET    (0x10)    
#define BD91401GW_INTERRUPT_TYPE_COMPL      (0x20)    
#define BD91401GW_INTERRUPT_TYPE_COMPH      (0x40)    
#define BD91401GW_INTERRUPT_TYPE_CHGDET     (0x80)    


#define BD91401GW_STATUS_TYPE_OVPVC         (0x01)    
#define BD91401GW_STATUS_TYPE_OVPVB         (0x02)    
#define BD91401GW_STATUS_TYPE_OCPVC         (0x04)    
#define BD91401GW_STATUS_TYPE_OCPVB         (0x08)    
#define BD91401GW_STATUS_TYPE_CHGPORT_NOUSB (0x00)    
#define BD91401GW_STATUS_TYPE_CHGPORT_SDP   (0x20)    
#define BD91401GW_STATUS_TYPE_CHGPORT_CDP   (0x40)    
#define BD91401GW_STATUS_TYPE_CHGPORT_DCP   (0x60)    
#define BD91401GW_STATUS_TYPE_CHGPORT_IRREGULAR  (0x80) 
#define BD91401GW_STATUS_NOT_OVP_OCP             (0x00) 


#define BD91401GW_IDSTATUS_TYPE_STEREO_EAR  (0x18)    
#define BD91401GW_IDSTATUS_TYPE_MONO_EAR    (0x1c)    
#define BD91401GW_IDSTATUS_TYPE_CHG_EAR     (0x1b)    
#define BD91401GW_IDSTATUS_TYPE_UART        (0x16)    
#define BD91401GW_IDSTATUS_TYPE_DEFAULT     (0x0d)    
#define BD91401GW_IDSTATUS_TYPE_HOOKSW      (0x12)    
#define BD91401GW_IDSTATUS_TYPE_OTGHOST     (0x10)    

#define BD91401GW_IDSTATUS_TYPE_WRONG       (0x1d)    



#define BD91401GW_SWCONTROL_EAR             (0x12)    
#define BD91401GW_SWCONTROL_EAR_MIC         (0x92)    
#define BD91401GW_SWCONTROL_FEED_POW_EAR    (0x12)    
#define BD91401GW_SWCONTROL_CHG_EAR         (0x1a)    
#define BD91401GW_SWCONTROL_UART            (0x09)    
#define BD91401GW_SWCONTROL_DEFAULT         (0x00)    
#define BD91401GW_SWCONTROL_MIC_CHECK       (0x80)    
#define BD91401GW_SWCONTROL_MIC_CHECK_MODE3 (0x18)    
#define BD91401GW_SWCONTROL_UNCONNECTION    (0x3f)    


#define BD91401GW_RETRY_READ_REG_COUNT    10    
#define BD91401GW_RETRY_WRITE_REG_COUNT    5    


#define BD91401GW_I2C_READ_SLEEP_TIME  10       
#define BD91401GW_I2C_WRITE_SLEEP_TIME 10       

#define BD91401GW_IDSTATUS_REG_READ_SLEEP_TIME 1       



#define BD91401GW_I2C_MSG_WRITE_NUMBER 1        
#define BD91401GW_I2C_MSG_READ_NUMBER  2        







#define BD91401GW_MSM_GPIO_LOW_VALUE 0         
#define BD91401GW_MSM_GPIO_HIGH_VALUE 1        
#define BD91401GW_MSM_GPIO_USBSW_RESET_ID 68   



#define BD91401GW_SENDEND_KEY_POLLING_INTERVAL_TIME           10    

#define BD91401GW_MIC_NOTICE_INTERVAL_TIME                    30    


struct bd91401gw_platform_data_struct {
    u32  (*bd91401gw_setup)(void) ;       
    void (*bd91401gw_shutdown)(void) ;    
};


#define BD91401GW_ALERM_EVENT 0x50
#define BD91401GW_I2C_ALERM_EVENT 0x67
#define BD91401GW_ALERM_INFO_I2C_READ     0x80
#define BD91401GW_ALERM_INFO_I2C_WRITE    0x81


#define BD91401GW_I2C_READ_LEN             (0x01)
#define BD91401GW_I2C_WRITE_LEN            (0x02)

#define BD91401GW_I2C_COM_FORMAT_SEND      (0x01)    
#define BD91401GW_I2C_COM_FORMAT_RECV      (0x02)    
#define BD91401GW_I2C_COM_FORMAT_COMP      (BD91401GW_I2C_COM_FORMAT_SEND | BD91401GW_I2C_COM_FORMAT_RECV)    

#define BD91401GW_I2C_DATA_DISABLE         (0x00)    
#define BD91401GW_I2C_DATA_ENABLE_1BYTE    (0x04)    
#define BD91401GW_I2C_DATA_ENABLE_2BYTE    (0x08)    


enum usb_vbus_mode {
	USB_VBUS_INIT = -1,     
	USB_VBUS_ON   =  1,     
	USB_VBUS_OFF  =  0,     
};


typedef enum{
    USB_HEADSET_STEREO = 0, 
    USB_HEADSET_MONO,       
}usb_headset_stereo_mono_enum;


typedef struct{
    usb_headset_stereo_mono_enum stereo_mono_type;
}usb_headset_info_type;


 


 
enum chg_type bd91401gw_get_chg_type(void);       

 




 

#endif 





/*
 * drivers/i2c/chips/bd91401gw.c
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






















#include "linux/i2c/bd91401gw.h"
#include "linux/i2c/usb_switch_if_knl.h"

#include "linux/mfd/wcd9310/headset_com.h"








#include <linux/wakelock.h>
#include <linux/pmic8058-othc.h>
#include <linux/msm-charger.h>


MODULE_DESCRIPTION(BD91401GW_I2C_DEVICE_NAME);
MODULE_ALIAS("platform:bd91401gw i2c driver");
MODULE_LICENSE( "GPL" ) ;
MODULE_VERSION( "0.1" ) ;
MODULE_DEVICE_TABLE(i2c , bd91401gw_id_table);



typedef struct{
    unsigned char    address;         
    unsigned char    value;           
}bd91401gw_reg_type;


typedef struct{
    bd91401gw_reg_type    interrupt;        
}bd91401gw_interrupt_info_type;


typedef struct{
    bd91401gw_reg_type        status;       
    bd91401gw_reg_type        id_status;    
}bd91401gw_identify_device_info_type;


typedef struct{
    bd91401gw_reg_type        control;      
}bd91401gw_switch_path_info_type;


typedef struct{
    bd91401gw_interrupt_info_type        interrupt_info;        
    bd91401gw_identify_device_info_type  identify_device_info;  
    bd91401gw_switch_path_info_type      switch_path_info;      
}bd91401gw_read_device_info_type;


typedef struct{
    bd91401gw_reg_type    reset;       
    bd91401gw_reg_type    ucdcnt;      
}bd91401gw_init_info_type;


typedef struct{
    bd91401gw_switch_path_info_type   switch_path_info;   
    bd91401gw_init_info_type          init_info;          
}bd91401gw_write_device_info_type;


typedef struct bd91401gw_i2c_data_struct{
    struct i2c_client *client;           
    bool   inuse;                        
}bd91401gw_i2c_data_struct;


static struct i2c_device_id bd91401gw_id_table[] = {
    { BD91401GW_I2C_DEVICE_NAME , 0 }
   ,{ }
};


struct work_struct usbsw_work;

struct delayed_work sendend_key_polling_work;



struct delayed_work set_otg_work;                                          



struct work_struct usb_mic_off;                                             
struct work_struct usb_mic_on;                                              


struct work_struct path_init;                                               



void bd91401gw_init(void);                                                  
void bd91401gw_path_initialize(void);                                       
void bd91401gw_detect_intb(void);                                           
static void bd91401gw_main(struct work_struct *w);                          
int  bd91401gw_set_switch(bd91401gw_reg_type* reg_p);                       
int  bd91401gw_read_reg(bd91401gw_reg_type* reg_p);                         
int  bd91401gw_write_reg(bd91401gw_reg_type* reg_p);                        
int bd91401gw_get_current_voltage_state( unsigned char* current_voltage_state_p );       
int bd91401gw_get_device_state( usb_sw_device_state_enum* device_state_p );     
static void  bd91401gw_set_device_state( void  );                                  
void bd91401gw_notify_chg_usb_charger_connected( void );                    

static void bd91401gw_usbsw_set_otg_workable(struct work_struct *w);        


static void bd91401gw_set_ovpocp_attach_state( void );                      


static void bd91401gw_usb_audio_mic_off(struct work_struct *w);             
static void bd91401gw_usb_audio_mic_on(struct work_struct *w);              








void bd91401gw_path_initialize_again_wrapper(void);                         
static void bd91401gw_path_initialize_again(struct work_struct* w);         





static      irqreturn_t  bd91401gw_irq_handler(int irq, void* dev_id);                                       
static      u32          bd91401gw_irq_setup(struct i2c_client* client);                                     
static int  __devinit    bd91401gw_i2c_probe(struct i2c_client* client , const struct i2c_device_id* id);    
static int  __devexit    bd91401gw_i2c_remove(struct i2c_client* client);                                    
static int  __init       bd91401gw_i2c_init(void);                                                           
static void __exit       bd91401gw_i2c_exit(void);                                                           
int                      bd91401gw_i2c_read(unsigned char reg , unsigned char* data);                        
int                      bd91401gw_i2c_write(unsigned char reg , unsigned char value);                       


void bd91401gw_start_sendend_key_polling(void);                                 
static void bd91401gw_sendend_key_polling_handler(struct work_struct *w);       
void bd91401gw_stop_sendend_key_polling(void);                                  


extern void usbsw_set_otg_workable(int enable);


module_init(bd91401gw_i2c_init);


module_exit(bd91401gw_i2c_exit);


bd91401gw_read_device_info_type  g_read_device_info;    
bd91401gw_write_device_info_type g_write_device_info;   
bd91401gw_i2c_data_struct g_bd91401gw_i2c_modules;      
bd91401gw_switch_path_info_type g_unconnection_path;    
static struct i2c_driver  g_bd91401gw_i2c_driver = {    
    .driver   = {
           .owner = THIS_MODULE,
           .name  = BD91401GW_I2C_DEVICE_NAME,
    },
    .id_table = bd91401gw_id_table,
    .probe    = bd91401gw_i2c_probe,
    .remove   = bd91401gw_i2c_remove,
};
bool g_sendend_key_polling_started_flg = false;          
 

 
static usb_sw_device_state_enum g_usb_sw_device_state = USB_SW_DEVICE_DISCONNECTED;


struct wake_lock g_usbsw_wlock;


DEFINE_MUTEX(g_bd91401gw_usb_lock);

DEFINE_MUTEX(g_bd91401gw_usb_sendend_key_polling_lock);


usb_headset_info_type g_usb_headset_info;


unsigned char g_usb_mic_dec_waittime = 0x3C;                                     
bool          g_bd91401gw_mic_detect = false;                                    
bool          g_bd91401gw_mic_flg = false;                                       
 


 

int g_bd91401gw_audio_state;                                                     

bool          g_bd91401gw_audio_notify_flg = false;                              
bool          g_bd91401gw_sendend_notify_flg = false;                            
bd91401gw_switch_path_info_type g_mic_path;                                      
bd91401gw_switch_path_info_type g_mic_path_mode3;                                


static bool g_bd91401gw_ovpocp_attach_flg = false;                               
static unsigned char g_bd91401gw_ovpocp_attach_value = 0x00;                     



static bool   g_bd91401gw_chg_notify_flg = false;                                



static int intb_state = USB_VBUS_OFF;                                            

static bool bd91401gw_control_mic(void);                               
static void bd91401gw_set_audio_info(unsigned char mode_number, bool mic_flg);
static void bd91401gw_notify_mic(bool notify_flg);
static void bd91401gw_notify_sendend_key(bool notify_flg);

static int bd91401gw_pack_i2c_info(                                              
    int i2c_ret_val,
    unsigned char i2c_data_len,
    unsigned char i2c_data[2],
    unsigned char i2c_format,
    unsigned int* i2c_alarm_info );






#define WLOCK_TIMEOUT_SEC               10              
#define OVP_WLOCK_TIMEOUT_SEC           15              


#define USB_CHG_DET_DELAY msecs_to_jiffies(1000)        


#define USB_SW_NO_AUDIO_DEVICE  0 
#define USB_SW_USB_AUDIO_DEVICE 1 
#define USB_SW_3_5_AUDIO_DEVICE 2 








void bd91401gw_init(void){
    
    int ret =BD91401GW_NG;
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    
    
    
    g_read_device_info.interrupt_info.interrupt.address = BD91401GW_INTERRUPT_ADDRESS;
    g_read_device_info.interrupt_info.interrupt.value   = 0x00;
    
    
    
    g_read_device_info.identify_device_info.status.address    = BD91401GW_STATUS_ADDRESS;
    g_read_device_info.identify_device_info.status.value      = 0x00;
    g_read_device_info.identify_device_info.id_status.address = BD91401GW_IDSTATUS_ADDRESS;
    g_read_device_info.identify_device_info.id_status.value   = 0x00;
    
    
    
    g_read_device_info.switch_path_info.control.address  = BD91401GW_SWCONTROL_ADDRESS;
    g_read_device_info.switch_path_info.control.value    = 0x00;
    
    
    
    g_write_device_info.switch_path_info.control.address = BD91401GW_SWCONTROL_ADDRESS;
    g_write_device_info.switch_path_info.control.value   = 0x00;
    
    
    g_write_device_info.init_info.reset.address  = BD91401GW_SOFTRESET_ADDRESS;
    g_write_device_info.init_info.reset.value    = BD91401GW_SOFTRESET_RESET_VALUE;
    
    
    g_write_device_info.init_info.ucdcnt.address = BD91401GW_UCDCNT_ADDRESS;
    g_write_device_info.init_info.ucdcnt.value   = BD91401GW_UCDCNT_RESET_VALUE;
    
    
    g_unconnection_path.control.address  = BD91401GW_SWCONTROL_ADDRESS;
    g_unconnection_path.control.value    = BD91401GW_SWCONTROL_UNCONNECTION;
    
    
    g_mic_path.control.address  = BD91401GW_SWCONTROL_ADDRESS;
    g_mic_path.control.value    = BD91401GW_SWCONTROL_MIC_CHECK;
    
    
    g_mic_path_mode3.control.address  = BD91401GW_SWCONTROL_ADDRESS;
    g_mic_path_mode3.control.value    = BD91401GW_SWCONTROL_MIC_CHECK_MODE3;
    
    
    g_bd91401gw_mic_detect = false;
    g_bd91401gw_mic_flg = false;
 

 

    g_bd91401gw_audio_state = BD91401GW_IDSTATUS_TYPE_DEFAULT;

    g_bd91401gw_audio_notify_flg = false;
    g_bd91401gw_sendend_notify_flg = false;
    
    
    usbsw_set_otg_workable(USB_VBUS_OFF);
    printk(KERN_DEBUG "[USBSW]LINE:%u %s usbsw_set_otg_workable(USB_VBUS_OFF) call\n",__LINE__,__func__);
    
    
    ret = bd91401gw_write_reg(&g_write_device_info.init_info.reset);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_init! write_reg error(08 reset)  ret:%d \n",ret);
        return;
    }
    
    
    ret = bd91401gw_write_reg(&g_write_device_info.init_info.ucdcnt);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_init! write_reg error(02 EARR/EARL enable)  ret:%d \n",ret);
        return;
    }
    
    
    INIT_WORK(&usbsw_work, bd91401gw_main);

    INIT_WORK(&path_init, bd91401gw_path_initialize_again);


    printk(KERN_DEBUG "LINE:%u %s INIT_DELAYED_WORK(&set_otg_work, bd91401gw_usbsw_set_otg_workable) call\n",__LINE__,__func__);
    INIT_DELAYED_WORK(&set_otg_work, bd91401gw_usbsw_set_otg_workable);

    INIT_DELAYED_WORK(&sendend_key_polling_work, bd91401gw_sendend_key_polling_handler);


    INIT_WORK(&usb_mic_off, bd91401gw_usb_audio_mic_off);
    INIT_WORK(&usb_mic_on, bd91401gw_usb_audio_mic_on);























    
    wake_lock_init(&g_usbsw_wlock, WAKE_LOCK_SUSPEND, "usbsw");
    
    printk(KERN_DEBUG "LINE:%u %s end  ret:%d \n",__LINE__,__func__,ret);
    return;
}







void bd91401gw_path_initialize(void){
    
    int ret = BD91401GW_NG;
    bd91401gw_reg_type temp_read_area;
    
    temp_read_area.address = BD91401GW_INTERRUPT_ADDRESS;
    temp_read_area.value = 0x00;
    
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    
    mutex_lock(&g_bd91401gw_usb_lock);
    

    
    enable_irq(g_bd91401gw_i2c_modules.client->irq);

    
    
    ret = bd91401gw_read_reg(&temp_read_area);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_path_initialize! read_reg error(05)  ret:%d \n",ret);
        mutex_unlock(&g_bd91401gw_usb_lock);
        return;
    }
    
    printk(KERN_INFO "bd91401gw_path_initialize reg value 05(first time):0x%02x \n"
            ,temp_read_area.value);
    
    
    ret = bd91401gw_read_reg(&g_read_device_info.identify_device_info.id_status);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_path_initialize! read_reg error(07)  ret:%d \n",ret);
        mutex_unlock(&g_bd91401gw_usb_lock);
        return;
    }
    
    
    
    
    
    ret = bd91401gw_read_reg(&g_read_device_info.interrupt_info.interrupt);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_path_initialize! read_reg error(05)  ret:%d \n",ret);
        mutex_unlock(&g_bd91401gw_usb_lock);
        return;
    }
    
    
    
    
    
    
    
    g_read_device_info.interrupt_info.interrupt.value |= (temp_read_area.value &
                              (BD91401GW_INTERRUPT_TYPE_COMPL | BD91401GW_INTERRUPT_TYPE_COMPH));
    
    
    ret = bd91401gw_read_reg(&g_read_device_info.identify_device_info.status);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_path_initialize! read_reg error(06)  ret:%d \n",ret);
        mutex_unlock(&g_bd91401gw_usb_lock);
        return;
    }
    
    printk(KERN_INFO "bd91401gw_path_initialize reg value 05:%02x, 06:%02x, 07:%02x\n"
            ,g_read_device_info.interrupt_info.interrupt.value
            ,g_read_device_info.identify_device_info.status.value
            ,g_read_device_info.identify_device_info.id_status.value);
    
    
    ret = bd91401gw_set_switch(&g_write_device_info.switch_path_info.control);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_path_initialize! set switch error  ret:%d \n",ret);
        mutex_unlock(&g_bd91401gw_usb_lock);
        return;
    }
    
    printk(KERN_DEBUG "bd91401gw_path_initialize setting path value 03:%02x\n"
            ,g_write_device_info.switch_path_info.control.value);
    
    if((g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_CHG_EAR)
      || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_EAR)
      || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_EAR_MIC)
      || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_FEED_POW_EAR))
    {
        
        ret = bd91401gw_write_reg(&g_unconnection_path.control);
        if(BD91401GW_OK != ret)
        {
            printk(KERN_ERR "Error in bd91401gw_path_initialize! write_reg error(03)  ret:%d \n",ret);
            mutex_unlock(&g_bd91401gw_usb_lock);
            return;
        }
        
        udelay(5);
    }
    
    
    ret = bd91401gw_write_reg(&g_write_device_info.switch_path_info.control);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_path_initialize! write_reg error(03)  ret:%d \n",ret);
        mutex_unlock(&g_bd91401gw_usb_lock);
        return;
    }
    
    if((g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_CHG_EAR)
      || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_EAR)
      || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_EAR_MIC)
      || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_FEED_POW_EAR))
    {


        
        printk(KERN_DEBUG "LINE:%u %s call bd91401gw_notify_mic(true)\n",__LINE__,__func__);
        bd91401gw_notify_mic(true);
    }
    
    
    bd91401gw_set_device_state();

    
    bd91401gw_set_ovpocp_attach_state();

    
    if(((BD91401GW_INTERRUPT_TYPE_VBUSDET & g_read_device_info.interrupt_info.interrupt.value) == BD91401GW_INTERRUPT_TYPE_VBUSDET
        || (BD91401GW_STATUS_TYPE_OVPVB & g_read_device_info.identify_device_info.status.value) == BD91401GW_STATUS_TYPE_OVPVB
        || (BD91401GW_STATUS_TYPE_OCPVB & g_read_device_info.identify_device_info.status.value) == BD91401GW_STATUS_TYPE_OCPVB)

        && (BD91401GW_IDSTATUS_MASKB & g_read_device_info.identify_device_info.id_status.value) != BD91401GW_IDSTATUS_TYPE_OTGHOST
        && (BD91401GW_STATUS_TYPE_CHGPORT_IRREGULAR & g_read_device_info.identify_device_info.status.value) != BD91401GW_STATUS_TYPE_CHGPORT_IRREGULAR)
    {
        
        usbsw_set_otg_workable(USB_VBUS_ON);
        printk(KERN_DEBUG "[USBSW]LINE:%u %s usbsw_set_otg_workable(USB_VBUS_ON) call\n",__LINE__,__func__);
    }
    
    mutex_unlock(&g_bd91401gw_usb_lock);
    
    printk(KERN_DEBUG "LINE:%u %s end  ret:%d \n",__LINE__,__func__,ret);
    return;
}







void bd91401gw_detect_intb(void){
    
    printk(KERN_DEBUG "[USBSW]LINE:%u %s start\n",__LINE__,__func__);
    
    schedule_work(&usbsw_work);
    
    printk(KERN_DEBUG "LINE:%u %s end\n",__LINE__,__func__);
    return;
}







static void bd91401gw_main(struct work_struct* w){
    
    int ret = BD91401GW_NG;
    bd91401gw_reg_type temp_read_area;
    
    int read_id_sta = BD91401GW_IDSTATUS_TYPE_DEFAULT;

    int retry_counter = 0;                        


    temp_read_area.address = BD91401GW_INTERRUPT_ADDRESS;
    temp_read_area.value = 0x00;
    
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    
    mutex_lock(&g_bd91401gw_usb_lock);
    
    
    ret = bd91401gw_read_reg(&temp_read_area);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_main! read_reg error(05)  ret:%d \n",ret);
        mutex_unlock(&g_bd91401gw_usb_lock);
        return;
    }
    
    printk(KERN_INFO "bd91401gw_main reg value 05(first time):0x%02x \n"
            ,temp_read_area.value);
    
    
    ret = bd91401gw_read_reg(&g_read_device_info.identify_device_info.id_status);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_main! read_reg error(07)  ret:%d \n",ret);
        mutex_unlock(&g_bd91401gw_usb_lock);
        return;
    }
    

    
    
    if(g_bd91401gw_audio_notify_flg)
    {
        while((BD91401GW_IDSTATUS_TYPE_WRONG == (BD91401GW_IDSTATUS_MASKB & g_read_device_info.identify_device_info.id_status.value))
          && (retry_counter < 10))
        {
            printk(KERN_DEBUG "LINE:%u %s ID_STATUS BD91401GW_IDSTATUS_TYPE_WRONG retry_counter=%d\n",__LINE__,__func__,retry_counter);
            msleep(BD91401GW_IDSTATUS_REG_READ_SLEEP_TIME);
            
            ret = bd91401gw_read_reg(&g_read_device_info.identify_device_info.id_status);
            if(BD91401GW_OK != ret)
            {
                printk(KERN_ERR "Error in bd91401gw_main! Retry read_reg error(07) ret:%d \n",ret);
                mutex_unlock(&g_bd91401gw_usb_lock);
                return;
            }
            
            
            retry_counter++;
        }
        if(BD91401GW_IDSTATUS_TYPE_WRONG == (BD91401GW_IDSTATUS_MASKB & g_read_device_info.identify_device_info.id_status.value))
        {
            printk(KERN_DEBUG "LINE:%u %s ID_STATUS BD91401GW_IDSTATUS_TYPE_WRONG retry_count Over!\n",__LINE__,__func__);
            mutex_unlock(&g_bd91401gw_usb_lock);
            return;
        }
    }

    
    
    
    
    
    ret = bd91401gw_read_reg(&g_read_device_info.interrupt_info.interrupt);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_main! read_reg error(05)  ret:%d \n",ret);
        mutex_unlock(&g_bd91401gw_usb_lock);
        return;
    }
    
    
    
    
    
    
    
    g_read_device_info.interrupt_info.interrupt.value |= (temp_read_area.value &
                              (BD91401GW_INTERRUPT_TYPE_COMPL | BD91401GW_INTERRUPT_TYPE_COMPH));
    
    
    ret = bd91401gw_read_reg(&g_read_device_info.identify_device_info.status);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_main! read_reg error(06)  ret:%d \n",ret);
        mutex_unlock(&g_bd91401gw_usb_lock);
        return;
    }
    
    
    ret = bd91401gw_read_reg(&g_read_device_info.switch_path_info.control);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_main! read_reg error(03)  ret:%d \n",ret);
        mutex_unlock(&g_bd91401gw_usb_lock);
        return;
    }
    
    printk(KERN_INFO "%s reg value 05:%02x, 06:%02x, 07:%02x, 03:%02x\n"
            ,__func__
            ,g_read_device_info.interrupt_info.interrupt.value
            ,g_read_device_info.identify_device_info.status.value
            ,g_read_device_info.identify_device_info.id_status.value
            ,g_read_device_info.switch_path_info.control.value);
    
    
    ret = bd91401gw_set_switch(&g_write_device_info.switch_path_info.control);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_main! set switch error  ret:%d \n",ret);
        mutex_unlock(&g_bd91401gw_usb_lock);
        return;
    }
    
    printk(KERN_DEBUG "bd91401gw_main setting path value 03:%02x\n"
            ,g_write_device_info.switch_path_info.control.value);
    
    if(g_read_device_info.switch_path_info.control.value != g_write_device_info.switch_path_info.control.value)
    {
        
        printk(KERN_INFO "bd91401gw_main change path! %02x to %02x\n"
            ,g_read_device_info.switch_path_info.control.value
            ,g_write_device_info.switch_path_info.control.value);
        
        if((g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_CHG_EAR)
          || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_EAR)
          || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_EAR_MIC)
          || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_FEED_POW_EAR))
        {
            
            ret = bd91401gw_write_reg(&g_unconnection_path.control);
            if(BD91401GW_OK != ret)
            {
                printk(KERN_ERR "Error in bd91401gw_main! write_reg error(03)  ret:%d \n",ret);
                mutex_unlock(&g_bd91401gw_usb_lock);
                return;
            }
            
            udelay(5);
        }
        
        
        ret = bd91401gw_write_reg(&g_write_device_info.switch_path_info.control);
        if(BD91401GW_OK != ret)
        {
            printk(KERN_ERR "Error in bd91401gw_main! write_reg error(03)  ret:%d \n",ret);
            mutex_unlock(&g_bd91401gw_usb_lock);
            return;
        }
        
        if((g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_CHG_EAR)
          || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_EAR)
          || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_EAR_MIC)
          || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_FEED_POW_EAR))
        {
            
            printk(KERN_DEBUG "LINE:%u %s call bd91401gw_notify_mic(true)\n",__LINE__,__func__);
            bd91401gw_notify_mic(true);
        }
        else
        {
            
            printk(KERN_DEBUG "LINE:%u %s call bd91401gw_notify_mic(false)\n",__LINE__,__func__);
            bd91401gw_notify_mic(false);
        }
    }
    else
    {
        
        printk(KERN_INFO "bd91401gw_main not change path.\n");
        
        
        if(BD91401GW_INTERRUPT_TYPE_COMPL == (g_read_device_info.interrupt_info.interrupt.value & BD91401GW_INTERRUPT_TYPE_COMPL))
        {
            
            read_id_sta = BD91401GW_IDSTATUS_MASKB & g_read_device_info.identify_device_info.id_status.value;
            switch(read_id_sta)
            {
            case BD91401GW_IDSTATUS_TYPE_STEREO_EAR:
            case BD91401GW_IDSTATUS_TYPE_MONO_EAR:
            case BD91401GW_IDSTATUS_TYPE_CHG_EAR:
                bd91401gw_start_sendend_key_polling();
                break;
            default:
                break;
            }
        }
    }
    
    
    bd91401gw_set_device_state();
    

    
    bd91401gw_set_ovpocp_attach_state();
    
    
    if(((BD91401GW_INTERRUPT_TYPE_VBUSDET & g_read_device_info.interrupt_info.interrupt.value) == BD91401GW_INTERRUPT_TYPE_VBUSDET
        || (BD91401GW_STATUS_TYPE_OVPVB & g_read_device_info.identify_device_info.status.value) == BD91401GW_STATUS_TYPE_OVPVB
        || (BD91401GW_STATUS_TYPE_OCPVB & g_read_device_info.identify_device_info.status.value) == BD91401GW_STATUS_TYPE_OCPVB)

        && (BD91401GW_IDSTATUS_MASKB & g_read_device_info.identify_device_info.id_status.value) != BD91401GW_IDSTATUS_TYPE_OTGHOST
        && (BD91401GW_STATUS_TYPE_CHGPORT_IRREGULAR & g_read_device_info.identify_device_info.status.value) != BD91401GW_STATUS_TYPE_CHGPORT_IRREGULAR)
    {

        
        intb_state = USB_VBUS_ON;
        printk(KERN_DEBUG "[USBSW]LINE:%u %s intb_state:%d\n",__LINE__,__func__,intb_state);
		printk(KERN_DEBUG "[USBSW]LINE:%u %s schedule_delayed_work(&set_otg_work,USB_CHG_DET_DELAY) call\n",__LINE__,__func__);
        schedule_delayed_work(&set_otg_work,USB_CHG_DET_DELAY);


        
        wake_lock_timeout(&g_usbsw_wlock, (WLOCK_TIMEOUT_SEC * HZ) );
        printk(KERN_DEBUG "[USBSW]LINE:%u %s wake_lock_timeout() call\n",__LINE__,__func__);
    }
    else if((BD91401GW_INTERRUPT_TYPE_VBUSDET & g_read_device_info.interrupt_info.interrupt.value) != BD91401GW_INTERRUPT_TYPE_VBUSDET

        && (BD91401GW_STATUS_TYPE_OVPVB & g_read_device_info.identify_device_info.status.value) != BD91401GW_STATUS_TYPE_OVPVB
        && (BD91401GW_STATUS_TYPE_OCPVB & g_read_device_info.identify_device_info.status.value) != BD91401GW_STATUS_TYPE_OCPVB)

    {

        
        intb_state = USB_VBUS_OFF;
        printk(KERN_DEBUG "[USBSW]LINE:%u %s intb_state:%d\n",__LINE__,__func__,intb_state);
        printk(KERN_DEBUG "[USBSW]LINE:%u %s schedule_delayed_work(&set_otg_work,USB_CHG_DET_DELAY) call\n",__LINE__,__func__);
        schedule_delayed_work(&set_otg_work,USB_CHG_DET_DELAY);


        
        wake_lock_timeout(&g_usbsw_wlock, (WLOCK_TIMEOUT_SEC * HZ) );
        printk(KERN_DEBUG "[USBSW]LINE:%u %s wake_lock_timeout() call\n",__LINE__,__func__);
    }
    
    
    if((BD91401GW_STATUS_TYPE_OVPVB & g_read_device_info.identify_device_info.status.value) == BD91401GW_STATUS_TYPE_OVPVB)
    {
        
        wake_lock_timeout(&g_usbsw_wlock, (OVP_WLOCK_TIMEOUT_SEC * HZ) );
    }
    
    
    if((BD91401GW_INTERRUPT_TYPE_COMPH & g_read_device_info.interrupt_info.interrupt.value) == BD91401GW_INTERRUPT_TYPE_COMPH)
    {

        
        if(BD91401GW_IDSTATUS_TYPE_DEFAULT != (BD91401GW_IDSTATUS_MASKB & g_read_device_info.identify_device_info.id_status.value))
        {
            
            printk(KERN_DEBUG "LINE:%u %s COMPH WRONG DETECTION! \n",__LINE__,__func__);
        }
        else
        {
            
            if(g_bd91401gw_mic_flg)
            {
 






 
                g_bd91401gw_mic_flg = false;
            }
            
            printk(KERN_DEBUG "LINE:%u %s call bd91401gw_notify_mic(false)\n",__LINE__,__func__);
            bd91401gw_notify_mic(false);
            g_bd91401gw_mic_detect = false;
            bd91401gw_stop_sendend_key_polling();
        }
    }

    
    mutex_unlock(&g_bd91401gw_usb_lock);
    
    printk(KERN_DEBUG "LINE:%u %s end  ret:%d \n",__LINE__,__func__,ret);
    return;
}








int  bd91401gw_set_switch(bd91401gw_reg_type* write_reg_p)
{
    
    bool bret = false;
    unsigned char read_id = BD91401GW_IDSTATUS_MASKB & g_read_device_info.identify_device_info.id_status.value;
    unsigned char read_vbusdet = BD91401GW_INTERRUPT_TYPE_VBUSDET & g_read_device_info.interrupt_info.interrupt.value;
    unsigned char read_ovpvb = BD91401GW_STATUS_TYPE_OVPVB & g_read_device_info.identify_device_info.status.value;
    
    printk(KERN_DEBUG "[USBSW]LINE:%u %s start\n",__LINE__,__func__);
    printk(KERN_DEBUG "[USBSW]LINE:%u %s ID:%02x, VBUSDET:%02x, OVPVB:%02x\n",__LINE__,__func__, read_id, read_vbusdet, read_ovpvb);
    
    if(write_reg_p == NULL)
    {
        printk(KERN_ERR "Error in bd91401gw_set_switch! write_reg_p == NULL\n");
        return BD91401GW_NG;
    }
    
    
    
    if(BD91401GW_IDSTATUS_TYPE_CHG_EAR == read_id)
    {
        
        bret = bd91401gw_control_mic();
        if(bret == false)
        {
            printk(KERN_ERR "[USBSW]LINE:%u %s Error in bd91401gw_set_switch! mic check error\n",__LINE__,__func__);
            return BD91401GW_NG;
        }
        
        write_reg_p->value = BD91401GW_SWCONTROL_CHG_EAR;
    }
    
    else if((BD91401GW_IDSTATUS_TYPE_STEREO_EAR  == read_id)
          || (BD91401GW_IDSTATUS_TYPE_MONO_EAR   == read_id))
    {

        if(BD91401GW_STATUS_TYPE_OVPVB == read_ovpvb)
        {

            g_bd91401gw_mic_detect = false;


            
            bd91401gw_set_audio_info(read_id, false);


            
            write_reg_p->value = BD91401GW_SWCONTROL_FEED_POW_EAR;
        }

        else if(BD91401GW_INTERRUPT_TYPE_VBUSDET == read_vbusdet)
        {
            
            bret = bd91401gw_control_mic();
            if(bret == false)
            {
                printk(KERN_ERR "[USBSW]LINE:%u %s Error in bd91401gw_set_switch! mic check error\n",__LINE__,__func__);
                return BD91401GW_NG;
            }
            
            write_reg_p->value = BD91401GW_SWCONTROL_FEED_POW_EAR;

        }
        else
        {
            
            bret = bd91401gw_control_mic();
            if(bret == false)
            {
                printk(KERN_ERR "[USBSW]LINE:%u %s Error in bd91401gw_set_switch! mic check error\n",__LINE__,__func__);
                return BD91401GW_NG;
            }
            if(g_bd91401gw_mic_flg)
            {
                
                
                write_reg_p->value = BD91401GW_SWCONTROL_EAR_MIC;
            }
            else
            {
                
                
                write_reg_p->value = BD91401GW_SWCONTROL_EAR;
            }
        }
    }
    
    else if(BD91401GW_IDSTATUS_TYPE_HOOKSW == read_id)
    {
        
        write_reg_p->value = g_read_device_info.switch_path_info.control.value;
    }
    
    else if(BD91401GW_IDSTATUS_TYPE_UART == read_id) 
    {
        
        write_reg_p->value = BD91401GW_SWCONTROL_UART;
    }
    
    else
    {
        
        write_reg_p->value = BD91401GW_SWCONTROL_DEFAULT;
    }
    
    printk(KERN_DEBUG "[USBSW]LINE:%u %s end\n",__LINE__,__func__);
    return BD91401GW_OK;
}








int  bd91401gw_read_reg(bd91401gw_reg_type* reg_p){
    
    int ret = BD91401GW_I2C_TRANSFER_NG;    
    
    if(reg_p == NULL )
    {
        printk(KERN_ERR "[USBSW]LINE:%u %s Error in bd91401gw_read_reg! reg_p == NULL\n",__LINE__,__func__);
        return BD91401GW_NG;
    }
    else
    {
        
        ret = bd91401gw_i2c_read( reg_p->address , &(reg_p->value) );
        if(ret != BD91401GW_I2C_TRANSFER_OK)
        {
            return BD91401GW_NG;
        }
        printk(KERN_DEBUG "[USBSW]LINE:%u %s SUCCESS address=0x%02x value=0x%02x\n",__LINE__,__func__, reg_p->address, reg_p->value);
    }
    
    return BD91401GW_OK;
}








int  bd91401gw_write_reg(bd91401gw_reg_type* reg_p){
    
    int ret = BD91401GW_I2C_TRANSFER_NG;      
    
    if(reg_p == NULL )
    {
        printk(KERN_ERR "[USBSW]LINE:%u %s Error in bd91401gw_write_reg! reg_p == NULL\n",__LINE__,__func__ );
        return BD91401GW_NG;
    }
    else
    {
        
        ret = bd91401gw_i2c_write( reg_p->address , reg_p->value );
        if(ret != BD91401GW_I2C_TRANSFER_OK)
        {
            return BD91401GW_NG;
        }
        printk(KERN_DEBUG "[USBSW]LINE:%u %s SUCCESS address=0x%02x value=0x%02x\n",__LINE__,__func__, reg_p->address, reg_p->value);
    }
    
    return BD91401GW_OK;
}








static irqreturn_t bd91401gw_irq_handler(int irq, void* dev_id){
    
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    
    disable_irq_nosync(g_bd91401gw_i2c_modules.client->irq);
    
    
    bd91401gw_detect_intb( );
    
    enable_irq(g_bd91401gw_i2c_modules.client->irq);
    
    printk(KERN_DEBUG "LINE:%u %s end\n",__LINE__,__func__);
    return IRQ_HANDLED;
}








static u32 bd91401gw_irq_setup(struct i2c_client* client){
    
    int ret;
    if(client == NULL)
    {
        printk(KERN_ERR "Error in bd91401gw_irq_setup! client == NULL\n" );
        return BD91401GW_NG;
    }
    
    printk(KERN_DEBUG "LINE:%u %s start irq=%d\n",__LINE__,__func__,client->irq);
    
    ret = request_threaded_irq(client->irq,
                               NULL,
                               bd91401gw_irq_handler,
                               IRQF_TRIGGER_FALLING,
                               BD91401GW_I2C_DEVICE_NAME,
                               NULL);
    
    printk(KERN_DEBUG "LINE:%u %s end     ret:%d\n",__LINE__,__func__,ret);
    return ret;
}









static int __devinit bd91401gw_i2c_probe(struct i2c_client* client , const struct i2c_device_id* id){
    
    struct bd91401gw_platform_data_struct* pdata_p;    
    int ret;
    
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    
    
    if ( NULL == client ) {
        printk(KERN_ERR "BD91401GW I2C Probe(ERROR can't attach client)\n" );
        return -ENOMEM;
    }
    
    g_bd91401gw_i2c_modules.client = client;
    
    pdata_p = client->dev.platform_data;
    
    
    if ( pdata_p == NULL ) {
        printk( "BD91401GW I2C Probe(ERROR 01)\n" ) ;
        return -EINVAL;
    }
    
    
    ret = i2c_check_functionality( client->adapter , I2C_FUNC_I2C );
    if ( !ret ) {
        printk(KERN_ERR "BD91401GW I2C Probe(ERROR can't talk I2C)\n" );
        return -EIO;
    }
    
    
    if ( true == g_bd91401gw_i2c_modules.inuse ) {
        printk(KERN_ERR "BD91401GW I2C Probe(ERROR driver already in use)\n" );
        return -EBUSY;
    }
    
    
    
    g_bd91401gw_i2c_modules.inuse = true;
    




    
    
    gpio_set_value( BD91401GW_MSM_GPIO_USBSW_RESET_ID, BD91401GW_MSM_GPIO_HIGH_VALUE );

    


    

    if ( pdata_p->bd91401gw_setup != NULL ) {
        pdata_p->bd91401gw_setup();
    }
    
    
    bd91401gw_init();
    
    
    ret = bd91401gw_irq_setup(client);
    if(0 != ret){
        printk(KERN_ERR "BD91401GW I2C Probe(ERROR can't setup)\n" );
        return -EFAULT;
    }
    
    enable_irq_wake(client->irq);
    
    bd91401gw_path_initialize();
    
    printk(KERN_DEBUG "LINE:%u %s end\n",__LINE__,__func__);
    return 0;
}







static int __devexit bd91401gw_i2c_remove(struct i2c_client* client){
    
    g_bd91401gw_i2c_modules.inuse = false;              
    
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    
    i2c_release_client(client);
    printk(KERN_DEBUG "LINE:%u %s end\n",__LINE__,__func__);
    return 0;
}







static int __init bd91401gw_i2c_init(void){
    
    int ret;
    g_bd91401gw_i2c_modules.inuse = false;              
    
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    printk(KERN_DEBUG "LINE:%u %s end\n",__LINE__,__func__);
    
    ret = i2c_add_driver( &g_bd91401gw_i2c_driver );
    return ret;
}







static void __exit bd91401gw_i2c_exit(void){
    
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    
    i2c_del_driver( &g_bd91401gw_i2c_driver );
    
    printk(KERN_DEBUG "LINE:%u %s end\n",__LINE__,__func__);
    return;
}












static int bd91401gw_pack_i2c_info(
    int i2c_ret_val,
    unsigned char i2c_data_len,
    unsigned char i2c_data[2],
    unsigned char i2c_format,
    unsigned int* i2c_alarm_info ){
    
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    
    
    if( i2c_alarm_info == NULL )
    {
        printk(KERN_ERR "Error in bd91401gw_pack_i2c_info! i2c_alarm_info == NULL\n" );
        return BD91401GW_I2C_TRANSFER_NG;
    }
    
    
    *i2c_alarm_info = 0;
    
    
    *i2c_alarm_info = *i2c_alarm_info | (~i2c_ret_val + 1) << 16; 
    
    
    if( i2c_data_len == 0 ){
        *i2c_alarm_info = *i2c_alarm_info | 0x00;                   
        *i2c_alarm_info = *i2c_alarm_info | 0x00 << 8;              
        *i2c_alarm_info = *i2c_alarm_info | BD91401GW_I2C_DATA_DISABLE << 24;   
    }else if( i2c_data_len == 1 ){
        *i2c_alarm_info = *i2c_alarm_info | 0x00;                   
        *i2c_alarm_info = *i2c_alarm_info | i2c_data[0] << 8;       
        *i2c_alarm_info = *i2c_alarm_info | BD91401GW_I2C_DATA_ENABLE_1BYTE << 24;  
    }else{
        *i2c_alarm_info = *i2c_alarm_info | i2c_data[1];            
        *i2c_alarm_info = *i2c_alarm_info | i2c_data[0] << 8;       
        *i2c_alarm_info = *i2c_alarm_info | BD91401GW_I2C_DATA_ENABLE_2BYTE << 24;  
    }
    
    
    if( i2c_format == BD91401GW_I2C_COM_FORMAT_SEND ){
        *i2c_alarm_info = *i2c_alarm_info | BD91401GW_I2C_COM_FORMAT_SEND << 24; 
    }else if( i2c_format == BD91401GW_I2C_COM_FORMAT_RECV ){
        *i2c_alarm_info = *i2c_alarm_info | BD91401GW_I2C_COM_FORMAT_RECV << 24; 
    }else{
        
    }
    
    printk(KERN_DEBUG "LINE:%u %s end\n",__LINE__,__func__);
    
    return BD91401GW_I2C_TRANSFER_OK;
}









int bd91401gw_i2c_read(unsigned char reg , unsigned char* data){
    
    int ret = BD91401GW_I2C_TRANSFER_NG;          
    int i2c_ret = 0;                              
    int retry_counter = 0;                        
    unsigned int  i2c_alarm_info;                 
    unsigned char i2c_data[2];
    struct i2c_msg msgs[] =                       
    {
        {   
            .addr  = BD91401GW_I2C_SLAVE_ADDRESS, 
            .flags = 0,                           
            .len   = BD91401GW_I2C_READ_LEN,      
            .buf   = &reg,                        
        },
        {   
            .addr  = BD91401GW_I2C_SLAVE_ADDRESS, 
            .flags = I2C_M_RD,                    
            .len   = BD91401GW_I2C_READ_LEN,      
            .buf   = data,                        
        },                                      
    };
    
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    
    i2c_ret = i2c_transfer(g_bd91401gw_i2c_modules.client->adapter, msgs, BD91401GW_I2C_MSG_READ_NUMBER );
    
    
    if ( i2c_ret < 0 )
    {
        
        i2c_data[0] = *data;
        i2c_data[1] = 0x00;
        
        ret = bd91401gw_pack_i2c_info( 
            i2c_ret,
            BD91401GW_I2C_READ_LEN,
            i2c_data,
            BD91401GW_I2C_COM_FORMAT_RECV,
            &i2c_alarm_info
        );
        if( ret == BD91401GW_I2C_TRANSFER_NG )
        {
            
            printk(KERN_INFO "Error in bd91401gw_i2c_read! bd91401gw_pack_i2c_info() packing error \n");
        }
        

        
        printk(KERN_ERR "Error in bd91401gw_i2c_read! Read failed!\n");

        
        
        while( ( i2c_ret < 0 ) && (retry_counter < BD91401GW_RETRY_READ_REG_COUNT) )
        {
            msleep(BD91401GW_I2C_READ_SLEEP_TIME);
            
            
            i2c_ret = i2c_transfer(g_bd91401gw_i2c_modules.client->adapter, msgs, BD91401GW_I2C_MSG_READ_NUMBER );
            
            
            retry_counter++;
        }
        
        
        if( i2c_ret < 0 )
        {
            
            i2c_data[0] = *data;
            i2c_data[1] = 0x00;
            
            ret = bd91401gw_pack_i2c_info(
                i2c_ret,
                BD91401GW_I2C_READ_LEN,
                i2c_data,
                BD91401GW_I2C_COM_FORMAT_RECV,
                &i2c_alarm_info
            );
            if( ret == BD91401GW_I2C_TRANSFER_NG )
            {
                
                printk(KERN_INFO "Error in bd91401gw_i2c_read! bd91401gw_pack_i2c_info() packing error \n");
            }
            
            
            printk(KERN_ERR "Error in bd91401gw_i2c_read! Read retry failed!  ret:%d \n",ret);
            printk(KERN_ERR "[T][ARM]Event:0x%02X Info:0x%02X\n", BD91401GW_I2C_ALERM_EVENT, i2c_alarm_info);
        }
        else
        {
            ret = BD91401GW_I2C_TRANSFER_OK;
        }
    }
    else
    {
       ret = BD91401GW_I2C_TRANSFER_OK;
    }
    
    printk(KERN_DEBUG "LINE:%u %s end     ret:%d\n",__LINE__,__func__,ret);
    
    return ret;
}









int bd91401gw_i2c_write(unsigned char reg , unsigned char value){
    
    int ret = BD91401GW_I2C_TRANSFER_NG;   
    int i2c_ret = 0;                              
    unsigned char data[2];                 
    int retry_counter = 0;                        
    unsigned int  i2c_alarm_info;        
    struct i2c_msg msg[] =                 
    {
        {
            .addr  = BD91401GW_I2C_SLAVE_ADDRESS,   
            .flags = 0,                             
            .len   = BD91401GW_I2C_WRITE_LEN,       
            .buf   = data,                          
        },
    };
    
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    
    data[0] = reg;
    data[1] = value;
    
    i2c_ret = i2c_transfer(g_bd91401gw_i2c_modules.client->adapter, msg, BD91401GW_I2C_MSG_WRITE_NUMBER );
    
    
    if ( i2c_ret < 0 )
    {
        
        ret = bd91401gw_pack_i2c_info( 
            i2c_ret,
            BD91401GW_I2C_WRITE_LEN,
            data,
            BD91401GW_I2C_COM_FORMAT_SEND,
            &i2c_alarm_info
        );
        if( ret == BD91401GW_I2C_TRANSFER_NG )
        {
            
            printk(KERN_INFO "Error in bd91401gw_i2c_write! bd91401gw_pack_i2c_info() packing error \n");
        }
        

        
        printk(KERN_ERR "Error in bd91401gw_i2c_write! Write failed!\n");
      
        
        
        while( ( i2c_ret < 0 ) && (retry_counter < BD91401GW_RETRY_WRITE_REG_COUNT) )
        {
            msleep(BD91401GW_I2C_READ_SLEEP_TIME);
            
            
            i2c_ret = i2c_transfer(g_bd91401gw_i2c_modules.client->adapter, msg, BD91401GW_I2C_MSG_WRITE_NUMBER );
            
            
            retry_counter++;
        }
        
        
        if( i2c_ret < 0 )
        {
            
            ret = bd91401gw_pack_i2c_info( 
                i2c_ret,
                BD91401GW_I2C_WRITE_LEN,
                data,
                BD91401GW_I2C_COM_FORMAT_SEND,
                &i2c_alarm_info
            );
            if( ret == BD91401GW_I2C_TRANSFER_NG )
            {
            
                printk(KERN_INFO "Error in bd91401gw_i2c_write! bd91401gw_pack_i2c_info() packing error \n");
            }
            
            
            printk(KERN_ERR "Error in bd91401gw_i2c_write! Write retry failed!  ret:%d \n",ret);
            printk(KERN_ERR "[T][ARM]Event:0x%02X Info:0x%02X\n", BD91401GW_I2C_ALERM_EVENT, i2c_alarm_info);
        }
        else
        {
            ret = BD91401GW_I2C_TRANSFER_OK;
        }
    }
    else
    {
       ret = BD91401GW_I2C_TRANSFER_OK;
    }
    
    printk(KERN_DEBUG "LINE:%u %s end     ret:%d\n",__LINE__,__func__,ret);
    
    return ret;
}








static bool bd91401gw_control_mic(void){
    
    int ret = BD91401GW_NG;
    
    unsigned char read_id_sta = BD91401GW_IDSTATUS_TYPE_DEFAULT;
    
    unsigned char read_status = BD91401GW_INTERRUPT_TYPE_DEFAULT;
    
    printk(KERN_DEBUG "[USBSW]LINE:%u %s start\n",__LINE__,__func__);
    
    
    if(g_bd91401gw_mic_detect)
    {
        
        printk(KERN_DEBUG "[USBSW]LINE:%u %s already mic detect.\n",__LINE__,__func__);
        return true;
    }
    
    
    read_id_sta = BD91401GW_IDSTATUS_MASKB & g_read_device_info.identify_device_info.id_status.value;
    

    read_status = BD91401GW_INTERRUPT_TYPE_VBUSDET & g_read_device_info.interrupt_info.interrupt.value; 
    

    if((read_id_sta == BD91401GW_IDSTATUS_TYPE_STEREO_EAR) || (read_id_sta == BD91401GW_IDSTATUS_TYPE_MONO_EAR))
    {
        
        
        if(read_status == BD91401GW_INTERRUPT_TYPE_DEFAULT)
        {
            
            
            
            ret = bd91401gw_write_reg(&g_mic_path.control);
            if(BD91401GW_OK != ret)
            {
                printk(KERN_ERR "[USBSW]LINE:%u %s Error! set switch error ret:0x%02x \n",__LINE__,__func__,ret);
                return false;
            }


            
            ret = hedset_detect_irq(AUDIOIF_DEVICE_USB, AUDIOIF_IRQ_INSERT);
            
            if((ret == AUDIOIF_E_NG) || (ret == AUDIOIF_E_DEVICE))
            {
                
                printk(KERN_ERR "[USBSW]LINE:%u %s Error! hedset_detect_irq ret is:0x%02x \n",__LINE__,__func__,ret);
            }
            
            else if(ret == AUDIOIF_E_OK_BEFORESTART)
            {
                
                disable_irq_nosync(g_bd91401gw_i2c_modules.client->irq);
                
                
                ret = bd91401gw_write_reg(&g_unconnection_path.control);
                if(BD91401GW_OK != ret)
                {
                    printk(KERN_ERR "[USBSW]LINE:%u %s Error! write_reg(03) ret:%d \n",__LINE__,__func__,ret);
                    return false;
                }
                
                printk(KERN_DEBUG "[USBSW]LINE:%u %s Stop to initialize path setting\n",__LINE__,__func__);
                return false;
            }

            ret = hedset_detect_mictype(AUDIOIF_DEVICE_USB);
            if(AUDIOIF_E_DEVICE == ret)
            {
                printk(KERN_ERR "[USBSW]LINE:%u %s Error! hedset_detect_mictype ret is:0x%02x \n",__LINE__,__func__,ret);
            }
            g_bd91401gw_mic_flg = ret;

        }
        else
        {
        


            
            ret = hedset_detect_irq(AUDIOIF_DEVICE_USB, AUDIOIF_IRQ_INSERT_NOMIC);
            
            if((ret == AUDIOIF_E_NG) || (ret == AUDIOIF_E_DEVICE))
            {
                
                printk(KERN_ERR "[USBSW]LINE:%u %s Error! hedset_detect_irq ret is:0x%02x \n",__LINE__,__func__,ret);
            }
            
            else if(ret == AUDIOIF_E_OK_BEFORESTART)
            {
                
                disable_irq_nosync(g_bd91401gw_i2c_modules.client->irq);
                
                
                ret = bd91401gw_write_reg(&g_unconnection_path.control);
                if(BD91401GW_OK != ret)
                {
                    printk(KERN_ERR "[USBSW]LINE:%u %s Error! write_reg(03) ret:%d \n",__LINE__,__func__,ret);
                    return false;
                }
                
                printk(KERN_DEBUG "[USBSW]LINE:%u %s Stop to initialize path setting\n",__LINE__,__func__);
                return false;
            }

            ret = hedset_detect_mictype(AUDIOIF_DEVICE_USB);
            if(AUDIOIF_E_DEVICE == ret)
            {
                printk(KERN_ERR "[USBSW]LINE:%u %s Error! hedset_detect_mictype ret is:0x%02x \n",__LINE__,__func__,ret);
            }
            g_bd91401gw_mic_flg = ret;

        }
    }
    else if(read_id_sta == BD91401GW_IDSTATUS_TYPE_CHG_EAR)
    {
        
        
        ret = bd91401gw_write_reg(&g_mic_path_mode3.control);
        if(BD91401GW_OK != ret)
        {
            printk(KERN_ERR "Error! set switch error  ret:0x%02x \n",ret);
            return false;
        }
;
        printk(KERN_DEBUG "LINE:%u %s CHGPORT DET Wait(300ms)\n",__LINE__,__func__);
        
        
        
        msleep(300);
;
;

        
        ret = hedset_detect_irq(AUDIOIF_DEVICE_USB, AUDIOIF_IRQ_INSERT);
        
        if((ret == AUDIOIF_E_NG) || (ret == AUDIOIF_E_DEVICE))
        {
            
            printk(KERN_ERR "[USBSW]LINE:%u %s Error! hedset_detect_irq ret is:0x%02x \n",__LINE__,__func__,ret);
        }
        
        else if(ret == AUDIOIF_E_OK_BEFORESTART)
        {
            
            disable_irq_nosync(g_bd91401gw_i2c_modules.client->irq);
            
            
            ret = bd91401gw_write_reg(&g_unconnection_path.control);
            if(BD91401GW_OK != ret)
            {
                printk(KERN_ERR "[USBSW]LINE:%u %s Error! write_reg(03) ret:%d \n",__LINE__,__func__,ret);
                return false;
            }
            
            printk(KERN_DEBUG "[USBSW]LINE:%u %s Stop to initialize path setting\n",__LINE__,__func__);
            return false;
        }

        ret = hedset_detect_mictype(AUDIOIF_DEVICE_USB);
        if(AUDIOIF_E_DEVICE == ret)
        {
            printk(KERN_ERR "[USBSW]LINE:%u %s Error! hedset_detect_mictype ret is:0x%02x \n",__LINE__,__func__,ret);
        }
        g_bd91401gw_mic_flg = ret;

    }
    else{
        
        
        
        
        
        
        bd91401gw_set_audio_info(read_id_sta,false);
        return false;
    }
    
    
    if(!g_bd91401gw_mic_flg)
    {
    
    }
    g_bd91401gw_mic_detect = true;
    
    
    bd91401gw_set_audio_info(read_id_sta,g_bd91401gw_mic_flg);
    printk(KERN_DEBUG "[USBSW]LINE:%u %s end\n",__LINE__,__func__);
    return true;
}








static void bd91401gw_set_audio_info(unsigned char mode_number, bool mic_flg){
    
    

    int audio_state = BD91401GW_IDSTATUS_TYPE_DEFAULT;

    
    printk(KERN_DEBUG "[USBSW]LINE:%u %s start\n",__LINE__,__func__);
    printk(KERN_DEBUG "[USBSW]LINE:%u %s: mode_number=0x%02x, mic_flg=0x%02x\n",__LINE__,__func__, mode_number, mic_flg);
    switch(mode_number)
    {
    
    case BD91401GW_IDSTATUS_TYPE_STEREO_EAR:
        
        g_usb_headset_info.stereo_mono_type = USB_HEADSET_STEREO;
        printk(KERN_DEBUG "LINE:%u %s USB_HEADSET_STEREO\n",__LINE__,__func__);
        if( mic_flg )
        {
            

            audio_state = AUDIOIF_IRQ_HS_STEREO_USE;

            printk(KERN_DEBUG "LINE:%u %s AUDIOIF_IRQ_HS_STEREO_USE\n",__LINE__,__func__);
        }
        else
        {
            

            audio_state = AUDIOIF_IRQ_HS_STEREO_NONE;

            printk(KERN_DEBUG "LINE:%u %s AUDIOIF_IRQ_HS_STEREO_NONE\n",__LINE__,__func__);
        }
        break;
    
    case BD91401GW_IDSTATUS_TYPE_MONO_EAR:
    
    case BD91401GW_IDSTATUS_TYPE_CHG_EAR:
        
        g_usb_headset_info.stereo_mono_type = USB_HEADSET_MONO;
        printk(KERN_DEBUG "LINE:%u %s USB_HEADSET_MONO\n",__LINE__,__func__);
        if( mic_flg )
        {
            

            audio_state = AUDIOIF_IRQ_HS_MONO_USE;

            printk(KERN_DEBUG "LINE:%u %s AUDIOIF_IRQ_HS_MONO_USE\n",__LINE__,__func__);
        }
        else
        {
            

            audio_state = AUDIOIF_IRQ_HS_MONO_NONE;

            printk(KERN_DEBUG "LINE:%u %s AUDIOIF_IRQ_HS_MONO_NONE\n",__LINE__,__func__);
        }
        break;
    case BD91401GW_IDSTATUS_TYPE_DEFAULT:
    default:
        
        
        printk(KERN_DEBUG "LINE:%u %s USBAUDIO\n",__LINE__,__func__);
        break;
    }
    
    

    g_bd91401gw_audio_state = audio_state;
    printk(KERN_DEBUG "[USBSW]LINE:%u %s g_bd91401gw_audio_state:0x%02x \n",__LINE__,__func__,g_bd91401gw_audio_state);
    printk(KERN_DEBUG "[USBSW]LINE:%u %s end\n",__LINE__,__func__);

}







static void bd91401gw_notify_mic(bool notify_flg){
    
    int ret = 0;
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    printk(KERN_DEBUG "%s: notify_flg=0x%02x, audio_notify_flg=0x%02x\n", __func__, notify_flg, g_bd91401gw_audio_notify_flg);
    
    if(notify_flg)
    {
        if(g_bd91401gw_audio_notify_flg)
        {
            
            
        }
        else
        {
            

            ret = hedset_detect_irq(AUDIOIF_DEVICE_USB, g_bd91401gw_audio_state);
            if(AUDIOIF_E_OK != ret)
            {
                printk(KERN_ERR "[USBSW]LINE:%u %s Error in bd91401gw_notify_mic! hedset_detect_irq ret is:0x%02x \n",__LINE__,__func__,ret);
            }

            g_bd91401gw_audio_notify_flg = true;
        }
    }
    
    else
    {
        if(g_bd91401gw_audio_notify_flg)
        {
            

            ret = hedset_detect_irq(AUDIOIF_DEVICE_USB, AUDIOIF_IRQ_REMOVE);
            if(AUDIOIF_E_OK != ret)
            {
                printk(KERN_ERR "[USBSW]LINE:%u %s Error in bd91401gw_notify_mic! hedset_detect_irq ret is:0x%02x \n",__LINE__,__func__,ret);
            }

            g_bd91401gw_audio_notify_flg = false;
        }
        else
        {
            
            
        }
        
        bd91401gw_set_audio_info(BD91401GW_IDSTATUS_TYPE_DEFAULT,false);
    }
}







void bd91401gw_notify_sendend_key(bool notify_flg){
    
    int ret = AUDIOIF_E_NG;
    
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    printk(KERN_DEBUG "%s: notify_flg=0x%02x, sendend_notify_flg=0x%02x\n", __func__, notify_flg, g_bd91401gw_sendend_notify_flg);
    
    if(notify_flg)
    {
        
        if(g_bd91401gw_sendend_notify_flg)
        {
            
            
        }
        else
        {

            ret = hedset_detect_irq(AUDIOIF_DEVICE_USB,AUDIOIF_IRQ_MIC_ON);
            if(AUDIOIF_E_OK != ret)
            {
                printk(KERN_ERR "[USBSW]LINE:%u %s Error in bd91401gw_notify_sendend_key! hedset_detect_irq ret is:0x%02x \n",__LINE__,__func__,ret);
            }

            g_bd91401gw_sendend_notify_flg = true;
        }
    }
    else
    {
        
        if(g_bd91401gw_sendend_notify_flg)
        {

            ret = hedset_detect_irq(AUDIOIF_DEVICE_USB,AUDIOIF_IRQ_MIC_OFF);
            if(AUDIOIF_E_OK != ret)
            {
                printk(KERN_ERR "[USBSW]LINE:%u %s Error in bd91401gw_notify_sendend_key! hedset_detect_irq ret is:0x%02x \n",__LINE__,__func__,ret);
            }

            g_bd91401gw_sendend_notify_flg = false;
        }
        else
        {
            
            
        }
    }
}







void bd91401gw_start_sendend_key_polling(void){
    
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    
    
    if(g_sendend_key_polling_started_flg == true)
    {
        
        printk(KERN_DEBUG "LINE:%u %s polling started\n",__LINE__,__func__);
        return;
    }
    
    g_sendend_key_polling_started_flg = true;
    
    
    schedule_delayed_work(&sendend_key_polling_work, BD91401GW_SENDEND_KEY_POLLING_INTERVAL_TIME);
    
    
    bd91401gw_notify_sendend_key(true);
    printk(KERN_DEBUG "LINE:%u %s end\n",__LINE__,__func__);
}







void bd91401gw_sendend_key_polling_handler(struct work_struct *w){
    
    int ret = BD91401GW_NG;
    bd91401gw_reg_type temp_read_area;
    unsigned char read_compl = BD91401GW_INTERRUPT_TYPE_COMPL;
    
    temp_read_area.address = BD91401GW_INTERRUPT_ADDRESS;
    temp_read_area.value = 0x00;
    
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    mutex_lock(&g_bd91401gw_usb_sendend_key_polling_lock);
    
    
    if(g_sendend_key_polling_started_flg == false)
    {
        
        printk(KERN_DEBUG "LINE:%u %s not polling started\n",__LINE__,__func__);
        mutex_unlock(&g_bd91401gw_usb_sendend_key_polling_lock);
        return;
    }
    
    
    ret = bd91401gw_read_reg(&temp_read_area);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_sendend_key_polling_handler! read_reg error(05)  ret:%d \n",ret);
        mutex_unlock(&g_bd91401gw_usb_sendend_key_polling_lock);
        return;
    }
    read_compl = BD91401GW_INTERRUPT_TYPE_COMPL & temp_read_area.value;
    printk(KERN_DEBUG "INT_STA(05h):%02x, COMPL:%02x\n", temp_read_area.value, read_compl);
    
    if(read_compl == BD91401GW_INTERRUPT_TYPE_COMPL) {
        
        schedule_delayed_work(&sendend_key_polling_work, BD91401GW_SENDEND_KEY_POLLING_INTERVAL_TIME);
    } else {
        
        bd91401gw_stop_sendend_key_polling();
    }
    
    mutex_unlock(&g_bd91401gw_usb_sendend_key_polling_lock);
    printk(KERN_DEBUG "LINE:%u %s end\n",__LINE__,__func__);
}







void bd91401gw_stop_sendend_key_polling(void){
    
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    
    
    if(g_sendend_key_polling_started_flg == false)
    {
        
        printk(KERN_DEBUG "LINE:%u %s not polling started\n",__LINE__,__func__);
        return;
    }
    
    
    g_sendend_key_polling_started_flg = false;
    
    
    bd91401gw_notify_sendend_key(false);
    printk(KERN_DEBUG "LINE:%u %s end\n",__LINE__,__func__);
}





























enum chg_type bd91401gw_get_chg_type(void){
    
    enum chg_type temp = USB_CHG_TYPE__INVALID;
    unsigned char read_chgport = BD91401GW_STATUS_CHGPORT_MASKB & g_read_device_info.identify_device_info.status.value;
    unsigned char read_id = BD91401GW_IDSTATUS_MASKB & g_read_device_info.identify_device_info.id_status.value;
    printk(KERN_DEBUG "LINE:%u %s start\n",__LINE__,__func__);
    
    printk(KERN_DEBUG "%s: chgport=0x%02x, id=0x%02x\n", __func__, read_chgport, read_id);
    switch(read_chgport)
    {
    case BD91401GW_STATUS_TYPE_CHGPORT_NOUSB:
        printk(KERN_DEBUG "%s: No USB Port.\n", __func__);
        temp = USB_CHG_TYPE__WALLCHARGER;
        break;
    case BD91401GW_STATUS_TYPE_CHGPORT_SDP:
        if(BD91401GW_IDSTATUS_TYPE_DEFAULT == read_id)
        {
            printk(KERN_DEBUG "%s: SDP.\n", __func__);
            temp = USB_CHG_TYPE__SDP;
        }
        else
        {
            printk(KERN_DEBUG "%s: DCP.\n", __func__);
            temp = USB_CHG_TYPE__WALLCHARGER;
        }
        break;
    case BD91401GW_STATUS_TYPE_CHGPORT_CDP:
        printk(KERN_DEBUG "%s: CDP.\n", __func__);
        temp = USB_CHG_TYPE__WALLCHARGER;
        break;
    case BD91401GW_STATUS_TYPE_CHGPORT_DCP:
        printk(KERN_DEBUG "%s: DCP.\n", __func__);
        temp = USB_CHG_TYPE__WALLCHARGER;
        break;
    default:
        printk(KERN_DEBUG "%s: Irregular.\n", __func__);
        temp = USB_CHG_TYPE__WALLCHARGER;
        break;
    }
    
    printk(KERN_DEBUG "LINE:%u %s end chg_type=%d\n",__LINE__,__func__,temp);
    return temp;
}

 










































 














int bd91401gw_get_current_voltage_state( unsigned char* current_voltage_state_p ){
    
    
    int retVal = BD91401GW_NG;            
    
    
    bd91401gw_reg_type read_status_reg =
    {
       BD91401GW_STATUS_ADDRESS,      
       0                              
    };
    
    printk(KERN_INFO "LINE:%u %s start\n",__LINE__,__func__);
    
    
    if( current_voltage_state_p == NULL )
    {
        
        printk(KERN_ERR "Error in bd91401gw_get_current_voltage_state! current_voltage_state_p == NULL\n");
        return BD91401GW_NG;
    }
    
    

    
    if(g_bd91401gw_ovpocp_attach_flg == true)
    {
        printk(KERN_INFO "[%s] g_bd91401gw_ovpocp_attach_flg = %d, g_bd91401gw_ovpocp_attach_value = 0x%02x  \n", __func__,
            g_bd91401gw_ovpocp_attach_flg, g_bd91401gw_ovpocp_attach_value);
        
        read_status_reg.value = g_bd91401gw_ovpocp_attach_value;
    }
    else
    {
        
        retVal = bd91401gw_read_reg(&read_status_reg);
        if(retVal != BD91401GW_OK)
        {
            return BD91401GW_NG;
        }
    }


    
    *current_voltage_state_p = read_status_reg.value & BD91401GW_STATUS_OVPOCP_MASKB;
    
    printk(KERN_INFO "bd91401gw_get_current_voltage_state: read register value = 0x%02x", read_status_reg.value);
    printk(KERN_INFO "LINE:%u %s end\n",__LINE__,__func__);
    
    
    return BD91401GW_OK;
}













int bd91401gw_get_device_state( usb_sw_device_state_enum* device_state_p ){
    
    printk(KERN_INFO "LINE:%u %s start\n",__LINE__,__func__);
    
    
    if( device_state_p == NULL )
    {
        
        printk(KERN_ERR "Error in bd91401gw_get_device_state! device_state_p == NULL\n");
        return BD91401GW_NG;
    }
    
    *device_state_p = g_usb_sw_device_state;
    
    printk(KERN_INFO "LINE:%u %s end\n",__LINE__,__func__);
    
    return BD91401GW_OK;
}








static void  bd91401gw_set_device_state( void ){
    
    printk(KERN_INFO "LINE:%u %s start\n",__LINE__,__func__);
    
    

    if((g_read_device_info.interrupt_info.interrupt.value & BD91401GW_INTERRUPT_TYPE_COMPH)
      && (BD91401GW_IDSTATUS_TYPE_DEFAULT == (BD91401GW_IDSTATUS_MASKB & g_read_device_info.identify_device_info.id_status.value)))

    {
        
        g_usb_sw_device_state = USB_SW_DEVICE_DISCONNECTED;
    }
    else
    {
        
        if((!(g_read_device_info.interrupt_info.interrupt.value & BD91401GW_INTERRUPT_TYPE_VBUSDET)) &&
            ((g_read_device_info.identify_device_info.status.value & BD91401GW_STATUS_OVPOCP_VB_MASKB) == BD91401GW_STATUS_NOT_OVP_OCP))
        {
            
            if(((g_read_device_info.identify_device_info.status.value & BD91401GW_STATUS_CHGPORT_MASKB) == BD91401GW_STATUS_TYPE_CHGPORT_NOUSB)&&
                ((g_read_device_info.identify_device_info.id_status.value & BD91401GW_IDSTATUS_MASKB) == BD91401GW_IDSTATUS_TYPE_DEFAULT))
            {
                
                g_usb_sw_device_state = USB_SW_DEVICE_DISCONNECTED;
            }
            else
            {
                
                g_usb_sw_device_state = USB_SW_OTHER_DEVICE_CONNECTED;
            }
        }
        else
        {
            
            if((g_read_device_info.identify_device_info.id_status.value & BD91401GW_IDSTATUS_MASKB) == BD91401GW_IDSTATUS_TYPE_OTGHOST)
            {
                
                g_usb_sw_device_state = USB_SW_OTHER_DEVICE_CONNECTED;
            }
            else if((g_read_device_info.identify_device_info.id_status.value & BD91401GW_IDSTATUS_MASKB) == BD91401GW_IDSTATUS_TYPE_DEFAULT)
            {
                
                
                if((g_read_device_info.identify_device_info.status.value & BD91401GW_STATUS_CHGPORT_MASKB) == BD91401GW_STATUS_TYPE_CHGPORT_DCP)
                {
                    
                    g_usb_sw_device_state = USB_SW_DCP_CONNECTED;
                }
                else if((g_read_device_info.identify_device_info.status.value & BD91401GW_STATUS_CHGPORT_MASKB) == BD91401GW_STATUS_TYPE_CHGPORT_CDP)
                {
                    
                    g_usb_sw_device_state = USB_SW_OTHER_DCP_CONNECTED;
                }
                else if((g_read_device_info.identify_device_info.status.value & BD91401GW_STATUS_CHGPORT_MASKB) == BD91401GW_STATUS_TYPE_CHGPORT_SDP)
                {
                    
                    g_usb_sw_device_state = USB_SW_SDP_CONNECTED;
                }
                else if((g_read_device_info.identify_device_info.status.value & BD91401GW_STATUS_CHGPORT_MASKB) == BD91401GW_STATUS_TYPE_CHGPORT_NOUSB)
                {
                    
                    g_usb_sw_device_state = USB_SW_OTHER_DCP_CONNECTED;
                }
                else
                {
                    
                    g_usb_sw_device_state = USB_SW_OTHER_DEVICE_CONNECTED;
                }
            }
            else
            {
                
                g_usb_sw_device_state = USB_SW_OTHER_DCP_CONNECTED;
            }
        }
    }
    
    
    printk(KERN_INFO "LINE:%u %s end g_usb_sw_device_state = %d\n",__LINE__,__func__, g_usb_sw_device_state);
}








void bd91401gw_notify_chg_usb_charger_connected( void ){
    
    printk(KERN_INFO "LINE:%u %s start\n",__LINE__,__func__);
    
    if(g_bd91401gw_chg_notify_flg != true)
    {
        if((g_usb_sw_device_state != USB_SW_DEVICE_DISCONNECTED) && (g_usb_sw_device_state != USB_SW_OTHER_DEVICE_CONNECTED))
        {


             g_bd91401gw_chg_notify_flg = true;
        }
    }
    printk(KERN_INFO "LINE:%u %s end\n",__LINE__,__func__);
}









void bd91401gw_usbsw_set_otg_workable(struct work_struct* w){
    
    printk(KERN_DEBUG "[USBSW]LINE:%u %s start\n",__LINE__,__func__);
    printk(KERN_DEBUG "[USBSW]LINE:%u %s usbsw_set_otg_workable(%d) call\n",__LINE__,__func__,intb_state);
    usbsw_set_otg_workable(intb_state);
    printk(KERN_DEBUG "LINE:%u %s end\n",__LINE__,__func__);
    return;
}









static void bd91401gw_set_ovpocp_attach_state( void )
{
    
    if((g_usb_sw_device_state == USB_SW_OTHER_DCP_CONNECTED)&&
       (g_bd91401gw_chg_notify_flg == false)&&
       ((g_read_device_info.identify_device_info.status.value & BD91401GW_STATUS_OVPOCP_VB_MASKB) != BD91401GW_STATUS_NOT_OVP_OCP))
    {
        
        g_bd91401gw_ovpocp_attach_flg = true;
        
        g_bd91401gw_ovpocp_attach_value = g_read_device_info.identify_device_info.status.value;
    }
    else if(g_usb_sw_device_state == USB_SW_DEVICE_DISCONNECTED)
    {
        
        g_bd91401gw_ovpocp_attach_flg = false;
        
        g_bd91401gw_ovpocp_attach_value = BD91401GW_STATUS_NOT_OVP_OCP;
    }
    else
    {
        
    }
    printk(KERN_INFO "[%s] g_bd91401gw_ovpocp_attach_flg = %d, g_bd91401gw_ovpocp_attach_value = 0x%02x  \n", __func__,
        g_bd91401gw_ovpocp_attach_flg, g_bd91401gw_ovpocp_attach_value);
}


















































static void bd91401gw_usb_audio_mic_on(struct work_struct* w)
{
    int ret = BD91401GW_NG;
    
    
    
    
    
    printk(KERN_DEBUG "[USBSW]LINE:%u %s start\n",__LINE__,__func__);
    mutex_lock(&g_bd91401gw_usb_lock);
    
    
    ret = bd91401gw_read_reg(&g_read_device_info.switch_path_info.control);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_main! read_reg error(03)  ret:%d \n",ret);
        ret = hedset_detect_irq(AUDIOIF_DEVICE_USB,AUDIOIF_IRQ_REMOVE);
        if(AUDIOIF_E_OK != ret)
        {
            printk(KERN_ERR "Error in bd91401gw_control_mic! set switch error  ret:%d \n",ret);
        }
        mutex_unlock(&g_bd91401gw_usb_lock);
        return;
    }
    
    
    ret = bd91401gw_read_reg(&g_read_device_info.identify_device_info.id_status);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_main! read_reg error(07)  ret:%d \n",ret);
        ret = hedset_detect_irq(AUDIOIF_DEVICE_USB,AUDIOIF_IRQ_REMOVE);
        if(AUDIOIF_E_OK != ret)
        {
            printk(KERN_ERR "Error in bd91401gw_control_mic! set switch error  ret:%d \n",ret);
        }
        mutex_unlock(&g_bd91401gw_usb_lock);
        return;
    }
    
    
    printk(KERN_INFO "%s reg value 07:%02x, 03:%02x\n"
            ,__func__
            ,g_read_device_info.identify_device_info.id_status.value
            ,g_read_device_info.switch_path_info.control.value);
    
    
    ret = bd91401gw_set_switch(&g_write_device_info.switch_path_info.control);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_path_initialize! set switch error  ret:%d \n",ret);
        ret = hedset_detect_irq(AUDIOIF_DEVICE_USB,AUDIOIF_IRQ_REMOVE);
        if(AUDIOIF_E_OK != ret)
        {
            printk(KERN_ERR "Error in bd91401gw_control_mic! set switch error  ret:%d \n",ret);
        }
        mutex_unlock(&g_bd91401gw_usb_lock);
        return;
    }
    
    if(g_read_device_info.switch_path_info.control.value != g_write_device_info.switch_path_info.control.value)
    {
        
        printk(KERN_INFO "bd91401gw_main change path! %02x to %02x\n"
            ,g_read_device_info.switch_path_info.control.value
            ,g_write_device_info.switch_path_info.control.value);
        
        if((g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_CHG_EAR)
          || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_EAR)
          || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_EAR_MIC)
          || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_FEED_POW_EAR))
        {
            
            ret = bd91401gw_write_reg(&g_unconnection_path.control);
            if(BD91401GW_OK != ret)
            {
                printk(KERN_ERR "Error in bd91401gw_main! write_reg error(03)  ret:%d \n",ret);
                ret = hedset_detect_irq(AUDIOIF_DEVICE_USB,AUDIOIF_IRQ_REMOVE);
                if(AUDIOIF_E_OK != ret)
                {
                    printk(KERN_ERR "Error in bd91401gw_control_mic! set switch error  ret:%d \n",ret);
                }
                mutex_unlock(&g_bd91401gw_usb_lock);
                return;
            }
            
            udelay(5);
        }
        
        
        ret = bd91401gw_write_reg(&g_write_device_info.switch_path_info.control);
        if(BD91401GW_OK != ret)
        {
            printk(KERN_ERR "Error in bd91401gw_main! write_reg error(03)  ret:%d \n",ret);
            ret = hedset_detect_irq(AUDIOIF_DEVICE_USB,AUDIOIF_IRQ_REMOVE);
            if(AUDIOIF_E_OK != ret)
            {
                 printk(KERN_ERR "Error in bd91401gw_control_mic! set switch error  ret:%d \n",ret);
            }
            mutex_unlock(&g_bd91401gw_usb_lock);
            return;
        }
        
        if((g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_CHG_EAR)
          || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_EAR)
          || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_EAR_MIC)
          || (g_write_device_info.switch_path_info.control.value == BD91401GW_SWCONTROL_FEED_POW_EAR))
        {
            
            printk(KERN_DEBUG "LINE:%u %s call bd91401gw_notify_mic(true)\n",__LINE__,__func__);
            bd91401gw_notify_mic(true);
        }
        else
        {
            
            printk(KERN_DEBUG "LINE:%u %s call bd91401gw_notify_mic(false)\n",__LINE__,__func__);
            bd91401gw_notify_mic(false);
        }
    }
    
    mutex_unlock(&g_bd91401gw_usb_lock);
    
    printk(KERN_DEBUG "LINE:%u %s end\n",__LINE__,__func__);
    return;
}






static void bd91401gw_usb_audio_mic_off(struct work_struct* w)
{
    int ret = BD91401GW_NG;
    
    unsigned char read_id_sta = BD91401GW_IDSTATUS_TYPE_DEFAULT;
    
    printk(KERN_DEBUG "[USBSW]LINE:%u %s start\n",__LINE__,__func__);
    
    mutex_lock(&g_bd91401gw_usb_lock);
    
    
    ret = bd91401gw_write_reg(&g_unconnection_path.control);
    if(BD91401GW_OK != ret)
    {
        printk(KERN_ERR "Error in bd91401gw_main! write_reg error(03)  ret:%d \n",ret);
        ret = hedset_detect_irq(AUDIOIF_DEVICE_USB,AUDIOIF_IRQ_REMOVE);
        if(AUDIOIF_E_OK != ret)
        {
             printk(KERN_ERR "Error in bd91401gw_control_mic! set switch error  ret:%d \n",ret);
        }
        mutex_unlock(&g_bd91401gw_usb_lock);
        return;
    }
    
    mutex_unlock(&g_bd91401gw_usb_lock);
    
    
    printk(KERN_INFO "%s reg value 07:%02x, 03:%02x\n"
            ,__func__
            ,g_read_device_info.identify_device_info.id_status.value
            ,g_read_device_info.switch_path_info.control.value);
    
    
    g_bd91401gw_mic_flg = false;
    
    read_id_sta = g_read_device_info.identify_device_info.id_status.value;
    
    bd91401gw_set_audio_info(read_id_sta,g_bd91401gw_mic_flg);
    
    
    hedset_detect_irq(AUDIOIF_DEVICE_USB,AUDIOIF_IRQ_REMOVE_CALLBACK);
    if(AUDIOIF_E_OK != ret)
    {
         printk(KERN_ERR "Error in bd91401gw_control_mic! set switch error  ret:%d \n",ret);
    }
    printk(KERN_DEBUG "LINE:%u %s end\n",__LINE__,__func__);
    return;
}








void bd91401gw_path_initialize_again_wrapper(void)
{
    
    schedule_work(&path_init);

    
    return;
}






static void bd91401gw_path_initialize_again(struct work_struct* w)
{
    
    bd91401gw_path_initialize();
    
    
    return;
}





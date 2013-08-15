/*=========================================================================
 FILENAME : ncdiag-cxo-offs.c

 Copyright (c) 2012 NEC CASIO Mobile Communications, LTD.  All Rights Reserved.        
=========================================================================== */











#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/sysdev.h>
#include <linux/err.h>
#include <linux/stat.h>
#include <linux/earlysuspend.h>
#include <linux/leds.h>
#include <linux/suspend.h>
#include <linux/leds_cmd.h>




#define USE_WORK_CUEUE_READ_RPM_STATS_FOR_LEDTRIG 0


#define LTRGXOFF_XO_SHUTDOWN_INDEX 0
#define LTRGXOFF_VDD_MIN_INDEX 2






static phys_addr_t ltrgxoff_rpm_stats_phys_addr_base = 0x0010DD04;

static u32 ltrgxoff_rpm_stats_phy_size = SZ_8K;




static int ltrgxoff_rpm_status_counter_index = LTRGXOFF_VDD_MIN_INDEX; 



static void __iomem *ltrgxoff_rpm_stats_reg_base = NULL;


static int ltrgxoff_pm_callback(struct notifier_block *nfb,unsigned long action,    void *ignored);


#if USE_WORK_CUEUE_READ_RPM_STATS_FOR_LEDTRIG
static struct work_struct ltrgxoff_wq;   
#else
static struct delayed_work ltrgxoff_wq;  
#endif

static struct workqueue_struct *ltrgxoff_wq_que = NULL;


DEFINE_LED_TRIGGER(ledtrig_cxo_off)


static struct notifier_block ltrgxoff_pm_notifier = 
{
    .notifier_call = ltrgxoff_pm_callback,
    .priority = 0,
};


static u32 ltrgxoff_registed = false;


static u32 ltrgxoff_led_on = false;



static u32 ltrgxoff_prev_rpm_stats_count = 0;



















static void ltrgxoff_led_off_on(struct work_struct *work)
{
    if(ltrgxoff_led_on)
    {
        
        leds_cmd(LEDS_CMD_TYPE_RGB_RED,   0x1E );
        leds_cmd(LEDS_CMD_TYPE_RGB_GREEN, 0 );
        leds_cmd(LEDS_CMD_TYPE_RGB_BLUE, 0 );
    }
    else
    {
        
        leds_cmd(LEDS_CMD_TYPE_RGB_RED,   0 );
        leds_cmd(LEDS_CMD_TYPE_RGB_GREEN, 0 );
        leds_cmd(LEDS_CMD_TYPE_RGB_BLUE, 0 );
    }
}


































static inline u32 ltrgxoff_rpm_stats_read(void)
{


























    return readl_relaxed(ltrgxoff_rpm_stats_reg_base                        
                        + sizeof(u32)                                       
                        + (ltrgxoff_rpm_status_counter_index * (sizeof(u32) * 3))  
                        + (sizeof(u32) * 2));                               



}



















static void ltrgxoff_read_rpm_stats(struct work_struct *work)
{
    
    
    u32 rpm_stats_count;
    

    
    
    rpm_stats_count = ltrgxoff_rpm_stats_read();
    






    
    
    if( rpm_stats_count != ltrgxoff_prev_rpm_stats_count )
    
    {
        ltrgxoff_led_on = true;
    }
    else
    {
        ltrgxoff_led_on = false;
    }

    
    
    ltrgxoff_prev_rpm_stats_count = rpm_stats_count;
    

#if USE_WORK_CUEUE_READ_RPM_STATS_FOR_LEDTRIG
    
    ltrgxoff_led_off_on(NULL);
#else
    
      
    queue_delayed_work(ltrgxoff_wq_que,&ltrgxoff_wq,20);
#endif

}




















static int ltrgxoff_pm_callback(struct notifier_block *nfb,unsigned long action,void *ignored)
{

    switch (action) {
    case PM_HIBERNATION_PREPARE:
    case PM_SUSPEND_PREPARE:
        
#if USE_WORK_CUEUE_READ_RPM_STATS_FOR_LEDTRIG
        cancel_work_sync(&ltrgxoff_wq);            
#else
        cancel_delayed_work_sync(&ltrgxoff_wq);    
#endif

        return NOTIFY_OK;
    case PM_POST_HIBERNATION:
    case PM_POST_SUSPEND:
        
#if USE_WORK_CUEUE_READ_RPM_STATS_FOR_LEDTRIG
                       
        queue_work(ltrgxoff_wq_que,&ltrgxoff_wq);         
#else
        ltrgxoff_read_rpm_stats(NULL);               
#endif
        return NOTIFY_OK;
    }

    return NOTIFY_DONE;
}























static int ltrgxoff_state_show(char *str, struct kernel_param *kp)
{
    int ret;
    if(ltrgxoff_registed == true)
    {
        ret = snprintf(str, 2, "%d ",1);
    }
    else
    {
        ret = snprintf(str, 2, "%d ",0);
    }
    return ret;
}























static int ltrgxoff_state_store(const char *str, struct kernel_param *kp)
{
    char* next;
    int ltrgxoff_state;
    ltrgxoff_state = simple_strtoul(&str[0],&next,10);

    if(ltrgxoff_state==1)
    {
        if(ltrgxoff_registed == false)
        {
            
            led_trigger_register_simple("cxo_off", &ledtrig_cxo_off);
            register_pm_notifier(&ltrgxoff_pm_notifier);       
            ltrgxoff_registed = true;

            
            
            
            ltrgxoff_prev_rpm_stats_count = ltrgxoff_rpm_stats_read();
            
        }
    }
    else
    {
        if(ltrgxoff_registed == true)
        {
            
            unregister_pm_notifier(&ltrgxoff_pm_notifier);
            led_trigger_unregister_simple(ledtrig_cxo_off);    
            ltrgxoff_registed = false;
        }
    }
        
    return 0;
}




















static int __init ltrgxoff_init(void)
{
    
    ltrgxoff_rpm_stats_reg_base = ioremap(ltrgxoff_rpm_stats_phys_addr_base, ltrgxoff_rpm_stats_phy_size);
    if(!ltrgxoff_rpm_stats_reg_base)
    {
        printk("Error ioremap for sleep check cxo off");
        return 1;
    }

    
    ltrgxoff_wq_que = create_workqueue("ltrgxoff");
    if(!ltrgxoff_wq_que)
    {
        printk("Error create_workqueue for sleep check cxo off");
        return 1;
    }

#if USE_WORK_CUEUE_READ_RPM_STATS_FOR_LEDTRIG
    INIT_WORK(&ltrgxoff_wq,ltrgxoff_read_rpm_stats);
#else
    INIT_DELAYED_WORK(&ltrgxoff_wq,ltrgxoff_led_off_on);
#endif

    return 0;
}




















static void __exit ltrgxoff_exit(void)
{
    if(ltrgxoff_registed==true)
    {
        
        unregister_pm_notifier(&ltrgxoff_pm_notifier);
        led_trigger_unregister_simple(ledtrig_cxo_off);
        ltrgxoff_registed = false;
    }

    if(ltrgxoff_rpm_stats_reg_base!=NULL)
    {
        
        iounmap(ltrgxoff_rpm_stats_reg_base);
        ltrgxoff_rpm_stats_reg_base = NULL;
    }

    
#if USE_WORK_CUEUE_READ_RPM_STATS_FOR_LEDTRIG
        cancel_work_sync(&ltrgxoff_wq);            
#else
        cancel_delayed_work_sync(&ltrgxoff_wq);    
#endif

    
    if(ltrgxoff_wq_que!=NULL)
    {
        destroy_workqueue( ltrgxoff_wq_que );
    }
}




module_init(ltrgxoff_init);


module_exit(ltrgxoff_exit);


module_param_call(state,  ltrgxoff_state_store, ltrgxoff_state_show, NULL, 0640);


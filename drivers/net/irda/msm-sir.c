
/*********************************************************************
 *
 * Filename:      msm-sir.c
 * Version:       0.1
 * Description:   Implementation for msm  dongle
 * Status:        Experimental.
 * Author:        UENO Yoshiyuki
 * Created at:    2010..8.17
 * Modified at:   
 * Modified by:   
  *
 *     Copyright (c) 2010 NEC corpration, All Rights Reserved.
 *
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation; either version 2 of
 *     the License, or (at your option) any later version.
 *
 ********************************************************************/
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/


#define RPM975H16 1


#include <linux/module.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/miscdevice.h>

#include <linux/irq.h>
#include <linux/tty.h>
#include <linux/poll.h>

#include <net/irda/irda.h>
#include <net/irda/irlmp.h>
#include <net/irda/timer.h>
#include <net/irda/irias_object.h>
#include <net/irda/discovery.h>
#include <net/irda/irdrv.h>
#include <linux/mfd/pm8xxx/misc.h> 

#include <asm/gpio.h>
#include <mach/msm_serial_hs.h>
#include <linux/regulator/consumer.h> 

#include <mach/irqs.h>
#include <linux/mfd/pm8xxx/pm8921.h>


#include "sir-dev.h"
#include "irtty-sir.h"










struct irda_paramdata irda_param;                   
int    irda_discovery_state = 0;                    

wait_queue_head_t irdrv_irlap_event_wait;           
unsigned char irdrv_irlap_event_flag;               
unsigned char irdrv_icon_flag;                      

wait_queue_head_t irdrv_disc_event_wait;            
unsigned char irdrv_disc_wait_flag;                 

extern struct irlmp_cb *irlmp;

extern char sysctl_devname[];
extern int  sysctl_discovery;
extern int  sysctl_discovery_slots;
extern int  sysctl_discovery_timeout;






static int msmsir_reset(struct sir_dev *dev);
static int msmsir_open(struct sir_dev *dev);
static int msmsir_close(struct sir_dev *dev);
static int msmsir_change_speed(struct sir_dev *dev, unsigned speed);

extern void msmsir_enable_rx(struct sir_dev *dev);
extern void msmsir_disable_rx(struct sir_dev *dev);
extern int msmsir_rx_receiving(struct sir_dev *dev);    

void msm_hs_enable_rx_locked(struct uart_port *uport);
void msm_hs_disable_rx_locked(struct uart_port *uport);
int msm_hs_rx_receiving(struct uart_port *uport);       









#define MSM_UART2DM_PHYS      0x16640000





#define UART_IRDA_ADDR      0x38
#define IRDA_EN             0x0001
#define INVERT_IRDA_RX      0x0002









#define PM8921_GPIO_BASE        NR_GPIO_IRQS
#define PM8921_GPIO_PM_TO_SYS(pm_gpio)  (pm_gpio - 1 + PM8921_GPIO_BASE)









#define GPIO_IRDA_SWDOWN    40
#define GPIO_IRDA_SHDN      17





struct pm_gpio irda_swdown_on = {

        .direction      = PM_GPIO_DIR_OUT,
        .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
        .output_value   = 1,
        .pull           = PM_GPIO_PULL_NO,
        .vin_sel        = PM_GPIO_VIN_VPH,
        .out_strength   = PM_GPIO_STRENGTH_LOW,
        .function       = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol    = 0,
    };



struct pm_gpio irda_swdown_off = {

        .direction      = PM_GPIO_DIR_OUT,
        .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
        .output_value   = 0,
        .pull           = PM_GPIO_PULL_NO,
        .vin_sel        = PM_GPIO_VIN_VPH,
        .out_strength   = PM_GPIO_STRENGTH_LOW,
        .function       = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol    = 0,
    };





struct pm_gpio irda_shdn_on = {

        .direction      = PM_GPIO_DIR_OUT,
        .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
        .output_value   = 1,
        .pull           = PM_GPIO_PULL_NO,
        .vin_sel        = PM_GPIO_VIN_S4,
        .out_strength   = PM_GPIO_STRENGTH_LOW,
        .function       = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol    = 0,
    };



struct pm_gpio irda_shdn_off = {

        .direction      = PM_GPIO_DIR_OUT,
        .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
        .output_value   = 0,
        .pull           = PM_GPIO_PULL_NO,
        .vin_sel        = PM_GPIO_VIN_S4,
        .out_strength   = PM_GPIO_STRENGTH_LOW,
        .function       = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol    = 0,
    };





































struct pm_gpio irda_ioen_off = {

        .direction      = PM_GPIO_DIR_OUT,
        .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
        .output_value   = 1,
        .pull           = PM_GPIO_PULL_NO,
        .vin_sel        = PM_GPIO_VIN_S4,
        .out_strength   = PM_GPIO_STRENGTH_LOW,
        .function       = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol    = 0,
    };


#define PMGPIO_UART_RX        33



struct pm_gpio irda_uart_rx_on = {

        .direction      = PM_GPIO_DIR_IN,
        .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
        .output_value   = 0,
        .pull           = PM_GPIO_PULL_NO,
        .vin_sel        = PM_GPIO_VIN_S4,
        .out_strength   = PM_GPIO_STRENGTH_NO,
        .function       = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol    = 0,
    };



struct pm_gpio irda_uart_rx_off = {

        .direction      = PM_GPIO_DIR_IN,
        .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
        .output_value   = 0,


        .pull           = PM_GPIO_PULL_NO,

        .vin_sel        = PM_GPIO_VIN_S4,
        .out_strength   = PM_GPIO_STRENGTH_NO,
        .function       = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol    = 0,
    };


#define PMGPIO_UART_M_RX        38
struct pm_gpio irda_uart_m_rx = {
        .direction      = PM_GPIO_DIR_OUT,
        .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
        .output_value   = 0,
        .pull           = PM_GPIO_PULL_NO,
        .vin_sel        = PM_GPIO_VIN_S4,


        .out_strength   = PM_GPIO_STRENGTH_LOW,



        .function       = PM_GPIO_FUNC_2,

        .inv_int_pol    = 0,
    };














































































































































































































































































struct irda_gpio_info {
    unsigned no;
    char *name;
};


static struct dongle_driver msmsir = {
    .owner      = THIS_MODULE,
    .driver_name    = "MSMSIR",
    .type       = IRDA_DVE031_DONGLE,
    .open       = msmsir_open,
    .close      = msmsir_close,
    .reset      = msmsir_reset,
    .set_speed  = msmsir_change_speed,
};

static  void __iomem *uart2base;


static volatile unsigned char gGpioReqRxdFlag; 
static volatile unsigned char gGpioReqTxdFlag; 
static volatile unsigned char gGpioOutShdnFlag; 






































void msmsir_enable_rx(struct sir_dev *dev)
{

    struct sirtty_cb *priv = dev->priv;
    struct tty_struct *tty;
    struct uart_state *state;
    struct uart_port *port;



    IRDA_ASSERT(priv != NULL, return;);
    IRDA_ASSERT(priv->magic == IRTTY_MAGIC, return;);

    tty = priv->tty;
    state = tty->driver_data;
    port = state->uart_port;

    IRDA_ASSERT(port != NULL, return;);

    if (irdrv_icon_flag == IR_COMMSTAT_POWEROFF)
        return;












    msm_hs_enable_rx_locked(port);

}

void msmsir_disable_rx(struct sir_dev *dev)
{
    struct sirtty_cb *priv = dev->priv;
    struct tty_struct *tty;
    struct uart_state *state;
    struct uart_port *port;



    IRDA_ASSERT(priv != NULL, return;);
    IRDA_ASSERT(priv->magic == IRTTY_MAGIC, return;);

    tty = priv->tty;
    state = tty->driver_data;
    port = state->uart_port;

    IRDA_ASSERT(port != NULL, return;);













    msm_hs_disable_rx_locked(port);

}


int msmsir_rx_receiving(struct sir_dev *dev)
{
    struct sirtty_cb *priv = dev->priv;
    struct tty_struct *tty;
    struct uart_state *state;
    struct uart_port *port;
    int rx_count;



    IRDA_ASSERT(priv != NULL, return -1;);
    IRDA_ASSERT(priv->magic == IRTTY_MAGIC, return -1;);

    tty = priv->tty;
    state = tty->driver_data;
    port = state->uart_port;

    IRDA_ASSERT(port != NULL, return -1;);

    rx_count = msm_hs_rx_receiving(port);

    return rx_count;
}


static int msmsir_irhw_enable(void)
{
    int ret = 0;
    unsigned long uart_irda_reg;

































    static const struct irda_gpio_info irda_rxd    = { 83, "irda_rxd"};
    static const struct irda_gpio_info irda_txd    = { 82, "irda_txd"};



    IRDA_DEBUG(2, "%s()\n", __func__ );


    
    ret = pm8xxx_gpio_config(PM8921_GPIO_PM_TO_SYS(GPIO_IRDA_SWDOWN), &irda_swdown_on);
    if (ret) {
        pr_err("%s: PMIC GPIO %d write(L) failed\n", __func__,
            (GPIO_IRDA_SWDOWN));
        return ret;
    }


#if !defined(RPM975H16)












    
    msleep(10);

    
    ret = pm8xxx_gpio_config(PM8921_GPIO_PM_TO_SYS(GPIO_IRDA_IO_EN), &irda_ioen_on);
    if (ret) {
        pr_err("%s: PMIC GPIO %d write(L) failed\n", __func__,
            (GPIO_IRDA_IO_EN));
        return ret;
    }
#endif 

    











    uart_irda_reg = IRDA_EN|INVERT_IRDA_RX;

    printk("UART_IRDA=%lx\n",uart_irda_reg );
    iowrite32(uart_irda_reg, uart2base + UART_IRDA_ADDR);



    ret = pm8xxx_uart_gpio_mux_ctrl(UART_TX1_RX1);

    printk("pm8xxx_uart_gpio_mux_ctrl return %d\n",ret );






































    
    gpio_free(irda_rxd.no);
    gGpioReqRxdFlag = 0;

    gpio_free(irda_txd.no);
    gGpioReqTxdFlag = 0;

















































































































































    ret = pm8xxx_gpio_config(PM8921_GPIO_PM_TO_SYS(PMGPIO_UART_M_RX), &irda_uart_m_rx);
    if (ret) {
        pr_err("%s: PMIC GPIO %d write(PMGPIO_UART_M_RX) failed\n", __func__,
            (PMGPIO_UART_M_RX));
    }


    
    ret = pm8xxx_gpio_config(PM8921_GPIO_PM_TO_SYS(PMGPIO_UART_RX), &irda_uart_rx_on);
    if (ret) {
        pr_err("%s: PMIC GPIO %d write(No pull) failed\n", __func__,
            (PMGPIO_UART_RX));
        return ret;
    }
























    
    ret = pm8xxx_gpio_config(PM8921_GPIO_PM_TO_SYS(GPIO_IRDA_SHDN), &irda_shdn_off);
    if (ret) {
        pr_err("%s: PMIC GPIO %d write(L) failed\n", __func__,
            (GPIO_IRDA_SHDN));
        return ret;
    }

    
    udelay(500);































































































































    return 0;
}


static int msmsir_irhw_disable(void)
{
    int ret = 0;
    unsigned long uart_irda_reg;



























    static const struct irda_gpio_info irda_rxd    = { 83, "irda_rxd"};
    static const struct irda_gpio_info irda_txd    = { 82, "irda_txd"};



    IRDA_DEBUG(2, "%s()\n", __func__ );



    ret = pm8xxx_uart_gpio_mux_ctrl(UART_NONE);

    printk("pm8xxx_uart_gpio_mux_ctrl return %d\n",ret );



    
    if (gGpioReqRxdFlag != 1){
        ret = gpio_request(irda_rxd.no, irda_rxd.name);
        if (ret) {
            printk(KERN_ERR "%s: gpio(rxd) enable failed: %d\n",
                __func__, ret);
            return -ENOMEM;
        }
        gGpioReqRxdFlag = 1;
    }

    if (gGpioReqTxdFlag != 1){
        ret = gpio_request(irda_txd.no, irda_txd.name);
        if (ret) {
            printk(KERN_ERR "%s: gpio(txd) enable failed: %d\n",
                __func__, ret);
            return -ENOMEM;
        }
        gGpioReqTxdFlag = 1;
    }

    
    gpio_direction_input(irda_rxd.no);
    gpio_set_value(irda_txd.no, 0);



    
    uart_irda_reg = ioread32(uart2base + UART_IRDA_ADDR);
    printk("UART_IRDA=%lx\n",uart_irda_reg );
    uart_irda_reg &= ~IRDA_EN;
    printk("UART_IRDA=%lx\n",uart_irda_reg );
    iowrite32(uart_irda_reg, uart2base + UART_IRDA_ADDR);































































































































    
    ret = pm8xxx_gpio_config(PM8921_GPIO_PM_TO_SYS(GPIO_IRDA_SHDN), &irda_shdn_on);
    if (ret) {
        pr_err("%s: PMIC GPIO %d write(H) failed\n", __func__,
            (GPIO_IRDA_SHDN));
        return ret;
    }




















    
    ret = pm8xxx_gpio_config(PM8921_GPIO_PM_TO_SYS(PMGPIO_UART_RX), &irda_uart_rx_off);
    if (ret) {
        pr_err("%s: PMIC GPIO %d write(Pull down) failed\n", __func__,
            (PMGPIO_UART_RX));
        return ret;
    }


#if !defined(RPM975H16)

    
    ret = pm8xxx_gpio_config(PM8921_GPIO_PM_TO_SYS(GPIO_IRDA_IO_EN), &irda_ioen_off);
    if (ret) {
        pr_err("%s: PMIC GPIO %d write(H) failed\n", __func__,
            (GPIO_IRDA_IO_EN));
        return ret;
    }

    
    msleep(10);

#endif 


    
    ret = pm8xxx_gpio_config(PM8921_GPIO_PM_TO_SYS(GPIO_IRDA_SWDOWN), &irda_swdown_off);
    if (ret) {
        pr_err("%s: PMIC GPIO %d write(H) failed\n", __func__,
            (GPIO_IRDA_SWDOWN));
        return ret;
    }
















































































    return 0;
}


static int msmsir_open(struct sir_dev *dev)
{
    struct qos_info *qos = &dev->qos;
    int ret = 0;

    struct sirtty_cb *priv = dev->priv;
    struct tty_struct *tty;


    IRDA_ASSERT(priv != NULL, return -1;);
    IRDA_ASSERT(priv->magic == IRTTY_MAGIC, return -1;);

    tty = priv->tty;

    IRDA_DEBUG(2, "%s()\n", __func__ );


    if (tty->ops->stop)
        tty->ops->stop(tty);





    ret = msmsir_irhw_enable();
    if (ret <0){
        IRDA_DEBUG(0, "%s: error enable ir hw\n", __func__ );
        return ret;
    }




    tty_driver_flush_buffer(tty);

    if (tty->ops->start)
        tty->ops->start(tty);



    
    sirdev_set_dtr_rts(dev, TRUE, TRUE);

    irdrv_irlap_event_flag = 1;                         
    irdrv_icon_flag = IR_COMMSTAT_POWERON;              



    irdrv_disc_wait_flag = 0;                           

    
    qos->baud_rate.bits &= IR_9600|IR_19200|IR_38400|IR_57600|IR_115200;
    qos->min_turn_time.bits = 0x03;
    irda_qos_bits_to_value(qos);

    

    return 0;
}

static int msmsir_close(struct sir_dev *dev)
{
    int ret = 0;

    IRDA_DEBUG(2, "%s()\n", __func__ );

    
    sirdev_set_dtr_rts(dev, FALSE, FALSE);

    ret = msmsir_irhw_disable();
    if (ret <0){
        IRDA_DEBUG(0, "%s: error disalbe ir hw\n", __func__ );
        return ret;
    }



    irdrv_irlap_event_flag = 1;                         
    irdrv_icon_flag = IR_COMMSTAT_POWEROFF;             




    return 0;
}







static int msmsir_change_speed(struct sir_dev *dev, unsigned speed)
{
    int ret = 0;

    unsigned long uart_irda_reg;


    IRDA_DEBUG(2, "%s()\n", __func__ );

    uart_irda_reg = IRDA_EN|INVERT_IRDA_RX;

    printk("UART_IRDA=%lx\n",uart_irda_reg );

    iowrite32(uart_irda_reg, uart2base + UART_IRDA_ADDR);


    dev->speed = speed;
    return ret;
}







static int msmsir_reset(struct sir_dev *dev)
{
    return 0;
}







static int irdrv1_open(struct inode *inode,
        struct file *file)
{
    int ret = 0;

    IRDA_DEBUG(2, "%s()\n", __func__ );

    init_waitqueue_head(&irdrv_irlap_event_wait);

    return ret;
}

static int irdrv1_release(struct inode *inode,
        struct file *file)
{
    int ret = 0;

    IRDA_DEBUG(2, "%s()\n", __func__ );
    return  ret;
}

static ssize_t irdrv1_read(struct file *file, char *c, size_t data_size, loff_t *data)
{
    int ret = 0;

    IRDA_DEBUG(2, "%s()\n", __func__ );

    
    ret = copy_to_user(c,  &irdrv_icon_flag, 1);                                  

    if ( ret < 0 ){
        IRDA_DEBUG(1, "%s() fait to get paramdata err=%d\n", __func__, ret );
        ret = EINVAL;
    } else {
        IRDA_DEBUG(2, "%s() line state is %d\n", __func__,  irdrv_icon_flag);
        ret = 1;
    }
    return ret;
}

static unsigned int irdrv1_poll(struct file *file, struct poll_table_struct *poll_table)
{
    unsigned int mask = 0;

    IRDA_DEBUG(4, "%s()\n", __func__ );

    if(!irdrv_irlap_event_flag)                                             
    {
        IRDA_DEBUG(4, "%s() start waiting\n", __func__ );

        poll_wait(file, &irdrv_irlap_event_wait, poll_table);               

        IRDA_DEBUG(4, "%s() wakeup\n", __func__ );
    }
    if(irdrv_irlap_event_flag)                                              
    {
        irdrv_irlap_event_flag = 0;                                         
        mask |= POLLIN | POLLRDNORM;                                        
        IRDA_DEBUG(4, "%s() POLLIN | POLLRDNORM\n", __func__ );
    }

    return mask;
}

static struct file_operations irdrv1_fops = {
    .owner      = THIS_MODULE,
    .read       = irdrv1_read,
    .poll       = irdrv1_poll,
    .open       = irdrv1_open,
    .release    = irdrv1_release,
};

static struct miscdevice irdrv1_device = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = "irdrv1",
    .fops   = &irdrv1_fops,
};






static int irdrv2_open(struct inode *inode,
        struct file *file)
{
    int ret = 0;

    IRDA_DEBUG(2, "%s()\n", __func__ );

    return ret;
}

static int irdrv2_release(struct inode *inode,
        struct file *file)
{
    int ret = 0;

    IRDA_DEBUG(2, "%s()\n", __func__ );
    return  ret;
}

static unsigned int irdrv2_poll(struct file *file, struct poll_table_struct *poll_table)
{
    unsigned int mask = 0;

    IRDA_DEBUG(4, "%s()\n", __func__ );

    if(irdrv_disc_wait_flag==1)                                             
    {



        IRDA_DEBUG(4, "%s() start waiting\n", __func__ );

        poll_wait(file,&irdrv_disc_event_wait,poll_table);                  

        IRDA_DEBUG(4, "%s() wakeup\n", __func__ );
    }
    
    if(irdrv_disc_wait_flag == 0)                                           
    {
        IRDA_DEBUG(4, "%s() POLLHUP\n", __func__ );

        mask |= POLLHUP;
    }
    
    return mask;
}


static long irdrv2_ioctl(struct file *file, unsigned int cmd , unsigned long arg)

{
    int ret = 0;

    IRDA_DEBUG(2, "%s()\n", __func__ );

    switch( cmd ){
        case IRDRVPARAMSET:         
            IRDA_DEBUG(4, "%s() param set\n", __func__ );
            irda_discovery_state = 0;

            strcpy(sysctl_devname, "Linux");

            memset(&irda_param, 0, sizeof(irda_param) );

            ret = copy_from_user( &irda_param, 
                            (struct irda_paramdata*)arg,
                            sizeof(struct irda_paramdata)
                           );
            if ( ret < 0 ){
                IRDA_DEBUG(1, "%s() fait to get paramdata err=%d\n", __func__, ret );
                ret = EINVAL;
            }
            if(irda_param.name_len > NICKNAME_MAX_LEN)                     
            {
                irda_param.name_len = NICKNAME_MAX_LEN;
            }

            if (irda_param.name_len > 0){
                strcpy(sysctl_devname, irda_param.dev_nickname);
            }

            IRDA_DEBUG(4, "%s() dev nickname = %s\n", __func__,  sysctl_devname);

            break;

        case IRDRVDISCOVERYSTART:           

            IRDA_DEBUG(4, "%s() start discovery timeout =%d\n", __func__, sysctl_discovery_timeout );

           if (irlmp == NULL) {
                ret = -ENODEV;
                IRDA_DEBUG(1, "%s() irlmp is null\n", __func__ );
            }

            irlmp_start_discovery_timer(irlmp, sysctl_discovery_timeout*HZ);

           break;

        case IRDRVDISCOVERYSTOP:            

            IRDA_DEBUG(4, "%s() stop discovery\n", __func__ );
            irda_discovery_state = 1;

            if (irlmp == NULL) {
                ret = -ENODEV;
                IRDA_DEBUG(1, "%s() irlmp is null\n", __func__ );
            }

           del_timer_sync(&irlmp->discovery_timer);

           break;

        default:
            IRDA_DEBUG(1, "%s() cannot find ioctl %d\n", __func__, cmd );
            ret = -EINVAL;
    }
    return ret;
}

static struct file_operations irdrv2_fops = {
    .owner      = THIS_MODULE,

    .poll       = irdrv2_poll,


    .unlocked_ioctl     = irdrv2_ioctl,

    .open       = irdrv2_open,
    .release    = irdrv2_release,
};

static struct miscdevice irdrv2_device = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = "irdrv2",
    .fops   = &irdrv2_fops,
};







static int irdiag_open(struct inode *inode,
        struct file *file)
{
    printk(KERN_INFO ">>  %s: mmc diag open!\n", __func__);

    return msmsir_irhw_enable();
}

static int irdiag_release(struct inode *inode,
        struct file *file)
{
    printk(KERN_INFO ">>  %s: mmc diag close!\n", __func__);
    return  msmsir_irhw_disable();
}

static struct file_operations irdiag_fops = {
    .owner      = THIS_MODULE,

    .open       = irdiag_open,
    .release    = irdiag_release,
};

static struct miscdevice irdaig_device = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = "irdiag",
    .fops   = &irdiag_fops,
};




static int __init msmsir_sir_init(void)
{












    uart2base = ioremap(MSM_UART2DM_PHYS, PAGE_SIZE);
    if (!uart2base){
        printk(KERN_ERR "%s:Could not ioremap region\n", __func__);
        return -ENOMEM;
    }
    printk("%s:iobaseaddress =%8lx\n", __func__, (unsigned long)uart2base);

    misc_register(&irdaig_device);

    misc_register(&irdrv1_device);

    misc_register(&irdrv2_device);


    gGpioReqRxdFlag = 0;
    gGpioReqTxdFlag = 0;
    gGpioOutShdnFlag = 0;


    return irda_register_dongle(&msmsir);
}

static void __exit msmsir_sir_cleanup(void)
{
    misc_deregister(&irdrv2_device);

    misc_deregister(&irdrv1_device);

    misc_deregister(&irdaig_device);

    irda_unregister_dongle(&msmsir);
}


MODULE_AUTHOR("UENO Yoshiyuki");
MODULE_DESCRIPTION("NEC Corp MSM dongle driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("irda-dongle-10"); 

module_init(msmsir_sir_init);
module_exit(msmsir_sir_cleanup);


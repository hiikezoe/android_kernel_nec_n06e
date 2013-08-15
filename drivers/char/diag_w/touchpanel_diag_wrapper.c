/*
 * touchpanel_diag_wrapper.c
 *
 * Copyright (C) 2010 NEC NEC Corporation
 *
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/
 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/rwsem.h>
#include <linux/ioctl.h>

#include <linux/touchpanel_diag_wrapper.h>
#include <linux/touch_panel_cmd.h>
#include <mach/rpc_server_handset.h>

#include <linux/slab.h>


#define DEVICEWRPPER_NAME       "touchpanel_diag_wrapper"


#define TOUCH_TIMEOUT   (HZ * 30)



static wait_queue_head_t touch_cmd_wait;
touch_diag_result g_rsp_touch_data;
static char g_touch_flg;
static long g_touch_timeout = (HZ * 30);

static int diag_wrapper_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "touchpanel_diag_wrapper_open\n");
    return 0;
}

void diag_touch_panel_cmd_pd_mdcmd(void *par)
{
    wait_queue_head_t   *wq = &touch_cmd_wait;
    touch_diag_result *coord;

    coord = (touch_diag_result *)par;

    memcpy (&g_rsp_touch_data, coord, sizeof (touch_diag_result));
    g_touch_flg = 1;

    wake_up(wq);
    return;
}

static long diag_wrapper_ioctl(struct file *file, unsigned int iocmd, unsigned long data )
{
    int ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_OK;
    unsigned char ret_val;
    int i;
    int conf_ret = 0;
    unsigned char touch_finger;
    touch_diag_result *touch_result = NULL;



    int err;
    long    timeout;
    unsigned char *pkt_params = NULL;
    unsigned char type = 0;
    
    size_t res_size = sizeof(char);
    
    init_waitqueue_head(&touch_cmd_wait);

    printk(KERN_DEBUG "[touchpanel_diag_wrapper]%s: ioctl Enter (iocmd:0x%02X)\n", __func__,iocmd);
    g_touch_flg = 0;

    switch(iocmd){
    case DIAG_WRAPPER_IOCTL_C01:
        ret = (int)touch_panel_cmd(TOUCH_PANEL_CMD_TYPE_RESET, NULL);
        if (ret != 0)
            ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_NG;
        break;
        
    case DIAG_WRAPPER_IOCTL_C02:
        ret = (int)touch_panel_cmd(TOUCH_PANEL_CMD_TYPE_ENTR_SLEEP, NULL);
        if (ret != 0)
            ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_NG;
        break;
        
    case DIAG_WRAPPER_IOCTL_C03:
        ret = (int)touch_panel_cmd(TOUCH_PANEL_CMD_TYPE_EXIT_SLEEP, NULL);
        if (ret != 0)
            ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_NG;
        break;
        
    case DIAG_WRAPPER_IOCTL_C04:
        ret = (int)touch_panel_cmd(TOUCH_PANEL_CMD_TYPE_POWER_OFF, NULL);
        if (ret != 0)
            ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_NG;
        break;
        
    case DIAG_WRAPPER_IOCTL_C05:
        ret = (int)touch_panel_cmd(TOUCH_PANEL_CMD_TYPE_POWER_ON, NULL);
        if (ret != 0)
            ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_NG;
        break;

    case DIAG_WRAPPER_IOCTL_C06:
        res_size = ( sizeof(char) * 3 );
        pkt_params = (char *)kmalloc(res_size, GFP_KERNEL);
        if( !pkt_params )
            return -ENOMEM;

        err = copy_from_user( pkt_params, (unsigned char *)data, res_size );
        if (err) {
            kfree( pkt_params );
            return -EINVAL;
        }







        
        ret = (int)touch_panel_cmd(TOUCH_PANEL_CMD_TYPE_GET_REVISION, pkt_params );

        ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_OK;

        break;

    case DIAG_WRAPPER_IOCTL_C07:
        res_size = ( sizeof(char) * 5 );






        pkt_params = (char *)kmalloc(res_size, GFP_KERNEL);

        if( !pkt_params )
            return -ENOMEM;

        touch_result = (touch_diag_result *)kmalloc(sizeof(touch_diag_result), GFP_KERNEL);

        if( !touch_result ){
            kfree(pkt_params);
            return -ENOMEM;
        }

        err = copy_from_user( pkt_params, (unsigned char *)data, res_size );
        if (err) {
            kfree(pkt_params);
            kfree(touch_result);
            return -EINVAL;
        }

        touch_finger = pkt_params[1];

        if( touch_finger < TOUCH_PANEL_FINGER_NUM_MAX )
        {
            ret = (int)touch_panel_cmd(TOUCH_PANEL_CMD_TYPE_GET_COORD, touch_result);
            pkt_params[1] = touch_result->finger[touch_finger].finger_data_x[0];
            pkt_params[2] = touch_result->finger[touch_finger].finger_data_x[1];
            pkt_params[3] = touch_result->finger[touch_finger].finger_data_y[0];
            pkt_params[4] = touch_result->finger[touch_finger].finger_data_y[1];
        }
        else
        {
            pkt_params[1] = 0xFF;
            pkt_params[2] = 0xFF;
            pkt_params[3] = 0xFF;
            pkt_params[4] = 0xFF;
        }



















        kfree(touch_result);
        break;

    case DIAG_WRAPPER_IOCTL_C10:
        res_size = ( sizeof(char) * 2 );
        pkt_params = (char *)kmalloc(res_size, GFP_KERNEL);
        if( !pkt_params )
            return -ENOMEM;
            
        err = copy_from_user( pkt_params, (unsigned char *)data, res_size );
        if (err) {
            kfree(pkt_params);
            return -EINVAL;
        }
        ret = (int)touch_panel_cmd(TOUCH_PANEL_CMD_TYPE_FW_UPDATE, NULL);
        pkt_params[1] = ret;
        ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_OK;
        break;
    case DIAG_WRAPPER_IOCTL_C11:
        res_size = ( sizeof(char) * 3 );
        pkt_params = (char *)kmalloc(res_size, GFP_KERNEL);
        if( !pkt_params )
            return -ENOMEM;
            
        err = copy_from_user( pkt_params, (unsigned char *)data, res_size );
        if (err) {
            kfree(pkt_params);
            return -EINVAL;
        }
        ret = (int)touch_panel_cmd(TOUCH_PANEL_CMD_TYPE_GET_CHECKSUM, pkt_params);
        pkt_params[1] = (unsigned char)((ret >> 8) & 0xFF);
        pkt_params[2] = (unsigned char)(ret & 0xFF);
        break;

    case DIAG_WRAPPER_IOCTL_C12:
        res_size = ( sizeof(char) * 41 );

        pkt_params = (char *)kmalloc(res_size, GFP_KERNEL);
        if( !pkt_params )
            return -ENOMEM;


        ret_val = (int)touch_panel_cmd_callback(TOUCH_PANEL_CMD_TYPE_LINE_TEST, 0, diag_touch_panel_cmd_pd_mdcmd);
        if (ret_val != 0)
        {
            memset(pkt_params, 0xFF, res_size );
            ret = -1;
        }
        else
        {
            g_touch_flg = 0;
            timeout = wait_event_timeout(touch_cmd_wait, g_touch_flg != 0, (g_touch_timeout * HZ)+10);
            if(timeout == 0)
            {
                ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_NG;
            } else {
                pkt_params[0]  = g_rsp_touch_data.ret;
                for (i = 0 ; i < 5 ; i++)
                {
                    pkt_params[(i*4)+1]  = g_rsp_touch_data.finger[i].finger_data_x[0];
                    pkt_params[(i*4)+2]  = g_rsp_touch_data.finger[i].finger_data_x[1];
                    pkt_params[(i*4)+3]  = g_rsp_touch_data.finger[i].finger_data_y[0];
                    pkt_params[(i*4)+4]  = g_rsp_touch_data.finger[i].finger_data_y[1];
                }
                ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_OK;
            }
        }

        break;
    case DIAG_WRAPPER_IOCTL_C13:
        err = copy_from_user( &g_touch_timeout, (unsigned char *)data, sizeof(long) );
        if (err) {
            kfree(pkt_params);
            return -EINVAL;
        }
        break;
    case DIAG_WRAPPER_IOCTL_C16:
        res_size = ( sizeof(char) * 3 );
        pkt_params = (char *)kmalloc(res_size, GFP_KERNEL);
        if( !pkt_params )
            return -ENOMEM;

        err = copy_from_user( pkt_params, (unsigned char *)data, res_size );
        if (err) {
            kfree(pkt_params);
            return -EINVAL;
        }












        
        ret = (int)touch_panel_cmd(TOUCH_PANEL_CMD_TYPE_GET_FW_REVISION, pkt_params );

        ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_OK;
        break;

    case DIAG_WRAPPER_IOCTL_C25:
        res_size = ( sizeof(char) * 6 );
        pkt_params = (char *)kmalloc(res_size, GFP_KERNEL);
        if( !pkt_params )
            return -ENOMEM;

        err = copy_from_user( pkt_params, (unsigned char *)data, res_size );
        if (err) {
            kfree(pkt_params);
            return -EINVAL;
        }

        ret = (int)touch_panel_cmd(TOUCH_PANEL_CMD_TYPE_GET_FW_CONFIG_ID, &conf_ret);
        
        if( ret == 0xFF )
        {
            pkt_params[1] = 0x01;
            pkt_params[2] = 0xFF;
            pkt_params[3] = 0xFF;
            pkt_params[4] = 0xFF;
            pkt_params[5] = 0xFF;
            ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_NG;
        }
        else
        {
            pkt_params[1] = 0x00;
            pkt_params[2] = (unsigned char)((conf_ret >> 24) & 0xFF);
            pkt_params[3] = (unsigned char)((conf_ret >> 16) & 0xFF);
            pkt_params[4] = (unsigned char)((conf_ret >>  8) & 0xFF);
            pkt_params[5] = (unsigned char)(conf_ret & 0xFF);

            ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_OK;
        }
        break;

    case DIAG_WRAPPER_IOCTL_C26:
        res_size = ( sizeof(char) * 6 );
        pkt_params = (char *)kmalloc(res_size, GFP_KERNEL);
        if( !pkt_params )
            return -ENOMEM;

        err = copy_from_user( pkt_params, (unsigned char *)data, res_size );
        if (err) {
            kfree(pkt_params);
            return -EINVAL;
        }

        ret = (int)touch_panel_cmd(TOUCH_PANEL_CMD_TYPE_GET_CONFIG_ID, &conf_ret);
        
        if( ret == 0xFF )
        {
            pkt_params[1] = 0x01;
            pkt_params[2] = 0xFF;
            pkt_params[3] = 0xFF;
            pkt_params[4] = 0xFF;
            pkt_params[5] = 0xFF;
            ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_NG;
        }
        else
        {
            pkt_params[1] = 0x00;
            pkt_params[2] = (unsigned char)((conf_ret >> 24) & 0xFF);
            pkt_params[3] = (unsigned char)((conf_ret >> 16) & 0xFF);
            pkt_params[4] = (unsigned char)((conf_ret >>  8) & 0xFF);
            pkt_params[5] = (unsigned char)(conf_ret & 0xFF);

            ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_OK;
        }

        break;

    case DIAG_WRAPPER_IOCTL_C19:
    case DIAG_WRAPPER_IOCTL_C20:
        if( iocmd == DIAG_WRAPPER_IOCTL_C19 ) {
            type = TOUCH_PANEL_CMD_TYPE_WRITE_SPI;
        } else if( iocmd == DIAG_WRAPPER_IOCTL_C20 ) {
            type = TOUCH_PANEL_CMD_TYPE_READ_SPI;
        }

        res_size = 270;
        pkt_params = (char *)kmalloc(res_size, GFP_KERNEL);
        if( !pkt_params )
            return -ENOMEM;
            
        err = copy_from_user( pkt_params, (unsigned char *)data, res_size );
        if (err) {
            kfree(pkt_params);
            return -EINVAL;
        }
        ret = (int)touch_panel_cmd(type, pkt_params);

        if( ret == 0xFF )
            ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_OK;
        else
            ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_NG;
        break;
    default:
        ret = DIAG_TOUCHPANEL_WRAPPER_IOCTL_NG;
        break;
    }
    
    if(pkt_params){
        err = copy_to_user((unsigned char *)data, pkt_params, res_size);
        if (err) {
            ret = -1;
        }
        kfree(pkt_params);
    }
    printk(KERN_DEBUG "[touchpanel_diag_wrapper]%s: ioctl Exit\n", __func__);
    return ret;
}


static const struct file_operations diag_wrapper_fops = {
    .owner          = THIS_MODULE,
    .open           = diag_wrapper_open,
    .unlocked_ioctl = diag_wrapper_ioctl,
};

static struct miscdevice diag_wrapper = {
    .fops       = &diag_wrapper_fops,
    .name       = DEVICEWRPPER_NAME,
    .minor      = MISC_DYNAMIC_MINOR,
};

static int __init input_device_wrapper_init(void)
{
	printk(KERN_DEBUG "[touchpanel_diag_wrapper]%s: init Enter\n", __func__);
    return misc_register(&diag_wrapper);
}

static void __exit input_device_wrapper_exit(void)
{
    misc_deregister(&diag_wrapper);
}

module_init(input_device_wrapper_init);
module_exit(input_device_wrapper_exit);



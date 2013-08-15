/*
 * key_diag_wrapper.c
 *
 * Copyright (C) 2010 NEC Corporation
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
#include <linux/module.h>
#include <linux/ioctl.h>

#include <linux/keypad_cmd.h>
#include <linux/key_diag_wrapper.h>
#include <linux/input/pmic8xxx-pwrkey.h>

#include <linux/slab.h>


#define DEVICEWRPPER_NAME       "key_diag_wrapper"
#define KEY_TIMEOUT     (HZ * 30)

static wait_queue_head_t key_rand_wait;
static int g_rsp_key_data;

static int diag_wrapper_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "key_diag_wrapper_open\n");
    return 0;
}

void diag_keypad_cmd_rand_ctl(void *key_in)
{
    wait_queue_head_t   *wq = &key_rand_wait;
    keypad_cmd_callback_param_keycode *key_type;

    key_type = (keypad_cmd_callback_param_keycode *)key_in;

    g_rsp_key_data = key_type->key_code;

    wake_up(wq);
    return;
}

static long diag_wrapper_ioctl(struct file *file, unsigned int iocmd, unsigned long data)
{
    int ret = DIAG_WRAPPER_IOCTL_OK;
    unsigned char ret_val;





    int err;
    long    timeout;
    
    int *key_val = NULL;
    int key_mask;
    size_t req_size = sizeof(char);
    
    
    init_waitqueue_head(&key_rand_wait);

    printk(KERN_DEBUG "[key_diag_wrapper]%s: ioctl Enter (iocmd:0x%02X)\n", __func__,iocmd);

    switch(iocmd)
    {
    case DIAG_WRAPPER_IOCTL_A01:
        ret_val = keypad_cmd_callback(KEYPAD_CMD_TYPE_GET_KEYCODE, 0, diag_keypad_cmd_rand_ctl);
        if (ret_val != 1)
        {
            ret = 0;
        }
        else
        {
            g_rsp_key_data = 0xff;
            timeout = wait_event_timeout(key_rand_wait, g_rsp_key_data != 0xff, KEY_TIMEOUT);
            if(timeout == 0)
            {
                g_rsp_key_data = 0x00;
                key_mask = 0;
                keypad_cmd(KEYPAD_CMD_TYPE_MASK, &key_mask);
            }
            ret = g_rsp_key_data;
        }
        break;

    case DIAG_WRAPPER_IOCTL_A02:
        key_mask = 1;
        ret = keypad_cmd(KEYPAD_CMD_TYPE_MASK, &key_mask);
        break;

    case DIAG_WRAPPER_IOCTL_A03:
        key_mask = 0;
        ret = keypad_cmd(KEYPAD_CMD_TYPE_MASK, &key_mask);
        break;

    case DIAG_WRAPPER_IOCTL_A04:
        req_size = ( sizeof(int) * 4 );
        key_val = (int*)kmalloc(req_size, GFP_KERNEL);
        if(!key_val)
        {
            return -ENOMEM;
        }
        else
        {
            
        }

        err = copy_from_user( key_val, (int *)data, req_size );
        if (err)
        {
            kfree(key_val);
            return DIAG_WRAPPER_IOCTL_NG;
        }
        else
        {
            
        }
        
        if( key_val[3] == KEY_DIAG_EMULATION_KEY )
        {
            ret = keypad_cmd(KEYPAD_CMD_TYPE_KEY_EMULATION, key_val);
        }
        else if( key_val[3] == KEY_DIAG_EMULATION_HANDSET )
        {

            ret = pwr_key_cmd(key_val);
        }
        else
        {
            
        }
        
        kfree(key_val);
        
        break;
    default:
        ret = DIAG_WRAPPER_IOCTL_NG;
        break;
    }
    
    printk(KERN_DEBUG "[key_diag_wrapper]%s: ioctl Exit\n", __func__);
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
	printk(KERN_DEBUG "[key_diag_wrapper]%s: init Enter\n", __func__);
    return misc_register(&diag_wrapper);
}

static void __exit input_device_wrapper_exit(void)
{
    misc_deregister(&diag_wrapper);
}

module_init(input_device_wrapper_init);
module_exit(input_device_wrapper_exit);



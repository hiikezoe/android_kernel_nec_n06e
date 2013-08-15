/*
 * drivers/leds/leds-diag_wrapper.c
 *
 * Copyright (C) NEC CASIO Mobile Communications, Ltd.
 *
 */

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
#include <linux/leds.h>
#include <linux/ioctl.h>
#include <linux/leds-diag_wrapper.h>
#include <linux/leds_cmd.h>

#include <mach/rpc_server_handset.h>

#include <linux/slab.h>
#include <linux/leds-adp8861.h>
#include <linux/leds-lcd-common.h>

#define LEDDIAG_NAME       "led_diag_wrapper"

#if !defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)

#define LOCAL_CONFIG_FEATURE_DVE021_DVE902

#endif 


#define LED_DIAG_DEBUG_PLUS


static int led_diag_wrapper_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "led_diag_wrapper_open\n");
    return 0;
}

static int diag_leds_cmd_ctl(unsigned char red_f,
                             unsigned char grn_f,
                             unsigned char ble_f)
{
    int ret = LEDS_CMD_RET_OK;
    unsigned char ret_val;

    ret_val = leds_cmd(LEDS_CMD_TYPE_RGB_RED, red_f);
    if (ret_val != LEDS_CMD_RET_OK)
    {
        ret = LEDS_CMD_RET_NG;
    }
    ret_val = leds_cmd(LEDS_CMD_TYPE_RGB_GREEN, grn_f);
    if (ret_val != LEDS_CMD_RET_OK)
    {
        ret = LEDS_CMD_RET_NG;
    }
    ret_val = leds_cmd(LEDS_CMD_TYPE_RGB_BLUE, ble_f);
    if (ret_val != LEDS_CMD_RET_OK)
    {
        ret = LEDS_CMD_RET_NG;
    }

    return ret;
}


static int diag_er_leds_cmd_ctl(unsigned char red_f,
                             unsigned char grn_f,
                             unsigned char ble_f)
{
    int ret = LEDS_CMD_RET_OK;
    unsigned char ret_val;

    ret_val = leds_cmd(LEDS_CMD_TYPE_ER_RGB_RED, red_f);
    if (ret_val != LEDS_CMD_RET_OK)
    {
        ret = LEDS_CMD_RET_NG;
    }
    ret_val = leds_cmd(LEDS_CMD_TYPE_ER_RGB_GREEN, grn_f);
    if (ret_val != LEDS_CMD_RET_OK)
    {
        ret = LEDS_CMD_RET_NG;
    }
    ret_val = leds_cmd(LEDS_CMD_TYPE_ER_RGB_BLUE, ble_f);
    if (ret_val != LEDS_CMD_RET_OK)
    {
        ret = LEDS_CMD_RET_NG;
    }

    return ret;
}



static long led_diag_wrapper_ioctl(struct file *file, unsigned int iocmd, unsigned long data)




{
    int ret = LED_DIAG_IOCTL_OK;
    int led_ret = LEDS_CMD_RET_OK;
    int err;
    unsigned char *pkt_params = NULL;
#if !defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)
    unsigned char read_data = 0;
#endif 

	struct led_request_rgb key_request;
	union u_led_isc_reg led_key_bright1, led_key_bright2, led_key_bright3;


    size_t res_size = sizeof(char);
    size_t req_size = sizeof(char);

    struct adp8861_3color_led_parame_nv  adp8861_3color_led_parame_nv;
    memset( &adp8861_3color_led_parame_nv, 0, sizeof(  struct adp8861_3color_led_parame_nv ));
    adp8861_get_3color_nv(&adp8861_3color_led_parame_nv);


    printk(KERN_DEBUG "[diag_wrapper]%s: ioctl Enter (iocmd:0x%02X)\n", __func__,iocmd);

    switch(iocmd){
    case LED_DIAG_IOCTL_01:		

        led_ret = diag_leds_cmd_ctl(adp8861_3color_led_parame_nv.illu_red_lumin.one, 0, 0);



        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_02:		

        led_ret = diag_leds_cmd_ctl(0, adp8861_3color_led_parame_nv.illu_green_lumin.one, 0);



        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_03:		

        led_ret = diag_leds_cmd_ctl(0, 0, adp8861_3color_led_parame_nv.illu_blue_lumin.one);



        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_04:		

        led_ret = diag_leds_cmd_ctl(adp8861_3color_led_parame_nv.illu_red_lumin.two, adp8861_3color_led_parame_nv.illu_green_lumin.two, 0);



        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_05:		

        led_ret = diag_leds_cmd_ctl(adp8861_3color_led_parame_nv.illu_red_lumin.two, 0, adp8861_3color_led_parame_nv.illu_blue_lumin.two);



        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_06:		

        led_ret = diag_leds_cmd_ctl(0, adp8861_3color_led_parame_nv.illu_green_lumin.two, adp8861_3color_led_parame_nv.illu_blue_lumin.two);



        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_07:		

        led_ret = diag_leds_cmd_ctl(adp8861_3color_led_parame_nv.illu_red_lumin.three, adp8861_3color_led_parame_nv.illu_green_lumin.three, adp8861_3color_led_parame_nv.illu_blue_lumin.three);



        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_08:		
        led_ret = diag_leds_cmd_ctl(0, 0, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

#if !defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)
    case LED_DIAG_IOCTL_09:		
        led_ret= leds_cmd(LEDS_CMD_TYPE_KEY, LEDS_LED_KEYBL1);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_10:		
        led_ret = leds_cmd(LEDS_CMD_TYPE_KEY, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;
#endif 

#if defined(LED_DIAG_DEBUG_PLUS)
    case LED_DIAG_IOCTL_14:		
        led_ret = leds_cmd(LEDS_CMD_TYPE_FLASH_STILL, LEDS_SND_CAM_FLASH_LED1);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_15:		
        led_ret = leds_cmd(LEDS_CMD_TYPE_FLASH_MOVIE, LEDS_SND_CAM_FLASH_LED2);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_16:		
        led_ret = leds_cmd(LEDS_CMD_TYPE_FLASH_TORCH, LEDS_SND_CAM_FLASH_LED3);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_17:		
        led_ret = leds_cmd(LEDS_CMD_TYPE_FLASH, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_18:		
        led_ret = leds_cmd(LEDS_CMD_TYPE_PREVENT_PEEPING, LEDS_LED_CAM_IND);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_19:		
        led_ret = leds_cmd(LEDS_CMD_TYPE_PREVENT_PEEPING, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;
#endif	


#if !defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)
    case LED_DIAG_IOCTL_12:		



		led_ret = lm3537_main_lcd_don( LM3537_LED_ON );

        if( led_ret < 0 ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;
#endif 

#if !defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)
    case LED_DIAG_IOCTL_13:		



		led_ret = lm3537_main_lcd_don( LM3537_LED_OFF );

        if( led_ret < 0 ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_46:
        req_size = ( sizeof(char) * 2 );
        pkt_params = (char *)kmalloc(req_size, GFP_KERNEL);
        if( !pkt_params )
            return -ENOMEM;

        err = copy_from_user( pkt_params, (unsigned char *)data, req_size );
        if (err) {
            kfree(pkt_params);
            return LED_DIAG_IOCTL_NG;
        }









        ret = lm3537_i2c_smbus_write(0xA0, pkt_params[0]);

		ret |= lm3537_i2c_smbus_write( 0x10, 0xFF );
		ret |= lm3537_i2c_smbus_write( 0x00, 0x04 );
        if( ret != LM3537_LED_SET_OK )
            ret = LED_DIAG_IOCTL_NG;

        break;
#endif 

#if !defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)
    case LED_DIAG_IOCTL_30:		
        req_size = ( sizeof(char) * 2 );
        pkt_params = (char *)kmalloc(req_size, GFP_KERNEL);
        if( !pkt_params ){
            return -ENOMEM;
        }
        err = copy_from_user( pkt_params, (unsigned char *)data, req_size );
        if (err) {
            kfree(pkt_params);
            return LED_DIAG_IOCTL_NG;
        }



        ret = lm3537_i2c_smbus_write(pkt_params[0], pkt_params[1]);

        break;

    case LED_DIAG_IOCTL_31:		
        req_size = ( sizeof(char) * 2 );
        pkt_params = (char *)kmalloc(req_size, GFP_KERNEL);
        if( !pkt_params ){
            return -ENOMEM;
        }
        err = copy_from_user( pkt_params, (unsigned char *)data, req_size );
        if (err) {
            kfree(pkt_params);
            return LED_DIAG_IOCTL_NG;
        }

        if(pkt_params[0] != 0x00){



			ret = lm3537_i2c_smbus_read(pkt_params[0], &read_data);

		}
		pkt_params[1] = read_data;
        break;
#endif 

    case LED_DIAG_IOCTL_32:		
        req_size = ( sizeof(char) * 2 );
        pkt_params = (char *)kmalloc(req_size, GFP_KERNEL);
        if( !pkt_params ){
            return -ENOMEM;
        }
        err = copy_from_user( pkt_params, (unsigned char *)data, req_size );
        if (err) {
            kfree(pkt_params);
            return LED_DIAG_IOCTL_NG;
        }

        ret = adp8861_reg_write(pkt_params[0], pkt_params[1], ID_0);



        break;

    case LED_DIAG_IOCTL_33:		
        req_size = ( sizeof(char) * 2 );
        pkt_params = (char *)kmalloc(req_size, GFP_KERNEL);
        if( !pkt_params ){
            return -ENOMEM;
        }
        err = copy_from_user( pkt_params, (unsigned char *)data, req_size );
        if (err) {
            kfree(pkt_params);
            return LED_DIAG_IOCTL_NG;
        }

        ret = adp8861_i2c_smbus_read(pkt_params[0], &pkt_params[1], ID_0);



        break;

    case LED_DIAG_IOCTL_47:		
        led_ret= leds_cmd(LEDS_CMD_TYPE_PREVENT_PEEPING, LEDS_LED_CAM_IND);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_48:		
        led_ret = leds_cmd(LEDS_CMD_TYPE_PREVENT_PEEPING, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_49:		
        led_ret = leds_cmd(LEDS_CMD_TYPE_FLASH_MOVIE, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_50:		
        led_ret = leds_cmd(LEDS_CMD_TYPE_FLASH_MOVIE, LEDS_SND_CAM_FLASH_LED2);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_51:		
        req_size = ( sizeof(char) * 2 );
        pkt_params = (char *)kmalloc(req_size, GFP_KERNEL);
        if( !pkt_params ){
            return -ENOMEM;
        }
        err = copy_from_user( pkt_params, (unsigned char *)data, req_size );
        if (err) {
            kfree(pkt_params);
            return LED_DIAG_IOCTL_NG;
        }
        ret = adp8861_reg_write(pkt_params[0], pkt_params[1], ID_1);
        break;

    case LED_DIAG_IOCTL_52:		
        req_size = ( sizeof(char) * 2 );
        pkt_params = (char *)kmalloc(req_size, GFP_KERNEL);
        if( !pkt_params ){
            return -ENOMEM;
        }
        err = copy_from_user( pkt_params, (unsigned char *)data, req_size );
        if (err) {
            kfree(pkt_params);
            return LED_DIAG_IOCTL_NG;
        }
        ret = adp8861_i2c_smbus_read(pkt_params[0], &pkt_params[1], ID_1);
        break;

    case LED_DIAG_IOCTL_53:		
        led_ret = diag_er_leds_cmd_ctl(adp8861_3color_led_parame_nv.hsj_red_lumin.one, 0, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_54:		
        led_ret = diag_er_leds_cmd_ctl(0, adp8861_3color_led_parame_nv.hsj_green_lumin.one, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_55:		
        led_ret = diag_er_leds_cmd_ctl(0, 0, adp8861_3color_led_parame_nv.hsj_blue_lumin.one);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_56:		
        led_ret = diag_er_leds_cmd_ctl(adp8861_3color_led_parame_nv.hsj_red_lumin.two, adp8861_3color_led_parame_nv.hsj_green_lumin.two, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_57:		
        led_ret = diag_er_leds_cmd_ctl(adp8861_3color_led_parame_nv.hsj_red_lumin.two, 0, adp8861_3color_led_parame_nv.hsj_blue_lumin.two);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_58:		
        led_ret = diag_er_leds_cmd_ctl(0, adp8861_3color_led_parame_nv.hsj_green_lumin.two, adp8861_3color_led_parame_nv.hsj_blue_lumin.two);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_59:		
        led_ret = diag_er_leds_cmd_ctl(adp8861_3color_led_parame_nv.hsj_red_lumin.three, adp8861_3color_led_parame_nv.hsj_green_lumin.three, adp8861_3color_led_parame_nv.hsj_blue_lumin.three);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_60:		
        led_ret = diag_er_leds_cmd_ctl(0, 0, 0);
        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;



    case LED_DIAG_IOCTL_90:		
		led_key_bright1.us = 0;
		led_key_bright1.st2.set_flag = 1;

		led_key_bright1.st2.scd = adp8861_3color_led_parame_nv.keybl1_red_lumin.one;



		led_key_bright2.us = 0;
		led_key_bright2.st2.set_flag = 1;
		led_key_bright2.st2.scd = 0x00;
		led_key_bright3.us = 0;
		led_key_bright3.st2.set_flag = 1;
		led_key_bright3.st2.scd = 0x00;

		led_ret  = adp8861_right_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );
		led_key_bright1.st2.scd = adp8861_3color_led_parame_nv.keybl2_red_lumin.one;
		led_ret |= adp8861_left_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );



		key_request.dmy1  = 0;
		key_request.set_r = 1;
		key_request.set_g = 0;
		key_request.set_b = 0;

		led_ret |= adp8861_right_key_led_set( &key_request );
		led_ret |= adp8861_left_key_led_set( &key_request );



        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_91:		
		led_key_bright1.us = 0;
		led_key_bright1.st2.set_flag = 1;
		led_key_bright1.st2.scd = 0x00;
		led_key_bright2.us = 0;
		led_key_bright2.st2.set_flag = 1;

		led_key_bright2.st2.scd = adp8861_3color_led_parame_nv.keybl1_green_lumin.one;



		led_key_bright3.us = 0;
		led_key_bright3.st2.set_flag = 1;
		led_key_bright3.st2.scd = 0x00;

		led_ret  = adp8861_right_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );
		led_key_bright2.st2.scd = adp8861_3color_led_parame_nv.keybl2_green_lumin.one;
		led_ret |= adp8861_left_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );



		key_request.dmy1  = 0;
		key_request.set_r = 0;
		key_request.set_g = 1;
		key_request.set_b = 0;

		led_ret |= adp8861_right_key_led_set( &key_request );
		led_ret |= adp8861_left_key_led_set( &key_request );



        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_92:		
		led_key_bright1.us = 0;
		led_key_bright1.st2.set_flag = 1;
		led_key_bright1.st2.scd = 0x00;
		led_key_bright2.us = 0;
		led_key_bright2.st2.set_flag = 1;
		led_key_bright2.st2.scd = 0x00;
		led_key_bright3.us = 0;
		led_key_bright3.st2.set_flag = 1;

		led_key_bright3.st2.scd = adp8861_3color_led_parame_nv.keybl1_blue_lumin.one;




		led_ret  = adp8861_right_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );
		led_key_bright3.st2.scd = adp8861_3color_led_parame_nv.keybl2_blue_lumin.one;
		led_ret |= adp8861_left_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );



		key_request.dmy1  = 0;
		key_request.set_r = 0;
		key_request.set_g = 0;
		key_request.set_b = 1;

		led_ret |= adp8861_right_key_led_set( &key_request );
		led_ret |= adp8861_left_key_led_set( &key_request );



        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_93:		
		led_key_bright1.us = 0;
		led_key_bright1.st2.set_flag = 1;

		led_key_bright1.st2.scd = adp8861_3color_led_parame_nv.keybl1_red_lumin.two;
		led_key_bright2.us = 0;
		led_key_bright2.st2.set_flag = 1;
		led_key_bright2.st2.scd = adp8861_3color_led_parame_nv.keybl1_green_lumin.two;
		led_key_bright3.us = 0;







		led_key_bright3.st2.set_flag = 1;
		led_key_bright3.st2.scd = 0x00;

		led_ret  = adp8861_right_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );
		led_key_bright1.st2.scd = adp8861_3color_led_parame_nv.keybl2_red_lumin.two;
		led_key_bright2.st2.scd = adp8861_3color_led_parame_nv.keybl2_green_lumin.two;
		led_ret |= adp8861_left_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );



		key_request.dmy1  = 0;
		key_request.set_r = 1;
		key_request.set_g = 1;
		key_request.set_b = 0;

		led_ret |= adp8861_right_key_led_set( &key_request );
		led_ret |= adp8861_left_key_led_set( &key_request );



        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_94:		
		led_key_bright1.us = 0;
		led_key_bright1.st2.set_flag = 1;

		led_key_bright1.st2.scd = adp8861_3color_led_parame_nv.keybl1_red_lumin.two;



		led_key_bright2.us = 0;
		led_key_bright2.st2.set_flag = 1;
		led_key_bright2.st2.scd = 0x00;
		led_key_bright3.us = 0;
		led_key_bright3.st2.set_flag = 1;

		led_key_bright3.st2.scd = adp8861_3color_led_parame_nv.keybl1_blue_lumin.two;




		led_ret  = adp8861_right_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );
		led_key_bright1.st2.scd = adp8861_3color_led_parame_nv.keybl2_red_lumin.two;
		led_key_bright3.st2.scd = adp8861_3color_led_parame_nv.keybl2_blue_lumin.two;
		led_ret |= adp8861_left_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );



		key_request.dmy1  = 0;
		key_request.set_r = 1;
		key_request.set_g = 0;
		key_request.set_b = 1;

		led_ret |= adp8861_right_key_led_set( &key_request );
		led_ret |= adp8861_left_key_led_set( &key_request );



        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_95:		
		led_key_bright1.us = 0;
		led_key_bright1.st2.set_flag = 1;
		led_key_bright1.st2.scd = 0x00;
		led_key_bright2.us = 0;
		led_key_bright2.st2.set_flag = 1;

		led_key_bright2.st2.scd = adp8861_3color_led_parame_nv.keybl1_green_lumin.two;



		led_key_bright3.us = 0;
		led_key_bright3.st2.set_flag = 1;

		led_key_bright3.st2.scd = adp8861_3color_led_parame_nv.keybl1_blue_lumin.two;




		led_ret  = adp8861_right_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );
		led_key_bright2.st2.scd = adp8861_3color_led_parame_nv.keybl2_green_lumin.two;
		led_key_bright3.st2.scd = adp8861_3color_led_parame_nv.keybl2_blue_lumin.two;
		led_ret |= adp8861_left_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );



		key_request.dmy1  = 0;
		key_request.set_r = 0;
		key_request.set_g = 1;
		key_request.set_b = 1;

		led_ret |= adp8861_right_key_led_set( &key_request );
		led_ret |= adp8861_left_key_led_set( &key_request );



        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_96:		
		led_key_bright1.us = 0;
		led_key_bright1.st2.set_flag = 1;

		led_key_bright1.st2.scd = adp8861_3color_led_parame_nv.keybl1_red_lumin.three;



		led_key_bright2.us = 0;
		led_key_bright2.st2.set_flag = 1;

		led_key_bright2.st2.scd = adp8861_3color_led_parame_nv.keybl1_green_lumin.three;



		led_key_bright3.us = 0;
		led_key_bright3.st2.set_flag = 1;

		led_key_bright3.st2.scd = adp8861_3color_led_parame_nv.keybl1_blue_lumin.three;




		led_ret  = adp8861_right_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );
		led_key_bright1.st2.scd = adp8861_3color_led_parame_nv.keybl2_red_lumin.three;
		led_key_bright2.st2.scd = adp8861_3color_led_parame_nv.keybl2_green_lumin.three;
		led_key_bright3.st2.scd = adp8861_3color_led_parame_nv.keybl2_blue_lumin.three;
		led_ret |= adp8861_left_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );



		key_request.dmy1  = 0;
		key_request.set_r = 1;
		key_request.set_g = 1;
		key_request.set_b = 1;

		led_ret |= adp8861_right_key_led_set( &key_request );
		led_ret |= adp8861_left_key_led_set( &key_request );



        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;

    case LED_DIAG_IOCTL_97:		
		led_key_bright1.us = 0;
		led_key_bright1.st2.set_flag = 1;
		led_key_bright1.st2.scd = 0x00;
		led_key_bright2.us = 0;
		led_key_bright2.st2.set_flag = 1;
		led_key_bright2.st2.scd = 0x00;
		led_key_bright3.us = 0;
		led_key_bright3.st2.set_flag = 1;
		led_key_bright3.st2.scd = 0x00;

		led_ret  = adp8861_right_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );
		led_ret |= adp8861_left_key_led_bright( &led_key_bright1, &led_key_bright2, &led_key_bright3 );



		key_request.dmy1  = 0;
		key_request.set_r = 0;
		key_request.set_g = 0;
		key_request.set_b = 0;

		led_ret |= adp8861_right_key_led_set( &key_request );
		led_ret |= adp8861_left_key_led_set( &key_request );



        if( led_ret != LEDS_CMD_RET_OK ){
            ret = LED_DIAG_IOCTL_NG;
        }
        break;


    default:
        ret = LED_DIAG_IOCTL_NG;
        break;
    }

    if(pkt_params){
        err = copy_to_user((unsigned char *)data, pkt_params, res_size);
        if (err) {
            ret = -1;
        }
        kfree(pkt_params);
    }
    printk(KERN_DEBUG "[diag_wrapper]%s: ioctl Exit\n", __func__);
    return ret;
}

static const struct file_operations led_diag_wrapper_fops = {
    .owner      = THIS_MODULE,
    .open       = led_diag_wrapper_open,

    .unlocked_ioctl = led_diag_wrapper_ioctl,
};

static struct miscdevice led_diag = {
    .fops       = &led_diag_wrapper_fops,
    .name       = LEDDIAG_NAME,
    .minor      = MISC_DYNAMIC_MINOR,
};

static int __init led_diag_wrapper_init(void)
{
	printk(KERN_DEBUG "[led_diag]%s: init Enter\n", __func__);
    return misc_register(&led_diag);
}

static void __exit led_diag_wrapper_exit(void)
{
    misc_deregister(&led_diag);
}

module_init(led_diag_wrapper_init);
module_exit(led_diag_wrapper_exit);



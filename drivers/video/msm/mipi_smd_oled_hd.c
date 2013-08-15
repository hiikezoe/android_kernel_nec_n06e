/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
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
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_smd_oled_hd.h"

#include <linux/delay.h>
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#include <../../../arch/arm/mach-msm/board-8064.h>
#include <../../../arch/arm/mach-msm/devices.h>


#include <linux/oemnc_info.h>

#include <linux/regulator/gpio-regulator.h>
#include <linux/mfd/pm8xxx/pm8921.h>
#include <linux/regulator/consumer.h>

#include <linux/pm_obs_api.h>

#define MIPI_SMD_MAX_GAMMA_LV 30
#define GAMMA_CNT_MAX 26


static struct msm_panel_common_pdata *mipi_smd_oled_hd_pdata;

static struct dsi_buf smd_oled_hd_tx_buf;
static struct dsi_buf smd_oled_hd_rx_buf;

static int ch_used[3];


static mipi_smd_oled_hd_state_t mipi_smd_oled_hd_state = MIPI_SMD_OLED_HD_STATE_OFF; 


static mipi_smd_oled_hd_hw_rev_t hw_rev = 0;



static struct regulator *vreg_l_2  = NULL;
static struct regulator *vreg_l_16 = NULL;
static struct regulator *vreg_l_17 = NULL;
static boolean pm_gpio_25_requested = FALSE;


static int mipi_dsi_oled_hd_acl_setting = 1; 


static boolean mipi_smd_oled_gamma_request_flg = FALSE;


static boolean mipi_smd_oled_pt_bkl_lock = 1;
extern boolean msm_fb_disable_sleep;


static boolean mipi_smd_vreg_l2_locked = FALSE;
static boolean oled_is_force_poweroff = FALSE;



static DEFINE_MUTEX(mipi_smd_oled_cmd_lock);



static void mipi_smd_oled_hd_set_display_on( struct msm_fb_data_type *mfd );
static void mipi_smd_oled_hd_set_display_off( struct msm_fb_data_type *mfd );
int mipi_smd_oled_hd_read_dev_reg( struct msm_fb_data_type *mfd, struct msmfb_register_read *data);


int mipi_smd_oled_hd_read_hw_rev( void );






static char mtp_key_com_1[] = {
    0xF0, 
    0x5A, 
    0x5A, 
};

static char mtp_key_com_2[] = {
    0xF1, 
    0x5A, 
    0x5A, 
};


static char panel_control[] = {
    0xF8, 
    0x3D, 
    0x33, 
    0x00, 
    0x00, 
    0x00, 
    0x8F, 
    0x00, 
    0x3B, 
    0x7A, 
    0x08, 
    0x26, 
    0x7A, 
    0x3D, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x04, 
    0x08, 
    0x6B, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x07, 
    0x07, 
    0x22, 
    0x22, 
    0xC0, 
    0xC8, 
    0x08, 
    0x48, 
    0xC1, 
    0x00, 
    0xC1, 
    0xFF, 
    0xFF, 
    0xC8, 
};



static char panel_control_new[] = {
    0xF8, 
    0x19, 
    0x33, 
    0x00, 
    0x00, 
    0x00, 
    0x8F, 
    0x00, 
    0x3B, 
    0x7A, 
    0x08, 
    0x26, 
    0x7A, 
    0x3D, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x04, 
    0x08, 
    0x6B, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x07, 
    0x07, 
    0x22, 
    0x22, 
    0xC0, 
    0xC1, 
    0x01, 
    0x81, 
    0xC1, 
    0x00, 
    0xC1, 
    0xF6, 
    0xF6, 
    0xC1, 
};


static char display_condition[] = {
    0xF2, 
    0x80, 
    0x03, 
    0x0D, 
};


static char default_gamma_condition_300[] = {
    0xFA, 
    0x01, 
    0x58, 
    0x41, 
    0x59, 
    0xA8, 
    0xB6, 
    0xA6, 
    0xA9, 
    0xB7, 
    0xA5, 
    0xBF, 
    0xC3, 
    0xBE, 
    0x92, 
    0x96, 
    0x8F, 
    0xAC, 
    0xAE, 
    0xAB, 
    0x00, 
    0xBA, 
    0x00, 
    0xB2, 
    0x00, 
    0xD5, 
};


static char default_gamma_condition_300_new[] = {
    0xFA, 
    0x01, 
    0x80, 
    0x80, 
    0x80, 
    0x80, 
    0x80, 
    0x80, 
    0x80, 
    0x80, 
    0x80, 
    0x80, 
    0x80, 
    0x80, 
    0x80, 
    0x80, 
    0x80, 
    0x80, 
    0x80, 
    0x80, 
    0x01, 
    0x00, 
    0x01, 
    0x00, 
    0x01, 
    0x00, 
};



static char default_gamma_condition_20[] = {

    0xFA, 
    0x01, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
};


static char gamma_ltps_update[] = {
    0xF7, 
    0x03, 
};

static char source_ctl[] = {
    0xF6, 
    0x00, 
    0x02, 
    0x00, 
};

static char pen_tile_ctl[] = {
    0xB6, 
    0x0C, 
    0x02, 
    0x03, 
    0x32, 
    0xFF, 
    0x44, 
    0x44, 
    0xC0, 
    0x00, 
};

static char nvm_setting[] = {
    0xD9, 
    0x14, 
    0x40, 
    0x0C, 
    0xCB, 
    0xCE, 
    0x6E, 
    0xC4, 
    0x07, 
    0x40, 
    0x41, 
    0xCB, 
    0x00, 
    0x60, 
    0x19, 
};

static char power_ctl[] = {
    0xF4, 
    0xCF, 
    0x0A, 
    0x12, 
    0x10, 
    0x1E, 
    0x33, 
    0x02, 
};


static char elvss_read[] = {
    0xDC, 
    0x00, 
};


static char dynamic_elvss_ctl[] = {
    0xB1, 
    0x04, 
    0x86, 
};

static char acl_control_2[] = {
    0xC1, 
    0x47, 
    0x53, 
    0x13, 
    0x53, 
    0x00, 
    0x00, 
    0x02, 
    0xCF, 
    0x00, 
    0x00, 
    0x04, 
    0xFF, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x01, 
    0x03, 
    0x07, 
    0x0C, 
    0x12, 
    0x19, 
    0x1F, 
    0x26, 
};


static char acl_off[] = {
    0xC0, 
    0x00, 
};


static char acl_on[] = {
    0xC0, 
    0x01, 
};


static char sleep_in[] = {
    0x10, 
    0x00, 
};


static char sleep_out[] = {
    0x11, 
    0x00, 
};


static char display_on[] = {
    0x29, 
    0x00, 
};


static char display_off[] = {
    0x28, 
    0x00, 
};


static char read_position[] = {
    0xB0, 
    0x00, 
};


static char packet_size[] = { 
    0x04,
    0x00,
};

static char read_target_reg[] = {
    0xFF,
    0x00,
};



static struct dsi_cmd_desc mipi_smd_oled_hd_initialize_cmds[] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(mtp_key_com_1), mtp_key_com_1},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(mtp_key_com_2), mtp_key_com_2},
    {DTYPE_DCS_WRITE,  1, 0, 0, 20, sizeof(sleep_out), sleep_out},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(panel_control), panel_control},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(display_condition), display_condition},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(default_gamma_condition_300), default_gamma_condition_300},
    {DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(gamma_ltps_update), gamma_ltps_update},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(source_ctl), source_ctl},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(pen_tile_ctl), pen_tile_ctl},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(nvm_setting), nvm_setting},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(power_ctl), power_ctl},





};


static struct dsi_cmd_desc mipi_smd_oled_hd_elvss_read =
    {DTYPE_DCS_READ, 1, 0, 1, 5,  sizeof(elvss_read), elvss_read};

static struct dsi_cmd_desc mipi_smd_oled_hd_dynamic_elvss_ctl =
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(dynamic_elvss_ctl), dynamic_elvss_ctl};


static struct dsi_cmd_desc mipi_smd_oled_hd_initialize_cmds_black[] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(mtp_key_com_1), mtp_key_com_1},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(mtp_key_com_2), mtp_key_com_2},
    {DTYPE_DCS_WRITE,  1, 0, 0, 20, sizeof(sleep_out), sleep_out},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(panel_control), panel_control},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(display_condition), display_condition},

    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(default_gamma_condition_20), default_gamma_condition_20},

    {DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(gamma_ltps_update), gamma_ltps_update},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(source_ctl), source_ctl},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(pen_tile_ctl), pen_tile_ctl},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(nvm_setting), nvm_setting},
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(power_ctl), power_ctl},





};



static struct dsi_cmd_desc mipi_smd_oled_hd_sleep_in_cmd =
    {DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(sleep_in), sleep_in};


static struct dsi_cmd_desc mipi_smd_oled_hd_dispon_cmd =
    {DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(display_on), display_on};


static struct dsi_cmd_desc mipi_smd_oled_hd_dispoff_cmd =
    {DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(display_off), display_off};


static struct dsi_cmd_desc mipi_smd_oled_hd_acl_enable_cmds[] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0,  sizeof(acl_control_2), acl_control_2},
    {DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(acl_on), acl_on},
};


static struct dsi_cmd_desc mipi_smd_oled_hd_acl_disable_cmd =
    {DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(acl_off), acl_off};



static char mipi_smd_oled_pt_gamma_value = 0; 



char gamma_nv_table[30][26];

static struct dsi_cmd_desc mipi_smd_oled_hd_gamma_table[MIPI_SMD_MAX_GAMMA_LV] = {
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[0]},   
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[1]},   
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[2]},   
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[3]},   
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[4]},   
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[5]},   
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[6]},   
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[7]},   
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[8]},   
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[9]},   
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[10]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[11]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[12]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[13]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[14]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[15]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[16]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[17]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[18]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[19]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[20]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[21]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[22]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[23]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[24]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[25]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[26]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[27]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[28]},  
    {DTYPE_DCS_LWRITE, 1, 0, 0, 0, 26, gamma_nv_table[29]},  
};

static struct dsi_cmd_desc mipi_smd_oled_hd_gamma_ltps_update_cmd = 
    {DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(gamma_ltps_update), gamma_ltps_update};


static struct dsi_cmd_desc mipi_smd_oled_hd_read_reg_cmd = 
    { DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(read_target_reg), read_target_reg};


static struct dsi_cmd_desc mipi_smd_oled_hd_packet_size_cmd = 
    { DTYPE_MAX_PKTSIZE, 1, 0, 0, 0, sizeof(packet_size), packet_size};


static struct dsi_cmd_desc mipi_smd_oled_hd_read_pos_cmd = 
    { DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(read_position), read_position};










static void mipi_smd_oled_hd_state_transition( mipi_smd_oled_hd_state_t current_value,
                                               mipi_smd_oled_hd_state_t set_value )
{
    if (mipi_smd_oled_hd_state != current_value)
    {
        
        printk(KERN_ERR "%s. (%d->%d) found %d\n", __func__, current_value,
                                                   set_value, mipi_smd_oled_hd_state);
    }

    mipi_smd_oled_hd_state = set_value;
    return;
}










void mipi_smd_oled_hd_pt_gamma_setting(uint8 *data, int loop_cnt)
{
    int cnt;

    gamma_nv_table[loop_cnt][0] = 0xFA;
    gamma_nv_table[loop_cnt][1] = 0x01;

    
    for (cnt = 2; cnt < GAMMA_CNT_MAX; cnt++)
    {
        gamma_nv_table[loop_cnt][cnt] = data[cnt - 2];
    }


    if (loop_cnt == 0) {
        for (cnt = 2; cnt < GAMMA_CNT_MAX; cnt++) {
            
            default_gamma_condition_20[cnt] = gamma_nv_table[0][cnt];
        }
    }


    if( loop_cnt == 29)
	{
        mipi_smd_oled_pt_bkl_lock = FALSE;
		printk(KERN_INFO "%s. OLED Gamma Table Setting Done.\n", __func__);
	}
    return;
}












static void mipi_smd_oled_hd_send_tx_command(struct dsi_cmd_desc *cmds, int size)
{
	int ret;
	int cnt;

	mutex_lock(&mipi_smd_oled_cmd_lock);

	for (cnt = 0; cnt < 10; cnt++) {
		ret = mipi_dsi_cmds_tx(&smd_oled_hd_tx_buf, cmds, size);

		if (ret != -EBUSY) {
			
			mutex_unlock(&mipi_smd_oled_cmd_lock);
			return;
		}
		usleep(1000); 
	}

	mutex_unlock(&mipi_smd_oled_cmd_lock);
	printk(KERN_ERR "%s. Failed to send dsi command!!\n", __func__);
}












static void mipi_smd_oled_hd_set_backlight(struct msm_fb_data_type *mfd)
{
    char gamma_lvl = 0;
    mipi_smd_oled_hd_state_t curr_state;
    int val;

    if ( (msm_fb_disable_sleep == TRUE && mfd->panel_info.type != DTV_PANEL) || (mipi_smd_oled_pt_bkl_lock == TRUE) )
        return ;


    curr_state = mipi_smd_oled_hd_state;

#if defined (CONFIG_FEATURE_DVE021_ONLY_FOR_PRODUCTION_PROCESS_DVE082)
    gamma_lvl = MIPI_SMD_MAX_GAMMA_LV;
#else
    if (mfd->bl_level >= MIPI_SMD_MAX_GAMMA_LV)
        gamma_lvl = MIPI_SMD_MAX_GAMMA_LV;
    else
        gamma_lvl = (char)mfd->bl_level;
#endif


	if (oled_is_force_poweroff)
		return;


    if (mipi_smd_oled_pt_gamma_value != gamma_lvl)
    {
        if (curr_state == MIPI_SMD_OLED_HD_STATE_NORMAL_MODE)
        {
            if (mfd->bl_level == 0)
            {
                
                mipi_smd_oled_hd_set_display_off(mfd);
                mipi_smd_oled_pt_gamma_value = 0;
                mipi_smd_oled_gamma_request_flg = TRUE; 

                pm_obs_a_lcdbacklight(0);
            } else {
                

               
               
				mipi_smd_oled_hd_send_tx_command(&(mipi_smd_oled_hd_gamma_table[gamma_lvl-1]), 1);
				mipi_smd_oled_hd_send_tx_command(&mipi_smd_oled_hd_gamma_ltps_update_cmd, 1);


                if (mipi_smd_oled_pt_gamma_value == 0)
                {
                    
                    mipi_smd_oled_hd_set_display_on(mfd);
                }

                
                mipi_smd_oled_pt_gamma_value = gamma_lvl;

                val = (gamma_lvl * 100) / MIPI_SMD_MAX_GAMMA_LV;
                pm_obs_a_lcdbacklight(val);
            }
        } else {
            
            printk(KERN_INFO "%s. backlight level change request. but device is not active.\n",__func__);
        }
    }

    return;
}










int mipi_smd_oled_hd_ctrl_gpio_25(int on)
{
    int ret;
    static int gpio25;

    gpio25 = PM8921_GPIO_PM_TO_SYS(25);

    if (pm_gpio_25_requested == FALSE)
    {
        ret = gpio_request(gpio25, "disp_rst_n");
        if (ret)
        {
            printk(KERN_ERR "%s. gpio_request() failed!!\n", __func__);
            return -ENODEV;
        }
        pm_gpio_25_requested = TRUE;
    }

    if (on)
        gpio_set_value_cansleep(gpio25, 1);
    else
        gpio_set_value_cansleep(gpio25, 0);

    return 0;
}












int mipi_smd_oled_hd_power_ctl( int on )
{
    int rc;

    
    if (mipi_smd_vreg_l2_locked != TRUE) {
	    if (vreg_l_2 == NULL)
	    {
	        
	        vreg_l_2 = regulator_get(&msm_mipi_dsi1_device.dev, "dsi1_pll_vdda");
	
	        if (IS_ERR_OR_NULL(vreg_l_2))
	        {
	            pr_err("could not get 8921_l2, rc = %ld\n", PTR_ERR(vreg_l_2));
	            return -ENODEV;
	        }
	
	        rc = regulator_set_voltage(vreg_l_2, 1200000, 1200000);
	        if (rc)
	        {
	            pr_err("set_voltage l2 failed, rc=%d\n", rc);
	            return -EINVAL;
	        }
	    }
	}

    if (vreg_l_16 == NULL)
    {
        
        vreg_l_16 = regulator_get(&msm_mipi_dsi1_device.dev, "dsi1_vci");
        if (IS_ERR(vreg_l_16))
        {
            pr_err("could not get 8921_l16, rc = %ld\n", PTR_ERR(vreg_l_16));
            return -ENODEV;
        }

        rc = regulator_set_voltage(vreg_l_16, 3300000, 3300000);
        if (rc)
        {
            pr_err("set_voltage l16 failed, rc=%d\n", rc);
            return -EINVAL;
        }
    }

    if (vreg_l_17 == NULL)
    {
        
        vreg_l_17 = regulator_get(&msm_mipi_dsi1_device.dev, "dsi1_vdd3");
        if (IS_ERR(vreg_l_17))
        {
            pr_err("could not get 8921_l17, rc = %ld\n", PTR_ERR(vreg_l_17));
            return -ENODEV;
        }

        rc = regulator_set_voltage(vreg_l_17, 2700000, 2700000);
        if (rc)
        {
            pr_err("set_voltage l17 failed, rc=%d\n", rc);
            return -EINVAL;
        }
    }

    
    if (on)
    {
        
	    if (mipi_smd_vreg_l2_locked != TRUE) {
	        
	        rc = regulator_set_optimum_mode(vreg_l_2, 100000);
	        if (rc < 0) 
	        {
	            pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
	            return -EINVAL;
	        }
	        rc = regulator_enable(vreg_l_2);
	        if (rc)
	        {
	            pr_err("enable l2 failed, rc=%d\n", rc);
	            return -ENODEV;
	        }
		}

        
        rc = regulator_enable(vreg_l_17);
        if (rc)
        {
            pr_err("enable l17 failed, rc=%d\n", rc);
            return -ENODEV;
        }

        
        rc = regulator_enable(vreg_l_16);
        if (rc)
        {
            pr_err("enable l16 failed, rc=%d\n", rc);
            return -ENODEV;
        }

        
        usleep(1000);

        
        mipi_smd_oled_hd_ctrl_gpio_25(1);

        



        usleep(12000);


        
        MIPI_OUTP(MIPI_DSI_BASE + 0xA8, 0x10000000);

    } else {
        
	    if (mipi_smd_vreg_l2_locked != TRUE) {
	        
	        rc = regulator_disable(vreg_l_2);
	        if (rc)
	        {
	            pr_err("disable reg_l2 failed, rc=%d\n", rc);
	            return -ENODEV;
	        }
		}

        
        rc = regulator_disable(vreg_l_16);
        if (rc)
        {
            pr_err("disable l16 failed, rc=%d\n", rc);
            return -ENODEV;
        }

        
        mipi_smd_oled_hd_ctrl_gpio_25(0);

        
        rc = regulator_disable(vreg_l_17);
        if (rc)
        {
            pr_err("disable l17 failed, rc=%d\n", rc);
            return -ENODEV;
        }
    }

    return 0;

}










int mipi_smd_oled_hd_acl_ctl(struct msm_fb_data_type *mfd, int on)
{

    if (mipi_smd_oled_hd_state == MIPI_SMD_OLED_HD_STATE_NORMAL_MODE)
    {
        
        if (!on) {
            

           
           
           
            mipi_smd_oled_hd_send_tx_command(&mipi_smd_oled_hd_acl_disable_cmd, 1);


        } else {
            

           
           
           
            mipi_smd_oled_hd_send_tx_command(mipi_smd_oled_hd_acl_enable_cmds,
                                             ARRAY_SIZE(mipi_smd_oled_hd_acl_enable_cmds));


        }
    }

    
    mipi_dsi_oled_hd_acl_setting = on;
    return 0;
}












static void mipi_smd_oled_hd_initialize_setup( struct msm_fb_data_type *mfd )
{
	int len = 4;
	char data;


    if (mipi_smd_oled_gamma_request_flg) {
        

       
       
       
       

       
       
		mipi_dsi_buf_init(&smd_oled_hd_rx_buf);

		mipi_smd_oled_hd_send_tx_command(mipi_smd_oled_hd_initialize_cmds_black,
				ARRAY_SIZE(mipi_smd_oled_hd_initialize_cmds_black));

		
		len = mipi_dsi_cmds_rx(mfd, &smd_oled_hd_tx_buf, &smd_oled_hd_rx_buf, &mipi_smd_oled_hd_elvss_read, len);
		data = *smd_oled_hd_rx_buf.data;
		if ((data & (1 << 7)) && ~(data & (1 << 6)))
		{
			dynamic_elvss_ctl[2] = data;
		}
		else
		{
			dynamic_elvss_ctl[2] = 0x8F;
		}

		mipi_smd_oled_hd_send_tx_command(&mipi_smd_oled_hd_dynamic_elvss_ctl, 1);



    } else {
        

       
       
       
       

       
       
		mipi_dsi_buf_init(&smd_oled_hd_rx_buf);

		mipi_smd_oled_hd_send_tx_command(mipi_smd_oled_hd_initialize_cmds,
				ARRAY_SIZE(mipi_smd_oled_hd_initialize_cmds));

		
		len = mipi_dsi_cmds_rx(mfd, &smd_oled_hd_tx_buf, &smd_oled_hd_rx_buf, &mipi_smd_oled_hd_elvss_read, len);
		data = *smd_oled_hd_rx_buf.data;
		if ((data & (1 << 7)) && ~(data & (1 << 6)))
		{
			dynamic_elvss_ctl[2] = data;
		}
		else
		{
			dynamic_elvss_ctl[2] = 0x8F;
		}

		mipi_smd_oled_hd_send_tx_command(&mipi_smd_oled_hd_dynamic_elvss_ctl, 1);



    }


    
    if (mipi_dsi_oled_hd_acl_setting)
    {
        


       
       
       
        mipi_smd_oled_hd_send_tx_command(mipi_smd_oled_hd_acl_enable_cmds, 
                                         ARRAY_SIZE(mipi_smd_oled_hd_acl_enable_cmds));


    } else {
        

      
      
      
        mipi_smd_oled_hd_send_tx_command(&mipi_smd_oled_hd_acl_disable_cmd, 1);


    }

    usleep(120000);
    return;
}










static void mipi_smd_oled_hd_set_display_on( struct msm_fb_data_type *mfd )
{
    

  
	mipi_smd_oled_hd_send_tx_command(&mipi_smd_oled_hd_dispon_cmd, 1);


    return;
}










static void mipi_smd_oled_hd_set_display_off( struct msm_fb_data_type *mfd )
{
    

   
	mipi_smd_oled_hd_send_tx_command(&mipi_smd_oled_hd_dispoff_cmd, 1);


    return;
}










static void mipi_smd_oled_hd_enter_standby( struct msm_fb_data_type *mfd )
{
    


  
	mipi_smd_oled_hd_send_tx_command(&mipi_smd_oled_hd_sleep_in_cmd, 1);


    msleep(120);
    return;
}










static int mipi_smd_oled_hd_on( struct platform_device *pdev )
{
    struct msm_fb_data_type *mfd;
    mipi_smd_oled_hd_state_t curr_state;

    printk(KERN_INFO "[In]%s. stat:%d \n", __func__, mipi_smd_oled_hd_state);

    mfd = platform_get_drvdata(pdev);

    if (!mfd)
        return -ENODEV;
    if (mfd->key != MFD_KEY)
        return -EINVAL;

    mipi_dsi_buf_init(&smd_oled_hd_tx_buf);

    
    curr_state = mipi_smd_oled_hd_state;

    switch (curr_state)
    {
        case MIPI_SMD_OLED_HD_STATE_OFF :
            
            mipi_smd_oled_hd_initialize_setup(mfd);
            
            mipi_smd_oled_hd_set_display_on(mfd);

            
            mipi_smd_oled_hd_state_transition(curr_state, MIPI_SMD_OLED_HD_STATE_NORMAL_MODE);
            break;

        case MIPI_SMD_OLED_HD_STATE_READY :
            
            printk(KERN_INFO "%s. Driver Status Error. %d.\n", __func__, curr_state);
            break;

        case MIPI_SMD_OLED_HD_STATE_STANDBY :
            
            printk(KERN_INFO "%s. Driver Status Error. %d.\n", __func__, curr_state);
            break;

        case MIPI_SMD_OLED_HD_STATE_NORMAL_MODE :
            
            break;

        default :
            printk(KERN_ERR "Invalid Status !! %d\n", __LINE__);
            break;
    }

    printk(KERN_INFO "[Out]%s. stat:%d \n", __func__, mipi_smd_oled_hd_state);
    return 0;
}










static int mipi_smd_oled_hd_off( struct platform_device *pdev )
{
    struct msm_fb_data_type *mfd;
    mipi_smd_oled_hd_state_t curr_state;

    printk(KERN_INFO "[In]%s. stat:%d \n", __func__, mipi_smd_oled_hd_state);

    mfd = platform_get_drvdata(pdev);

    if (!mfd)
        return -ENODEV;
    if (mfd->key != MFD_KEY)
        return -EINVAL;


#if !defined (CONFIG_FEATURE_DVE021_ONLY_FOR_PRODUCTION_PROCESS_DVE082)

    
    if (msm_fb_disable_sleep == TRUE && mfd->panel_info.type != DTV_PANEL)
        return 0;

#endif


    
    curr_state = mipi_smd_oled_hd_state;

    switch (curr_state)
    {
        case MIPI_SMD_OLED_HD_STATE_OFF :

			if (oled_is_force_poweroff) {
				
				oled_is_force_poweroff = FALSE;
			}

            break;

        case MIPI_SMD_OLED_HD_STATE_READY :
            
            printk(KERN_INFO "%s. Driver Status Error. %d.\n", __func__, curr_state);
            break;

        case MIPI_SMD_OLED_HD_STATE_STANDBY :
            
            printk(KERN_INFO "%s. Driver Status Error. %d.\n", __func__, curr_state);
            break;

        case MIPI_SMD_OLED_HD_STATE_NORMAL_MODE :
            if (mipi_smd_oled_pt_gamma_value != 0)
            {
                
                mipi_smd_oled_hd_set_display_off(mfd);
            }

            
            mipi_smd_oled_hd_enter_standby(mfd);

            
            mipi_smd_oled_hd_state_transition(curr_state, MIPI_SMD_OLED_HD_STATE_OFF);
            break;

        default :
            printk(KERN_ERR "Invalid Status !! %d\n", __LINE__);
            break;
    }

    printk(KERN_INFO "[Out]%s. stat:%d \n", __func__, mipi_smd_oled_hd_state);
    return 0;
}











static int __devinit mipi_smd_oled_hd_lcd_probe(struct platform_device *pdev)
{
    int cnt;

    if (pdev->id == 0) 
    {
        mipi_smd_oled_hd_pdata = pdev->dev.platform_data;
        return 0;
    }

    msm_fb_add_device(pdev);


    mipi_smd_oled_hd_read_hw_rev();
    if (hw_rev == MIPI_SMD_OLED_HD_HW_REV_NEW)
    {
        for( cnt = 0; cnt < 39; cnt++)
        {
		    panel_control[cnt] = panel_control_new[cnt];
        }
        for( cnt = 0; cnt < 26; cnt++)
        {
            default_gamma_condition_300[cnt] = default_gamma_condition_300_new[cnt];
        }
    }

    return 0;
}


static struct platform_driver this_driver = {
    .probe  = mipi_smd_oled_hd_lcd_probe,
    .driver = {
        .name   = "mipi_smd_oled_hd_hd",
    },
};

static struct msm_fb_panel_data smd_oled_hd_panel_data = {
    .on     = mipi_smd_oled_hd_on,
    .off    = mipi_smd_oled_hd_off,
    .set_backlight = mipi_smd_oled_hd_set_backlight,
};











int mipi_smd_oled_hd_device_register(struct msm_panel_info *pinfo, u32 channel, u32 panel)
{
    struct platform_device *pdev = NULL;
    int ret;

    if ( (channel >= 3) || ch_used[channel] )
        return -ENODEV;

    ch_used[channel] = TRUE;

    pdev = platform_device_alloc("mipi_smd_oled_hd_hd", (panel << 8)|channel);
    if (!pdev)
        return -ENOMEM;

    smd_oled_hd_panel_data.panel_info = *pinfo;

    ret = platform_device_add_data(pdev, &smd_oled_hd_panel_data,
                                   sizeof(smd_oled_hd_panel_data) );

    if (ret) {
        printk(KERN_ERR "%s: platform_device_add_data failed!\n", __func__);
        goto err_device_put;
    }

    ret = platform_device_add(pdev);
    if (ret) {
        printk(KERN_ERR "%s: platform_device_register failed!\n", __func__);
        goto err_device_put;
    }

    
    printk(KERN_INFO "Create mipi_smd_oled_hd_hd device success! \n");
    return 0;

err_device_put:
    platform_device_put(pdev);
    return ret;
}










static int __init mipi_smd_oled_hd_lcd_init(void)
{
    int ret;

    mipi_dsi_buf_alloc(&smd_oled_hd_tx_buf, DSI_BUF_SIZE);
    mipi_dsi_buf_alloc(&smd_oled_hd_rx_buf, DSI_BUF_SIZE);

    ret = platform_driver_register(&this_driver);
    return ret;
}











void mipi_smd_oled_hd_set_idle_state(struct msm_fb_data_type *mfd)
{
    mipi_smd_oled_hd_state_t curr_state;

    
    curr_state = mipi_smd_oled_hd_state;

    switch (curr_state)
    {
        case MIPI_SMD_OLED_HD_STATE_OFF :
            
            if (mipi_smd_oled_hd_power_ctl(1) != 0)
            {
                printk(KERN_ERR "%s. LCD Power On Request Failed!!\n", __func__);
                return;
            }

            
            mipi_smd_oled_gamma_request_flg = FALSE;
            mipi_smd_oled_hd_initialize_setup(mfd);
            mipi_smd_oled_gamma_request_flg = TRUE;

            
            mipi_smd_oled_hd_set_display_on(mfd);

            
            mipi_smd_oled_hd_state_transition(curr_state, MIPI_SMD_OLED_HD_STATE_NORMAL_MODE);
            break;

        case MIPI_SMD_OLED_HD_STATE_READY :
            
            mipi_smd_oled_hd_set_display_on(mfd);

            
            mipi_smd_oled_hd_state_transition(curr_state, MIPI_SMD_OLED_HD_STATE_NORMAL_MODE);
            break;

        case MIPI_SMD_OLED_HD_STATE_STANDBY :
            
            mipi_smd_oled_gamma_request_flg = FALSE;
            mipi_smd_oled_hd_initialize_setup(mfd);
            mipi_smd_oled_gamma_request_flg = TRUE;

            
            mipi_smd_oled_hd_set_display_on(mfd);

            
            mipi_smd_oled_hd_state_transition(curr_state, MIPI_SMD_OLED_HD_STATE_NORMAL_MODE);
            break;

        case MIPI_SMD_OLED_HD_STATE_NORMAL_MODE :
            
            break;

        default :
            printk(KERN_ERR "Invalid Status !! %d\n", __LINE__);
            break;
    }

    return;
}












int mipi_smd_oled_hd_power_seq(int on, struct msm_fb_data_type *mfd)
{
    mipi_smd_oled_hd_state_t curr_state;

    
    curr_state = mipi_smd_oled_hd_state;


    
    if (msm_fb_disable_sleep == TRUE && mfd->panel_info.type != DTV_PANEL)
        return 0;


    switch (curr_state)
    {
        case MIPI_SMD_OLED_HD_STATE_OFF :
            if (on)
            {
                
                if (mipi_smd_oled_hd_power_ctl(1) != 0)
                {
                    printk(KERN_ERR "%s. LCD Power On Request Failed!!\n", __func__);
                    return -1;
                }

                
                mipi_smd_oled_gamma_request_flg = FALSE;
                mipi_smd_oled_hd_initialize_setup(mfd);
                mipi_smd_oled_gamma_request_flg = TRUE;

                
                mipi_smd_oled_hd_set_display_on(mfd);

                
                mipi_smd_oled_hd_state_transition(curr_state, MIPI_SMD_OLED_HD_STATE_NORMAL_MODE);
            }
            break;

        case MIPI_SMD_OLED_HD_STATE_READY :
            
            printk(KERN_INFO "%s. Driver Status Error. %d.\n", __func__, curr_state);
            break;

        case MIPI_SMD_OLED_HD_STATE_STANDBY :
            
            printk(KERN_INFO "%s. Driver Status Error. %d.\n", __func__, curr_state);
            break;

        case MIPI_SMD_OLED_HD_STATE_NORMAL_MODE :
            if (!on)
            {
                
                mipi_smd_oled_hd_set_display_off(mfd);

                
                mipi_smd_oled_hd_enter_standby(mfd);

                
                if (mipi_smd_oled_hd_power_ctl(0) != 0)
                {
                    printk(KERN_ERR "%s. LCD Power Off Request Failed!!\n", __func__);
                    return -1;
                }

                
                MIPI_OUTP(MIPI_DSI_BASE + 0xA8, 0x00000000);

                
                mipi_smd_oled_hd_state_transition(curr_state, MIPI_SMD_OLED_HD_STATE_OFF);
            }
            break;

        default :
            printk(KERN_ERR "Invalid Status !! %d\n", __LINE__);
            break;
    }

    return 0;
}













int mipi_smd_oled_hd_power_ctl_custom(int on, struct msm_fb_data_type *mfd)
{
    mipi_smd_oled_hd_state_t curr_state;

    
    curr_state = mipi_smd_oled_hd_state;

    if (on)
    {
        if (curr_state == MIPI_SMD_OLED_HD_STATE_OFF)
        {
            if (mipi_smd_vreg_l2_locked == TRUE)
                mipi_smd_vreg_l2_locked = FALSE;

            
            if (mipi_smd_oled_hd_power_ctl(1) != 0)
            {
                printk(KERN_ERR "%s. LCD Power On Request Failed!!\n", __func__);
                return -1;
            }

            mipi_smd_oled_gamma_request_flg = FALSE;

            
            mipi_smd_oled_hd_initialize_setup(mfd);
            mipi_smd_oled_gamma_request_flg = TRUE;

            
            mipi_smd_oled_hd_set_display_on(mfd);

            
            oled_is_force_poweroff = FALSE;
            mipi_smd_oled_hd_state_transition(curr_state, MIPI_SMD_OLED_HD_STATE_NORMAL_MODE);

        }
    } else {

        if (curr_state == MIPI_SMD_OLED_HD_STATE_NORMAL_MODE)
        {
            
            mipi_smd_oled_hd_set_display_off(mfd);

            
            mipi_smd_oled_hd_enter_standby(mfd);

            
            mipi_smd_vreg_l2_locked = TRUE;
            if (mipi_smd_oled_hd_power_ctl(0) != 0)
            {
                printk(KERN_ERR "%s. LCD Power Off Request Failed!!\n", __func__);
                return -1;
            }

            
            MIPI_OUTP(MIPI_DSI_BASE + 0xA8, 0x00000000);

            
            oled_is_force_poweroff = TRUE;
            mipi_smd_oled_hd_state_transition(curr_state, MIPI_SMD_OLED_HD_STATE_OFF);
        }
    }

    return 0;
}











int mipi_smd_oled_hd_register_write_cmd( struct msm_fb_data_type *mfd, 
                                     struct msmfb_register_write *data )
{
    char send_data[255];
    struct dsi_cmd_desc param;

    mipi_dsi_buf_init(&smd_oled_hd_tx_buf);

    
    memcpy(send_data, (char*)(&data->data[0]), data->len);

    
    param.dtype = (int)data->di;
    param.last  = 1;
    param.vc    = 0;
    param.ack   = 0;
    param.wait  = 0;
    param.dlen  = (int)data->len;
    param.payload = send_data;

    
    mipi_dsi_cmds_tx(&smd_oled_hd_tx_buf, &param, 1);

    return 0;
}











int mipi_smd_oled_hd_register_read_cmd( struct msm_fb_data_type *mfd, 
                                        struct msmfb_register_read *data )
{
    int ret = 0;

    ret = mipi_smd_oled_hd_read_dev_reg(mfd, data);

    return ret;
}










int mipi_smd_oled_hd_user_request_ctrl( struct msmfb_request_parame *data )
{
    int ret = 0;

    switch( data->request )
    {
        case MSM_FB_REQUEST_OVERLAY_ALPHA:
            ret = copy_from_user(&mdp4_overlay_argb_enable, data->data, sizeof(mdp4_overlay_argb_enable));
            break;

        default:
            
            printk(KERN_ERR "%s user_request error", __func__);
            break;
    }

    return ret;
}










int mipi_smd_oled_hd_read_dev_reg( struct msm_fb_data_type *mfd, struct msmfb_register_read *data)
{
    const int one_read_size = 4;
    const int loop_limit = 16;
    int read_pos = 0;
    int readed_size = 0;
    int data_cnt = 0;
    int i,j;

    
    read_target_reg[0] = (char)data->w_data[0];
    packet_size[0] = (char)data->len;

    
    mipi_dsi_cmds_tx(&smd_oled_hd_tx_buf, &(mipi_smd_oled_hd_packet_size_cmd), 1);

    for (j = 0; j < loop_limit; j++)
    {
        read_position[1] = read_pos;

        
        if ( mipi_dsi_cmds_tx(&smd_oled_hd_tx_buf, &(mipi_smd_oled_hd_read_pos_cmd), 1) < 1 )
        {
            printk(KERN_ERR "%s. mipi_dsi_cmds_tx FAILED.\n", __func__);
            return -1;
        }

        mipi_dsi_op_mode_config(DSI_CMD_MODE);
        mipi_dsi_sw_reset();

        mipi_dsi_buf_init(&smd_oled_hd_rx_buf);

        
        readed_size = mipi_dsi_cmds_rx(mfd, &smd_oled_hd_tx_buf, &smd_oled_hd_rx_buf, &mipi_smd_oled_hd_read_reg_cmd, one_read_size);

        
        for (i = 0; i < readed_size; i++, data_cnt++)
        {
            if( data_cnt < data->len )
                data->r_data[data_cnt] = smd_oled_hd_rx_buf.data[i];
        }

        mipi_dsi_sw_reset();
        mipi_dsi_op_mode_config(DSI_VIDEO_MODE);

        
        read_pos += readed_size;
        if( read_pos > data->len )
            break;
    }

    return 0;
}











int mipi_smd_oled_hd_read_hw_rev(void)
{
    uint32_t __hw_rev = 0;
    __hw_rev = hw_revision_read();

    if( __hw_rev > HW_REV_0P2 )
    {
        hw_rev = MIPI_SMD_OLED_HD_HW_REV_NEW;
    }
    else
    {
        hw_rev = MIPI_SMD_OLED_HD_HW_REV_CURRENT;
    }

     return 0;
}

module_init(mipi_smd_oled_hd_lcd_init);

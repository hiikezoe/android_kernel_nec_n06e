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

#if !defined(MIPI_SMD_OLED_HD_H)
#define MIPI_SMD_OLED_HD_H


typedef enum {
    MIPI_SMD_OLED_HD_STATE_OFF,          
    MIPI_SMD_OLED_HD_STATE_READY,        
    MIPI_SMD_OLED_HD_STATE_STANDBY,      
    MIPI_SMD_OLED_HD_STATE_NORMAL_MODE,  
} mipi_smd_oled_hd_state_t;


typedef enum {
    MIPI_SMD_OLED_HD_HW_REV_CURRENT,     
    MIPI_SMD_OLED_HD_HW_REV_NEW,         
} mipi_smd_oled_hd_hw_rev_t;


int mipi_smd_oled_hd_device_register(struct msm_panel_info *pinfo, u32 channel, u32 panel);

int mipi_smd_oled_hd_acl_ctl(struct msm_fb_data_type *mfd, int on);
void mipi_smd_oled_hd_pt_gamma_setting(uint8 *data, int loop_cnt);


void mipi_smd_oled_hd_set_idle_state( struct msm_fb_data_type *mfd );

int mipi_smd_oled_hd_power_seq( int on, struct msm_fb_data_type *mfd );

int mipi_smd_oled_hd_user_request_ctrl( struct msmfb_request_parame *data );

int mipi_smd_oled_hd_register_write_cmd( struct msm_fb_data_type *mfd,
                                         struct msmfb_register_write *data );

int mipi_smd_oled_hd_register_read_cmd( struct msm_fb_data_type *mfd, 
                                        struct msmfb_register_read *data );

int mipi_smd_oled_hd_power_ctl( int on );


int mipi_smd_oled_hd_power_ctl_custom( int on, struct msm_fb_data_type *mfd);

#endif 

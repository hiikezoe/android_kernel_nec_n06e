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

static struct msm_panel_info pinfo;

static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
    



    
    
    { 0x03, 0x0A, 0x04, 0x00, 0x20 },
    
    { 0xB8, 0x38, 0x1F, 0x00, 0x97, 0x99, 0x22, 
      0x3A, 0x23, 0x03, 0x04, 0xA0 },
    
    { 0x5F, 0x00, 0x00, 0x10 },
    
    { 0xFF, 0x00, 0x06 },
    
    { 0x00, 0xE6, 0x31, 0xDA, 0x00, 0x50, 0x48, 
      0x63, 0x31, 0x0F, 0x03, 0x00, 0x14, 0x03, 
      0x00, 0x02, 0x00, 0x20, 0x00, 0x01        
    },
}; 

static int __init mipi_smd_video_hd_pt_init(void)
{
    int ret;

    if (msm_fb_detect_client("mipi_smd_video_hd_pt"))
        return 0;

    pinfo.xres = 720;
    pinfo.yres = 1280;

    pinfo.type = MIPI_VIDEO_PANEL;
    pinfo.pdest = DISPLAY_1;
    pinfo.wait_cycle = 0;
    pinfo.bpp = 24;

    pinfo.lcdc.h_back_porch = 152;
    pinfo.lcdc.h_front_porch = 168;
    pinfo.lcdc.h_pulse_width = 4;
    pinfo.lcdc.v_back_porch = 1;
    pinfo.lcdc.v_front_porch = 13;
    pinfo.lcdc.v_pulse_width = 2;
    pinfo.lcdc.border_clr = 0;  
    pinfo.lcdc.underflow_clr = 0xff;    
    pinfo.lcdc.hsync_skew = 0;

    pinfo.bl_max = 30;
    pinfo.bl_min = 1;
    pinfo.fb_num = 2;
    pinfo.clk_rate = 496000000; 

    pinfo.mipi.mode = DSI_VIDEO_MODE;

    pinfo.mipi.pulse_mode_hsa_he = FALSE;
    pinfo.mipi.hfp_power_stop = FALSE;
    pinfo.mipi.hbp_power_stop = FALSE;
    pinfo.mipi.hsa_power_stop = FALSE;
    pinfo.mipi.eof_bllp_power_stop = TRUE;
    pinfo.mipi.bllp_power_stop = TRUE;

    pinfo.mipi.traffic_mode = DSI_NON_BURST_SYNCH_EVENT;
    pinfo.mipi.dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
    pinfo.mipi.vc = 0;

    pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB;

    pinfo.mipi.data_lane0 = TRUE;
    pinfo.mipi.data_lane1 = TRUE;
    pinfo.mipi.data_lane2 = TRUE;
    pinfo.mipi.data_lane3 = TRUE;
    pinfo.mipi.t_clk_post = 0x19;
    pinfo.mipi.t_clk_pre = 0x30;
    pinfo.mipi.stream = 0; 

	pinfo.mipi.esc_byte_ratio = 2;

    pinfo.mipi.mdp_trigger = DSI_CMD_TRIGGER_SW;
    pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;

    pinfo.mipi.frame_rate = 60;
    pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db;
    pinfo.mipi.tx_eot_append = TRUE;

    
    pinfo.lcd.refx100 = pinfo.mipi.frame_rate * 100;
    pinfo.lcd.v_back_porch = pinfo.lcdc.v_back_porch;
    pinfo.lcd.v_front_porch = pinfo.lcdc.v_front_porch;
    pinfo.lcd.v_pulse_width = pinfo.lcdc.v_pulse_width;

    ret = mipi_smd_oled_hd_device_register(&pinfo, MIPI_DSI_PRIM,
                        MIPI_DSI_PANEL_720P_PT);
    if (ret)
        printk(KERN_ERR "%s: failed to register device!\n", __func__);

    return ret;
}

module_init(mipi_smd_video_hd_pt_init);

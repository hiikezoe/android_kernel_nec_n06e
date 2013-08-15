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

#include "msm_sensor.h"
#include "msm.h"
#include "msm_ispif.h"
#include <linux/mfd/pm8xxx/pm8921.h>

#define SENSOR_NAME "iu091"
#define PLATFORM_DRIVER_NAME "msm_camera_iu091"
#define iu091_obj iu091_##obj



#define IU091_LOG_ERR_ON  1   
#define IU091_LOG_DBG_ON  0   

#if IU091_LOG_ERR_ON
#define IU091_LOG_ERR(fmt, args...) printk(KERN_ERR "[iu091]%s(%d) " fmt, __func__, __LINE__, ##args)
#else
#define IU091_LOG_ERR(fmt, args...) do{}while(0)
#endif

#if IU091_LOG_DBG_ON
#define IU091_LOG_DBG(fmt, args...) printk(KERN_INFO "[iu091]%s(%d) " fmt, __func__, __LINE__, ##args)
#else
#define IU091_LOG_DBG(fmt, args...) do{}while(0)
#endif

#define IU091_LOG_INF(fmt, args...) printk(KERN_INFO "[iu091]%s(%d) " fmt, __func__, __LINE__, ##args)



#define IU091_WAIT_PWON_VREG_L12      1       
#define IU091_WAIT_PWON_V_EN2         1       
#define IU091_WAIT_PWON_V_EN1         1       
#define IU091_WAIT_PWON_VREG_L11      1       
#define IU091_WAIT_PWON_RST_N         1       
#define IU091_WAIT_PWON_MCLK         10       


#define IU091_WAIT_PWOFF_I2C_STOP    50       
#define IU091_WAIT_PWOFF_MCLK         1       
#define IU091_WAIT_PWOFF_RST_N        1       
#define IU091_WAIT_PWOFF_VREG_L11     5       
#define IU091_WAIT_PWOFF_V_EN1       10       
#define IU091_WAIT_PWOFF_V_EN2        5       
#define IU091_WAIT_PWOFF_VREG_L12     5       


#define IU091_GPIO_CAM_MCLK0         5        


#define PM8921_GPIO_BASE        NR_GPIO_IRQS
#define PM8921_GPIO_PM_TO_SYS(pm_gpio)  (pm_gpio - 1 + PM8921_GPIO_BASE)

#define IU091_PMGPIO_CAM_V_EN1  PM8921_GPIO_PM_TO_SYS(13)   
#define IU091_PMGPIO_CAM_V_EN2  PM8921_GPIO_PM_TO_SYS(26)   
#define IU091_PMGPIO_CAM_RST_N  PM8921_GPIO_PM_TO_SYS(28)   


#define CAM_VANA_MINUV                    2600000
#define CAM_VANA_MAXUV                    3300000
#define CAM_VANA_LOAD_UA                  150000

#define CAM_VDIG_MINUV                    1100000
#define CAM_VDIG_MAXUV                    1500000
#define CAM_VDIG_LOAD_UA                  150000

static struct regulator *cam_vana = NULL;
static struct regulator *cam_vdig = NULL;


static boolean camera_iu091_pm_vdig = false;
static boolean camera_iu091_pmgpio_26 = false;
static boolean camera_iu091_pmgpio_13 = false;
static boolean camera_iu091_pm_vana = false;
static boolean camera_iu091_pmgpio_28 = false;
static boolean camera_iu091_mclk = false;
static boolean camera_iu091_i2c = false;


struct pm_gpio iu091_cam_v_en_on = {
    .direction      = PM_GPIO_DIR_OUT,
    .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
    .output_value   = 1,
    .pull           = PM_GPIO_PULL_NO,
    .vin_sel        = PM_GPIO_VIN_S4,
    .out_strength   = PM_GPIO_STRENGTH_LOW,
    .function       = PM_GPIO_FUNC_NORMAL,
    .inv_int_pol    = 0,
    .disable_pin    = 0,
};

struct pm_gpio iu091_cam_v_en_off = {
    .direction      = PM_GPIO_DIR_OUT,
    .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
    .output_value   = 0,
    .pull           = PM_GPIO_PULL_NO,
    .vin_sel        = PM_GPIO_VIN_S4,
    .out_strength   = PM_GPIO_STRENGTH_LOW,
    .function       = PM_GPIO_FUNC_NORMAL,
    .inv_int_pol    = 0,
    .disable_pin    = 0,
};

static struct msm_cam_clk_info iu091_cam_clk_info[] = {
    {"cam_clk", MSM_SENSOR_MCLK_25HZ},
};

DEFINE_MUTEX(iu091_mut);
static struct msm_sensor_ctrl_t iu091_s_ctrl;

static struct msm_camera_i2c_reg_conf iu091_start_settings[] = {
	{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf iu091_stop_settings[] = {
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf iu091_groupon_settings[] = {
	{0x0104, 0x01},
};

static struct msm_camera_i2c_reg_conf iu091_groupoff_settings[] = {
	{0x0104, 0x00},
};

static struct msm_camera_i2c_reg_conf iu091_snap_settings[] = {

	
    {0x0305, 0x04},
    {0x0307, 0x57},
    {0x30A4, 0x02},
    {0x303C, 0x50},
	
    {0x0112, 0x0A},
    {0x0113, 0x0A},
    {0x0340, 0x0C},
    {0x0341, 0xA0},
    {0x0342, 0x12},
    {0x0343, 0x0C},
    {0x0344, 0x00},
    {0x0345, 0x08},
    {0x0346, 0x00},
    {0x0347, 0x30},
    {0x0348, 0x10},
    {0x0349, 0x77},
    {0x034A, 0x0C},
    {0x034B, 0x5F},
    {0x034C, 0x10},
    {0x034D, 0x70},
    {0x034E, 0x0C},
    {0x034F, 0x30},
    {0x0381, 0x01},
    {0x0383, 0x01},
    {0x0385, 0x01},
    {0x0387, 0x01},
    {0x3033, 0x00},
    {0x303D, 0x10},
    {0x303E, 0xD0},
    {0x3040, 0x08},
    {0x3041, 0x97},
    {0x3048, 0x00},
    {0x304C, 0x7F},
    {0x304D, 0x04},
    {0x3064, 0x12},
    {0x309B, 0x20},
    {0x309E, 0x00},
    {0x30A0, 0x14},
    {0x30B2, 0x00},
    {0x30D5, 0x00},
    {0x30D6, 0x85},
    {0x30D7, 0x2A},
    {0x30D8, 0x64},
    {0x30D9, 0x89},
    {0x30DA, 0x00},
    {0x30DB, 0x00},
    {0x30DC, 0x00},
    {0x30DD, 0x00},
    {0x30DE, 0x00},
    {0x3102, 0x10},
    {0x3103, 0x44},
    {0x3104, 0x40},
    {0x3105, 0x00},
    {0x3106, 0x0D},
    {0x3107, 0x01},
    {0x310A, 0x0A},
    {0x315C, 0x99},
    {0x315D, 0x98},
    {0x316E, 0x9A},
    {0x316F, 0x99},
    {0x3301, 0x03},
    {0x3304, 0x05},
    {0x3305, 0x04},
    {0x3306, 0x12},
    {0x3307, 0x03},
    {0x3308, 0x0D},
    {0x3309, 0x05},
    {0x330A, 0x09},
    {0x330B, 0x04},
    {0x330C, 0x08},
    {0x330D, 0x05},
    {0x330E, 0x03},
    {0x3318, 0x64},
    {0x3322, 0x02},
    {0x3342, 0x0F},
    {0x3348, 0xE0},
};

static struct msm_camera_i2c_reg_conf iu091_prev_settings[] = {

	
    {0x0305, 0x04},
    {0x0307, 0x57},
    {0x30A4, 0x02},
    {0x303C, 0x50},
	
    {0x0112, 0x0A},
    {0x0113, 0x0A},
    {0x0340, 0x06},
    {0x0341, 0x50},
    {0x0342, 0x09},
    {0x0343, 0x06},
    {0x0344, 0x00},
    {0x0345, 0x08},
    {0x0346, 0x00},
    {0x0347, 0x30},
    {0x0348, 0x10},
    {0x0349, 0x77},
    {0x034A, 0x0C},
    {0x034B, 0x5F},
    {0x034C, 0x08},
    {0x034D, 0x38},
    {0x034E, 0x06},
    {0x034F, 0x18},
    {0x0381, 0x01},
    {0x0383, 0x01},
    {0x0385, 0x01},
    {0x0387, 0x03},
    {0x3033, 0x84},
    {0x303D, 0x10},
    {0x303E, 0x41},
    {0x3040, 0x08},
    {0x3041, 0x97},
    {0x3048, 0x01},
    {0x304C, 0x3F},
    {0x304D, 0x02},
    {0x3064, 0x10},
    {0x309B, 0x48},
    {0x309E, 0x04},
    {0x30A0, 0x14},
    {0x30B2, 0x00},
    {0x30D5, 0x04},
    {0x30D6, 0x85},
    {0x30D7, 0x2A},
    {0x30D8, 0x64},
    {0x30D9, 0x89},
    {0x30DA, 0x00},
    {0x30DB, 0x00},
    {0x30DC, 0x00},
    {0x30DD, 0x00},
    {0x30DE, 0x00},
    {0x3102, 0x09},
    {0x3103, 0x23},
    {0x3104, 0x24},
    {0x3105, 0x00},
    {0x3106, 0x8B},
    {0x3107, 0x00},
    {0x310A, 0x0A},
    {0x315C, 0x4A},
    {0x315D, 0x49},
    {0x316E, 0x4B},
    {0x316F, 0x4A},
    {0x3301, 0x03},
    {0x3304, 0x05},
    {0x3305, 0x04},
    {0x3306, 0x12},
    {0x3307, 0x03},
    {0x3308, 0x0D},
    {0x3309, 0x05},
    {0x330A, 0x09},
    {0x330B, 0x04},
    {0x330C, 0x08},
    {0x330D, 0x05},
    {0x330E, 0x03},
    {0x3318, 0x69},
    {0x3322, 0x02},
    {0x3342, 0x0F},
    {0x3348, 0xF1},
};

static struct msm_camera_i2c_reg_conf iu091_fhd_video_30fps_settings[] = {

	
    {0x0305, 0x04},
    {0x0307, 0x57},
    {0x30A4, 0x02},
    {0x303C, 0x50},
	
    {0x0112, 0x0A},
    {0x0113, 0x0A},
    {0x0340, 0x06},
    {0x0341, 0x50},
    {0x0342, 0x09},
    {0x0343, 0x06},
    {0x0344, 0x00},
    {0x0345, 0x08},
    {0x0346, 0x01},
    {0x0347, 0xA8},
    {0x0348, 0x10},
    {0x0349, 0x77},
    {0x034A, 0x0A},
    {0x034B, 0xE7},
    {0x034C, 0x08},
    {0x034D, 0x38},
    {0x034E, 0x04},
    {0x034F, 0xA0},
    {0x0381, 0x01},
    {0x0383, 0x01},
    {0x0385, 0x01},
    {0x0387, 0x03},
    {0x3033, 0x84},
    {0x303D, 0x10},
    {0x303E, 0x41},
    {0x3040, 0x08},
    {0x3041, 0x97},
    {0x3048, 0x01},
    {0x304C, 0x3F},
    {0x304D, 0x02},
    {0x3064, 0x10},
    {0x309B, 0x48},
    {0x309E, 0x04},
    {0x30A0, 0x14},
    {0x30B2, 0x00},
    {0x30D5, 0x04},
    {0x30D6, 0x85},
    {0x30D7, 0x2A},
    {0x30D8, 0x64},
    {0x30D9, 0x89},
    {0x30DA, 0x00},
    {0x30DB, 0x00},
    {0x30DC, 0x00},
    {0x30DD, 0x00},
    {0x30DE, 0x00},
    {0x3102, 0x09},
    {0x3103, 0x23},
    {0x3104, 0x24},
    {0x3105, 0x00},
    {0x3106, 0x8B},
    {0x3107, 0x00},
    {0x310A, 0x0A},
    {0x315C, 0x4A},
    {0x315D, 0x49},
    {0x316E, 0x4B},
    {0x316F, 0x4A},
    {0x3301, 0x03},
    {0x3304, 0x05},
    {0x3305, 0x04},
    {0x3306, 0x12},
    {0x3307, 0x03},
    {0x3308, 0x0D},
    {0x3309, 0x05},
    {0x330A, 0x09},
    {0x330B, 0x04},
    {0x330C, 0x08},
    {0x330D, 0x05},
    {0x330E, 0x03},
    {0x3318, 0x69},
    {0x3322, 0x02},
    {0x3342, 0x0F},
    {0x3348, 0xF1},
};





















































































static struct msm_camera_i2c_reg_conf iu091_hs_prev_snap_settings[] = {

	
    {0x0305, 0x04},
    {0x0307, 0x57},
    {0x30A4, 0x02},
    {0x303C, 0x50},
	
    {0x0112, 0x0A},
    {0x0113, 0x0A},
    {0x0340, 0x06},
    {0x0341, 0x50},
    {0x0342, 0x09},
    {0x0343, 0x06},
    {0x0344, 0x00},
    {0x0345, 0x08},
    {0x0346, 0x00},
    {0x0347, 0x30},
    {0x0348, 0x10},
    {0x0349, 0x77},
    {0x034A, 0x0C},
    {0x034B, 0x5F},
    {0x034C, 0x08},
    {0x034D, 0x38},
    {0x034E, 0x06},
    {0x034F, 0x18},
    {0x0381, 0x01},
    {0x0383, 0x01},
    {0x0385, 0x01},
    {0x0387, 0x03},
    {0x3033, 0x84},
    {0x303D, 0x10},
    {0x303E, 0x41},
    {0x3040, 0x08},
    {0x3041, 0x97},
    {0x3048, 0x01},
    {0x304C, 0x3F},
    {0x304D, 0x02},
    {0x3064, 0x10},
    {0x309B, 0x28},
    {0x309E, 0x00},
    {0x30A0, 0x14},
    {0x30B2, 0x00},
    {0x30D5, 0x04},
    {0x30D6, 0x85},
    {0x30D7, 0x2A},
    {0x30D8, 0x64},
    {0x30D9, 0x89},
    {0x30DA, 0x00},
    {0x30DB, 0x00},
    {0x30DC, 0x00},
    {0x30DD, 0x00},
    {0x30DE, 0x00},
    {0x3102, 0x09},
    {0x3103, 0x23},
    {0x3104, 0x24},
    {0x3105, 0x00},
    {0x3106, 0x8B},
    {0x3107, 0x00},
    {0x310A, 0x0A},
    {0x315C, 0x4A},
    {0x315D, 0x49},
    {0x316E, 0x4B},
    {0x316F, 0x4A},
    {0x3301, 0x03},
    {0x3304, 0x05},
    {0x3305, 0x04},
    {0x3306, 0x12},
    {0x3307, 0x03},
    {0x3308, 0x0D},
    {0x3309, 0x05},
    {0x330A, 0x09},
    {0x330B, 0x04},
    {0x330C, 0x08},
    {0x330D, 0x05},
    {0x330E, 0x03},
    {0x3318, 0x69},
    {0x3322, 0x02},
    {0x3342, 0x0F},
    {0x3348, 0xF1},
};

static struct msm_camera_i2c_reg_conf iu091_recommend_settings[] = {
    
	{0x0101, 0x00},
	
	{0x3087, 0x53},
	{0x309D, 0x94},
	{0x30A1, 0x08},
	{0x30AA, 0x04},
	{0x30B1, 0x03},
	{0x30C7, 0x00},
	{0x3115, 0x0E},
	{0x3118, 0x42},
	{0x311D, 0x34},
	{0x3121, 0x0D},
	{0x3212, 0xF2},
	{0x3213, 0x0F},
	{0x3215, 0x0F},
	{0x3217, 0x0B},
	{0x3219, 0x0B},
	{0x321B, 0x0D},
	{0x321D, 0x0D},
	
	{0x3032, 0x40},
};


static struct msm_camera_i2c_reg_conf iu091_shutter_lag_short_1_settings[] = {
    {0x3035, 0x10},
    {0x303B, 0x14},
    {0x3312, 0x45},
    {0x3313, 0xC0},
    {0x3310, 0x20},
    {0x3310, 0x00},
    {0x303B, 0x04},
    {0x303D, 0x00},
    {0x0100, 0x10},
    {0x3035, 0x00},
};

static struct msm_camera_i2c_reg_conf iu091_ae_for_capture_settings[] = {
    {0x0340, 0x0C},
    {0x0341, 0x8C},
};

static struct msm_camera_i2c_reg_conf iu091_shutter_lag_short_2_settings[] = {
    
    {0x0342, 0x12},
    {0x0343, 0x0C},
    {0x034C, 0x10},
    {0x034D, 0x70},
    {0x034E, 0x0C},
    {0x034F, 0x30},
    {0x0387, 0x01},
    {0x3033, 0x00},
    {0x303E, 0xD0},
    {0x3048, 0x00},
    {0x304C, 0x7F},
    {0x304D, 0x04},
    {0x3064, 0x12},
    {0x309B, 0x20},
    {0x309E, 0x00},
    {0x30D5, 0x00},
    {0x3102, 0x10},
    {0x3103, 0x44},
    {0x3104, 0x40},
    {0x3106, 0x0D},
    {0x3107, 0x01},
    {0x315C, 0x99},
    {0x315D, 0x98},
    {0x316E, 0x9A},
    {0x316F, 0x99},
    {0x3318, 0x64},
    {0x3348, 0xE0},
    
    {0x30B1, 0x43},
    
    {0x3311, 0x80},
    {0x3311, 0x00},
};

static struct msm_camera_i2c_reg_conf iu091_shutter_lag_short_3_settings[] = {
    
    {0x30B1, 0x03},
};

static struct msm_camera_i2c_conf_array iu091_comm_confs[] = {
	{&iu091_shutter_lag_short_1_settings[0],
	ARRAY_SIZE(iu091_shutter_lag_short_1_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&iu091_ae_for_capture_settings[0],
	ARRAY_SIZE(iu091_ae_for_capture_settings),      0, MSM_CAMERA_I2C_BYTE_DATA},
	{&iu091_shutter_lag_short_2_settings[0],
	ARRAY_SIZE(iu091_shutter_lag_short_2_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&iu091_shutter_lag_short_3_settings[0],
	ARRAY_SIZE(iu091_shutter_lag_short_3_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct v4l2_subdev_info iu091_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	
};

static struct msm_camera_i2c_conf_array iu091_init_conf[] = {
	{&iu091_recommend_settings[0],
	ARRAY_SIZE(iu091_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array iu091_confs[] = {
	{&iu091_snap_settings[0],
	ARRAY_SIZE(iu091_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&iu091_prev_settings[0],
	ARRAY_SIZE(iu091_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&iu091_prev_settings[0],
	ARRAY_SIZE(iu091_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&iu091_hs_prev_snap_settings[0],
	ARRAY_SIZE(iu091_hs_prev_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&iu091_fhd_video_30fps_settings[0],
	ARRAY_SIZE(iu091_fhd_video_30fps_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&iu091_snap_settings[0],
	ARRAY_SIZE(iu091_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&iu091_hs_prev_snap_settings[0],
	ARRAY_SIZE(iu091_hs_prev_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t iu091_dimensions[] = {
	{
	 
		.x_output = 0x1070, 
		.y_output = 0x0C30, 
		.line_length_pclk = 0x120C, 
		.frame_length_lines = 0x0CA0, 
		.vt_pixel_clk = 222720000,
		.op_pixel_clk = 222720000,
		.binning_factor = 1,
	},
	{
	 
		.x_output = 0x0838, 
		.y_output = 0x0618, 
		.line_length_pclk = 0x0906, 
		.frame_length_lines = 0x0650, 
		.vt_pixel_clk = 111360000,
		.op_pixel_clk = 111360000,
		.binning_factor = 1,
	},
	{
	 
		.x_output = 0x0838, 
		.y_output = 0x0618, 
		.line_length_pclk = 0x0906, 
		.frame_length_lines = 0x0650, 
		.vt_pixel_clk = 111360000,
		.op_pixel_clk = 111360000,
		.binning_factor = 1,
	},
	{
	 
		.x_output = 0x0838, 
		.y_output = 0x0618, 
		.line_length_pclk = 0x0906, 
		.frame_length_lines = 0x0650, 
		.vt_pixel_clk = 111360000,
		.op_pixel_clk = 111360000,
		.binning_factor = 2,
	},
	{
	 
		.x_output = 0x0838, 
		.y_output = 0x04A0, 
		.line_length_pclk = 0x0906, 
		.frame_length_lines = 0x0650, 
		.vt_pixel_clk = 111360000,
		.op_pixel_clk = 111360000,
		.binning_factor = 1,
	},
	{
	 
		.x_output = 0x1070, 
		.y_output = 0x0C30, 
		.line_length_pclk = 0x120C, 
		.frame_length_lines = 0x0CA0, 
		.vt_pixel_clk = 222720000,
		.op_pixel_clk = 222720000,
		.binning_factor = 1,
	},
	{
	 
		.x_output = 0x0838, 
		.y_output = 0x0618, 
		.line_length_pclk = 0x0906, 
		.frame_length_lines = 0x0650, 
		.vt_pixel_clk = 111360000,
		.op_pixel_clk = 111360000,
		.binning_factor = 2,
	},
};































static struct msm_sensor_output_reg_addr_t iu091_reg_addr = {
	.x_output = 0x034C,
	.y_output = 0x034E,
	.line_length_pclk = 0x0342,
	.frame_length_lines = 0x0340,
};

static struct msm_sensor_id_info_t iu091_id_info = {
	.sensor_id_reg_addr = 0x0000,
	.sensor_id = 0x0091,
};

static struct msm_sensor_exp_gain_info_t iu091_exp_gain_info = {
	.coarse_int_time_addr = 0x0202,
	.global_gain_addr = 0x0204,
	.vert_offset = 5,
};

static enum msm_camera_vreg_name_t iu091_veg_seq[] = {




};

static const struct i2c_device_id iu091_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&iu091_s_ctrl},
	{ }
};

static struct i2c_driver iu091_i2c_driver = {
	.id_table = iu091_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client iu091_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};


#define OTP_READ_BANK_ADDR               0x34C9
#define OTP_READ_BANK_MAX                0x0F
#define OTP_READ_BANK_DATA_SIZE          0x08
#define OTP_READ_OUT_REGISTER_START_ADDR 0x3500
#define OTP_READ_OUT_REGISTER_DATA_SIZE  0x80

static char otp_data[OTP_READ_OUT_REGISTER_DATA_SIZE];





static int32_t iu091_sensor_otp_read(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc = 0;

    char bank = 0x00;
    int  read_offset;
    uint16_t* opt_data_uint16;
    uint16_t read_addr = OTP_READ_OUT_REGISTER_START_ADDR;

    IU091_LOG_DBG("START\n");

    for( bank = 0; bank <= OTP_READ_BANK_MAX; bank++)
    {
        rc = msm_camera_i2c_write(
                s_ctrl->sensor_i2c_client,
                OTP_READ_BANK_ADDR,
                bank,
                MSM_CAMERA_I2C_BYTE_DATA);

        if(rc < 0){
            IU091_LOG_ERR("%s: i2c_write failed\n", __func__);
            return rc;
        }

        for( read_offset = 0; read_offset < OTP_READ_BANK_DATA_SIZE ; read_offset+=sizeof(uint16_t) )
        {
            rc = msm_camera_i2c_read(
                s_ctrl->sensor_i2c_client,
                read_addr, 
                (uint16_t*)&otp_data[bank*OTP_READ_BANK_DATA_SIZE + read_offset],
                sizeof(uint16_t));

            if(rc < 0){
                IU091_LOG_ERR("%s: i2c_read failed\n", __func__);
                return rc;
            }

            IU091_LOG_DBG("bank=0x%X,read_addr=0x%X,read_offset=0x%X\n",bank,read_addr,read_offset);
            read_addr = read_addr + sizeof(uint16_t);
        }
    }
    
    
    read_addr = 0;
    opt_data_uint16 = (uint16_t*)&otp_data[0];

#if IU091_LOG_DBG_ON
    IU091_LOG_DBG("----- otp_read -----\n");
    for( bank = 0; bank <= OTP_READ_BANK_MAX; bank++)
    {
        IU091_LOG_DBG("bank 0x%02X:",bank);
        for ( read_offset = 0; read_offset < 4; read_offset++)
            IU091_LOG_DBG(" 0x%04X",opt_data_uint16[bank*4 + read_offset]);
        IU091_LOG_DBG("\n");
    }
#endif

    IU091_LOG_DBG("END\n");
    return rc;
}


int32_t iu091_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
    struct msm_camera_sensor_info *data = s_ctrl->sensordata;
    IU091_LOG_INF("START\n");

    
    if(camera_iu091_i2c)
    {
	    if (data->sensor_platform_info->i2c_conf &&
	        data->sensor_platform_info->i2c_conf->use_i2c_mux)
	    {
	        msm_sensor_disable_i2c_mux(
	            data->sensor_platform_info->i2c_conf);
	        IU091_LOG_DBG("msm_sensor_disable_i2c_mux()\n");
	    }

	    
	    IU091_LOG_DBG("wait %d[ms]\n",IU091_WAIT_PWOFF_I2C_STOP);
	    mdelay(IU091_WAIT_PWOFF_I2C_STOP);
	    camera_iu091_i2c = false;
    }


    
    if(camera_iu091_mclk)
    {




	    msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
	        iu091_cam_clk_info, s_ctrl->cam_clk, ARRAY_SIZE(iu091_cam_clk_info), 0);

    	    msm_camera_request_gpio_table(data, 0);

	    
	    IU091_LOG_DBG("wait %d[ms]\n",IU091_WAIT_PWOFF_MCLK);
	    mdelay(IU091_WAIT_PWOFF_MCLK);
	    camera_iu091_mclk = false;
    }


    
    if(camera_iu091_pmgpio_28)
    {
	    pm8xxx_gpio_config(IU091_PMGPIO_CAM_RST_N, &iu091_cam_v_en_off);

	    IU091_LOG_DBG("PMGPIO[28]:OFF\n");

	    
	    IU091_LOG_DBG("wait %d[ms]\n",IU091_WAIT_PWOFF_RST_N);
	    mdelay(IU091_WAIT_PWOFF_RST_N);
	    camera_iu091_pmgpio_28 = false;
    }


    
    if(camera_iu091_pm_vana)
    {
	    if (cam_vana){
	        regulator_set_optimum_mode(cam_vana, 0);
	        regulator_set_voltage(cam_vana, 0, CAM_VANA_MAXUV);
	        regulator_disable(cam_vana);
	        regulator_put(cam_vana);

	        IU091_LOG_DBG("cam_vana:%d [ON/OFF]",regulator_is_enabled(cam_vana));
	        IU091_LOG_DBG("cam_vana:%d [V]",regulator_get_voltage(cam_vana));

	        cam_vana = NULL;
	    }

	    
	    IU091_LOG_DBG("wait %d[ms]\n",IU091_WAIT_PWOFF_VREG_L11);
	    mdelay(IU091_WAIT_PWOFF_VREG_L11);
	    camera_iu091_pm_vana = false;
    }


    
    if(camera_iu091_pmgpio_13)
    {
	    pm8xxx_gpio_config(IU091_PMGPIO_CAM_V_EN1, &iu091_cam_v_en_off);

	    IU091_LOG_DBG("PMGPIO[13]:OFF\n");

	    
	    IU091_LOG_DBG("wait %d[ms]\n",IU091_WAIT_PWOFF_V_EN1);
	    mdelay(IU091_WAIT_PWOFF_V_EN1);
	    camera_iu091_pmgpio_13 = false;
    }


    
    if(camera_iu091_pmgpio_26)
    {
	    pm8xxx_gpio_config(IU091_PMGPIO_CAM_V_EN2, &iu091_cam_v_en_off);

	    IU091_LOG_DBG("PMGPIO[26]:OFF\n");

	    
	    IU091_LOG_DBG("wait %d[ms]\n",IU091_WAIT_PWOFF_V_EN2);
	    mdelay(IU091_WAIT_PWOFF_V_EN2);
	    camera_iu091_pmgpio_26 = false;
    }

    
    if(camera_iu091_pm_vdig)
    {
	    if (cam_vdig) {
	        regulator_set_voltage(cam_vdig, 0, CAM_VDIG_MAXUV);
	        regulator_set_optimum_mode(cam_vdig, 0);
	        regulator_disable(cam_vdig);
	        regulator_put(cam_vdig);

	        IU091_LOG_DBG("cam_vdig:%d [ON/OFF]",regulator_is_enabled(cam_vdig));
	        IU091_LOG_DBG("cam_vdig:%d [V]",regulator_get_voltage(cam_vdig));

	        cam_vdig = NULL;
	    }

	    
	    IU091_LOG_DBG("wait %d[ms]\n",IU091_WAIT_PWOFF_VREG_L12);
	    mdelay(IU091_WAIT_PWOFF_VREG_L12);
	    camera_iu091_pm_vdig = false;
    }

    IU091_LOG_INF("END\n");
    return 0;
}


int32_t iu091_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc = 0;
    struct msm_camera_sensor_info *data = s_ctrl->sensordata;

    IU091_LOG_INF("START\n");

    
    if (cam_vdig == NULL) {
        cam_vdig = regulator_get(&s_ctrl->sensor_i2c_client->client->dev, "cam_vdig");
        if (IS_ERR(cam_vdig)) {
            IU091_LOG_ERR("VREG CAM VDIG get failed\n");
            cam_vdig = NULL;
            iu091_sensor_power_down(s_ctrl);
            return -EFAULT;
        }
        if (regulator_set_voltage(cam_vdig, CAM_VDIG_MINUV,
            CAM_VDIG_MAXUV)) {
            IU091_LOG_ERR("VREG CAM VDIG set voltage failed\n");
            iu091_sensor_power_down(s_ctrl);
            return -EFAULT;
        }
        if (regulator_set_optimum_mode(cam_vdig,
            CAM_VDIG_LOAD_UA) < 0) {
            IU091_LOG_ERR("VREG CAM VDIG set optimum mode failed\n");
            iu091_sensor_power_down(s_ctrl);
            return -EFAULT;
        }
        if (regulator_enable(cam_vdig)) {
            IU091_LOG_ERR("VREG CAM VDIG enable failed\n");
            iu091_sensor_power_down(s_ctrl);
            return -EFAULT;
        }
    }

    IU091_LOG_DBG("cam_vdig:%d [ON/OFF]",regulator_is_enabled(cam_vdig));
    IU091_LOG_DBG("cam_vdig:%d [V]",regulator_get_voltage(cam_vdig));

    
    IU091_LOG_DBG("wait %d[ms]\n",IU091_WAIT_PWON_VREG_L12);
    mdelay(IU091_WAIT_PWON_VREG_L12);
    camera_iu091_pm_vdig = true;


    
    rc = pm8xxx_gpio_config(IU091_PMGPIO_CAM_V_EN2, &iu091_cam_v_en_on);
    if (rc) {
        
        IU091_LOG_ERR("IU091_PMGPIO_CAM_V_EN2(%d) Error, rc = %d\n", IU091_PMGPIO_CAM_V_EN2, rc);
        iu091_sensor_power_down(s_ctrl);
        return -EFAULT;
    }
    IU091_LOG_DBG("PMGPIO[26]:ON\n");

    
    IU091_LOG_DBG("wait %d[ms]\n",IU091_WAIT_PWON_V_EN2);
    mdelay(IU091_WAIT_PWON_V_EN2);
    camera_iu091_pmgpio_26 = true;


    
    rc = pm8xxx_gpio_config(IU091_PMGPIO_CAM_V_EN1, &iu091_cam_v_en_on);
    if (rc) {
        
        IU091_LOG_ERR("IU091_PMGPIO_CAM_V_EN1(%d) Error, rc = %d\n", IU091_PMGPIO_CAM_V_EN1, rc);
        iu091_sensor_power_down(s_ctrl);
        return -EFAULT;
    }
    IU091_LOG_DBG("PMGPIO[13]:ON\n");

    
    IU091_LOG_DBG("wait %d[ms]\n",IU091_WAIT_PWON_V_EN1);
    mdelay(IU091_WAIT_PWON_V_EN1);
    camera_iu091_pmgpio_13 = true;


    
    if (cam_vana == NULL) {
        cam_vana = regulator_get(&s_ctrl->sensor_i2c_client->client->dev, "cam_vana");
        if (IS_ERR(cam_vana)) {
            IU091_LOG_ERR("VREG CAM VANA get failed\n");
            iu091_sensor_power_down(s_ctrl);
            return -EFAULT;
        }
        if (regulator_set_voltage(cam_vana, CAM_VANA_MINUV,
            CAM_VANA_MAXUV)) {
            IU091_LOG_ERR("VREG CAM VANA set voltage failed\n");
            iu091_sensor_power_down(s_ctrl);
            return -EFAULT;
        }
        if (regulator_set_optimum_mode(cam_vana,
            CAM_VANA_LOAD_UA) < 0) {
            IU091_LOG_ERR("VREG CAM VANA set optimum mode failed\n");
            iu091_sensor_power_down(s_ctrl);
            return -EFAULT;
        }
        if (regulator_enable(cam_vana)) {
            IU091_LOG_ERR("VREG CAM VANA enable failed\n");
            iu091_sensor_power_down(s_ctrl);
            return -EFAULT;
        }
    }

    IU091_LOG_DBG("cam_vana:%d [ON/OFF]",regulator_is_enabled(cam_vana));
    IU091_LOG_DBG("cam_vana:%d [V]",regulator_get_voltage(cam_vana));

    
    IU091_LOG_DBG("wait %d[ms]\n",IU091_WAIT_PWON_VREG_L11);
    mdelay(IU091_WAIT_PWON_VREG_L11);
    camera_iu091_pm_vana = true;


    
    rc = pm8xxx_gpio_config(IU091_PMGPIO_CAM_RST_N, &iu091_cam_v_en_on);
    if (rc) {
        
        IU091_LOG_DBG("IU091_PMGPIO_CAM_RST_N(%d) Error, rc = %d\n", IU091_PMGPIO_CAM_RST_N, rc);
        iu091_sensor_power_down(s_ctrl);
        return -EFAULT;
    }
    IU091_LOG_DBG("PMGPIO[28]:ON\n");

    
    IU091_LOG_DBG("wait %d[ms]\n",IU091_WAIT_PWON_RST_N);
    mdelay(IU091_WAIT_PWON_RST_N);
    camera_iu091_pmgpio_28 = true;


    
    rc = msm_camera_request_gpio_table(data, 1);
    if (rc < 0) {
        IU091_LOG_ERR("request gpio failed\n");
        iu091_sensor_power_down(s_ctrl);
        return -EFAULT;
    }

    if (s_ctrl->clk_rate != 0)
        iu091_cam_clk_info->clk_rate = s_ctrl->clk_rate;




    rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
    iu091_cam_clk_info, s_ctrl->cam_clk, ARRAY_SIZE(iu091_cam_clk_info), 1);

    if (rc < 0) {
        IU091_LOG_ERR("clk enable failed\n");
        iu091_sensor_power_down(s_ctrl);
        return -EFAULT;
    }
    IU091_LOG_DBG("MCLK ENABLE\n");

    
    IU091_LOG_DBG("wait %d[ms]\n",IU091_WAIT_PWON_MCLK);
    mdelay(IU091_WAIT_PWON_MCLK);
    camera_iu091_mclk = true;


    
    if (data->sensor_platform_info->i2c_conf &&
        data->sensor_platform_info->i2c_conf->use_i2c_mux){
        msm_sensor_enable_i2c_mux(data->sensor_platform_info->i2c_conf);
        IU091_LOG_DBG("msm_sensor_enable_i2c_mux()\n");
    }
    camera_iu091_i2c = true;

    IU091_LOG_INF("END\n");
    return rc;
}

static int curr_frame_length_lines = 0;

int32_t iu091_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
            int update_type, int res)
{
    int32_t rc = 0;

    int linecount;
    int frame_time,exposure_time; 
    struct msm_sensor_output_info_t curr_info,new_info;

    IU091_LOG_INF("START\n");

    curr_info = iu091_dimensions[s_ctrl->curr_res];
    new_info  = iu091_dimensions[res];
    if( res != 0)
    {
       s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
    }


    if (update_type == MSM_SENSOR_REG_INIT) {
        curr_frame_length_lines = 0;



        rc = msm_sensor_enable_debugfs(s_ctrl);
        if(rc < 0){
            
            IU091_LOG_DBG("msm_sensor_enable_debugfs (rc:%d)\n",rc);
        }

        rc = msm_sensor_write_init_settings(s_ctrl);
        if(rc < 0){
            IU091_LOG_ERR("%s: msm_sensor_write_init_settings (rc:%d)\n", __func__,rc);
            return rc;
        }


    } else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
        if( s_ctrl->curr_res != MSM_SENSOR_INVALID_RES)
        {
            IU091_LOG_DBG("setting change %d -> %d\n",s_ctrl->curr_res,res);
            if (curr_info.frame_length_lines > curr_frame_length_lines )
            {
                linecount = curr_info.frame_length_lines;
            }else{
                linecount = curr_frame_length_lines;
            }
            frame_time = 1000/(curr_info.vt_pixel_clk/(curr_info.line_length_pclk*linecount));

            if( res != 0 )
            {
                
                IU091_LOG_DBG("current frame_out_time = %d line_length_pclk =%d linecount = %d vt_pixel_clk = %d\n", frame_time,curr_info.line_length_pclk,linecount,curr_info.vt_pixel_clk);
                msleep(frame_time);
            }
        }

        
        curr_frame_length_lines = new_info.frame_length_lines;
        exposure_time = 1000/(new_info.vt_pixel_clk/(new_info.line_length_pclk*new_info.frame_length_lines));

        if( ((s_ctrl->curr_res == MSM_SENSOR_RES_QTR) && (res == MSM_SENSOR_RES_FULL)) ||
            ((s_ctrl->curr_res == MSM_SENSOR_RES_2  ) && (res == MSM_SENSOR_RES_FULL)) )
        {
            
            
            rc = msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client,
                (struct msm_camera_i2c_reg_conf *)
                iu091_comm_confs[0].conf,
                iu091_comm_confs[0].size,
                iu091_comm_confs[0].data_type);

            if(rc < 0){
                return rc;
            }




















            v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
                NOTIFY_PCLK_CHANGE, &s_ctrl->msm_sensor_reg->
                output_settings[res].op_pixel_clk);

            
            rc = msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client,
                (struct msm_camera_i2c_reg_conf *)
                iu091_comm_confs[1].conf,
                iu091_comm_confs[1].size,
                iu091_comm_confs[1].data_type);

            if(rc < 0){
                return rc;
            }

            
            rc = msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client,
                (struct msm_camera_i2c_reg_conf *)
                iu091_comm_confs[2].conf,
                iu091_comm_confs[2].size,
                iu091_comm_confs[2].data_type);

            if(rc < 0){
                return rc;
            }

            s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
            msleep(10);

            
            rc = msm_camera_i2c_write_tbl(s_ctrl->sensor_i2c_client,
                (struct msm_camera_i2c_reg_conf *)
                iu091_comm_confs[3].conf,
                iu091_comm_confs[3].size,
                iu091_comm_confs[3].data_type);

            if(rc < 0){
                return rc;
            }

        } else {
            rc = msm_sensor_write_res_settings(s_ctrl, res);

           if(rc < 0){
                return rc;
            }




















            v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
                NOTIFY_PCLK_CHANGE, &s_ctrl->msm_sensor_reg->
                output_settings[res].op_pixel_clk);
            s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
            msleep(exposure_time);
        }
    }
    IU091_LOG_INF("END\n");
    return rc;
}


int32_t iu091_sensor_write_exp_gain1(struct msm_sensor_ctrl_t *s_ctrl,


		uint16_t gain, uint32_t line, int32_t luma_avg, uint16_t fgain)

{
    int32_t rc = 0;
    uint32_t fl_lines;
    uint8_t offset;
    fl_lines = s_ctrl->curr_frame_length_lines;
    fl_lines = (fl_lines * s_ctrl->fps_divider) / Q10;
    offset = s_ctrl->sensor_exp_gain_info->vert_offset;
    if (line > (fl_lines - offset))
        fl_lines = line + offset;

    curr_frame_length_lines = fl_lines;
    IU091_LOG_DBG("Gain:%d, Linecount:%d\n", gain, line);

        s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
            s_ctrl->sensor_output_reg_addr->frame_length_lines,
            fl_lines, MSM_CAMERA_I2C_WORD_DATA);

        if(rc < 0){
            return rc;
        }

        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
            s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
            line, MSM_CAMERA_I2C_WORD_DATA);

        if(rc < 0){
            return rc;
        }

    if(rc >= 0){
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
            s_ctrl->sensor_exp_gain_info->global_gain_addr, gain,
            MSM_CAMERA_I2C_WORD_DATA);

        if(rc < 0){
            return rc;
        }
    }

    s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);

    return rc;
}




static int32_t iu091_get_maker_note(struct msm_sensor_ctrl_t *s_ctrl,
                                      struct get_exif_maker_note_cfg *get_exif_maker_note)
{


    get_exif_maker_note->fd_freq        = 0x0001;      

    get_exif_maker_note->device_id      = iu091_id_info.sensor_id;
    get_exif_maker_note->awb_temp       = 0x0002;
    get_exif_maker_note->awb_gain_r     = 0x0003;
    get_exif_maker_note->awb_gain_g     = 0x0004;
    get_exif_maker_note->awb_gain_b     = 0x0005;
    get_exif_maker_note->awb_saturation = 0x0006;

	get_exif_maker_note->calib_rg           = *(uint16_t*)&otp_data[0x08];  
	get_exif_maker_note->calib_bg           = *(uint16_t*)&otp_data[0x0A];  
	get_exif_maker_note->calib_grgb         = *(uint16_t*)&otp_data[0x0C];  
	get_exif_maker_note->af_inf_position    = *(uint16_t*)&otp_data[0x0E];  
	get_exif_maker_note->af_1m_position     = *(uint16_t*)&otp_data[0x10];  
	get_exif_maker_note->af_macro_position  = *(uint16_t*)&otp_data[0x12];  
	get_exif_maker_note->start_current      = *(uint16_t*)&otp_data[0x14];  
	get_exif_maker_note->op_sensitivity     = *(uint16_t*)&otp_data[0x16];  
	get_exif_maker_note->trial_ver          = *(uint16_t*)&otp_data[0x06];  
	get_exif_maker_note->lot_code_date      = *(uint16_t*)&otp_data[0x72];  
	get_exif_maker_note->lot_code_num       = *(uint32_t*)&otp_data[0x74];  
























    return 0;
}




static int32_t iu091_get_exif_param(struct msm_sensor_ctrl_t *s_ctrl,
                                      struct get_exif_param_inf *get_exif_param)
{
    int32_t rc = 0;
    
    uint16_t coarse_integration_time = 0x0000; 
    uint16_t line_length_DVE046         = 0x0000; 
    uint16_t fine_integration_time   = 0x0000; 
    uint16_t analog_gain_code_global = 0x0000; 
    uint16_t digital_gain_greenr     = 0x0000; 

    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                             0x0202, &coarse_integration_time, MSM_CAMERA_I2C_WORD_DATA);
    if(rc < 0){
        return rc;
    }

    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                             0x0342, &line_length_DVE046, MSM_CAMERA_I2C_WORD_DATA);
    if(rc < 0){
        return rc;
    }

    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                             0x0204, &analog_gain_code_global, MSM_CAMERA_I2C_WORD_DATA);
    if(rc < 0){
        return rc;
    }

    get_exif_param->coarse_integration_time = coarse_integration_time; 
    get_exif_param->line_length_DVE046         = line_length_DVE046;         
    get_exif_param->fine_integration_time   = fine_integration_time;   
    get_exif_param->analog_gain_code_global = analog_gain_code_global; 
    get_exif_param->digital_gain_greenr     = digital_gain_greenr;     











    return rc;
}




static int32_t iu091_get_eeprom_otp_info(struct msm_sensor_ctrl_t *s_ctrl,
                                      struct eeprom_otp_info_t *eeprom_otp_info)
{

    eeprom_otp_info->otp_bank00 = *(uint64_t*)&otp_data[0x00]; 
    eeprom_otp_info->otp_bank01 = *(uint64_t*)&otp_data[0x08]; 
    eeprom_otp_info->otp_bank02 = *(uint64_t*)&otp_data[0x10]; 
    eeprom_otp_info->otp_bank03 = *(uint64_t*)&otp_data[0x18]; 
    eeprom_otp_info->otp_bank04 = *(uint64_t*)&otp_data[0x20]; 
    eeprom_otp_info->otp_bank05 = *(uint64_t*)&otp_data[0x28]; 
    eeprom_otp_info->otp_bank06 = *(uint64_t*)&otp_data[0x30]; 
    eeprom_otp_info->otp_bank07 = *(uint64_t*)&otp_data[0x38]; 
    eeprom_otp_info->otp_bank08 = *(uint64_t*)&otp_data[0x40]; 
    eeprom_otp_info->otp_bank09 = *(uint64_t*)&otp_data[0x48]; 
    eeprom_otp_info->otp_bank10 = *(uint64_t*)&otp_data[0x50]; 
    eeprom_otp_info->otp_bank11 = *(uint64_t*)&otp_data[0x58]; 
    eeprom_otp_info->otp_bank12 = *(uint64_t*)&otp_data[0x60]; 
    eeprom_otp_info->otp_bank13 = *(uint64_t*)&otp_data[0x68]; 
    eeprom_otp_info->otp_bank14 = *(uint64_t*)&otp_data[0x70]; 
    eeprom_otp_info->otp_bank15 = *(uint64_t*)&otp_data[0x78]; 
    return 0;
}




static int32_t iu091_get_device_id(struct msm_sensor_ctrl_t *s_ctrl, uint16_t *device_id)
{
    int32_t rc = 0;
    uint16_t chipid = 0xFFFF;

    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                             s_ctrl->sensor_id_info->sensor_id_reg_addr, &chipid, 
                             MSM_CAMERA_I2C_WORD_DATA);
    if (rc < 0) {
        IU091_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
        return rc;
    }

    *device_id = chipid;

    return rc;
}


static int __init iu091_sensor_init_module(void)
{
    return i2c_add_driver(&iu091_i2c_driver);
}

static struct v4l2_subdev_core_ops iu091_subdev_core_ops = {
    .ioctl = msm_sensor_subdev_ioctl,
    .s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops iu091_subdev_video_ops = {
    .enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops iu091_subdev_ops = {
    .core = &iu091_subdev_core_ops,
    .video  = &iu091_subdev_video_ops,
};

static struct msm_sensor_fn_t iu091_func_tbl = {
    .sensor_start_stream = msm_sensor_start_stream,
    .sensor_stop_stream = msm_sensor_stop_stream,
    .sensor_group_hold_on = msm_sensor_group_hold_on,
    .sensor_group_hold_off = msm_sensor_group_hold_off,
    .sensor_set_fps = msm_sensor_set_fps,
    .sensor_write_exp_gain = iu091_sensor_write_exp_gain1,
    .sensor_write_snapshot_exp_gain = iu091_sensor_write_exp_gain1,
    .sensor_setting = iu091_sensor_setting,
    .sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
    .sensor_mode_init = msm_sensor_mode_init,
    .sensor_get_output_info = msm_sensor_get_output_info,
    .sensor_config = msm_sensor_config,
    .sensor_power_up = iu091_sensor_power_up,
    .sensor_power_down = iu091_sensor_power_down,
    .sensor_adjust_frame_lines = msm_sensor_adjust_frame_lines1,
    .sensor_get_csi_params = msm_sensor_get_csi_params,
    .sensor_set_parm_pm_obs = msm_sensor_set_parm_pm_obs,
    .sensor_get_maker_note = iu091_get_maker_note,
    .sensor_get_exif_param = iu091_get_exif_param,
    .sensor_get_eeprom_otp_info = iu091_get_eeprom_otp_info,
    .sensor_otp_read = iu091_sensor_otp_read,
    .sensor_get_device_id = iu091_get_device_id,
};

static struct msm_sensor_reg_t iu091_regs = {
    .default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .start_stream_conf = iu091_start_settings,
    .start_stream_conf_size = ARRAY_SIZE(iu091_start_settings),
    .stop_stream_conf = iu091_stop_settings,
    .stop_stream_conf_size = ARRAY_SIZE(iu091_stop_settings),
    .group_hold_on_conf = iu091_groupon_settings,
    .group_hold_on_conf_size = ARRAY_SIZE(iu091_groupon_settings),
    .group_hold_off_conf = iu091_groupoff_settings,
    .group_hold_off_conf_size = ARRAY_SIZE(iu091_groupoff_settings),
    .init_settings = &iu091_init_conf[0],
    .init_size = ARRAY_SIZE(iu091_init_conf),
    .mode_settings = &iu091_confs[0],
    .output_settings = &iu091_dimensions[0],
    .num_conf = ARRAY_SIZE(iu091_confs),
};

static struct msm_sensor_ctrl_t iu091_s_ctrl = {
    .msm_sensor_reg = &iu091_regs,
    .sensor_i2c_client = &iu091_sensor_i2c_client,
    .sensor_i2c_addr = 0x34,

    .vreg_seq = iu091_veg_seq,
    .num_vreg_seq = ARRAY_SIZE(iu091_veg_seq),

    .sensor_output_reg_addr = &iu091_reg_addr,
    .sensor_id_info = &iu091_id_info,
    .sensor_exp_gain_info = &iu091_exp_gain_info,
    .cam_mode = MSM_SENSOR_MODE_INVALID,



    .msm_sensor_mutex = &iu091_mut,
    .sensor_i2c_driver = &iu091_i2c_driver,
    .sensor_v4l2_subdev_info = iu091_subdev_info,
    .sensor_v4l2_subdev_info_size = ARRAY_SIZE(iu091_subdev_info),
    .sensor_v4l2_subdev_ops = &iu091_subdev_ops,
    .func_tbl = &iu091_func_tbl,
    .clk_rate = MSM_SENSOR_MCLK_25HZ,
};

module_init(iu091_sensor_init_module);
MODULE_DESCRIPTION("SONY 13MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");

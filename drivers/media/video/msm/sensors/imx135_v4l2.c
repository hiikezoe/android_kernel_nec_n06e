/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
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

#define SENSOR_NAME "imx135"
#define PLATFORM_DRIVER_NAME "msm_camera_imx135"
#define imx135_obj imx135_##obj


#define IMX135_LOG_ERR_ON  1   
#define IMX135_LOG_DBG_ON  0   

#if IMX135_LOG_ERR_ON
#define IMX135_LOG_ERR(fmt, args...) printk(KERN_ERR "imx135:%s(%d) " fmt, __func__, __LINE__, ##args)
#else
#define IMX135_LOG_ERR(fmt, args...) do{}while(0)
#endif

#if IMX135_LOG_DBG_ON
#define IMX135_LOG_DBG(fmt, args...) printk(KERN_INFO "imx135:%s(%d) " fmt, __func__, __LINE__, ##args)
#else
#define IMX135_LOG_DBG(fmt, args...) do{}while(0)
#endif

#define IMX135_LOG_INF(fmt, args...) printk(KERN_INFO "[imx135]%s(%d) " fmt, __func__, __LINE__, ##args)


#define IMX135_WAIT_PWON_VREG_L12      2       
#define IMX135_WAIT_PWON_V_EN2         1       
#define IMX135_WAIT_PWON_V_EN1         1       
#define IMX135_WAIT_PWON_VREG_L11      1       
#define IMX135_WAIT_PWON_RST_N         1       
#define IMX135_WAIT_PWON_MCLK         13      


#define IMX135_WAIT_PWOFF_I2C_STOP    50       
#define IMX135_WAIT_PWOFF_MCLK         1       
#define IMX135_WAIT_PWOFF_RST_N        1       
#define IMX135_WAIT_PWOFF_VREG_L11     5       
#define IMX135_WAIT_PWOFF_V_EN1       10       
#define IMX135_WAIT_PWOFF_V_EN2        5       
#define IMX135_WAIT_PWOFF_VREG_L12     5       


#define IMX135_GPIO_CAM_MCLK0         5        


#define PM8921_GPIO_BASE        NR_GPIO_IRQS
#define PM8921_GPIO_PM_TO_SYS(pm_gpio)  (pm_gpio - 1 + PM8921_GPIO_BASE)

#define IMX135_PMGPIO_CAM_V_EN1  PM8921_GPIO_PM_TO_SYS(13)   
#define IMX135_PMGPIO_CAM_V_EN2  PM8921_GPIO_PM_TO_SYS(26)   
#define IMX135_PMGPIO_CAM_RST_N  PM8921_GPIO_PM_TO_SYS(28)   


#define CAM_VANA_MINUV                    2600000
#define CAM_VANA_MAXUV                    3300000
#define CAM_VANA_LOAD_UA                  150000

#define CAM_VDIG_MINUV                    1050000
#define CAM_VDIG_MAXUV                    1500000
#define CAM_VDIG_LOAD_UA                  150000

 static struct regulator *cam_vana = NULL;
 static struct regulator *cam_vdig = NULL;


struct pm_gpio imx135_cam_v_en_on = {
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


struct pm_gpio imx135_cam_v_en_off = {
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

static struct msm_cam_clk_info imx135_cam_clk_info[] = {
    {"cam_clk", MSM_SENSOR_MCLK_25HZ},
};

DEFINE_MUTEX(imx135_mut);
static struct msm_sensor_ctrl_t imx135_s_ctrl;

static struct msm_camera_i2c_reg_conf imx135_start_settings[] = {
	{0x3A43, 0x01}, 
	{0x0100, 0x01}, 
};

static struct msm_camera_i2c_reg_conf imx135_stop_settings[] = {
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf imx135_groupon_settings[] = {
	{0x104, 0x01},
};

static struct msm_camera_i2c_reg_conf imx135_groupoff_settings[] = {
	{0x104, 0x00},
};

static struct msm_camera_i2c_reg_conf imx135_recommend_settings[] = {

	{0x0101, 0x03},
	{0x0105, 0x01},
	{0x0110, 0x00},
	{0x0220, 0x01},
	{0x3302, 0x11},
	{0x3833, 0x20},
	{0x3893, 0x00},
	{0x3906, 0x08},
	{0x3907, 0x01},
	{0x391B, 0x01},
	{0x3C09, 0x01},
	{0x600A, 0x00},
	{0x3008, 0xB0},
	{0x320A, 0x01},
	{0x320D, 0x10},
	{0x3216, 0x2E},
	{0x322C, 0x02},
	{0x3409, 0x0C},
	{0x340C, 0x2D},
	{0x3411, 0x39},
	{0x3414, 0x1E},
	{0x3427, 0x04},
	{0x3480, 0x1E},
	{0x3484, 0x1E},
	{0x3488, 0x1E},
	{0x348C, 0x1E},
	{0x3490, 0x1E},
	{0x3494, 0x1E},
	{0x3511, 0x8F},
	{0x364F, 0x2D},


	{0x380A, 0x00},
	{0x380B, 0x00},
	{0x4103, 0x00},

	{0x4243, 0x9A},
	{0x4330, 0x01},
	{0x4331, 0x90},
	{0x4332, 0x02},
	{0x4333, 0x58},
	{0x4334, 0x03},
	{0x4335, 0x20},
	{0x4336, 0x03},
	{0x4337, 0x84},
	{0x433C, 0x01},
	{0x4340, 0x02},
	{0x4341, 0x58},
	{0x4342, 0x03},
	{0x4343, 0x52},

	{0x4364, 0x0B},
	{0x4368, 0x00},
	{0x4369, 0x0F},
	{0x436A, 0x03},
	{0x436B, 0xA8},
	{0x436C, 0x00},
	{0x436D, 0x00},
	{0x436E, 0x00},
	{0x436F, 0x06},

	{0x4281, 0x21},
	{0x4282, 0x18},
	{0x4283, 0x04},
	{0x4284, 0x08},
	{0x4287, 0x7F},
	{0x4288, 0x08},
	{0x428B, 0x7F},
	{0x428C, 0x08},
	{0x428F, 0x7F},
	{0x4297, 0x00},
	{0x4298, 0x7E},
	{0x4299, 0x7E},
	{0x429A, 0x7E},
	{0x42A4, 0xFB},
	{0x42A5, 0x7E},
	{0x42A6, 0xDF},
	{0x42A7, 0xB7},
	{0x42AF, 0x03},

	{0x4207, 0x03},
	{0x4216, 0x08},
	{0x4217, 0x10},

	{0x4218, 0x00},
	{0x421B, 0x20},
	{0x421F, 0x04},
	{0x4222, 0x04},
	{0x4223, 0x44},
	{0x422E, 0x54},
	{0x422F, 0xFB},
	{0x4230, 0xFF},
	{0x4231, 0xFE},
	{0x4232, 0xFF},
	{0x4235, 0x58},
	{0x4236, 0xF7},
	{0x4237, 0xFD},
	{0x4239, 0x4E},
	{0x423A, 0xFC},
	{0x423B, 0xFD},

	{0x4300, 0x00},
	{0x4316, 0x12},
	{0x4317, 0x22},
	{0x4318, 0x00},
	{0x4319, 0x00},
	{0x431A, 0x00},
	{0x4324, 0x03},
	{0x4325, 0x20},
	{0x4326, 0x03},
	{0x4327, 0x84},
	{0x4328, 0x03},
	{0x4329, 0x20},
	{0x432A, 0x03},
	{0x432B, 0x20},
	{0x432C, 0x01},
	{0x432D, 0x01},
	{0x4338, 0x02},
	{0x4339, 0x00},
	{0x433A, 0x00},
	{0x433B, 0x02},
	{0x435A, 0x03},
	{0x435B, 0x84},
	{0x435E, 0x01},
	{0x435F, 0xFF},
	{0x4360, 0x01},
	{0x4361, 0xF4},
	{0x4362, 0x03},
	{0x4363, 0x84},
	{0x437B, 0x01},
	{0x4401, 0x3F},
	{0x4402, 0xFF},
	{0x4404, 0x13},
	{0x4405, 0x26},
	{0x4406, 0x07},
	{0x4408, 0x20},
	{0x4409, 0xE5},
	{0x440A, 0xFB},
	{0x440C, 0xF6},
	{0x440D, 0xEA},
	{0x440E, 0x20},
	{0x4410, 0x00},
	{0x4411, 0x00},
	{0x4412, 0x3F},
	{0x4413, 0xFF},
	{0x4414, 0x1F},
	{0x4415, 0xFF},
	{0x4416, 0x20},
	{0x4417, 0x00},
	{0x4418, 0x1F},
	{0x4419, 0xFF},
	{0x441A, 0x20},
	{0x441B, 0x00},
	{0x441D, 0x40},
	{0x441E, 0x1E},
	{0x441F, 0x38},
	{0x4420, 0x01},
	{0x4444, 0x00},
	{0x4445, 0x00},
	{0x4446, 0x1D},
	{0x4447, 0xF9},
	{0x4452, 0x00},
	{0x4453, 0xA0},
	{0x4454, 0x08},
	{0x4455, 0x00},
	{0x4456, 0x0F},
	{0x4457, 0xFF},
	{0x4458, 0x18},
	{0x4459, 0x18},
	{0x445A, 0x3F},
	{0x445B, 0x3A},
	{0x445C, 0x00},
	{0x445D, 0x28},
	{0x445E, 0x01},
	{0x445F, 0x90},
	{0x4460, 0x00},
	{0x4461, 0x60},
	{0x4462, 0x00},
	{0x4463, 0x00},
	{0x4464, 0x00},
	{0x4465, 0x00},
	{0x446C, 0x00},
	{0x446D, 0x00},
	{0x446E, 0x00},

	{0x452A, 0x02},

	{0x0712, 0x01},
	{0x0713, 0x00},
	{0x0714, 0x01},
	{0x0715, 0x00},
	{0x0716, 0x01},
	{0x0717, 0x00},
	{0x0718, 0x01},
	{0x0719, 0x00},

	{0x4500, 0x1F}
};


static struct msm_camera_i2c_reg_conf imx135_snap_settings[] = {

	{0x011E, 0x19},
	{0x011F, 0x99},
	{0x0301, 0x05},
	{0x0303, 0x01},
	{0x0305, 0x08},
	{0x0309, 0x05},
	{0x030B, 0x01},
	{0x030C, 0x00},
	{0x030D, 0xCD},
	{0x030E, 0x01},
	{0x3A06, 0x11},

	{0x0108, 0x03},
	{0x0112, 0x0A},
	{0x0113, 0x0A},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x01},
	{0x0390, 0x00},
	{0x0391, 0x11},
	{0x0392, 0x00},
	{0x0401, 0x00},
	{0x0404, 0x00},
	{0x0405, 0x10},
	{0x4082, 0x01},
	{0x4083, 0x01},
	{0x7006, 0x04},

	{0x0700, 0x00},
	{0x3A63, 0x00},
	{0x4100, 0xF8},
	{0x4203, 0xC3},
	{0x4344, 0x00},
	{0x441C, 0x01},

	{0x0340, 0x0E},
	{0x0341, 0xD8},
	{0x0342, 0x12},
	{0x0343, 0x38},
	{0x0344, 0x00},
	{0x0345, 0x00},
	{0x0346, 0x00},
	{0x0347, 0x00},
	{0x0348, 0x10},
	{0x0349, 0x6F},
	{0x034A, 0x0C},
	{0x034B, 0x3F},
	{0x034C, 0x10},
	{0x034D, 0x70},
	{0x034E, 0x0C},
	{0x034F, 0x40},
	{0x0350, 0x00},
	{0x0351, 0x00},
	{0x0352, 0x00},
	{0x0353, 0x00},
	{0x0354, 0x10},
	{0x0355, 0x70},
	{0x0356, 0x0C},
	{0x0357, 0x40},
	{0x301D, 0x28},
	{0x3310, 0x10},
	{0x3311, 0x70},
	{0x3312, 0x0C},
	{0x3313, 0x40},
	{0x331C, 0x01},
	{0x331D, 0xE0},
	{0x4084, 0x00},
	{0x4085, 0x00},
	{0x4086, 0x00},
	{0x4087, 0x00},
	{0x4400, 0x00},

	{0x0830, 0x77},
	{0x0831, 0x2F},
	{0x0832, 0x4F},
	{0x0833, 0x2F},
	{0x0834, 0x2F},
	{0x0835, 0x37},
	{0x0836, 0xA7},
	{0x0837, 0x37},
	{0x0839, 0x1F},
	{0x083A, 0x17},
	{0x083B, 0x02},

	{0x0202, 0x0E},
	{0x0203, 0xD4},

	{0x0205, 0x00},
	{0x020E, 0x01},
	{0x020F, 0x00},
	{0x0210, 0x01},
	{0x0211, 0x00},
	{0x0212, 0x01},
	{0x0213, 0x00},
	{0x0214, 0x01},
	{0x0215, 0x00},

	{0x0230, 0x00},
	{0x0231, 0x00},
	{0x0233, 0x00},
	{0x0234, 0x00},
	{0x0235, 0x40},
	{0x0238, 0x00},
	{0x0239, 0x04},
	{0x023B, 0x00},
	{0x023C, 0x01},
	{0x33B0, 0x04},
	{0x33B1, 0x00},
	{0x33B3, 0x00},
	{0x33B4, 0x01},
	{0x3800, 0x00},
};


static struct msm_camera_i2c_reg_conf imx135_prev_settings[] = {

	{0x011E, 0x19},
	{0x011F, 0x99},
	{0x0301, 0x05},
	{0x0303, 0x01},
	{0x0305, 0x08},
	{0x0309, 0x05},
	{0x030B, 0x02},
	{0x030C, 0x00},
	{0x030D, 0xCD},
	{0x030E, 0x01},
	{0x3A06, 0x12},

	{0x0108, 0x03},
	{0x0112, 0x0A},
	{0x0113, 0x0A},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x01},
	{0x0390, 0x01},
	{0x0391, 0x22},
	{0x0392, 0x00},
	{0x0401, 0x00},
	{0x0404, 0x00},
	{0x0405, 0x10},
	{0x4082, 0x01},
	{0x4083, 0x01},
	{0x7006, 0x04},

	{0x0700, 0x00},
	{0x3A63, 0x00},
	{0x4100, 0xF8},
	{0x4203, 0xFF},
	{0x4344, 0x00},
	{0x441C, 0x01},

	{0x0340, 0x07},
	{0x0341, 0x6E},
	{0x0342, 0x12},
	{0x0343, 0x38},
	{0x0344, 0x00},
	{0x0345, 0x00},
	{0x0346, 0x00},
	{0x0347, 0x00},
	{0x0348, 0x10},
	{0x0349, 0x6F},
	{0x034A, 0x0C},
	{0x034B, 0x3F},
	{0x034C, 0x08},
	{0x034D, 0x38},
	{0x034E, 0x06},
	{0x034F, 0x20},
	{0x0350, 0x00},
	{0x0351, 0x00},
	{0x0352, 0x00},
	{0x0353, 0x00},
	{0x0354, 0x08},
	{0x0355, 0x38},
	{0x0356, 0x06},
	{0x0357, 0x20},
	{0x301D, 0x28},
	{0x3310, 0x08},
	{0x3311, 0x38},
	{0x3312, 0x06},
	{0x3313, 0x20},
	{0x331C, 0x00},
	{0x331D, 0x8C},
	{0x4084, 0x00},
	{0x4085, 0x00},
	{0x4086, 0x00},
	{0x4087, 0x00},
	{0x4400, 0x00},

	{0x0830, 0x5F},
	{0x0831, 0x1F},
	{0x0832, 0x3F},
	{0x0833, 0x1F},
	{0x0834, 0x1F},
	{0x0835, 0x17},
	{0x0836, 0x67},
	{0x0837, 0x27},
	{0x0839, 0x1F},
	{0x083A, 0x17},
	{0x083B, 0x02},

	{0x0202, 0x07},
	{0x0203, 0x6A},

	{0x0205, 0x00},
	{0x020E, 0x01},
	{0x020F, 0x00},
	{0x0210, 0x01},
	{0x0211, 0x00},
	{0x0212, 0x01},
	{0x0213, 0x00},
	{0x0214, 0x01},
	{0x0215, 0x00},

	{0x0230, 0x00},
	{0x0231, 0x00},
	{0x0233, 0x00},
	{0x0234, 0x00},
	{0x0235, 0x40},
	{0x0238, 0x00},
	{0x0239, 0x04},
	{0x023B, 0x00},
	{0x023C, 0x01},
	{0x33B0, 0x04},
	{0x33B1, 0x00},
	{0x33B3, 0x00},
	{0x33B4, 0x01},
	{0x3800, 0x00},
};

static struct msm_camera_i2c_reg_conf imx135_fhd_video_30fps_settings[] = {

	{0x011E, 0x19},
	{0x011F, 0x99},
	{0x0301, 0x05},
	{0x0303, 0x01},
	{0x0305, 0x08},
	{0x0309, 0x05},
	{0x030B, 0x02},
	{0x030C, 0x00},
	{0x030D, 0xCD},
	{0x030E, 0x01},
	{0x3A06, 0x12},

	{0x0108, 0x03},
	{0x0112, 0x0A},
	{0x0113, 0x0A},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x01},
	{0x0390, 0x01},
	{0x0391, 0x22},
	{0x0392, 0x00},
	{0x0401, 0x00},
	{0x0404, 0x00},
	{0x0405, 0x10},
	{0x4082, 0x01},
	{0x4083, 0x01},
	{0x7006, 0x04},

	{0x0700, 0x00},
	{0x3A63, 0x00},
	{0x4100, 0xF8},
	{0x4203, 0xFF},
	{0x4344, 0x00},
	{0x441C, 0x01},

	{0x0340, 0x07},
	{0x0341, 0x6E},
	{0x0342, 0x12},
	{0x0343, 0x38},
	{0x0344, 0x00},
	{0x0345, 0x00},
	{0x0346, 0x01},
	{0x0347, 0x78},
	{0x0348, 0x10},
	{0x0349, 0x6F},
	{0x034A, 0x0A},
	{0x034B, 0xB7},
	{0x034C, 0x08},
	{0x034D, 0x38},
	{0x034E, 0x04},
	{0x034F, 0xA0},
	{0x0350, 0x00},
	{0x0351, 0x00},
	{0x0352, 0x00},
	{0x0353, 0x00},
	{0x0354, 0x08},
	{0x0355, 0x38},
	{0x0356, 0x04},
	{0x0357, 0xA0},
	{0x301D, 0x30},
	{0x3310, 0x08},
	{0x3311, 0x38},
	{0x3312, 0x04},
	{0x3313, 0xA0},
	{0x331C, 0x00},
	{0x331D, 0x8C},
	{0x4084, 0x00},
	{0x4085, 0x00},
	{0x4086, 0x00},
	{0x4087, 0x00},
	{0x4400, 0x00},

	{0x0830, 0x5F},
	{0x0831, 0x1F},
	{0x0832, 0x3F},
	{0x0833, 0x1F},
	{0x0834, 0x1F},
	{0x0835, 0x17},
	{0x0836, 0x67},
	{0x0837, 0x27},
	{0x0839, 0x1F},
	{0x083A, 0x17},
	{0x083B, 0x02},

	{0x0202, 0x07},
	{0x0203, 0x6A},

	{0x0205, 0x00},
	{0x020E, 0x01},
	{0x020F, 0x00},
	{0x0210, 0x01},
	{0x0211, 0x00},
	{0x0212, 0x01},
	{0x0213, 0x00},
	{0x0214, 0x01},
	{0x0215, 0x00},

	{0x0230, 0x00},
	{0x0231, 0x00},
	{0x0233, 0x00},
	{0x0234, 0x00},
	{0x0235, 0x40},
	{0x0238, 0x00},
	{0x0239, 0x04},
	{0x023B, 0x00},
	{0x023C, 0x01},
	{0x33B0, 0x04},
	{0x33B1, 0x00},
	{0x33B3, 0x00},
	{0x33B4, 0x01},
	{0x3800, 0x00},
};

static struct msm_camera_i2c_reg_conf imx135_fhd_hdr_video_24fps_settings[] = {

	{0x011E, 0x19},
	{0x011F, 0x99},
	{0x0301, 0x05},
	{0x0303, 0x01},
	{0x0305, 0x08},
	{0x0309, 0x05},
	{0x030B, 0x02},
	{0x030C, 0x00},
	{0x030D, 0xCD},
	{0x030E, 0x01},
	{0x3A06, 0x12},

	{0x0108, 0x03},
	{0x0112, 0x0E},
	{0x0113, 0x0A},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x01},
	{0x0390, 0x00},
	{0x0391, 0x11},
	{0x0392, 0x00},
	{0x0401, 0x00},
	{0x0404, 0x00},
	{0x0405, 0x10},
	{0x4082, 0x01},
	{0x4083, 0x01},
	{0x7006, 0x04},

	{0x0700, 0x00},
	{0x3A63, 0x00},
	{0x4100, 0xF8},
	{0x4203, 0xFF},
	{0x4344, 0x00},
	{0x441C, 0x01},

	{0x0340, 0x09},
	{0x0341, 0x58},
	{0x0342, 0x12},
	{0x0343, 0x38},
	{0x0344, 0x00},
	{0x0345, 0x00},
	{0x0346, 0x01},
	{0x0347, 0x78},
	{0x0348, 0x10},
	{0x0349, 0x6F},
	{0x034A, 0x0A},
	{0x034B, 0xB7},
	{0x034C, 0x08},
	{0x034D, 0x38},
	{0x034E, 0x04},
	{0x034F, 0xA0},
	{0x0350, 0x00},
	{0x0351, 0x00},
	{0x0352, 0x00},
	{0x0353, 0x00},
	{0x0354, 0x08},
	{0x0355, 0x38},
	{0x0356, 0x04},
	{0x0357, 0xA0},
	{0x301D, 0x30},
	{0x3310, 0x08},
	{0x3311, 0x38},
	{0x3312, 0x04},
	{0x3313, 0xA0},
	{0x331C, 0x00},
	{0x331D, 0x10},
	{0x4084, 0x00},
	{0x4085, 0x00},
	{0x4086, 0x00},
	{0x4087, 0x00},
	{0x4400, 0x00},

	{0x0830, 0x5F},
	{0x0831, 0x1F},
	{0x0832, 0x3F},
	{0x0833, 0x1F},
	{0x0834, 0x1F},
	{0x0835, 0x17},
	{0x0836, 0x67},
	{0x0837, 0x27},
	{0x0839, 0x1F},
	{0x083A, 0x17},
	{0x083B, 0x02},

	{0x0202, 0x09},
	{0x0203, 0x54},

	{0x0205, 0x00},
	{0x020E, 0x01},
	{0x020F, 0x00},
	{0x0210, 0x01},
	{0x0211, 0x00},
	{0x0212, 0x01},
	{0x0213, 0x00},
	{0x0214, 0x01},
	{0x0215, 0x00},

	{0x0230, 0x00},
	{0x0231, 0x00},
	{0x0233, 0x00},
	{0x0234, 0x00},
	{0x0235, 0x40},
	{0x0238, 0x00},
	{0x0239, 0x04},
	{0x023B, 0x03},
	{0x023C, 0x01},
	{0x33B0, 0x08},
	{0x33B1, 0x38},
	{0x33B3, 0x01},
	{0x33B4, 0x01},
	{0x3800, 0x00},
};

static struct msm_camera_i2c_reg_conf imx135_snap_24fps_settings[] = {

	{0x011E, 0x19},
	{0x011F, 0x99},
	{0x0301, 0x05},
	{0x0303, 0x01},
	{0x0305, 0x04},
	{0x0309, 0x05},
	{0x030B, 0x01},
	{0x030C, 0x00},
	{0x030D, 0x7D},
	{0x030E, 0x01},
	{0x3A06, 0x11},

	{0x0108, 0x03},
	{0x0112, 0x0A},
	{0x0113, 0x0A},
	{0x0381, 0x01},
	{0x0383, 0x01},
	{0x0385, 0x01},
	{0x0387, 0x01},
	{0x0390, 0x00},
	{0x0391, 0x11},
	{0x0392, 0x00},
	{0x0401, 0x00},
	{0x0404, 0x00},
	{0x0405, 0x10},
	{0x4082, 0x01},
	{0x4083, 0x01},
	{0x7006, 0x04},

	{0x0700, 0x00},
	{0x3A63, 0x00},
	{0x4100, 0xF8},
	{0x4203, 0xC3},
	{0x4344, 0x00},
	{0x441C, 0x01},

	{0x0340, 0x0C},
	{0x0341, 0x56},
	{0x0342, 0x11},
	{0x0343, 0xDC},
	{0x0344, 0x00},
	{0x0345, 0x00},
	{0x0346, 0x00},
	{0x0347, 0x00},
	{0x0348, 0x10},
	{0x0349, 0x6F},
	{0x034A, 0x0C},
	{0x034B, 0x3F},
	{0x034C, 0x10},
	{0x034D, 0x70},
	{0x034E, 0x0C},
	{0x034F, 0x40},
	{0x0350, 0x00},
	{0x0351, 0x00},
	{0x0352, 0x00},
	{0x0353, 0x00},
	{0x0354, 0x10},
	{0x0355, 0x70},
	{0x0356, 0x0C},
	{0x0357, 0x40},
	{0x301D, 0x28},
	{0x3310, 0x10},
	{0x3311, 0x70},
	{0x3312, 0x0C},
	{0x3313, 0x40},
	{0x331C, 0x01},
	{0x331D, 0x68},
	{0x4084, 0x00},
	{0x4085, 0x00},
	{0x4086, 0x00},
	{0x4087, 0x00},
	{0x4400, 0x00},

	{0x0830, 0x7F},
	{0x0831, 0x37},
	{0x0832, 0x5F},
	{0x0833, 0x37},
	{0x0834, 0x37},
	{0x0835, 0x3F},
	{0x0836, 0xC7},
	{0x0837, 0x3F},
	{0x0839, 0x1F},
	{0x083A, 0x17},
	{0x083B, 0x02},

	{0x0202, 0x0C},
	{0x0203, 0x52},

	{0x0205, 0x00},
	{0x020E, 0x01},
	{0x020F, 0x00},
	{0x0210, 0x01},
	{0x0211, 0x00},
	{0x0212, 0x01},
	{0x0213, 0x00},
	{0x0214, 0x01},
	{0x0215, 0x00},

	{0x0230, 0x00},
	{0x0231, 0x00},
	{0x0233, 0x00},
	{0x0234, 0x00},
	{0x0235, 0x40},
	{0x0238, 0x00},
	{0x0239, 0x04},
	{0x023B, 0x00},
	{0x023C, 0x01},
	{0x33B0, 0x04},
	{0x33B1, 0x00},
	{0x33B3, 0x00},
	{0x33B4, 0x01},
	{0x3800, 0x00},
};

static struct v4l2_subdev_info imx135_subdev_info[] = {
	{
	.code		= V4L2_MBUS_FMT_SBGGR10_1X10,
	.colorspace	= V4L2_COLORSPACE_JPEG,
	.fmt		= 1,
	.order		= 0,
	},
	
};

static struct msm_camera_i2c_conf_array imx135_init_conf[] = {
	{&imx135_recommend_settings[0],
	ARRAY_SIZE(imx135_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array imx135_confs[] = {
	{&imx135_snap_settings[0],
	ARRAY_SIZE(imx135_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx135_prev_settings[0],
	ARRAY_SIZE(imx135_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx135_fhd_video_30fps_settings[0],
	ARRAY_SIZE(imx135_fhd_video_30fps_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx135_fhd_hdr_video_24fps_settings[0],
	ARRAY_SIZE(imx135_fhd_hdr_video_24fps_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&imx135_snap_24fps_settings[0],
	ARRAY_SIZE(imx135_snap_24fps_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t imx135_dimensions[] = {
	
	{
		.x_output = 4208,
		.y_output = 3136,
		.line_length_pclk = 4664,
		.frame_length_lines = 3800,
		.vt_pixel_clk = 262400000,
		.op_pixel_clk = 262400000,
		.binning_factor = 1,
	},
	
	{
		.x_output = 2104,
		.y_output = 1568,
		.line_length_pclk = 4664,
		.frame_length_lines = 1902,
		.vt_pixel_clk = 262400000,
		.op_pixel_clk = 131200000,
		.binning_factor = 1,
	},
	
	{
		.x_output = 2104,
		.y_output = 1184,
		.line_length_pclk = 4664,
		.frame_length_lines = 1902,
		.vt_pixel_clk = 262400000,
		.op_pixel_clk = 131200000,
		.binning_factor = 1,
	},
	
	{
		.x_output = 2104,
		.y_output = 1184,
		.line_length_pclk = 4664,
		.frame_length_lines = 2392,
		.vt_pixel_clk = 262400000,
		.op_pixel_clk = 131200000,
		.binning_factor = 1,
	},
	
	{
		.x_output = 4208,
		.y_output = 3136,
		.line_length_pclk = 4572,
		.frame_length_lines = 3158,
		.vt_pixel_clk = 320000000,
		.op_pixel_clk = 320000000,
		.binning_factor = 1,
	},
};

static struct msm_sensor_output_reg_addr_t imx135_reg_addr = {
	.x_output = 0x34C,
	.y_output = 0x34E,
	.line_length_pclk = 0x342,
	.frame_length_lines = 0x340,
};

static struct msm_sensor_id_info_t imx135_id_info = {
	.sensor_id_reg_addr = 0x0016,
	.sensor_id = 0x0135,
};

static struct msm_sensor_exp_gain_info_t imx135_exp_gain_info = {
	.coarse_int_time_addr = 0x202,
	.global_gain_addr = 0x205,
	.vert_offset = 4,
};

static const struct i2c_device_id imx135_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&imx135_s_ctrl},
	{ }
};

static struct i2c_driver imx135_i2c_driver = {
	.id_table = imx135_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client imx135_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&imx135_i2c_driver);
}

static struct v4l2_subdev_core_ops imx135_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops imx135_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops imx135_subdev_ops = {
	.core = &imx135_subdev_core_ops,
	.video  = &imx135_subdev_video_ops,
};

int32_t imx135_write_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,


	uint16_t gain, uint32_t line, int32_t luma_avg, uint16_t fgain)

{
	uint32_t fl_lines;
	uint8_t offset;
	fl_lines = s_ctrl->curr_frame_length_lines;
	fl_lines = (fl_lines * s_ctrl->fps_divider) / Q10;
	offset = s_ctrl->sensor_exp_gain_info->vert_offset;
	if (line > (fl_lines - offset))
		fl_lines = line + offset;

	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines, fl_lines,
		MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr, line,
		MSM_CAMERA_I2C_WORD_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr, gain,
		MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	return 0;
}


static int imx135_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
    struct msm_camera_sensor_info *data = s_ctrl->sensordata;
    IMX135_LOG_DBG("START\n");

    
    if (data->sensor_platform_info->i2c_conf &&
        data->sensor_platform_info->i2c_conf->use_i2c_mux)
    {
        msm_sensor_disable_i2c_mux(
            data->sensor_platform_info->i2c_conf);
        IMX135_LOG_DBG("msm_sensor_disable_i2c_mux()\n");
    }

    
    IMX135_LOG_DBG("wait %d[ms]\n",IMX135_WAIT_PWOFF_I2C_STOP);
    mdelay(IMX135_WAIT_PWOFF_I2C_STOP);


    
    msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
        imx135_cam_clk_info, s_ctrl->cam_clk, ARRAY_SIZE(imx135_cam_clk_info), 0);

    msm_camera_request_gpio_table(data, 0);

    
    IMX135_LOG_DBG("wait %d[ms]\n",IMX135_WAIT_PWOFF_MCLK);
    mdelay(IMX135_WAIT_PWOFF_MCLK);


    
    pm8xxx_gpio_config(IMX135_PMGPIO_CAM_RST_N, &imx135_cam_v_en_off);

    IMX135_LOG_DBG("PMGPIO[28]:OFF\n");

    
    IMX135_LOG_DBG("wait %d[ms]\n",IMX135_WAIT_PWOFF_RST_N);
    mdelay(IMX135_WAIT_PWOFF_RST_N);


    
    if (cam_vana){
        regulator_set_optimum_mode(cam_vana, 0);
        regulator_set_voltage(cam_vana, 0, CAM_VANA_MAXUV);
        regulator_disable(cam_vana);

        IMX135_LOG_DBG("cam_vana:%d [ON/OFF]",regulator_is_enabled(cam_vana));
        IMX135_LOG_DBG("cam_vana:%d [V]",regulator_get_voltage(cam_vana));
        regulator_put(cam_vana);

        cam_vana = NULL;
    }

    
    IMX135_LOG_DBG("wait %d[ms]\n",IMX135_WAIT_PWOFF_VREG_L11);
    mdelay(IMX135_WAIT_PWOFF_VREG_L11);


    
    pm8xxx_gpio_config(IMX135_PMGPIO_CAM_V_EN1, &imx135_cam_v_en_off);

    IMX135_LOG_DBG("PMGPIO[13]:OFF\n");

    
    IMX135_LOG_DBG("wait %d[ms]\n",IMX135_WAIT_PWOFF_V_EN1);
    mdelay(IMX135_WAIT_PWOFF_V_EN1);


    
    pm8xxx_gpio_config(IMX135_PMGPIO_CAM_V_EN2, &imx135_cam_v_en_off);

    IMX135_LOG_DBG("PMGPIO[26]:OFF\n");

    
    IMX135_LOG_DBG("wait %d[ms]\n",IMX135_WAIT_PWOFF_V_EN2);
    mdelay(IMX135_WAIT_PWOFF_V_EN2);

    
    if (cam_vdig) {
        regulator_set_voltage(cam_vdig, 0, CAM_VDIG_MAXUV);
        regulator_set_optimum_mode(cam_vdig, 0);
        regulator_disable(cam_vdig);

        IMX135_LOG_DBG("cam_vdig:%d [ON/OFF]",regulator_is_enabled(cam_vdig));
        IMX135_LOG_DBG("cam_vdig:%d [V]",regulator_get_voltage(cam_vdig));
        regulator_put(cam_vdig);

        cam_vdig = NULL;
    }

    
    IMX135_LOG_DBG("wait %d[ms]\n",IMX135_WAIT_PWOFF_VREG_L12);

    IMX135_LOG_DBG("END\n");

    return 0;
}


static int32_t imx135_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc = 0;
    struct msm_camera_sensor_info *data = s_ctrl->sensordata;

    IMX135_LOG_DBG("START\n");

    
    if (cam_vdig == NULL) {
        IMX135_LOG_DBG("cam_vdig ON START\n");
        cam_vdig = regulator_get(&s_ctrl->sensor_i2c_client->client->dev, "cam_vdig");
        if (IS_ERR(cam_vdig)) {
            IMX135_LOG_ERR("VREG CAM VDIG get failed\n");
	      goto vdig_regulator_get_err;
        }
        if (regulator_set_voltage(cam_vdig, CAM_VDIG_MINUV,
            CAM_VDIG_MAXUV)) {
            IMX135_LOG_ERR("VREG CAM VDIG set voltage failed\n");
            goto vdig_set_vol_err;
        }
        if (regulator_set_optimum_mode(cam_vdig,
            CAM_VDIG_LOAD_UA) < 0) {
            IMX135_LOG_ERR("VREG CAM VDIG set optimum mode failed\n");
	      goto vdig_set_optimum_err;
        }
        if (regulator_enable(cam_vdig)) {
            IMX135_LOG_ERR("VREG CAM VDIG enable failed\n");
            goto vdig_regulator_en_err;
        }
    }

    IMX135_LOG_DBG("cam_vdig:%d [ON/OFF]",regulator_is_enabled(cam_vdig));
    IMX135_LOG_DBG("cam_vdig:%d [V]",regulator_get_voltage(cam_vdig));

    
    IMX135_LOG_DBG("wait %d[ms]\n",IMX135_WAIT_PWON_VREG_L12);
    mdelay(IMX135_WAIT_PWON_VREG_L12);


    
    rc = pm8xxx_gpio_config(IMX135_PMGPIO_CAM_V_EN2, &imx135_cam_v_en_on);
    if (rc) {
        
        IMX135_LOG_ERR("IMX135_PMGPIO_CAM_V_EN2(%d) Error, rc = %d\n", IMX135_PMGPIO_CAM_V_EN2, rc);
	  goto pmgpio_26_config_err;
    }
    IMX135_LOG_DBG("PMGPIO[26]:ON\n");

    
    IMX135_LOG_DBG("wait %d[ms]\n",IMX135_WAIT_PWON_V_EN2);
    mdelay(IMX135_WAIT_PWON_V_EN2);


    
    rc = pm8xxx_gpio_config(IMX135_PMGPIO_CAM_V_EN1, &imx135_cam_v_en_on);
    if (rc) {
        
        IMX135_LOG_ERR("IMX135_PMGPIO_CAM_V_EN1(%d) Error, rc = %d\n", IMX135_PMGPIO_CAM_V_EN1, rc);
	  goto pmgpio_13_config_err;
    }
    IMX135_LOG_DBG("PMGPIO[13]:ON\n");

    
    IMX135_LOG_DBG("wait %d[ms]\n",IMX135_WAIT_PWON_V_EN1);
    mdelay(IMX135_WAIT_PWON_V_EN1);


    
    if (cam_vana == NULL) {
        IMX135_LOG_DBG("cam_vana START\n");
        cam_vana = regulator_get(&s_ctrl->sensor_i2c_client->client->dev, "cam_vana");
        if (IS_ERR(cam_vana)) {
            IMX135_LOG_ERR("VREG CAM VANA get failed\n");
	      goto vana_regulator_get_err;
        }
        if (regulator_set_voltage(cam_vana, CAM_VANA_MINUV,
            CAM_VANA_MAXUV)) {
            IMX135_LOG_ERR("VREG CAM VANA set voltage failed\n");
	      goto vana_set_vol_err;
        }
        if (regulator_set_optimum_mode(cam_vana,
            CAM_VANA_LOAD_UA) < 0) {
            IMX135_LOG_ERR("VREG CAM VANA set optimum mode failed\n");
            goto vana_set_optimum_err;
        }
        if (regulator_enable(cam_vana)) {
            IMX135_LOG_ERR("VREG CAM VANA enable failed\n");
            goto vana_regulator_en_err;
        }
    }

    IMX135_LOG_DBG("cam_vana:%d [ON/OFF]",regulator_is_enabled(cam_vana));
    IMX135_LOG_DBG("cam_vana:%d [V]",regulator_get_voltage(cam_vana));

    
    IMX135_LOG_DBG("wait %d[ms]\n",IMX135_WAIT_PWON_VREG_L11);
    mdelay(IMX135_WAIT_PWON_VREG_L11);



    
    rc = pm8xxx_gpio_config(IMX135_PMGPIO_CAM_RST_N, &imx135_cam_v_en_on);
    if (rc) {
        
        IMX135_LOG_DBG("IMX135_PMGPIO_CAM_RST_N(%d) Error, rc = %d\n", IMX135_PMGPIO_CAM_RST_N, rc);
	  goto pmgpio_28_config_err;
    }
    IMX135_LOG_DBG("PMGPIO[28]:ON\n");

    
    IMX135_LOG_DBG("wait %d[ms]\n",IMX135_WAIT_PWON_RST_N);
    mdelay(IMX135_WAIT_PWON_RST_N);


    
    rc = msm_camera_request_gpio_table(data, 1);
    if (rc < 0) {
        IMX135_LOG_ERR("request gpio failed\n");
	  goto mclk_request_gpio_err;
    }

    if (s_ctrl->clk_rate != 0)
        imx135_cam_clk_info->clk_rate = s_ctrl->clk_rate;

    rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
        imx135_cam_clk_info, s_ctrl->cam_clk, ARRAY_SIZE(imx135_cam_clk_info), 1);
    if (rc < 0) {
        IMX135_LOG_ERR("clk enable failed\n");
        goto mclk_enable_err;
    }
    IMX135_LOG_DBG("MCLK ENABLE\n");

    
    IMX135_LOG_DBG("wait %d[ms]\n",IMX135_WAIT_PWON_MCLK);
    mdelay(IMX135_WAIT_PWON_MCLK);

    
    if (data->sensor_platform_info->i2c_conf &&
        data->sensor_platform_info->i2c_conf->use_i2c_mux){
        msm_sensor_enable_i2c_mux(data->sensor_platform_info->i2c_conf);
        IMX135_LOG_DBG("msm_sensor_enable_i2c_mux()\n");
    }
    IMX135_LOG_DBG("END\n");
    return rc;

mclk_enable_err:
    msm_camera_request_gpio_table(data, 0);
mclk_request_gpio_err:
    pm8xxx_gpio_config(IMX135_PMGPIO_CAM_RST_N, &imx135_cam_v_en_off);
pmgpio_28_config_err:
    regulator_disable(cam_vana);
vana_regulator_en_err:
    regulator_set_optimum_mode(cam_vana, 0);
vana_set_optimum_err:
    regulator_set_voltage(cam_vana, 0, CAM_VDIG_MAXUV);
vana_set_vol_err:
    regulator_put(cam_vana);
vana_regulator_get_err:
    cam_vana = NULL;
    pm8xxx_gpio_config(IMX135_PMGPIO_CAM_V_EN1, &imx135_cam_v_en_off);
pmgpio_13_config_err:
    pm8xxx_gpio_config(IMX135_PMGPIO_CAM_V_EN2, &imx135_cam_v_en_off);
pmgpio_26_config_err:
    regulator_disable(cam_vdig);
vdig_regulator_en_err:
    regulator_set_optimum_mode(cam_vdig, 0);
vdig_set_optimum_err:
    regulator_set_voltage(cam_vdig, 0, CAM_VDIG_MAXUV);
vdig_set_vol_err:
    regulator_put(cam_vdig);
vdig_regulator_get_err:
    cam_vdig = NULL;
    IMX135_LOG_ERR("power up imx135 error\n");
    return -EFAULT;
}




static int32_t imx135_get_exif_param(struct msm_sensor_ctrl_t *s_ctrl,
                                      struct get_exif_param_inf *get_exif_param)
{
    int32_t rc = 0;

    uint16_t coarse_integration_time = 0x0000; 
    uint16_t line_length_DVE046         = 0x0000; 
    uint16_t fine_integration_time   = 0x0000; 
    uint16_t analog_gain_code_global = 0x0000; 
    uint16_t digital_gain_greenr     = 0x0000; 

    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,
                             0x3012, &coarse_integration_time, MSM_CAMERA_I2C_WORD_DATA);
    if(rc < 0){
        return rc;
    }

    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,
                             0x300C, &line_length_DVE046, MSM_CAMERA_I2C_WORD_DATA);
    if(rc < 0){
        return rc;
    }

    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,
                             0x3028, &analog_gain_code_global, MSM_CAMERA_I2C_WORD_DATA);
    if(rc < 0){
        return rc;
    }

    get_exif_param->coarse_integration_time = coarse_integration_time; 
    get_exif_param->line_length_DVE046         = line_length_DVE046;         
    get_exif_param->fine_integration_time   = fine_integration_time;   
    get_exif_param->analog_gain_code_global = analog_gain_code_global; 
    get_exif_param->digital_gain_greenr     = digital_gain_greenr;     


    IMX135_LOG_DBG("for debug");
    IMX135_LOG_DBG("coarse_integration_time  : 0x%x",coarse_integration_time);  
    IMX135_LOG_DBG("line_length_DVE046          : 0x%x",line_length_DVE046);          
    IMX135_LOG_DBG("fine_integration_time    : 0x%x",fine_integration_time);    
    IMX135_LOG_DBG("analog_gain_code_global  : 0x%x",analog_gain_code_global);  
    IMX135_LOG_DBG("digital_gain_greenr      : 0x%x",digital_gain_greenr);      

    return rc;
}

static uint16_t otp_data_awb[3];
static uint16_t otp_data_af[5];
static uint16_t otp_data_trial_ver;
static uint16_t otp_data_lot_code_date[3];
static uint16_t otp_data_lot_sum;





static int32_t imx135_sensor_otp_prepare_read(struct msm_sensor_ctrl_t *s_ctrl,uint16_t page)
{
    int32_t i=0;
	uint16_t check_status;
	
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3B02, page, MSM_CAMERA_I2C_BYTE_DATA);
	
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3B00, 0x01, MSM_CAMERA_I2C_BYTE_DATA);
	
	do{
		msleep(10);
		msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x3B01, &check_status,MSM_CAMERA_I2C_BYTE_DATA);

	}while(check_status != 0x01 && i++ < 5);
	return  ((check_status == 0x01)? 0 :-1);
}
static uint16_t imx135_sensor_otp_read_merge(struct msm_sensor_ctrl_t *s_ctrl,uint16_t addr)
{
	uint16_t data_l, data_h,ouput_d;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, addr, &data_l,MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, addr+1, &data_h,MSM_CAMERA_I2C_BYTE_DATA);
	ouput_d = data_l << 8 | data_h ;
	return ouput_d;
}

static int32_t imx135_sensor_otp_read(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc = 0;

	
	if (!imx135_sensor_otp_prepare_read(s_ctrl, 0)){
		otp_data_trial_ver =imx135_sensor_otp_read_merge (s_ctrl , 0x3B3A);
		
		otp_data_awb[0]=imx135_sensor_otp_read_merge (s_ctrl , 0x3B40);
		otp_data_awb[1]=imx135_sensor_otp_read_merge (s_ctrl , 0x3B42);
	}
	if (!imx135_sensor_otp_prepare_read(s_ctrl, 1)){
		otp_data_awb[2]=imx135_sensor_otp_read_merge (s_ctrl , 0x3B04);
		otp_data_af[0]=imx135_sensor_otp_read_merge (s_ctrl , 0x3B08);
		otp_data_af[1]=imx135_sensor_otp_read_merge (s_ctrl , 0x3B0A);
		otp_data_af[2]=imx135_sensor_otp_read_merge (s_ctrl , 0x3B0C);
		otp_data_af[3]=imx135_sensor_otp_read_merge (s_ctrl , 0x3B0E);
		otp_data_af[4]=imx135_sensor_otp_read_merge (s_ctrl , 0x3B10);
	}
	if (!imx135_sensor_otp_prepare_read(s_ctrl, 16)){
		otp_data_lot_code_date[0]=imx135_sensor_otp_read_merge (s_ctrl , 0x3B24);
		otp_data_lot_code_date[1]=imx135_sensor_otp_read_merge (s_ctrl , 0x3B26);
		otp_data_lot_code_date[2]=imx135_sensor_otp_read_merge (s_ctrl , 0x3B28);
		otp_data_lot_sum=imx135_sensor_otp_read_merge (s_ctrl , 0x3B32);
	}
	IMX135_LOG_DBG(">>>>otp_data_trial_ver=0x%04x\n",otp_data_trial_ver);

	IMX135_LOG_DBG(">>>>otp_data_awb[0]=0x%04x\n",otp_data_awb[0]);
	IMX135_LOG_DBG(">>>>otp_data_awb[1]=0x%04x\n",otp_data_awb[1]);
	IMX135_LOG_DBG(">>>>otp_data_awb[2]=0x%04x\n",otp_data_awb[2]);

	IMX135_LOG_DBG(">>>>otp_data_af[0]=0x%04x\n",otp_data_af[0]);
	IMX135_LOG_DBG(">>>>otp_data_af[1]=0x%04x\n",otp_data_af[1]);
	IMX135_LOG_DBG(">>>>otp_data_af[2]=0x%04x\n",otp_data_af[2]);
	IMX135_LOG_DBG(">>>>otp_data_af[3]=0x%04x\n",otp_data_af[3]);
	IMX135_LOG_DBG(">>>>otp_data_af[4]=0x%04x\n",otp_data_af[4]);

	IMX135_LOG_DBG(">>>>otp_data_lot_code_date[0]=0x%04x\n",otp_data_lot_code_date[0]);
	IMX135_LOG_DBG(">>>>otp_data_lot_code_date[1]=0x%04x\n",otp_data_lot_code_date[1]);
	IMX135_LOG_DBG(">>>>otp_data_lot_code_date[2]=0x%04x\n",otp_data_lot_code_date[2]);

	IMX135_LOG_DBG(">>>>otp_data_lot_sum=0x%04x\n",otp_data_lot_sum);

    return rc;
}


struct reg_access_param_t {
    uint16_t reg_addr;
    uint16_t reg_data;
};

int32_t imx135_sensor_write_exp_gain2(struct msm_sensor_ctrl_t *s_ctrl,
        uint16_t gain, uint16_t digital_gain_num, uint16_t digital_gain_dec, uint32_t line)
{
    int32_t rc = 0;
    uint32_t fl_lines;
    uint8_t offset;
    int cnt = 0;

    struct reg_access_param_t reg_access_params[]={
        {0x020E, digital_gain_num}, 
        {0x020F, digital_gain_dec}, 
        {0x0210, digital_gain_num}, 
        {0x0211, digital_gain_dec}, 
        {0x0212, digital_gain_num}, 
        {0x0213, digital_gain_dec}, 
        {0x0214, digital_gain_num}, 
        {0x0215, digital_gain_dec}, 
    };

    fl_lines = s_ctrl->curr_frame_length_lines;
    fl_lines = (fl_lines * s_ctrl->fps_divider) / Q10;
    offset = s_ctrl->sensor_exp_gain_info->vert_offset;
    if (line > (fl_lines - offset))
        fl_lines = line + offset;

    
    IMX135_LOG_DBG("AnalogGain:%d, DigitalGain:%d %d, Linecount:%d\n",
        gain, digital_gain_num, digital_gain_dec, line);

    s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
        s_ctrl->sensor_output_reg_addr->frame_length_lines,
        fl_lines, MSM_CAMERA_I2C_WORD_DATA);

    if(rc < 0){
        IMX135_LOG_ERR("write frame_length_lines failed\n");
        return rc;
    }

    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
        s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
        line, MSM_CAMERA_I2C_WORD_DATA);

    if(rc < 0){
        IMX135_LOG_ERR("write coarse_int_time_addr failed\n");
        return rc;
    }

    if(rc >= 0){
        
        
        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
            s_ctrl->sensor_exp_gain_info->global_gain_addr, gain,
            MSM_CAMERA_I2C_BYTE_DATA);
        

        if(rc < 0){
            IMX135_LOG_ERR("write global_gain_addr failed\n");
            return rc;
        }

        
        for(cnt=0; cnt < ARRAY_SIZE(reg_access_params); cnt++) {
            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
                reg_access_params[cnt].reg_addr, reg_access_params[cnt].reg_data,
                MSM_CAMERA_I2C_BYTE_DATA);

            if (rc < 0) {
                IMX135_LOG_ERR("write DigitalGain failed\n");
                return rc;
            }
        }
    }

    s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);

    return rc;
}





static int32_t imx135_get_maker_note(struct msm_sensor_ctrl_t *s_ctrl,
                                      struct get_exif_maker_note_cfg *get_exif_maker_note)
{

    get_exif_maker_note->fd_freq        = 0x0001;

    get_exif_maker_note->device_id      = imx135_id_info.sensor_id;
    get_exif_maker_note->awb_temp       = 0x0002;
    get_exif_maker_note->awb_gain_r     = 0x0003;
    get_exif_maker_note->awb_gain_g     = 0x0004;
    get_exif_maker_note->awb_gain_b     = 0x0005;
    get_exif_maker_note->awb_saturation = 0x0006;

	get_exif_maker_note->calib_rg           = otp_data_awb[0];
	get_exif_maker_note->calib_bg           = otp_data_awb[1];
	get_exif_maker_note->calib_grgb         = otp_data_awb[2];
	get_exif_maker_note->af_inf_position    = otp_data_af[0];
	get_exif_maker_note->af_1m_position     = otp_data_af[1];
	get_exif_maker_note->af_macro_position  = otp_data_af[2];
	get_exif_maker_note->start_current      = otp_data_af[3];
	get_exif_maker_note->op_sensitivity     = otp_data_af[4];
	get_exif_maker_note->trial_ver          = otp_data_trial_ver;
	get_exif_maker_note->lot_code_date      = otp_data_lot_code_date[0];
	get_exif_maker_note->lot_code_date1     = otp_data_lot_code_date[1];
	get_exif_maker_note->lot_code_date2     = otp_data_lot_code_date[2];
	get_exif_maker_note->lot_code_num       = (uint32_t)otp_data_lot_sum;

    return 0;
}



























static struct msm_sensor_fn_t imx135_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = imx135_write_exp_gain,
	.sensor_write_snapshot_exp_gain = imx135_write_exp_gain,

	.sensor_write_exp_gain2 = imx135_sensor_write_exp_gain2,
	.sensor_write_snapshot_exp_gain2 = imx135_sensor_write_exp_gain2,

	.sensor_setting = msm_sensor_setting,
	.sensor_csi_setting = msm_sensor_setting1,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = imx135_sensor_power_up,
	.sensor_power_down = imx135_sensor_power_down,
	.sensor_adjust_frame_lines = msm_sensor_adjust_frame_lines1,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
	.sensor_get_maker_note = imx135_get_maker_note,
	.sensor_get_eeprom_otp_info = NULL,
	.sensor_otp_read = imx135_sensor_otp_read,
	.sensor_get_exif_param = imx135_get_exif_param,

};

static struct msm_sensor_reg_t imx135_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = imx135_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(imx135_start_settings),
	.stop_stream_conf = imx135_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(imx135_stop_settings),
	.group_hold_on_conf = imx135_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(imx135_groupon_settings),
	.group_hold_off_conf = imx135_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(imx135_groupoff_settings),
	.init_settings = &imx135_init_conf[0],
	.init_size = ARRAY_SIZE(imx135_init_conf),
	.mode_settings = &imx135_confs[0],
	.output_settings = &imx135_dimensions[0],
	.num_conf = ARRAY_SIZE(imx135_confs),
};

static struct msm_sensor_ctrl_t imx135_s_ctrl = {
	.msm_sensor_reg = &imx135_regs,
	.sensor_i2c_client = &imx135_sensor_i2c_client,
	.sensor_i2c_addr = 0x20,
	.sensor_output_reg_addr = &imx135_reg_addr,
	.sensor_id_info = &imx135_id_info,
	.sensor_exp_gain_info = &imx135_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.msm_sensor_mutex = &imx135_mut,
	.sensor_i2c_driver = &imx135_i2c_driver,
	.sensor_v4l2_subdev_info = imx135_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(imx135_subdev_info),
	.sensor_v4l2_subdev_ops = &imx135_subdev_ops,
	.func_tbl = &imx135_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_25HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Sony 13MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");

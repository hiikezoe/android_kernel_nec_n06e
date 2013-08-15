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

#include <linux/module.h>
#include "msm_camera_eeprom.h"
#include "msm_camera_i2c.h"


#define IU091_EEPROM_BANK_SEL_REG 0x34C9


 DEFINE_MUTEX(iu091_eeprom_mutex);
static struct msm_eeprom_ctrl_t iu091_eeprom_t;

static const struct i2c_device_id iu091_eeprom_i2c_id[] = {
	{"iu091_eeprom", (kernel_ulong_t)&iu091_eeprom_t},
	{ }
};

static struct i2c_driver iu091_eeprom_i2c_driver = {
	.id_table = iu091_eeprom_i2c_id,
	.probe  = msm_eeprom_i2c_probe,
	.remove = __exit_p(iu091_eeprom_i2c_remove),
	.driver = {
		.name = "iu091_eeprom",
	},
};

static int __init iu091_eeprom_i2c_add_driver(void)
{
	int rc = 0;
	rc = i2c_add_driver(iu091_eeprom_t.i2c_driver);
	return rc;
}


void iu091_set_dev_addr(struct msm_eeprom_ctrl_t *eclient,
	uint32_t *reg_addr) {
	uint32_t eprom_addr = *reg_addr;
	msm_camera_i2c_write(&eclient->i2c_client, IU091_EEPROM_BANK_SEL_REG,
		(eprom_addr & 0xFF) >> 3, MSM_CAMERA_I2C_BYTE_DATA);
}


static struct v4l2_subdev_core_ops iu091_eeprom_subdev_core_ops = {
	.ioctl = msm_eeprom_subdev_ioctl,
};

static struct v4l2_subdev_ops iu091_eeprom_subdev_ops = {
	.core = &iu091_eeprom_subdev_core_ops,
};






uint8_t iu091_otp_data_1[8];
uint8_t iu091_otp_data_2[6];
uint8_t iu091_otp_data_3[90];
uint16_t iu091_calibration_data_1[4];
uint16_t iu091_calibration_data_2[3];


static struct msm_camera_eeprom_info_t iu091_calib_supp_info = {





        {TRUE, sizeof(iu091_calibration_data_1), 0, 1},
        {TRUE, sizeof(iu091_calibration_data_2), 1, 1024},
        {FALSE, 0, 0, 1},


        {FALSE, 0, 0, 1},


};

static struct msm_camera_eeprom_read_t iu091_eeprom_read_tbl[] = {



	{0x350E, &iu091_otp_data_1[0], 2, 0},
	{0x3510, &iu091_otp_data_1[2], 6, 0},
	{0x3508, &iu091_otp_data_2[0], 6, 0},
	{0x3518, &iu091_otp_data_3[0], 8, 0},
	{0x3520, &iu091_otp_data_3[8], 8, 0},
	{0x3528, &iu091_otp_data_3[16], 8, 0},
	{0x3530, &iu091_otp_data_3[24], 8, 0},
	{0x3538, &iu091_otp_data_3[32], 8, 0},
	{0x3540, &iu091_otp_data_3[40], 8, 0},
	{0x3548, &iu091_otp_data_3[48], 8, 0},
	{0x3550, &iu091_otp_data_3[56], 8, 0},
	{0x3558, &iu091_otp_data_3[64], 8, 0},
	{0x3560, &iu091_otp_data_3[72], 8, 0},
	{0x3568, &iu091_otp_data_3[80], 8, 0},
	{0x3570, &iu091_otp_data_3[88], 2, 0},

};


static struct msm_camera_eeprom_data_t iu091_eeprom_data_tbl[] = {



	{&iu091_calibration_data_1, sizeof(iu091_calibration_data_1)},
	{&iu091_calibration_data_2, sizeof(iu091_calibration_data_2)},
	{&iu091_otp_data_3, sizeof(iu091_otp_data_3)},

};






















static void iu091_format_data_1(void)
{
	iu091_calibration_data_1[0] = (uint16_t)(iu091_otp_data_1[0] << 8) |
		iu091_otp_data_1[1];
	iu091_calibration_data_1[1] = (uint16_t)(iu091_otp_data_1[2] << 8) |
		iu091_otp_data_1[3];
	iu091_calibration_data_1[2] = (uint16_t)(iu091_otp_data_1[4] << 8) |
		iu091_otp_data_1[5];
	iu091_calibration_data_1[3] = (uint16_t)(iu091_otp_data_1[6] << 8) |
		iu091_otp_data_1[7];
}

static void iu091_format_data_2(void)
{
	iu091_calibration_data_2[0] = (uint16_t)(iu091_otp_data_2[0] << 8) |
		iu091_otp_data_2[1];
	iu091_calibration_data_2[1] = (uint16_t)(iu091_otp_data_2[2] << 8) |
		iu091_otp_data_2[3];
	iu091_calibration_data_2[2] = (uint16_t)(iu091_otp_data_2[4] << 8) |
		iu091_otp_data_2[5];
}



void iu091_format_calibrationdata(void)
{



	iu091_format_data_1();
	iu091_format_data_2();

}
static struct msm_eeprom_ctrl_t iu091_eeprom_t = {
	.i2c_driver = &iu091_eeprom_i2c_driver,


	.i2c_addr = 0x34,

	.eeprom_v4l2_subdev_ops = &iu091_eeprom_subdev_ops,

	.i2c_client = {


		.addr_type = MSM_CAMERA_I2C_WORD_ADDR,

	},

	.eeprom_mutex = &iu091_eeprom_mutex,

	.func_tbl = {
		.eeprom_init = NULL,
		.eeprom_release = NULL,
		.eeprom_get_info = msm_camera_eeprom_get_info,
		.eeprom_get_data = msm_camera_eeprom_get_data,


		.eeprom_set_dev_addr = iu091_set_dev_addr,

		.eeprom_format_data = iu091_format_calibrationdata,
	},
	.info = &iu091_calib_supp_info,
	.info_size = sizeof(struct msm_camera_eeprom_info_t),
	.read_tbl = iu091_eeprom_read_tbl,
	.read_tbl_size = ARRAY_SIZE(iu091_eeprom_read_tbl),
	.data_tbl = iu091_eeprom_data_tbl,
	.data_tbl_size = ARRAY_SIZE(iu091_eeprom_data_tbl),
};

subsys_initcall(iu091_eeprom_i2c_add_driver);
MODULE_DESCRIPTION("iu091 EEPROM");
MODULE_LICENSE("GPL v2");

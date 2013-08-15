/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#include <linux/module.h>
#include "msm_camera_eeprom.h"
#include "msm_camera_i2c.h"

DEFINE_MUTEX(imx135_eeprom_mutex);
static struct msm_eeprom_ctrl_t imx135_eeprom_t;

#define DPC_SIZE 100

static const struct i2c_device_id imx135_eeprom_i2c_id[] = {
 {"imx135_eeprom", (kernel_ulong_t)&imx135_eeprom_t},
 { }
};

static struct i2c_driver imx135_eeprom_i2c_driver = {
 .id_table = imx135_eeprom_i2c_id,
 .probe  = msm_eeprom_i2c_probe,
 .remove = __exit_p(imx135_eeprom_i2c_remove),
 .driver = {
  .name = "imx135_eeprom",
 },
};

static int __init imx135_eeprom_i2c_add_driver(void)
{
 int rc = 0;
 rc = i2c_add_driver(imx135_eeprom_t.i2c_driver);
 return rc;
}

static struct v4l2_subdev_core_ops imx135_eeprom_subdev_core_ops = {
 .ioctl = msm_eeprom_subdev_ioctl,
};

static struct v4l2_subdev_ops imx135_eeprom_subdev_ops = {
 .core = &imx135_eeprom_subdev_core_ops,
};



uint8_t imx135_otp_data_1[8];
uint8_t imx135_otp_data_2[6];
uint8_t imx135_dpccalib_data[DPC_SIZE*4];


uint16_t imx135_calibration_data_1[4];
uint16_t imx135_calibration_data_2[3];
struct msm_calib_dpc imx135_dpc_data;

static struct msm_camera_eeprom_info_t imx135_calib_supp_info = {


 {TRUE, sizeof(imx135_calibration_data_1), 0, 1},
 {TRUE, sizeof(imx135_calibration_data_2), 1, 1024},
 {FALSE, 0, 0, 1},
 {FALSE, sizeof(struct msm_calib_dpc), 2, 1},
};

static struct msm_camera_eeprom_read_t imx135_eeprom_read_tbl[] = {










 {0x013B08, &imx135_otp_data_1[0], sizeof(imx135_otp_data_1), 0},
 {0x003B40, &imx135_otp_data_2[0], 4, 0},
 {0x013B04, &imx135_otp_data_2[4], 2, 0},
 {0x013B24, &imx135_dpccalib_data[0], 32, 0},
 {0x023B04, &imx135_dpccalib_data[32], 64, 0},
 {0x033B04, &imx135_dpccalib_data[96], 64, 0},
 {0x043B04, &imx135_dpccalib_data[160], 64, 0},
 {0x053B04, &imx135_dpccalib_data[224], 64, 0},
 {0x063B04, &imx135_dpccalib_data[288], 64, 0},
 {0x073B04, &imx135_dpccalib_data[352], 48, 0},
};

static struct msm_camera_eeprom_data_t imx135_eeprom_data_tbl[] = {


 {&imx135_calibration_data_1, sizeof(imx135_calibration_data_1)},
 {&imx135_calibration_data_2, sizeof(imx135_calibration_data_2)},
 {&imx135_dpc_data, sizeof(struct msm_calib_dpc)},
};

static void imx135_format_dpcdata(void)
{
 uint16_t i;
 uint16_t msb_xcord, lsb_xcord, msb_ycord, lsb_ycord, count = 0;

 for (i = 0; i < DPC_SIZE; i++) {
  msb_xcord = (imx135_dpccalib_data[i*4] & 0x001F);
  lsb_xcord = imx135_dpccalib_data[i*4+1];
  msb_ycord = (imx135_dpccalib_data[i*4+2] & 0x000F);
  lsb_ycord = imx135_dpccalib_data[i*4+3];
  imx135_dpc_data.snapshot_coord[i].x =
   (msb_xcord << 8) | lsb_xcord;
  imx135_dpc_data.snapshot_coord[i].y =
   (msb_ycord << 8) | lsb_ycord;
 }

 for (i = 0; i < DPC_SIZE; i++)
  if (!((imx135_dpc_data.snapshot_coord[i].x == 0) &&
   (imx135_dpc_data.snapshot_coord[i].y == 0)))
    count++;

 imx135_dpc_data.validcount = count;

 CDBG("%s count = %d\n", __func__, count);
 for (i = 0; i < count; i++)
  CDBG("snapshot_dpc_cord[%d] X= %d Y = %d\n",
  i, imx135_dpc_data.snapshot_coord[i].x,
  imx135_dpc_data.snapshot_coord[i].y);


 for (i = 0; i < count; i++) {
  imx135_dpc_data.preview_coord[i].x =
   imx135_dpc_data.snapshot_coord[i].x;
  imx135_dpc_data.preview_coord[i].y =
   ((imx135_dpc_data.snapshot_coord[i].y / 4)*2)
  + (imx135_dpc_data.snapshot_coord[i].y % 2);
 }


 for (i = 0; i < count; i++) {
  imx135_dpc_data.video_coord[i].x =
   imx135_dpc_data.snapshot_coord[i].x;
  imx135_dpc_data.video_coord[i].y =
   (imx135_dpc_data.snapshot_coord[i].y - 483) / 3;
 }
}






















static void imx135_format_data_2(void)
{
 imx135_calibration_data_2[0] = (uint16_t)(imx135_otp_data_2[0] << 8) |
  imx135_otp_data_2[1];
 imx135_calibration_data_2[1] = (uint16_t)(imx135_otp_data_2[2] << 8) |
  imx135_otp_data_2[3];
 imx135_calibration_data_2[2] = (uint16_t)(imx135_otp_data_2[4] << 8) |
  imx135_otp_data_2[5];
}

















static void imx135_format_data_1(void)
{
 imx135_calibration_data_1[0] = (uint16_t)(imx135_otp_data_1[0] << 8) |
  imx135_otp_data_1[1];
 imx135_calibration_data_1[1] = (uint16_t)(imx135_otp_data_1[2] << 8) |
  imx135_otp_data_1[3];
 imx135_calibration_data_1[2] = (uint16_t)(imx135_otp_data_1[4] << 8) |
  imx135_otp_data_1[5];
 imx135_calibration_data_1[3] = (uint16_t)(imx135_otp_data_1[6] << 8) |
  imx135_otp_data_1[7];
}

void imx135_format_calibrationdata(void)
{


 imx135_format_data_1();
 imx135_format_data_2();
 imx135_format_dpcdata();
}

void imx135_set_dev_addr(struct msm_eeprom_ctrl_t *ectrl,
 uint32_t *reg_addr) {
 int32_t rc = 0;

 rc = msm_camera_i2c_write(&ectrl->i2c_client,
  0x3B02, (*reg_addr) >> 16, MSM_CAMERA_I2C_BYTE_DATA);
 if (rc != 0)
  pr_err("%s: Page write error\n", __func__);

 rc = msm_camera_i2c_write(&ectrl->i2c_client,
  0x3B00, 0x01, MSM_CAMERA_I2C_BYTE_DATA);
 if (rc != 0)
  pr_err("%s: Turn ON read mode write error\n", __func__);

 rc = msm_camera_i2c_poll(&ectrl->i2c_client,
  0x3B01, 0x01, MSM_CAMERA_I2C_BYTE_DATA);
 if (rc != 0)
  pr_err("%s: Ready ready poll error\n", __func__);

}

static struct msm_eeprom_ctrl_t imx135_eeprom_t = {
 .i2c_driver = &imx135_eeprom_i2c_driver,
 .i2c_addr = 0x20,
 .eeprom_v4l2_subdev_ops = &imx135_eeprom_subdev_ops,

 .i2c_client = {
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
 },

 .eeprom_mutex = &imx135_eeprom_mutex,

 .func_tbl = {
  .eeprom_init = NULL,
  .eeprom_release = NULL,
  .eeprom_get_info = msm_camera_eeprom_get_info,
  .eeprom_get_data = msm_camera_eeprom_get_data,
  .eeprom_set_dev_addr = imx135_set_dev_addr,
  .eeprom_format_data = imx135_format_calibrationdata,
 },
 .info = &imx135_calib_supp_info,
 .info_size = sizeof(struct msm_camera_eeprom_info_t),
 .read_tbl = imx135_eeprom_read_tbl,
 .read_tbl_size = ARRAY_SIZE(imx135_eeprom_read_tbl),
 .data_tbl = imx135_eeprom_data_tbl,
 .data_tbl_size = ARRAY_SIZE(imx135_eeprom_data_tbl),
};

subsys_initcall(imx135_eeprom_i2c_add_driver);
MODULE_DESCRIPTION("imx135 EEPROM");
MODULE_LICENSE("GPL v2");

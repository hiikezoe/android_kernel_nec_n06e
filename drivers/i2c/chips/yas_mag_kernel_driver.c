/*
 * Copyright (c) 2010-2011 Yamaha Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#include <asm/atomic.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <asm/uaccess.h>
#include <linux/gpio.h>



#include "../../../include/linux/i2c/yas530_driver.h"
#include "../../../include/linux/i2c/yas.h"
#include "../../../drivers/i2c/chips/yas_mag_driver.c"


#define GEOMAGNETIC_I2C_DEVICE_NAME	"geomagnetic"
#define GEOMAGNETIC_INPUT_NAME		"geomagnetic"
#define GEOMAGNETIC_INPUT_RAW_NAME	"geomagnetic_raw"


#define ABS_STATUS			(ABS_BRAKE)
#define ABS_WAKE			(ABS_MISC)

#define ABS_RAW_DISTORTION		(ABS_THROTTLE)
#define ABS_RAW_THRESHOLD		(ABS_RUDDER)
#define ABS_RAW_SHAPE			(ABS_WHEEL)
#define ABS_RAW_MODE			(ABS_HAT0X)
#define ABS_RAW_REPORT			(ABS_GAS)
DEFINE_MUTEX(g_magne_mutex);


#define OFFSET_X_COMPLETED		(1)
#define OFFSET_X_UNCOMPLETED	(0)
#define OFFSET_Y1_COMPLETED		(1)
#define OFFSET_Y1_UNCOMPLETED	(0)
#define OFFSET_Y2_COMPLETED		(1)
#define OFFSET_Y2_UNCOMPLETED	(0)


static struct yas530_data			yas_k_data;

static struct yas530_mag_nv_data		g_mag_nv_data;


static struct yas530_mag_offset_data	g_backup_offset_data;

static int g_magne_is_nv_set = 0;
static int g_magne_i2c_error_alarmed = false; 

static int magne_diag_get(struct yas530_mag_user_data *diag_data);
static int magne_get_cal_regist(struct yas530_data	*k_data);
static int magne_soft_reset(void);
static int magne_init_device(void);
static int magne_diag_set_offset(struct yas530_mag_offset_data	*offset_data);

struct geomagnetic_data {
	struct input_dev *input_data;
	struct input_dev *input_raw;
	struct delayed_work work;
	struct semaphore driver_lock;
	struct semaphore multi_lock;
	atomic_t last_data[3];
	atomic_t last_status;
	atomic_t enable;
	int filter_enable;
	int filter_len;
	int32_t filter_noise[3];
	int32_t filter_threshold;
	int delay;
	int32_t threshold;
	int32_t distortion[3];
	int32_t shape;
	int32_t ellipsoid_mode;
	struct yas_mag_offset driver_offset;
#if DEBUG
	int suspend;
#endif



	struct yas_matrix static_matrix;
	struct yas_matrix dynamic_matrix;




};
static struct yas530_platform_data	*yas530_pdata;

static struct i2c_client *this_client = {0};
#include <linux/miscdevice.h>
















































































































































































































































static int
geomagnetic_i2c_open(void)
{
	return 0;
}

static int
geomagnetic_i2c_close(void)
{
	return 0;
}

#if YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS529
static int
geomagnetic_i2c_write(const uint8_t *buf, int len)
{
	if (i2c_master_send(this_client, buf, len) < 0)
		return -1;
#if DEBUG
	YLOGD(("[W] [%02x]\n", buf[0]));
#endif

	return 0;
}

static int
geomagnetic_i2c_read(uint8_t *buf, int len)
{
	if (i2c_master_recv(this_client, buf, len) < 0)
		return -1;

#if DEBUG
	if (len == 1) {
		YLOGD(("[R] [%02x]\n", buf[0]));
	} else if (len == 6) {
		YLOGD(("[R] "
		"[%02x%02x%02x%02x%02x%02x]\n",
		buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]));
	} else if (len == 8) {
		YLOGD(("[R] "
		"[%02x%02x%02x%02x%02x%02x%02x%02x]\n",
		buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6],
		buf[7]));
	} else if (len == 9) {
		YLOGD(("[R] "
		"[%02x%02x%02x%02x%02x%02x%02x%02x%02x]\n",
		buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
		buf[8]));
	} else if (len == 16) {
		YLOGD(("[R] "
		"[%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
		"%02x]\n",
		buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
		buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14],
		buf[15]));
	}
#endif

	return 0;
}

#else

static int
geomagnetic_i2c_write(uint8_t addr, const uint8_t *buf, int len)
{
	int ret = 0;
	int ret_i2c_write;
	uint8_t tmp[16];
	unsigned char info1 = 0x01;
	
	if (sizeof(tmp) - 1 < len)
		return -1;

	tmp[0] = addr;
	memcpy(&tmp[1], buf, len);

	ret = i2c_master_send(this_client, tmp, len + 1);

	if(ret < 0)
	{
		pr_err("yas530 %s: transfer error\n", __func__);
		if (g_magne_i2c_error_alarmed == false) 
		{
			if (len <= 1) 
			{
				info1 |= (0x01 << 2); 
			} 
			else 
			{
				info1 |= (0x02 << 2); 
			}
			pr_err("[T][ARM]Event:0x74 Info:0x%02x%02x%02x%02x\n",
				info1, (-ret) & 0xFF, tmp[0],
				((len > 1) ? tmp[1] : 0));
			g_magne_i2c_error_alarmed = true;
			ret_i2c_write = magne_soft_reset();
		}
		return -EIO;
	} 
	else 
	{
		g_magne_i2c_error_alarmed = false;
	}
#if DEBUG
	YLOGD(("[W] addr[%02x] [%02x]\n", addr, buf[0]));
#endif

	return 0;
}

static int
geomagnetic_i2c_read(uint8_t addr, uint8_t *buf, int len)
{
	struct i2c_msg msg[2];
	unsigned char info1 = (len > 0) ? 0x03 : 0x01;
	
	char rxData0Bak = (buf == NULL) ? 0 : buf[0];
	int err;
	int ret_i2c_read;

	msg[0].addr = this_client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &addr;
	msg[1].addr = this_client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = len;
	msg[1].buf = buf;

	err = i2c_transfer(this_client->adapter, msg, 2);
	if (err != 2) 
	{







		pr_err("yas530 %s: transfer error\n", __func__);
		if (g_magne_i2c_error_alarmed == false)
		{
			info1 |= (0x01 << 2);
			pr_err("[T][ARM]Event:0x74 Info:0x%02x%02x%02x%02x\n",
				info1, (-err) & 0xFF,rxData0Bak, 0);
			g_magne_i2c_error_alarmed = true;
			ret_i2c_read = magne_soft_reset();
		}
		
		return -EIO;
	} 
	else 
	{
		g_magne_i2c_error_alarmed = false;
	}

#if DEBUG
	if (len == 1) {
		YLOGD(("[R] addr[%02x] [%02x]\n", addr, buf[0]));
	} else if (len == 6) {
		YLOGD(("[R] addr[%02x] "
		"[%02x%02x%02x%02x%02x%02x]\n",
		addr, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]));
	} else if (len == 8) {
		YLOGD(("[R] addr[%02x] "
		"[%02x%02x%02x%02x%02x%02x%02x%02x]\n",
		addr, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6],
		buf[7]));
	} else if (len == 9) {
		YLOGD(("[R] addr[%02x] "
		"[%02x%02x%02x%02x%02x%02x%02x%02x%02x]\n",
		addr, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6],
		buf[7], buf[8]));
	} else if (len == 16) {
		YLOGD(("[R] addr[%02x] "
		"[%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
		"%02x]\n",
		addr,
		buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
		buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14],
		buf[15]));
	}
#endif

	return 0;
}

#endif





static int sensor_open(struct inode *inode, struct file *f)
{
	return 0;
}

static int sensor_release(struct inode *inode, struct file *f)
{
	return 0;
}







static void magne_reset(void)
{
    gpio_set_value(yas530_pdata->reset, 0);
    udelay(120);
    gpio_set_value(yas530_pdata->reset, 1);
    udelay(1000);
	
}








static int magne_soft_reset(void)
{
    int ret;
	
	magne_reset();
	
	msleep(100);
	
	ret = magne_init_device();
	if(ret != 0)
	{
		pr_err("%s: Failed magne_init_device!\n", __func__);
		return ret;
	}
	
	ret = magne_diag_set_offset(&g_backup_offset_data);
	if(ret != 0)
	{
		pr_err("%s: Failed magne_diag_set_offset!\n", __func__);
		return ret;
	}
	
	return 0;
	
}








static int magne_diag_set_offset(struct yas530_mag_offset_data	*offset_data)
{
	int ret = 0;
	int i = 0;
	unsigned char addr;
	unsigned char buf[3];
	
	addr = YAS530_OFFSET_START_REG;

	buf[0] = offset_data->x_offset;
	buf[1] = offset_data->y1_offset;
	buf[2] = offset_data->y2_offset;
	
	for(i=0;i<3;i++)
	{	
		ret = geomagnetic_i2c_write(addr+i, &buf[i],1);
	
		if(ret != 0)
		{
			pr_err("%s: Failed magne_diag_set_offset!\n", __func__);
			return ret;
		}
	}
	
	return 0;
}







static int magne_diag_get_offset(struct yas530_mag_offset_data	*offset_data)
{
	int ret = 0;
	int i = 0;
	unsigned char	addr;
	unsigned char	buf[3];
	
	addr = YAS530_OFFSET_START_REG;
	
	for(i=0;i<3;i++)
	{	
		ret = geomagnetic_i2c_read(addr+i, &buf[i],1);
	
		if(ret != 0)
		{
			pr_err("%s: Failed magne_diag_get_offset!\n", __func__);
			return ret;
		}
	}
	
	offset_data->x_offset = buf[0];
	offset_data->y1_offset = buf[1];
	offset_data->y2_offset = buf[2];
	
	return 0;
}








static int magne_diag_get_true_offset(struct yas530_mag_offset_data	*offset_data)
{
	int ret = 0;
	struct yas530_mag_offset_data	context_offset;
	struct yas530_mag_user_data 	context_data;
	unsigned char	flag_x  = OFFSET_X_UNCOMPLETED;
	unsigned char	flag_y1 = OFFSET_Y1_UNCOMPLETED;
	unsigned char	flag_y2 = OFFSET_Y2_UNCOMPLETED;
	signed char		data_x	=0;
	signed char		data_y1 =0;
	signed char		data_y2 =0;
	signed char		calc_bit = 0x10;
	
	
	ret = magne_diag_get_offset(&context_offset);
	if(ret != 0)
	{
		pr_err("%s: Failed magne_diag_get_offset!\n", __func__);
		return ret;
	}
	
	
	ret = magne_diag_set_offset(offset_data);
	if(ret != 0)
	{
		pr_err("%s: Failed magne_diag_set_offset!\n", __func__);
		return ret;
	}
	
	do
	{
		memset(&context_data, 0, sizeof(struct yas530_mag_user_data));
		ret = magne_diag_get(&context_data);
		if(ret != 0)
		{
			pr_err("%s: Failed magne_diag_get!\n", __func__);
			return ret;
		}
		
		if(flag_x == OFFSET_X_UNCOMPLETED)
		{
			if(context_data.x_sensor >= 2049)
			{
				data_x = data_x + calc_bit;
			}
			else if(context_data.x_sensor <= 2047)
			{	
				data_x = data_x - calc_bit;
			}
			else
			{
				
				flag_x = OFFSET_X_COMPLETED;
			}
		}
			
		if(flag_y1 == OFFSET_Y1_UNCOMPLETED)
		{
			if(context_data.y1_sensor >= 2049)
			{
				data_y1 = data_y1 + calc_bit;
			}
			else if(context_data.y1_sensor <= 2047)
			{	
				data_y1 = data_y1 - calc_bit;
			}
			else
			{
				
				flag_y1 = OFFSET_Y1_COMPLETED;
			}
		}
		
		if(flag_y2 == OFFSET_Y2_UNCOMPLETED)
		{
			if(context_data.y2_sensor >= 2049)
			{
				data_y2 = data_y2 + calc_bit;
			}
			else if(context_data.y2_sensor <= 2047)
			{	
				data_y2 = data_y2 - calc_bit;
			}
			else
			{
				
				flag_y2 = OFFSET_Y2_COMPLETED;
			}
		}
		
		offset_data->x_offset  = data_x;
		offset_data->y1_offset = data_y1;
		offset_data->y2_offset = data_y2;

		ret = magne_diag_set_offset(offset_data);
		if(ret != 0)
		{
			pr_err("%s: Failed magne_diag_set_offset!\n", __func__);
			return ret;
		}
		
		calc_bit = calc_bit >> 1;
		
		if(calc_bit == 0)
		{
			break;
		}
	
	}while((flag_x == OFFSET_X_UNCOMPLETED) || (flag_y1 == OFFSET_Y2_UNCOMPLETED) || (flag_y2 == OFFSET_Y2_UNCOMPLETED));


	
	ret = magne_diag_set_offset(&context_offset);
	if(ret != 0)
	{
		pr_err("%s: Failed magne_diag_set_offset!\n", __func__);
		return ret;
	}

	return ret;
}









static int magne_init_device(void)
{
	int ret =0;
	unsigned char buf;
	
	
	buf = 0x00;
	ret = geomagnetic_i2c_write(YAS530_TEST1_REG, &buf, 1);
	
	msleep(100);
	
	if(ret != 0)
	{
		pr_err("%s: magne_write_reg_byte YAS530_TEST1_REG failed\n", __func__);
		return ret;
	}
	
	buf = 0x00;
	ret = geomagnetic_i2c_write(YAS530_TEST2_REG, &buf, 1);
	
	msleep(100);
	
	if(ret != 0)
	{
		pr_err("%s: magne_write_reg_byte YAS530_TEST2_REG failed\n", __func__);
		return ret;
	}
	
	msleep(100);
	
	
	ret = magne_get_cal_regist(&yas_k_data);
	if(ret != 0)
	{
		pr_err("%s: magne_get_cal_regist magne_init_device failed\n", __func__);
		return ret;
	}
	
    
	buf = ((yas_k_data.dck << 2) | 0x00 );
	
	ret = geomagnetic_i2c_write(YAS530_CONFIG_REG, &buf, 1);
	
	msleep(100);
	
	if(ret != 0)
	{
		pr_err("%s: magne_write_reg_byte init YAS530_CONFIG_REG\n", __func__);
		return ret;
	}
	
	
    buf = 0x14;
	ret = geomagnetic_i2c_write(YAS530_ACQUISITION_INTERVAL_REG, &buf ,1);
	
	msleep(100);
	
	if(ret != 0)
	{
		pr_err("%s: magne_write_reg_byte init YAS530_CONFIG_REG\n", __func__);
		return ret;
	}
	
	
    buf = 0x00;
	ret = geomagnetic_i2c_write(YAS530_RESET_COIL_DRIVING_REG, &buf ,1);
	
	msleep(100);
	
	if(ret != 0)
	{
		pr_err("%s: magne_write_reg_byte init YAS530_CONFIG_REG\n", __func__);
		return ret;
	}
	
	return ret;
	
}








static void covert_from_rawdata_to_magdata(unsigned char mag_data1, unsigned char mag_data2, signed short *buf)
{













    *buf = (int16_t)((((int32_t)mag_data1 << 5) & 0xfe0) | ((mag_data2 >> 3) & 0x1f));
}







static void covert_from_rawdata_to_tempdata(unsigned char temp_data1, unsigned char temp_data2, signed short *buf)
{













    *buf = (int16_t)((((int32_t)temp_data1 << 2) & 0x1fc) | ((temp_data2 >> 6) & 0x03));
}








static int magne_diag_get(struct yas530_mag_user_data *diag_data)
{
	int ret =0;	


	unsigned char command;
    signed short t,x,y1,y2;
	uint8_t data[8];


    ret = magne_init_device();
    if (ret)
    {
        pr_err("%s: magne_init_device error!\n", __func__);
		ret = -EFAULT;
		return ret;
    }

	command = YAS530_SELECT_DLYMES_NORMAL;
	ret = geomagnetic_i2c_write(YAS530_ACQUISITION_COMMAND_REG, &command, 1);
	if (ret < 0) 
	{
		pr_err("i2c failed YAS530_ACQUISITION_COMMAND_REG!\n");
		ret = -EFAULT;
		return ret;
	}
	
	msleep(50);








	if (geomagnetic_i2c_read(YAS530_ACQUISITION_TEMP_START_REG, data, 8) < 0)
    {
		pr_err("i2c failed YAS530_ACQUISITION_TEMP_START_REG\n");
		return -EFAULT;
    }

	covert_from_rawdata_to_tempdata(data[0], data[1], &t);
	covert_from_rawdata_to_magdata(data[2], data[3], &x);
	covert_from_rawdata_to_magdata(data[4], data[5], &y1);
	covert_from_rawdata_to_magdata(data[6], data[7], &y2);

    diag_data->temp_data = t;
    diag_data->x_sensor	 = x;
    diag_data->y1_sensor = y1;
    diag_data->y2_sensor = y2;








	
	return ret;
	
}







static int magne_get_cal_regist(struct yas530_data	*k_data)
{
	int ret = 0;
	unsigned char addr = YAS530_CAL_REG;
	unsigned char buf[11];
	int i;
	
	
	for(i=0;i<11;i++)
	{
		ret = geomagnetic_i2c_read(addr+i, &buf[i], 1);
		
		if (ret != 0)
		{
			pr_err("%s: geomagnetic_i2c_read!\n", __func__);
			return ret;
		}
		
	}
	
	msleep(10);
	
	addr = YAS530_CAL_REG;
	
	for(i=0;i<11;i++)
	{
		ret = geomagnetic_i2c_read(addr+i, &buf[i], 1);
		
		if (ret != 0)
		{
			pr_err("%s: geomagnetic_i2c_read!\n", __func__);
			return ret;
		}
		
	}
	
	
	k_data->dx  =buf[0];
	k_data->dy1 =buf[1];
	k_data->dy2 =buf[2];
	
	k_data->d2 = ((buf[3]>>2) & 0x3f);
	k_data->d3 = ((((buf[3]<<2) & 0x0c) | ((buf[4]>>6) & 0x03)));
	k_data->d4 = (buf[4] & 0x3f);
	k_data->d5 = ((buf[5]>>2) & 0x3f);
	k_data->d6 = (((buf[5]<<4) & 0x30) | ((buf[6]>>4) & 0x0f));
	k_data->d7 = (((buf[6]<<3) & 0x78) | ((buf[7]>>5) & 0x07));
	k_data->d8 = (((buf[7]<<1) & 0x3e) | ((buf[8]>>7) & 0x01));
	k_data->d9 = (((buf[8]<<1) & 0xfe) | ((buf[9]>>7) & 0x01));
	k_data->d0 = ((buf[9]>>2) & 0x1f);
	k_data->dck = (((buf[9]<<1) & 0x06) | ((buf[10]>>7) & 0x01));

	msleep(100);
	return ret;
}









static void magne_calc_data(struct yas530_data *k_data, struct yas530_mag_calc_data *calc_data)
{
	signed int dx, dy1,dy2;
	signed int d2, d3, d4, d5;
	signed int d6, d7, d8, d9, d0;
	
	
	dx 	= (signed int)k_data->dx;
	dy1 = (signed int)k_data->dy1;
	dy2 = (signed int)k_data->dy2;
	d2	= (signed int)k_data->d2;
	d3	= (signed int)k_data->d3;
	d4	= (signed int)k_data->d4;
	d5	= (signed int)k_data->d5;
	d6	= (signed int)k_data->d6;
	d7	= (signed int)k_data->d7;
	d8	= (signed int)k_data->d8;
	d9	= (signed int)k_data->d9;
	d0	= (signed int)k_data->d0;
	
	calc_data->m_cx 	= (dx * 6) - 768;
	calc_data->m_cy1 	= (dy1 * 6) - 768;
	calc_data->m_cy2 	= (dy2 * 6) - 768;
	calc_data->m_a2 	= d2 - 32;
	calc_data->m_a3 	= d3 - 8;
	calc_data->m_a4 	= d4 - 32;
	calc_data->m_a5 	= d5 + 38;
	calc_data->m_a6 	= d6 - 32;
	calc_data->m_a7 	= d7 - 64;
	calc_data->m_a8 	= d8 - 32;
	calc_data->m_a9 	= d9;
	calc_data->m_k		= d0 + 10;
}








static int magne_diag_self_test(struct yas530_mag_selftest_data *selftest_data)
{
	int 	ret = 0;
	int 	i = 0, j = 0;
	unsigned char command;
	unsigned char buf[6];



	signed short	x_sens;
	signed short	y1_sens;
	signed short	y2_sens;
	unsigned char addr;
	struct yas530_mag_offset_data		backup_offset, con_offset;

	
	ret = magne_init_device();
	if (ret < 0) 
	{
		pr_err("%s: Failed to magne_init_device !\n", __func__);
		ret = -EFAULT;
		return ret;
	}

	
	ret = magne_diag_get_offset(&backup_offset);
	if(ret != 0)
	{
		pr_err("%s: Failed magne_diag_get_offset!\n", __func__);
		return ret;
	}

	
	ret = magne_diag_get_true_offset(&con_offset);
	if (ret != 0)
	{
		pr_err("%s: Failed to YAS530_IOCTL_DIAG_GET_OFFSET !\n", __func__);
		return ret;
	}

	
	ret = magne_diag_set_offset(&con_offset);
	if(ret != 0)
	{
		pr_err("%s: Failed magne_diag_set_offset!\n", __func__);
		return ret;
	}

	
	
	
	for(j=0;j<2;j++)
	{
		
		command = 0x00;
		
		command = YAS530_SELECT_DLYMES_NORMAL;
		
		if(j==0)
		{
			command |= YAS530_COMMAND_DATA_FORS_1;
		}
		else
		{
			command |= YAS530_COMMAND_DATA_FORS_0;
		}
	
		command |= YAS530_COMMAND_COIL_ON;
		
		ret = geomagnetic_i2c_write(YAS530_ACQUISITION_COMMAND_REG, &command, 1);
	
		if (ret < 0) 
		{
			pr_err("%s: Failed to geomagnetic_i2c_write from start_addr_1 !\n", __func__);
			ret = -EFAULT;
			return ret;
		}
	
		msleep(10);
		
		addr = YAS530_ACQUISITION_MAG_START_REG;
	
		for(i=0;i<6;i++)
		{
			ret = geomagnetic_i2c_read(addr+i, &buf[i], 1);
		
			if (ret < 0) 
			{
				pr_err("%s: Failed to magne_read_reg_byte !\n", __func__);
				ret = -EFAULT;
				return ret;
			}
		
			msleep(20);

		}
	
		covert_from_rawdata_to_magdata(buf[0], buf[1], &x_sens);
		covert_from_rawdata_to_magdata(buf[2], buf[3], &y1_sens);
		covert_from_rawdata_to_magdata(buf[4], buf[5], &y2_sens);
	
		if(j==0)
		{
			selftest_data->xm_sensor = x_sens;
			



			selftest_data->y1m_sensor = y1_sens;
			selftest_data->y2m_sensor = y2_sens;
		}
		else
		{
			selftest_data->xp_sensor = x_sens;
			



			selftest_data->y1p_sensor = y1_sens;
			selftest_data->y2p_sensor = y2_sens;
		}
	}
	
	
	ret = magne_diag_set_offset(&backup_offset);
	if(ret != 0)
	{
		pr_err("%s: Failed magne_diag_set_offset!\n", __func__);
		return ret;
	}

	return ret;
	
}







static int magne_diag_regist_get(unsigned char addr, unsigned char *data)
{
	int ret;
	unsigned char buff = 0;

	ret = geomagnetic_i2c_read(addr, &buff, 1);
	
	if(ret != 0)
	{
		pr_err("%s: resd_reg failed!\n", __func__);
		return ret;
	}
	
	*data = buff;
	return 0;
}







static long magne_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

	int ret = 0x00;


	void __user *argp = (void __user *)arg;
	struct yas530_mag_test_reg			test_reg;
	struct yas530_mag_regist_data		regist_data;
	unsigned char address;
	unsigned char *point = NULL;
	int i = 0;
	unsigned char buffer;
	struct yas530_mag_offset_data		offset_data;
	struct yas530_mag_user_data			user_data;
	static struct yas530_mag_calc_data		yas_cal_data;
	struct yas530_mag_selftest_data		selftest_data;

	
	mutex_lock(&g_magne_mutex);









	switch (cmd) 
	{
		case YAS530_IOCTL_MEASURE_START:
		{
			;
		}
		break;
		
		case YAS530_IOCTL_MEASURE_STOP:
		{
			;
		}
		break;
		
		case YAS530_IOCTL_DIAG_REG_INIT:
		{
			memset(&regist_data, 0, sizeof(struct yas530_mag_regist_data));
			
			point = &(regist_data.regist_data80);
			address = YAS530_READ_REG_START_1;
			
			for(i=0;i<5;i++)
			{
				if(address == 0x81)
				{
					address = 0x84;
				}

				ret = geomagnetic_i2c_read(address, &buffer,1);

				if (ret != 0) 
				{
					pr_err("%s: Failed to magne_read_reg_byte from start_addr_1 !\n", __func__);
					ret = -EFAULT;
					goto ioctl_exit;
				}
			
				*point = buffer;
				
				point++;
				address++;
				buffer = 0;
			}
			
			point = &(regist_data.regist_dataB0);
			address = YAS530_READ_REG_START_2;
			
			for(i=0;i<8;i++)
			{
				ret = geomagnetic_i2c_read(address+i, &buffer,1);
		
				if (ret != 0) 
				{
					pr_err("%s: Failed to magne_read_reg_byte from start_addr_2!\n", __func__);
					ret = -EFAULT;
					goto ioctl_exit;
				}
			
				*point = buffer;
				
				point++;
				buffer = 0;
			}

			ret = copy_to_user(argp, &regist_data, sizeof(struct yas530_mag_regist_data));
		
			if (ret != 0) 
			{
				pr_err("%s: Failed to copy data to user  !\n", __func__);
				goto ioctl_exit;
				ret = -EFAULT;
			}
		}
		break;
		case YAS530_IOCTL_DIAG_RESET:
		{
			magne_reset();
			msleep(100);
			ret = 0;
		}
		break;

		case YAS530_IOCTL_DIAG_REG_READ:
		{
			ret = copy_from_user(&test_reg, argp, sizeof(struct yas530_mag_test_reg));			
			if (ret != 0) 
			{
				pr_err("%s: Failed to copy_from_user. YAS530_IOCTL_DIAG_TEST_REG!\n", __func__);
				ret = -EFAULT;
				goto ioctl_exit;
			}

			if (geomagnetic_i2c_read(test_reg.test_reg_addr, &test_reg.test_reg_data, 1) < 0)
			{
				pr_err("%s: I2C read failed!\n", __func__);
				ret = -EFAULT;
				goto ioctl_exit;
			}

			ret = copy_to_user(argp, &test_reg, sizeof(struct yas530_mag_test_reg));			
			if (ret != 0) 
			{
				pr_err("%s: Failed to copy_to_user. YAS530_IOCTL_DIAG_TEST_REG!\n", __func__);
				ret = -EFAULT;
				goto ioctl_exit;
			}


		}
		break;


		case YAS530_IOCTL_DIAG_REG_WRITE:
		{
			memset(&regist_data, 0, sizeof(struct yas530_mag_regist_data));
			
			ret = copy_from_user(&regist_data, argp, sizeof(struct yas530_mag_regist_data));
			
			if (ret != 0) 
			{
				pr_err("%s: Failed to copy data from user  !\n", __func__);
				ret = -EFAULT;
				goto ioctl_exit;
			}
			
			point = &(regist_data.regist_data80);
			address = YAS530_READ_REG_START_1;
			
			for(i=0;i<5;i++)
			{
				if(address == 0x81)
				{
					address = 0x84;
				}
				
				ret = geomagnetic_i2c_write(address, point+i,1);
		
				if (ret != 0) 
				{
					pr_err("%s: Failed to magne_read_reg_byte from start_addr_1 !\n", __func__);
					ret = -EFAULT;
					goto ioctl_exit;
				}
				
				address++;
				
			}
			
			point = &(regist_data.regist_dataB0);
			address = YAS530_READ_REG_START_2;
			
			for(i=0;i<8;i++)
			{
				ret = geomagnetic_i2c_write(address+i, point+i,1);
		
				if (ret != 0) 
				{
					pr_err("%s: Failed to magne_read_reg_byte from start_addr_2!\n", __func__);
					ret = -EFAULT;
					goto ioctl_exit;
				}
				
			}
			
		}
		break;

		case YAS530_IOCTL_DIAG_CALC_DATA:
		{
			memset(&yas_cal_data, 0, sizeof(struct yas530_mag_calc_data));

			ret = magne_get_cal_regist(&yas_k_data);
	
			if(ret != 0)
			{
				pr_err("%s: magne_get_cal_regist failed init\n", __func__);
				ret = -EFAULT;
				goto ioctl_exit;
			}

			magne_calc_data(&yas_k_data, &yas_cal_data);


			ret = copy_to_user(argp, &yas_cal_data, sizeof(struct yas530_mag_calc_data));
		
			if (ret != 0) 
			{
				pr_err("%s: Failed to copy data to user YAS530_IOCTL_DIAG_CALC_DATA !\n", __func__);
				ret = -EFAULT;
				goto ioctl_exit;
			}
			
		}
		break;


		case YAS530_IOCTL_DIAG_GET_MAGNE:
		{
			
			memset(&user_data, 0, sizeof(struct yas530_mag_user_data));
			
			ret = magne_diag_get(&user_data);
			
			if (ret != 0) 
			{
				pr_err("%s: Failed magne_diag_get !\n", __func__);
				ret = -EFAULT;
				goto ioctl_exit;
			}
			
			ret = copy_to_user(argp, &user_data, sizeof(struct yas530_mag_user_data));
			
			if (ret != 0) 
			{
				pr_err("%s: Failed to copy_to_user. YAS530_IOCTL_DIAG_GET_MAGNE !\n", __func__);
				ret = -EFAULT;
				goto ioctl_exit;
			}
			
		}
		break;

		case YAS530_IOCTL_DIAG_SELF_TEST:
		{
			memset(&selftest_data, 0, sizeof(struct yas530_mag_selftest_data));
			
			ret = magne_diag_self_test(&selftest_data);
			
			if (ret != 0) 
			{
				pr_err("%s: Failed to magne_read_reg_byte from start_addr_1 !\n", __func__);
				ret = -EFAULT;
				goto ioctl_exit;
			}
			
			ret = copy_to_user(argp, &selftest_data, sizeof(struct yas530_mag_selftest_data));
			
			if (ret != 0) 
			{
				pr_err("%s: Failed to copy_to_user. YAS530_IOCTL_DIAG_SELF_TEST !\n", __func__);
				ret = -EFAULT;
				goto ioctl_exit;
			}
		}
		break;
		case YAS530_IOCTL_DIAG_GET_OFFSET:
		{

			memset(&offset_data, 0, sizeof(struct yas530_mag_offset_data));
			
			ret = magne_diag_get_true_offset(&offset_data);
			
			if (ret != 0) 
			{
				pr_err("%s: Failed to YAS530_IOCTL_DIAG_GET_OFFSET !\n", __func__);
				ret = -EFAULT;
				goto ioctl_exit;
			}
			
			ret = copy_to_user(argp, &offset_data, sizeof(struct yas530_mag_offset_data));
			
			if (ret != 0) 
			{
				pr_err("%s: Failed to copy_to_user. YAS530_IOCTL_DIAG_GET_OFFSET!\n", __func__);
				ret = -EFAULT;
				goto ioctl_exit;
			}
		}
		break;
		case YAS530_IOCTL_DIAG_TEST_REG:
		{
			memset(&test_reg, 0, sizeof(struct yas530_mag_test_reg));
			
			ret = copy_from_user(&test_reg, argp, sizeof(struct yas530_mag_test_reg));
			
			if (ret != 0) 
			{
				pr_err("%s: Failed to copy_from_user. YAS530_IOCTL_DIAG_TEST_REG!\n", __func__);
				ret = -EFAULT;
				goto ioctl_exit;
			}
			
			
			if((test_reg.test_reg_addr >= 0x90) && (test_reg.test_reg_addr <= 0x97))
			{
				ret = magne_diag_regist_get(test_reg.test_reg_addr, &(test_reg.test_reg_data));
				ret = 0;
			}
			
			ret = magne_diag_regist_get(test_reg.test_reg_addr, &(test_reg.test_reg_data));
			
			if (ret != 0) 
			{
				pr_err("%s: Failed to magne_diag_regist_get. YAS530_IOCTL_DIAG_TEST_REG!\n", __func__);
				ret = -EFAULT;
				goto ioctl_exit;
			}
			
			ret = copy_to_user(argp, &test_reg, sizeof(struct yas530_mag_test_reg));
			
			if (ret != 0) 
			{
				pr_err("%s: Failed to copy_to_user. YAS530_IOCTL_DIAG_TEST_REG!\n", __func__);
				ret = -EFAULT;
				goto ioctl_exit;
			}
			
		}
		break;
		
		case YAS530_IOCTL_SET_OFFSET_NV:
		{
		

			g_magne_is_nv_set = 0;
			
			ret = copy_from_user(&g_mag_nv_data, (void *)arg, sizeof(struct yas530_mag_nv_data));
			
			if (ret != 0) 
			{
				pr_err("%s: Failed to copy data from user!\n", __func__);
				mutex_unlock(&g_magne_mutex);
				return -EFAULT;
			}
			
			offset_data.x_offset 	= g_mag_nv_data.nv_reg[0];
			offset_data.y1_offset 	= g_mag_nv_data.nv_reg[1];
			offset_data.y2_offset	= g_mag_nv_data.nv_reg[2];
			
			
			g_backup_offset_data.x_offset  = offset_data.x_offset;
			g_backup_offset_data.y1_offset = offset_data.y1_offset;
			g_backup_offset_data.y2_offset = offset_data.y2_offset;
			
			ret = magne_diag_set_offset(&offset_data);
		
			g_magne_is_nv_set = 1;


			ret = 0;
		}
		break;

		case YAS530_IOCTL_GET_OFFSET_NV:
		{
			if(g_magne_is_nv_set==1)
			{
				ret = magne_diag_get_offset(&offset_data);
				if(ret != 0)
				{
					pr_err("%s: Failed magne_diag_get_offset!\n", __func__);
					return ret;
				}

				g_mag_nv_data.nv_reg[0] = offset_data.x_offset;
				g_mag_nv_data.nv_reg[1] = offset_data.y1_offset;
				g_mag_nv_data.nv_reg[2] = offset_data.y2_offset;

				ret = copy_to_user((void *)argp, &g_mag_nv_data, sizeof(struct yas530_mag_nv_data));
				if (ret != 0)
				{
					pr_err("%s: Failed to copy data to user!\n", __func__);
					ret = -EFAULT;
					goto ioctl_exit;
				}
				
				ret = 0;
			}
			else
			{
				pr_err("%s: Not Set NV DATA!\n", __func__);
				goto ioctl_exit;
				ret = -EFAULT;
			}
		}
		break;

		default:
			pr_err("%s: invalid ioctl command %d\n", __func__, cmd);
			ret = -EFAULT;
			goto ioctl_exit;
	}

ioctl_exit:
	mutex_unlock(&g_magne_mutex);

	return (long)ret;
}

static const struct file_operations sensor_fops_diag = {
	.owner = THIS_MODULE,
	.open = sensor_open,
	.release = sensor_release,
	.unlocked_ioctl = magne_ioctl,
};

static struct miscdevice sensor_diag = {
	.name = "treaxis_magnetic_sensor",
	.fops = &sensor_fops_diag,
	.minor = MISC_DYNAMIC_MINOR,
};
static int
geomagnetic_lock(void)
{
	struct geomagnetic_data *data = NULL;
	int rt;

	if (this_client == NULL)
		return -1;

	data = i2c_get_clientdata(this_client);
	rt = down_interruptible(&data->driver_lock);
	if (rt < 0)
		up(&data->driver_lock);
	return rt;
}

static int
geomagnetic_unlock(void)
{
	struct geomagnetic_data *data = NULL;

	if (this_client == NULL)
		return -1;

	data = i2c_get_clientdata(this_client);
	up(&data->driver_lock);
	return 0;
}

static void
geomagnetic_msleep(int ms)
{
	msleep(ms);
}

static void
geomagnetic_current_time(int32_t *sec, int32_t *msec)
{
	struct timeval tv;

	do_gettimeofday(&tv);

	*sec = tv.tv_sec;
	*msec = tv.tv_usec / 1000;
}

static struct yas_mag_driver hwdriver = {
	.callback = {
		.lock		= geomagnetic_lock,
		.unlock		= geomagnetic_unlock,
		.device_open	= geomagnetic_i2c_open,
		.device_close	= geomagnetic_i2c_close,
		.device_read	= geomagnetic_i2c_read,
		.device_write	= geomagnetic_i2c_write,
		.msleep		= geomagnetic_msleep,
		.current_time	= geomagnetic_current_time,
	},
};

static int
geomagnetic_multi_lock(void)
{
	struct geomagnetic_data *data = NULL;
	int rt;

	if (this_client == NULL)
		return -1;

	data = i2c_get_clientdata(this_client);
	rt = down_interruptible(&data->multi_lock);
	if (rt < 0)
		up(&data->multi_lock);
	return rt;
}

static int
geomagnetic_multi_unlock(void)
{
	struct geomagnetic_data *data = NULL;

	if (this_client == NULL)
		return -1;

	data = i2c_get_clientdata(this_client);
	up(&data->multi_lock);
	return 0;
}

static int
geomagnetic_enable(struct geomagnetic_data *data)
{
	if (!atomic_cmpxchg(&data->enable, 0, 1))
		schedule_delayed_work(&data->work, 0);

	return 0;
}

static int
geomagnetic_disable(struct geomagnetic_data *data)
{
	if (atomic_cmpxchg(&data->enable, 1, 0))
		cancel_delayed_work_sync(&data->work);

	return 0;
}


static ssize_t
geomagnetic_delay_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_data);
	int delay;

	geomagnetic_multi_lock();

	delay = data->delay;

	geomagnetic_multi_unlock();

	return sprintf(buf, "%d\n", delay);
}

static ssize_t
geomagnetic_delay_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_data);
	long value;

	if (hwdriver.set_delay == NULL)
		return -ENOTTY;
	if (strict_strtol(buf, 10, &value) < 0)
		return -EINVAL;

	geomagnetic_multi_lock();

	if (hwdriver.set_delay(value) == 0)
		data->delay = value;

	geomagnetic_multi_unlock();

	return count;
}

static ssize_t
geomagnetic_enable_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_data);

	return sprintf(buf, "%d\n", atomic_read(&data->enable));
}

static ssize_t
geomagnetic_enable_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_data);
	long value;

	if (hwdriver.set_enable == NULL)
		return -ENOTTY;
	if (strict_strtol(buf, 10, &value) < 0)
		return -EINVAL;
	value = !!value;

	if (geomagnetic_multi_lock() < 0)
		return count;
	if (hwdriver.set_enable(value) == 0) {
		if (value)
			geomagnetic_enable(data);
		else
			geomagnetic_disable(data);
	}

	geomagnetic_multi_unlock();

	return count;
}

static ssize_t
geomagnetic_filter_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_data);
	int filter_enable;

	geomagnetic_multi_lock();

	filter_enable = data->filter_enable;

	geomagnetic_multi_unlock();

	return sprintf(buf, "%d\n", filter_enable);
}

static ssize_t
geomagnetic_filter_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_data);
	long value;

	if (hwdriver.set_filter_enable == NULL)
		return -ENOTTY;
	if (strict_strtol(buf, 10, &value) < 0)
		return -EINVAL;

	if (geomagnetic_multi_lock() < 0)
		return count;

	if (hwdriver.set_filter_enable(value) == 0)
		data->filter_enable = !!value;

	geomagnetic_multi_unlock();

	return count;
}

static ssize_t
geomagnetic_filter_len_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_data);
	int filter_len;

	geomagnetic_multi_lock();

	filter_len = data->filter_len;

	geomagnetic_multi_unlock();

	return sprintf(buf, "%d\n", filter_len);
}

static ssize_t
geomagnetic_filter_len_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_data);
	struct yas_mag_filter filter;
	long value;

	if (hwdriver.get_filter == NULL || hwdriver.set_filter == NULL)
		return -ENOTTY;
	if (strict_strtol(buf, 10, &value) < 0)
		return -EINVAL;

	if (geomagnetic_multi_lock() < 0)
		return count;

	hwdriver.get_filter(&filter);
	filter.len = value;
	if (hwdriver.set_filter(&filter) == 0)
		data->filter_len = value;

	geomagnetic_multi_unlock();

	return count;
}

static ssize_t
geomagnetic_filter_noise_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct input_dev *input_raw = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_raw);
	int rt;

	geomagnetic_multi_lock();

	rt = sprintf(buf, "%d %d %d\n", data->filter_noise[0],
			data->filter_noise[1], data->filter_noise[2]);

	geomagnetic_multi_unlock();

	return rt;
}

static ssize_t
geomagnetic_filter_noise_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct input_dev *input_raw = to_input_dev(dev);
	struct yas_mag_filter filter;
	struct geomagnetic_data *data = input_get_drvdata(input_raw);
	int32_t noise[3];

	geomagnetic_multi_lock();

	sscanf(buf, "%d %d %d", &noise[0], &noise[1], &noise[2]);
	hwdriver.get_filter(&filter);
	memcpy(filter.noise, noise, sizeof(filter.noise));
	if (hwdriver.set_filter(&filter) == 0)
		memcpy(data->filter_noise, noise, sizeof(data->filter_noise));

	geomagnetic_multi_unlock();

	return count;
}

static ssize_t
geomagnetic_filter_threshold_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_data);
	int32_t filter_threshold;

	geomagnetic_multi_lock();

	filter_threshold = data->filter_threshold;

	geomagnetic_multi_unlock();

	return sprintf(buf, "%d\n", filter_threshold);
}

static ssize_t
geomagnetic_filter_threshold_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_data);
	struct yas_mag_filter filter;
	long value;

	if (hwdriver.get_filter == NULL || hwdriver.set_filter == NULL)
		return -ENOTTY;
	if (strict_strtol(buf, 10, &value) < 0)
		return -EINVAL;

	if (geomagnetic_multi_lock() < 0)
		return count;

	hwdriver.get_filter(&filter);
	filter.threshold = value;
	if (hwdriver.set_filter(&filter) == 0)
		data->filter_threshold = value;

	geomagnetic_multi_unlock();

	return count;
}

static ssize_t
geomagnetic_position_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	if (hwdriver.get_position == NULL)
		return -ENOTTY;
	return sprintf(buf, "%d\n", hwdriver.get_position());
}

static ssize_t
geomagnetic_position_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	long value;

	if (hwdriver.set_position == NULL)
		return -ENOTTY;
	if (strict_strtol(buf, 10, &value) < 0)
		return -EINVAL;

	hwdriver.set_position(value);

	return count;
}

static ssize_t
geomagnetic_data_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_data);
	int rt;

	rt = sprintf(buf, "%d %d %d\n", atomic_read(&data->last_data[0]),
			atomic_read(&data->last_data[1]),
			atomic_read(&data->last_data[2]));

	return rt;
}

static ssize_t
geomagnetic_status_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_data);
	int rt;

	rt = sprintf(buf, "%d\n", atomic_read(&data->last_status));

	return rt;
}

static ssize_t
geomagnetic_wake_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_data);
	static int16_t cnt = 1;





	input_report_abs(data->input_data, ABS_WAKE, cnt++);
	input_sync(data->input_data);


	return count;
}

#if DEBUG

static int geomagnetic_suspend(struct i2c_client *client, pm_message_t mesg);
static int geomagnetic_resume(struct i2c_client *client);

static ssize_t
geomagnetic_debug_suspend_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input);

	return sprintf(buf, "%d\n", data->suspend);
}

static ssize_t
geomagnetic_debug_suspend_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long suspend;

	if (strict_strtol(buf, 10, &suspend) < 0)
		return -EINVAL;
	if (suspend) {
		pm_message_t msg;
		memset(&msg, 0, sizeof(msg));
		geomagnetic_suspend(this_client, msg);
	} else {
		geomagnetic_resume(this_client);
	}

	return count;
}

#endif 

static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP,
	geomagnetic_delay_show, geomagnetic_delay_store);
static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP,
	geomagnetic_enable_show, geomagnetic_enable_store);
static DEVICE_ATTR(filter_enable, S_IRUGO|S_IWUSR|S_IWGRP,
	geomagnetic_filter_enable_show,
	geomagnetic_filter_enable_store);
static DEVICE_ATTR(filter_len, S_IRUGO|S_IWUSR|S_IWGRP,
	geomagnetic_filter_len_show, geomagnetic_filter_len_store);
static DEVICE_ATTR(filter_threshold, S_IRUGO|S_IWUSR|S_IWGRP,
	geomagnetic_filter_threshold_show,
	geomagnetic_filter_threshold_store);
static DEVICE_ATTR(filter_noise, S_IRUGO|S_IWUSR|S_IWGRP,
	geomagnetic_filter_noise_show, geomagnetic_filter_noise_store);
static DEVICE_ATTR(data, S_IRUGO, geomagnetic_data_show, NULL);
static DEVICE_ATTR(status, S_IRUGO, geomagnetic_status_show, NULL);
static DEVICE_ATTR(wake, S_IWUSR|S_IWGRP, NULL, geomagnetic_wake_store);
static DEVICE_ATTR(position, S_IRUGO|S_IWUSR,
	geomagnetic_position_show, geomagnetic_position_store);
#if DEBUG
static DEVICE_ATTR(debug_suspend, S_IRUGO|S_IWUSR,
	geomagnetic_debug_suspend_show, geomagnetic_debug_suspend_store);
#endif 

static struct attribute *geomagnetic_attributes[] = {
	&dev_attr_delay.attr,
	&dev_attr_enable.attr,
	&dev_attr_filter_enable.attr,
	&dev_attr_filter_len.attr,
	&dev_attr_filter_threshold.attr,
	&dev_attr_filter_noise.attr,
	&dev_attr_data.attr,
	&dev_attr_status.attr,
	&dev_attr_wake.attr,
	&dev_attr_position.attr,
#if DEBUG
	&dev_attr_debug_suspend.attr,
#endif 
	NULL
};

static struct attribute_group geomagnetic_attribute_group = {
	.attrs = geomagnetic_attributes
};

static ssize_t
geomagnetic_raw_threshold_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct input_dev *input_raw = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_raw);
	int threshold;

	geomagnetic_multi_lock();

	threshold = data->threshold;

	geomagnetic_multi_unlock();

	return sprintf(buf, "%d\n", threshold);
}

static ssize_t
geomagnetic_raw_threshold_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct input_dev *input_raw = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_raw);
	long value;

	if (strict_strtol(buf, 10, &value) < 0)
		return -EINVAL;

	geomagnetic_multi_lock();

	if (0 <= value && value <= 2) {





		data->threshold = value;

		input_report_abs(data->input_raw, ABS_RAW_THRESHOLD, value);
		input_sync(data->input_raw);

	}

	geomagnetic_multi_unlock();

	return count;
}

static ssize_t
geomagnetic_raw_distortion_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct input_dev *input_raw = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_raw);
	int rt;

	geomagnetic_multi_lock();

	rt = sprintf(buf, "%d %d %d\n", data->distortion[0],
			data->distortion[1], data->distortion[2]);

	geomagnetic_multi_unlock();

	return rt;
}

static ssize_t
geomagnetic_raw_distortion_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct input_dev *input_raw = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_raw);
	int32_t distortion[3];
	static int32_t val = 1;
	int i;

	geomagnetic_multi_lock();

	sscanf(buf, "%d %d %d", &distortion[0], &distortion[1], &distortion[2]);
	if (distortion[0] > 0 && distortion[1] > 0 && distortion[2] > 0) {





		for (i = 0; i < 3; i++)
			data->distortion[i] = distortion[i];

		input_report_abs(data->input_raw, ABS_RAW_DISTORTION, val++);
		input_sync(data->input_raw);

	}

	geomagnetic_multi_unlock();

	return count;
}

static ssize_t
geomagnetic_raw_shape_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct input_dev *input_raw = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_raw);
	int shape;

	geomagnetic_multi_lock();

	shape = data->shape;

	geomagnetic_multi_unlock();

	return sprintf(buf, "%d\n", shape);
}

static ssize_t
geomagnetic_raw_shape_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input_raw = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_raw);
	long value;

	if (strict_strtol(buf, 10, &value) < 0)
		return -EINVAL;

	geomagnetic_multi_lock();

	if (0 <= value && value <= 1) {





		data->shape = value;

		input_report_abs(data->input_raw, ABS_RAW_SHAPE, value);
		input_sync(data->input_raw);

	}

	geomagnetic_multi_unlock();

	return count;
}

static ssize_t
geomagnetic_raw_offsets_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct input_dev *input_raw = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_raw);
	struct yas_mag_offset offset;
	int accuracy;

	geomagnetic_multi_lock();

	offset = data->driver_offset;
	accuracy = atomic_read(&data->last_status);

	geomagnetic_multi_unlock();

	return sprintf(buf, "%d %d %d %d %d %d %d\n", offset.hard_offset[0],
			offset.hard_offset[1], offset.hard_offset[2],
			offset.calib_offset.v[0], offset.calib_offset.v[1],
			offset.calib_offset.v[2], accuracy);
}

static ssize_t
geomagnetic_raw_offsets_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct input_dev *input_raw = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_raw);
	struct yas_mag_offset offset;
	int32_t hard_offset[3];
	int i, accuracy;

	geomagnetic_multi_lock();

	sscanf(buf, "%d %d %d %d %d %d %d", &hard_offset[0], &hard_offset[1],
			&hard_offset[2], &offset.calib_offset.v[0],
			&offset.calib_offset.v[1], &offset.calib_offset.v[2],
			&accuracy);
	if (0 <= accuracy && accuracy <= 3) {
		for (i = 0; i < 3; i++)
			offset.hard_offset[i] = (int8_t)hard_offset[i];
		if (hwdriver.set_offset(&offset) == 0) {
			atomic_set(&data->last_status, accuracy);
			data->driver_offset = offset;
		}
	}

	geomagnetic_multi_unlock();

	return count;
}








































static ssize_t
geomagnetic_raw_static_matrix_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct input_dev *input_raw = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_raw);
	struct yas_matrix matrix;

	geomagnetic_multi_lock();

	matrix = data->static_matrix;

	geomagnetic_multi_unlock();

	return sprintf(buf, "%d %d %d %d %d %d %d %d %d\n", matrix.matrix[0],
			matrix.matrix[1], matrix.matrix[2], matrix.matrix[3],
			matrix.matrix[4], matrix.matrix[5], matrix.matrix[6],
			matrix.matrix[7], matrix.matrix[8]);
}

static ssize_t
geomagnetic_raw_static_matrix_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct input_dev *input_raw = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_raw);
	struct yas_matrix matrix;

	geomagnetic_multi_lock();

	sscanf(buf, "%d %d %d %d %d %d %d %d %d", &matrix.matrix[0],
			&matrix.matrix[1], &matrix.matrix[2],
			&matrix.matrix[3], &matrix.matrix[4], &matrix.matrix[5],
			&matrix.matrix[6], &matrix.matrix[7],
			&matrix.matrix[8]);
	if (hwdriver.set_static_matrix(&matrix) == 0)
		data->static_matrix = matrix;

	geomagnetic_multi_unlock();

	return count;
}

static ssize_t
geomagnetic_raw_dynamic_matrix_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct input_dev *input_raw = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_raw);
	struct yas_matrix matrix;

	geomagnetic_multi_lock();

	matrix = data->dynamic_matrix;

	geomagnetic_multi_unlock();

	return sprintf(buf, "%d %d %d %d %d %d %d %d %d\n", matrix.matrix[0],
			matrix.matrix[1], matrix.matrix[2], matrix.matrix[3],
			matrix.matrix[4], matrix.matrix[5], matrix.matrix[6],
			matrix.matrix[7], matrix.matrix[8]);
}

static ssize_t
geomagnetic_raw_dynamic_matrix_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct input_dev *input_raw = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_raw);
	struct yas_matrix matrix;

	geomagnetic_multi_lock();

	sscanf(buf, "%d %d %d %d %d %d %d %d %d", &matrix.matrix[0],
			&matrix.matrix[1], &matrix.matrix[2],
			&matrix.matrix[3], &matrix.matrix[4], &matrix.matrix[5],
			&matrix.matrix[6], &matrix.matrix[7],
			&matrix.matrix[8]);
	if (hwdriver.set_dynamic_matrix(&matrix) == 0)
		data->dynamic_matrix = matrix;

	geomagnetic_multi_unlock();

	return count;
}

static ssize_t
geomagnetic_raw_ellipsoid_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct input_dev *input_raw = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_raw);
	int ellipsoid_mode;

	geomagnetic_multi_lock();

	ellipsoid_mode = data->ellipsoid_mode;

	geomagnetic_multi_unlock();

	return sprintf(buf, "%d\n", ellipsoid_mode);
}

static ssize_t
geomagnetic_raw_ellipsoid_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct input_dev *input_raw = to_input_dev(dev);
	struct geomagnetic_data *data = input_get_drvdata(input_raw);
	long value;

	if (strict_strtol(buf, 10, &value) < 0)
		return -EINVAL;
	value = !!value;





	geomagnetic_multi_lock();





	data->ellipsoid_mode = value;

	input_report_abs(data->input_raw, ABS_RAW_MODE, value);
	input_sync(data->input_raw);


	geomagnetic_multi_unlock();

	return count;
}

static DEVICE_ATTR(threshold, S_IRUGO|S_IWUSR,
	geomagnetic_raw_threshold_show, geomagnetic_raw_threshold_store);
static DEVICE_ATTR(distortion, S_IRUGO|S_IWUSR,
	geomagnetic_raw_distortion_show, geomagnetic_raw_distortion_store);
static DEVICE_ATTR(shape, S_IRUGO|S_IWUSR,
	geomagnetic_raw_shape_show, geomagnetic_raw_shape_store);
static DEVICE_ATTR(offsets, S_IRUGO|S_IWUSR,
	geomagnetic_raw_offsets_show, geomagnetic_raw_offsets_store);





static DEVICE_ATTR(static_matrix, S_IRUGO|S_IWUSR,
	geomagnetic_raw_static_matrix_show,
	geomagnetic_raw_static_matrix_store);
static DEVICE_ATTR(dynamic_matrix, S_IRUGO|S_IWUSR,
	geomagnetic_raw_dynamic_matrix_show,
	geomagnetic_raw_dynamic_matrix_store);
static DEVICE_ATTR(ellipsoid_mode, S_IRUGO|S_IWUSR,
	geomagnetic_raw_ellipsoid_mode_show,
	geomagnetic_raw_ellipsoid_mode_store);

static struct attribute *geomagnetic_raw_attributes[] = {
	&dev_attr_threshold.attr,
	&dev_attr_distortion.attr,
	&dev_attr_shape.attr,
	&dev_attr_offsets.attr,



	&dev_attr_static_matrix.attr,
	&dev_attr_dynamic_matrix.attr,
	&dev_attr_ellipsoid_mode.attr,
	NULL
};

static struct attribute_group geomagnetic_raw_attribute_group = {
	.attrs = geomagnetic_raw_attributes
};



static int
geomagnetic_work(struct yas_mag_data *magdata)
{
	struct geomagnetic_data *data = i2c_get_clientdata(this_client);
	uint32_t time_delay_ms = 100;
	static int cnt;
	int rt, i, accuracy;





	if (hwdriver.measure == NULL || hwdriver.get_offset == NULL)
		return time_delay_ms;

	rt = hwdriver.measure(magdata, &time_delay_ms);
	if (rt < 0) {
		YLOGE(("measure failed[%d]\n", rt));
		return 100;
	}
	YLOGD(("xy1y2 [%d][%d][%d] raw[%d][%d][%d]\n",
				magdata->xy1y2.v[0], magdata->xy1y2.v[1],
				magdata->xy1y2.v[2], magdata->xyz.v[0],
				magdata->xyz.v[1], magdata->xyz.v[2]));

	accuracy = atomic_read(&data->last_status);

	if ((rt & YAS_REPORT_OVERFLOW_OCCURED)
			|| (rt & YAS_REPORT_HARD_OFFSET_CHANGED)
			|| (rt & YAS_REPORT_CALIB_OFFSET_CHANGED)) {
		static uint16_t count = 1;
		int code = 0;
		int value = 0;

		hwdriver.get_offset(&data->driver_offset);
		if (rt & YAS_REPORT_OVERFLOW_OCCURED) {
			atomic_set(&data->last_status, 0);
			accuracy = 0;
		}

		
		code |= (rt & YAS_REPORT_OVERFLOW_OCCURED);
		code |= (rt & YAS_REPORT_HARD_OFFSET_CHANGED);
		code |= (rt & YAS_REPORT_CALIB_OFFSET_CHANGED);
		value = (count++ << 16) | (code);




		input_report_abs(data->input_raw, ABS_RAW_REPORT, value);
		input_sync(data->input_raw);

	}

	if (rt & YAS_REPORT_DATA) {









		
		input_report_abs(data->input_data, ABS_X, magdata->xyz.v[0]);
		input_report_abs(data->input_data, ABS_Y, magdata->xyz.v[1]);
		input_report_abs(data->input_data, ABS_Z, magdata->xyz.v[2]);
		if (atomic_read(&data->last_data[0]) == magdata->xyz.v[0]
				&& atomic_read(&data->last_data[1])
				== magdata->xyz.v[1]
				&& atomic_read(&data->last_data[2])
				== magdata->xyz.v[2]) {
			input_report_abs(data->input_data, ABS_RUDDER, cnt++);
		}
		input_report_abs(data->input_data, ABS_STATUS, accuracy);
		input_sync(data->input_data);


		for (i = 0; i < 3; i++)
			atomic_set(&data->last_data[i], magdata->xyz.v[i]);
	}

	if (rt & YAS_REPORT_CALIB) {








		
		input_report_abs(data->input_raw, ABS_X, magdata->raw.v[0]);
		input_report_abs(data->input_raw, ABS_Y, magdata->raw.v[1]);
		input_report_abs(data->input_raw, ABS_Z, magdata->raw.v[2]);
		input_sync(data->input_raw);

	}

	return time_delay_ms;

}

static void
geomagnetic_input_work_func(struct work_struct *work)
{
	struct geomagnetic_data *data
		= container_of((struct delayed_work *)work,
			struct geomagnetic_data, work);
	uint32_t delay;
	struct yas_mag_data magdata;

	delay = geomagnetic_work(&magdata);







	schedule_delayed_work(&data->work, msecs_to_jiffies(10));

}

static int
geomagnetic_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct geomagnetic_data *data = i2c_get_clientdata(client);

	if (atomic_read(&data->enable))
		cancel_delayed_work_sync(&data->work);
#if DEBUG
	data->suspend = 1;
#endif

	return 0;
}

static int
geomagnetic_resume(struct i2c_client *client)
{
	struct geomagnetic_data *data = i2c_get_clientdata(client);

	if (atomic_read(&data->enable))
		schedule_delayed_work(&data->work, 0);

#if DEBUG
	data->suspend = 0;
#endif

	return 0;
}


static int
geomagnetic_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct geomagnetic_data *data = NULL;
	struct input_dev *input_data = NULL, *input_raw = NULL;
	int rt, sysfs_created = 0, sysfs_raw_created = 0;
	int data_registered = 0, raw_registered = 0, i;
	struct yas_mag_filter filter;

	i2c_set_clientdata(client, NULL);
	data = kzalloc(sizeof(struct geomagnetic_data), GFP_KERNEL);
	if (data == NULL) {
		rt = -ENOMEM;
		goto err;
	}
	data->threshold = YAS_DEFAULT_MAGCALIB_THRESHOLD;
	for (i = 0; i < 3; i++)
		data->distortion[i] = YAS_DEFAULT_MAGCALIB_DISTORTION;
	data->shape = 0;
	atomic_set(&data->enable, 0);
	for (i = 0; i < 3; i++)
		atomic_set(&data->last_data[i], 0);
	atomic_set(&data->last_status, 0);
	INIT_DELAYED_WORK(&data->work, geomagnetic_input_work_func);
	sema_init(&data->driver_lock, 1);
	sema_init(&data->multi_lock, 1);
	yas530_pdata = client->dev.platform_data;
	if (yas530_pdata == NULL) 
	{
        yas530_pdata = kzalloc(sizeof(*yas530_pdata), GFP_KERNEL);
        
		if (yas530_pdata == NULL) 
		{
            rt = -ENOMEM;
            goto exit_free;
		}
		else
		{
			yas530_pdata->reset = 51;
		}
    }
	
	rt = gpio_request(yas530_pdata->reset, "yas530");
    if (rt < 0)
	{
        printk(KERN_ERR "yas530_init_client: request reset gpio failed\n");
        goto exit_free;
    }
    
	rt = gpio_direction_output(yas530_pdata->reset, 1);
    if (rt < 0)
	{
        printk(KERN_ERR "yas530_init_client: request reset gpio failed\n");
        goto exit_free_gpio;
    }

	input_data = input_allocate_device();
	if (input_data == NULL) {
		rt = -ENOMEM;
		YLOGE(("geomagnetic_probe: Failed to allocate input_data "
					"device\n"));
		goto err;
	}

	input_data->name = GEOMAGNETIC_INPUT_NAME;
	input_data->id.bustype = BUS_I2C;
	set_bit(EV_ABS, input_data->evbit);
	input_set_abs_params(input_data, ABS_X, INT_MIN, INT_MAX, 0, 0);
	input_set_abs_params(input_data, ABS_Y, INT_MIN, INT_MAX, 0, 0);
	input_set_abs_params(input_data, ABS_Z, INT_MIN, INT_MAX, 0, 0);
	input_set_abs_params(input_data, ABS_RUDDER, INT_MIN, INT_MAX, 0, 0);
	input_set_abs_params(input_data, ABS_STATUS, 0, 3, 0, 0);
	input_set_abs_params(input_data, ABS_WAKE, INT_MIN, INT_MAX, 0, 0);
	input_data->dev.parent = &client->dev;

	rt = input_register_device(input_data);
	if (rt) {
		YLOGE(("geomagnetic_probe: Unable to register input_data "
					"device: %s\n", input_data->name));
		goto err;
	}
	data_registered = 1;

	rt = sysfs_create_group(&input_data->dev.kobj,
			&geomagnetic_attribute_group);
	if (rt) {
		YLOGE(("geomagnetic_probe: sysfs_create_group failed[%s]\n",
					input_data->name));
		goto err;
	}
	sysfs_created = 1;

	input_raw = input_allocate_device();
	if (input_raw == NULL) {
		rt = -ENOMEM;
		YLOGE(("geomagnetic_probe: Failed to allocate input_raw "
					"device\n"));
		goto err;
	}

	input_raw->name = GEOMAGNETIC_INPUT_RAW_NAME;
	input_raw->id.bustype = BUS_I2C;
	set_bit(EV_ABS, input_raw->evbit);
	input_set_abs_params(input_raw, ABS_X, INT_MIN, INT_MAX, 0, 0);
	input_set_abs_params(input_raw, ABS_Y, INT_MIN, INT_MAX, 0, 0);
	input_set_abs_params(input_raw, ABS_Z, INT_MIN, INT_MAX, 0, 0);
	input_set_abs_params(input_raw, ABS_RAW_DISTORTION, 0, INT_MAX, 0, 0);
	input_set_abs_params(input_raw, ABS_RAW_THRESHOLD, 0, 2, 0, 0);
	input_set_abs_params(input_raw, ABS_RAW_SHAPE, 0, 1, 0, 0);
	input_set_abs_params(input_raw, ABS_RAW_MODE, 0, 1, 0, 0);
	input_set_abs_params(input_raw, ABS_RAW_REPORT, INT_MIN, INT_MAX, 0, 0);
	input_raw->dev.parent = &client->dev;

	rt = input_register_device(input_raw);
	if (rt) {
		YLOGE(("geomagnetic_probe: Unable to register input_raw device:"
					" %s\n", input_raw->name));
		goto err;
	}
	raw_registered = 1;

	rt = sysfs_create_group(&input_raw->dev.kobj,
			&geomagnetic_raw_attribute_group);
	if (rt) {
		YLOGE(("geomagnetic_probe: sysfs_create_group failed[%s]\n",
					input_data->name));
		goto err;
	}
	sysfs_raw_created = 1;

	this_client = client;
	data->input_raw = input_raw;
	data->input_data = input_data;
	input_set_drvdata(input_data, data);
	input_set_drvdata(input_raw, data);
	i2c_set_clientdata(client, data);

	rt = yas_mag_driver_init(&hwdriver);
	if (rt < 0) {
		YLOGE(("yas_mag_driver_init failed[%d]\n", rt));
		goto err;
	}
	if (hwdriver.init != NULL) {
		rt = hwdriver.init();
		if (rt < 0) {
			YLOGE(("hwdriver.init() failed[%d]\n", rt));
			goto err;
		}
	}
	if (hwdriver.set_position != NULL) {
		if (hwdriver.set_position(
					CONFIG_INPUT_YAS_MAGNETOMETER_POSITION)
				< 0) {
			YLOGE(("hwdriver.set_position() failed[%d]\n", rt));
			goto err;
		}
	}
	if (hwdriver.get_offset != NULL) {
		if (hwdriver.get_offset(&data->driver_offset) < 0) {
			YLOGE(("hwdriver get_driver_state failed\n"));
			goto err;
		}
	}
	if (hwdriver.get_delay != NULL)
		data->delay = hwdriver.get_delay();
	if (hwdriver.set_filter_enable != NULL) {
		
		if (hwdriver.set_filter_enable(1) == 0)
			data->filter_enable = 1;
	}
	if (hwdriver.get_filter != NULL) {
		if (hwdriver.get_filter(&filter) < 0) {
			YLOGE(("hwdriver get_filter failed\n"));
			goto err;
		}
		data->filter_len = filter.len;
		for (i = 0; i < 3; i++)
			data->filter_noise[i] = filter.noise[i];
		data->filter_threshold = filter.threshold;
	}
	if (hwdriver.get_static_matrix != NULL)
		hwdriver.get_static_matrix(&data->static_matrix);
	if (hwdriver.get_dynamic_matrix != NULL)
		hwdriver.get_dynamic_matrix(&data->dynamic_matrix);











	if (misc_register(&sensor_diag) < 0)
		goto err;

	return 0;

err:
	if (data != NULL) {
		if (input_raw != NULL) {
			if (sysfs_raw_created) {
				sysfs_remove_group(&input_raw->dev.kobj,
					&geomagnetic_raw_attribute_group);
			}
			if (raw_registered)
				input_unregister_device(input_raw);
			else
				input_free_device(input_raw);
		}
		if (input_data != NULL) {
			if (sysfs_created) {
				sysfs_remove_group(&input_data->dev.kobj,
					&geomagnetic_attribute_group);
			}
			if (data_registered)
				input_unregister_device(input_data);
			else
				input_free_device(input_data);
		}
		kfree(data);
	}
exit_free_gpio:
	gpio_free(yas530_pdata->reset);
exit_free:
	kfree(data);
	return rt;
}

static int
geomagnetic_remove(struct i2c_client *client)
{
	struct geomagnetic_data *data = i2c_get_clientdata(client);





	misc_deregister(&sensor_diag);

	if (data != NULL) {
		geomagnetic_disable(data);
		if (hwdriver.term != NULL)
			hwdriver.term();

		input_unregister_device(data->input_raw);
		sysfs_remove_group(&data->input_data->dev.kobj,
				&geomagnetic_attribute_group);
		sysfs_remove_group(&data->input_raw->dev.kobj,
				&geomagnetic_raw_attribute_group);
		input_unregister_device(data->input_data);
		kfree(data);
	}

	return 0;
}


static struct i2c_device_id geomagnetic_idtable[] = {
	{GEOMAGNETIC_I2C_DEVICE_NAME, 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, geomagnetic_idtable);

static struct i2c_driver geomagnetic_i2c_driver = {
	.driver = {
		.name		= GEOMAGNETIC_I2C_DEVICE_NAME,
		.owner		= THIS_MODULE,
	},

	.id_table		= geomagnetic_idtable,
	.probe			= geomagnetic_probe,
	.remove			= geomagnetic_remove,
	.suspend		= geomagnetic_suspend,
	.resume			= geomagnetic_resume,
};

static int __init
geomagnetic_init(void)
{
	return i2c_add_driver(&geomagnetic_i2c_driver);
}

static void __exit
geomagnetic_term(void)
{
	i2c_del_driver(&geomagnetic_i2c_driver);
}































































module_init(geomagnetic_init);
module_exit(geomagnetic_term);

MODULE_AUTHOR("Yamaha Corporation");
#if YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS529
MODULE_DESCRIPTION("YAS529 Geomagnetic Sensor Driver");
#elif YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS530
MODULE_DESCRIPTION("YAS530 Geomagnetic Sensor Driver");
#elif YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS532
MODULE_DESCRIPTION("YAS532 Geomagnetic Sensor Driver");
#endif
MODULE_LICENSE("GPL");
MODULE_VERSION("4.3.701a");

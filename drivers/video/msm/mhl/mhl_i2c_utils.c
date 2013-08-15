/* 
 * SiI8334 Linux Driver
 *
 * Copyright (C) 2011-2012 Silicon Image Inc.
 * Copyright (c) 2012, Code Aurora Forum. All rights reserved.
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
#include <linux/i2c.h>
#include <linux/mhl_8334.h>

#include <linux/mfd/pm8xxx/pm8921-charger.h>


#include "mhl_i2c_utils.h"


extern struct mhl_msm_state_t *mhl_drv_data;


uint8_t slave_addrs[MAX_PAGES] = {
	DEV_PAGE_TPI_0    ,
	DEV_PAGE_TX_L0_0  ,
	DEV_PAGE_TX_L1_0  ,
	DEV_PAGE_TX_2_0   ,
	DEV_PAGE_TX_3_0   ,
	DEV_PAGE_CBUS     ,
	DEV_PAGE_DDC_EDID ,
	DEV_PAGE_DDC_SEGM ,
};

static void msm_mhl_i2c_alarm_watch(int32_t cause, uint16_t saddr , unsigned char *data, int direction)
{
	unsigned char event_id    = 0x00;   
	unsigned char error_info  = 0x00;   
	unsigned char error_cause = 0x00;   
	
	if( direction == 1){
		
		error_info = 0x06;
	} else {
		
		error_info = 0x09;
	}
	
	switch(saddr){
	case 0x39:
		event_id = 0x7F;
		break;
	case 0x3D:
		event_id = 0xA1;
		break;
	case 0x49:
		event_id = 0xA2;
		break;
	case 0x4D:
		event_id = 0xA3;
		break;
	case 0x60:
		event_id = 0xA4;
		break;
	case 0x64:
		event_id = 0xA5;
		break;
	default:
		event_id = 0;
	}
		
	error_cause = ( unsigned char )( cause * -1 );
	if ((0 != error_cause) && (0 != event_id))
	{
		
		printk( KERN_ERR "[T][ARM]Event:0x%02X Info:0x%02X%02X%02X%02X",
			event_id, error_info, error_cause, data[0],data[1]);
	}
}

int mhl_i2c_reg_read(uint8_t slave_addr_index, uint8_t reg_offset)
{
	struct i2c_msg msgs[2];
	uint8_t buffer = 0;
	int ret = -1;
	uint8_t data[2];

	MHL_DEV_DBG("MRR: Reading from slave_addr_index=[%x] and offset=[%x]\n",
		slave_addr_index, reg_offset);
	MHL_DEV_DBG("MRR: Addr slave_addr_index=[%x]\n",
		slave_addrs[slave_addr_index]);







	/* Slave addr */
	msgs[0].addr = slave_addrs[slave_addr_index] >> 1;
	msgs[1].addr = slave_addrs[slave_addr_index] >> 1;

	/* Write Command */
	msgs[0].flags = 0;
	msgs[1].flags = I2C_M_RD;

	/* Register offset for the next transaction */
	msgs[0].buf = &reg_offset;
	msgs[1].buf = &buffer;

	/* Offset is 1 Byte long */
	msgs[0].len = 1;
	msgs[1].len = 1;
	data[0] = reg_offset;
	data[1] = 0;

	ret = i2c_transfer(mhl_drv_data->i2c_client->adapter, msgs, 2);




	if (ret < 1) {
		pr_err("I2C READ FAILED=[%d]\n", ret);
		data[1] = buffer;
		msm_mhl_i2c_alarm_watch(ret,slave_addrs[slave_addr_index] >> 1,data,1);
		return -EACCES;
	}

	MHL_DEV_DBG("Buffer is [%x]\n", buffer);




	return buffer;
}


int mhl_i2c_reg_write(uint8_t slave_addr_index, uint8_t reg_offset,
	uint8_t value)
{
	return mhl_i2c_reg_write_cmds(slave_addr_index, reg_offset, &value, 1);
}

int mhl_i2c_reg_write_cmds(uint8_t slave_addr_index, uint8_t reg_offset,
	uint8_t *value, uint16_t count)
{
	struct i2c_msg msgs[1];
	uint8_t data[2];
	int status = -EACCES;

	msgs[0].addr = slave_addrs[slave_addr_index] >> 1;
	msgs[0].flags = 0;
	msgs[0].len = 2;
	msgs[0].buf = data;
	data[0] = reg_offset;
	data[1] = *value;


	status = i2c_transfer(mhl_drv_data->i2c_client->adapter, msgs, 1);




	if (status < 1) {
		pr_err("I2C WRITE FAILED=[%d]\n", status);
		msm_mhl_i2c_alarm_watch(status,slave_addrs[slave_addr_index] >> 1,data,0);
		return -EACCES;
	}

	return status;
}

void mhl_i2c_reg_modify(uint8_t slave_addr_index, uint8_t reg_offset,
	uint8_t mask, uint8_t val)
{
	uint8_t temp;

	temp = mhl_i2c_reg_read(slave_addr_index, reg_offset);
	temp &= (~mask);
	temp |= (mask & val);
	mhl_i2c_reg_write(slave_addr_index, reg_offset, temp);
}


int mhl_charge_on(int state, int charge_current)
{
	int ret = 0;

	switch (state) {
		case POWER_ON:
			ret = pm8921_set_usb_power_supply_type
				(POWER_SUPPLY_TYPE_USB);
			if (ret) {
				MHL_DEV_INFO("MHL SII8334 : pm8921_set_usb_power_supply_type() error\n");
			}
			pm8921_charger_vbus_draw(charge_current);

			break;
		case POWER_OFF:
			pm8921_charger_vbus_draw(3);
			break;
		case POWER_ERROR:
		default:
			ret = -1;
	}
	return ret;
}

#define MHL_SII8334_DEV_STATE		"MHL SII8334 : DEV STATE		"
#define MHL_SII8334_MHL_VERSION		"MHL SII8334 : MHL VERSION		"
#define MHL_SII8334_DEV_CAT			"MHL SII8334 : DEV CAT			"
#define MHL_SII8334_ADOPTER_ID_H	"MHL SII8334 : ADOPTER ID HIGHT "
#define MHL_SII8334_ADOPTER_ID_L	"MHL SII8334 : ADOPTER ID LOW   "
#define MHL_SII8334_VID_LINK_MODE	"MHL SII8334 : VID LINK MODE    "
#define MHL_SII8334_AUD_LINK_MODE	"MHL SII8334 : AUD LINK MODE    "
#define MHL_SII8334_VIDEO_TYPE		"MHL SII8334 : VIDEO TYPE       "
#define MHL_SII8334_LOG_DEV_MAP		"MHL SII8334 : LOG DEV MAP      "
#define MHL_SII8334_BANDWIDTH		"MHL SII8334 : BANDWIDTH        "
#define MHL_SII8334_FEATURE_FLAG	"MHL SII8334 : FEATURE FLAG	    "
#define MHL_SII8334_DEVICE_ID_H		"MHL SII8334 : DEVICE ID HIGHT  "
#define MHL_SII8334_DEVICE_ID_L		"MHL SII8334 : DEVICE ID LOW    "
#define MHL_SII8334_SCRATCHPAD_SIZE	"MHL SII8334 : SCRATCHPAD SIZE  "
#define MHL_SII8334_INT_STAT_SIZE	"MHL SII8334 : INT STAT SIZE    "
#define MHL_SII8334_RESERVED		"MHL SII8334 : RESERVED         "

void mhl_debug_devcap_info(struct mhl_msm_state_t *mhl_drv_data, int index)
{
	uint8_t regval;

	regval = mhl_drv_data->state.peer_devcap[index];
	switch(index) {
		case DEVCAP_OFFSET_DEV_STATE:
			MHL_DEV_DBG("MHL SII8334 DCAP: %04x %s: %04x DEV_TYPE=%X POW=%s\n",
					index, MHL_SII8334_DEV_STATE, regval,
					regval & 0x0F, (regval & 0x10) ? "y" : "n");
			break;
		case DEVCAP_OFFSET_MHL_VERSION:
			MHL_DEV_DBG("MHL SII8334 DCAP: %04x %s: %04x\n",
					index, MHL_SII8334_MHL_VERSION, regval);
			break;
		case DEVCAP_OFFSET_DEV_CAT:
			MHL_DEV_DBG("MHL SII8334 DCAP: %04x %s: %04x\n",
					index, MHL_SII8334_DEV_CAT, regval);
			break;
		case DEVCAP_OFFSET_ADOPTER_ID_H:
			MHL_DEV_DBG("MHL SII8334 DCAP: %04x %s: %04x\n",
					index, MHL_SII8334_ADOPTER_ID_H, regval);
			break;
		case DEVCAP_OFFSET_ADOPTER_ID_L:
			MHL_DEV_DBG("MHL SII8334 DCAP: %04x %s: %04x\n",
					index, MHL_SII8334_ADOPTER_ID_L, regval);
			break;
		case DEVCAP_OFFSET_VID_LINK_MODE:
			MHL_DEV_DBG("MHL SII8334 DCAP: %04x %s: %04x\n",
					index, MHL_SII8334_VID_LINK_MODE, regval);
			break;
		case DEVCAP_OFFSET_AUD_LINK_MODE:
			MHL_DEV_DBG("MHL SII8334 DCAP: %04x %s: %04x\n",
					index, MHL_SII8334_AUD_LINK_MODE, regval);
			break;
		case DEVCAP_OFFSET_VIDEO_TYPE:
			MHL_DEV_DBG("MHL SII8334 DCAP: %04x %s: %04x\n",
					index, MHL_SII8334_VIDEO_TYPE, regval);
			break;
		case DEVCAP_OFFSET_LOG_DEV_MAP:
			MHL_DEV_DBG("MHL SII8334 DCAP: %04x %s: %04x\n",
					index, MHL_SII8334_LOG_DEV_MAP, regval);
			break;
		case DEVCAP_OFFSET_BANDWIDTH:
			MHL_DEV_DBG("MHL SII8334 DCAP: %04x %s: %04x\n",
					index, MHL_SII8334_BANDWIDTH, regval);
			break;
		case DEVCAP_OFFSET_FEATURE_FLAG:
			MHL_DEV_DBG("MHL SII8334 DCAP: %04x %s: %04x RCP=%s RAP=%s SP=%s\n",
					index, MHL_SII8334_FEATURE_FLAG, regval,
					(regval & 0x01) ? "y" : "n",
					(regval & 0x02) ? "y" : "n",
					(regval & 0x04) ? "y" : "n");
			break;
		case DEVCAP_OFFSET_DEVICE_ID_H:
			MHL_DEV_DBG("MHL SII8334 DCAP: %04x %s: %04x\n",
					index, MHL_SII8334_DEVICE_ID_H, regval);
			break;
		case DEVCAP_OFFSET_DEVICE_ID_L:
			MHL_DEV_DBG("MHL SII8334 DCAP: %04x %s: %04x\n",
					index, MHL_SII8334_DEVICE_ID_L, regval);
			break;
		case DEVCAP_OFFSET_SCRATCHPAD_SIZE:
			MHL_DEV_DBG("MHL SII8334 DCAP: %04x %s: %04x\n",
					index, MHL_SII8334_SCRATCHPAD_SIZE, regval);
			break;
		case DEVCAP_OFFSET_INT_STAT_SIZE:
			MHL_DEV_DBG("MHL SII8334 DCAP: %04x %s: %04x\n",
					index, MHL_SII8334_INT_STAT_SIZE, regval);
			break;
		case DEVCAP_OFFSET_RESERVED:
			MHL_DEV_DBG("MHL SII8334 DCAP: %04x %s: %04x\n",
					index, MHL_SII8334_RESERVED, regval);
			break;
		case DEVCAP_SIZE:
		default:
			MHL_DEV_DBG("MHL SII8334 DCAP: should not be here\n");

	}
}

int atoi(const char *name)
{
	int val = 0;

	for (;; name++) {
		switch (*name) {
			case '0' ... '9':
				val = 10*val+(*name-'0');
				break;
			default:
				return val;
		}
	}
}




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




#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <mach/msm_hdmi_audio.h>
#include <mach/clk.h>
#include <linux/err.h>
#include <mach/msm_iomap.h>
#include <linux/completion.h>
#include <mach/socinfo.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/workqueue.h>

#include <linux/mfd/pm8xxx/pm8921-charger.h>

#include <linux/regulator/consumer.h>
#include <linux/device.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>
#include <linux/mhl_defs.h>
#include <linux/mhl_8334.h>

#include "../msm_fb.h"
#include "../external_common.h"
#include "../hdmi_msm.h"
#include "mhl_i2c_utils.h"
#include <linux/pm_obs_api.h>





















































#define MSC_COMMAND_TIME_OUT 2050


struct timer_list enable_irq_timer_2;
struct work_struct enable_irq_timer_work_2;
struct mhl_msm_state_t *mhl_drv_data;
static DEFINE_MUTEX(mhl_state_mutex);
static struct wake_lock mhl_wake_lock;
struct workqueue_struct *mhl_workqueue;


static bool mhl_charging_flag = false;
static int mhl_charging_mA = 0;


static DEFINE_MUTEX(msc_command_queue_mutex);
struct workqueue_struct *msc_command_workqueue;

static DEFINE_MUTEX(usb_online_mutex);


int mhl_has_unplugged = false;

static int mhl_diag_test_mode_on = false;

static const struct i2c_device_id mhl_i2c_id[] = {
	{ MHL_DRIVER_NAME, 0 },
	{ }
};

static void mhl_init_reg_settings(bool mhl_disc_en);
static int mhl_i2c_probe(struct i2c_client *client,
		const struct i2c_device_id *id);
static int mhl_i2c_remove(struct i2c_client *client);
static void force_usb_switch_open(void);
static void release_usb_switch_open(void);
static void switch_mode(enum mhl_st_type to_mode);
static irqreturn_t mhl_tx_isr(int irq, void *dev_id);
static int mhl_drive_tmds(int to_state);
static int mhl_drive_hpd(uint8_t to_state);
void (*notify_usb_online)(int online);
static int mhl_notify_plugged(void);

static int mhl_msc_send_write_stat(uint8_t offset, uint8_t value);
static int mhl_msc_recv_write_stat(uint8_t offset, uint8_t value);
static int mhl_msc_send_set_int(uint8_t offset, uint8_t mask);
static int mhl_msc_recv_set_int(uint8_t offset, uint8_t mask);
static int mhl_msc_read_devcap(uint8_t offset);
static int mhl_msc_read_devcap_all(void);
static int mhl_msc_send_msc_msg(uint8_t sub_cmd, uint8_t cmd_data);
static int mhl_msc_command_done(cbus_req_t *req);



static int mhl_refresh_peer_devcap_entries(int offset, uint8_t devcap);
static int mhl_rcp_recv(uint8_t key_code);
static int mhl_rap_recv(uint8_t action_code);



static int mhl_i2c_suspend(struct device *dev);
static int mhl_i2c_resume(struct device *dev);
static struct dev_pm_ops mhl_pm_ops = {
	    .suspend    = mhl_i2c_suspend,
		.resume     = mhl_i2c_resume,
};


static struct i2c_driver mhl_i2c_driver = {
	.driver = {
		.name = MHL_DRIVER_NAME,
		.owner = THIS_MODULE,

		.pm = &mhl_pm_ops,

	},
	.probe = mhl_i2c_probe,
	/*.remove =  __exit_p(mhl_i2c_remove),*/
	.remove = mhl_i2c_remove,
	.id_table = mhl_i2c_id,
};

#define MSC_START_BIT_MSC_CMD		        (0x01 << 0)
#define MSC_START_BIT_VS_CMD		        (0x01 << 1)
#define MSC_START_BIT_READ_REG		        (0x01 << 2)
#define MSC_START_BIT_WRITE_REG		        (0x01 << 3)
#define MSC_START_BIT_WRITE_BURST	        (0x01 << 4)













static int mhl_send_msc_command(cbus_req_t *req)
{
	uint8_t startbit = 0x00;
	uint8_t *burst_data;
	int timeout;
	int i;

	


	mutex_lock(&mhl_state_mutex);
	if (mhl_drv_data->cur_state != POWER_STATE_D0_NO_MHL) {
		MHL_DEV_DBG("%s: cur_state:%04x CBUS(0x0A):%04x\n",
		__func__,
		mhl_drv_data->cur_state, mhl_i2c_reg_read(TX_PAGE_CBUS, 0x0A));
		mutex_unlock(&mhl_state_mutex);
		return -EFAULT;
	}
	mutex_unlock(&mhl_state_mutex);

	if (!req)
		return -EFAULT;

	MHL_DEV_DBG("%s: command=0x%04x offset=0x%04x %04x %04x",
		__func__,
		req->command,
		req->offsetdata,
		req->payload_u.msgdata[0],
		req->payload_u.msgdata[1]);

	


	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x13, req->offsetdata);
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x14, req->payload_u.msgdata[0]);
	if(req->command == MHL_SET_HPD)
	{
		MHL_DEV_DBG("Get MHL_SET_HPD\n");
	}
	else if(req->command == MHL_CLR_HPD)
	{
		MHL_DEV_DBG("Get MHL_CLR_HPD\n");
	}

	switch (req->command) {
	
	case MHL_SET_INT:
		startbit = MSC_START_BIT_WRITE_REG;
    
	case MHL_WRITE_STAT:
		startbit = MSC_START_BIT_WRITE_REG;
		break;
    
	case MHL_READ_DEVCAP:
		startbit = MSC_START_BIT_READ_REG;
		break;
	
	case MHL_GET_STATE: 
	
	case MHL_GET_VENDOR_ID: 
	
	case MHL_SET_HPD: 
	
	case MHL_CLR_HPD:
	
	case MHL_GET_SC1_ERRORCODE:
	
	case MHL_GET_DDC_ERRORCODE:
	
	case MHL_GET_MSC_ERRORCODE:
	
	case MHL_GET_SC3_ERRORCODE:
		mhl_i2c_reg_write(TX_PAGE_CBUS, 0x13, req->command);
		startbit = MSC_START_BIT_MSC_CMD;
		break;
	case MHL_MSC_MSG:
		startbit = MSC_START_BIT_VS_CMD;
		mhl_i2c_reg_write(TX_PAGE_CBUS, 0x15, req->payload_u.msgdata[1]);
		mhl_i2c_reg_write(TX_PAGE_CBUS, 0x13, req->command);
		break;
	case MHL_WRITE_BURST:
		
		startbit = MSC_START_BIT_WRITE_BURST;
		mhl_i2c_reg_write(TX_PAGE_CBUS, 0x20, req->length - 1);
		if (!(req->payload_u.pdatabytes)) {
			MHL_DEV_ERR("%s: Drv: Put pointer to WRITE_BURST data in req.pdatabytes!!!\n", __func__);
			goto cbus_command_send_out;
		}
		burst_data = req->payload_u.pdatabytes;
		for (i = 0; i < req->length; i++, burst_data++)
			mhl_i2c_reg_write(TX_PAGE_CBUS, 0xC0 + i, *burst_data);
		break;
	default:
		MHL_DEV_ERR("%s: unknown command! (%04x)\n",
			__func__, req->command);
		goto cbus_command_send_out;
	}

	init_completion(&mhl_drv_data->msc_command_done);
	


	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x12, startbit);
	timeout = wait_for_completion_interruptible_timeout
		(&mhl_drv_data->msc_command_done,
		msecs_to_jiffies(MSC_COMMAND_TIME_OUT));
	if (!timeout) {
		MHL_DEV_ERR("%s: cbus_command_send timed out!\n", __func__);
		goto cbus_command_send_out;
	}

	
	switch (req->command) {
	case MHL_READ_DEVCAP:
		req->reqstatus = mhl_i2c_reg_read(TX_PAGE_CBUS, 0x16);	
		break;
	case MHL_MSC_MSG:
		
		if (mhl_i2c_reg_read(TX_PAGE_CBUS, 0x20) & BIT6)
			return -EAGAIN;
		
	default:
		req->reqstatus = 0;
		break;
	}
	mhl_msc_command_done(req);

	MHL_DEV_DBG("%s: done\n", __func__);

	return 0;
cbus_command_send_out:
	return -EFAULT;
}

void set_mhl_mode(int flag)
{
	unsigned long flags;
	spin_lock_irqsave(&mhl_drv_data->mhl_mode_lock, flags);
	if (flag == mhl_drv_data->mhl_mode) {
		spin_unlock_irqrestore(&mhl_drv_data->mhl_mode_lock, flags);
		return;
	} else {
		mhl_drv_data->mhl_mode = flag;
	}
	spin_unlock_irqrestore(&mhl_drv_data->mhl_mode_lock, flags);

	return;
}

static int mhl_set_state(unsigned int state)
{
	mhl_drv_data->mhl_online |= state;

	return 0;
}
static int mhl_notify_online(void)
{
	if (!mhl_drv_data)
		return -EFAULT;

	
	mhl_msc_send_write_stat(
			MHL_STATUS_REG_CONNECTED_RDY,
			MHL_STATUS_DCAP_RDY);

	
	mhl_msc_send_set_int(
			MHL_RCHANGE_INT,
			MHL_INT_DCAP_CHG);

	mhl_drv_data->mhl_online |= MHL_LINK_EST;

	return 0;
}

static int mhl_notify_offline(void)
{
	mhl_drv_data->mhl_online = 0;
	mhl_drv_data->hpd_state = 0;
	mhl_drv_data->devcap_state = 0;
	memset(&mhl_drv_data->state, 0, sizeof(struct mhl_state));

	return 0;
}


static int mhl_clear_state(unsigned int state)
{

	mhl_drv_data->mhl_online &= ~state;

	return 0;
}


bool mhl_is_charge(void)
{
	return mhl_charging_flag;
}
int get_mhl_charging_current(void)
{
	return mhl_charging_mA;
}






static int mhl_msc_command_done(cbus_req_t *req)
{
	uint8_t regval;
	char *envp[2];

	switch (req->command) {
	case MHL_WRITE_STAT:
		if (req->offsetdata == MHL_STATUS_REG_LINK_MODE) {
			if (req->payload_u.msgdata[0]
				& MHL_STATUS_PATH_ENABLED) {
				
				if (mhl_drv_data->full_operation)
					mhl_drive_tmds(true);



			} else {
				
				mhl_drive_tmds(false);



			}
		}
		break;
	case MHL_READ_DEVCAP:
		mhl_refresh_peer_devcap_entries(req->offsetdata, req->reqstatus);
		mhl_drv_data->devcap_state |= BIT(req->offsetdata);
		if ((mhl_drv_data->devcap_state & 0xFFFF) == 0xFFFF) {
			mhl_drv_data->devcap_state = 0;
			envp[0] = "DEVCAP_CHANGED";
			envp[1] = NULL;
			kobject_uevent_env(&mhl_drv_data->i2c_client->dev.kobj,
					   KOBJ_CHANGE, envp);
		}
		switch (req->offsetdata) {
		case MHL_DEV_CATEGORY_OFFSET:
			if (req->reqstatus & MHL_DEV_CATEGORY_POW_BIT) {
				regval = mhl_i2c_reg_read(TX_PAGE_3, 0x17);
				mhl_i2c_reg_write(TX_PAGE_3, 0x17, regval | 0x04);

			
			if(req->reqstatus == 0x11)
			{
				if (mhl_drv_data->charging_enable) {
					pm8921_regulate_input_voltage(4600);
					mhl_charging_flag = true;
					mhl_charging_mA = 1500;
					mhl_drv_data->charging_enable(TRUE, 1500);
				}
			}
			
			else if(req->reqstatus == 0x12)
			{
				if (mhl_drv_data->charging_enable) {
					mhl_charging_flag = false;
					mhl_charging_mA = 0;
					mhl_drv_data->charging_enable(FALSE, 0);
					pm8921_regulate_input_voltage(4500);

				}
			}
			
			else if(req->reqstatus  == 0x13)
			{
				if (mhl_drv_data->charging_enable) {
					pm8921_regulate_input_voltage(4600);
					mhl_charging_flag = true;
					mhl_charging_mA = 1500;
					mhl_drv_data->charging_enable(TRUE, 1500);

				}
			}
			}

			mhl_set_state(MHL_DCAP_OK);
			break;
		case DEVCAP_OFFSET_MHL_VERSION:
		case DEVCAP_OFFSET_INT_STAT_SIZE:







			mhl_drive_tmds(true);

			break;
		case DEVCAP_OFFSET_ADOPTER_ID_H:
			MHL_DEV_INFO("MHL: DEVCAP_OFFSET_ADOPTER_ID_H (0x%04x)\n", req->reqstatus);
			break;
		case DEVCAP_OFFSET_ADOPTER_ID_L:
			MHL_DEV_INFO("MHL: DEVCAP_OFFSET_ADOPTER_ID_L (0x%04x)\n", req->reqstatus);
			break;
		case DEVCAP_OFFSET_DEVICE_ID_H:
			MHL_DEV_INFO("MHL: DEVCAP_OFFSET_DEVICE_ID_H (0x%04x)\n", req->reqstatus);
			break;
		case DEVCAP_OFFSET_DEVICE_ID_L:
			MHL_DEV_INFO("MHL: DEVCAP_OFFSET_DEVICE_ID_L (0x%04x)\n", req->reqstatus);
			break;
		}

		break;
	}
	return 0;
}

static void mhl_msc_command_work(struct work_struct *work)
{
	struct mhl_event *event;
	int retry;
	int ret;

	mutex_lock(&msc_command_queue_mutex);
	while (!list_empty(&mhl_drv_data->msc_queue)) {
		event = list_first_entry(&mhl_drv_data->msc_queue, struct mhl_event,
			msc_queue);
		list_del(&event->msc_queue);
		mutex_unlock(&msc_command_queue_mutex);
		ret = mhl_send_msc_command
			(&event->msc_command_queue);

		if (ret == -EAGAIN) {
			retry = 2;
			while (retry--) {
				ret = mhl_send_msc_command
					(&event->msc_command_queue);
				if (ret != -EAGAIN)
					break;
			}
		}
		if (ret == -EAGAIN)
			MHL_DEV_ERR("%s: send msc command retry out!\n", __func__);

		kfree(event);

		mutex_lock(&msc_command_queue_mutex);
		if (mhl_drv_data->msc_command_counter)
			mhl_drv_data->msc_command_counter--;
		else
			MHL_DEV_ERR("%s: msc_command_counter fail!\n", __func__);
	}
	mutex_unlock(&msc_command_queue_mutex);
}

static int mhl_queue_msc_command(cbus_req_t *comm, int prior)
{
	struct mhl_event *new_event;

	mutex_lock(&msc_command_queue_mutex);
	if (mhl_drv_data->msc_command_counter >= MSC_COMMAND_QUEUE_SIZE) {
		MHL_DEV_ERR("%s: queue full!\n", __func__);
		mutex_unlock(&msc_command_queue_mutex);
		return -EBUSY;
	}

	new_event = kmalloc(sizeof(struct mhl_event), GFP_KERNEL);
	if (!new_event) {
		MHL_DEV_ERR("%s: out of memory!\n", __func__);
		mutex_unlock(&msc_command_queue_mutex);
		return -ENOMEM;
	}
	memcpy(&new_event->msc_command_queue, comm,
		sizeof(cbus_req_t));

	mhl_drv_data->msc_command_counter++;
	if (prior)
		list_add(&new_event->msc_queue, &mhl_drv_data->msc_queue);
	else
		list_add_tail(&new_event->msc_queue, &mhl_drv_data->msc_queue);

	mutex_unlock(&msc_command_queue_mutex);
	queue_work(msc_command_workqueue, &mhl_drv_data->msc_command_work);
	return 0;
}

/*
 * MSC: 0x60 WRITE_STAT
 */
static int mhl_msc_send_write_stat(uint8_t offset, uint8_t value)
{
	cbus_req_t req;
    req.retrycount  = 2;
	req.command = MHL_WRITE_STAT;
	req.offsetdata = offset;
	req.payload_u.msgdata[0] = value;
	return mhl_queue_msc_command(&req, MSC_NORMAL_SEND);
}

static int mhl_msc_recv_write_stat(uint8_t offset, uint8_t value)
{
	if (offset >= 2)
		return -EFAULT;

	switch (offset) {
	case 0:
		
		if (((value ^ mhl_drv_data->state.device_status[offset]) &
			MHL_STATUS_DCAP_RDY)) {
			if (value & MHL_STATUS_DCAP_RDY) {
				mhl_drv_data->devcap_state = 0;
				mhl_msc_read_devcap_all();
			} else {
				
				mhl_clear_state(MHL_DCAP_OK);
			}
		}
		break;
	case 1:
		
		if ((value ^ mhl_drv_data->state.device_status[offset])
			& MHL_STATUS_PATH_ENABLED) {
			if (value & MHL_STATUS_PATH_ENABLED) {
				mhl_drv_data->state.peer_status[1]
					|= (MHL_STATUS_PATH_ENABLED |
					MHL_STATUS_CLK_MODE_NORMAL);
				mhl_msc_send_write_stat(
					MHL_STATUS_REG_LINK_MODE,
					mhl_drv_data->state.peer_status[1]);
			} else {
				mhl_drv_data->state.peer_status[1]
					&= ~(MHL_STATUS_PATH_ENABLED |
					MHL_STATUS_CLK_MODE_NORMAL);
				mhl_msc_send_write_stat(
					MHL_STATUS_REG_LINK_MODE,
					mhl_drv_data->state.peer_status[1]);
			}
		}
		break;
	}
	mhl_drv_data->state.device_status[offset] = value;
	return 0;
}




static int mhl_msc_send_set_int(uint8_t offset, uint8_t mask)
{
	cbus_req_t req;
    req.retrycount  = 2;
	req.command = MHL_SET_INT;
	req.offsetdata = offset;
	req.payload_u.msgdata[0] = mask;
	return mhl_queue_msc_command(&req, MSC_NORMAL_SEND);
}

static int mhl_msc_recv_set_int(uint8_t offset, uint8_t mask)
{
	if (offset >= 2)
		return -EFAULT;

	switch (offset) {
	case 0:
		
		if (mask & MHL_INT_DCAP_CHG) {
			
			mhl_clear_state(MHL_DCAP_OK);
			mhl_drv_data->devcap_state = 0;
			mhl_msc_read_devcap_all();
		}
		
		if (mask & MHL_INT_DSCR_CHG)
		
		if (mask & MHL_INT_REQ_WRT) {
			
			mhl_msc_send_set_int(
				MHL_RCHANGE_INT,
				MHL_INT_GRT_WRT);
		}
		
		if (mask & MHL_INT_GRT_WRT)
			complete_all(&mhl_drv_data->req_write_done);
		break;
	case 1:
		
		if (mask & MHL_INT_EDID_CHG) {
            

            MHL_DEV_DBG("Set HPD False in %s",__func__);
            mhl_drive_hpd(false);
            msleep(110);
            MHL_DEV_DBG("Set HPD True in %s",__func__);
            mhl_drive_hpd(true);
		}
		break;
	}
	return 0;
}











static int mhl_msc_read_devcap(uint8_t offset)
{
	cbus_req_t req;
    req.retrycount  = 2;
	req.command = MHL_READ_DEVCAP;
	req.offsetdata = offset;
	
	req.payload_u.msgdata[0] = 0;
	return mhl_queue_msc_command(&req, MSC_NORMAL_SEND);
}

static int mhl_msc_read_devcap_all(void)
{
	int offset;
	int ret;

	
	if (mhl_drv_data->mhl_online & MHL_DCAP_OK)
		return 0;

	for (offset = 0; offset < DEVCAP_SIZE; offset++) {
		ret = mhl_msc_read_devcap(offset);
		if (ret == -EBUSY)
			MHL_DEV_ERR("%s: queue busy!\n", __func__);
	}

	return 0;
}

static int mhl_refresh_peer_devcap_entries(int offset, uint8_t devcap)
{
	if (offset < 0 || offset > 15)
		return -EFAULT;
	mhl_drv_data->state.peer_devcap[offset] = devcap;
	mhl_debug_devcap_info(mhl_drv_data, offset);

	return 0;
}
/*
 * MSC: 0x68 MSC_MSG
 */
static int mhl_msc_send_msc_msg(uint8_t sub_cmd, uint8_t cmd_data)
{
	cbus_req_t req;
	req.command = MHL_MSC_MSG;
	req.payload_u.msgdata[0] = sub_cmd;
	req.payload_u.msgdata[1] = cmd_data;
	return mhl_queue_msc_command(&req, MSC_NORMAL_SEND);
}

static int mhl_prior_send_msc_command_msc_msg(uint8_t sub_cmd, uint8_t cmd_data)
{
	cbus_req_t req;
	req.command = MHL_MSC_MSG;
	req.payload_u.msgdata[0] = sub_cmd;
	req.payload_u.msgdata[1] = cmd_data;
	return mhl_queue_msc_command(&req, MSC_PRIOR_SEND);
}

static int mhl_msc_rap_vs_rcp_process(uint8_t sub_cmd, uint8_t cmd_data)
{
	int rc = 0;
	if(sub_cmd == 0xff || cmd_data == 0xff) {
		MHL_DEV_ERR("%s: sub_cmd = %x, cmd_data = %x\n", __func__, sub_cmd, cmd_data);
		return -EFAULT;
	}

	switch (sub_cmd) {
	case MHL_MSC_MSG_RCP:
		MHL_DEV_INFO("MHL: receive RCP(0x%04x)\n", cmd_data);
		rc = mhl_rcp_recv(cmd_data);
		break;
	case MHL_MSC_MSG_RCPK:
		MHL_DEV_INFO("MHL: receive RCPK(0x%04x)\n", cmd_data);
		break;
	case MHL_MSC_MSG_RCPE:
		MHL_DEV_INFO("MHL: receive RCPE(0x%04x)\n", cmd_data);
		break;
	case MHL_MSC_MSG_RAP:
		MHL_DEV_INFO("MHL: receive RAP(0x%04x)\n", cmd_data);
		rc = mhl_rap_recv(cmd_data);
		break;
	case MHL_MSC_MSG_RAPK:
		MHL_DEV_INFO("MHL: receive RAPK(0x%04x)\n", cmd_data);
		break;
	default:
		break;
	}
	return rc;
}

static int mhl_notify_rcp_recv(uint8_t key_code)
{
	char *envp[2];
	envp[0] = kmalloc(128, GFP_KERNEL);
	if (!envp[0])
		return -ENOMEM;
	snprintf(envp[0], 128, "RCP_KEYCODE=%x", key_code);
	envp[1] = NULL;
	kobject_uevent_env(&mhl_drv_data->i2c_client->dev.kobj, KOBJ_CHANGE, envp);
	kfree(envp[0]);
	return 0;
}


static uint8_t is_rcp_key_code_supported(uint8_t keycode) {
	switch (keycode & 0x7f) {
		case 0x00:  
		case 0x01:  
		case 0x02:  
		case 0x03:   
		case 0x04:   
		case 0x09:   
		case 0x0D:   
			return keycode + 1;
		default:   
			return 0;
	}
	return 0;
}

static int mhl_rcp_recv(uint8_t key_code)
{
	int rc;
	if (is_rcp_key_code_supported(key_code)) {
		
		mhl_notify_rcp_recv(key_code);
		
		rc = mhl_prior_send_msc_command_msc_msg(
			MHL_MSC_MSG_RCPK,
			key_code);
	} else {
		
		rc = mhl_prior_send_msc_command_msc_msg(
			MHL_MSC_MSG_RCPE,
			RCPE_INEEFECTIVE_KEY_CODE);
		if (rc)
			return rc;
		
		rc = mhl_msc_send_msc_msg(
			MHL_MSC_MSG_RCPK,
			key_code);
	}
	return rc;
}

static int mhl_rap_action(uint8_t action_code)
{
	switch (action_code) {
	case MHL_RAP_CONTENT_ON:
		mhl_drive_tmds(true);
		break;
	case MHL_RAP_CONTENT_OFF:
		mhl_drive_tmds(false);
		break;
	default:
		break;
	}
	return 0;
}

static int mhl_notify_rap_recv(uint8_t action_code)
{
	char *envp[2];
	envp[0] = kmalloc(128, GFP_KERNEL);
	if (!envp[0])
		return -ENOMEM;
	snprintf(envp[0], 128, "RAP_ACTIONCODE=%x", action_code);
	envp[1] = NULL;
	kobject_uevent_env(&mhl_drv_data->i2c_client->dev.kobj, KOBJ_CHANGE, envp);
	kfree(envp[0]);
	return 0;
}

static int mhl_rap_recv(uint8_t action_code)
{
	uint8_t error_code;

	switch (action_code) {
	case MHL_RAP_CONTENT_POLL:
	case MHL_RAP_CONTENT_ON:
	case MHL_RAP_CONTENT_OFF:
		if (mhl_drv_data->full_operation) {
			mhl_rap_action(action_code);
			error_code = RAPK_NO_ERROR;
			
			mhl_notify_rap_recv(action_code);
		} else
			error_code = RAPK_INEEFECTIVE_ACTION_CODE;
		break;
	default:
		error_code = RAPK_INEEFECTIVE_ACTION_CODE;
		break;
	}
	
	return mhl_prior_send_msc_command_msc_msg(
		MHL_MSC_MSG_RAPK,
		error_code);
}

static void mhl_notify_hpd(int to_state)
{
	MHL_DEV_DBG("%s To state=[0x%x]\n",__func__,to_state);
	mutex_lock(&mhl_state_mutex);
	if (mhl_drv_data->cur_state != POWER_STATE_D0_NO_MHL) {
		MHL_DEV_ERR("MHL: invalid state to ctrl HPD cur_state %x\n",mhl_drv_data->cur_state);
		mutex_unlock(&mhl_state_mutex);
		return;
	}
	mutex_unlock(&mhl_state_mutex);

	if (mhl_drv_data->hpd_state == to_state){
		MHL_DEV_DBG("To state=[0x%x]\n",to_state);
		return;
	}
	else
		mhl_drv_data->hpd_state = to_state;

	MHL_DEV_DBG("%s: To state=[0x%x]\n", __func__, to_state);
    MHL_DEV_DBG("%s: After TMDS Reg is %x \n", __func__,mhl_i2c_reg_read(TX_PAGE_L0, 0x80));

	return;
}

static void switch_mode(enum mhl_st_type to_mode)
{
	MHL_DEV_DBG("%s to_mode is %x",__func__,to_mode);
	switch (to_mode) {
	case POWER_STATE_D0_NO_MHL:
		MHL_DEV_INFO("%s to_mode = POWER_STATE_D0_NO_MHL\n", __func__);
			break;
	case POWER_STATE_D0_MHL:
		MHL_DEV_INFO("%s to_mode = POWER_STATE_D0_MHL\n", __func__);
		mutex_lock(&mhl_state_mutex);
		mhl_drv_data->cur_state = POWER_STATE_D0_MHL;
		mutex_unlock(&mhl_state_mutex);

		mhl_init_reg_settings(true);

		mhl_i2c_reg_write(TX_PAGE_3, 0x10, 0x25);
		mhl_i2c_reg_modify(TX_PAGE_L0, 0x1E, BIT0 | BIT1, 0x00);
		MHL_DEV_INFO("mhl: power state switched to D0\n");
		break;
	case POWER_STATE_D3:
		MHL_DEV_INFO("%s to_mode = POWER_STATE_D3\n", __func__);

		
		mhl_drive_hpd(false);

		

		mhl_i2c_reg_write(TX_PAGE_3, 0x30, 0xD0);
		msleep(50);

		
		mhl_i2c_reg_modify(TX_PAGE_L1, 0x3D, BIT0, 0x00);
		mutex_lock(&mhl_state_mutex);
		mhl_drv_data->cur_state = POWER_STATE_D3;
		mutex_unlock(&mhl_state_mutex);

		MHL_DEV_DBG("mhl: power state switched to D3\n");
		break;
	default:
		MHL_DEV_ERR("mhl: invalid power state! (%d)\n", to_mode);
		break;
	}
}

static void int_1_isr(void)
{
    
    uint8_t intr_1_stat;
	uint8_t cbus_stat;

    
	intr_1_stat = mhl_i2c_reg_read(TX_PAGE_L0, 0x71);

	if (intr_1_stat) {
		
		
		mhl_i2c_reg_write(TX_PAGE_L0, 0x71, intr_1_stat);
		
		if (intr_1_stat & BIT6) {
			




			
			cbus_stat = mhl_i2c_reg_read(TX_PAGE_CBUS, 0x0D);
			mhl_notify_hpd(!!(cbus_stat & BIT6));
			if(BIT6 & cbus_stat) {
				MHL_DEV_INFO("Set HPD True in %s",__func__);
				mhl_drive_hpd(TRUE);
			}
		}
 	}
    return;
 }






static int mhl_msm_read_rgnd_int(void)
{
    int ret = 0;
	unsigned long flags;
	uint8_t rgnd_imp;
	







	rgnd_imp = mhl_i2c_reg_read(TX_PAGE_3, 0x1C) & (BIT1 | BIT0);
	MHL_DEV_DBG("Imp Range read = %04x\n", (int)rgnd_imp);

	if (rgnd_imp == 0x02) {
		MHL_DEV_INFO("%s: MHL: MHL DEVICE!!!\n", __func__);
		if (timer_pending(&mhl_drv_data->discovery_timer))
			del_timer(&mhl_drv_data->discovery_timer);
		mutex_lock(&mhl_state_mutex);
		mhl_drv_data->notify_plugged = true;
		mutex_unlock(&mhl_state_mutex);
		wake_lock(&mhl_wake_lock);
		set_mhl_mode(true);

		/*
		 * Handling the MHL event in driver
		 */ 
		mhl_notify_plugged();














	} else {
		MHL_DEV_INFO("%s: MHL: NON-MHL DEVICE!!!\n", __func__);
		wake_unlock(&mhl_wake_lock);
		set_mhl_mode(false);

		
		mhl_i2c_reg_modify(TX_PAGE_3, 0x18, BIT3, BIT3);


	}
	complete_all(&mhl_drv_data->rgnd_done);
	
	spin_lock_irqsave(&mhl_drv_data->mhl_mode_lock, flags);
	ret =  mhl_drv_data->mhl_mode ?
		MHL_DISCOVERY_RESULT_MHL : MHL_DISCOVERY_RESULT_USB;
	spin_unlock_irqrestore(&mhl_drv_data->mhl_mode_lock, flags);

	return ret;
}


































int mhl_drive_all(int enable)
{
	MHL_DEV_INFO("%s: %s\n", __func__, enable ? "enable" : "disable");

	if (enable) {



			MHL_DEV_INFO("%s: enabling TMDS output..\n", __func__);
			
			mhl_drive_tmds(true);
			
			MHL_DEV_DBG("Set HPD True in %s",__func__);
			mhl_drive_hpd(true);



	} else {



			MHL_DEV_INFO("%s: disabling TMDS output..\n", __func__);
			
			MHL_DEV_DBG("Set HPD True in %s",__func__);
			mhl_drive_hpd(false);
			
			mhl_drive_tmds(false);



	}
	mhl_drv_data->full_operation = enable;

	return 0;
}
EXPORT_SYMBOL(mhl_drive_all);

static void mhl_msm_connection(void)
{

	mutex_lock(&mhl_state_mutex);
	MHL_DEV_DBG("%s: power state = [0x%x]\n", __func__,
			mhl_drv_data->cur_state);
	if (mhl_drv_data->cur_state == POWER_STATE_D0_NO_MHL) {
		mutex_unlock(&mhl_state_mutex);
		/* Already in D0 - MHL power state */
		return;
	}
	
	mhl_drv_data->cur_state = POWER_STATE_D0_NO_MHL;
	mutex_unlock(&mhl_state_mutex);
	

	mhl_i2c_reg_write(TX_PAGE_3, 0x30, 0x10);

	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x07, 0xF2);

	/*
	 * Keep the discovery enabled. Need RGND interrupt
	 * Possibly chip disables discovery after MHL_EST??
	 * Need to re-enable here
	 */
	mhl_i2c_reg_modify(TX_PAGE_3, 0x10, BIT0, BIT0);

	mhl_notify_online();

	return;
}

static void mhl_msm_disconnection(void)
{
	/*
	 * MHL TX CTL1
	 * Disabling Tx termination
	 */
	mhl_i2c_reg_write(TX_PAGE_3, 0x30, 0xD0);
	/* Change HPD line to drive it low */




	/* switch power state to D3 */
	switch_mode(POWER_STATE_D3);

	mhl_notify_offline();

	return;
}

static void force_usb_switch_open(void)
{
	
	mhl_i2c_reg_write(TX_PAGE_3, 0x10, 0x26);

	
	mhl_i2c_reg_modify(TX_PAGE_3, 0x15, BIT6, BIT6);

	/* Force HPD to 0 when not in Mobile HD mode. */
	mhl_i2c_reg_write(TX_PAGE_3, 0x12, 0x86);
}

static void release_usb_switch_open(void)
{
	msleep(50);
	mhl_i2c_reg_modify(TX_PAGE_3, 0x15, BIT6, 0x00);
	
	mhl_i2c_reg_modify(TX_PAGE_3, 0x10, BIT0, BIT0);
}

int mhl_notify_plugged(void)
{
	mhl_drv_data->mhl_online |= MHL_PLUGGED;

	
	queue_work(mhl_workqueue, &mhl_drv_data->usb_online_work);

	return 0;
}

int mhl_notify_unplugged(void)
{
	mhl_drv_data->mhl_online = 0;
	mhl_drv_data->hpd_state = 0;



	mhl_drv_data->devcap_state = 0;
	memset(&mhl_drv_data->state, 0, sizeof(struct mhl_state));

	
	queue_work(mhl_workqueue, &mhl_drv_data->usb_online_work);

	return 0;
}

static void mhl_enable_irq_timer_work(struct work_struct *w)
{
	mhl_drive_hpd(0);
	mhl_drive_hpd(1);
        enable_irq(mhl_drv_data->i2c_client->irq);
}

static void mhl_enable_irq_timer_work_2(struct work_struct *w)
{
        enable_irq(mhl_drv_data->i2c_client->irq);
	msleep(100); 
	disable_irq(mhl_drv_data->i2c_client->irq);
}
static void mhl_enable_irq_timer(unsigned long data)
{
       schedule_work(&mhl_drv_data->enable_irq_timer_work);
}

static void mhl_enable_irq_timer_2(unsigned long data)
{
       schedule_work(&enable_irq_timer_work_2);
}
static void mhl_discovery_timer_work(struct work_struct *w)
{
	mutex_lock(&mhl_state_mutex);
	if (mhl_drv_data->notify_plugged) {
		mhl_notify_unplugged();
		wake_unlock(&mhl_wake_lock);


	if (mhl_drv_data->charging_enable) {
		mhl_charging_flag = false;
		mhl_charging_mA = 0;
		mhl_drv_data->charging_enable(FALSE, 0);
		pm8921_regulate_input_voltage(4500);
	}

		del_timer(&mhl_drv_data->discovery_timer);
		mhl_drv_data->notify_plugged = false;
	}
	mutex_unlock(&mhl_state_mutex);
}

static void mhl_discovery_timer(unsigned long data)
{
	schedule_work(&mhl_drv_data->timer_work);
}

static void int_4_isr(void)
{
    uint8_t status;

    
	status = mhl_i2c_reg_read(TX_PAGE_3, 0x21);

    



   
    mutex_lock(&mhl_state_mutex);
    if ((0x00 == status) && (mhl_drv_data->cur_state == POWER_STATE_D3)) {
        MHL_DEV_WARN("MHL: spurious interrupt\n");
		mutex_unlock(&mhl_state_mutex);
        return;
    }
    mutex_unlock(&mhl_state_mutex);

	if (status != 0xFF && status != 0x00) {
		MHL_DEV_DBG("%s: %04x\n", __func__, status);
		
		if ((status & BIT0) && (mhl_drv_data->chip_rev_id < 1)) {
            uint8_t tmds_cstat;
            uint8_t mhl_fifo_status;
            
            tmds_cstat = mhl_i2c_reg_read(TX_PAGE_3, 0x40);
            MHL_DEV_DBG("TMDS CSTAT: 0x%04x\n", tmds_cstat);
            if (tmds_cstat & 0x02) {
                mhl_fifo_status = mhl_i2c_reg_read(TX_PAGE_3,
                        0x23);
                MHL_DEV_DBG("MHL FIFO status: 0x%04x\n",
                        mhl_fifo_status);
                if (mhl_fifo_status & 0x0C) {
                    mhl_i2c_reg_write(TX_PAGE_3, 0x23,
                            0x0C);

                    MHL_DEV_DBG("Apply MHL FIFO Reset\n");
                    mhl_i2c_reg_write(TX_PAGE_3, 0x00,
                            0x94);
                    mhl_i2c_reg_write(TX_PAGE_3, 0x00,
                            0x84);
                }
            }

        }

		if (status & BIT1)
            MHL_DEV_DBG("MHL: INT4 BIT1 is set that to mean 5V status has changed\n");

        
		if (status & BIT2) {
			MHL_DEV_INFO("%s : MHL_EST interrupt, mhl device has been detected\n", __func__);
			mhl_has_unplugged = false;
			set_mhl_mode(true);
			wake_lock(&mhl_wake_lock);
			mhl_connect_api(true);
			mhl_msm_connection();
			
			pm_obs_a_mhl(PM_OBS_MHL_MODE, TRUE);
			MHL_DEV_DBG("MHL Connect  Drv: INT4 Status = %04x\n",
					(int) status);
		}
		
		if (status & BIT3) {
			MHL_DEV_INFO("%s: MHL: USB-A type device detected.\n", __func__);
			
			mod_timer(&mhl_drv_data->discovery_timer,
					jiffies + 4*HZ/10);
			
			mhl_i2c_reg_write(TX_PAGE_3, 0x21, status);
			switch_mode(POWER_STATE_D3);
			return;
		}
		
		if (status & BIT5) {
			MHL_DEV_INFO("%s: VBUS_LOW mhl disconnect\n", __func__);
			mhl_has_unplugged = true;
			mhl_connect_api(false);
			mod_timer(&mhl_drv_data->discovery_timer,
				jiffies + 4*HZ/10);
			set_mhl_mode(false);
			/* Clear interrupts - REG INTR4 */
			mhl_i2c_reg_write(TX_PAGE_3, 0x21, status);
			mhl_msm_disconnection();
			
			pm_obs_a_mhl(PM_OBS_MHL_MODE, FALSE);
			wake_unlock(&mhl_wake_lock);
			return;
		}
        
        mutex_lock(&mhl_state_mutex);
        if (mhl_drv_data->cur_state == POWER_STATE_D3) {
            mutex_unlock(&mhl_state_mutex);
            if (status & BIT6) {
                
                switch_mode(POWER_STATE_D0_MHL);
				if (mhl_msm_read_rgnd_int()
					== MHL_DISCOVERY_RESULT_USB) {
					;
					mhl_i2c_reg_write(TX_PAGE_3, 0x1C, 0x80);
					
					
					mhl_i2c_reg_write(TX_PAGE_3, 0x21, status);
					switch_mode(POWER_STATE_D3);
					return;
				}
            }
        } else {
            mutex_unlock(&mhl_state_mutex);
        }

		
        
		mutex_lock(&mhl_state_mutex);
        if (mhl_drv_data->cur_state != POWER_STATE_D3) {
			mutex_unlock(&mhl_state_mutex);
            
            




            if (status & BIT4) {
                force_usb_switch_open();
                release_usb_switch_open();
            }
        } else {
            mutex_unlock(&mhl_state_mutex);
        }

	}
	
	mhl_i2c_reg_write(TX_PAGE_3, 0x21, status);
}

static void int_5_isr(void)
{
	/*
	 * Clear INT 5 ??
	 * Probably need to revisit this later
	 * INTR5 is related to FIFO underflow/overflow reset
	 * which is handled in 8334 by auto FIFO reset
	 */
	mhl_i2c_reg_modify(TX_PAGE_3, 0x23, 0x00, 0x00);
}

static void mhl_cbus_process_errors(uint8_t int_status)
{
	uint8_t abort_reason = 0;
	if (int_status & BIT2) {
		abort_reason = mhl_i2c_reg_read(TX_PAGE_CBUS, 0x0B);
		MHL_DEV_ERR("%s: CBUS DDC Abort Reason(0x%04x)\n",
			__func__, abort_reason);
	}
	if (int_status & BIT5) {
		abort_reason = mhl_i2c_reg_read(TX_PAGE_CBUS, 0x0D);
		MHL_DEV_ERR("%s: CBUS MSC Requestor Abort Reason(0x%04x)\n",
			__func__, abort_reason);
		mhl_i2c_reg_write(TX_PAGE_CBUS, 0x0D, 0xFF);
	}
	if (int_status & BIT6) {
		abort_reason = mhl_i2c_reg_read(TX_PAGE_CBUS, 0x0E);
		MHL_DEV_ERR("%s: CBUS MSC Responder Abort Reason(0x%04x)\n",
			__func__, abort_reason);
		mhl_i2c_reg_write(TX_PAGE_CBUS, 0x0E, 0xFF);
	}
}

/*
 * RCP, RAP messages - mandatory for compliance
 *
 */
static void mhl_cbus_isr(void)
{
	uint8_t regval;
	int req_done = false;
	uint8_t sub_cmd = 0xff;
	uint8_t cmd_data = 0xff;
	int msc_msg_recved = false;
	int rc = -1;

	regval  = mhl_i2c_reg_read(TX_PAGE_CBUS, 0x08);
	if (regval == 0xff)
		return;

	/* clear all interrupts that were raised even if we did not process */
	if (regval)
		mhl_i2c_reg_write(TX_PAGE_CBUS, 0x08, regval);

	MHL_DEV_DBG("%s: CBUS_INT = %04x\n", __func__, regval);

	/* MSC_MSG (RCP/RAP) */
	if (regval & BIT3) {
		sub_cmd = mhl_i2c_reg_read(TX_PAGE_CBUS, 0x18);
		cmd_data = mhl_i2c_reg_read(TX_PAGE_CBUS, 0x19);
		msc_msg_recved = true;
	}

	
	if (regval & (BIT6 | BIT5 | BIT2))
		mhl_cbus_process_errors(regval);

	/* MSC_REQ_DONE */
	if (regval & BIT4)
		req_done = true;

	/* Now look for interrupts on CBUS_MSC_INT2 */
	regval  = mhl_i2c_reg_read(TX_PAGE_CBUS, 0x1E);

	/* clear all interrupts that were raised */
	/* even if we did not process */
	if (regval) {
		mhl_i2c_reg_write(TX_PAGE_CBUS, 0x1E, regval);
		MHL_DEV_DBG("%s: CBUS_MSC_INT2 = %04x\n", __func__, regval);
	}
	/* received SET_INT */
	if (regval & BIT2) {
		uint8_t intr;

		intr = mhl_i2c_reg_read(TX_PAGE_CBUS, 0xA0);
		MHL_DEV_DBG("%s: MHL_INT_0 = %04x\n", __func__, intr);
		mhl_msc_recv_set_int(0, intr);

		intr = mhl_i2c_reg_read(TX_PAGE_CBUS, 0xA1);
		MHL_DEV_DBG("%s: MHL_INT_1 = %04x\n", __func__, intr);
		mhl_msc_recv_set_int(1, intr);

		mhl_i2c_reg_write(TX_PAGE_CBUS, 0xA0, 0xFF);
		mhl_i2c_reg_write(TX_PAGE_CBUS, 0xA1, 0xFF);
		mhl_i2c_reg_write(TX_PAGE_CBUS, 0xA2, 0xFF);
		mhl_i2c_reg_write(TX_PAGE_CBUS, 0xA3, 0xFF);
	}

	/* received WRITE_STAT */
	if (regval & BIT3) {
		uint8_t stat;

		stat = mhl_i2c_reg_read(TX_PAGE_CBUS, 0xB0);
		MHL_DEV_DBG("%s: MHL_STATUS_0 = %04x\n", __func__, stat);
		if (stat ^ mhl_drv_data->state.device_status[0])
			mhl_msc_recv_write_stat(0, stat);

		stat = mhl_i2c_reg_read(TX_PAGE_CBUS, 0xB1);
		MHL_DEV_DBG("%s: MHL_STATUS_1 = %04x\n", __func__, stat);
		if (stat ^ mhl_drv_data->state.device_status[1])
			mhl_msc_recv_write_stat(1, stat);

		mhl_i2c_reg_write(TX_PAGE_CBUS, 0xB0, 0xFF);
		mhl_i2c_reg_write(TX_PAGE_CBUS, 0xB1, 0xFF);
		mhl_i2c_reg_write(TX_PAGE_CBUS, 0xB2, 0xFF);
		mhl_i2c_reg_write(TX_PAGE_CBUS, 0xB3, 0xFF);
	}

	/* received MSC_MSG */
	if (msc_msg_recved) {
		rc = mhl_msc_rap_vs_rcp_process(sub_cmd, cmd_data);
		if (rc)
			MHL_DEV_ERR("MHL: mhl_msc_rap_vs_rcp_process failed(%d)!\n", rc);
	}

	
	if (req_done)
		complete_all(&mhl_drv_data->msc_command_done);
}

static irqreturn_t mhl_tx_isr(int irq, void *dev_id)
{
	/*
	 * Check RGND, MHL_EST, CBUS_LOCKOUT, SCDT
	 * interrupts. In D3, we get only RGND
	 */
	int_4_isr();

	mutex_lock(&mhl_state_mutex);
	if (mhl_drv_data->cur_state == POWER_STATE_D0_NO_MHL) {
		mutex_unlock(&mhl_state_mutex);
		MHL_DEV_DBG("MHL: got interrupt in POWER_STATE_D0_MHL state!\n");
		




		int_5_isr();

		




		mhl_cbus_isr();
		int_1_isr();
	} else if (mhl_drv_data->cur_state == POWER_STATE_D0_MHL) {
		mutex_unlock(&mhl_state_mutex);
	} else if (mhl_drv_data->cur_state == POWER_STATE_D3) {
		mutex_unlock(&mhl_state_mutex);
	} else {
		mutex_unlock(&mhl_state_mutex);
		MHL_DEV_ERR("MHL: got interrupt in invalid state!\n");
	}
	return IRQ_HANDLED;
}









static int mhl_drive_tmds(int to_state)
{
	MHL_DEV_DBG("%s E\n",__func__);
	mutex_lock(&mhl_state_mutex);
	if (mhl_drv_data->cur_state != POWER_STATE_D0_NO_MHL) {
		MHL_DEV_DBG("%s: power_state:%04x\n",
			__func__, mhl_drv_data->cur_state);
		mutex_unlock(&mhl_state_mutex);
		return -EFAULT;
	}
	mutex_unlock(&mhl_state_mutex);
	if (to_state == HPD_UP) {
		MHL_DEV_INFO("Set Tmds UP in %s",__func__);
		



		MHL_DEV_DBG("%s: Before TMDS Reg is %x \n", __func__,mhl_i2c_reg_read(TX_PAGE_L0, 0x80));
                if(mhl_diag_test_mode_on) {
			mhl_i2c_reg_modify(TX_PAGE_L0, 0x80, BIT4, 0x00);
		} else {
			mhl_i2c_reg_modify(TX_PAGE_L0, 0x80, BIT4, BIT4);
		}

		MHL_DEV_DBG("%s: enabled\n", __func__);
		MHL_DEV_DBG("%s: After TMDS Reg is %x \n", __func__,mhl_i2c_reg_read(TX_PAGE_L0, 0x80));

		MHL_DEV_DBG("Set HPD True in %s",__func__);
		mhl_drive_hpd(true);
	} else if (to_state == HPD_DOWN) {
		MHL_DEV_INFO("Set Tmds DOWN in %s",__func__);
		



		mhl_i2c_reg_modify(TX_PAGE_L0, 0x80, BIT4, 0x00);
	}
	MHL_DEV_DBG("%s X\n",__func__);
	return 0;
}









static int mhl_drive_hpd(uint8_t to_state)
{
	uint8_t regval;



	MHL_DEV_DBG("%s to_state is %d\n",__func__,to_state);
	mutex_lock(&mhl_state_mutex);
	if (mhl_drv_data->cur_state != POWER_STATE_D0_NO_MHL) {
		MHL_DEV_ERR("MHL: invalid state to ctrl HPD cur_state %x\n",mhl_drv_data->cur_state);
		mutex_unlock(&mhl_state_mutex);
		return -EFAULT;
	}
	mutex_unlock(&mhl_state_mutex);

	if (to_state == HPD_UP) {
		MHL_DEV_INFO("Set HPD UP in %s",__func__);
		





		mhl_i2c_reg_modify(TX_PAGE_3, 0x20, BIT4, 0x00);
		MHL_DEV_DBG("%s: enabled\n", __func__);

		
		regval = mhl_i2c_reg_read(TX_PAGE_CBUS, 0x0D);
    	MHL_DEV_DBG("%s hpd_ststus %x\n",__func__,regval);
		if (regval & BIT6)
			mhl_notify_hpd(true);
	} else if (to_state == HPD_DOWN) {
		MHL_DEV_INFO("Set HPD DOWN in %s",__func__);
		



		mhl_i2c_reg_modify(TX_PAGE_3, 0x20, BIT4 | BIT5, BIT4);
		mhl_notify_hpd(false);
		MHL_DEV_DBG("%s: disabled\n", __func__);
	}
	return 0;
}

bool mhl_is_connected(void)
{
	return true;
}






int mhl_device_discovery(const char *name, int *result)
{
	unsigned long flags;
	int timeout;
    	if (!name) {
		    MHL_DEV_ERR("%s: name is NULL\n", __func__);
        	return -EFAULT;
	}
    
	MHL_DEV_INFO("%s: start\n", __func__);
	mutex_lock(&mhl_state_mutex);
	if (mhl_drv_data->cur_state == POWER_STATE_D3) {
		mutex_unlock(&mhl_state_mutex);
		
		init_completion(&mhl_drv_data->rgnd_done);
		timeout = wait_for_completion_interruptible_timeout
			(&mhl_drv_data->rgnd_done, HZ/2);
		if (!timeout) {
			MHL_DEV_DBG("%s: RGND timed out!\n", __func__);
			
			
			MHL_DEV_DBG("Timedout Returning from discovery mode\n");
			*result = MHL_DISCOVERY_RESULT_USB;
			return 0;
		}
		spin_lock_irqsave(&mhl_drv_data->mhl_mode_lock, flags);
		*result = mhl_drv_data->mhl_mode ?
			MHL_DISCOVERY_RESULT_MHL : MHL_DISCOVERY_RESULT_USB;
		spin_unlock_irqrestore(&mhl_drv_data->mhl_mode_lock, flags);
	} else {
		mutex_unlock(&mhl_state_mutex);
		
		*result = MHL_DISCOVERY_RESULT_MHL;
	}
	MHL_DEV_INFO("%s: done\n", __func__);

    return 0;
}
EXPORT_SYMBOL(mhl_device_discovery);







int mhl_register_callback(const char *name, void (*callback)(int on))
{
    MHL_DEV_DBG("%s E\n", __func__);

    if (!callback)
        return -EINVAL;
    if (!notify_usb_online) {
        notify_usb_online = callback;
    } else {
        MHL_DEV_ERR("%s: usb & mhl callback function is already registered!\n", __func__);
        return -EFAULT;
    }
    MHL_DEV_DBG("%s X\n", __func__);
    return 0;
}
EXPORT_SYMBOL(mhl_register_callback);







int mhl_unregister_callback(const char *name)
{
    MHL_DEV_DBG("%s E\n", __func__);

    if (notify_usb_online) {
        notify_usb_online = NULL;
    } else {
        MHL_DEV_ERR("%s: usb & mhl callback function is already unregistered!\n", __func__);
        return -EFAULT;
	}

    MHL_DEV_DBG("%s X\n", __func__);
    return 0;
}
EXPORT_SYMBOL(mhl_unregister_callback);

static void cbus_reset(void)
{
	uint8_t i = 0;

	MHL_DEV_INFO("%s E\n",__func__);
	


	mhl_i2c_reg_modify(TX_PAGE_3, 0x00, BIT3, BIT3);
	msleep(20);
	mhl_i2c_reg_modify(TX_PAGE_3, 0x00, BIT3, 0x00);

	


	mhl_i2c_reg_write(TX_PAGE_L0, 0x75, (BIT6 | BIT5));
	mhl_i2c_reg_write(TX_PAGE_3, 0x22,
			(BIT0 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6));
	
	if (mhl_drv_data->chip_rev_id < 1) {
		mhl_i2c_reg_write(TX_PAGE_3, 0x24,
				(BIT4 | BIT3));
	} else {
		
		mhl_i2c_reg_write(TX_PAGE_3, 0x24, 0x00);
	}
	
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x09,
			(BIT6 | BIT5 | BIT4 | BIT3 | BIT2));
	
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x1F,
			(BIT3 | BIT2));
	for (i = 0; i < 4; i++) {

		



		mhl_i2c_reg_write(TX_PAGE_CBUS, (0xE0 + i), 0xFF);

		



		mhl_i2c_reg_write(TX_PAGE_CBUS, (0xF0 + i), 0xFF);
	}
	MHL_DEV_INFO("%s X\n",__func__);

}




static void init_cbus_regs(void)
{
	MHL_DEV_INFO("%s E\n",__func__);
	
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x07, 0xF2);
	
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x36, 0x0B);
	
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x39, 0x30);
	
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x40, 0x03);

	



	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x80 | DEVCAP_OFFSET_DEV_STATE,
			DEVCAP_VAL_DEV_STATE);
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x80 | DEVCAP_OFFSET_MHL_VERSION,
			DEVCAP_VAL_MHL_VERSION);
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x80 | DEVCAP_OFFSET_DEV_CAT,
			DEVCAP_VAL_DEV_CAT);  
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x80 | DEVCAP_OFFSET_ADOPTER_ID_H,
			DEVCAP_VAL_ADOPTER_ID_H); 
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x80 | DEVCAP_OFFSET_ADOPTER_ID_L,
			DEVCAP_VAL_ADOPTER_ID_L); 
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x80 | DEVCAP_OFFSET_VID_LINK_MODE,
			DEVCAP_VAL_VID_LINK_MODE);
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x80 | DEVCAP_OFFSET_AUD_LINK_MODE,
			DEVCAP_VAL_AUD_LINK_MODE);
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x80 | DEVCAP_OFFSET_VIDEO_TYPE,
			DEVCAP_VAL_VIDEO_TYPE);
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x80 | DEVCAP_OFFSET_LOG_DEV_MAP,
			DEVCAP_VAL_LOG_DEV_MAP); 
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x80 | DEVCAP_OFFSET_BANDWIDTH,
			DEVCAP_VAL_BANDWIDTH);
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x80 | DEVCAP_OFFSET_FEATURE_FLAG,
			DEVCAP_VAL_FEATURE_FLAG);
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x80 | DEVCAP_OFFSET_DEVICE_ID_H,
			DEVCAP_VAL_DEVICE_ID_H); 
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x80 | DEVCAP_OFFSET_DEVICE_ID_L,
			DEVCAP_VAL_DEVICE_ID_L);
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x80 | DEVCAP_OFFSET_SCRATCHPAD_SIZE,
			DEVCAP_VAL_SCRATCHPAD_SIZE);
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x80 | DEVCAP_OFFSET_INT_STAT_SIZE,
			DEVCAP_VAL_INT_STAT_SIZE);
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x80 | DEVCAP_OFFSET_RESERVED,
			DEVCAP_VAL_RESERVED);

	



	mhl_i2c_reg_modify(TX_PAGE_CBUS, 0x31, BIT2 | BIT3, BIT2 | BIT3);
	
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x22, 0x0F);
	
	mhl_i2c_reg_write(TX_PAGE_CBUS, 0x30, 0x01);
	
	mhl_i2c_reg_modify(TX_PAGE_CBUS, 0x2E, BIT4, BIT4);
	MHL_DEV_INFO("%s X\n",__func__);
}




static void mhl_init_reg_settings(bool mhl_disc_en)
{

	MHL_DEV_INFO("%s E\n",__func__);

	
	mhl_i2c_reg_write(TX_PAGE_L1, 0x3D, 0x3F);
	/*
	 * Wait for the source power to be enabled
	 * before enabling pll clocks.
	 */
	msleep(50);
	
	mhl_i2c_reg_write(TX_PAGE_2, 0x11, 0x01);
	
	mhl_i2c_reg_write(TX_PAGE_2, 0x12, 0x11);
	
	mhl_i2c_reg_write(TX_PAGE_3, 0x30, 0x10);
	
	mhl_i2c_reg_write(TX_PAGE_3, 0x35, 0xBC); 
	
	mhl_i2c_reg_write(TX_PAGE_3, 0x31, 0x3C);



    
    
    mhl_i2c_reg_write(TX_PAGE_3, 0x33, 0xDB);

	mhl_i2c_reg_write(TX_PAGE_3, 0x36, 0x03); 
	
	mhl_i2c_reg_write(TX_PAGE_3, 0x37, 0x0A); 
	




	
	mhl_i2c_reg_write(TX_PAGE_L0, 0x80, 0x08); 
	mhl_i2c_reg_write(TX_PAGE_L0, 0xF8, 0x8C); 
	mhl_i2c_reg_write(TX_PAGE_L0, 0x85, 0x02);
	mhl_i2c_reg_write(TX_PAGE_2, 0x00, 0x00);
	mhl_i2c_reg_write(TX_PAGE_2, 0x05, 0x07);  
	mhl_i2c_reg_write(TX_PAGE_2, 0x13, 0x60);
	
	mhl_i2c_reg_write(TX_PAGE_2, 0x17, 0x03);
	
	mhl_i2c_reg_write(TX_PAGE_2, 0x1A, 0x20);
	
	mhl_i2c_reg_write(TX_PAGE_2, 0x22, 0xE0);
	mhl_i2c_reg_write(TX_PAGE_2, 0x23, 0xC0);
	mhl_i2c_reg_write(TX_PAGE_2, 0x24, 0xA0);
	mhl_i2c_reg_write(TX_PAGE_2, 0x25, 0x80);
	mhl_i2c_reg_write(TX_PAGE_2, 0x26, 0x60);
	mhl_i2c_reg_write(TX_PAGE_2, 0x27, 0x40);
	mhl_i2c_reg_write(TX_PAGE_2, 0x28, 0x20);
	mhl_i2c_reg_write(TX_PAGE_2, 0x29, 0x00);
	
	mhl_i2c_reg_write(TX_PAGE_2, 0x31, 0x0A);
	
	mhl_i2c_reg_write(TX_PAGE_2, 0x45, 0x06);
	mhl_i2c_reg_write(TX_PAGE_2, 0x4B, 0x06);
	
	mhl_i2c_reg_write(TX_PAGE_2, 0x4C, 0x60); 
	mhl_i2c_reg_write(TX_PAGE_2, 0x4C, 0xE0);
	
	mhl_i2c_reg_write(TX_PAGE_2, 0x4D, 0x00);
	mhl_i2c_reg_write(TX_PAGE_L0, 0x08, 0x35);
	




	mhl_i2c_reg_write(TX_PAGE_3, 0x11, 0xAD);
	
	mhl_i2c_reg_write(TX_PAGE_3, 0x14, 0x57); 
	
	mhl_i2c_reg_write(TX_PAGE_3, 0x15, 0x11);
	
	mhl_i2c_reg_write(TX_PAGE_3, 0x17, 0x82);
	mhl_i2c_reg_write(TX_PAGE_3, 0x18, 0x24);
	
	mhl_i2c_reg_write(TX_PAGE_3, 0x13, 0x8C);
	
	if (mhl_disc_en)
		
		mhl_i2c_reg_write(TX_PAGE_3, 0x10, 0x27);
	else
		
		mhl_i2c_reg_write(TX_PAGE_3, 0x10, 0x26);
	mhl_i2c_reg_write(TX_PAGE_3, 0x16, 0x20);
	
	mhl_i2c_reg_write(TX_PAGE_3, 0x12, 0x86);
	
	mutex_lock(&mhl_state_mutex);
	if (mhl_drv_data->cur_state != POWER_STATE_D0_MHL){
		MHL_DEV_DBG("Set HPD False cur_state %x in %s",mhl_drv_data->cur_state,__func__);
		mhl_drv_data->cur_state = POWER_STATE_D0_NO_MHL;
		mutex_unlock(&mhl_state_mutex);
		MHL_DEV_DBG("Cur_state %x in %s",mhl_drv_data->cur_state,__func__);
		mhl_drive_hpd(false);
	} else {
		mutex_unlock(&mhl_state_mutex);
		MHL_DEV_DBG("Cur_state %x in %s",mhl_drv_data->cur_state,__func__);
	}

	


	
	mhl_i2c_reg_write(TX_PAGE_3, 0x00, 0x084);
	
	mhl_i2c_reg_write(TX_PAGE_L0, 0x0D, 0x1C);





























































































































	cbus_reset();

	init_cbus_regs();

	MHL_DEV_INFO("%s X\n",__func__);
}

static void mhl_chip_init(void)
{
	MHL_DEV_INFO("%s E\n",__func__);
	mhl_drv_data->pdata->mhl_chip_reset(true);

	mhl_drv_data->chip_rev_id = mhl_i2c_reg_read(TX_PAGE_L0, 0x04);

	MHL_DEV_INFO("%s: %04x%04x (%04x)\n",
		__func__,
		mhl_i2c_reg_read(TX_PAGE_L0, 0x03),
		mhl_i2c_reg_read(TX_PAGE_L0, 0x02),
		mhl_drv_data->chip_rev_id);

	mhl_init_reg_settings(true);

	
	switch_mode(POWER_STATE_D3);
	MHL_DEV_INFO("%s X\n",__func__);
}






static ssize_t mhl_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned long flags;
	char *name = NULL;
	int ret;

	spin_lock_irqsave(&mhl_drv_data->mhl_mode_lock, flags);
	ret = mhl_drv_data->mhl_mode ?
		MHL_DISCOVERY_RESULT_MHL : MHL_DISCOVERY_RESULT_USB;
	spin_unlock_irqrestore(&mhl_drv_data->mhl_mode_lock, flags);

	switch (ret) {	
		case MHL_DISCOVERY_RESULT_MHL: 
			name = "mhl\n";
			break;
		case MHL_DISCOVERY_RESULT_USB: 
			name = "usb\n";
			break;
		default:
			name = "error\n"; 
	}

	return snprintf(buf, strlen(name), "%s\n", name);
}





static ssize_t mhl_rcp_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	ssize_t len;
	uint8_t rcp_code;
	uint8_t is_support;
	
	
	len = strnlen(buf, PAGE_SIZE);
	
	is_support = mhl_drv_data->state.peer_devcap[MHL_DEV_FEATURE_FLAG_OFFSET];
	rcp_code = (uint8_t) atoi(buf);

	
	if (!(is_support & MHL_FEATURE_RCP_SUPPORT)) {
		MHL_DEV_ERR("%s:%d error !!!\n", __func__, __LINE__);	
		ret = -EFAULT;
		goto ERROR;
	}

	mhl_msc_send_msc_msg(MHL_MSC_MSG_RCP, rcp_code & 0x7f);

	return len;
ERROR:
	return ret;
}





static ssize_t mhl_rap_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	ssize_t len;
	uint8_t rap_code;
	uint8_t is_support;
	
	
	len = strnlen(buf, PAGE_SIZE);
	
	is_support = mhl_drv_data->state.peer_devcap[MHL_DEV_FEATURE_FLAG_OFFSET];
	rap_code = (uint8_t) atoi(buf);

	
	if (!(is_support & MHL_FEATURE_RAP_SUPPORT)) {
		MHL_DEV_ERR("%s:%d error !!!\n", __func__, __LINE__);	
		ret = -EFAULT;
		goto ERROR;
	}

	if ( rap_code == MHL_RAP_CONTENT_POLL ||
		 rap_code == MHL_RAP_CONTENT_ON ||
		 rap_code == MHL_RAP_CONTENT_OFF) {
		
		 mhl_msc_send_msc_msg(
				MHL_MSC_MSG_RAP,
				rap_code);
	} else {
		MHL_DEV_ERR("%s:%d error !!!\n", __func__, __LINE__);	
		ret = -EFAULT;
		goto ERROR;
	}

	return len;
ERROR:
	return ret;
}







static ssize_t mhl_adop_id_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	uint8_t adopter_id_h;
	uint8_t adopter_id_l;

	adopter_id_h = mhl_drv_data->state.peer_devcap[DEVCAP_OFFSET_ADOPTER_ID_H];
	adopter_id_l = mhl_drv_data->state.peer_devcap[DEVCAP_OFFSET_ADOPTER_ID_L];

	ret = snprintf(buf, PAGE_SIZE, "%x %x\n", adopter_id_h, adopter_id_l);

	return ret;
}







static ssize_t mhl_dev_id_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	uint8_t device_id_h;
	uint8_t device_id_l;

	device_id_h = mhl_drv_data->state.peer_devcap[DEVCAP_OFFSET_DEVICE_ID_H];
	device_id_l = mhl_drv_data->state.peer_devcap[DEVCAP_OFFSET_DEVICE_ID_H];

	ret = snprintf(buf, PAGE_SIZE, "%x %x\n", device_id_h, device_id_l);

	return ret;
}


static ssize_t mhl_output_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t len;
	int enable;
	
	
	len = strnlen(buf, PAGE_SIZE);
	
	enable = (uint8_t) atoi(buf);

	
	if (enable) {
		MHL_DEV_INFO("%s MHL full speed operation TRUE\n",__func__);
		
		mhl_drive_all(TRUE);
	} else {
		MHL_DEV_INFO("%s MHL full speed operation FALSE\n",__func__);
		
		mhl_drive_all(FALSE);
	}


	return len;
}



#define MSM_HDMI_SAMPLE_RATE_32KHZ		0
#define MSM_HDMI_SAMPLE_RATE_44_1KHZ		1
#define MSM_HDMI_SAMPLE_RATE_48KHZ		2
#define MSM_HDMI_SAMPLE_RATE_88_2KHZ		3
#define MSM_HDMI_SAMPLE_RATE_96KHZ		4
#define MSM_HDMI_SAMPLE_RATE_176_4KHZ		5
#define MSM_HDMI_SAMPLE_RATE_192KHZ		6
#define MSM_HDMI_SAMPLE_RATE_MAX		7
#define MSM_HDMI_SAMPLE_RATE_FORCE_32BIT	0x7FFFFFFF


static ssize_t mhl_hdmi_sample_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	uint32_t sample_code;
	unsigned int cur_sample_rate;
	
	cur_sample_rate=hdmi_msm_audio_get_sample_rate();
	
	switch (cur_sample_rate) {
	case MSM_HDMI_SAMPLE_RATE_48KHZ:
		sample_code = 48000;
		break;
	case MSM_HDMI_SAMPLE_RATE_44_1KHZ:
		sample_code = 44100;
		break;
	case MSM_HDMI_SAMPLE_RATE_32KHZ:
		sample_code = 32000;
		break;
	case MSM_HDMI_SAMPLE_RATE_88_2KHZ:
		sample_code = 88200;
		break;
	case MSM_HDMI_SAMPLE_RATE_96KHZ:
		sample_code = 96000;
		break;
	case MSM_HDMI_SAMPLE_RATE_176_4KHZ:
		sample_code = 176400;
		break;
	case MSM_HDMI_SAMPLE_RATE_192KHZ:
		sample_code = 192000;
		break;
	default:
		sample_code = 48000;
	}		
	ret = snprintf(buf, PAGE_SIZE, "%d\n", sample_code);

	return ret;
}



static ssize_t mhl_hdmi_sample_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret=0;
	ssize_t len;
	uint32_t sample_code;
	unsigned int cur_sample_rate;
	
	
	len = strnlen(buf, PAGE_SIZE);
	
	
	sample_code = (uint32_t) atoi(buf);	

	switch (sample_code) {
	case 48000:
		cur_sample_rate = MSM_HDMI_SAMPLE_RATE_48KHZ;
		break;
	case 44100:
		cur_sample_rate = MSM_HDMI_SAMPLE_RATE_44_1KHZ;
		break;
	case 32000:
		cur_sample_rate = MSM_HDMI_SAMPLE_RATE_32KHZ;
		break;
	case 88200:
		cur_sample_rate = MSM_HDMI_SAMPLE_RATE_88_2KHZ;
		break;
	case 96000:
		cur_sample_rate = MSM_HDMI_SAMPLE_RATE_96KHZ;
		break;
	case 176400:
		cur_sample_rate = MSM_HDMI_SAMPLE_RATE_176_4KHZ;
		break;
	case 192000:
		cur_sample_rate = MSM_HDMI_SAMPLE_RATE_192KHZ;
		break;
	default:
		cur_sample_rate = MSM_HDMI_SAMPLE_RATE_48KHZ;
	}
	if (cur_sample_rate != hdmi_msm_audio_get_sample_rate())
		hdmi_msm_audio_sample_rate_reset(cur_sample_rate);
	else
		pr_debug("Previous sample rate and current"
			"sample rate are same\n");

	return ret;
}


static ssize_t mhl_diag_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t len;
	uint32_t enable;
	
	
	len = strnlen(buf, PAGE_SIZE);
	
	
	enable = (uint32_t) atoi(buf);	

	if (enable) {
		mhl_diag_test_mode_on = true;
	} else {
		mhl_diag_test_mode_on = false;
	}

	return len;
}


static DEVICE_ATTR(mhl_adopter_id, 0440, mhl_adop_id_show, NULL);
static DEVICE_ATTR(mhl_device_id, 0440, mhl_dev_id_show, NULL);
static DEVICE_ATTR(mhl_state, 0440, mhl_state_show, NULL);
static DEVICE_ATTR(mhl_rap, 0220, NULL, mhl_rap_store);
static DEVICE_ATTR(mhl_rcp, 0220, NULL, mhl_rcp_store);
static DEVICE_ATTR(mhl_output, 0220, NULL, mhl_output_store);
static DEVICE_ATTR(mhl_hdmi_sample, 0660, mhl_hdmi_sample_show, mhl_hdmi_sample_store);
static DEVICE_ATTR(mhl_diag, 0220, NULL, mhl_diag_store);

static struct attribute *mhl_8334_fs_attrs[] = {
    &dev_attr_mhl_state.attr,
    &dev_attr_mhl_rcp.attr,
    &dev_attr_mhl_rap.attr,
    &dev_attr_mhl_device_id.attr,
    &dev_attr_mhl_adopter_id.attr,
    &dev_attr_mhl_output.attr,
    &dev_attr_mhl_hdmi_sample.attr,
    &dev_attr_mhl_diag.attr,
    NULL,
};
static struct attribute_group mhl_8334_fs_attr_group = {
        .attrs = mhl_8334_fs_attrs,
};






static int mhl_i2c_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
    int ret = -ENODEV;
	MHL_DEV_INFO("%s E\n", __func__);

	if (!client->dev.platform_data) {
		MHL_DEV_ERR("%s: invalid platform data\n", __func__);
		goto probe_exit;
	}

	mhl_drv_data->pdata = client->dev.platform_data;

	mhl_drv_data->i2c_client = client;

	if (mhl_drv_data->pdata->mhl_power_on) {
        ret = mhl_drv_data->pdata->mhl_power_on(true);
        if (ret) {
			MHL_DEV_ERR("%s: setup power failed\n", __func__);
			goto probe_exit;
		}
	} else
		MHL_DEV_WARN("%s: mhl_power_on not supported\n", __func__);
	if (mhl_drv_data->pdata->mhl_setup_gpio) {
        ret = mhl_drv_data->pdata->mhl_setup_gpio(true);
        if (ret) {
			MHL_DEV_ERR("%s: setup gpio failed\n", __func__);
			goto probe_gpio_error;
		}
	} else
		MHL_DEV_WARN("%s: mhl_setup_gpio not supported\n", __func__);

	 
	MHL_DEV_DBG("MHL: mhl_client->irq=[%d]\n", client->irq);
	ret = request_threaded_irq(client->irq, NULL,
			&mhl_tx_isr,
			IRQF_TRIGGER_LOW | IRQF_ONESHOT,
			"mhl_tx_isr", NULL);
	if (ret != 0) {
		MHL_DEV_ERR("request_threaded_irq failed, status: %d\n",
				ret);
		ret = -EACCES; 
		goto probe_irq_request_error;
	} else {
		MHL_DEV_INFO("request_threaded_irq succeeded\n");
		disable_irq(mhl_drv_data->i2c_client->irq);
	}

	mhl_chip_init();

	if (mhl_drv_data->pdata->charging_enable)
			 mhl_drv_data->charging_enable = mhl_drv_data->pdata->charging_enable;
	 else
			 MHL_DEV_ERR("%s: MHL charging not supported\n", __func__);

	init_timer(&mhl_drv_data->discovery_timer);
	mhl_drv_data->discovery_timer.function =
		mhl_discovery_timer;
	mhl_drv_data->discovery_timer.data = 0;
	mhl_drv_data->discovery_timer.expires = 0xffffffffL;
	add_timer(&mhl_drv_data->discovery_timer);

       init_timer(&mhl_drv_data->enable_irq_timer);
       mhl_drv_data->enable_irq_timer.function =
               mhl_enable_irq_timer;
       mhl_drv_data->enable_irq_timer.data = 0;
       mhl_drv_data->enable_irq_timer.expires = 0xffffffffL;
       add_timer(&mhl_drv_data->enable_irq_timer);
       INIT_WORK(&mhl_drv_data->enable_irq_timer_work, mhl_enable_irq_timer_work);
       mod_timer(&mhl_drv_data->enable_irq_timer, jiffies + 20*HZ);
	init_timer(&enable_irq_timer_2);
	enable_irq_timer_2.function = mhl_enable_irq_timer_2;
	enable_irq_timer_2.data = 0;
	enable_irq_timer_2.expires = 0xffffffffL;
	add_timer(&enable_irq_timer_2);
	INIT_WORK(&enable_irq_timer_work_2, mhl_enable_irq_timer_work_2);
	mod_timer(&enable_irq_timer_2, jiffies + HZ/2);
	INIT_LIST_HEAD(&mhl_drv_data->msc_queue);
	INIT_WORK(&mhl_drv_data->msc_command_work, mhl_msc_command_work);
	init_completion(&mhl_drv_data->rgnd_done);
	INIT_WORK(&mhl_drv_data->timer_work, mhl_discovery_timer_work);


    ret = sysfs_create_group(&client->dev.kobj,
            &mhl_8334_fs_attr_group);
    if (ret) {
        MHL_DEV_ERR("%s: sysfs group creation failed, rc=%d\n", __func__,
                ret);
        return ret;
    }


	MHL_DEV_INFO("%s X\n", __func__);
	return 0;

probe_irq_request_error:
	
	if (mhl_drv_data->pdata->mhl_setup_gpio) {
		mhl_drv_data->pdata->mhl_setup_gpio(false);
	}
probe_gpio_error:
	if (mhl_drv_data->pdata->mhl_power_on) {
		ret = mhl_drv_data->pdata->mhl_power_on(false);
	}
probe_exit:
	return ret;
}

static int mhl_i2c_remove(struct i2c_client *client)
{
	MHL_DEV_INFO("%s E\n", __func__);

	if (!client->dev.platform_data) {
		MHL_DEV_ERR("%s: client->dev.platform_data == NULL\n", __func__);
		return -EFAULT;
	}


    sysfs_remove_group(&client->dev.kobj,
            &mhl_8334_fs_attr_group);


	free_irq(mhl_drv_data->i2c_client->irq, NULL);

	if (mhl_drv_data->pdata->mhl_power_on)
		mhl_drv_data->pdata->mhl_power_on(false);

	if (mhl_drv_data->pdata->mhl_setup_gpio)
		mhl_drv_data->pdata->mhl_setup_gpio(false);

	MHL_DEV_DBG("%s X\n", __func__);
	
	return 0;
}





static int mhl_i2c_suspend(struct device *dev)
{
	MHL_DEV_DBG("%s \n",__func__);
	




	disable_irq(mhl_drv_data->i2c_client->irq);

	/*
	 * sencondly, flush all the pending works 
	 */
	flush_work_sync(&mhl_drv_data->timer_work);

	



	enable_irq_wake(mhl_drv_data->i2c_client->irq);


	return 0;
}




static int mhl_i2c_resume(struct device *dev)
{
	MHL_DEV_DBG("%s \n",__func__);
	


	disable_irq_wake(mhl_drv_data->i2c_client->irq);

	


	enable_irq(mhl_drv_data->i2c_client->irq);

	return 0;
}


static void mhl_usb_online_work(struct work_struct *work)
{

	if (notify_usb_online) {
		MHL_DEV_INFO("%s: mhl usb online(%d)\n",
			__func__, !!(mhl_drv_data->mhl_online & MHL_PLUGGED));
		notify_usb_online
			(!!(mhl_drv_data->mhl_online & MHL_PLUGGED));
	}
}

static void init_mhl_data(void) {
	mutex_lock(&mhl_state_mutex);

	mhl_drv_data->cur_state = POWER_STATE_D3;
	mutex_unlock(&mhl_state_mutex);
	spin_lock_init(&mhl_drv_data->mhl_mode_lock);
	return ;
}

static int __init mhl_i2c_init(void)
{
	int rc;

	MHL_DEV_INFO("%s E\n",__func__);
	mhl_drv_data = kzalloc(sizeof(*mhl_drv_data), GFP_KERNEL);
	if (!mhl_drv_data) {
		MHL_DEV_ERR("%s: out of memory!\n", __func__);
		return -ENOMEM;
	}

	init_mhl_data();

	rc = i2c_add_driver(&mhl_i2c_driver);
	if (rc) {
		MHL_DEV_ERR("%s: i2c_add_driver failed (%d)\n",
			__func__, rc);
		kfree(mhl_drv_data);
		goto init_out;
	}
    



    mhl_workqueue = create_workqueue("mhl_8334");
    INIT_WORK(&mhl_drv_data->usb_online_work, mhl_usb_online_work);
	msc_command_workqueue = create_singlethread_workqueue ("mhl_msc_cmd_queue");

	wake_lock_init(&mhl_wake_lock, WAKE_LOCK_SUSPEND, "mhl_wake_lock");

	MHL_DEV_INFO("%s X\n",__func__);
	return 0;

init_out:
	MHL_DEV_INFO("%s X ERR\n",__func__);
	return rc;
}

static void __exit mhl_i2c_exit(void)
{
	MHL_DEV_INFO("%s E\n",__func__);
	wake_lock_destroy(&mhl_wake_lock);
	i2c_del_driver(&mhl_i2c_driver);
	kfree(mhl_drv_data);
	MHL_DEV_INFO("%s X\n",__func__);
}

module_init(mhl_i2c_init);
module_exit(mhl_i2c_exit);

MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0");
MODULE_AUTHOR("NEC");
MODULE_DESCRIPTION("MHL 8334 Driver");


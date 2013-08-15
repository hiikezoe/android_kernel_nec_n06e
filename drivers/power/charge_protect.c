/* Copyright (C) 2012, NEC CASIO Mobile Communications. All rights reserved.
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






















#define pr_fmt(fmt)	"%s: " fmt, __func__

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include "charge_protect.h"

#include <linux/mfd/pm8xxx/pm8921-charger.h>





#define CHARGE_PROTECT_DEVICE_MAX_NUM	4















enum charge_protect_pm_state {
	CHARGE_PROTECT_PM_STATE_NORMAL = 0,
	CHARGE_PROTECT_PM_STATE_WARM,
	
	CHARGE_PROTECT_PM_STATE_WARM_1,
	CHARGE_PROTECT_PM_STATE_HOT_1,
	
	CHARGE_PROTECT_PM_STATE_HOT,
	CHARGE_PROTECT_PM_STATE_MAX
};

enum charge_protect_pm_recovery {
	CHARGE_PROTECT_PM_RECOVERY_NORMAL = 0,
	CHARGE_PROTECT_PM_RECOVERY_HIGH,
	CHARGE_PROTECT_PM_RECOVERY_MAX
};

struct charge_protect_pm_threshold {
	int high2;
	int high1;
	int high3;
	
	int high4;
	int high5;
	
};

struct charge_protect_data {
	enum charge_protect_state state;
	bool recovery;
	charge_protect_notifier notifier;
};

struct charge_protect_timer {
	bool					is_timeout;
	bool					is_timer;
	int					count;
	int					timer_chg_fc;
	int					err_cycle;
	spinlock_t				slock;
	struct delayed_work			timer_work;
};

struct charge_protect_info {
	struct charge_protect_operations	ops;
	spinlock_t				data_slock;
	struct charge_protect_data		data[CHARGE_PROTECT_TYPE_MAX];
	int					current_protect;
	bool					chg_start;
	bool					batt_temp_recovered;
	bool					pm_temp_recovered;
	pm_pw_ctrl_temp_err_type		batt_temp;
	pm_charge_off_on_xo_temp_cam_type	pm_temp_cam;
	pm_charge_off_on_xo_temp_lte_type	pm_temp_lte;
	pm_charge_off_on_xo_temp_mm_type	pm_temp_mm;
	
	pm_charge_off_on_xo_temp_you_type	pm_temp_you;
	pm_charge_off_on_xo_temp_wfd_type	pm_temp_wfd;
	pm_charge_off_on_xo_temp_mhl_type	pm_temp_mhl; 
	
	pm_charge_off_on_xo_temp_type		pm_temp;



	struct charge_protect_timer		timer;
};

static struct charge_protect_info cp_info;

static void charge_protect_timer_work(struct work_struct *work);

static int charge_protect_set_state(enum charge_protect_type type,
					enum charge_protect_state state)
{
	unsigned long flags;

	if (type < 0 || type >= CHARGE_PROTECT_TYPE_MAX) {
		pr_err("Invalid type=%d\n", type);
		return -1;
	}

	if (state < 0 || state >= CHARGE_PROTECT_STATE_MAX) {
		pr_err("Invalid state=%d\n", state);
		return -1;
	}

	spin_lock_irqsave(&cp_info.data_slock, flags);
	cp_info.data[type].state = state;
	spin_unlock_irqrestore(&cp_info.data_slock, flags);

	return 0;
}

int charge_protect_get_state(enum charge_protect_type type)
{
	unsigned long flags;
	int state;

	if (type < 0 || type >= CHARGE_PROTECT_TYPE_MAX) {
		pr_err("Invalid type=%d\n", type);
		return CHARGE_PROTECT_TYPE_NONE;
	}

	spin_lock_irqsave(&cp_info.data_slock, flags);
	state = cp_info.data[type].state;
	spin_unlock_irqrestore(&cp_info.data_slock, flags);

	return state;
}

static int charge_protect_set_recovery(enum charge_protect_type type, bool flag)
{
	unsigned long flags;

	if (type < 0 || type >= CHARGE_PROTECT_TYPE_MAX) {
		pr_err("Invalid type=%d\n", type);
		return -1;
	}

	spin_lock_irqsave(&cp_info.data_slock, flags);
	cp_info.data[type].recovery = flag;
	spin_unlock_irqrestore(&cp_info.data_slock, flags);

	return 0;
}

static bool charge_protect_get_recovery(enum charge_protect_type type)
{
	unsigned long flags;
	int flag;

	if (type < 0 || type >= CHARGE_PROTECT_TYPE_MAX) {
		pr_err("Invalid type=%d\n", type);
		return false;
	}

	spin_lock_irqsave(&cp_info.data_slock, flags);
	flag = cp_info.data[type].recovery;
	spin_unlock_irqrestore(&cp_info.data_slock, flags);

	return flag;
}

void charge_protect_init(struct pm_param_struct *pm_param)
{
	memset(&cp_info, 0, sizeof(cp_info));

	cp_info.current_protect = CHARGE_PROTECT_TYPE_NONE;
	cp_info.chg_start = true;
	cp_info.batt_temp_recovered = false;
	cp_info.pm_temp_recovered = false;
	spin_lock_init(&cp_info.data_slock);

	cp_info.timer.is_timeout = false;
	cp_info.timer.is_timer = false;
	cp_info.timer.count = 0;
	spin_lock_init(&cp_info.timer.slock);
	INIT_DELAYED_WORK(&cp_info.timer.timer_work, charge_protect_timer_work);

	charge_protect_set_param(pm_param);
}

void charge_protect_set_param(struct pm_param_struct *pm_param)
{
	unsigned long flags;

	memcpy(&cp_info.batt_temp, &pm_param->pm_pw_ctrl_temp_err,
						sizeof(cp_info.batt_temp));
	memcpy(&cp_info.pm_temp_cam, &pm_param->pm_charge_off_on_xo_temp_cam,
						sizeof(cp_info.pm_temp_cam));
	memcpy(&cp_info.pm_temp_lte, &pm_param->pm_charge_off_on_xo_temp_lte,
						sizeof(cp_info.pm_temp_lte));
	memcpy(&cp_info.pm_temp_mm, &pm_param->pm_charge_off_on_xo_temp_mm,
						sizeof(cp_info.pm_temp_mm));
	
	memcpy(&cp_info.pm_temp_you, &pm_param->pm_charge_off_on_xo_temp_you,
						sizeof(cp_info.pm_temp_you));
	memcpy(&cp_info.pm_temp_wfd, &pm_param->pm_charge_off_on_xo_temp_wfd,
						sizeof(cp_info.pm_temp_wfd));
	
	
	memcpy(&cp_info.pm_temp_mhl, &pm_param->pm_charge_off_on_xo_temp_mhl,
						sizeof(cp_info.pm_temp_mhl));
	
	memcpy(&cp_info.pm_temp, &pm_param->pm_charge_off_on_xo_temp,
						sizeof(cp_info.pm_temp));







	spin_lock_irqsave(&cp_info.timer.slock, flags);
	cp_info.timer.timer_chg_fc =
			pm_param->pm_chg_setting.data[PM_TIMER_CHG_FC];
	cp_info.timer.err_cycle =
			pm_param->pm_chg_setting.data[PM_BATTERY_ERR_CYCLE];
	spin_unlock_irqrestore(&cp_info.timer.slock, flags);
}

int charge_protect_register_notifier(enum charge_protect_type type,
					charge_protect_notifier notifier)
{
	if (type < 0 || type >= CHARGE_PROTECT_TYPE_MAX) {
		pr_err("Invalid type=%d\n", type);
		return -1;
	}

	cp_info.data[type].notifier = notifier;

	return 0;
}

void charge_protect_set_ops(struct charge_protect_operations *ops)
{
	memcpy(&cp_info.ops, ops, sizeof(cp_info.ops));
}


int charge_protect_check_batt_temp(int temp)
{
	int low1, low2, high1, high2;
	int ret;

	low1  = cp_info.batt_temp.data[PM_TEMP_ERR_LOW1];
	low2  = cp_info.batt_temp.data[PM_TEMP_ERR_LOW2];
	high1 = cp_info.batt_temp.data[PM_TEMP_ERR_HIGH1];
	high2 = cp_info.batt_temp.data[PM_TEMP_ERR_HIGH2];

	
	if (temp >= low1) {
		
		ret = CHARGE_PROTECT_BATT_STATE_COLD;
	} else if (temp <= high2) {
		
		ret = CHARGE_PROTECT_BATT_STATE_HOT;
	} else {
		if (temp <= high1) {
			
			ret = CHARGE_PROTECT_BATT_STATE_WARM;
		} else if ((temp > high1) && (temp < low2)) {
			
			ret = CHARGE_PROTECT_BATT_STATE_NORMAL;
		} else {
			
			ret = CHARGE_PROTECT_BATT_STATE_COOL;
		}
	}

	return ret;
}


static void charge_protect_get_pm_threshold(
			struct charge_protect_pm_threshold *pm_threshold)
{
	
	int i;
	int dc_state = 0;
	int device_count = 0;
	struct charge_protect_device_state device_state;
	struct charge_protect_pm_threshold temp_cam, temp_lte, temp_mm;
	struct charge_protect_pm_threshold temp_you, temp_wfd, temp;
	struct charge_protect_pm_threshold temp_mhl; 
	struct charge_protect_pm_threshold
				*comp_temp[CHARGE_PROTECT_DEVICE_MAX_NUM];
	

	memset(&device_state, 0, sizeof(device_state));

	cp_info.ops.get_device_state(&device_state);

	if (cp_info.ops.get_dc_state) {
		dc_state = cp_info.ops.get_dc_state();
	}

	if (dc_state == CHARGE_PROTECT_DC_STATE_CONNECT) {





















		
		temp_cam.high2 =
			cp_info.pm_temp_cam.data[PM_PM_TEMP_ERR_HIGH2_CAM_CRA];
		temp_cam.high1 =
			cp_info.pm_temp_cam.data[PM_PM_TEMP_ERR_HIGH1_CAM_CRA];
		temp_cam.high3 =
			cp_info.pm_temp_cam.data[PM_PM_TEMP_ERR_HIGH3_CAM_CRA];
		
		temp_cam.high4 =
			cp_info.pm_temp_cam.data[PM_PM_TEMP_ERR_HIGH4_CAM_CRA];
		temp_cam.high5 =
			cp_info.pm_temp_cam.data[PM_PM_TEMP_ERR_HIGH5_CAM_CRA];
		
		temp_lte.high2 =
			cp_info.pm_temp_lte.data[PM_PM_TEMP_ERR_HIGH2_LTE_CRA];
		temp_lte.high1 =
			cp_info.pm_temp_lte.data[PM_PM_TEMP_ERR_HIGH1_LTE_CRA];
		temp_lte.high3 =
			cp_info.pm_temp_lte.data[PM_PM_TEMP_ERR_HIGH3_LTE_CRA];
		
		temp_lte.high4 =
			cp_info.pm_temp_lte.data[PM_PM_TEMP_ERR_HIGH4_LTE_CRA];
		temp_lte.high5 =
			cp_info.pm_temp_lte.data[PM_PM_TEMP_ERR_HIGH5_LTE_CRA];
		
		temp_mm.high2 =
			cp_info.pm_temp_mm.data[PM_PM_TEMP_ERR_HIGH2_MM_CRA];
		temp_mm.high1 =
			cp_info.pm_temp_mm.data[PM_PM_TEMP_ERR_HIGH1_MM_CRA];
		temp_mm.high3 =
			cp_info.pm_temp_mm.data[PM_PM_TEMP_ERR_HIGH3_MM_CRA];
		
		
		temp_mm.high4 =
			cp_info.pm_temp_mm.data[PM_PM_TEMP_ERR_HIGH4_MM_CRA];
		temp_mm.high5 =
			cp_info.pm_temp_mm.data[PM_PM_TEMP_ERR_HIGH5_MM_CRA];
		
		temp_you.high2 =
			cp_info.pm_temp_you.data[PM_PM_TEMP_ERR_HIGH2_YOU_CRA];
		temp_you.high1 =
			cp_info.pm_temp_you.data[PM_PM_TEMP_ERR_HIGH1_YOU_CRA];
		temp_you.high3 =
			cp_info.pm_temp_you.data[PM_PM_TEMP_ERR_HIGH3_YOU_CRA];
		
		temp_you.high4 =
			cp_info.pm_temp_you.data[PM_PM_TEMP_ERR_HIGH4_YOU_CRA];
		temp_you.high5 =
			cp_info.pm_temp_you.data[PM_PM_TEMP_ERR_HIGH5_YOU_CRA];
		
		temp_wfd.high2 =
			cp_info.pm_temp_wfd.data[PM_PM_TEMP_ERR_HIGH2_WFD_CRA];
		temp_wfd.high1 =
			cp_info.pm_temp_wfd.data[PM_PM_TEMP_ERR_HIGH1_WFD_CRA];
		temp_wfd.high3 =
			cp_info.pm_temp_wfd.data[PM_PM_TEMP_ERR_HIGH3_WFD_CRA];
		
		temp_wfd.high4 =
			cp_info.pm_temp_wfd.data[PM_PM_TEMP_ERR_HIGH4_WFD_CRA];
		temp_wfd.high5 =
			cp_info.pm_temp_wfd.data[PM_PM_TEMP_ERR_HIGH5_WFD_CRA];
		
		
		
		temp_mhl.high2 =
			cp_info.pm_temp_mhl.data[PM_PM_TEMP_ERR_HIGH2_MHL_CRA];
		temp_mhl.high1 =
			cp_info.pm_temp_mhl.data[PM_PM_TEMP_ERR_HIGH1_MHL_CRA];
		temp_mhl.high3 =
			cp_info.pm_temp_mhl.data[PM_PM_TEMP_ERR_HIGH3_MHL_CRA];
		temp_mhl.high4 =
			cp_info.pm_temp_mhl.data[PM_PM_TEMP_ERR_HIGH4_MHL_CRA];
		temp_mhl.high5 =
			cp_info.pm_temp_mhl.data[PM_PM_TEMP_ERR_HIGH5_MHL_CRA];
		
		temp.high2 = cp_info.pm_temp.data[PM_PM_TEMP_ERR_HIGH2_CRA];
		temp.high1 = cp_info.pm_temp.data[PM_PM_TEMP_ERR_HIGH1_CRA];
		temp.high3 = cp_info.pm_temp.data[PM_PM_TEMP_ERR_HIGH3_CRA];
		
		temp.high4 = cp_info.pm_temp.data[PM_PM_TEMP_ERR_HIGH4_CRA];
		temp.high5 = cp_info.pm_temp.data[PM_PM_TEMP_ERR_HIGH5_CRA];
		


	} else {
		
		temp_cam.high2 =
			cp_info.pm_temp_cam.data[PM_PM_TEMP_ERR_HIGH2_CAM];
		temp_cam.high1 =
			cp_info.pm_temp_cam.data[PM_PM_TEMP_ERR_HIGH1_CAM];
		temp_cam.high3 =
			cp_info.pm_temp_cam.data[PM_PM_TEMP_ERR_HIGH3_CAM];
		
		temp_cam.high4 =
			cp_info.pm_temp_cam.data[PM_PM_TEMP_ERR_HIGH4_CAM];
		temp_cam.high5 =
			cp_info.pm_temp_cam.data[PM_PM_TEMP_ERR_HIGH5_CAM];
		
		temp_lte.high2 =
			cp_info.pm_temp_lte.data[PM_PM_TEMP_ERR_HIGH2_LTE];
		temp_lte.high1 =
			cp_info.pm_temp_lte.data[PM_PM_TEMP_ERR_HIGH1_LTE];
		temp_lte.high3 =
			cp_info.pm_temp_lte.data[PM_PM_TEMP_ERR_HIGH3_LTE];
		
		temp_lte.high4 =
			cp_info.pm_temp_lte.data[PM_PM_TEMP_ERR_HIGH4_LTE];
		temp_lte.high5 =
			cp_info.pm_temp_lte.data[PM_PM_TEMP_ERR_HIGH5_LTE];
		
		temp_mm.high2 =
			cp_info.pm_temp_mm.data[PM_PM_TEMP_ERR_HIGH2_MM];
		temp_mm.high1 =
			cp_info.pm_temp_mm.data[PM_PM_TEMP_ERR_HIGH1_MM];
		temp_mm.high3 =
			cp_info.pm_temp_mm.data[PM_PM_TEMP_ERR_HIGH3_MM];
		
		temp_mm.high4 =
			cp_info.pm_temp_mm.data[PM_PM_TEMP_ERR_HIGH4_MM];
		temp_mm.high5 =
		    cp_info.pm_temp_mm.data[PM_PM_TEMP_ERR_HIGH5_MM];
		
		
		temp_you.high2 =
			cp_info.pm_temp_you.data[PM_PM_TEMP_ERR_HIGH2_YOU];
		temp_you.high1 =
			cp_info.pm_temp_you.data[PM_PM_TEMP_ERR_HIGH1_YOU];
		temp_you.high3 =
			cp_info.pm_temp_you.data[PM_PM_TEMP_ERR_HIGH3_YOU];
		
		temp_you.high4 =
			cp_info.pm_temp_you.data[PM_PM_TEMP_ERR_HIGH4_YOU];
		temp_you.high5 =
			cp_info.pm_temp_you.data[PM_PM_TEMP_ERR_HIGH5_YOU];
		
		temp_wfd.high2 =
			cp_info.pm_temp_wfd.data[PM_PM_TEMP_ERR_HIGH2_WFD];
		temp_wfd.high1 =
			cp_info.pm_temp_wfd.data[PM_PM_TEMP_ERR_HIGH1_WFD];
		temp_wfd.high3 =
			cp_info.pm_temp_wfd.data[PM_PM_TEMP_ERR_HIGH3_WFD];
		
		temp_wfd.high4 =
			cp_info.pm_temp_wfd.data[PM_PM_TEMP_ERR_HIGH4_WFD];
		temp_wfd.high5 =
			cp_info.pm_temp_wfd.data[PM_PM_TEMP_ERR_HIGH5_WFD];
		
		
		
		temp_mhl.high2 =
			cp_info.pm_temp_mhl.data[PM_PM_TEMP_ERR_HIGH2_MHL];
		temp_mhl.high1 =
			cp_info.pm_temp_mhl.data[PM_PM_TEMP_ERR_HIGH1_MHL];
		temp_mhl.high3 =
			cp_info.pm_temp_mhl.data[PM_PM_TEMP_ERR_HIGH3_MHL];
		temp_mhl.high4 =
			cp_info.pm_temp_mhl.data[PM_PM_TEMP_ERR_HIGH4_MHL];
		temp_mhl.high5 =
			cp_info.pm_temp_mhl.data[PM_PM_TEMP_ERR_HIGH5_MHL];
		
		temp.high2 = cp_info.pm_temp.data[PM_PM_TEMP_ERR_HIGH2];
		temp.high1 = cp_info.pm_temp.data[PM_PM_TEMP_ERR_HIGH1];
		temp.high3 = cp_info.pm_temp.data[PM_PM_TEMP_ERR_HIGH3];
		
		temp.high4 = cp_info.pm_temp.data[PM_PM_TEMP_ERR_HIGH4];
		temp.high5 = cp_info.pm_temp.data[PM_PM_TEMP_ERR_HIGH5];
		
	}
	
	
















	
	if (device_state.youtube || device_state.wfd) {
		if (device_state.wfd) {
			comp_temp[device_count] = &temp_wfd;
		} else {
			comp_temp[device_count] = &temp_you;
		}
		device_count++;
	} else {
		if (device_state.camera) {
			comp_temp[device_count] = &temp_cam;
			device_count++;
		}
		if (device_state.lte) {
			comp_temp[device_count] = &temp_lte;
			device_count++;
		}
		if (device_state.multimedia) {
			comp_temp[device_count] = &temp_mm;
			device_count++;
		}
		
		if (device_state.mhl) {
			comp_temp[device_count] = &temp_mhl;
			device_count++;
		}
		
	}






	if (device_count > 0) {
		pm_threshold->high2 = comp_temp[0]->high2;
		pm_threshold->high1 = comp_temp[0]->high1;
		pm_threshold->high3 = comp_temp[0]->high3;
		
		pm_threshold->high4 = comp_temp[0]->high4;
		pm_threshold->high5 = comp_temp[0]->high5;
		

		
		for (i = 1; i < device_count; i++) {
			if (pm_threshold->high2 > comp_temp[i]->high2) {
				pm_threshold->high2 = comp_temp[i]->high2;
			}
			if (pm_threshold->high1 > comp_temp[i]->high1) {
				pm_threshold->high1 = comp_temp[i]->high1;
			}
			if (pm_threshold->high3 > comp_temp[i]->high3) {
				pm_threshold->high3 = comp_temp[i]->high3;
			}
			
			if (pm_threshold->high4 > comp_temp[i]->high4) {
				pm_threshold->high4 = comp_temp[i]->high4;
			}
			if (pm_threshold->high5 > comp_temp[i]->high5) {
				pm_threshold->high5 = comp_temp[i]->high5;
			
			}
		}
	} else {
		
		pm_threshold->high2 = temp.high2;
		pm_threshold->high1 = temp.high1;
		pm_threshold->high3 = temp.high3;
		
		pm_threshold->high4 = temp.high4;
		pm_threshold->high5 = temp.high5;
		
	}
	
}


static int charge_protect_pm_flag = 0;
int charge_protect_check_pm_temp_flag(void)
{
	return charge_protect_pm_flag;
}


static int charge_protect_check_pm_temp(int temp, int *recovery)
{
	struct charge_protect_pm_threshold pm_threshold;
	int ret;
	charge_protect_get_pm_threshold(&pm_threshold);














	if (temp >= pm_threshold.high2) {
		ret = CHARGE_PROTECT_PM_STATE_HOT;
	} else if (temp < pm_threshold.high1) {
		ret = CHARGE_PROTECT_PM_STATE_NORMAL;
	}
	else if ((temp >= pm_threshold.high5) && (temp < pm_threshold.high2))
	{
	    ret = CHARGE_PROTECT_PM_STATE_HOT_1;
	}
	else {
		if(temp < pm_threshold.high4){
			ret = CHARGE_PROTECT_PM_STATE_WARM;
		}
		else{
			ret = CHARGE_PROTECT_PM_STATE_WARM_1;
			}
	}
	charge_protect_pm_flag = ret;

	if (temp < pm_threshold.high3) {
		*recovery = CHARGE_PROTECT_PM_RECOVERY_NORMAL;
	} else {
		*recovery = CHARGE_PROTECT_PM_RECOVERY_HIGH;
	}
	
	return ret;
}

static void charge_protect_judge_batt(int batt_state, int pm_state)
{
	int current_state = CHARGE_PROTECT_TYPE_NONE;
	int batt_err;
	int batt_limit;

	
	batt_err = charge_protect_get_state(CHARGE_PROTECT_TYPE_BATT_TEMP_HOT);
	batt_limit =
		charge_protect_get_state(CHARGE_PROTECT_TYPE_BATT_TEMP_WARM);

	if (batt_err != CHARGE_PROTECT_STATE_NORMAL) {
		current_state = CHARGE_PROTECT_TYPE_BATT_TEMP_HOT;
	} else {
		if (batt_limit != CHARGE_PROTECT_STATE_NORMAL) {
			current_state = CHARGE_PROTECT_TYPE_BATT_TEMP_WARM;
		}
	}

	switch (current_state) {
	case CHARGE_PROTECT_TYPE_BATT_TEMP_HOT:
		
		switch (batt_state) {
		case CHARGE_PROTECT_BATT_STATE_NORMAL:
			if (pm_state == CHARGE_PROTECT_PM_STATE_NORMAL) {
				
				charge_protect_control(
					CHARGE_PROTECT_TYPE_BATT_TEMP_HOT,
					CHARGE_PROTECT_CTRL_OFF);
			} else {
				
				charge_protect_control(
					CHARGE_PROTECT_TYPE_BATT_TEMP_HOT,
					CHARGE_PROTECT_CTRL_FORCE_OFF);
				
				cp_info.batt_temp_recovered = true;
			}
			break;
		case CHARGE_PROTECT_BATT_STATE_HOT:
		case CHARGE_PROTECT_BATT_STATE_WARM:
		case CHARGE_PROTECT_BATT_STATE_COOL:
		case CHARGE_PROTECT_BATT_STATE_COLD:
			
			break;
		default:
			pr_err("Invalid battery state=%d\n", batt_state);
			break;
		}
		break;
	case CHARGE_PROTECT_TYPE_BATT_TEMP_WARM:
		
		switch (batt_state) {
		case CHARGE_PROTECT_BATT_STATE_NORMAL:
		case CHARGE_PROTECT_BATT_STATE_COOL:
			
			charge_protect_control(
				CHARGE_PROTECT_TYPE_BATT_TEMP_WARM,
				CHARGE_PROTECT_CTRL_OFF);
			break;
		case CHARGE_PROTECT_BATT_STATE_HOT:
		case CHARGE_PROTECT_BATT_STATE_COLD:
			
			charge_protect_control(
				CHARGE_PROTECT_TYPE_BATT_TEMP_HOT,
				CHARGE_PROTECT_CTRL_ON);
			charge_protect_control(
				CHARGE_PROTECT_TYPE_BATT_TEMP_WARM,
				CHARGE_PROTECT_CTRL_FORCE_OFF);
			break;
		case CHARGE_PROTECT_BATT_STATE_WARM:
			
			break;
		default:
			pr_err("Invalid battery state=%d\n", batt_state);
			break;
		}
		break;
	default:
		
		switch (batt_state) {
		case CHARGE_PROTECT_BATT_STATE_NORMAL:
		case CHARGE_PROTECT_BATT_STATE_COOL:
			
			break;
		case CHARGE_PROTECT_BATT_STATE_WARM:
		case CHARGE_PROTECT_BATT_STATE_HOT:
			
			
			if (cp_info.chg_start) {
				
				cp_info.batt_temp_recovered = false;
				charge_protect_control(
					CHARGE_PROTECT_TYPE_BATT_TEMP_HOT,
					CHARGE_PROTECT_CTRL_ON);
			} else {
				if (pm_state ==
					CHARGE_PROTECT_PM_STATE_NORMAL) {
					
					cp_info.batt_temp_recovered = false;
				}
				
				charge_protect_control(
					CHARGE_PROTECT_TYPE_BATT_TEMP_WARM,
					CHARGE_PROTECT_CTRL_ON);
			}
			
			break;
		case CHARGE_PROTECT_BATT_STATE_COLD:
			
			cp_info.batt_temp_recovered = false;
			charge_protect_control(
				CHARGE_PROTECT_TYPE_BATT_TEMP_HOT,
				CHARGE_PROTECT_CTRL_ON);
			break;
		default:
			pr_err("Invalid battery state=%d\n", batt_state);
			break;
		}
		cp_info.chg_start = false;
		break;
	}
}


static void charge_protect_judge_pm_warm(int batt_state, int pm_recovery)
{
	if (pm_recovery == CHARGE_PROTECT_PM_RECOVERY_HIGH) {
		if (batt_state != CHARGE_PROTECT_BATT_STATE_NORMAL &&
		    cp_info.pm_temp_recovered) {
			
			charge_protect_control(
				CHARGE_PROTECT_TYPE_PM_TEMP_HOT,
				CHARGE_PROTECT_CTRL_FORCE_ON);
			charge_protect_control(
				CHARGE_PROTECT_TYPE_PM_TEMP_WARM,
				CHARGE_PROTECT_CTRL_FORCE_OFF);
		}
		
		cp_info.pm_temp_recovered = false;
	}
}


static void charge_protect_judge_pm(int batt_state, int pm_state,
							int pm_recovery)
{
	int current_state = CHARGE_PROTECT_TYPE_NONE;
	int pm_err;
	int pm_limit;
	int flag; 
         static int hot_1_flag = 0;
	flag = charge_protect_check_pm_temp_flag(); 

	
	pm_err = charge_protect_get_state(CHARGE_PROTECT_TYPE_PM_TEMP_HOT);
	pm_limit = charge_protect_get_state(CHARGE_PROTECT_TYPE_PM_TEMP_WARM);
	if (pm_err != CHARGE_PROTECT_STATE_NORMAL) {



		if(pm_state == CHARGE_PROTECT_PM_STATE_HOT_1)
			current_state = CHARGE_PROTECT_TYPE_PM_TEMP_HOT_1;
		else
		{
			current_state = CHARGE_PROTECT_TYPE_PM_TEMP_HOT;
		}

	} else {
		if (pm_limit != CHARGE_PROTECT_STATE_NORMAL) {
			current_state = CHARGE_PROTECT_TYPE_PM_TEMP_WARM;
		}
	}
	switch (current_state) {

	case CHARGE_PROTECT_TYPE_PM_TEMP_HOT_1:
	    
		
			
			
		
			
			
	     if (pm_recovery == CHARGE_PROTECT_PM_RECOVERY_NORMAL) {
		hot_1_flag = 1;
		if (batt_state ==CHARGE_PROTECT_BATT_STATE_NORMAL) {
			charge_protect_control(CHARGE_PROTECT_TYPE_PM_TEMP_HOT,CHARGE_PROTECT_CTRL_OFF);
		} else {
			charge_protect_control(CHARGE_PROTECT_TYPE_PM_TEMP_HOT,CHARGE_PROTECT_CTRL_FORCE_OFF);
			cp_info.pm_temp_recovered = true;
		}
	     }
		break;

        case CHARGE_PROTECT_TYPE_PM_TEMP_HOT:
		
		switch (pm_state) {
		case CHARGE_PROTECT_PM_STATE_NORMAL:
		case CHARGE_PROTECT_PM_STATE_HOT:
		case CHARGE_PROTECT_PM_STATE_WARM:
		case CHARGE_PROTECT_PM_STATE_WARM_1:
		
		
		
		
			if (pm_recovery == CHARGE_PROTECT_PM_RECOVERY_NORMAL) {
				if (batt_state ==
					CHARGE_PROTECT_BATT_STATE_NORMAL) {
					
					charge_protect_control(
						CHARGE_PROTECT_TYPE_PM_TEMP_HOT,
						CHARGE_PROTECT_CTRL_OFF);
				} else {
					
					charge_protect_control(
						CHARGE_PROTECT_TYPE_PM_TEMP_HOT,
						CHARGE_PROTECT_CTRL_FORCE_OFF);
					
					cp_info.pm_temp_recovered = true;
				}
			}
			break;










		default:
			pr_err("Invalid PM state=%d\n", pm_state);
			break;
		}
		break;
	case CHARGE_PROTECT_TYPE_PM_TEMP_WARM:
		
		switch (pm_state) {
		case CHARGE_PROTECT_PM_STATE_NORMAL:
			
			charge_protect_control(
				CHARGE_PROTECT_TYPE_PM_TEMP_WARM,
				CHARGE_PROTECT_CTRL_OFF);
			break;
		case CHARGE_PROTECT_PM_STATE_HOT:
			
			charge_protect_control(
				CHARGE_PROTECT_TYPE_PM_TEMP_HOT,
				CHARGE_PROTECT_CTRL_ON);
			charge_protect_control(
				CHARGE_PROTECT_TYPE_PM_TEMP_WARM,
				CHARGE_PROTECT_CTRL_FORCE_OFF);
			cp_info.pm_temp_recovered = false;
			break;
		
        case CHARGE_PROTECT_PM_STATE_HOT_1:
              charge_protect_control(
                                CHARGE_PROTECT_TYPE_PM_TEMP_HOT_1,
                                CHARGE_PROTECT_CTRL_ON);
              charge_protect_control(
                                CHARGE_PROTECT_TYPE_PM_TEMP_WARM,
                                CHARGE_PROTECT_CTRL_FORCE_OFF);
              cp_info.pm_temp_recovered = false;
              break;
		
		case CHARGE_PROTECT_PM_STATE_WARM:
		case CHARGE_PROTECT_PM_STATE_WARM_1: 
			if (cp_info.batt_temp_recovered &&
			    pm_recovery == CHARGE_PROTECT_PM_RECOVERY_NORMAL) {
				
				charge_protect_control(
					CHARGE_PROTECT_TYPE_PM_TEMP_WARM,
					CHARGE_PROTECT_CTRL_OFF);
			} else {
				
				charge_protect_judge_pm_warm(batt_state,
								pm_recovery);
				
			}
			break;
		default:
			pr_err("Invalid PM state=%d\n", pm_state);
			break;
		}
		break;
	default:
		
		switch (pm_state) {
		case CHARGE_PROTECT_PM_STATE_NORMAL:
			
			
		   
		   if(!pm8921_is_batfet_closed())
		   {
			   nc_pm8921_charger_enable(true);
		   }
		   charge_protect_control(
		   CHARGE_PROTECT_TYPE_PM_TEMP_HOT_1,
		   CHARGE_PROTECT_CTRL_FORCE_OFF);
		   
			break;
		case CHARGE_PROTECT_PM_STATE_WARM:
		case CHARGE_PROTECT_PM_STATE_WARM_1: 
			
			
			charge_protect_control(
				CHARGE_PROTECT_TYPE_PM_TEMP_HOT,
				CHARGE_PROTECT_CTRL_OFF);
			
			charge_protect_control(
				CHARGE_PROTECT_TYPE_PM_TEMP_WARM,
				CHARGE_PROTECT_CTRL_ON);
			
			charge_protect_control(
				CHARGE_PROTECT_TYPE_PM_TEMP_HOT_1,
				CHARGE_PROTECT_CTRL_FORCE_OFF);
			
			
			charge_protect_judge_pm_warm(batt_state, pm_recovery);
			
			break;
		
		case CHARGE_PROTECT_PM_STATE_HOT_1:
			if(hot_1_flag == 1)
			{
				charge_protect_control(
				CHARGE_PROTECT_TYPE_PM_TEMP_WARM,
				CHARGE_PROTECT_CTRL_ON);
				hot_1_flag = 0;
			}
			else{
			charge_protect_control(
				CHARGE_PROTECT_TYPE_PM_TEMP_HOT_1,
				CHARGE_PROTECT_CTRL_ON);
			}
			cp_info.pm_temp_recovered = false;
			break;
		
		case CHARGE_PROTECT_PM_STATE_HOT:
			
			charge_protect_control(
				CHARGE_PROTECT_TYPE_PM_TEMP_HOT,
				CHARGE_PROTECT_CTRL_ON);
			
			charge_protect_control(
			CHARGE_PROTECT_TYPE_PM_TEMP_HOT_1,
			CHARGE_PROTECT_CTRL_FORCE_OFF);
			
			cp_info.pm_temp_recovered = false;
			break;
		default:
			pr_err("Invalid PM state=%d\n", pm_state);
			break;
		}
		break;
	}
}

void charge_protect_check_temp(void)
{
	int batt_state;
	int pm_state;
	int pm_recovery;
	int batt_temp_mv;
	int pm_temp;
	if (!cp_info.ops.get_battery_temperature_uv ||
	    !cp_info.ops.get_pmic_temperature) {
		return;
	}

	batt_temp_mv = (int)cp_info.ops.get_battery_temperature_uv() / 1000;
	pm_temp = cp_info.ops.get_pmic_temperature();

	batt_state = charge_protect_check_batt_temp(batt_temp_mv);

	pm_state = charge_protect_check_pm_temp(pm_temp, &pm_recovery);

	charge_protect_judge_batt(batt_state, pm_state);

	charge_protect_judge_pm(batt_state, pm_state, pm_recovery);
}

void charge_protect_check_usb_ovp(void)
{
	int dc_state = CHARGE_PROTECT_DC_STATE_DISCONNECT;
	int usb_state = CHARGE_PROTECT_USB_STATE_DISCONNECT;

	if (cp_info.ops.get_dc_state) {
		dc_state = cp_info.ops.get_dc_state();
	}










	
	if (cp_info.ops.get_usb_state) {
		usb_state = cp_info.ops.get_usb_state();
	}

	
	switch (cp_info.current_protect) {
	case CHARGE_PROTECT_TYPE_USB_OVP:
		
		if (usb_state != CHARGE_PROTECT_USB_STATE_OVP) {
			
			charge_protect_control(
				CHARGE_PROTECT_TYPE_USB_OVP,
				CHARGE_PROTECT_CTRL_OFF);
		}
		break;
	default:
		if (usb_state == CHARGE_PROTECT_USB_STATE_OVP) {
			
			
			charge_protect_control(
				CHARGE_PROTECT_TYPE_USB_OVP,
				CHARGE_PROTECT_CTRL_ON);
		}
		break;
	}
}

void charge_protect_check_dc_ovp(void)
{
	int dc_state = CHARGE_PROTECT_DC_STATE_DISCONNECT;
	int usb_state = CHARGE_PROTECT_USB_STATE_DISCONNECT;

	if (cp_info.ops.get_usb_state) {
		usb_state = cp_info.ops.get_usb_state();
	}










	
	if (cp_info.ops.get_dc_state) {
		dc_state = cp_info.ops.get_dc_state();
	}

	switch (cp_info.current_protect) {
	case CHARGE_PROTECT_TYPE_DC_OVP:
		
		if (dc_state != CHARGE_PROTECT_DC_STATE_OVP) {
			
			charge_protect_control(
				CHARGE_PROTECT_TYPE_DC_OVP,
				CHARGE_PROTECT_CTRL_OFF);
		}
		break;
	default:
		if (dc_state == CHARGE_PROTECT_DC_STATE_OVP) {
			
			charge_protect_control(
				CHARGE_PROTECT_TYPE_DC_OVP,
				CHARGE_PROTECT_CTRL_ON);
		}
		break;
	}
}


bool charge_protect_is_ovp(void)
{
	bool ret = false;
	int dc_state = CHARGE_PROTECT_DC_STATE_DISCONNECT;
	int usb_state = CHARGE_PROTECT_USB_STATE_DISCONNECT;

	if (cp_info.ops.get_dc_state) {
		dc_state = cp_info.ops.get_dc_state();
	}

	if (cp_info.ops.get_usb_state) {
		usb_state = cp_info.ops.get_usb_state();
	}

	if (dc_state == CHARGE_PROTECT_DC_STATE_OVP) {
		
		if (usb_state == CHARGE_PROTECT_USB_STATE_OVP ||
		    usb_state == CHARGE_PROTECT_USB_STATE_DISCONNECT) {
			ret = true;
		}
	}
	if (usb_state == CHARGE_PROTECT_USB_STATE_OVP) {
		
		if (dc_state == CHARGE_PROTECT_DC_STATE_OVP ||
		    dc_state == CHARGE_PROTECT_DC_STATE_DISCONNECT) {
			ret = true;
		}
	}

	return ret;
}


int charge_protect_control(enum charge_protect_type type,
				enum charge_protect_control control)
{
	int state = 0;

	if (type < 0 || type >= CHARGE_PROTECT_TYPE_MAX) {
		pr_err("Invalid type=%d\n", type);
		return -1;
	}

	switch (control) {
	case CHARGE_PROTECT_CTRL_ON:
		state = charge_protect_get_state(type);
		if (state == CHARGE_PROTECT_STATE_NORMAL) {
			charge_protect_set_state(type,
					CHARGE_PROTECT_STATE_NEED_NOTIFY);
		}
		break;
	
	case CHARGE_PROTECT_CTRL_FORCE_ON:
		charge_protect_set_state(type, CHARGE_PROTECT_STATE_NOTIFIED);
		if (cp_info.current_protect > type) {
			
			cp_info.current_protect = type;
		}
		break;
	
	case CHARGE_PROTECT_CTRL_FORCE_OFF:
		charge_protect_set_state(type, CHARGE_PROTECT_STATE_NORMAL);
		break;
	case CHARGE_PROTECT_CTRL_OFF:
		state = charge_protect_get_state(type);
		if (state != CHARGE_PROTECT_STATE_NORMAL) {
			charge_protect_set_recovery(type, true);
		}
		break;
	case CHARGE_PROTECT_CTRL_OFF_RECOVERED:
		break;
	default:
		pr_err("Invalid control=%d\n", control);
		return -1;
	}

	return 0;
}

static void charge_protect_reset(void)
{
	int type;

	for (type = 0; type < CHARGE_PROTECT_TYPE_MAX; type++) {
		charge_protect_set_state(type, CHARGE_PROTECT_STATE_NORMAL);
		charge_protect_set_recovery(type, false);
	}

	
	cp_info.current_protect = CHARGE_PROTECT_TYPE_NONE;
	cp_info.chg_start = true;
	cp_info.batt_temp_recovered = false;
	cp_info.pm_temp_recovered = false;

	cp_info.timer.is_timeout = false;
	cp_info.timer.is_timer = false;
	cp_info.timer.count = 0;
	
}

void charge_protect_charger_remove(void)
{
	int dc_state = CHARGE_PROTECT_DC_STATE_DISCONNECT;
	int usb_state = CHARGE_PROTECT_USB_STATE_DISCONNECT;

	if (cp_info.ops.get_dc_state) {
		dc_state = cp_info.ops.get_dc_state();
	}
	if (cp_info.ops.get_usb_state) {
		usb_state = cp_info.ops.get_usb_state();
	}

	switch (cp_info.current_protect) {
	case CHARGE_PROTECT_TYPE_NONE:
		if (usb_state == CHARGE_PROTECT_USB_STATE_DISCONNECT) {
			cp_info.chg_start = true;
		}
		break;
	default:
		if (dc_state  == CHARGE_PROTECT_DC_STATE_DISCONNECT &&
		    usb_state == CHARGE_PROTECT_USB_STATE_DISCONNECT) {
			
			charge_protect_reset();
		}
		break;
	}
}

bool charge_protect_is_temp_recovered(void)
{
	return (cp_info.batt_temp_recovered || cp_info.pm_temp_recovered);
}

void charge_protect_check_state(void)
{
	int type;
	bool recovery;
	int state;
	int ret;
	int control;

	for (type = 0; type < CHARGE_PROTECT_TYPE_MAX; type++) {
		recovery = charge_protect_get_recovery(type);
		if (recovery) {
			
			ret = 0;
			control = CHARGE_PROTECT_CTRL_OFF;

			switch (type) {
			case CHARGE_PROTECT_TYPE_BATT_TEMP_WARM:
				
				if (cp_info.pm_temp_recovered) {
					control =
					    CHARGE_PROTECT_CTRL_OFF_RECOVERED;
					cp_info.pm_temp_recovered = false;
				}
				break;
			case CHARGE_PROTECT_TYPE_PM_TEMP_WARM:
				
				if (cp_info.batt_temp_recovered) {
					control =
					    CHARGE_PROTECT_CTRL_OFF_RECOVERED;
					cp_info.batt_temp_recovered = false;
				}
				break;
			case CHARGE_PROTECT_TYPE_BATT_TEMP_HOT:
				cp_info.pm_temp_recovered = false;
				break;
			case CHARGE_PROTECT_TYPE_PM_TEMP_HOT:
				cp_info.batt_temp_recovered = false;
				break;
			
			case CHARGE_PROTECT_TYPE_PM_TEMP_HOT_1:
				cp_info.batt_temp_recovered = false;
				break;
			
			}

			if (cp_info.data[type].notifier) {
				ret = cp_info.data[type].notifier(control);
			}

			charge_protect_set_recovery(type, false);

			if (!ret) {
				charge_protect_set_state(type,
						CHARGE_PROTECT_STATE_NORMAL);

				if (cp_info.current_protect == type) {
					
					cp_info.current_protect =
						CHARGE_PROTECT_TYPE_NONE;
				}
			}
		}

		state = charge_protect_get_state(type);
		switch (state) {
		case CHARGE_PROTECT_STATE_NEED_NOTIFY:
			
			if (cp_info.data[type].notifier) {
				ret = cp_info.data[type].notifier(
							CHARGE_PROTECT_CTRL_ON);

				if (!ret) {
					
					charge_protect_set_state(type,
						CHARGE_PROTECT_STATE_NOTIFIED);
				}
			}

			if (cp_info.current_protect ==
				CHARGE_PROTECT_TYPE_NONE) {
				
				cp_info.current_protect = type;
			} else {
				if (cp_info.current_protect > type) {
					
					cp_info.current_protect = type;
				}
			}
			break;
		case CHARGE_PROTECT_STATE_NOTIFIED:
			
			if (cp_info.current_protect ==
				CHARGE_PROTECT_TYPE_NONE) {
				
				cp_info.current_protect = type;
			}
			break;
		case CHARGE_PROTECT_STATE_NORMAL:
			
			if (cp_info.current_protect == type) {
				
				cp_info.current_protect =
						CHARGE_PROTECT_TYPE_NONE;
			}
			break;
		default:
			break;
		}
	}
}

int charge_protect_get_current_protect(void)
{
	return cp_info.current_protect;
}

static void charge_protect_timer_work(struct work_struct *work)
{
	unsigned long flags;

	spin_lock_irqsave(&cp_info.timer.slock, flags);
	cp_info.timer.count++;
	if (cp_info.timer.count >= cp_info.timer.err_cycle) {
		
		pr_info("CHARGE_PROTECT_TYPE_TIMEOUT is on\n");
		cp_info.timer.is_timeout = true;
		cp_info.timer.count      = 0;
		cp_info.timer.is_timer   = false;
		charge_protect_control(CHARGE_PROTECT_TYPE_TIMEOUT,
						CHARGE_PROTECT_CTRL_ON);
	} else {
		pr_info("timer count=%d.\n", cp_info.timer.count);
		schedule_delayed_work(&cp_info.timer.timer_work,
			round_jiffies_relative(msecs_to_jiffies(
				cp_info.timer.timer_chg_fc * 60 * 1000)));
	}
	spin_unlock_irqrestore(&cp_info.timer.slock, flags);
}

void charge_protect_timer_enable(bool enable)
{
	unsigned long flags;
	
	int state;
	

	spin_lock_irqsave(&cp_info.timer.slock, flags);
	if (enable) {
		if (!cp_info.timer.is_timer && !cp_info.timer.is_timeout) {
			pr_debug("Charge protection timer start\n");

			cp_info.timer.count      = 0;
			cp_info.timer.is_timer   = true;
			cp_info.timer.is_timeout = false;

			schedule_delayed_work(&cp_info.timer.timer_work,
				round_jiffies_relative(msecs_to_jiffies(
				cp_info.timer.timer_chg_fc * 60 * 1000)));
		}
	} else {
		
		if (cp_info.timer.is_timer) {
			pr_debug("Charge protection timer canceled\n");
			cancel_delayed_work(&cp_info.timer.timer_work);
		}

		state = charge_protect_get_state(CHARGE_PROTECT_TYPE_TIMEOUT);
		if (state == CHARGE_PROTECT_STATE_NORMAL) {
			pr_debug("Charge protection timer reset\n");
			cp_info.timer.count      = 0;
			cp_info.timer.is_timer   = false;
			cp_info.timer.is_timeout = false;
		}
		
	}
	spin_unlock_irqrestore(&cp_info.timer.slock, flags);
}

bool charge_protect_timer_is_timeout(void)
{
	unsigned long flags;
	bool timeout;

	spin_lock_irqsave(&cp_info.timer.slock, flags);
	timeout = cp_info.timer.is_timeout;
	spin_unlock_irqrestore(&cp_info.timer.slock, flags);

	return timeout;
}

void charge_protect_timer_dc_check(void)
{
	unsigned long flags;
	bool timer_set = true;
	int dc_state = CHARGE_PROTECT_DC_STATE_DISCONNECT;
	int usb_state = CHARGE_PROTECT_USB_STATE_DISCONNECT;

	if (cp_info.ops.get_dc_state) {
		dc_state = cp_info.ops.get_dc_state();
	}
	if (cp_info.ops.get_usb_state) {
		usb_state = cp_info.ops.get_usb_state();
	}

	pr_debug("usb=%d, dc=%d\n", usb_state, dc_state);

	spin_lock_irqsave(&cp_info.timer.slock, flags);
	if (cp_info.timer.is_timeout &&
	    usb_state == CHARGE_PROTECT_USB_STATE_CONNECT) {
		



		pr_debug("It's timeout, but usb is connected\n");
		timer_set = false;
	}
	spin_unlock_irqrestore(&cp_info.timer.slock, flags);

	if (timer_set) {
		
		charge_protect_control(CHARGE_PROTECT_TYPE_TIMEOUT,
						CHARGE_PROTECT_CTRL_OFF);
		charge_protect_check_state();
		charge_protect_timer_enable(false);

		
		if (dc_state == CHARGE_PROTECT_DC_STATE_CONNECT ||
		    usb_state == CHARGE_PROTECT_USB_STATE_CONNECT) {
			if (cp_info.ops.is_charging &&
			    cp_info.ops.is_charging()) {
				charge_protect_timer_enable(true);
			}
		}
	}
}

void charge_protect_timer_usb_check(void)
{
	static int prev_usb_state = CHARGE_PROTECT_USB_STATE_DISCONNECT;
	int dc_state = CHARGE_PROTECT_DC_STATE_DISCONNECT;
	int usb_state = CHARGE_PROTECT_USB_STATE_DISCONNECT;

	if (cp_info.ops.get_dc_state) {
		dc_state = cp_info.ops.get_dc_state();
	}
	if (cp_info.ops.get_usb_state) {
		usb_state = cp_info.ops.get_usb_state();
	}

	pr_debug("cur_usb=%d, prev_usb=%d, dc=%d\n",
					usb_state, prev_usb_state, dc_state);

	if (dc_state == CHARGE_PROTECT_DC_STATE_DISCONNECT) {
		
		if ((usb_state == CHARGE_PROTECT_USB_STATE_CONNECT &&
		     prev_usb_state == CHARGE_PROTECT_USB_STATE_DISCONNECT) ||
		     usb_state == CHARGE_PROTECT_USB_STATE_DISCONNECT) {
			
			charge_protect_control(
				CHARGE_PROTECT_TYPE_TIMEOUT,
				CHARGE_PROTECT_CTRL_OFF);
			charge_protect_check_state();
			charge_protect_timer_enable(false);
		}

		
		if (usb_state == CHARGE_PROTECT_USB_STATE_CONNECT) {
			if (cp_info.ops.is_charging &&
			    cp_info.ops.is_charging()) {
				charge_protect_timer_enable(true);
			}
		}
	} else {
		
		pr_debug("Charge protection timer is continued\n");
	}

	prev_usb_state = usb_state;
}

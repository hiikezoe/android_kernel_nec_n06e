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

















#if !defined(__CHARGE_PROTECT_H__)
#define __CHARGE_PROTECT_H__

#include <linux/types.h>
#include <linux/pm_param.h>

enum charge_protect_type {
	CHARGE_PROTECT_TYPE_NONE = -1,

	CHARGE_PROTECT_TYPE_TIMEOUT = 0,
	CHARGE_PROTECT_TYPE_DC_OVP,
	CHARGE_PROTECT_TYPE_USB_OVP,
	CHARGE_PROTECT_TYPE_BATT_TEMP_HOT,
	CHARGE_PROTECT_TYPE_PM_TEMP_HOT,
	CHARGE_PROTECT_TYPE_BATT_TEMP_WARM,
	CHARGE_PROTECT_TYPE_PM_TEMP_WARM,
	CHARGE_PROTECT_TYPE_PM_TEMP_HOT_1,
	CHARGE_PROTECT_TYPE_MAX
};

enum charge_protect_state {
	CHARGE_PROTECT_STATE_NORMAL = 0,
	CHARGE_PROTECT_STATE_NEED_NOTIFY,
	CHARGE_PROTECT_STATE_NOTIFIED,
	CHARGE_PROTECT_STATE_MAX
};

enum charge_protect_control {
	CHARGE_PROTECT_CTRL_ON = 0,
	CHARGE_PROTECT_CTRL_OFF,
	CHARGE_PROTECT_CTRL_FORCE_OFF,
	CHARGE_PROTECT_CTRL_OFF_RECOVERED,
	
	CHARGE_PROTECT_CTRL_FORCE_ON,
	
	CHARGE_PROTECT_CTRL_MAX
};

enum charge_protect_dc_state {
	CHARGE_PROTECT_DC_STATE_DISCONNECT = 0,
	CHARGE_PROTECT_DC_STATE_CONNECT,
	CHARGE_PROTECT_DC_STATE_OVP,
	CHARGE_PROTECT_DC_STATE_MAX
};

enum charge_protect_usb_state {
	CHARGE_PROTECT_USB_STATE_DISCONNECT = 0,
	CHARGE_PROTECT_USB_STATE_CONNECT,
	CHARGE_PROTECT_USB_STATE_OVP,
	CHARGE_PROTECT_USB_STATE_MAX
};













enum charge_protect_batt_state {
	CHARGE_PROTECT_BATT_STATE_NORMAL = 0,
	CHARGE_PROTECT_BATT_STATE_WARM,
	CHARGE_PROTECT_BATT_STATE_COOL,
	CHARGE_PROTECT_BATT_STATE_HOT,
	CHARGE_PROTECT_BATT_STATE_COLD,
	CHARGE_PROTECT_BATT_STATE_MAX
};


struct charge_protect_device_state {
	bool camera;
	bool lte;
	bool multimedia;
	
	bool youtube;
	bool wfd;
	
	bool mhl; 
};

struct charge_protect_operations {
	int (*get_usb_state)(void);
	int (*get_dc_state)(void);
	bool (*is_charging)(void);
	int (*get_device_state)(struct charge_protect_device_state *state);
	int (*get_battery_temperature_uv)(void);
	int (*get_pmic_temperature)(void);
};

typedef int (*charge_protect_notifier)(enum charge_protect_control control);

extern void charge_protect_init(struct pm_param_struct *pm_param);
extern void charge_protect_set_param(struct pm_param_struct *pm_param);
extern int charge_protect_register_notifier(enum charge_protect_type type,
					charge_protect_notifier notifier);
extern void charge_protect_set_ops(struct charge_protect_operations *ops);


extern int charge_protect_check_batt_temp(int temp);

extern void charge_protect_check_temp(void);
extern int charge_protect_check_pm_temp_flag(void); 
extern void charge_protect_check_usb_ovp(void);
extern void charge_protect_check_dc_ovp(void);

extern bool charge_protect_is_ovp(void);

extern int charge_protect_control(enum charge_protect_type type,
				enum charge_protect_control control);
extern void charge_protect_charger_remove(void);
extern bool charge_protect_is_temp_recovered(void);
extern int charge_protect_get_state(enum charge_protect_type type);
extern void charge_protect_check_state(void);
extern int charge_protect_get_current_protect(void);

extern void charge_protect_timer_enable(bool enable);
extern bool charge_protect_timer_is_timeout(void);
extern void charge_protect_timer_dc_check(void);
extern void charge_protect_timer_usb_check(void);

#endif 

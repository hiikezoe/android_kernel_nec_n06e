/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
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

#ifndef __MHL_MSM_H__
#define __MHL_MSM_H__



#include <linux/types.h>
#include <linux/platform_device.h>

#include <linux/i2c.h>

#include <mach/board.h>

#include <linux/device.h>

#include <linux/mhl_devcap.h>
#include <linux/mhl_defs.h>


#if !defined(DEV_DBG_PREFIX)
#define DEV_DBG_PREFIX "MHL SII8334 : "
#endif
#if defined(DEBUG)
#define MHL_DEV_DBG(args...)    pr_debug(DEV_DBG_PREFIX args)
#else
#define MHL_DEV_DBG(args...)    (void)0
#endif 
#define MHL_DEV_INFO(args...)   pr_info(DEV_DBG_PREFIX args);
#define MHL_DEV_WARN(args...)   pr_warn(DEV_DBG_PREFIX args)
#define MHL_DEV_ERR(args...)    pr_err(DEV_DBG_PREFIX args)


#define MHL_DEVICE_NAME "sii8334"
#define MHL_DRIVER_NAME "sii8334"

#define HPD_UP               1
#define HPD_DOWN             0

enum discovery_result_enum {
	MHL_DISCOVERY_RESULT_USB = 0,
	MHL_DISCOVERY_RESULT_MHL,
};





extern int mhl_device_discovery(const char *name, int *result);

extern int mhl_register_callback
	(const char *name, void (*callback)(int online));

extern int mhl_unregister_callback(const char *name);



















struct mhl_platform_data {
	int (*mhl_chip_reset)(int enable);
	int (*mhl_power_on)(int enable);
	int (*mhl_setup_gpio)(int enable);

	int (*charging_enable)(int enable, int max_curr);

};













enum {
	TX_PAGE_TPI          = 0x00, 
	TX_PAGE_L0           = 0x01, 
	TX_PAGE_L1           = 0x02, 
	TX_PAGE_2            = 0x03, 
	TX_PAGE_3            = 0x04, 
	TX_PAGE_CBUS         = 0x05, 
	TX_PAGE_DDC_EDID     = 0x06, 
	TX_PAGE_DDC_SEGM     = 0x07, 
};

enum mhl_st_type {
	POWER_STATE_D0_NO_MHL = 0,
	POWER_STATE_D0_MHL    = 2,
	POWER_STATE_D3        = 3,
};

enum {
	DEV_PAGE_TPI_0      = (0x72),
	DEV_PAGE_TX_L0_0    = (0x72),
	DEV_PAGE_TPI_1      = (0x76),
	DEV_PAGE_TX_L0_1    = (0x76),
	DEV_PAGE_TX_L1_0    = (0x7A),
	DEV_PAGE_TX_L1_1    = (0x7E),
	DEV_PAGE_TX_2_0     = (0x92),
	DEV_PAGE_TX_2_1     = (0x96),
	DEV_PAGE_TX_3_0	    = (0x9A),
	DEV_PAGE_TX_3_1	    = (0x9E),
	DEV_PAGE_CBUS       = (0xC8),
	DEV_PAGE_DDC_EDID   = (0xA0),
	DEV_PAGE_DDC_SEGM   = (0x60),
};


#define POWER_ON	0
#define POWER_OFF   1
#define POWER_ERROR -1




struct mhl_state {
	uint8_t device_status[2];
	uint8_t peer_status[2];
	uint8_t peer_devcap[16];
};

#define MHL_OFFLINE			0
#define MHL_PLUGGED			BIT0
#define MHL_LINK_EST		BIT1
#define MHL_DCAP_OK			BIT2
#define MHL_ONLINE			(MHL_PLUGGED | MHL_LINK_EST | MHL_DCAP_OK)

#define MSC_COMMAND_QUEUE_SIZE 80
#define MSC_NORMAL_SEND	0
#define MSC_PRIOR_SEND	1




typedef struct {
	uint8_t reqstatus;   
	uint8_t retrycount;
	uint8_t command;	
	uint8_t offsetdata;      
	uint8_t length; 
	union {   
		uint8_t msgdata[16];   
		unsigned char *pdatabytes;            
	}payload_u;
} cbus_req_t;

struct mhl_event {
	struct list_head msc_queue;
	cbus_req_t msc_command_queue;
};

extern int mhl_drive_all(int enable);
extern void set_mhl_mode(int flag);

extern bool mhl_is_charge(void);
extern int get_mhl_charging_current(void);


struct msc_cmd_envelope {
	


	struct list_head msc_queue_envelope;
	cbus_req_t msc_cmd_msg;
};




















struct mhl_msm_state_t {
	int irq; 
	struct device dev;
	struct i2c_client *i2c_client;

	struct mhl_platform_data *pdata;

	int full_operation;
	
	uint8_t cur_state;

	struct mhl_state state;
	unsigned int mhl_online;
	unsigned int hpd_state;
	unsigned int tmds_state;
	uint16_t devcap_state;
	
	uint8_t chip_rev_id;

	
	struct completion rgnd_done;
	int mhl_mode;
	spinlock_t mhl_mode_lock;
	struct timer_list discovery_timer;
	struct timer_list enable_irq_timer;
	int notify_plugged;

	struct work_struct msc_command_work;
	int msc_command_counter;
	struct list_head msc_queue;
	
	struct completion msc_command_done;
	
	struct completion req_write_done;

	
	struct work_struct usb_online_work;

	struct work_struct timer_work;
	struct work_struct enable_irq_timer_work;

	int (*charging_enable)(int enable, int max_curr);

};	




#endif /* __MHL_MSM_H__ */

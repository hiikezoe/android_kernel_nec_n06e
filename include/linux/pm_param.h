/* Copyright (C) 2011, NEC CASIO Mobile Communications. All rights reserved.  
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#if !defined(__PM_PARAM_H__)
#define __PM_PARAM_H__
























#include <linux/types.h>






enum {
	PM_RTC_ADJ = 0,
	PM_PW_CTRL_RTCADJ_I_MAX
};


enum {
	PM_TEMP_ERR_LOW1 = 0,
	PM_TEMP_ERR_LOW2,
	PM_TEMP_ERR_HIGH1,
	PM_TEMP_ERR_HIGH2,
	PM_PW_CTRL_TEMP_ERR_I_MAX
};








enum {
	 
	PM_PM_TEMP_ERR_HIGH2_CAM = 0,
	PM_PM_TEMP_ERR_HIGH1_CAM,
	PM_PM_TEMP_ERR_HIGH3_CAM,
	
	PM_PM_TEMP_ERR_HIGH4_CAM,
	PM_PM_TEMP_ERR_HIGH5_CAM,
	
	 
	
	PM_PM_TEMP_ERR_HIGH2_CAM_CRA,
	PM_PM_TEMP_ERR_HIGH1_CAM_CRA,
	PM_PM_TEMP_ERR_HIGH3_CAM_CRA,
	
	PM_PM_TEMP_ERR_HIGH4_CAM_CRA,
	PM_PM_TEMP_ERR_HIGH5_CAM_CRA,
	
	
	PM_CHARGE_OFF_ON_XO_TEMP_CAM_I_MAX
};





enum {
	 
	PM_PM_TEMP_ERR_HIGH2_LTE = 0,
	PM_PM_TEMP_ERR_HIGH1_LTE,
	PM_PM_TEMP_ERR_HIGH3_LTE,
	
	PM_PM_TEMP_ERR_HIGH4_LTE,
	PM_PM_TEMP_ERR_HIGH5_LTE,
	
	 
	
	PM_PM_TEMP_ERR_HIGH2_LTE_CRA,
	PM_PM_TEMP_ERR_HIGH1_LTE_CRA,
	PM_PM_TEMP_ERR_HIGH3_LTE_CRA,
	
	PM_PM_TEMP_ERR_HIGH4_LTE_CRA,
	PM_PM_TEMP_ERR_HIGH5_LTE_CRA,
	
	
	PM_CHARGE_OFF_ON_XO_TEMP_LTE_I_MAX
};


enum {
	 
	PM_PM_TEMP_ERR_HIGH2 = 0,
	PM_PM_TEMP_ERR_HIGH1,
	PM_PM_TEMP_ERR_HIGH3,
	
	PM_PM_TEMP_ERR_HIGH4,
	PM_PM_TEMP_ERR_HIGH5,
	
	 
	
	PM_PM_TEMP_ERR_HIGH2_CRA,
	PM_PM_TEMP_ERR_HIGH1_CRA,
	PM_PM_TEMP_ERR_HIGH3_CRA,
	
	PM_PM_TEMP_ERR_HIGH4_CRA,
	PM_PM_TEMP_ERR_HIGH5_CRA,
	
	
	PM_CHARGE_OFF_ON_XO_TEMP_I_MAX
};






enum {
	PM_PM_TEMP_ERR_HIGH2_MM = 0,
	PM_PM_TEMP_ERR_HIGH1_MM,
	PM_PM_TEMP_ERR_HIGH3_MM,
	
	PM_PM_TEMP_ERR_HIGH4_MM,
	PM_PM_TEMP_ERR_HIGH5_MM,
	
	
	PM_PM_TEMP_ERR_HIGH2_MM_CRA,
	PM_PM_TEMP_ERR_HIGH1_MM_CRA,
	PM_PM_TEMP_ERR_HIGH3_MM_CRA,
	
	PM_PM_TEMP_ERR_HIGH4_MM_CRA,
	PM_PM_TEMP_ERR_HIGH5_MM_CRA,
	
	
	PM_CHARGE_OFF_ON_XO_TEMP_MM_I_MAX
};








enum {
	PM_PM_TEMP_ERR_HIGH2_YOU = 0,
	PM_PM_TEMP_ERR_HIGH1_YOU,
	PM_PM_TEMP_ERR_HIGH3_YOU,
	
	PM_PM_TEMP_ERR_HIGH4_YOU,
	PM_PM_TEMP_ERR_HIGH5_YOU,
	
	PM_PM_TEMP_ERR_HIGH2_YOU_CRA,
	PM_PM_TEMP_ERR_HIGH1_YOU_CRA,
	PM_PM_TEMP_ERR_HIGH3_YOU_CRA,
	
	PM_PM_TEMP_ERR_HIGH4_YOU_CRA,
	PM_PM_TEMP_ERR_HIGH5_YOU_CRA,
	
	PM_CHARGE_OFF_ON_XO_TEMP_YOU_I_MAX
};





enum {
	PM_PM_TEMP_ERR_HIGH2_WFD = 0,
	PM_PM_TEMP_ERR_HIGH1_WFD,
	PM_PM_TEMP_ERR_HIGH3_WFD,
	
	PM_PM_TEMP_ERR_HIGH4_WFD,
	PM_PM_TEMP_ERR_HIGH5_WFD,
	
	PM_PM_TEMP_ERR_HIGH2_WFD_CRA,
	PM_PM_TEMP_ERR_HIGH1_WFD_CRA,
	PM_PM_TEMP_ERR_HIGH3_WFD_CRA,
	
	PM_PM_TEMP_ERR_HIGH4_WFD_CRA,
	PM_PM_TEMP_ERR_HIGH5_WFD_CRA,
	
	PM_CHARGE_OFF_ON_XO_TEMP_WFD_I_MAX
};



enum {
	PM_PM_TEMP_ERR_HIGH2_MHL = 0,
	PM_PM_TEMP_ERR_HIGH1_MHL,
	PM_PM_TEMP_ERR_HIGH3_MHL,
	PM_PM_TEMP_ERR_HIGH4_MHL,
	PM_PM_TEMP_ERR_HIGH5_MHL,
	PM_PM_TEMP_ERR_HIGH2_MHL_CRA,
	PM_PM_TEMP_ERR_HIGH1_MHL_CRA,
	PM_PM_TEMP_ERR_HIGH3_MHL_CRA,
	PM_PM_TEMP_ERR_HIGH4_MHL_CRA,
	PM_PM_TEMP_ERR_HIGH5_MHL_CRA,
	PM_CHARGE_OFF_ON_XO_TEMP_MHL_I_MAX
};


enum {
	PM_HEADSET_NOISE_THR = 0,
	PM_CHARGE_HEADSET_NOISE_I_MAX
};


enum {
	PM_PM_TEMP_ERR_NCONT_CHARGE = 0,

	PM_PM_TEMP_ERR_NCONT_CHARGE_YOU,
	PM_PM_TEMP_ERR_NCONT_CHARGE_MM,

	PM_CHARGE_OFF_PMIC_NCONT_I_MAX
};



enum {
	
	PM_I_CHG_HIGH1 = 0,
	PM_I_CHG_HIGH2,
	PM_I_CHG_HIGH3, 
	PM_V_CHG_HIGH1,
	PM_V_CHG_HIGH2,
	PM_V_CHG_VBATDET,
	PM_I_CHG_COMP,
	PM_TIMER_CHG_FC,
	PM_BATTERY_ERR_CYCLE,
	
	
	CHG_COMP_CAP,
	CHG_RESTART_CAP,
	
	PM_CHG_SETTING_I_MAX
};




enum {
	PM_BAT_ALM_ON_OFF = 0,
	PM_DET_VOLT_UP_LOW,
	PM_CLOCK_DIVIDER,
	PM_CLOCK_SCALER,
	PM_MEAS_TIME,
	PM_BAT_ALM_CUT_OFF_I_MAX
};






typedef struct {
	uint8_t data[PM_PW_CTRL_RTCADJ_I_MAX];
} __attribute__ ((packed)) pm_pw_ctrl_rtcadj_type;

typedef struct {
	uint16_t data[PM_PW_CTRL_TEMP_ERR_I_MAX];
} __attribute__ ((packed)) pm_pw_ctrl_temp_err_type;



typedef struct {
	uint16_t data[PM_CHARGE_OFF_ON_XO_TEMP_CAM_I_MAX];
} __attribute__ ((packed)) pm_charge_off_on_xo_temp_cam_type;

typedef struct {
	uint16_t data[PM_CHARGE_OFF_ON_XO_TEMP_LTE_I_MAX];
} __attribute__ ((packed)) pm_charge_off_on_xo_temp_lte_type;

typedef struct {
	uint16_t data[PM_CHARGE_OFF_ON_XO_TEMP_I_MAX];
} __attribute__ ((packed)) pm_charge_off_on_xo_temp_type;


typedef struct {
	uint16_t data[PM_CHARGE_OFF_ON_XO_TEMP_MM_I_MAX];
} __attribute__ ((packed)) pm_charge_off_on_xo_temp_mm_type;



typedef struct {
	uint16_t data[PM_CHARGE_OFF_ON_XO_TEMP_YOU_I_MAX];
} __attribute__ ((packed)) pm_charge_off_on_xo_temp_you_type;

typedef struct {
	uint16_t data[PM_CHARGE_OFF_ON_XO_TEMP_WFD_I_MAX];
} __attribute__ ((packed)) pm_charge_off_on_xo_temp_wfd_type;



typedef struct {
	uint16_t data[PM_CHARGE_OFF_ON_XO_TEMP_MHL_I_MAX];
} __attribute__ ((packed)) pm_charge_off_on_xo_temp_mhl_type;


typedef struct {
	uint16_t data[PM_CHARGE_HEADSET_NOISE_I_MAX];
} __attribute__ ((packed)) pm_charge_headset_noise_type;


typedef struct {
	uint16_t data[PM_CHARGE_OFF_PMIC_NCONT_I_MAX];
} __attribute__ ((packed)) pm_charge_off_pmic_ncont_type;


typedef struct {
	uint16_t data[PM_CHG_SETTING_I_MAX];
} __attribute__ ((packed)) pm_chg_setting_type;




typedef struct {
	uint16_t data[PM_BAT_ALM_CUT_OFF_I_MAX];
} __attribute__ ((packed)) pm_bat_alm_cut_off_type;


struct pm_param_struct {
	pm_pw_ctrl_rtcadj_type			pm_pw_ctrl_rtcadj;
	pm_pw_ctrl_temp_err_type		pm_pw_ctrl_temp_err;
	
	pm_charge_off_on_xo_temp_cam_type	pm_charge_off_on_xo_temp_cam;
	pm_charge_off_on_xo_temp_lte_type	pm_charge_off_on_xo_temp_lte;
	pm_charge_off_on_xo_temp_type		pm_charge_off_on_xo_temp;
	pm_charge_off_on_xo_temp_mm_type	pm_charge_off_on_xo_temp_mm; 
	
	pm_charge_off_on_xo_temp_you_type	pm_charge_off_on_xo_temp_you;
	pm_charge_off_on_xo_temp_wfd_type	pm_charge_off_on_xo_temp_wfd;
	pm_charge_off_on_xo_temp_mhl_type	pm_charge_off_on_xo_temp_mhl; 
	
	pm_charge_off_pmic_ncont_type		pm_charge_off_pmic_ncont; 
	pm_chg_setting_type			pm_chg_setting;
	
	pm_bat_alm_cut_off_type			pm_bat_alm_cut_off; 
	pm_charge_headset_noise_type		pm_charge_headset_noise;
};

extern struct pm_param_struct pm_param;

#endif 

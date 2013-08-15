/*
 * Copyright (C) 2012, NEC CASIO Mobile Communications. All rights reserved.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */



























#if !defined(__BOARD_8064_PMIC_OEM_H__)
#define __BOARD_8064_PMIC_OEM_H__

#include "board-8064.h"





struct pm8xxx_gpio_init {
	unsigned			gpio;
	struct pm_gpio			config;
};

struct pm8xxx_mpp_init {
	unsigned			mpp;
	struct pm8xxx_mpp_config_data	config;
};









#define NCM_PM8XXX_MPP_AIN_CTRL  0





#define PM8921_GPIO_INIT(_gpio, _dir, _buf, _val, _pull, _vin, _out_strength, \
			_func, _inv, _disable) \
{ \
	.gpio	= PM8921_GPIO_PM_TO_SYS(_gpio), \
	.config	= { \
		.direction		= PM_GPIO_DIR_##_dir, \
		.output_buffer	= PM_GPIO_OUT_BUF_##_buf, \
		.output_value	= _val, \
		.pull			= PM_GPIO_PULL_##_pull, \
		.vin_sel		= PM_GPIO_VIN_##_vin, \
		.out_strength	= PM_GPIO_STRENGTH_##_out_strength, \
		.function		= PM_GPIO_##_func, \
		.inv_int_pol	= _inv, \
		.disable_pin	= _disable, \
	} \
}

#define PM8921_MPP_INIT(_mpp, _type, _level, _control) \
{ \
	.mpp	= PM8921_MPP_PM_TO_SYS(_mpp), \
	.config	= { \
		.type		= PM8XXX_MPP_TYPE_##_type, \
		.level		= _level, \
		.control	= _control, \
	} \
}

#define PM8821_MPP_INIT(_mpp, _type, _level, _control) \
{ \
	.mpp	= PM8821_MPP_PM_TO_SYS(_mpp), \
	.config	= { \
		.type		= PM8XXX_MPP_TYPE_##_type, \
		.level		= _level, \
		.control	= _control, \
	} \
}









struct pm8xxx_mpp_init pm8xxx_mpps[] __initdata = {
	
	PM8921_MPP_INIT( 1, D_OUTPUT, PM8921_MPP_DIG_LEVEL_VPH, PM8XXX_MPP_DOUT_CTRL_HIGH), 
	
	PM8921_MPP_INIT( 3, D_OUTPUT, PM8921_MPP_DIG_LEVEL_VPH, PM8XXX_MPP_DOUT_CTRL_LOW ), 
	PM8921_MPP_INIT( 4, A_INPUT,  PM8XXX_MPP_AIN_AMUX_CH6,  NCM_PM8XXX_MPP_AIN_CTRL  ), 
	
	
	PM8921_MPP_INIT( 7, D_OUTPUT, PM8921_MPP_DIG_LEVEL_VPH, PM8XXX_MPP_DOUT_CTRL_LOW ), 
	PM8921_MPP_INIT( 8, D_OUTPUT, PM8921_MPP_DIG_LEVEL_S4,  PM8XXX_MPP_DOUT_CTRL_LOW ), 
	PM8921_MPP_INIT( 9, D_INPUT,  PM8921_MPP_DIG_LEVEL_S4,  PM8XXX_MPP_DIN_TO_INT)      
	
	
	
};



















struct pm8xxx_gpio_init pm8921_gpios[] __initdata = {
	
	PM8921_GPIO_INIT( 1, IN,   CMOS, 0,  UP_30,  S4,  NO,          FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT( 2, IN,   CMOS, 0,  UP_30,  S4,  NO,          FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT( 3, OUT,  CMOS, 0,  NO,     S4,  HIGH,        FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT( 4, OUT,  CMOS, 0,  NO,     S4,  HIGH,        FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT( 5, IN,   CMOS, 0,  UP_30,  S4,  NO,          FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT( 6, IN,   CMOS, 0,  UP_30,  S4,  NO,          FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT( 7, OUT,  CMOS, 1,  NO,     S4,  MED,         FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT( 8, IN,   CMOS, 0,  NO,     S4,  NO,          FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT( 9, OUT,  CMOS, 0,  NO,     S4,  HIGH,        FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(10, OUT,  CMOS, 0,  NO,     S4,  HIGH,        FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(11, OUT,  CMOS, 0,  NO,     S4,  HIGH,        FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(12, OUT,  CMOS, 0,  NO,     S4,  HIGH,        FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(13, OUT,  CMOS, 0,  NO,     S4,  LOW,         FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(14, IN,   CMOS, 0,  DN,     S4,  NO,          FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(15, OUT,  CMOS, 0,  NO,     S4,  HIGH,        FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(16, OUT,  CMOS, 0,  NO,     S4,  HIGH,        FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(17, OUT,  CMOS, 1,  NO,     S4,  LOW,         FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(18, OUT,  CMOS, 0,  NO,     S4,  HIGH,        FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(19, OUT,  CMOS, 0,  NO,     S4,  LOW,         FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(20, OUT,  CMOS, 0,  NO,     S4,  HIGH,        FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(21, OUT,  CMOS, 0,  NO,     S4,  HIGH,        FUNC_2,       0,  0), 
	PM8921_GPIO_INIT(22, OUT,  CMOS, 1,  NO,     S4,  LOW,         FUNC_2,       0,  0), 
	PM8921_GPIO_INIT(23, OUT,  CMOS, 0,  NO,     S4,  LOW,         FUNC_2,       0,  0), 
	PM8921_GPIO_INIT(24, OUT,  CMOS, 0,  NO,     S4,  LOW,         FUNC_2,       0,  0), 
	PM8921_GPIO_INIT(25, OUT,  CMOS, 0,  NO,     L17, LOW,         FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(26, OUT,  CMOS, 0,  NO,     S4,  LOW,         FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(27, OUT,  CMOS, 0,  NO,     S4,  HIGH,        FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(28, OUT,  CMOS, 0,  NO,     S4,  LOW,         FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(29, OUT,  CMOS, 0,  NO,     S4,  LOW,         FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(30, OUT,  CMOS, 0,  NO,     S4,  LOW,         FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(31, IN,   CMOS, 0,  NO,     S4,  NO,          FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(32, OUT,  CMOS, 0,  NO,     S4,  HIGH,        FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(33, IN,   CMOS, 0,  NO,     S4,  NO,          FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(34, IN,   CMOS, 0,  UP_1P5, S4,  NO,          FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(35, IN,   CMOS, 0,  DN,     S4,  NO,          FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(36, OUT,  CMOS, 0,  NO,     S4,  LOW,         FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(37, OUT,  CMOS, 0,  NO,     S4,  HIGH,        FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(38, OUT,  CMOS, 0,  NO,     S4,  LOW,         FUNC_2,       0,  0), 
	
	PM8921_GPIO_INIT(40, OUT,  CMOS, 0,  NO,     VPH, LOW,         FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(41, IN,   CMOS, 0,  DN,     S4,  NO,          FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(42, OUT,  CMOS, 0,  NO,     S4,  LOW,         FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(43, OUT,  CMOS, 0,  NO,     S4,  HIGH,        FUNC_NORMAL,  0,  0), 
	PM8921_GPIO_INIT(44, OUT,  CMOS, 0,  NO,     S4,  HIGH,        FUNC_NORMAL,  0,  0), 
};










static int apq8064_pm8921_therm_mitigation[] = {
	1100,
	700,
	600,
	325,
};

 
#define MAX_VOLTAGE_MV		4340
#define CHG_TERM_MA		50
static struct pm8921_charger_platform_data
apq8064_pm8921_chg_pdata __devinitdata = {
	.update_time		= 60000,
	.max_voltage		= MAX_VOLTAGE_MV,	
	.min_voltage		= 3300,			
	.resume_voltage_delta	= 80,			
	.resume_charge_percent	= 99,
	.term_current		= CHG_TERM_MA,		
	.cool_temp		= 5,			
	.warm_temp		= 45,			
	.temp_check_period	= 1,
	.max_bat_chg_current	= 1400,			
	.cool_bat_chg_current	= 350,			
	.warm_bat_chg_current	= 350,			
	.cool_bat_voltage	= 4140,			
	.warm_bat_voltage	= 4140,			
	.thermal_mitigation	= apq8064_pm8921_therm_mitigation,
	.thermal_levels		= ARRAY_SIZE(apq8064_pm8921_therm_mitigation),
	.rconn_mohm		= 18,
};
 

static struct pm8xxx_ccadc_platform_data
apq8064_pm8xxx_ccadc_pdata = {
	.r_sense_uohm		= 10000,
	.calib_delay_ms		= 600000,
};



static struct pm8921_bms_platform_data
apq8064_pm8921_bms_pdata __devinitdata = {
	.battery_type	= BATT_NCM_Nxx_DVE905,
	.r_sense_uohm			= 10000,
	.v_cutoff			= 3300,
	.max_voltage_uv		= MAX_VOLTAGE_MV * 1000,
	.rconn_mohm			= 18,
	.shutdown_soc_valid_limit	= 20,
	.adjust_soc_low_threshold	= 25,
	.chg_term_ua			= CHG_TERM_MA * 1000,
	.normal_voltage_calc_ms		= 20000,
	.low_voltage_calc_ms		= 1000,
	.alarm_low_mv			= 3400,
	.alarm_high_mv			= 4000,
	.high_ocv_correction_limit_uv	= 50,
	.low_ocv_correction_limit_uv	= 100,
	.hold_soc_est			= 3,
	.ignore_shutdown_soc		= 0,
	.enable_fcc_learning		= 0,
	.low_voltage_detect		= 1,
	.vbatt_cutoff_retries		=15,
};



















































  
static struct pm8xxx_adc_amux apq8064_pm8921_adc_channels_data[] = {
	{"vcoin", CHANNEL_VCOIN, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"vbat", CHANNEL_VBAT, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"dcin", CHANNEL_DCIN, CHAN_PATH_SCALING4, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"ichg", CHANNEL_ICHG, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"vph_pwr", CHANNEL_VPH_PWR, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"ibat", CHANNEL_IBAT, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"m4", CHANNEL_MPP_1, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT},
	{"m5", CHANNEL_MPP_2, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2,  ADC_SCALE_DEFAULT},
	{"batt_therm", CHANNEL_BATT_THERM, CHAN_PATH_SCALING1, AMUX_RSV2,
		ADC_DECIMATION_TYPE2, ADC_SCALE_BATT_THERM},
	{"batt_id", CHANNEL_BATT_ID, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"usbin", CHANNEL_USBIN, CHAN_PATH_SCALING3, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"pmic_therm", CHANNEL_DIE_TEMP, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_PMIC_THERM},
	{"625mv", CHANNEL_625MV, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"125v", CHANNEL_125V, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"chg_temp", CHANNEL_CHG_TEMP, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"xo_therm", CHANNEL_MUXOFF, CHAN_PATH_SCALING1, AMUX_RSV0,
		ADC_DECIMATION_TYPE2, ADC_SCALE_XOTHERM},
	{"pa_therm0", ADC_MPP_1_AMUX3, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_PA_THERM},
	{"batt_therm_uv", CHANNEL_BATT_THERM_UV, CHAN_PATH_SCALING1, AMUX_RSV2,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"xo_therm_uv", CHANNEL_MUXOFF_UV, CHAN_PATH_SCALING1, AMUX_RSV0,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"pa_therm0_uv", ADC_MPP_1_AMUX3_UV, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
};
  

#endif  

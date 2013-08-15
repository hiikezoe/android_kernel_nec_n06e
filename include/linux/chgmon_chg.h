/* Copyright (C) 2011-2012, NEC CASIO Mobile Communications. All rights reserved.
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
























#if !defined(CHGMON_CHG_H_)
#define CHGMON_CHG_H_





enum CHGMON_PM_VAL_ID{
	CHGMON_PM_VAL_CHARGER   = 0x00, 
	CHGMON_PM_VAL_CHG_STATE = 0x01, 
	CHGMON_PM_VAL_FSM_STATE = 0x02, 
	CHGMON_PM_VAL_VBAT      = 0x03, 
	CHGMON_PM_VAL_VCHG      = 0x04, 
	CHGMON_PM_VAL_VMAX      = 0x05, 
	CHGMON_PM_VAL_IMAX      = 0x06, 
	CHGMON_PM_VAL_VBATDET   = 0x07, 
	CHGMON_PM_VAL_IBAT      = 0x08, 
	CHGMON_PM_VAL_VOUT      = 0x09, 
	CHGMON_PM_VAL_VIN       = 0x0A, 
	CHGMON_PM_VAL_IUSB      = 0x0B, 
	CHGMON_PM_VAL_TBAT      = 0x0C, 
	CHGMON_PM_VAL_XOTH      = 0x0D, 
	CHGMON_PM_VAL_PMTH      = 0x0E, 
	CHGMON_PM_VAL_LOOP      = 0x0F, 
	CHGMON_PM_DC_PRESENT    = 0x10, 
	CHGMON_PM_USB_PRESENT   = 0x11, 
	CHGMON_PM_EOC_DONE_CNT  = 0x12, 
	CHGMON_PM_JUMPING_CNT   = 0x13, 
	CHGMON_PM_BAT_FULL_CNT  = 0x14, 
	CHGMON_PM_VAL_MAX_NUM,
};





enum CHGMON_PM_CHG_TIME_ID{

	CHGMON_PM_TIME_RTC = 0x00,    

};



enum CHGMON_PM_CHG_STATE_ID{
	CHGMON_PM_CHG_STATE_OFF = 0x00,      
	CHGMON_PM_CHG_STATE_CC       = 0x01,  
	CHGMON_PM_CHG_STATE_CC_H     = 0x02,  
	CHGMON_PM_CHG_STATE_CV       = 0x03,  
	CHGMON_PM_CHG_STATE_CV_H     = 0x04,  
	CHGMON_PM_CHG_STATE_FULL     = 0x05,  
	CHGMON_PM_CHG_STATE_BAT_ERR  = 0x06,  
	CHGMON_PM_CHG_STATE_OVP      = 0x07,  
	CHGMON_PM_CHG_STATE_OCP      = 0x08,  
	CHGMON_PM_CHG_STATE_NO_BATT  = 0x09,  
	CHGMON_PM_CHG_STATE_TEMP_FST = 0x0A,  
	CHGMON_PM_CHG_STATE_CC_H2 = 0x0C,
	CHGMON_PM_CHG_STATE_CC_H3 = 0x0D,
	CHGMON_PM_CHG_STATE_MAX_NUM,
} ;



enum CHGMON_PM_CHG_CHARGER_ID{
	CHGMON_PM_CHG_CHARGER_NONE = 0x00,       
	CHGMON_PM_CHG_CHARGER_Other = 0x01,      
	CHGMON_PM_CHG_CHARGER_DCP = 0x02,        
	CHGMON_PM_CHG_CHARGER_SDP = 0x03,        

	CHGMON_PM_CHG_CHARGER_Other_DCP = 0x04, 
	CHGMON_PM_CHG_CHARGER_Sub_Charger = 0x05,     

	CHGMON_PM_CHG_CHARGER_Wireless = 0x06,

	CHGMON_PM_CHG_CHARGER_MHL = 0x07,



	CHGMON_PM_CHARGER_MAX_NUM,
};

   
enum BMSMON_PM_VAL_ID{
	BMSMON_PM_VAL_OCV = 0,
	BMSMON_PM_VAL_FCC,
	BMSMON_PM_VAL_RC,
	BMSMON_PM_VAL_PC_RC,
	BMSMON_PM_VAL_CC,
	BMSMON_PM_VAL_PC_CC,
	BMSMON_PM_VAL_UUC,
	BMSMON_PM_VAL_PC_UUC,
	BMSMON_PM_VAL_RUC,
	BMSMON_PM_VAL_SOC_0,
	BMSMON_PM_VAL_SOC_1,
	BMSMON_PM_VAL_SOC_2,
	BMSMON_PM_VAL_SOC_3,
	BMSMON_PM_VAL_SOC_4,
	BMSMON_PM_VAL_SOC_5,
	BMSMON_PM_VAL_SOC,
	BMSMON_PM_VAL_MaxVoltage,
	BMSMON_PM_VAL_BattTemp,
	BMSMON_PM_VAL_BattCurrent,
	BMSMON_PM_VAL_Iavg,
	BMSMON_PM_VAL_Rbatt,
	BMSMON_PM_VAL_Vbatt,
	BMSMON_PM_VAL_OCV_EST,
	BMSMON_PM_VAL_PC_EST,
	BMSMON_PM_VAL_SOC_EST,
	BMSMON_PM_VAL_ChargeIncrease,
	BMSMON_PM_VAL_ChargeCycles,
	BMSMON_PM_VAL_ChgEndCnt,
	BMSMON_PM_VAL_BatFullCnt,
	BMSMON_PM_VAL_OcvRenewCnt,
	BMSMON_PM_VAL_Vsense,
	BMSMON_PM_VAL_Rsense,
	BMSMON_PM_VAL_StartCC,
	BMSMON_PM_VAL_StartOCV,
	BMSMON_PM_VAL_StartSOC,
	BMSMON_PM_VAL_EndCC,
	BMSMON_PM_VAL_EndOCV,
	BMSMON_PM_VAL_EndSOC,
	BMSMON_PM_VAL_MAX_NUM,
};
  

#endif 
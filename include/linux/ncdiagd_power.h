/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#if !defined(__NCDIAGD_POWER_H__)
#define __NCDIAGD_POWER_H__


















#include <linux/ioctl.h>
#include <linux/types.h>

#define DIAG_PW_IOCTL_MAGIC 0xE0 
#define PW_MAXARG 11 

#define IOCTL_PW_GR_TCXO_CTL             _IOW (DIAG_PW_IOCTL_MAGIC, PW_GR_TCXO_CTL_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_GR_TCXO_CMD             _IOW (DIAG_PW_IOCTL_MAGIC, PW_GR_TCXO_CMD_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_GR_TCXO_CMD2            _IOW (DIAG_PW_IOCTL_MAGIC, PW_GR_TCXO_CMD2_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_GR_XO_SLP_CLK           _IOW (DIAG_PW_IOCTL_MAGIC, PW_GR_XO_SLP_CLK_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_GR_XO_ENABLE            _IOW (DIAG_PW_IOCTL_MAGIC, PW_GR_XO_ENABLE_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_GR_XO_RFBUF             _IOW (DIAG_PW_IOCTL_MAGIC, PW_GR_XO_RFBUF_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_GR_XO_BOOST             _IOW (DIAG_PW_IOCTL_MAGIC, PW_GR_XO_BOOST_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_GR_RTC_TM_SET           _IOW (DIAG_PW_IOCTL_MAGIC, PW_GR_RTC_TM_SET_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_RG_LP_CTL               _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_LP_CTL_MAGIC,             ioctl_pw_value_type )
#define IOCTL_PW_RG_CTL                  _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_CTL_MAGIC,                ioctl_pw_value_type )
#define IOCTL_PW_RG_SET_LVL              _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_SET_LVL_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_RG_SMPS_CNF             _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_SMPS_CNF_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_RG_SMPS_CLK             _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_SMPS_CLK_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_RG_SMPS_XO_DIV          _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_SMPS_XO_DIV_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_RG_BPS_SET              _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_BPS_SET_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_RG_BPS_CLR              _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_BPS_CLR_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_RG_SMPS_SZ_SET          _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_SMPS_SZ_SET_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_RG_SMPS_PSK             _IOW (DIAG_PW_IOCTL_MAGIC, PW_RG_SMPS_PSK_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_VT_PLDWN_SW             _IOW (DIAG_PW_IOCTL_MAGIC, PW_VT_PLDWN_SW_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_CHG_TRN_SW              _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_TRN_SW_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_CHG_TRN_IMSEL           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_TRN_IMSEL_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_PLS_CNF             _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_PLS_CNF_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_CHG_PLS_SW              _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_PLS_SW_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_CHG_PLS_V_CNF           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_PLS_V_CNF_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_BOOT_STUP           _IO  (DIAG_PW_IOCTL_MAGIC, PW_CHG_BOOT_STUP_MAGIC                              )
#define IOCTL_PW_CHG_BOOT_CRUP           _IO  (DIAG_PW_IOCTL_MAGIC, PW_CHG_BOOT_CRUP_MAGIC                              )
#define IOCTL_PW_CHG_COIN_CNF            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_COIN_CNF_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_COIN_SW             _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_COIN_SW_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_CHG_IS_SPT              _IOR (DIAG_PW_IOCTL_MAGIC, PW_CHG_IS_SPT_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_CHG_TRK_EBL             _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_TRK_EBL_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_CHG_USB_SSPD            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_USB_SSPD_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_IM_CAL_SET          _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_IM_CAL_SET_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_CHG_IM_GAIN_SET         _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_IM_GAIN_SET_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_CHG_BAT_EBL             _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_BAT_EBL_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_CHG_BOOST_EBL           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_BOOST_EBL_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_DSBL                _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_DSBL_MAGIC,              ioctl_pw_value_type )
#define IOCTL_PW_CHG_VCP_EBL             _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_VCP_EBL_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_CHG_ATC_DSBL            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_ATC_DSBL_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_AUTO_DSBL           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_AUTO_DSBL_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_OVRD_EBL            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_OVRD_EBL_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_DONE_EBL            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_DONE_EBL_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_MODE_SET            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_MODE_SET_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_BTFET_DSBL          _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_BTFET_DSBL_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_CHG_BATT_DSBL           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_BATT_DSBL_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_FAIL_CLR            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_FAIL_CLR_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_VMAX_SET            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_VMAX_SET_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_VBAT_SET            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_VBAT_SET_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_IMAX_SET            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_IMAX_SET_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_VTRKL_SET           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_VTRKL_SET_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_ITRKL_SET           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_ITRKL_SET_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_ITERM_SET           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_ITERM_SET_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_TTRKL_SET           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_TTRKL_SET_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_TCHG_SET            _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_TCHG_SET_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_CHG_STATE_GET           _IOR (DIAG_PW_IOCTL_MAGIC, PW_CHG_STATE_GET_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_STATE_SET           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_STATE_SET_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_CHG_VBT_TRIM_SET        _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_VBT_TRIM_SET_MAGIC,      ioctl_pw_value_type )
#define IOCTL_PW_CHG_VBT_TRIM_GET        _IOR (DIAG_PW_IOCTL_MAGIC, PW_CHG_VBT_TRIM_GET_MAGIC,      ioctl_pw_value_type )
#define IOCTL_PW_CHG_VBT_TRIM_GETLIM     _IOR (DIAG_PW_IOCTL_MAGIC, PW_CHG_VBT_TRIM_GETLIM_MAGIC,   ioctl_pw_value_type )
#define IOCTL_PW_CHG_VBT_TRIM_MDSW       _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_VBT_TRIM_MDSW_MAGIC,     ioctl_pw_value_type )
#define IOCTL_PW_ADC_INIT                _IO  (DIAG_PW_IOCTL_MAGIC, PW_ADC_INIT_MAGIC                                   )
#define IOCTL_PW_ADC_CNF_MUX             _IOW (DIAG_PW_IOCTL_MAGIC, PW_ADC_CNF_MUX_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_ADC_CALIB_REQD          _IOR (DIAG_PW_IOCTL_MAGIC, PW_ADC_CALIB_REQD_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_ADC_AMUX_OFF            _IOR (DIAG_PW_IOCTL_MAGIC, PW_ADC_AMUX_OFF_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_ADC_REG_AVBL            _IOR (DIAG_PW_IOCTL_MAGIC, PW_ADC_REG_AVBL_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_ADC_GET_PRESC           _IOWR(DIAG_PW_IOCTL_MAGIC, PW_ADC_GET_PRESC_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_ADC_RD_CHANNEL          _IOWR(DIAG_PW_IOCTL_MAGIC, PW_ADC_RD_CHANNEL_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_LS_LED_SET              _IOW (DIAG_PW_IOCTL_MAGIC, PW_LS_LED_SET_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_LS_LED_GET              _IOWR(DIAG_PW_IOCTL_MAGIC, PW_LS_LED_GET_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_LS_LED_SET_CUR          _IOW (DIAG_PW_IOCTL_MAGIC, PW_LS_LED_SET_CUR_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_LS_LED_SET_MODE         _IOW (DIAG_PW_IOCTL_MAGIC, PW_LS_LED_SET_MODE_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_LS_LED_SET_POL          _IOW (DIAG_PW_IOCTL_MAGIC, PW_LS_LED_SET_POL_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_LS_VIB_SET_VOLT         _IOW (DIAG_PW_IOCTL_MAGIC, PW_LS_VIB_SET_VOLT_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_LS_VIB_SET_MODE         _IOW (DIAG_PW_IOCTL_MAGIC, PW_LS_VIB_SET_MODE_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_KHZ_XTAL_OSC_CMD        _IOW (DIAG_PW_IOCTL_MAGIC, PW_KHZ_XTAL_OSC_CMD_MAGIC,      ioctl_pw_value_type )
#define IOCTL_PW_SP_SMPLD_SW             _IOW (DIAG_PW_IOCTL_MAGIC, PW_SP_SMPLD_SW_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_SP_SMPLD_TM_SET         _IOW (DIAG_PW_IOCTL_MAGIC, PW_SP_SMPLD_TM_SET_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_MPP_CNFDG_IPUT          _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_CNFDG_IPUT_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_MPP_CNFDG_OPUT          _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_CNFDG_OPUT_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_MPP_CNFDG_IOPUT         _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_CNFDG_MAGIC,             ioctl_pw_value_type )
#define IOCTL_PW_MPP_CNFAN_IPUT          _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_CNFAN_IPUT_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_MPP_CNFAN_OPUT          _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_CNFAN_OPUT_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_MPP_CNF_I_SINK          _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_CNF_I_SINK_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW_MPP_CNF_TEST            _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_CNF_TEST_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_MPP_SCR_CNFDG_OPUT      _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_SCR_CNFDG_OPUT_MAGIC,    ioctl_pw_value_type )
#define IOCTL_PW_MPP_SCR_CNFDG_SET       _IOW (DIAG_PW_IOCTL_MAGIC, PW_MPP_SCR_CNFDG_SET_MAGIC,     ioctl_pw_value_type )
#define IOCTL_PW_GPIO_CONFIG_SET         _IOW (DIAG_PW_IOCTL_MAGIC, PW_GPIO_CONFIG_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_SND_HSED_EBL            _IOW (DIAG_PW_IOCTL_MAGIC, PW_SND_HSED_EBL_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW_SND_HSED_CRTTRS_SET     _IOW (DIAG_PW_IOCTL_MAGIC, PW_SND_HSED_CRTTRS_SET_MAGIC,   ioctl_pw_value_type )
#define IOCTL_PW_SND_HSED_HYST_SET       _IOW (DIAG_PW_IOCTL_MAGIC, PW_SND_HSED_HYST_SET_MAGIC,     ioctl_pw_value_type )
#define IOCTL_PW_SND_HSED_PERIOD_SET     _IOW (DIAG_PW_IOCTL_MAGIC, PW_SND_HSED_PERIOD_SET_MAGIC,   ioctl_pw_value_type )
#define IOCTL_PW_SND_MIC_EN              _IOW (DIAG_PW_IOCTL_MAGIC, PW_SND_MIC_EN_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW_SND_MIC_IS_EN           _IOR (DIAG_PW_IOCTL_MAGIC, PW_SND_MIC_IS_EN_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_SND_MIC_VOL_SET         _IOW (DIAG_PW_IOCTL_MAGIC, PW_SND_MIC_VOL_SET_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_SND_MIC_VOL_GET         _IOR (DIAG_PW_IOCTL_MAGIC, PW_SND_MIC_VOL_GET_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_PCT_OVP_EBL             _IOW (DIAG_PW_IOCTL_MAGIC, PW_PCT_OVP_EBL_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_PCT_OVP_TRS_SET         _IOW (DIAG_PW_IOCTL_MAGIC, PW_PCT_OVP_TRS_SET_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_PCT_OVP_HYST_SET        _IOW (DIAG_PW_IOCTL_MAGIC, PW_PCT_OVP_HYST_SET_MAGIC,      ioctl_pw_value_type )
#define IOCTL_PW_PCT_OTP_STAGE_GET       _IOR (DIAG_PW_IOCTL_MAGIC, PW_PCT_OTP_STAGE_GET_MAGIC,     ioctl_pw_value_type )
#define IOCTL_PW_PCT_OTP_STG_OVD         _IOW (DIAG_PW_IOCTL_MAGIC, PW_PCT_OTP_STG_OVD_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW_IR_RT_STATUS_GET        _IOWR(DIAG_PW_IOCTL_MAGIC, PW_IR_RT_STATUS_GET_MAGIC,      ioctl_pw_value_type )
#define IOCTL_PW_REGISTER_READ           _IOWR(DIAG_PW_IOCTL_MAGIC, PW_REGISTER_READ_MAGIC,         ioctl_pw_value_type )
#define IOCTL_PW_REGISTER_WRITE          _IOW (DIAG_PW_IOCTL_MAGIC, PW_REGISTER_WRITE_MAGIC,        ioctl_pw_value_type )

#define IOCTL_PW_HW_PWR_OFF              _IO  (DIAG_PW_IOCTL_MAGIC, PW_HW_PWR_OFF_MAGIC                                 )
#define IOCTL_PW_HW_RESET                _IO  (DIAG_PW_IOCTL_MAGIC, PW_HW_RESET_MAGIC                                   )



#define DIAG_PW8821_IOCTL_MAGIC 0xE1 
#define IOCTL_PW8821_RG_LP_CTL               _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_RG_LP_CTL_MAGIC,             ioctl_pw_value_type )
#define IOCTL_PW8821_RG_CTL                  _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_RG_CTL_MAGIC,                ioctl_pw_value_type )
#define IOCTL_PW8821_RG_SET_LVL              _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_RG_SET_LVL_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW8821_RG_SMPS_CNF             _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_RG_SMPS_CNF_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW8821_RG_SMPS_CLK             _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_RG_SMPS_CLK_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW8821_RG_SMPS_XO_DIV          _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_RG_SMPS_XO_DIV_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW8821_RG_BPS_SET              _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_RG_BPS_SET_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW8821_RG_BPS_CLR              _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_RG_BPS_CLR_MAGIC,            ioctl_pw_value_type )
#define IOCTL_PW8821_RG_SMPS_SZ_SET          _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_RG_SMPS_SZ_SET_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW8821_RG_SMPS_PSK             _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_RG_SMPS_PSK_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW8821_VT_PLDWN_SW             _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_VT_PLDWN_SW_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW8821_MPP_CNFDG_IPUT          _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_MPP_CNFDG_IPUT_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW8821_MPP_CNFDG_OPUT          _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_MPP_CNFDG_OPUT_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW8821_MPP_CNFDG_IOPUT         _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_MPP_CNFDG_MAGIC,             ioctl_pw_value_type )
#define IOCTL_PW8821_MPP_CNFAN_IPUT          _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_MPP_CNFAN_IPUT_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW8821_MPP_CNFAN_OPUT          _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_MPP_CNFAN_OPUT_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW8821_MPP_CNF_I_SINK          _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_MPP_CNF_I_SINK_MAGIC,        ioctl_pw_value_type )
#define IOCTL_PW8821_MPP_CNF_TEST            _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_MPP_CNF_TEST_MAGIC,          ioctl_pw_value_type )
#define IOCTL_PW8821_MPP_SCR_CNFDG_OPUT      _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_MPP_SCR_CNFDG_OPUT_MAGIC,    ioctl_pw_value_type )
#define IOCTL_PW8821_MPP_SCR_CNFDG_SET       _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_MPP_SCR_CNFDG_SET_MAGIC,     ioctl_pw_value_type )
#define IOCTL_PW8821_PCT_OTP_STAGE_GET       _IOR (DIAG_PW8821_IOCTL_MAGIC, PW_PCT_OTP_STAGE_GET_MAGIC,     ioctl_pw_value_type )
#define IOCTL_PW8821_PCT_OTP_STG_OVD         _IOW (DIAG_PW8821_IOCTL_MAGIC, PW_PCT_OTP_STG_OVD_MAGIC,       ioctl_pw_value_type )
#define IOCTL_PW8821_IR_RT_STATUS_GET        _IOWR(DIAG_PW8821_IOCTL_MAGIC, PW_IR_RT_STATUS_GET_MAGIC,      ioctl_pw_value_type )



#define IOCTL_PW_TSENSOR_GET_DEG             _IOR (DIAG_PW_IOCTL_MAGIC, PW_TEMP_SENSOR_GET_DEG_MAGIC,     ioctl_pw_value_type )
#define IOCTL_PW_TSENSOR_GET_AD              _IOR (DIAG_PW_IOCTL_MAGIC, PW_TEMP_SENSOR_GET_ADC_MAGIC,     ioctl_pw_value_type )
#define IOCTL_PW_TSENSOR_SW                  _IOW (DIAG_PW_IOCTL_MAGIC, PW_TEMP_SENSOR_SW_MAGIC,          ioctl_pw_value_type )



#define IOCTL_PW_CHG_USB_DSBL                _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_USB_DSBL_MAGIC,            ioctl_pw_value_type )



#define IOCTL_PW_GPIO_GET_STATE              _IOWR(DIAG_PW_IOCTL_MAGIC, PW_GPIO_GET_STATE_MAGIC,          ioctl_pw_value_type )



#define IOCTL_PW_CHG_WCG_GET_STATE           _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_WCG_GET_STATE_MAGIC,           ioctl_pw_value_type )
#define IOCTL_PW_CHG_WCG_SW                  _IOW (DIAG_PW_IOCTL_MAGIC, PW_CHG_WCG_SW_MAGIC,                  ioctl_pw_value_type )


enum ioctl_pw_magic{

    
    PW_GR_TCXO_CTL_MAGIC,           
    PW_GR_TCXO_CMD_MAGIC,           
    PW_GR_TCXO_CMD2_MAGIC,          
    PW_GR_XO_SLP_CLK_MAGIC,         
    PW_GR_XO_ENABLE_MAGIC,          
    PW_GR_XO_RFBUF_MAGIC,           
    PW_GR_XO_BOOST_MAGIC,           
    PW_GR_RTC_TM_SET_MAGIC,         

    
    PW_RG_LP_CTL_MAGIC,             
    PW_RG_CTL_MAGIC,                
    PW_RG_SET_LVL_MAGIC,            
    PW_RG_SMPS_CNF_MAGIC,           
    PW_RG_SMPS_CLK_MAGIC,           
    PW_RG_SMPS_XO_DIV_MAGIC,        
    PW_RG_BPS_SET_MAGIC,            
    PW_RG_BPS_CLR_MAGIC,            
    PW_RG_SMPS_SZ_SET_MAGIC,        
    PW_RG_SMPS_PSK_MAGIC,           
    PW_VT_PLDWN_SW_MAGIC,           

    
    PW_CHG_TRN_SW_MAGIC,            
    PW_CHG_TRN_IMSEL_MAGIC,         
    PW_CHG_PLS_CNF_MAGIC,           
    PW_CHG_PLS_SW_MAGIC,            
    PW_CHG_PLS_V_CNF_MAGIC,         
    PW_CHG_BOOT_STUP_MAGIC,         
    PW_CHG_BOOT_CRUP_MAGIC,         
    PW_CHG_COIN_CNF_MAGIC,          
    PW_CHG_COIN_SW_MAGIC,           
    PW_CHG_IS_SPT_MAGIC,            
    PW_CHG_TRK_EBL_MAGIC,           
    PW_CHG_USB_SSPD_MAGIC,          
    PW_CHG_IM_CAL_SET_MAGIC,        
    PW_CHG_IM_GAIN_SET_MAGIC,       
    PW_CHG_BAT_EBL_MAGIC,           
    PW_CHG_BOOST_EBL_MAGIC,         
    PW_CHG_DSBL_MAGIC,              
    PW_CHG_VCP_EBL_MAGIC,           
    PW_CHG_ATC_DSBL_MAGIC,          
    PW_CHG_AUTO_DSBL_MAGIC,         
    PW_CHG_OVRD_EBL_MAGIC,          
    PW_CHG_DONE_EBL_MAGIC,          
    PW_CHG_MODE_SET_MAGIC,          
    PW_CHG_BTFET_DSBL_MAGIC,        
    PW_CHG_BATT_DSBL_MAGIC,         
    PW_CHG_FAIL_CLR_MAGIC,          
    PW_CHG_VMAX_SET_MAGIC,          
    PW_CHG_VBAT_SET_MAGIC,          
    PW_CHG_IMAX_SET_MAGIC,          
    PW_CHG_VTRKL_SET_MAGIC,         
    PW_CHG_ITRKL_SET_MAGIC,         
    PW_CHG_ITERM_SET_MAGIC,         
    PW_CHG_TTRKL_SET_MAGIC,         
    PW_CHG_TCHG_SET_MAGIC,          
    PW_CHG_STATE_GET_MAGIC,         
    PW_CHG_STATE_SET_MAGIC,         

    
    PW_CHG_VBT_TRIM_SET_MAGIC,      
    PW_CHG_VBT_TRIM_GET_MAGIC,      
    PW_CHG_VBT_TRIM_GETLIM_MAGIC,   
    PW_CHG_VBT_TRIM_MDSW_MAGIC,     

    
    PW_ADC_INIT_MAGIC,              
    PW_ADC_CNF_MUX_MAGIC,           
    PW_ADC_CALIB_REQD_MAGIC,        
    PW_ADC_AMUX_OFF_MAGIC,          
    PW_ADC_REG_AVBL_MAGIC,          
    PW_ADC_GET_PRESC_MAGIC,         
    PW_ADC_RD_CHANNEL_MAGIC,        

    
    PW_LS_LED_SET_MAGIC,            
    PW_LS_LED_GET_MAGIC,            
    PW_LS_LED_SET_CUR_MAGIC,        
    PW_LS_LED_SET_MODE_MAGIC,       
    PW_LS_LED_SET_POL_MAGIC,        
    PW_LS_VIB_SET_VOLT_MAGIC,       
    PW_LS_VIB_SET_MODE_MAGIC,       

    
    PW_KHZ_XTAL_OSC_CMD_MAGIC,      

    
    PW_SP_SMPLD_SW_MAGIC,           
    PW_SP_SMPLD_TM_SET_MAGIC,       

    
    PW_MPP_CNFDG_IPUT_MAGIC,        
    PW_MPP_CNFDG_OPUT_MAGIC,        
    PW_MPP_CNFDG_MAGIC,             
    PW_MPP_CNFAN_IPUT_MAGIC,        
    PW_MPP_CNFAN_OPUT_MAGIC,        
    PW_MPP_CNF_I_SINK_MAGIC,        
    PW_MPP_CNF_TEST_MAGIC,          
    PW_MPP_SCR_CNFDG_OPUT_MAGIC,    
    PW_MPP_SCR_CNFDG_SET_MAGIC,     

    
    PW_GPIO_CONFIG_MAGIC,           

    
    PW_SND_HSED_EBL_MAGIC,          
    PW_SND_HSED_CRTTRS_SET_MAGIC,   
    PW_SND_HSED_HYST_SET_MAGIC,     
    PW_SND_HSED_PERIOD_SET_MAGIC,   
    PW_SND_MIC_EN_MAGIC,            
    PW_SND_MIC_IS_EN_MAGIC,         
    PW_SND_MIC_VOL_SET_MAGIC,       
    PW_SND_MIC_VOL_GET_MAGIC,       

    
    PW_PCT_OVP_EBL_MAGIC,           
    PW_PCT_OVP_TRS_SET_MAGIC,       
    PW_PCT_OVP_HYST_SET_MAGIC,      
    PW_PCT_OTP_STAGE_GET_MAGIC,     
    PW_PCT_OTP_STG_OVD_MAGIC,       

    
    PW_IR_RT_STATUS_GET_MAGIC,      

    
    PW_REGISTER_READ_MAGIC,         
    PW_REGISTER_WRITE_MAGIC,        


    
    PW_HW_PWR_OFF_MAGIC,            
    PW_HW_RESET_MAGIC,              



    PW_TEMP_SENSOR_GET_DEG_MAGIC,   
    PW_TEMP_SENSOR_GET_ADC_MAGIC,   
    PW_TEMP_SENSOR_SW_MAGIC,        



    PW_CHG_USB_DSBL_MAGIC,          



    PW_GPIO_GET_STATE_MAGIC,        



    PW_CHG_WCG_GET_STATE_MAGIC,     
    PW_CHG_WCG_SW_MAGIC             

};

typedef enum
{
    


    PM_ERR_FLAG__SUCCESS,
    



    PM_ERR_FLAG__SBI_OPT_ERR,
    PM_ERR_FLAG__SBI_OPT2_ERR,
    PM_ERR_FLAG__SBI_OPT3_ERR,
    PM_ERR_FLAG__SBI_OPT4_ERR,

    


    PM_ERR_FLAG__PAR1_OUT_OF_RANGE,
    


    PM_ERR_FLAG__PAR2_OUT_OF_RANGE,
    


    PM_ERR_FLAG__PAR3_OUT_OF_RANGE,
    


    PM_ERR_FLAG__PAR4_OUT_OF_RANGE,
    


    PM_ERR_FLAG__PAR5_OUT_OF_RANGE,
    PM_ERR_FLAG__VLEVEL_OUT_OF_RANGE,
    PM_ERR_FLAG__VREG_ID_OUT_OF_RANGE,
    


    PM_ERR_FLAG__FEATURE_NOT_SUPPORTED,
    PM_ERR_FLAG__INVALID_PMIC_MODEL,
    PM_ERR_FLAG__SECURITY_ERR,
    PM_ERR_FLAG__IRQ_LIST_ERR,
    PM_ERR_FLAG__DEV_MISMATCH,
    PM_ERR_FLAG__ADC_INVALID_RES,
    PM_ERR_FLAG__ADC_NOT_READY,
    PM_ERR_FLAG__ADC_SIG_NOT_SUPPORTED,
    


    PM_ERR_FLAG__RTC_BAD_DIS_MODE,
    


    PM_ERR_FLAG__RTC_READ_FAILED,
    


    PM_ERR_FLAG__RTC_WRITE_FAILED,
    


    PM_ERR_FLAG__RTC_HALTED,
    


    PM_ERR_FLAG__DBUS_IS_BUSY_MODE,
    


    PM_ERR_FLAG__ABUS_IS_BUSY_MODE,
    



    PM_ERR_FLAG__MACRO_NOT_RECOGNIZED,
    



    PM_ERR_FLAG__DATA_VERIFY_FAILURE,
    



    PM_ERR_FLAG__SETTING_TYPE_NOT_RECOGNIZED,
    



    PM_ERR_FLAG__MODE_NOT_DEFINED_FOR_MODE_GROUP,
    



    PM_ERR_FLAG__MODE_GROUP_NOT_DEFINED,
    


    PM_ERR_FLAG__PRESTUB_FAILURE,
    



    PM_ERR_FLAG__POSTSTUB_FAILURE,
    



    PM_ERR_FLAG__MODE_NOT_RECORDED_CORRECTLY,
    



    PM_ERR_FLAG__MODE_GROUP_STATE_NOT_FOUND,
    



    PM_ERR_FLAG__SUPERSTUB_FAILURE,

    


    PM_ERR_FLAG__VBATT_CLIENT_TABLE_FULL,
    
    PM_ERR_FLAG__VBATT_REG_PARAMS_WRONG,
    
    PM_ERR_FLAG__VBATT_DEREGISTRATION_FAILED,
    
    PM_ERR_FLAG__VBATT_MODIFICATION_FAILED,
    
    PM_ERR_FLAG__VBATT_INTERROGATION_FAILED,
    
    PM_ERR_FLAG__VBATT_SET_FILTER_FAILED,
    


    PM_ERR_FLAG__VBATT_LAST_ERROR,
    PM_ERR_FLAG__PMIC_NOT_SUPPORTED,
    


    PM_ERR_FLAG__INVALID_KEYPAD_EVENT_COUNTER,
    


    PM_ERR_FLAG__DAL_SERVICE_FAILED,
    PM_ERR_FLAG__INVALID

} pm_err_flag_type;

#define ADC_ERR_FLAG__SUCCESS 0


typedef struct {
    unsigned char   req_buf[PW_MAXARG]; 
    unsigned char   rsp_buf[PW_MAXARG]; 
} ioctl_pw_value_type;

#endif 

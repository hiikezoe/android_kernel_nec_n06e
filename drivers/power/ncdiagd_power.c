/*  
 * Copyright (C) 2011-2012, NEC CASIO Mobile Communications. All rights reserved.
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






























#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/ncdiagd_power.h>

#include <linux/uaccess.h>
#include <mach/pmic.h>


#include <linux/pm.h>   
#include <asm/system.h> 



#include <linux/mfd/pm8xxx/pm8xxx-adc.h>        
#include <linux/mfd/pm8xxx/pm8921-charger.h>    
#include <linux/mfd/pm8xxx/pm8921.h>            
#include <linux/mfd/pm8xxx/tm.h>                
#include <mach/irqs.h>

#define PM8921_GPIO_BASE                NR_GPIO_IRQS
#define PM8921_GPIO_PM_TO_SYS(pm_gpio)  (pm_gpio + PM8921_GPIO_BASE)


#include <linux/mfd/pm8xxx/mpp.h>               
#include <linux/mfd/pm8xxx/irq.h>               
#include <linux/regulator/pm8xxx-regulator.h>   


#define PM8921_MPP_BASE                 (PM8921_GPIO_BASE + PM8921_NR_GPIOS)
#define PM8921_MPP_PM_TO_SYS(pm_mpp)    (pm_mpp + PM8921_MPP_BASE)
#define PM8921_IRQ_BASE                 (NR_MSM_IRQS + NR_GPIO_IRQS)



#define PM8821_MPP_BASE                 (PM8921_MPP_BASE + PM8921_NR_MPPS)
#define PM8821_MPP_PM_TO_SYS(pm_mpp)    (pm_mpp + PM8821_MPP_BASE)
#define PM8821_IRQ_BASE                 (PM8921_IRQ_BASE + PM8921_NR_IRQS)



#include <linux/mfd/pm8xxx/misc.h>


#include <linux/math64.h>



#include <linux/mfd/pm8xxx/gpio.h>


static short access_count = 0;

static unsigned char cvt_val(unsigned char c)
{
    int ten_val;
    int rm_val;
    if (c >= 0xA)
    {
        ten_val = c / 0x10;
        rm_val  = c % 0x10;
        return (ten_val * 10 + rm_val);
    }
    else
    {
        return c;
    }
}


static long ncdiagd_power_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    void __user *pArg = (void __user *)arg;
    printk(KERN_INFO "[%s] cmd:%d",__func__, cmd);

    switch(cmd)
    {































































































































































        


        
        case IOCTL_PW_RG_LP_CTL:
            {
                unsigned char enable;
                unsigned char vreg_id;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable  = buf.req_buf[0];
                vreg_id = buf.req_buf[1];

                printk(KERN_INFO "DIAG_PW_RG_LP_CTL enable:%x vreg_id:%x", enable, vreg_id);
                ret = nc_pm8xxx_lp_mode_control(PM8XXX_VERSION_8921, vreg_id, enable);  
            }
            break;
        
        case IOCTL_PW_RG_CTL:
            {
                unsigned char enable;
                unsigned char vreg_id;
                unsigned int min_volt, max_volt;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable  = buf.req_buf[0];
                vreg_id = buf.req_buf[1];
                min_volt  = cvt_val(buf.req_buf[3]);
                min_volt += cvt_val(buf.req_buf[2]) * 100;
                min_volt *= 1000; 
                max_volt  = cvt_val(buf.req_buf[5]);
                max_volt += cvt_val(buf.req_buf[4]) * 100;
                max_volt *= 1000; 
                
                printk(KERN_INFO "DIAG_PW_RG_CTL enable  :%x vreg_id :%x", enable, vreg_id);
                printk(KERN_INFO "               min_volt:%x max_volt:%x", min_volt, max_volt);
                ret = nc_pm8xxx_vreg_control(PM8XXX_VERSION_8921, enable, vreg_id, min_volt, max_volt);  
            }
            break;
        
        case IOCTL_PW_RG_SET_LVL:
            {
                unsigned char  vreg_id;
                unsigned int min_volt, max_volt;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                vreg_id   = buf.req_buf[0];
                min_volt  = cvt_val(buf.req_buf[2]);
                min_volt += cvt_val(buf.req_buf[1]) * 100;
                min_volt *= 1000; 
                max_volt  = cvt_val(buf.req_buf[4]);
                max_volt += cvt_val(buf.req_buf[3]) * 100;
                max_volt *= 1000; 
                
                printk(KERN_INFO "DIAG_PW_RG_SET_LVL vreg_id:%x min_volt:%d max_volt:%d", vreg_id, min_volt, max_volt);
                ret = nc_pm8xxx_vreg_set_level(PM8XXX_VERSION_8921, vreg_id, min_volt, max_volt);  
            }
            break;





































































































































        
        case IOCTL_PW_VT_PLDWN_SW:
            {
                unsigned char enable;
                unsigned char vreg_id;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable  = buf.req_buf[0];
                vreg_id = buf.req_buf[1];
                
                printk(KERN_INFO "DIAG_PW_RG_SMPS_PSK enable:%x vreg_id:%x", enable, vreg_id);
                ret = nc_pm8xxx_vreg_pull_down_switch(PM8XXX_VERSION_8921, vreg_id, enable);  
            }
            break;

        





















































































































































        
        case IOCTL_PW_CHG_COIN_SW:
            {
                struct pm8xxx_coincell_chg param;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                param.state = buf.req_buf[0];
                param.voltage = PM8XXX_COINCELL_VOLTAGE_3p2V;
                param.resistor = PM8XXX_COINCELL_RESISTOR_800_OHMS;
                
                ret = pm8xxx_coincell_chg_config(&param);
                printk(KERN_INFO "DIAG_PW_CHG_COIN_SW enable:%x", param.state);
            }
            break;































































































        
        case IOCTL_PW_CHG_BAT_EBL:
            {
                bool enable;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }
                enable = (buf.req_buf[0]&0x01);
                
                ret = nc_pm8921_charger_enable(enable);
                printk(KERN_INFO "DIAG_PW_CHG_BAT_EBL enable:%x", enable);
            }
            break;
























        case IOCTL_PW_CHG_DSBL:
            {
                unsigned char enable;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable = buf.req_buf[0];
                
                enable = enable ? 0x00 : 0x01;
                ret = pm8921_disable_source_current(enable);  
                printk(KERN_INFO "DIAG_PW_CHG_DSBL enable:%x", enable);
            }
            break;



        
        case IOCTL_PW_CHG_WCG_GET_STATE:
            {
                unsigned int  rt_id = 0x00;
                unsigned int  rt_status = 0x00;
                ioctl_pw_value_type buf;

                rt_id = PM8921_IRQ_BASE + PM8921_CABLE_IRQ;
                ret = nc_pm8921_get_rt_status(rt_id, &rt_status);
                
                if (ret != 0) {
                    printk(KERN_ERR "nc_pm8921_get_rt_status failed \n");
                    return PM_ERR_FLAG__SBI_OPT_ERR;
                }
                
                
                buf.rsp_buf[0] = (u8)rt_status;  

                if(copy_to_user((void *)pArg, &buf, sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_to_user failed");
                    return PM_ERR_FLAG__SBI_OPT_ERR; 
                }

                printk(KERN_INFO "IOCTL_PW_CHG_WCG_GET_STATE rt_id:%x rt_status:%x", rt_id, rt_status);
            }
            break;
        
        case IOCTL_PW_CHG_WCG_SW:
            {
                ioctl_pw_value_type buf;
                struct pm_gpio param;
                int gpio_port;
                unsigned char enable = 0x00;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__SBI_OPT_ERR; 
                }

                gpio_port             = PM8921_GPIO_PM_TO_SYS(PM_DIAG_GPIO_32);  
                param.direction       = PM_GPIO_DIR_OUT;
                param.output_buffer   = PM_GPIO_OUT_BUF_CMOS;
                param.output_value    = 0;
                param.pull            = PM_GPIO_PULL_NO;
                param.vin_sel         = PM_GPIO_VIN_S4;
                param.out_strength    = PM_GPIO_STRENGTH_LOW;
                param.function        = PM_GPIO_FUNC_NORMAL;
                param.inv_int_pol     = 0;
                param.disable_pin     = 0;

                enable = buf.req_buf[0];
                
                param.output_value = enable ? 0 : 1;
                
                ret = pm8xxx_gpio_config(gpio_port, &param);
		
		if (ret != 0) {
                    printk(KERN_ERR "pm8xxx_gpio_config failed");
                    return PM_ERR_FLAG__SBI_OPT_ERR;
		}
		
                printk(KERN_INFO "IOCTL_PW_CHG_WCG_SW enable:%x param.output_value:%x ", enable, param.output_value);
            }
            break;














































































































































































































        
        case IOCTL_PW_CHG_VMAX_SET:
            {
                unsigned short voltage;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                voltage  = buf.req_buf[1];
                voltage += buf.req_buf[0] * 0x100;

                ret = nc_pm8921_chg_vmaxsel_set(voltage);
                printk(KERN_INFO "DIAG_PW_CHG_VMAX_SET voltage:%x", voltage);
            }
            break;


        
        case IOCTL_PW_CHG_IMAX_SET:
            {
                unsigned short cur_val;
                ioctl_pw_value_type buf;
                
                
                int ibat = 0;
                unsigned int iusb = 0;
                const int ibat_min = 325;
                const int ibat_max = 2000;
                

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                cur_val  = buf.req_buf[1];
                cur_val += buf.req_buf[0] * 0x100;
                
                
                if (cur_val < ibat_min) {
                    ibat = ibat_min;
                } else if (cur_val > ibat_max) {
                    ibat = ibat_max;
                } else {
                    ibat = (int)cur_val;
                }
                
                iusb = (unsigned int)(cur_val | PM8921_DIAG_IUSB_SET);
                
                pm8921_set_max_battery_charge_current(ibat);
                pm8921_charger_vbus_draw(iusb);
                
                printk(KERN_INFO "DIAG_PW_CHG_IMAX_SET cur_val:0x%04x", cur_val);
                
            }
            break;


































































































        
        case IOCTL_PW_CHG_STATE_GET:
            {
                unsigned char chg_state = 0x00;
                ioctl_pw_value_type buf;
                
                ret = nc_pm8921_get_fsm_status((u64 *)&chg_state);
                
                buf.rsp_buf[0] = chg_state;
                
                if(copy_to_user((void *)pArg, &buf, sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_to_user failed");
                    return PM_ERR_FLAG__INVALID;
                }
                
                printk(KERN_INFO "DIAG_PW_CHG_STATE_GET chg_state:%x", chg_state);
            }
            break;
























        
        case IOCTL_PW_CHG_USB_DSBL:
            {
                unsigned char enable;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable = buf.req_buf[0];

                enable = enable ? 0x00 : 0x01;
                ret = nc_pm8921_chg_usb_suspend_enable(enable);

                printk(KERN_INFO "DIAG_PW_CHG_USB_DSBL enable:%x", enable);
            }
            break;














































































































































































































































        
        case IOCTL_PW_ADC_RD_CHANNEL:
            {
                unsigned char analog_chnl, mpp_chnl;
                struct pm8xxx_adc_chan_result result;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }
                
                analog_chnl = buf.req_buf[0];
                
                 
                if (analog_chnl == CHANNEL_BATT_THERM) {
                    analog_chnl = CHANNEL_BATT_THERM_UV;
                    ret = pm8xxx_adc_read(analog_chnl, &result);
                } else if ((analog_chnl <  PM8XXX_CHANNEL_MPP_SCALE1_IDX) ||
                           (analog_chnl == ADC_MPP_1_AMUX8 )              ||
                           (analog_chnl == ADC_MPP_1_AMUX3 )              ||
                           (analog_chnl == ADC_MPP_1_AMUX3_UV))
                {
                    ret = pm8xxx_adc_read(analog_chnl, &result);
                } else if ((analog_chnl >= PM8XXX_CHANNEL_MPP_SCALE1_IDX) && 
                           (analog_chnl <= ADC_MPP_1_ATEST_7       ) )
                {
                    mpp_chnl = analog_chnl - PM8XXX_CHANNEL_MPP_SCALE1_IDX;
                    ret = pm8xxx_adc_mpp_config_read(mpp_chnl, CHANNEL_MPP_1, &result);

                } else if ((analog_chnl >= PM8XXX_CHANNEL_MPP_SCALE3_IDX) && 
                           (analog_chnl <= ADC_MPP_2_ATEST_7       ) )
                {
                    mpp_chnl = analog_chnl - PM8XXX_CHANNEL_MPP_SCALE3_IDX;
                    ret = pm8xxx_adc_mpp_config_read(mpp_chnl, CHANNEL_MPP_2, &result);
                    
                } else 
                {
                    printk(KERN_ERR "ADC_channel failed");
                    return PM_ERR_FLAG__FEATURE_NOT_SUPPORTED;
                }
                 
                
                if(ret == PM_ERR_FLAG__SUCCESS)
                {
 
                    if ((analog_chnl != CHANNEL_BATT_THERM) &&             
                        (analog_chnl != ADC_MPP_1_AMUX8   ) &&             
                        (analog_chnl != CHANNEL_MUXOFF    ) &&             
                        (analog_chnl != ADC_MPP_1_AMUX3   ))               
                    {
                        if (result.physical != 0)
                        {
                            result.physical = div_u64(result.physical, 1000);
                        }
                        
                        if (result.physical >= 0xFFFF)
                        {
                            result.physical = 0xFFFF;
                        }
                    }
 
                    buf.rsp_buf[0] = (result.physical >> 8) & 0xff;
                    buf.rsp_buf[1] = result.physical & 0xff;
                }

                if(copy_to_user((void *)pArg, &buf, sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_to_user failed");
                    return PM_ERR_FLAG__INVALID;
                }
                
                printk(KERN_INFO "DIAG_PW_ADC_RD_CHANNEL analog_chnl :%x", analog_chnl);
                printk(KERN_INFO "                       adc_read_val[0]:%x", (int)((result.physical >> 8) & 0xff));
                printk(KERN_INFO "                       adc_read_val[1]:%x", (int)(result.physical & 0xff));
            }
            break;







































































































































































        


        
        case IOCTL_PW_SP_SMPLD_SW:
            {
                unsigned char enable;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable = buf.req_buf[0];

                ret = pm8xxx_smpl_control(enable);  
                printk(KERN_INFO "DIAG_PW_SP_SMPLD_SW enable:%x", enable);
            }
            break;
        
        case IOCTL_PW_SP_SMPLD_TM_SET:
            {
                unsigned char timer_set;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                timer_set = buf.req_buf[0];
                
                ret = pm8xxx_smpl_set_delay(timer_set);  
                printk(KERN_INFO "DIAG_PW_SP_SMPLD_TM_SET timer_set:%x", timer_set);
            }
            break;

        


        
        case IOCTL_PW_MPP_CNFDG_IPUT:
            {
                unsigned char mpp_port;
                unsigned char logi_level;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port   = buf.req_buf[0];
                logi_level = buf.req_buf[1];
                
                ret = nc_pm8xxx_mpp_config_digital_in(PM8921_MPP_PM_TO_SYS(mpp_port), logi_level, PM8XXX_MPP_DIN_TO_INT);  
                printk(KERN_INFO "DIAG_PW_MPP_CNFDG_IPUT mpp_port:%x logi_level:%x", mpp_port +1, logi_level);
            }
            break;
        
        case IOCTL_PW_MPP_CNFDG_OPUT:
            {
                unsigned char mpp_port;
                unsigned char logi_level;
                unsigned char out_ctl;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port   = buf.req_buf[0];
                logi_level = buf.req_buf[1];
                out_ctl    = buf.req_buf[2];
                
                ret = nc_pm8xxx_mpp_config_digital_out(PM8921_MPP_PM_TO_SYS(mpp_port), logi_level, out_ctl);  
                printk(KERN_INFO "DIAG_PW_MPP_CNFDG_OPUT mpp_port:%x logi_level:%x out_ctl:%x", mpp_port +1, logi_level, out_ctl);
            }
            break;
        
        case IOCTL_PW_MPP_CNFDG_IOPUT:
            {
                unsigned char mpp_port;
                unsigned char logi_level;
                unsigned char pull_set;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port   = buf.req_buf[0];
                logi_level = buf.req_buf[1];
                pull_set   = buf.req_buf[2];
                
                ret = nc_pm8xxx_mpp_config_bi_dir(PM8921_MPP_PM_TO_SYS(mpp_port), logi_level, pull_set);  
                printk(KERN_INFO "DIAG_PW_MPP_CNFDG_IOPUT mpp_port:%x logi_level:%x pull_set:%x", mpp_port +1, logi_level, pull_set);
            }
            break;
        
        case IOCTL_PW_MPP_CNFAN_IPUT:
            {
                unsigned char mpp_port;
                unsigned char ain_chn;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port = buf.req_buf[0];
                ain_chn  = buf.req_buf[1];
                
                ret = nc_pm8xxx_mpp_config_analog_input(PM8921_MPP_PM_TO_SYS(mpp_port), ain_chn, PM8XXX_MPP_AOUT_CTRL_DISABLE);  
                printk(KERN_INFO "DIAG_PW_MPP_CNFAN_IPUT mpp_port:%x ain_chn:%x", mpp_port + 1, ain_chn);
            }
            break;
        
        case IOCTL_PW_MPP_CNFAN_OPUT:
            {
                unsigned char mpp_port;
                unsigned char aout_level;
                unsigned char pm_onoff;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port   = buf.req_buf[0];
                aout_level = buf.req_buf[1];
                pm_onoff   = buf.req_buf[2];
                
                ret = nc_pm8xxx_mpp_config_analog_output(PM8921_MPP_PM_TO_SYS(mpp_port), aout_level, pm_onoff);  
                printk(KERN_INFO "DIAG_PW_MPP_CNFAN_OPUT mpp_port:%x aout_level:%x pm_onoff:%x", mpp_port +1 , aout_level, pm_onoff);
            }
            break;
        
        case IOCTL_PW_MPP_CNF_I_SINK:
            {
                unsigned char mpp_port;
                unsigned char sink_level;
                unsigned char pm_onoff;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port   = buf.req_buf[0];
                sink_level = buf.req_buf[1];
                pm_onoff   = buf.req_buf[2];
                
                ret = nc_pm8xxx_mpp_config_current_sink(PM8921_MPP_PM_TO_SYS(mpp_port), sink_level, pm_onoff);  
                printk(KERN_INFO "DIAG_PW_MPP_CNF_I_SINK mpp_port:%x sink_level:%x pm_onoff:%x", mpp_port + 1, sink_level, pm_onoff);
            }
            break;
























































































































        


        
        case IOCTL_PW_GPIO_CONFIG_SET:
            {
                ioctl_pw_value_type buf;
                struct pm_gpio param ;
                int gpio_port;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                gpio_port             = PM8921_GPIO_PM_TO_SYS(buf.req_buf[0]);
                param.direction       = buf.req_buf[1];
                param.output_buffer   = buf.req_buf[2];
                param.output_value    = buf.req_buf[3];
                param.pull            = buf.req_buf[4];
                param.vin_sel         = buf.req_buf[5];
                param.out_strength    = buf.req_buf[6];
                param.function        = buf.req_buf[7];
                param.inv_int_pol     = buf.req_buf[8];
                param.disable_pin     = buf.req_buf[9];
                
                ret = pm8xxx_gpio_config(gpio_port, &param);
                
                printk(KERN_INFO "IOCTL_PW_GPIO_CONFIG_SET gpio_port   :%x direction:%x output_buffer:%x", gpio_port, param.direction, param.output_buffer);
                printk(KERN_INFO "                         output_value:%x pull     :%x vin_sel      :%x", param.output_value, param.pull, param.vin_sel);
                printk(KERN_INFO "                         out_strength:%x function :%x inv_int_pol  :%x", param.out_strength, param.function, param.inv_int_pol);
                printk(KERN_INFO "                         pin_disable :%x                              ", param.disable_pin);
            }
            break;


        
        case IOCTL_PW_GPIO_GET_STATE:
            {
                unsigned int gpio_id = 0x00;
                unsigned int gpio_state = 0x00;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }
                
                gpio_id = (unsigned int)buf.req_buf[0];
                ret = nc_pm8921_gpio_get_state(gpio_id, &gpio_state);
                
                buf.rsp_buf[0] = (u8)gpio_state;
                
                if(copy_to_user((void *)pArg, &buf, sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_to_user failed");
                    return PM_ERR_FLAG__INVALID;
                }
                
                printk(KERN_INFO "DIAG_PW_GPIO_GET_STATE GPIO :%d value :%d", gpio_id+1, gpio_state);
            }
            break;



























































































































































































































        
        case IOCTL_PW_PCT_OTP_STAGE_GET:
            {
                unsigned char itemp_stage = 0x00;
                ioctl_pw_value_type buf;
                
                ret = nc_pm8921_itemp_get_stage(&itemp_stage);  

                buf.rsp_buf[0] = itemp_stage;
                
                if(copy_to_user((void *)pArg, &buf, sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_to_user failed");
                    return PM_ERR_FLAG__INVALID;
                }
                
                printk(KERN_INFO "DIAG_PW_PCT_OTP_STAGE_GET itemp_stage:%x", itemp_stage);
            }
            break;
        
        case IOCTL_PW_PCT_OTP_STG_OVD:
            {
                unsigned char enable;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable = buf.req_buf[0];
                
                ret = nc_pm8921_itemp_stage_override(enable);  

                printk(KERN_INFO "DIAG_PW_PCT_OTP_STG_OVD enable:%x", enable);
            }
            break;

        


        
        case IOCTL_PW_IR_RT_STATUS_GET:
            {
                
                unsigned int  rt_id = 0x00;
                unsigned int  rt_status = 0x00;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                
                rt_id = (unsigned int)buf.req_buf[0];
                rt_id += PM8921_IRQ_BASE;
                nc_pm8921_get_rt_status(rt_id, &rt_status);
                
                
                buf.rsp_buf[0] = (u8)rt_status;

                if(copy_to_user((void *)pArg, &buf, sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_to_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                printk(KERN_INFO "DIAG_PW_IR_RT_STATUS_GET rt_id:%x rt_status:%x", rt_id, rt_status);
                
            }
            break;










































































        


        
        case IOCTL_PW8821_RG_LP_CTL:
            {
                unsigned char enable;
                unsigned char vreg_id;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable  = buf.req_buf[0];
                vreg_id = buf.req_buf[1];

                printk(KERN_INFO "DIAG_PW8821_RG_LP_CTL enable:%x vreg_id:%x", enable, vreg_id);
                ret = nc_pm8xxx_lp_mode_control(PM8XXX_VERSION_8821, vreg_id, enable);
            }
            break;
        
        case IOCTL_PW8821_RG_CTL:
            {
                unsigned char enable;
                unsigned char vreg_id;
                unsigned int min_volt, max_volt;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable  = buf.req_buf[0];
                vreg_id = buf.req_buf[1];
                min_volt  = cvt_val(buf.req_buf[3]);
                min_volt += cvt_val(buf.req_buf[2]) * 100;
                min_volt *= 1000; 
                max_volt  = cvt_val(buf.req_buf[5]);
                max_volt += cvt_val(buf.req_buf[4]) * 100;
                max_volt *= 1000; 

                printk(KERN_INFO "DIAG_PW8821_RG_CTL enable  :%x vreg_id :%x", enable, vreg_id);
                printk(KERN_INFO "               min_volt:%x max_volt:%x", min_volt, max_volt);
                ret = nc_pm8xxx_vreg_control(PM8XXX_VERSION_8821, enable, vreg_id, min_volt, max_volt);
            }
            break;
        
        case IOCTL_PW8821_RG_SET_LVL:
            {
                unsigned char  vreg_id;
                unsigned int min_volt, max_volt;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                vreg_id   = buf.req_buf[0];
                min_volt  = cvt_val(buf.req_buf[2]);
                min_volt += cvt_val(buf.req_buf[1]) * 100;
                min_volt *= 1000; 
                max_volt  = cvt_val(buf.req_buf[4]);
                max_volt += cvt_val(buf.req_buf[3]) * 100;
                max_volt *= 1000; 

                printk(KERN_INFO "DIAG_PW8821_RG_SET_LVL vreg_id:%x min_volt:%x max_volt:%x", vreg_id, min_volt, max_volt);
                ret = nc_pm8xxx_vreg_set_level(PM8XXX_VERSION_8821, vreg_id, min_volt, max_volt);
            }
            break;





































































































































        
        case IOCTL_PW8821_VT_PLDWN_SW:
            {
                unsigned char enable;
                unsigned char vreg_id;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                enable  = buf.req_buf[0];
                vreg_id = buf.req_buf[1];
                
                

                printk(KERN_INFO "DIAG_PW8821_RG_SMPS_PSK enable:%x vreg_id:%x", enable, vreg_id);
                ret = nc_pm8xxx_vreg_pull_down_switch(PM8XXX_VERSION_8821, vreg_id, enable);
            }
            break;

        


        
        case IOCTL_PW8821_MPP_CNFDG_IPUT:
            {
                unsigned char mpp_port;
                unsigned char logi_level;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port   = buf.req_buf[0];
                logi_level = buf.req_buf[1];
                
                ret = nc_pm8xxx_mpp_config_digital_in(PM8821_MPP_PM_TO_SYS(mpp_port), logi_level, PM8XXX_MPP_DIN_TO_INT);
                printk(KERN_INFO "DIAG_PW8821_MPP_CNFDG_IPUT mpp_port:%x logi_level:%x", mpp_port, logi_level);
            }
            break;
        
        case IOCTL_PW8821_MPP_CNFDG_OPUT:
            {
                unsigned char mpp_port;
                unsigned char logi_level;
                unsigned char out_ctl;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port   = buf.req_buf[0];
                logi_level = buf.req_buf[1];
                out_ctl    = buf.req_buf[2];
                
               ret = nc_pm8xxx_mpp_config_digital_out(PM8821_MPP_PM_TO_SYS(mpp_port), logi_level, out_ctl);
                printk(KERN_INFO "DIAG_PW8821_MPP_CNFDG_OPUT mpp_port:%x logi_level:%x out_ctl:%x", mpp_port, logi_level, out_ctl);
            }
            break;
        
        case IOCTL_PW8821_MPP_CNFDG_IOPUT:
            {
                unsigned char mpp_port;
                unsigned char logi_level;
                unsigned char pull_set;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port   = buf.req_buf[0];
                logi_level = buf.req_buf[1];
                pull_set   = buf.req_buf[2];
                
                ret = nc_pm8xxx_mpp_config_bi_dir(PM8821_MPP_PM_TO_SYS(mpp_port), logi_level, pull_set);
                printk(KERN_INFO "DIAG_PW8821_MPP_CNFDG_IOPUT mpp_port:%x logi_level:%x pull_set:%x", mpp_port, logi_level, pull_set);
            }
            break;
        
        case IOCTL_PW8821_MPP_CNFAN_IPUT:
            {
                unsigned char mpp_port;
                unsigned char ain_chn;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port = buf.req_buf[0];
                ain_chn  = buf.req_buf[1];
                
                ret = nc_pm8xxx_mpp_config_analog_input(PM8821_MPP_PM_TO_SYS(mpp_port), ain_chn, PM8XXX_MPP_AOUT_CTRL_DISABLE);
                printk(KERN_INFO "DIAG_PW8821_MPP_CNFAN_IPUT mpp_port:%x ain_chn:%x", mpp_port, ain_chn);
            }
            break;
        
        case IOCTL_PW8821_MPP_CNFAN_OPUT:
            {
                unsigned char mpp_port;
                unsigned char aout_level;
                unsigned char pm_onoff;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port   = buf.req_buf[0];
                aout_level = buf.req_buf[1];
                pm_onoff   = buf.req_buf[2];
                
                ret = nc_pm8xxx_mpp_config_analog_output(PM8821_MPP_PM_TO_SYS(mpp_port), aout_level, pm_onoff);
                printk(KERN_INFO "DIAG_PW8821_MPP_CNFAN_OPUT mpp_port:%x aout_level:%x pm_onoff:%x", mpp_port, aout_level, pm_onoff);
            }
            break;
        
        case IOCTL_PW8821_MPP_CNF_I_SINK:
            {
                unsigned char mpp_port;
                unsigned char sink_level;
                unsigned char pm_onoff;
                ioctl_pw_value_type buf;

                if(copy_from_user(&buf,pArg,sizeof(ioctl_pw_value_type)))
                {
                    printk(KERN_ERR "copy_from_user failed");
                    return PM_ERR_FLAG__INVALID;
                }

                mpp_port   = buf.req_buf[0];
                sink_level = buf.req_buf[1];
                pm_onoff   = buf.req_buf[2];
                
                ret = nc_pm8xxx_mpp_config_current_sink(PM8821_MPP_PM_TO_SYS(mpp_port), sink_level, pm_onoff);
                printk(KERN_INFO "DIAG_PW8821_MPP_CNF_I_SINK mpp_port:%x sink_level:%x pm_onoff:%x", mpp_port, sink_level, pm_onoff);
            }
            break;















































































































































































        
        case IOCTL_PW_HW_RESET:
            printk(KERN_INFO "DVE005_FACTORY_DIAG_FTM_ONLINE_RESET_MODE");



            printk(KERN_DEBUG "[ncdiagd_power.c]%s: Goto arm_pm_restart() in \n", __func__ );
            arm_pm_restart(0, NULL);

            break;





























































































        default:
            printk(KERN_ERR "Invalid Parameter");
            return PM_ERR_FLAG__INVALID;
    }

    return ret;
}

static int ncdiagd_power_open(struct inode *ip, struct file *fp)
{
    if(access_count)
    {
        return -EBUSY;
    }
    access_count++;
    return 0;
}

static int ncdiagd_power_close(struct inode *ip, struct file *fp)
{
    printk(KERN_INFO "%s",__func__);
    access_count--;
    return 0;
}

static struct file_operations ncdiagd_power_fops = {
    .owner              = THIS_MODULE,
    .open               = ncdiagd_power_open,
    .release            = ncdiagd_power_close,
    .unlocked_ioctl     = ncdiagd_power_ioctl,
};

static struct miscdevice ncdiagd_power_dev = {
    .minor              = MISC_DYNAMIC_MINOR,
    .name               = "ncdiagd_power",
    .fops               = &ncdiagd_power_fops,
};

static int __init ncdiagd_power_init(void)
{
    int ret = 0;
    
    ret = misc_register(&ncdiagd_power_dev);
    if(ret)
    {
        printk(KERN_ERR "Fail to misc_register");
    }
    return ret;
}

static void __exit ncdiagd_power_cleanup(void)
{
    misc_deregister(&ncdiagd_power_dev);
}

late_initcall(ncdiagd_power_init);
module_exit(ncdiagd_power_cleanup);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("NEC CASIO Mobile Communications PMIC NCDIAG daemon");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:ncdiagd_power");


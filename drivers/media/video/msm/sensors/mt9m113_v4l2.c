/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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

#include "mt9m113_v4l2.h"


#define MT9M113_LOG_ERR_ON  1   
#define MT9M113_LOG_DBG_ON  0   

#if MT9M113_LOG_ERR_ON
#define MT9M113_LOG_ERR(fmt, args...) printk(KERN_ERR "mt9m113:%s(%d) " fmt, __func__, __LINE__, ##args)
#else
#define MT9M113_LOG_ERR(fmt, args...) do{}while(0)
#endif

#if MT9M113_LOG_DBG_ON
#define MT9M113_LOG_DBG(fmt, args...) printk(KERN_INFO "mt9m113:%s(%d) " fmt, __func__, __LINE__, ##args)
#else
#define MT9M113_LOG_DBG(fmt, args...) do{}while(0)
#endif

#define MT9M113_LOG_INF(fmt, args...) printk(KERN_INFO "mt9m113:%s(%d) " fmt, __func__, __LINE__, ##args)


#define MT9M113_DBG_REG_CHECK   MT9M113_LOG_DBG_ON   

static enum mt9m113_camera_mode_t mt9m113_camera_mode = MT9M113_CAMERA_MODE_PREVIEW;
static int mt9m113_frame_rate_mode = 0;
static struct msm_camera_i2c_reg_conf* mt9m113_scene_shot = NULL;
static struct msm_camera_i2c_reg_conf* mt9m113_ae_target = NULL;


struct pm_gpio mt9m113_cam2_v_en1_on = {
    .direction      = PM_GPIO_DIR_OUT,
    .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
    .output_value   = 1,
    .pull           = PM_GPIO_PULL_NO,
    .vin_sel        = PM_GPIO_VIN_S4,
    .out_strength   = PM_GPIO_STRENGTH_LOW,
    .function       = PM_GPIO_FUNC_NORMAL,
    .inv_int_pol    = 0,
    .disable_pin    = 0,
};


struct pm_gpio mt9m113_cam2_v_en1_off = {
    .direction      = PM_GPIO_DIR_OUT,
    .output_buffer  = PM_GPIO_OUT_BUF_CMOS,
    .output_value   = 0,
    .pull           = PM_GPIO_PULL_NO,
    .vin_sel        = PM_GPIO_VIN_S4,
    .out_strength   = PM_GPIO_STRENGTH_LOW,
    .function       = PM_GPIO_FUNC_NORMAL,
    .inv_int_pol    = 0,
    .disable_pin    = 0,
};

static struct msm_cam_clk_info mt9m113_cam_clk_info[] = {
    {"cam_clk", MT9M113_MCLK_FREQUENCY},
};

static struct v4l2_subdev_info mt9m113_subdev_info[] = {
    {
    .code   = V4L2_MBUS_FMT_YUYV8_2X8,
    .colorspace = V4L2_COLORSPACE_JPEG,
    .fmt    = 1,
    .order    = 0,
    },
    
};


static struct msm_sensor_output_info_t mt9m113_dimensions[] = {
    {
        .x_output = 0x0500, 
        .y_output = 0x0400, 
        .line_length_pclk = 0x0722,
        .frame_length_lines = 0x045B,
        .vt_pixel_clk = 30000000,
        .op_pixel_clk = 128000000,
        .binning_factor = 1,
    },
    {
        .x_output = 0x0500, 
        .y_output = 0x0400, 
        .line_length_pclk = 0x0722,
        .frame_length_lines = 0x045B,
        .vt_pixel_clk = 30000000,
        .op_pixel_clk = 128000000,
        .binning_factor = 1,
    },
    {
        .x_output = 0x0500, 
        .y_output = 0x0400, 
        .line_length_pclk = 0x0722,
        .frame_length_lines = 0x045B,
        .vt_pixel_clk = 30000000,
        .op_pixel_clk = 128000000,
        .binning_factor = 1,
    },
    {
        .x_output = 0x0500, 
        .y_output = 0x0400, 
        .line_length_pclk = 0x0722,
        .frame_length_lines = 0x045B,
        .vt_pixel_clk = 30000000,
        .op_pixel_clk = 128000000,
        .binning_factor = 1,
    },
};

































static enum msm_camera_vreg_name_t mt9m113_veg_seq[] = {




 };




static struct msm_sensor_output_reg_addr_t mt9m113_reg_addr = {
    
    .x_output = 0x0003,            
    .y_output = 0x0005,            
    .line_length_pclk = 0x0021,    
    .frame_length_lines = 0x001F,  
};

static struct msm_sensor_id_info_t mt9m113_id_info = {
    .sensor_id_reg_addr = 0x0000,    
    .sensor_id = 0x2480,             
};


#if MT9M113_DBG_REG_CHECK
static struct msm_camera_i2c_reg_conf mt9m113_check_settings[] = {
    {0x0000, 0x2480},
};

static void mt9m113_sensor_reg_check(struct msm_camera_i2c_reg_conf *reg_conf_tbl, uint16_t size)
{
    int i = 0;
    uint16_t val = 0xFFFF;

    MT9M113_LOG_DBG("START\n");

    for (i = 0; i < size; i++) {

        if(reg_conf_tbl[i].reg_addr == MT9M113_MCU_ADDRESS){

            msm_camera_i2c_write(mt9m113_s_ctrl.sensor_i2c_client,
                                MT9M113_MCU_ADDRESS, reg_conf_tbl[i].reg_data, MSM_CAMERA_I2C_WORD_DATA );
            mdelay(1);
            msm_camera_i2c_read(mt9m113_s_ctrl.sensor_i2c_client,
                                MT9M113_MCU_DATA0, &val, MSM_CAMERA_I2C_WORD_DATA );
            MT9M113_LOG_DBG("addr(0x%04X) : set(0x%04X) : now(0x%04X)\n",reg_conf_tbl[i].reg_data,reg_conf_tbl[i+1].reg_data,val);
            i++;
        }
        else{
            msm_camera_i2c_read(mt9m113_s_ctrl.sensor_i2c_client,
                                reg_conf_tbl[i].reg_addr, &val, MSM_CAMERA_I2C_WORD_DATA );
            MT9M113_LOG_DBG("addr(0x%04X) : set(0x%04X) : now(0x%04X)\n",reg_conf_tbl[i].reg_addr,reg_conf_tbl[i].reg_data,val);
        }

        mdelay(1);
    } 

    MT9M113_LOG_DBG("END\n");
}
#endif 


static int32_t mt9m113_sensor_reg_polling(uint16_t reg_addr, uint16_t mask, uint16_t terminate_value,
                                          uint16_t poll_interval, uint16_t poll_max, enum mt9m113_access_t reg_type)
{
    int rc = 0;
    int poll_count = 0;
    uint16_t val = 0xFFFF;

    MT9M113_LOG_DBG("START\n");

    MT9M113_LOG_DBG("reg_addr = 0x%04X\n",reg_addr);
    MT9M113_LOG_DBG("mask = 0x%04X\n",mask);
    MT9M113_LOG_DBG("terminate_value = 0x%04X\n",terminate_value);
    MT9M113_LOG_DBG("poll_interval = 0x%04X\n",poll_interval);
    MT9M113_LOG_DBG("poll_max = 0x%04X\n",poll_max);
    MT9M113_LOG_DBG("reg_type = 0x%04X\n",reg_type);
    
    
    for (poll_count = 0; poll_count < poll_max; poll_count++) {

        if( reg_type == MT9M113_ACCESS_VARIABLES ){
            
            rc = msm_camera_i2c_write(mt9m113_s_ctrl.sensor_i2c_client, 
                                      MT9M113_MCU_ADDRESS, reg_addr, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9M113_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
                return rc;
            }

            
            rc = msm_camera_i2c_read(mt9m113_s_ctrl.sensor_i2c_client, 
                                     MT9M113_MCU_DATA0, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9M113_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }

        } else { 
            
            rc = msm_camera_i2c_read(mt9m113_s_ctrl.sensor_i2c_client, 
                                     reg_addr, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9M113_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }

        }

        MT9M113_LOG_DBG("val=%d(0x%04X)\n",val,val);

        if ( (val & mask) == terminate_value ) {
            
            break;
        } else {
            
            val = 0xFFFF;
        }

        
        mdelay(poll_interval);
    }

    if(poll_count >= poll_max){
        MT9M113_LOG_ERR("failed addr=0x%04X, poll_count=%d(times), timeout=%d(ms)\n",reg_addr, poll_count , poll_count*poll_interval );
        rc = -EFAULT;
    }else {
        MT9M113_LOG_DBG("success addr=0x%04X, poll_count=%d(times), timeout=%d(ms)\n",reg_addr, poll_count+1 , (poll_count+1)*poll_interval );
        rc = 0;
    }

    MT9M113_LOG_DBG("END(%d)\n",rc);
    return rc;
}





static int32_t mt9m113_set_scene(struct msm_sensor_ctrl_t *s_ctrl, int scene)
{
    int32_t rc = 0;



















































    return rc;
}





static int32_t mt9m113_set_wb(struct msm_sensor_ctrl_t *s_ctrl, uint8_t wb_val)
{
    int32_t rc = 0;

    struct msm_camera_i2c_reg_conf*  setting_data=NULL;
    uint16_t setting_size = 0;

    MT9M113_LOG_DBG("START\n");
    MT9M113_LOG_DBG("wb_val = %d\n",wb_val);

    switch (wb_val)
    {
        case CAMERA_WB_AUTO:
            setting_data = mt9m113_wb_auto_settings;
            setting_size = ARRAY_SIZE(mt9m113_wb_auto_settings);
            break;

        case CAMERA_WB_DAYLIGHT:
            setting_data = mt9m113_wb_daylight_settings;
            setting_size = ARRAY_SIZE(mt9m113_wb_daylight_settings);
            break;

        case CAMERA_WB_CLOUDY_DAYLIGHT:
            setting_data = mt9m113_wb_cloudy_settings;
            setting_size = ARRAY_SIZE(mt9m113_wb_cloudy_settings);
            break;

        case CAMERA_WB_INCANDESCENT:
            setting_data = mt9m113_wb_incandescent_settings;
            setting_size = ARRAY_SIZE(mt9m113_wb_incandescent_settings);
            break;

        case CAMERA_WB_FLUORESCENT:
        case CAMERA_WB_FLUORESCENT_H:
            setting_data = mt9m113_wb_fluorescent_daylight_settings;
            setting_size = ARRAY_SIZE(mt9m113_wb_fluorescent_daylight_settings);
            break;

        case CAMERA_WB_FLUORESCENT_L:
            setting_data = mt9m113_wb_fluorescent_white_settings;
            setting_size = ARRAY_SIZE(mt9m113_wb_fluorescent_white_settings);
            break;

        default:
        MT9M113_LOG_DBG("NOT SUPPORTED\n");
            break;
    }

    if(setting_data)
    {
        rc = msm_camera_i2c_write_tbl(
                s_ctrl->sensor_i2c_client,
                setting_data, setting_size,
                MSM_CAMERA_I2C_WORD_DATA );
        

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:i2c_write (rc = %d)\n",rc);
            return rc;
        }
    }

    MT9M113_LOG_DBG("END(%d)\n",rc);
    return rc;
}




static int32_t mt9m113_set_effect(struct msm_sensor_ctrl_t *s_ctrl, int8_t effect)
{
    int32_t rc = 0;

    struct msm_camera_i2c_reg_conf*  setting_data=NULL;
    uint16_t setting_size = 0;


    MT9M113_LOG_DBG("START\n");
    MT9M113_LOG_DBG("effect = %d\n",effect);

    switch (effect)
    {
        case CAMERA_EFFECT_OFF:
            setting_data = mt9m113_effect_off_settings;
            setting_size = ARRAY_SIZE(mt9m113_effect_off_settings);
            break;

        case CAMERA_EFFECT_MONO:
            setting_data = mt9m113_effect_mono_settings;
            setting_size = ARRAY_SIZE(mt9m113_effect_mono_settings);
            break;

        case CAMERA_EFFECT_SEPIA:
            setting_data = mt9m113_effect_sepia_settings;
            setting_size = ARRAY_SIZE(mt9m113_effect_sepia_settings);
            break;

        default:
        MT9M113_LOG_DBG("NOT SUPPORTED\n");
            break;
    }

    if(setting_data)
    {
        rc = msm_camera_i2c_write_tbl(
                s_ctrl->sensor_i2c_client,
                setting_data, setting_size,
                MSM_CAMERA_I2C_WORD_DATA );

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:i2c_write (rc = %d)\n",rc);
            return rc;
        }

        rc = mt9m113_sensor_reg_polling(
                MT9M113_POLL_RFRSH_ADDR,      MT9M113_POLL_RFRSH_MASK,
                MT9M113_POLL_RFRSH_TERMINATE, MT9M113_POLL_RFRSH_INTERVAL,
                MT9M113_POLL_RFRSH_RETRY_100, MT9M113_ACCESS_VARIABLES);

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:reg_polling (rc = %d)\n",rc);
            return rc;
        }
    }

    MT9M113_LOG_DBG("END(%d)\n",rc);
    return rc;
}




static int32_t mt9m113_set_antibanding(struct msm_sensor_ctrl_t *s_ctrl, uint16_t antibanding)
{
    int32_t rc = 0;

    struct msm_camera_i2c_reg_conf*  setting_data=NULL;
    uint16_t setting_size = 0;


    MT9M113_LOG_DBG("START\n");
    MT9M113_LOG_DBG("antibanding = %d\n",antibanding);

    switch (antibanding)
    {
        case CAMERA_ANTIBANDING_AUTO:
            setting_data = mt9m113_antibanding_auto_settings;
            setting_size = ARRAY_SIZE(mt9m113_antibanding_auto_settings);
            break;

        case CAMERA_ANTIBANDING_50HZ:
            setting_data = mt9m113_antibanding_50Hz_settings;
            setting_size = ARRAY_SIZE(mt9m113_antibanding_50Hz_settings);
            break;

        case CAMERA_ANTIBANDING_60HZ:
            setting_data = mt9m113_antibanding_60Hz_settings;
            setting_size = ARRAY_SIZE(mt9m113_antibanding_60Hz_settings);
            break;

        case CAMERA_ANTIBANDING_OFF:
        default:
        MT9M113_LOG_DBG("NOT SUPPORTED\n");
            break;
    }

    if(setting_data)
    {
        rc = msm_camera_i2c_write_tbl(
                s_ctrl->sensor_i2c_client,
                setting_data, setting_size,
                MSM_CAMERA_I2C_WORD_DATA );

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:i2c_write (rc = %d)\n",rc);
            return rc;
        }

        rc = mt9m113_sensor_reg_polling(
                MT9M113_POLL_RFRSH_ADDR,      MT9M113_POLL_RFRSH_MASK,
                MT9M113_POLL_RFRSH_TERMINATE, MT9M113_POLL_RFRSH_INTERVAL,
                MT9M113_POLL_RFRSH_RETRY_100, MT9M113_ACCESS_VARIABLES);

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:reg_polling (rc = %d)\n",rc);
            return rc;
        }
    }

    MT9M113_LOG_DBG("END(%d)\n",rc);
    return rc;
}





static int32_t mt9m113_set_exp_compensation(struct msm_sensor_ctrl_t *s_ctrl, int8_t exposure)
{
    int32_t rc = 0;

    MT9M113_LOG_DBG("START\n");
    MT9M113_LOG_DBG("exposure = %d\n",exposure);

    if( (exposure < 0) || (ARRAY_SIZE(mt9m113_brightness_confs) <= exposure) )
    {
        MT9M113_LOG_ERR("ERROR: INVALID VALUE(%d)\n", exposure);
        return -EFAULT;
    }

    rc = msm_sensor_write_conf_array(
        s_ctrl->sensor_i2c_client,
        mt9m113_brightness_confs, exposure);

    if(rc < 0){
        MT9M113_LOG_ERR("ERROR:i2c_write (rc = %d)\n",rc);
        return rc;
    }

    rc = mt9m113_sensor_reg_polling(
            MT9M113_POLL_RFRSH_ADDR,      MT9M113_POLL_RFRSH_MASK,
            MT9M113_POLL_RFRSH_TERMINATE, MT9M113_POLL_RFRSH_INTERVAL,
            MT9M113_POLL_RFRSH_RETRY_100, MT9M113_ACCESS_VARIABLES);

    if(rc < 0){
        MT9M113_LOG_ERR("ERROR:reg_polling (rc = %d)\n",rc);
        return rc;
    }

    MT9M113_LOG_DBG("END(%d)\n",rc);
    return rc;
}





static int32_t mt9m113_get_maker_note(struct msm_sensor_ctrl_t *s_ctrl,
                                      struct get_exif_maker_note_cfg *get_exif_maker_note)
{
    int32_t rc = 0;
    int cnt = 0;
    uint16_t val = 0xFFFF;

    uint16_t device_id =0;            
    uint16_t fd_freq =0;              
    uint16_t awb_temp =0;             
    uint16_t awb_gain_r =0;           
    uint16_t awb_gain_g =0;           
    uint16_t awb_gain_b =0;           
    uint16_t awb_saturation =0;       

    struct reg_access_param_t reg_access_params[]={
        {MT9M113_ACCESS_VARIABLES, 0xA404, &fd_freq       },
        {MT9M113_ACCESS_VARIABLES, 0xA353, &awb_temp      },
        {MT9M113_ACCESS_VARIABLES, 0xA34E, &awb_gain_r    },
        {MT9M113_ACCESS_VARIABLES, 0xA34F, &awb_gain_g    },
        {MT9M113_ACCESS_VARIABLES, 0xA350, &awb_gain_b    },
        {MT9M113_ACCESS_VARIABLES, 0xA354, &awb_saturation},
    };

    MT9M113_LOG_DBG("START\n");

    for(cnt=0; cnt < ARRAY_SIZE(reg_access_params); cnt++)
    {
        if(reg_access_params[cnt].reg_type == MT9M113_ACCESS_VARIABLES){
        
            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 
                                      MT9M113_MCU_ADDRESS, reg_access_params[cnt].reg_addr, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9M113_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
                return rc;
            }
        
            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                                     MT9M113_MCU_DATA0, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9M113_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }
        } else { 
            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                                     reg_access_params[cnt].reg_addr, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9M113_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }
        }
        
        *reg_access_params[cnt].reg_data = val;
    }

    if( fd_freq & (1<<7) ){
        
        fd_freq = 0x00;
    } else{
        
        fd_freq = (fd_freq>>5)&1 ? 0x02 : 0x03;
    }

    device_id = 0x2480;     

    get_exif_maker_note->device_id      = device_id;    
    get_exif_maker_note->fd_freq        = fd_freq;
    get_exif_maker_note->awb_temp       = awb_temp;
    get_exif_maker_note->awb_gain_r     = awb_gain_r;
    get_exif_maker_note->awb_gain_g     = awb_gain_g;
    get_exif_maker_note->awb_gain_b     = awb_gain_b;
    get_exif_maker_note->awb_saturation = awb_saturation;













    MT9M113_LOG_DBG("END(%d)\n",rc);
    return rc;
}




static int32_t mt9m113_get_exif_param(struct msm_sensor_ctrl_t *s_ctrl,
                                      struct get_exif_param_inf *get_exif_param)
{
    int32_t rc = 0;
    int cnt = 0;
    uint16_t val = 0xFFFF;

    uint16_t coarse_integration_time;      
    uint16_t line_length_DVE046;              
    uint16_t fine_integration_time;        
    uint16_t analog_gain_code_global;      
    uint16_t digital_gain_greenr;          

    struct reg_access_param_t reg_access_params[]={
        {MT9M113_ACCESS_REGISTERS, 0x3012, &coarse_integration_time},
        {MT9M113_ACCESS_REGISTERS, 0x300C, &line_length_DVE046        },
        {MT9M113_ACCESS_REGISTERS, 0x3014, &fine_integration_time  },
        {MT9M113_ACCESS_REGISTERS, 0x3028, &analog_gain_code_global},



        {MT9M113_ACCESS_VARIABLES, 0x221F, &digital_gain_greenr    },

    };

    MT9M113_LOG_DBG("START\n");

    for(cnt=0; cnt < ARRAY_SIZE(reg_access_params); cnt++)
    {
        if(reg_access_params[cnt].reg_type == MT9M113_ACCESS_VARIABLES){
        
            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 
                                      MT9M113_MCU_ADDRESS, reg_access_params[cnt].reg_addr, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9M113_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
                return rc;
            }
        
            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                                     MT9M113_MCU_DATA0, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9M113_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }
        } else { 
            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                                     reg_access_params[cnt].reg_addr, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9M113_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }
        }
        
        *reg_access_params[cnt].reg_data = val;
    }

    get_exif_param->coarse_integration_time = coarse_integration_time;
    get_exif_param->line_length_DVE046         = line_length_DVE046;
    get_exif_param->fine_integration_time   = fine_integration_time;
    get_exif_param->analog_gain_code_global = analog_gain_code_global;
    get_exif_param->digital_gain_greenr     = digital_gain_greenr;











    MT9M113_LOG_DBG("END(%d)\n",rc);
    return rc;
}





static int32_t mt9m113_set_scene_shot(int frame_rate_mode)
{
    int32_t rc = 0;

    struct msm_camera_i2c_reg_conf*  setting_data=NULL;
    uint16_t setting_size = 0;

    MT9M113_LOG_DBG("START\n");
    MT9M113_LOG_DBG("frame_rate_mode = %d\n",frame_rate_mode);

    switch (frame_rate_mode)
    {
        case CAMERA_FPS_MODE_QUICK_SCENE_OFF_SHAKE_AUTO:
            
            setting_data = mt9m113_fps_mode_auto_human_on_preview_settings;
            setting_size = ARRAY_SIZE(mt9m113_fps_mode_auto_human_on_preview_settings);
            MT9M113_LOG_DBG("[Scene:Auto+Human  AS:On-Preview]\n");
            break;

        case CAMERA_FPS_MODE_QUICK_SCENE_OFF_SHAKE_OFF:
        case CAMERA_FPS_MODE_BEST_SCENE_OFF_SHAKE_OFF:
        case CAMERA_FPS_MODE_BEST_SCENE_AUTO_SHAKE_OFF:
        case CAMERA_FPS_MODE_BEST_SCENE_PORT_SHAKE_OFF:
            
            setting_data = mt9m113_fps_mode_auto_human_off_preview_settings;
            setting_size = ARRAY_SIZE(mt9m113_fps_mode_auto_human_off_preview_settings);
            MT9M113_LOG_DBG("[Scene:Auto+Human  AS:Off-Preview]\n");
            break;

        case CAMERA_FPS_MODE_MOVIE_SCENE_OFF:
            
            setting_data = mt9m113_fps_mode_auto_human_off_preview_settings;
            setting_size = ARRAY_SIZE(mt9m113_fps_mode_auto_human_off_preview_settings);
            MT9M113_LOG_DBG("[Scene:Auto+Human  AS:Off-Preview]\n");
            break;

        case CAMERA_FPS_MODE_STD_SCENE_OFF_SHAKE_AUTO:
        case CAMERA_FPS_MODE_STD_SCENE_AUTO_SHAKE_AUTO:
        case CAMERA_FPS_MODE_STD_SCENE_PORT_SHAKE_AUTO:
            
            setting_data = mt9m113_fps_mode_auto_human_on_preview_settings;
            setting_size = ARRAY_SIZE(mt9m113_fps_mode_auto_human_on_preview_settings);
            MT9M113_LOG_DBG("[Scene:Auto+Human  AS:On-Preview]\n");
            break;

        case CAMERA_FPS_MODE_STD_SCENE_OFF_SHAKE_OFF:
        case CAMERA_FPS_MODE_STD_SCENE_AUTO_SHAKE_OFF:
        case CAMERA_FPS_MODE_STD_SCENE_PORT_SHAKE_OFF:
        case CAMERA_FPS_MODE_HDR_SCENE_OFF_SHAKE_OFF:
            
            setting_data = mt9m113_fps_mode_auto_human_off_preview_settings;
            setting_size = ARRAY_SIZE(mt9m113_fps_mode_auto_human_off_preview_settings);
            MT9M113_LOG_DBG("[Scene:Auto+Human  AS:Off-Preview]\n");
            break;

















        default:
            MT9M113_LOG_DBG("NOT SUPPORTED\n");
            break;
    }


    if(mt9m113_scene_shot != setting_data)
    {
        MT9M113_LOG_DBG("mt9m113_scene_shot != setting_data\n");

        rc = msm_camera_i2c_write_tbl(
                mt9m113_s_ctrl.sensor_i2c_client,
                setting_data, setting_size,
                MSM_CAMERA_I2C_WORD_DATA );

        if (rc < 0) {
            MT9M113_LOG_ERR("ERROR:i2c_write (rc = %d)\n",rc);
            return rc;
        }

        
        rc = msm_sensor_write_conf_array(
                    mt9m113_s_ctrl.sensor_i2c_client,
                    mt9m113_refresh_confs, MT9M113_REFRESH_CMD_SETTING);

        if (rc < 0) {
            MT9M113_LOG_ERR("ERROR:i2c_write (rc = %d)\n",rc);
            return rc;
        }

        rc = mt9m113_sensor_reg_polling(MT9M113_POLL_RFRSH_ADDR,      MT9M113_POLL_RFRSH_MASK,
                                        MT9M113_POLL_RFRSH_TERMINATE, MT9M113_POLL_RFRSH_INTERVAL,
                                        MT9M113_POLL_RFRSH_RETRY_50,  MT9M113_ACCESS_VARIABLES);
        if (rc < 0) {
            MT9M113_LOG_ERR("ERROR:reg_polling (rc = %d)\n",rc);
            return rc;
        }

        mt9m113_scene_shot = setting_data;
    }

    MT9M113_LOG_DBG("END(%d)\n",rc);
    return rc;
}





static int32_t mt9m113_set_frame_rate_mode(struct msm_sensor_ctrl_t *s_ctrl, int frame_rate_mode)
{
    int32_t rc = 0;

    MT9M113_LOG_DBG("START\n");

    mt9m113_frame_rate_mode = frame_rate_mode;

    rc = mt9m113_set_scene_shot(mt9m113_frame_rate_mode);

    if(rc < 0){
        MT9M113_LOG_ERR("ERROR:mt9m113_set_scene_shot (rc = %d)\n",rc);
        return rc;
    }

    MT9M113_LOG_DBG("END(%d)\n",rc);
    return rc;
}





static int32_t mt9m113_get_hdr_brightness(struct msm_sensor_ctrl_t *s_ctrl,
                                            struct hdr_brightness_t *hdr_brightness)
{
    int32_t rc = 0;
    int cnt = 0;
    uint16_t val = 0xFFFF;

    uint16_t coarse_integration_time;      
    uint16_t line_length_DVE046;              
    uint16_t fine_integration_time;        

    struct reg_access_param_t reg_access_params[]={
        {MT9M113_ACCESS_REGISTERS, 0x3012, &coarse_integration_time  },
        {MT9M113_ACCESS_REGISTERS, 0x300C, &line_length_DVE046          },
        {MT9M113_ACCESS_REGISTERS, 0x3014, &fine_integration_time    },
    };

    MT9M113_LOG_DBG("START\n");

    for(cnt=0; cnt < ARRAY_SIZE(reg_access_params); cnt++)
    {
        if(reg_access_params[cnt].reg_type == MT9M113_ACCESS_VARIABLES){
        
            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 
                                      MT9M113_MCU_ADDRESS, reg_access_params[cnt].reg_addr, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9M113_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
                return rc;
            }
        
            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                                     MT9M113_MCU_DATA0, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9M113_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }
        } else { 
            rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                                     reg_access_params[cnt].reg_addr, &val, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9M113_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
                return rc;
            }
        }
        
        *reg_access_params[cnt].reg_data = val;
    }

    hdr_brightness->coarse_integration_time   = coarse_integration_time;
    hdr_brightness->line_length_DVE046           = line_length_DVE046;
    hdr_brightness->fine_integration_time     = fine_integration_time;

    MT9M113_LOG_DBG("END(%d)\n",rc);
    return rc;
}





static int32_t mt9m113_set_hdr_brightness(struct msm_sensor_ctrl_t *s_ctrl,
                                            struct hdr_brightness_t hdr_brightness)
{
    int32_t rc = 0;
    int cnt = 0;

    uint16_t coarse_integration_time   = hdr_brightness.coarse_integration_time;      

    struct reg_access_param_t reg_access_params[]={
        {MT9M113_ACCESS_REGISTERS, 0x3012, &coarse_integration_time  },
    };

    MT9M113_LOG_DBG("START\n");

    for(cnt=0; cnt < ARRAY_SIZE(reg_access_params); cnt++)
    {
        if(reg_access_params[cnt].reg_type == MT9M113_ACCESS_VARIABLES){
        
            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 
                                      MT9M113_MCU_ADDRESS, reg_access_params[cnt].reg_addr, MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9M113_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
                return rc;
            }
        
            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 
                                     MT9M113_MCU_DATA0, *(reg_access_params[cnt].reg_data), MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9M113_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
                return rc;
            }
        } else { 
            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 
                                     reg_access_params[cnt].reg_addr, *(reg_access_params[cnt].reg_data), MSM_CAMERA_I2C_WORD_DATA);
            if (rc < 0) {
                MT9M113_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
                return rc;
            }
        }
    }

    MT9M113_LOG_DBG("END(%d)\n",rc);
    return rc;
}





static int32_t mt9m113_set_cap_mode_enable(struct msm_sensor_ctrl_t *s_ctrl, int cap_mode_enable)
{
    int32_t rc = 0;

    struct msm_camera_i2c_reg_conf*  setting_data=NULL;
    uint16_t setting_size = 0;


    MT9M113_LOG_DBG("START\n");
    MT9M113_LOG_DBG("cap_mode_enable = %d\n",cap_mode_enable);

    switch (cap_mode_enable)
    {
        case CAMERA_CAP_MODE_ENABLE_ON:
            setting_data = mt9m113_cap_mode_enable_on_settings;
            setting_size = ARRAY_SIZE(mt9m113_cap_mode_enable_on_settings);
            break;

        case CAMERA_CAP_MODE_ENABLE_OFF:
            setting_data = mt9m113_cap_mode_enable_off_settings;
            setting_size = ARRAY_SIZE(mt9m113_cap_mode_enable_off_settings);
            break;

        default:
        MT9M113_LOG_DBG("NOT SUPPORTED\n");
            break;
    }

    if(setting_data)
    {
        rc = msm_camera_i2c_write_tbl(
                s_ctrl->sensor_i2c_client,
                setting_data, setting_size,
                MSM_CAMERA_I2C_WORD_DATA );

        if (rc < 0) {
            MT9M113_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
            return rc;
        }
    }

    rc = msm_sensor_write_conf_array(
                mt9m113_s_ctrl.sensor_i2c_client,
                mt9m113_refresh_confs, MT9M113_REFRESH_CMD_SETTING);

    if (rc < 0) {
        MT9M113_LOG_ERR("ERROR:i2c_write (rc = %d)\n",rc);
        return rc;
    }

    rc = mt9m113_sensor_reg_polling(MT9M113_POLL_RFRSH_ADDR,      MT9M113_POLL_RFRSH_MASK,
                                    MT9M113_POLL_RFRSH_TERMINATE, MT9M113_POLL_RFRSH_INTERVAL,
                                    MT9M113_POLL_RFRSH_RETRY_50,  MT9M113_ACCESS_VARIABLES);
    if (rc < 0) {
        MT9M113_LOG_ERR("ERROR:reg_polling (rc = %d)\n",rc);
        return rc;
    }

    MT9M113_LOG_DBG("END(%d)\n",rc);
    return rc;
}





static int32_t mt9m113_get_device_id(struct msm_sensor_ctrl_t *s_ctrl, uint16_t *device_id)
{
    int32_t rc = 0;
    uint16_t chipid = 0xFFFF;

    MT9M113_LOG_DBG("START\n");

    rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 
                             s_ctrl->sensor_id_info->sensor_id_reg_addr, &chipid, 
                             MSM_CAMERA_I2C_WORD_DATA);
    if (rc < 0) {
        MT9M113_LOG_ERR("msm_camera_i2c_read failed rc=%d\n", rc);
        return rc;
    }

    *device_id = chipid;

    MT9M113_LOG_DBG("chipid=%d\n",chipid);
    MT9M113_LOG_DBG("END(%d)\n",rc);
    return rc;
}





static int32_t mt9m113_set_ae_target(enum mt9m113_camera_mode_t camera_mode)
{
    int32_t rc = 0;
    struct msm_camera_i2c_reg_conf*  setting_data=NULL;
    uint16_t setting_size = 0;

    MT9M113_LOG_DBG("START\n");
    MT9M113_LOG_DBG("camera_mode = %d\n",camera_mode);

    switch (camera_mode)
    {
        case MT9M113_CAMERA_MODE_VIDEO:
            setting_data = mt9m113_ae_target_video_on_settings;
            setting_size = ARRAY_SIZE(mt9m113_ae_target_video_on_settings);
            MT9M113_LOG_DBG("case MT9M113_CAMERA_MODE_VIDEO\n");
            break;

        default:
            setting_data = mt9m113_ae_target_video_off_settings;
            setting_size = ARRAY_SIZE(mt9m113_ae_target_video_off_settings);
            MT9M113_LOG_DBG("default ( not MT9M113_CAMERA_MODE_VIDEO)\n");
            break;
    }

    if(mt9m113_ae_target != setting_data)
    {
        MT9M113_LOG_DBG("mt9m113_ae_target != setting_data\n");

        rc = msm_camera_i2c_write_tbl(
                mt9m113_s_ctrl.sensor_i2c_client,
                setting_data, setting_size,
                MSM_CAMERA_I2C_WORD_DATA );

        if (rc < 0) {
            MT9M113_LOG_ERR("msm_camera_i2c_write failed rc=%d\n", rc);
            return rc;
        }

        if(camera_mode == MT9M113_CAMERA_MODE_VIDEO)
        {
            
            rc = msm_sensor_write_conf_array(
                        mt9m113_s_ctrl.sensor_i2c_client,
                        mt9m113_refresh_confs, MT9M113_REFRESH_CMD_SETTING);

            if (rc < 0) {
                MT9M113_LOG_ERR("ERROR:i2c_write (rc = %d)\n",rc);
                return rc;
            }

            rc = mt9m113_sensor_reg_polling(MT9M113_POLL_RFRSH_ADDR,      MT9M113_POLL_RFRSH_MASK,
                                            MT9M113_POLL_RFRSH_TERMINATE, MT9M113_POLL_RFRSH_INTERVAL,
                                            MT9M113_POLL_RFRSH_RETRY_50,  MT9M113_ACCESS_VARIABLES);
            if (rc < 0) {
                MT9M113_LOG_ERR("ERROR:reg_polling (rc = %d)\n",rc);
                return rc;
            }
        }

        mt9m113_ae_target = setting_data;
    }

    MT9M113_LOG_DBG("END(%d)\n",rc);
    return rc;
}


static void mt9m113_sensor_start_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
    
}


static void mt9m113_sensor_stop_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
    
}


static int32_t mt9m113_sensor_write_res_settings(struct msm_sensor_ctrl_t *s_ctrl,
    uint16_t res)
{
    int32_t rc = 0;














    MT9M113_LOG_DBG("DO NOTHING(%d)\n",rc);


    return rc;
}


static int32_t mt9m113_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
                                      int update_type, int res)
{
    int32_t rc = 0;

    MT9M113_LOG_DBG("START\n");

    msleep(30);

    if (update_type == MSM_SENSOR_REG_INIT) {

        MT9M113_LOG_DBG("MSM_SENSOR_REG_INIT\n");

        mt9m113_scene_shot = NULL;
        mt9m113_ae_target = NULL;






















        rc = msm_sensor_write_all_conf_array(
                s_ctrl->sensor_i2c_client,
                mt9m113_init_confs,
                ARRAY_SIZE(mt9m113_init_confs) );

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:i2c_write (rc = %d)\n",rc);
            return rc;
        }

        rc = mt9m113_sensor_reg_polling(
                MT9M113_POLL_STNDBY_ADDR,      MT9M113_POLL_STNDBY_MASK,
                MT9M113_POLL_STNDBY_TERMINATE, MT9M113_POLL_STNDBY_INTERVAL, 
                MT9M113_POLL_STNDBY_RETRY,     MT9M113_ACCESS_REGISTERS);

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:reg_polling (rc = %d)\n",rc);
            return rc;
        }

        msleep(5);

        rc = msm_sensor_write_all_conf_array(
                s_ctrl->sensor_i2c_client,
                mt9m113_init_confs2,
                ARRAY_SIZE(mt9m113_init_confs2) );

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:i2c_write (rc = %d)\n",rc);
            return rc;
        }

        rc = msm_sensor_write_all_conf_array(
                s_ctrl->sensor_i2c_client,
                mt9m113_init_confs3,
                ARRAY_SIZE(mt9m113_init_confs3) );

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:i2c_write (rc = %d)\n",rc);
            return rc;
        }

        rc = mt9m113_sensor_reg_polling(
                MT9M113_POLL_STRM_ADDR,      MT9M113_POLL_STRM_MASK,
                MT9M113_POLL_STRM_TERMINATE, MT9M113_POLL_STRM_INTERVAL,
                MT9M113_POLL_STRM_RETRY,     MT9M113_ACCESS_REGISTERS);

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:reg_polling (rc = %d)\n",rc);
            return rc;
        }

        rc = msm_sensor_write_all_conf_array(
                s_ctrl->sensor_i2c_client,
                mt9m113_init_confs4,
                ARRAY_SIZE(mt9m113_init_confs4) );

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:i2c_write (rc = %d)\n",rc);
            return rc;
        }

        rc = msm_sensor_write_all_conf_array(
                s_ctrl->sensor_i2c_client,
                mt9m113_init_confs5,
                ARRAY_SIZE(mt9m113_init_confs5) );

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:i2c_write (rc = %d)\n",rc);
            return rc;
        }

        
        rc = msm_sensor_write_conf_array(
                    mt9m113_s_ctrl.sensor_i2c_client,
                    mt9m113_refresh_confs, MT9M113_REFRESH_MODE_SETTING);

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:i2c_write (rc = %d)\n",rc);
            return rc;
        }

        rc = mt9m113_sensor_reg_polling(
                MT9M113_POLL_RFRSH_ADDR,      MT9M113_POLL_RFRSH_MASK,
                MT9M113_POLL_RFRSH_TERMINATE, MT9M113_POLL_RFRSH_INTERVAL,
                MT9M113_POLL_RFRSH_RETRY_50,  MT9M113_ACCESS_VARIABLES);

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:reg_polling (rc = %d)\n",rc);
            return rc;
        }

        
        rc = msm_sensor_write_conf_array(
                    mt9m113_s_ctrl.sensor_i2c_client,
                    mt9m113_refresh_confs, MT9M113_REFRESH_CMD_SETTING);

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:i2c_write (rc = %d)\n",rc);
            return rc;
        }

        rc = mt9m113_sensor_reg_polling(
                MT9M113_POLL_RFRSH_ADDR,      MT9M113_POLL_RFRSH_MASK,
                MT9M113_POLL_RFRSH_TERMINATE, MT9M113_POLL_RFRSH_INTERVAL,
                MT9M113_POLL_RFRSH_RETRY_50,  MT9M113_ACCESS_VARIABLES);

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:reg_polling (rc = %d)\n",rc);
            return rc;
        }

    } else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {

        MT9M113_LOG_DBG("MSM_SENSOR_UPDATE_PERIODIC\n");
        MT9M113_LOG_DBG("res = %d\n",res);

        rc = mt9m113_sensor_write_res_settings(s_ctrl, res);

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:res_settings (rc = %d)\n",rc);
            return rc;
        }

        switch(res){
            case MSM_SENSOR_RES_FULL:   
                mt9m113_camera_mode = MT9M113_CAMERA_MODE_CAPTURE;
                break;
            case MSM_SENSOR_RES_QTR:    
                mt9m113_camera_mode = MT9M113_CAMERA_MODE_PREVIEW;
          break;
            case MSM_SENSOR_RES_2:      
                mt9m113_camera_mode = MT9M113_CAMERA_MODE_VIDEO;
                break;
            case MSM_SENSOR_RES_3:      
                mt9m113_camera_mode = MT9M113_CAMERA_MODE_ZSL;
                break;
            default:
                MT9M113_LOG_ERR("ERROR:res(%d) is not supported.\n",res);
                return -EFAULT;
        }

        rc = mt9m113_set_scene_shot(mt9m113_frame_rate_mode);

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:mt9m113_set_scene_shot (rc = %d)\n",rc);
            return rc;
        }

        rc = mt9m113_set_ae_target(mt9m113_camera_mode);

        if(rc < 0){
            MT9M113_LOG_ERR("ERROR:mt9m113_set_ae_target (rc = %d)\n",rc);
            return rc;
        }



























        v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
            NOTIFY_PCLK_CHANGE, &s_ctrl->msm_sensor_reg->
            output_settings[res].op_pixel_clk);
        s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
        msleep(30);

#if MT9M113_DBG_REG_CHECK
        mt9m113_sensor_reg_check(mt9m113_check_settings,ARRAY_SIZE(mt9m113_check_settings));
#endif 

    }

    MT9M113_LOG_DBG("END(%d)\n",rc);
    return rc;
}

static int mt9m113_i2c_probe(struct i2c_client *client,
                const struct i2c_device_id *id)
{
    int rc = 0;

    MT9M113_LOG_DBG("START\n");

    rc = msm_sensor_i2c_probe(client, id);

    MT9M113_LOG_DBG("END(%d)\n",rc);
    return rc;
}


static const struct i2c_device_id mt9m113_i2c_id[] = {
    {SENSOR_NAME, (kernel_ulong_t)&mt9m113_s_ctrl},
    { }
};


static struct i2c_driver mt9m113_i2c_driver = {
    .id_table = mt9m113_i2c_id,
    .probe  = mt9m113_i2c_probe,
    .driver = {
        .name = SENSOR_NAME,
    },
};


static struct msm_camera_i2c_client mt9m113_sensor_i2c_client = {
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    
};


static int mt9m113_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
    MT9M113_LOG_DBG("START\n");

    
    gpio_direction_output(MT9M113_GPIO_CAM2_RST_N, 0);
    gpio_free(MT9M113_GPIO_CAM2_RST_N);
    MT9M113_LOG_DBG("CAM2_RST_N(%d) Low\n", MT9M113_GPIO_CAM2_RST_N);

    
    MT9M113_LOG_DBG("WAIT %d[ms]\n",MT9M113_WAIT_PWOFF_RST_L);
    mdelay( MT9M113_WAIT_PWOFF_RST_L );

    





    msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
                       mt9m113_cam_clk_info, s_ctrl->cam_clk,
                       ARRAY_SIZE(mt9m113_cam_clk_info), 0);

    MT9M113_LOG_DBG("MCLK DISABLE\n");

    
    MT9M113_LOG_DBG("WAIT %d[ms]\n",MT9M113_WAIT_PWOFF_MCLK);
    mdelay( MT9M113_WAIT_PWOFF_MCLK );

    
    pm8xxx_gpio_config(MT9M113_PMGPIO_VDD_CAM2_V_EN1, &mt9m113_cam2_v_en1_off);
    MT9M113_LOG_DBG("VDD_CAM2_V_EN1'(%d) Low\n", MT9M113_PMGPIO_VDD_CAM2_V_EN1);

    
    MT9M113_LOG_DBG("WAIT %d[ms]\n",MT9M113_WAIT_PWOFF_V_EN1);
    mdelay( MT9M113_WAIT_PWOFF_V_EN1 );

    
    pm8xxx_gpio_config(MT9M113_PMGPIO_VDD_CAM2_V_EN2, &mt9m113_cam2_v_en1_off);
    MT9M113_LOG_DBG("VDD_CAM2_V_EN2'(%d) Low\n", MT9M113_PMGPIO_VDD_CAM2_V_EN2);

    MT9M113_LOG_DBG("END(0)\n");

    return 0;
}

static int32_t mt9m113_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc = 0;

    MT9M113_LOG_DBG("START\n");

    
    rc = gpio_request(MT9M113_GPIO_CAM2_RST_N, SENSOR_NAME);
    if (!rc) {
        gpio_direction_output(MT9M113_GPIO_CAM2_RST_N, 1);
    } else {
        
        MT9M113_LOG_ERR("CAM2_RST_N(%d) Error, rc = %d\n", MT9M113_GPIO_CAM2_RST_N, rc);
	  goto gpio_34_request_err;
    }
    MT9M113_LOG_DBG("CAM2_RST_N(%d) High\n", MT9M113_GPIO_CAM2_RST_N);

    
    MT9M113_LOG_DBG("WAIT %d[ms]\n",MT9M113_WAIT_PWON_RST_H);
    mdelay( MT9M113_WAIT_PWON_RST_H );


    
    rc = pm8xxx_gpio_config(MT9M113_PMGPIO_VDD_CAM2_V_EN2, &mt9m113_cam2_v_en1_on);

    if (rc) {
        
        MT9M113_LOG_ERR("VDD_CAM2_V_EN2(%d) Error, rc = %d\n", MT9M113_PMGPIO_VDD_CAM2_V_EN2, rc);
        goto pmgpio_42_config_err;
    }
    MT9M113_LOG_DBG("VDD_CAM2_V_EN2(%d) High\n", MT9M113_PMGPIO_VDD_CAM2_V_EN2);

    
    MT9M113_LOG_DBG("WAIT %d[ms]\n",MT9M113_WAIT_PWON_V_EN2);
    mdelay( MT9M113_WAIT_PWON_V_EN2 );


    
    rc = pm8xxx_gpio_config(MT9M113_PMGPIO_VDD_CAM2_V_EN1, &mt9m113_cam2_v_en1_on);

    if (rc) {
        
        MT9M113_LOG_ERR("VDD_CAM2_V_EN1(%d) Error, rc = %d\n", MT9M113_PMGPIO_VDD_CAM2_V_EN1, rc);
        goto pmgpio_36_config_err;
    }
    MT9M113_LOG_DBG("VDD_CAM2_V_EN1(%d) High\n", MT9M113_PMGPIO_VDD_CAM2_V_EN1);

    
    MT9M113_LOG_DBG("WAIT %d[ms]\n",MT9M113_WAIT_PWON_V_EN1);
    mdelay( MT9M113_WAIT_PWON_V_EN1 );


    
    gpio_direction_output(MT9M113_GPIO_CAM2_RST_N, 0);
    MT9M113_LOG_DBG("CAM2_RST_N(%d) Low\n", MT9M113_GPIO_CAM2_RST_N);
    
    
    MT9M113_LOG_DBG("WAIT %d[ms]\n",MT9M113_WAIT_PWON_RST_L);
    mdelay( MT9M113_WAIT_PWON_RST_L );


    
    if (s_ctrl->clk_rate != 0)
        mt9m113_cam_clk_info->clk_rate = s_ctrl->clk_rate;




    rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
        mt9m113_cam_clk_info, s_ctrl->cam_clk, ARRAY_SIZE(mt9m113_cam_clk_info), 1);

    if (rc < 0) {
        MT9M113_LOG_ERR("clk enable failed\n");
	  goto clk_enable_err;
    }
    MT9M113_LOG_DBG("MCLK ENABLE\n");

    
    MT9M113_LOG_DBG("WAIT %d[ms]\n",MT9M113_WAIT_PWON_MCLK); 
    msleep( MT9M113_WAIT_PWON_MCLK );


    
    gpio_direction_output(MT9M113_GPIO_CAM2_RST_N, 1);
    MT9M113_LOG_DBG("CAM2_RST_N(%d) High\n", MT9M113_GPIO_CAM2_RST_N);

    
    MT9M113_LOG_DBG("WAIT %d[ms]\n",MT9M113_WAIT_PWON_I2C_ACCESS);
    mdelay( MT9M113_WAIT_PWON_I2C_ACCESS );

    MT9M113_LOG_DBG("END(%d)\n",rc);

    return rc;


clk_enable_err:
    pm8xxx_gpio_config(MT9M113_PMGPIO_VDD_CAM2_V_EN1, &mt9m113_cam2_v_en1_off);
pmgpio_36_config_err:
    pm8xxx_gpio_config(MT9M113_PMGPIO_VDD_CAM2_V_EN2, &mt9m113_cam2_v_en1_off);
pmgpio_42_config_err:
    gpio_direction_output(MT9M113_GPIO_CAM2_RST_N, 0);
    gpio_free(MT9M113_GPIO_CAM2_RST_N);
gpio_34_request_err:
    MT9M113_LOG_ERR("power up mt9m113 error\n");
    return -EFAULT;

}

static int __init msm_sensor_init_module(void)
{
    MT9M113_LOG_DBG("Call i2c_add_driver()\n");
    return i2c_add_driver(&mt9m113_i2c_driver);
}


static struct v4l2_subdev_core_ops mt9m113_subdev_core_ops = {
    .s_ctrl = msm_sensor_v4l2_s_ctrl,
    .queryctrl = msm_sensor_v4l2_query_ctrl,
    .ioctl = msm_sensor_subdev_ioctl,
    .s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops mt9m113_subdev_video_ops = {
    .enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};


static struct v4l2_subdev_ops mt9m113_subdev_ops = {
    .core = &mt9m113_subdev_core_ops,
    .video  = &mt9m113_subdev_video_ops,
};


static struct msm_sensor_fn_t mt9m113_func_tbl = {
    .sensor_start_stream = mt9m113_sensor_start_stream,
    .sensor_stop_stream = mt9m113_sensor_stop_stream,
    .sensor_setting = mt9m113_sensor_setting,
    .sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
    .sensor_mode_init = msm_sensor_mode_init,
    .sensor_get_output_info = msm_sensor_get_output_info,
    .sensor_config = msm_sensor_config,
    .sensor_power_up = mt9m113_sensor_power_up,
    .sensor_power_down = mt9m113_sensor_power_down,
    .sensor_get_csi_params = msm_sensor_get_csi_params,
    .sensor_set_parm_pm_obs = msm_sensor_set_parm_pm_obs,
    .sensor_set_wb = mt9m113_set_wb,
    .sensor_set_effect = mt9m113_set_effect,
    .sensor_set_antibanding = mt9m113_set_antibanding,
    .sensor_set_exp_compensation = mt9m113_set_exp_compensation,
    .sensor_set_scene = mt9m113_set_scene,
    .sensor_get_maker_note = mt9m113_get_maker_note,
    .sensor_get_exif_param = mt9m113_get_exif_param,
    .sensor_set_frame_rate_mode = mt9m113_set_frame_rate_mode,
    .sensor_get_hdr_brightness = mt9m113_get_hdr_brightness,
    .sensor_set_hdr_brightness = mt9m113_set_hdr_brightness,
    .sensor_set_cap_mode_enable = mt9m113_set_cap_mode_enable,
    .sensor_get_device_id = mt9m113_get_device_id,
};

static struct msm_sensor_reg_t mt9m113_regs = {
    .default_data_type = MSM_CAMERA_I2C_WORD_DATA,
    .init_settings = &mt9m113_init_confs[0],
    .init_size = ARRAY_SIZE(mt9m113_init_confs),
    .mode_settings = &mt9m113_confs[0],
    .output_settings = &mt9m113_dimensions[0],
    .num_conf = ARRAY_SIZE(mt9m113_confs),
};

static struct msm_sensor_ctrl_t mt9m113_s_ctrl = {
    .msm_sensor_reg = &mt9m113_regs,
    .sensor_i2c_client = &mt9m113_sensor_i2c_client,



    .sensor_i2c_addr = 0x78, 


    .vreg_seq = mt9m113_veg_seq,
    .num_vreg_seq = ARRAY_SIZE(mt9m113_veg_seq),

    .sensor_output_reg_addr = &mt9m113_reg_addr,
    .sensor_id_info = &mt9m113_id_info,
    .cam_mode = MSM_SENSOR_MODE_INVALID,



    .msm_sensor_mutex = &mt9m113_mut,
    .sensor_i2c_driver = &mt9m113_i2c_driver,
    .sensor_v4l2_subdev_info = mt9m113_subdev_info,
    .sensor_v4l2_subdev_info_size = ARRAY_SIZE(mt9m113_subdev_info),
    .sensor_v4l2_subdev_ops = &mt9m113_subdev_ops,
    .func_tbl = &mt9m113_func_tbl,
    .clk_rate = MT9M113_MCLK_FREQUENCY,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Aptina 1.3MP YUV sensor driver");
MODULE_LICENSE("GPL v2");

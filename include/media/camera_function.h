/*
 * Copyright (C) 2010 NEC CASIO Mobile Communications, Ltd.
 *
 *  No permission to use, copy, modify and distribute this software
 *  and its documentation for any purpose is granted.
 *  This software is provided under applicable license agreement only.
 */

















#if !defined(CAMERA_FUNCTION_H)
#define CAMERA_FUNCTION_H


#define DVE005_CAMERA_DRV     
#define DVE021_CAM_IPL_ENABLED



#define FEATURE_DVE021_CAM_DRV


#define FEATURE_DVE021_CAM_ESD_MEASURES_ENABLE


#define FEATURE_DVE021_CAMERA_ENABLE


#undef FEATURE_DVE021_CAM_NV_ENABLE








#define FEATURE_DVE021_CAM_MT9M113
#define CONFIG_FEATURE_DVE021_CAM_MT9M113

#define FEATURE_DVE021_CAM_MCLK_ENABLE
#define CONFIG_FEATURE_DVE021_CAM_MCLK_ENABLE







#define FEATURE_DVE021_CAM_SPIDER
#if defined(FEATURE_DVE021_CAM_SPIDER)
  
  #define FEATURE_DVE021_CAM_MCLK_ENABLE
  
  #define FEATURE_DVE021_CAM_SPI_ENABLE
  
  #define FEATURE_DVE021_CAM_ISP_THREAD
  
  #define FEATURE_DVE021_CAM_STOP_OUTPUT
  
  #define FEATURE_DVE021_CAM_GB_BOOT_SENSOR
  

  
  #define FEATURE_DVE021_CAM_READ_FILE
  
  #define FEATURE_DVE021_CAM_PIC_SIZE_13M

  
  #define FEATURE_DVE021_CAM_ISP_DVE056
  
  #define FEATURE_DVE021_CAM_MT9M113
#endif 








  
  #define D_IMAGE_SHAKE_FUNCTION
  
  #undef D_MOVIE_SHAKE_FUNCTION_REDUCTION
  
  #define D_MOVIE_SHAKE_FUNCTION











  #define CONTINUOUS_SHOOT
  #define CONTINUOUS_SHOOT_RECEIVE_NOTIFY







  
  #define BEST_PHOTO




  
  #undef DVE005_INTERLEAVE_MODE_ENABLE
  #define QUICK_SHOOT







  #define FEATURE_DVE021_CAMERA_CONTINUOUS_AF



































  


  






  #define WHITEBALANCE

  
  #define FLICKER_REDUCTION


  #define D_SCENE_CHANGE_FUNCTION





  #define D_CAMERA_MODE_FUNCTION




















  
  #define FEATURE_DVE021_CAMERA_BRIGHTNESS

  
  #define FEATURE_DVE021_CAMERA_ISO




  
  #define FEATURE_DVE021_CAM_ZOOM

  
  #define FEATURE_DVE021_CAM_EFFECT

  

  
  


  
  #define FEATURE_DVE021_DVE042_ENABLE
#if defined(FEATURE_DVE021_DVE042_ENABLE)
    
    #define FEATURE_DVE021_DVE042_PA_THERM
  #endif 

  
  #undef FEATURE_DVE021_DVE039_PJ_DVE020
  #define FEATURE_DVE021_DVE039_PJ_REG

  
  #define FEATURE_DVE021_CAM_JPEG_RE_ENCODE


  
  #define FEATURE_DVE021_CAMERA_HDR









  


  


  


  


  


  


 


  
  #define FEATURE_DVE021_CAM_FASTBOOT
#endif 

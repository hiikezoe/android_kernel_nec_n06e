/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/



#if !defined(__LINUX_LEDS_CMD_H)
#define __LINUX_LEDS_CMD_H

#if !defined(CONFIG_FEATURE_DVE021_CAM_LIGHT_PREVENT_PEEPING)



#endif 

#if !defined(CONFIG_FEATURE_DVE021_DVE010)



#endif	


#define LEDS_CMD_TYPE_RGB_RED			0x01
#define LEDS_CMD_TYPE_RGB_GREEN			0x02
#define LEDS_CMD_TYPE_RGB_BLUE			0x03
#define LEDS_CMD_TYPE_KEY				0x04
#define LEDS_CMD_TYPE_FLASH				0x05



















#define LEDS_CMD_RET_OK				1
#define LEDS_CMD_RET_NG				0



























extern unsigned char leds_cmd(unsigned char type, unsigned char val);




#define BD6082GUL_IOCTL_MAGIC 'l'

#define LED_CLASS_WRAPPER_FD_PATH    "/dev/led_class_wrapper"

struct bd6082gul_led_flash_parame {
	char still;
	char video;
	char torch;
};

#if defined(CONFIG_FEATURE_DVE021_DVE010)
struct bd6082gul_led_reg_indicator_parame {
	char ind_r;
};

struct bd6082gul_led_green_indicator_parame {
	char ind_g;
};
#endif	

struct bd6082gul_led_prevent_peeping_parame {
	char prevent_peeping ;
};

#define BD6082GUL_CUSTOM_N40    _IOW(BD6082GUL_IOCTL_MAGIC, 0x40, struct bd6082gul_led_flash_parame *)
#if defined(CONFIG_FEATURE_DVE021_DVE010)
#define BD6082GUL_CUSTOM_N41    _IOW(BD6082GUL_IOCTL_MAGIC, 0x41, struct bd6082gul_led_reg_indicator_parame *)
#define BD6082GUL_CUSTOM_N42    _IOW(BD6082GUL_IOCTL_MAGIC, 0x42, struct bd6082gul_led_green_indicator_parame *)
#endif	
#define BD6082GUL_CUSTOM_N43    _IOW(BD6082GUL_IOCTL_MAGIC, 0x43, struct bd6082gul_led_prevent_peeping_parame *)
#define BD6082GUL_CUSTOM_N44    _IOW(BD6082GUL_IOCTL_MAGIC, 0x44, struct bd6082gul_led_flash_parame *)

#define BD6082GUL_CUSTOM_N45    _IOW(BD6082GUL_IOCTL_MAGIC, 0x45, struct BD6082GUL_3color_led_parame *)
#define BD6082GUL_CUSTOM_N46    _IOW(BD6082GUL_IOCTL_MAGIC, 0x46, struct BD6082GUL_3color_led_parame *)
#define BD6082GUL_CUSTOM_N47    _IOW(BD6082GUL_IOCTL_MAGIC, 0x51, struct BD6082GUL_3color_led_parame *)
#define BD6082GUL_CUSTOM_N48    _IOW(BD6082GUL_IOCTL_MAGIC, 0x47, struct BD6082GUL_3color_led_parame *)
#define BD6082GUL_CUSTOM_N49    _IOW(BD6082GUL_IOCTL_MAGIC, 0x48, struct BD6082GUL_3color_led_parame *)
#define BD6082GUL_CUSTOM_N50    _IOW(BD6082GUL_IOCTL_MAGIC, 0x49, struct BD6082GUL_3color_led_parame *)
#define BD6082GUL_CUSTOM_N51    _IOW(BD6082GUL_IOCTL_MAGIC, 0x4A, struct BD6082GUL_3color_led_parame *)
#define BD6082GUL_CUSTOM_N52    _IOW(BD6082GUL_IOCTL_MAGIC, 0x4B, struct BD6082GUL_3color_led_parame *)
#define BD6082GUL_CUSTOM_N53    _IOW(BD6082GUL_IOCTL_MAGIC, 0x4C, struct BD6082GUL_3color_led_parame *)
#define BD6082GUL_CUSTOM_N54    _IOW(BD6082GUL_IOCTL_MAGIC, 0x4D, struct BD6082GUL_3color_led_parame *)
#define BD6082GUL_CUSTOM_N55    _IOW(BD6082GUL_IOCTL_MAGIC, 0x4F, struct BD6082GUL_3color_led_parame *)
#define BD6082GUL_CUSTOM_N56    _IOW(BD6082GUL_IOCTL_MAGIC, 0x50, struct BD6082GUL_3color_led_parame *)


#endif 

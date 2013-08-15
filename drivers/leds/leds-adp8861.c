/*
 * drivers/leds/leds-adp8861.c
 *
 * - ADP8861 led controller
 *
 * Copyright (C) NEC CASIO Mobile Communications, Ltd.
 *
 */
 




#define LED_PM_OBS_DISABLE





#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#include <linux/module.h>


#if !defined(LED_PM_OBS_DISABLE)




#include <linux/pm_obs_api.h>

#endif 

#include <linux/leds.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#include <linux/leds-pmic8058.h>

#include <linux/mm.h>
#include <asm/page.h>

#include <linux/leds-adp8861.h>














#if !defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077)

#define LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077

#endif 

#if !defined(LOCAL_CONFIG_FEATURE_DVE021_DVE008)



#endif 

#if !defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)
#if  defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077)
#define LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM
#endif
#endif 

#if !defined(LOCAL_CONFIG_FEATURE_DVE021_CAM_LIGHT_PREVENT_PEEPING)

#if defined(CONFIG_FEATURE_DVE021_CAM_LIGHT_PREVENT_PEEPING) && defined(LOCAL_CONFIG_FEATURE_DVE021_DVE008)
#define LOCAL_CONFIG_FEATURE_DVE021_CAM_LIGHT_PREVENT_PEEPING
#endif 

#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#define LOCAL_CONFIG_FEATURE_DVE021_CAM_LIGHT_PREVENT_PEEPING
#endif 

#endif 




enum eled_devices_ids {
	LED_DRV_IC_DEVICE_ID_RGB_RED		= 0,			
	LED_DRV_IC_DEVICE_ID_RGB_GREEN,						
	LED_DRV_IC_DEVICE_ID_RGB_BLUE,						


	LED_DRV_IC_DEVICE_ID_ER_RGB_RED	,			
	LED_DRV_IC_DEVICE_ID_ER_RGB_GREEN,						
	LED_DRV_IC_DEVICE_ID_ER_RGB_BLUE,						

	
	LED_DRV_IC_DEVICE_ID_RGB_TIME1,						
	LED_DRV_IC_DEVICE_ID_RGB_TIME2,						
	
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 
	LED_DRV_IC_DEVICE_ID_KEYBL,							
#endif 

#if defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)


#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 
	LED_DRV_IC_DEVICE_ID_KEY_ILM1,						
#endif 


	LED_DRV_IC_DEVICE_ID_KEY_ILM2,						
	LED_DRV_IC_DEVICE_ID_KEY_ILM3,						

#endif 

	LED_DRV_IC_DEVICE_ID_FLASH_TORCH,					
	LED_DRV_IC_DEVICE_ID_FLASH_FLASH_MOVIE,				
	LED_DRV_IC_DEVICE_ID_FLASH_FLASH_STILL,				

	LED_DRV_IC_DEVICE_ID_FLASH_TORCH_SUB,					
	LED_DRV_IC_DEVICE_ID_FLASH_FLASH_MOVIE_SUB,				
	LED_DRV_IC_DEVICE_ID_FLASH_FLASH_STILL_SUB,				

	
#if defined(LOCAL_CONFIG_FEATURE_DVE021_CAM_LIGHT_PREVENT_PEEPING)
	LED_DRV_IC_DEVICE_ID_PREVENT_PEEPING,				
#endif 
	
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 
	LED_DRV_IC_DEVICE_ID_BTNBL,							
#endif 
	
	LED_DRV_IC_DEVICE_ID_NUM
};




enum eled_devices_ids_sub_rgb {
	LED_DRV_IC_DEVICE_ID_SUB_RGB_RED		= 0,
	LED_DRV_IC_DEVICE_ID_SUB_RGB_GREEN,
	LED_DRV_IC_DEVICE_ID_SUB_RGB_BLUE,
	LED_DRV_IC_DEVICE_ID_SUB_RGB_NUM
};

enum eled_devices_ids_sub_rgb_opt {
	LED_DRV_IC_DEVICE_ID_SUB_RGB_TIMER1			= 0,
	LED_DRV_IC_DEVICE_ID_SUB_RGB_TIMER2,
};

enum eled_devices_ids_sub_flash {
	LED_DRV_IC_DEVICE_ID_SUB_FLASH_TORCH		= 0,
	LED_DRV_IC_DEVICE_ID_SUB_FLASH_FLASH_MOVIE,
	LED_DRV_IC_DEVICE_ID_SUB_FLASH_FLASH_STILL,
	LED_DRV_IC_DEVICE_ID_SUB_FLASH_NUM
};

#if defined(LOCAL_CONFIG_FEATURE_DVE021_CAM_LIGHT_PREVENT_PEEPING)
enum eled_devices_ids_sub_prevent_peeping {
	LED_DRV_IC_DEVICE_ID_SUB_PREVENT_PEEPING =0,
};
#endif 





enum eled_rgb_lighting_num {
	LED_DRV_IC_RGB_LIGHTING_1		= 0,				
	LED_DRV_IC_RGB_LIGHTING_2,							
	LED_DRV_IC_RGB_LIGHTING_3,							
	LED_DRV_IC_RGB_LIGHTING_NUM
};



#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 
enum eled_keybl_brightness_num {
	LED_DRV_IC_KEYBL_BRIGHTNESS_1	= 0,				
	LED_DRV_IC_KEYBL_BRIGHTNESS_2,						
	LED_DRV_IC_KEYBL_BRIGHTNESS_3,						
	LED_DRV_IC_KEYBL_BRIGHTNESS_NUM
};
#endif 



enum eled_flash_type_num {
	LED_DRV_IC_FLASH_TYPE_STILL	= 0,					
	LED_DRV_IC_FLASH_TYPE_MOVIE,						
	LED_DRV_IC_FLASH_TYPE_IDLE,							
	LED_DRV_IC_FLASH_TYPE_NUM
};





#define LED_DRV_IC_DEVICE_NAME_RGB_RED				"red"
#define LED_DRV_IC_DEVICE_NAME_RGB_GREEN			"green"
#define LED_DRV_IC_DEVICE_NAME_RGB_BLUE				"blue"


#define LED_DRV_IC_DEVICE_NAME_ER_RGB_RED				"er-red"
#define LED_DRV_IC_DEVICE_NAME_ER_RGB_GREEN			"er-green"
#define LED_DRV_IC_DEVICE_NAME_ER_RGB_BLUE				"er-blue"


#define LED_DRV_IC_DEVICE_NAME_RGB_TIMER1			"rgb-time1"
#define LED_DRV_IC_DEVICE_NAME_RGB_TIMER2			"rgb-time2"

#define LED_DRV_IC_DEVICE_NAME_KEYBL				"keyboard-backlight"
#if defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)
#define LED_DRV_IC_DEVICE_NAME_KEY_ILM1				"keyboard-illuminations1"

#define LED_DRV_IC_DEVICE_NAME_KEY_ILM2				"keyboard-illuminations2"
#define LED_DRV_IC_DEVICE_NAME_KEY_ILM3				"keyboard-illuminations3"

#endif 
#define LED_DRV_IC_DEVICE_NAME_FLASH_TORCH			"torch-led"
#define LED_DRV_IC_DEVICE_NAME_FLASH_FLASH_MOVIE	"flash-movie-led"
#define LED_DRV_IC_DEVICE_NAME_FLASH_FLASH_STILL	"flash-still-led"


#define LED_DRV_IC_DEVICE_NAME_SUB_FLASH_TORCH			"sub-torch-led"
#define LED_DRV_IC_DEVICE_NAME_SUB_FLASH_FLASH_MOVIE	"sub-flash-movie-led"
#define LED_DRV_IC_DEVICE_NAME_SUB_FLASH_FLASH_STILL	"sub-flash-still-led"


#if defined(LOCAL_CONFIG_FEATURE_DVE021_CAM_LIGHT_PREVENT_PEEPING)
#define LED_DRV_IC_DEVICE_NAME_PREVENT_PEEPING		"prevent-peeping-led"
#endif 

#define LED_DRV_IC_DEVICE_NAME_BTNBL				"button-backlight"

#define DEVICEWRPPER_NAME							"led_class_wrapper"





#define LED_REG_MFDVID						0x00
#define LED_REG_MDCR						0x01
#define LED_REG_MDCR2						0x02
#define LED_REG_INTR_EN						0x03
#define LED_REG_CFGR						0x04
#define LED_REG_BLSEN						0x05
#define LED_REG_BLOFF						0x06
#define LED_REG_BLDIM						0x07
#define LED_REG_BLFR						0x08
#define LED_REG_BLMX						0x09
#define LED_REG_BLDM						0x0A
#define LED_REG_ISCFR						0x0F
#define LED_REG_ISCC						0x10
#define LED_REG_ISCT1						0x11
#define LED_REG_ISCT2						0x12
#define LED_REG_ISCF						0x13
#define LED_REG_ISC7						0x14
#define LED_REG_ISC6						0x15
#define LED_REG_ISC5						0x16
#define LED_REG_ISC4						0x17
#define LED_REG_ISC3						0x18
#define LED_REG_ISC2						0x19
#define LED_REG_ISC1						0x1A



#define LED_REG_MFDVID_INI					0x40
#define LED_REG_MDCR_INI					0x20
#define LED_REG_MDCR_INI_POFF				0x00



#define LED_REG_BLSEN_INI					0x7F

#define LED_REG_ISCC_INI					0x00
#define LED_REG_ISCT1_INI					0x00
#define LED_REG_ISCT2_INI					0x00


#define LED_REG_ISCC_COUNT					0x07




#define LED_REG_GPIO_HWEN_GPIO

#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#define LED_REG_GPIO_RESET					86
#else 
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077)
#if defined(LED_REG_GPIO_HWEN_GPIO)

#define CAM_LED_REG_GPIO_RESET					33
#define LED_REG_GPIO_RESET					86



#else 
#define LED_REG_GPIO_RESET					15
#endif 
#else 



#define LED_REG_GPIO_RESET					15

#endif 
#endif 
#define LED_REG_GPIO_WAIT					1

#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#define LED_REG_ISC1_MAX					0x2A
#define LED_REG_ISC2_MAX					0x2A
#define LED_REG_ISC3_MAX					0x2A

#define LED_REG_ISC4_MAX					0x2A

#define LED_REG_ISC7_MAX					0x7F
#define LED_REG_ISC7_D_MAX					0x3F
#else 
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077)













#define LED_REG_ISC1_MAX					0x3F
#define LED_REG_ISC2_MAX					0x3F
#define LED_REG_ISC3_MAX					0x3F
#define LED_REG_ISC4_MAX					0x2A
#define LED_REG_ISC5_MAX					0x2A
#define LED_REG_ISC6_MAX					0x2A

#define LED_REG_ISC1_1_MAX					0x2A
#define LED_REG_ISC2_1_MAX					0x2A
#define LED_REG_ISC3_1_MAX					0x2A
#define LED_REG_ISC4_1_MAX					0x2A
#define LED_REG_ISC5_1_MAX					0x2A
#define LED_REG_ISC6_1_MAX					0x2A

#define LED_REG_ISC7_MAX					0x7F
#define LED_REG_ISC7_D_MAX					0x7F













#else 
#if defined(LOCAL_CONFIG_FEATURE_DVE021_DVE008)
#define LED_REG_ISC1_MAX					0x1E
#define LED_REG_ISC2_MAX					0x1E
#define LED_REG_ISC3_MAX					0x1E

#define LED_REG_ISC1_MIN					0x0D
#define LED_REG_ISC2_MIN					0x0D
#define LED_REG_ISC3_MIN					0x0D
#else 
#define LED_REG_ISC1_MAX					0x6C
#define LED_REG_ISC2_MAX					0x59
#define LED_REG_ISC3_MAX					0x59
#endif 

#define LED_REG_ISC4_MAX					0x59
#define LED_REG_ISC5_MAX					0x59
#define LED_REG_ISC6_MAX					0x59

#define LED_REG_ISC7_MAX					0x67
#endif 

#define LED_REG_ISC4_SIZE					3
#endif 

enum led_state_num{
	LED_STATE_STOP = 0,						
	LED_STATE_INITIALIZATION_WAIT,			
	LED_STATE_INITIALIZATION,				
	LED_STATE_IDLE,							
	LED_STATE_ENDING,						
};




struct drv_ic_led_rgb_brightness {
	unsigned char					brightness[LED_DRV_IC_DEVICE_ID_SUB_RGB_NUM];
	int								keyilm_status;
	struct mutex	 				m_lock;
};


struct drv_ic_led_er_rgb_brightness {
	unsigned char					brightness[LED_DRV_IC_DEVICE_ID_SUB_RGB_NUM];
};



struct drv_ic_led_rgb_config {
	struct drv_ic_led_rgb_brightness		lighting[LED_DRV_IC_RGB_LIGHTING_NUM];
};


struct drv_ic_led_er_rgb_config {
	struct drv_ic_led_er_rgb_brightness		lighting[LED_DRV_IC_RGB_LIGHTING_NUM];
};


#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 

struct drv_ic_led_keybl_config {

	unsigned char			 		left_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_NUM];
	unsigned char			 		right_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_NUM];



#if defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)
	int								keyilm_status;
#endif 
	struct mutex	 				m_lock;
};
#endif 

struct drv_ic_led_flash_mode_config {
	union u_led_isc_reg					brightness[LED_DRV_IC_FLASH_TYPE_NUM];
};


struct drv_ic_led_prevent_peeping_config {
#if defined(LOCAL_CONFIG_FEATURE_DVE021_DVE008) || defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
	union u_led_isc_reg				brightness;
#else 
	unsigned char					brightness;
#endif 
};

struct drv_ic_led_configs {
	struct drv_ic_led_rgb_config			rbg_config;
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 
	struct drv_ic_led_keybl_config			keybl_config;
#endif 

	struct drv_ic_led_flash_mode_config		flash_config[ID_MAX];



	struct drv_ic_led_prevent_peeping_config	prevent_peeping_config;

	struct drv_ic_led_er_rgb_config			er_rbg_config;

};

struct drv_ic_data {
	struct led_classdev					leds[LED_DRV_IC_DEVICE_ID_NUM];
};

struct adp8861_led_data {
	spinlock_t data_lock;
};

struct adp8861_work {
	struct work_struct work;
};

struct adp8861_reg_init{
	unsigned char adr	; 
	unsigned char wdata	; 
};

struct adp8861_reg_tbl{
	union u_led_iscc_reg	iscc;
	union u_led_isct1_reg	isct1;
	union u_led_isct2_reg	isct2;
	
};

struct led_request_rgb_2{
	unsigned char	set:3			;
	unsigned char	dmy1:5			;
};

union u_led_request_rgb{
	struct led_request_rgb		st1;
	struct led_request_rgb_2	st2;
	unsigned char				uc;
};

struct adp8861_rgb_time_ctl{
	union u_led_isct1_reg		time1;
	union u_led_isct2_reg		time2;
	union u_led_request_rgb		rgb;
};


struct adp8861_driver_data{
	struct adp8861_work	*adp8861_led;			
	struct i2c_client	 *adp8861_client;		
	struct adp8861_reg_tbl adp8861_reg;
	struct adp8861_rgb_time_ctl adp8861_rgb_time;
	unsigned int led_starte;
	int id_device_num;
};

struct adp8861{
	struct adp8861_driver_data   driver_data[ID_MAX];
};

static struct adp8861 adp8861;

static struct adp8861_3color_led_parame_nv adp8861_3color_led_parame_nv;




static struct drv_ic_led_configs			*pg_led_congig = NULL;
static struct drv_ic_led_rgb_brightness		g_rgb_brightness;

static struct drv_ic_led_er_rgb_brightness		g_er_rgb_brightness;

#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 
static struct drv_ic_led_keybl_config		g_key_brightness;
#endif 














































static void led_brightness_set_rgb(int device_no, enum led_brightness value);

static void led_brightness_set_er_rgb(int device_no, enum led_brightness value);

static void led_brightness_set_rgb_opt(int device_no, unsigned char value);
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 
static void led_brightness_set_keybl(int device_no, enum led_brightness value);
#endif 

static void led_brightness_set_flash(int device_no, enum led_brightness value, enum device_id_num id_num);




#if defined(LOCAL_CONFIG_FEATURE_DVE021_CAM_LIGHT_PREVENT_PEEPING)
static void led_brightness_set_prevent_peeping(int device_no, enum led_brightness value);
#endif 


static int adp8861_init_reg(enum device_id_num id_num);








unsigned char leds_cmd(unsigned char type, unsigned char val)
{
	int			device_no;
	char		ret = LEDS_CMD_RET_OK;

	switch(type){
		case LEDS_CMD_TYPE_RGB_RED:
			device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_RED;
			led_brightness_set_rgb( device_no, (enum led_brightness)val );
			break;
		
		case LEDS_CMD_TYPE_RGB_GREEN:
			device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_GREEN;
			led_brightness_set_rgb( device_no, (enum led_brightness)val );
			break;
		
		case LEDS_CMD_TYPE_RGB_BLUE:
			device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_BLUE;
			led_brightness_set_rgb( device_no, (enum led_brightness)val );
			break;

		case LEDS_CMD_TYPE_ER_RGB_RED:
			device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_RED;
			led_brightness_set_er_rgb( device_no, (enum led_brightness)val );
			break;

		case LEDS_CMD_TYPE_ER_RGB_GREEN:
			device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_GREEN;
			led_brightness_set_er_rgb( device_no, (enum led_brightness)val );
			break;

		case LEDS_CMD_TYPE_ER_RGB_BLUE:
			device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_BLUE;
			led_brightness_set_er_rgb( device_no, (enum led_brightness)val );
			break;

		
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 
		case LEDS_CMD_TYPE_KEY:
			device_no = LED_DRV_IC_KEYBL_BRIGHTNESS_1;
			led_brightness_set_keybl( device_no, val );

			device_no = LED_DRV_IC_KEYBL_BRIGHTNESS_2;
			led_brightness_set_keybl( device_no, val );
			device_no = LED_DRV_IC_KEYBL_BRIGHTNESS_3;
			led_brightness_set_keybl( device_no, val );

			break;
#endif 
		
		case LEDS_CMD_TYPE_FLASH_MOVIE:
			device_no = LED_DRV_IC_DEVICE_ID_SUB_FLASH_FLASH_MOVIE;

			led_brightness_set_flash( device_no, (enum led_brightness)val, ID_0 );



			break;
		
		case LEDS_CMD_TYPE_FLASH_STILL:
			device_no = LED_DRV_IC_DEVICE_ID_SUB_FLASH_FLASH_STILL;

			led_brightness_set_flash( device_no, (enum led_brightness)val, ID_0 );



			break;
		
		case LEDS_CMD_TYPE_FLASH_TORCH:
			device_no = LED_DRV_IC_DEVICE_ID_SUB_FLASH_TORCH;

			led_brightness_set_flash( device_no, (enum led_brightness)val, ID_0 );



			break;
		
#if defined(LOCAL_CONFIG_FEATURE_DVE021_CAM_LIGHT_PREVENT_PEEPING)
		case LEDS_CMD_TYPE_PREVENT_PEEPING:
			device_no = LED_DRV_IC_DEVICE_ID_SUB_PREVENT_PEEPING;
			led_brightness_set_prevent_peeping( device_no, (enum led_brightness)val );
			break;
#endif 
		
		default:
			ret = LEDS_CMD_RET_NG;
			break;
	}
	return ret;
}
EXPORT_SYMBOL(leds_cmd);












static void led_brightness_set_common(struct led_classdev *led_cdev, enum led_brightness value)
{
	int			device_no;
	
	
	if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_RGB_RED)){
		device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_RED;
		led_brightness_set_rgb( device_no, value );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_RGB_GREEN)){
		device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_GREEN;
		led_brightness_set_rgb( device_no, value );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_RGB_BLUE)){
		device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_BLUE;
		led_brightness_set_rgb( device_no, value );
	}

	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_ER_RGB_RED)){
		device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_RED;
		led_brightness_set_er_rgb( device_no, value );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_ER_RGB_GREEN)){
		device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_GREEN;
		led_brightness_set_er_rgb( device_no, value );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_ER_RGB_BLUE)){
		device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_BLUE;
		led_brightness_set_er_rgb( device_no, value );
	}


	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_RGB_TIMER1)){
		device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_TIMER1;
		led_brightness_set_rgb_opt( device_no, value );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_RGB_TIMER2)){
		device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_TIMER2;
		led_brightness_set_rgb_opt( device_no, value );
	}
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 
#if defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)


	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_KEY_ILM1)){


		device_no = LED_DRV_IC_KEYBL_BRIGHTNESS_1;
		led_brightness_set_keybl( device_no, value );

	}


	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_KEY_ILM2)){
		device_no = LED_DRV_IC_KEYBL_BRIGHTNESS_2;
		led_brightness_set_keybl( device_no, value );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_KEY_ILM3)){
		device_no = LED_DRV_IC_KEYBL_BRIGHTNESS_3;
		led_brightness_set_keybl( device_no, value );
	}

#endif 
#endif 

	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_FLASH_TORCH))
	{
		device_no = LED_DRV_IC_DEVICE_ID_SUB_FLASH_TORCH;
		led_brightness_set_flash( device_no, value, ID_0 );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_FLASH_FLASH_MOVIE))
	{
		device_no = LED_DRV_IC_DEVICE_ID_SUB_FLASH_FLASH_MOVIE;
		led_brightness_set_flash( device_no, value, ID_0 );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_FLASH_FLASH_STILL))
	{
		device_no = LED_DRV_IC_DEVICE_ID_SUB_FLASH_FLASH_STILL;
		led_brightness_set_flash( device_no, value, ID_0 );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_SUB_FLASH_TORCH))
	{
		device_no = LED_DRV_IC_DEVICE_ID_SUB_FLASH_TORCH;
		led_brightness_set_flash( device_no, value, ID_1 );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_SUB_FLASH_FLASH_MOVIE))
	{
		device_no = LED_DRV_IC_DEVICE_ID_SUB_FLASH_FLASH_MOVIE;
		led_brightness_set_flash( device_no, value, ID_1 );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_SUB_FLASH_FLASH_STILL))
	{
		device_no = LED_DRV_IC_DEVICE_ID_SUB_FLASH_FLASH_STILL;
		led_brightness_set_flash( device_no, value, ID_1 );
	}

















#if defined(LOCAL_CONFIG_FEATURE_DVE021_CAM_LIGHT_PREVENT_PEEPING)
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_PREVENT_PEEPING))
	{
		device_no = LED_DRV_IC_DEVICE_ID_SUB_PREVENT_PEEPING;
		led_brightness_set_prevent_peeping( device_no, value );
	}
#endif 
	
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_KEYBL) ||
			 !strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_BTNBL) ){


		device_no = LED_DRV_IC_KEYBL_BRIGHTNESS_1;
		led_brightness_set_keybl( device_no, value );

		device_no = LED_DRV_IC_KEYBL_BRIGHTNESS_2;
		led_brightness_set_keybl( device_no, value );
		device_no = LED_DRV_IC_KEYBL_BRIGHTNESS_3;
		led_brightness_set_keybl( device_no, value );


	}
#endif 
}










static void led_brightness_set_er_rgb(int device_no, enum led_brightness value)
{
	union u_led_isc_reg		set_brigh[LED_DRV_IC_DEVICE_ID_SUB_RGB_NUM];
	struct led_request_rgb	set_onoff;
	int						ret;

	memset( &set_onoff, 0, sizeof( set_onoff ) );
	memset( set_brigh, 0, sizeof( set_brigh ) );

	g_er_rgb_brightness.brightness[device_no] = value;

	set_brigh[device_no].st2.set_flag = ADP8861_LED_ON;
	set_brigh[device_no].st2.scd	  = g_er_rgb_brightness.brightness[device_no];

	ret = adp8861_er_rgb_led_bright( &set_brigh[LED_DRV_IC_DEVICE_ID_SUB_RGB_RED],
								  &set_brigh[LED_DRV_IC_DEVICE_ID_SUB_RGB_GREEN],
								  &set_brigh[LED_DRV_IC_DEVICE_ID_SUB_RGB_BLUE] );	

	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_brightness_set_er_rgb: adp8861_er_rgb_led_bright = %d \n",ret);
	}
	if( g_er_rgb_brightness.brightness[LED_DRV_IC_DEVICE_ID_SUB_RGB_RED]	!= ADP8861_LED_OFF ){
		set_onoff.set_r = ADP8861_LED_ON;
	}
	if( g_er_rgb_brightness.brightness[LED_DRV_IC_DEVICE_ID_SUB_RGB_GREEN] != ADP8861_LED_OFF ){
		set_onoff.set_g = ADP8861_LED_ON;
	}
	if( g_er_rgb_brightness.brightness[LED_DRV_IC_DEVICE_ID_SUB_RGB_BLUE]	!= ADP8861_LED_OFF ){
		set_onoff.set_b = ADP8861_LED_ON;
	}

	ret = adp8861_er_rgb_led_set( &set_onoff );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_brightness_set_er_rgb: adp8861_er_rgb_led_set(%d,%d,%d) = %d \n",set_onoff.set_r,set_onoff.set_g,set_onoff.set_b,ret);
	}
}









static void led_brightness_set_rgb(int device_no, enum led_brightness value)
{
	union u_led_isc_reg		set_brigh[LED_DRV_IC_DEVICE_ID_SUB_RGB_NUM];
	struct led_request_rgb	set_onoff;
	int						ret;
	
	memset( &set_onoff, 0, sizeof( set_onoff ) );
	memset( set_brigh, 0, sizeof( set_brigh ) );
	
	g_rgb_brightness.brightness[device_no] = value;
	
	set_brigh[device_no].st2.set_flag = ADP8861_LED_ON;
	set_brigh[device_no].st2.scd	  = g_rgb_brightness.brightness[device_no];
	
	ret = adp8861_rgb_led_bright( &set_brigh[LED_DRV_IC_DEVICE_ID_SUB_RGB_RED],
								  &set_brigh[LED_DRV_IC_DEVICE_ID_SUB_RGB_GREEN],
								  &set_brigh[LED_DRV_IC_DEVICE_ID_SUB_RGB_BLUE] );	
	
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_brightness_set_rgb: adp8861_rgb_led_bright = %d \n",ret);
	}
	if( g_rgb_brightness.brightness[LED_DRV_IC_DEVICE_ID_SUB_RGB_RED]	!= ADP8861_LED_OFF ){
		set_onoff.set_r = ADP8861_LED_ON;
	}
	if( g_rgb_brightness.brightness[LED_DRV_IC_DEVICE_ID_SUB_RGB_GREEN] != ADP8861_LED_OFF ){
		set_onoff.set_g = ADP8861_LED_ON;
	}
	if( g_rgb_brightness.brightness[LED_DRV_IC_DEVICE_ID_SUB_RGB_BLUE]	!= ADP8861_LED_OFF ){
		set_onoff.set_b = ADP8861_LED_ON;
	}
	
	switch( device_no ){
	  case LED_DRV_IC_DEVICE_ID_SUB_RGB_RED:

		adp8861.driver_data[ID_0].adp8861_rgb_time.rgb.st1.set_r = ADP8861_LED_ON;




		break;
		
	  case LED_DRV_IC_DEVICE_ID_SUB_RGB_GREEN:

		adp8861.driver_data[ID_0].adp8861_rgb_time.rgb.st1.set_g = ADP8861_LED_ON;



		break;
		
	  case LED_DRV_IC_DEVICE_ID_SUB_RGB_BLUE:

		adp8861.driver_data[ID_0].adp8861_rgb_time.rgb.st1.set_b = ADP8861_LED_ON;



		break;
	}

	if(( adp8861.driver_data[ID_0].adp8861_rgb_time.time1.uc != 0 )
	&& ( adp8861.driver_data[ID_0].adp8861_rgb_time.time2.uc != 0 )){
		if( adp8861.driver_data[ID_0].adp8861_rgb_time.rgb.st2.set != 0x07 ){





			return;
		}
	}
	else{

		adp8861.driver_data[ID_0].adp8861_rgb_time.rgb.uc = 0;



	}
	
	
	ret = adp8861_rgb_led_set( &set_onoff );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_brightness_set_rgb: adp8861_rgb_led_set(%d,%d,%d) = %d \n",set_onoff.set_r,set_onoff.set_g,set_onoff.set_b,ret);
	}
	

		adp8861.driver_data[ID_0].adp8861_rgb_time.rgb.uc = 0;




}


static void led_brightness_set_rgb_opt(int device_no, unsigned char value)
{
	union u_led_isct1_reg		led_timer1_set, led_timer1_data;
	union u_led_isct2_reg		led_timer2_set, led_timer2_data;
	
	if( device_no == LED_DRV_IC_DEVICE_ID_SUB_RGB_TIMER1 ){
		
		memset( &led_timer1_set,  0, sizeof( led_timer1_set ) );
		led_timer1_set.st2.scon = ADP8861_LED_ON;
		
		led_timer1_data.uc = value;
		

		adp8861_led_timer_set1( &led_timer1_set, &led_timer1_data, ID_0 );



		return;
	}
	else if( device_no == LED_DRV_IC_DEVICE_ID_SUB_RGB_TIMER2 ){
		
		memset( &led_timer2_set,  0, sizeof( led_timer2_set ) );
		led_timer2_set.st2.sc1_off = ADP8861_LED_ON;
		led_timer2_set.st2.sc2_off = ADP8861_LED_ON;
		led_timer2_set.st2.sc3_off = ADP8861_LED_ON;
		
		led_timer2_data.uc = value;
		

		adp8861_led_timer_set2( &led_timer2_set, &led_timer2_data, ID_0 );



		return;
	}
	else{
		printk(KERN_INFO "led_brightness_set_rgb_opt: device_no Error = %d \n",device_no );
	}
}

#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 








static void led_brightness_set_keybl(int device_no, enum led_brightness value)
{

	union u_led_isc_reg		set_brigh[3];
	struct led_request_rgb	set_onoff;




	int						ret;
	
	mutex_lock( &g_key_brightness.m_lock );
	
	memset( &set_onoff, 0, sizeof( set_onoff ) );
	memset( &set_brigh, 0, sizeof( set_brigh ) );
	

	g_key_brightness.left_brightness[device_no] = value;
	g_key_brightness.right_brightness[device_no] = value;



	

	if( g_key_brightness.keyilm_status == ADP8861_LED_ON ){
		mutex_unlock( &g_key_brightness.m_lock );
		return;
	}

	









	set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_1].st2.set_flag = ADP8861_LED_ON;
	set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_1].st2.scd	  = g_key_brightness.left_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_1];
	set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_2].st2.set_flag = ADP8861_LED_ON;
	set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_2].st2.scd	  = g_key_brightness.left_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_2];
	set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_3].st2.set_flag = ADP8861_LED_ON;
	set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_3].st2.scd	  = g_key_brightness.left_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_3];
	ret = adp8861_left_key_led_bright( &set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_1], &set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_2], &set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_3] );

	memset( &set_brigh, 0, sizeof( set_brigh ) );

	set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_1].st2.set_flag = ADP8861_LED_ON;
	set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_1].st2.scd	  = g_key_brightness.right_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_1];
	set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_2].st2.set_flag = ADP8861_LED_ON;
	set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_2].st2.scd	  = g_key_brightness.right_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_2];
	set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_3].st2.set_flag = ADP8861_LED_ON;
	set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_3].st2.scd	  = g_key_brightness.right_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_3];
	ret = adp8861_right_key_led_bright( &set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_1], &set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_2], &set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_3] );






	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_brightness_set_keybl: adp8861_key_led_bright = %d \n",ret);
	}












	if( g_key_brightness.left_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_1] != ADP8861_LED_OFF ){
		set_onoff.set_r = ADP8861_LED_ON;
	}
	if( g_key_brightness.left_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_2] != ADP8861_LED_OFF ){
		set_onoff.set_g = ADP8861_LED_ON;
	}
	if( g_key_brightness.left_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_3] != ADP8861_LED_OFF ){
		set_onoff.set_b = ADP8861_LED_ON;
	}
	ret = adp8861_left_key_led_set( &set_onoff );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_brightness_set_keybl: adp8861_left_key_led_set(%d,%d,%d) = %d \n",set_onoff.set_r,set_onoff.set_g,set_onoff.set_b,ret);
	}

	memset( &set_onoff, 0, sizeof( set_onoff ) );

	if( g_key_brightness.right_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_1] != ADP8861_LED_OFF ){
		set_onoff.set_r = ADP8861_LED_ON;
	}
	if( g_key_brightness.right_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_2] != ADP8861_LED_OFF ){
		set_onoff.set_g = ADP8861_LED_ON;
	}
	if( g_key_brightness.right_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_3] != ADP8861_LED_OFF ){
		set_onoff.set_b = ADP8861_LED_ON;
	}
	ret = adp8861_right_key_led_set( &set_onoff );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_brightness_set_keybl: adp8861_right_key_led_set(%d,%d,%d) = %d \n",set_onoff.set_r,set_onoff.set_g,set_onoff.set_b,ret);
	}



















	mutex_unlock( &g_key_brightness.m_lock );
}

#if defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)








void led_brightness_set_keyilm_status( int status )
{
	union u_led_iscf_reg	led_fade;
	union u_led_isc_reg		set_brigh[3];

	struct led_request_rgb	set_onoff;



	int						ret;
	
	mutex_lock( &g_key_brightness.m_lock );
	
	memset( &led_fade,  0, sizeof( led_fade  ) );
	memset( set_brigh,  0, sizeof( set_brigh ) );
	memset( &set_onoff, 0, sizeof( set_onoff ) );
	

	ret = adp8861_led_fade_set( &led_fade, ID_1 );



	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_brightness_set_keyilm_status: adp8861_led_fade_set = %d \n",ret);
	}
	
	if( status == ADP8861_LED_ON ){
		g_key_brightness.keyilm_status = ADP8861_LED_ON;
		

		set_brigh[0].st2.set_flag = ADP8861_LED_ON;
		set_brigh[1].st2.set_flag = ADP8861_LED_ON;
		set_brigh[2].st2.set_flag = ADP8861_LED_ON;

		ret = adp8861_right_key_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );
		ret = adp8861_left_key_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );







		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_brightness_set_keyilm_status: adp8861_rgb_led_bright = %d \n",ret);
		}

		ret = adp8861_right_key_led_set( &set_onoff );
		ret = adp8861_left_key_led_set( &set_onoff );



		if( ret != ADP8861_LED_SET_OK ){

			printk(KERN_INFO "led_brightness_set_keyilm_status: adp8861_rgb_led_set(%d,%d,%d) = %d \n",set_onoff.set_r,set_onoff.set_g,set_onoff.set_b,ret);








		}
	}
	else{
		g_key_brightness.keyilm_status = ADP8861_LED_OFF;
		
		set_brigh[0].st2.set_flag = ADP8861_LED_ON;

		set_brigh[0].st2.scd	  = g_key_brightness.right_brightness[0];
		if( g_key_brightness.right_brightness[0] != 0 ){





			set_onoff.set_r = ADP8861_LED_ON;



		}

		set_brigh[1].st2.set_flag = ADP8861_LED_ON;

		set_brigh[1].st2.scd	  = g_key_brightness.right_brightness[1];
		if( g_key_brightness.right_brightness[1] != 0 ){





			set_onoff.set_g = ADP8861_LED_ON;



		}
		set_brigh[2].st2.set_flag = ADP8861_LED_ON;

		set_brigh[2].st2.scd	  = g_key_brightness.right_brightness[2];
		if( g_key_brightness.right_brightness[2] != 0 ){





			set_onoff.set_b = ADP8861_LED_ON;



		}



		ret = adp8861_right_key_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );
		ret = adp8861_left_key_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );






		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_adp8861_keyilm_ctrl: adp8861_rgb_led_bright = %d \n",ret);
		}

		ret = adp8861_right_key_led_set( &set_onoff );



		if( ret != ADP8861_LED_SET_OK ){

			printk(KERN_INFO "led_adp8861_keyilm_ctrl: adp8861_rgb_led_set(%d,%d,%d) = %d \n",set_onoff.set_r,set_onoff.set_g,set_onoff.set_b,ret);







		}

		memset( &set_onoff, 0, sizeof( set_onoff ) );
		memset( set_brigh,  0, sizeof( set_brigh ) );
		set_brigh[0].st2.set_flag = ADP8861_LED_ON;
		set_brigh[0].st2.scd	  = g_key_brightness.left_brightness[0];
		if( g_key_brightness.left_brightness[0] != 0 ){
			set_onoff.set_r = ADP8861_LED_ON;
		}
		set_brigh[1].st2.set_flag = ADP8861_LED_ON;
		set_brigh[1].st2.scd	  = g_key_brightness.left_brightness[1];
		if( g_key_brightness.left_brightness[1] != 0 ){
			set_onoff.set_g = ADP8861_LED_ON;
		}
		set_brigh[2].st2.set_flag = ADP8861_LED_ON;
		set_brigh[2].st2.scd	  = g_key_brightness.left_brightness[2];
		if( g_key_brightness.left_brightness[2] != 0 ){
			set_onoff.set_b = ADP8861_LED_ON;
		}
		ret = adp8861_left_key_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );
		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_adp8861_keyilm_ctrl: adp8861_rgb_led_bright = %d \n",ret);
		}
		ret = adp8861_left_key_led_set( &set_onoff );
		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_adp8861_keyilm_ctrl: adp8861_rgb_led_set(%d,%d,%d) = %d \n",set_onoff.set_r,set_onoff.set_g,set_onoff.set_b,ret);
		}

	}
	mutex_unlock( &g_key_brightness.m_lock );
}

EXPORT_SYMBOL(led_brightness_set_keyilm_status);

#endif 
#endif 










static void led_brightness_set_flash(int device_no, enum led_brightness value, enum device_id_num id_num)



{
	union u_led_isc_reg		set_brigh;
	struct led_request		set_onoff;
	int						ret;
	
	memset( &set_onoff, 0, sizeof( set_onoff ) );
	memset( &set_brigh, 0, sizeof( set_brigh ) );
	
	
	if( value > LED_OFF ){
		
		switch(device_no){
		  case LED_DRV_IC_DEVICE_ID_SUB_FLASH_FLASH_MOVIE:
			set_brigh.st2.set_flag = ADP8861_LED_ON;

			set_brigh.st2.scr =
				 pg_led_congig->flash_config[id_num].brightness[LED_DRV_IC_FLASH_TYPE_MOVIE].st2.scr;
			set_brigh.st2.scd =
				 pg_led_congig->flash_config[id_num].brightness[LED_DRV_IC_FLASH_TYPE_MOVIE].st2.scd;






			set_onoff.flash_flag = ADP8861_LED_ON;
			break;
			
		  case LED_DRV_IC_DEVICE_ID_SUB_FLASH_FLASH_STILL:
			set_brigh.st2.set_flag = ADP8861_LED_ON;

			set_brigh.st2.scr =
				 pg_led_congig->flash_config[id_num].brightness[LED_DRV_IC_FLASH_TYPE_STILL].st2.scr;
			set_brigh.st2.scd =
				 pg_led_congig->flash_config[id_num].brightness[LED_DRV_IC_FLASH_TYPE_STILL].st2.scd;






			set_onoff.flash_flag = ADP8861_LED_ON;
			break;
			
		  case LED_DRV_IC_DEVICE_ID_SUB_FLASH_TORCH:
			set_brigh.st2.set_flag = ADP8861_LED_ON;

			set_brigh.st2.scr =
				 pg_led_congig->flash_config[id_num].brightness[LED_DRV_IC_FLASH_TYPE_IDLE ].st2.scr ;
			set_brigh.st2.scd =
				 pg_led_congig->flash_config[id_num].brightness[LED_DRV_IC_FLASH_TYPE_IDLE ].st2.scd ;






			set_onoff.flash_flag = ADP8861_LED_OFF;
			break;
			
		  default:
			printk(KERN_INFO "led_brightness_set_flash: device_no Error %d \n",device_no );
			return;
		}
		set_onoff.set = ADP8861_LED_ON;
	}
	else{
		
		set_brigh.st2.set_flag = ADP8861_LED_OFF;
		set_brigh.st2.scr = ADP8861_LED_OFF;
		set_brigh.st2.scd = ADP8861_LED_OFF;
		
		set_onoff.set = ADP8861_LED_OFF;
	}

	ret = adp8861_flash_led_bright( &set_brigh, id_num );



	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_brightness_set_flash: adp8861_dec_led1_3_setting = %d \n",ret);
	}
	

	ret = adp8861_flash_led_set( &set_onoff, id_num );



	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_brightness_set_flash: adp8861_flash_led_set(%d) = %d \n",set_onoff.set,ret);
	}
}

#if defined(LOCAL_CONFIG_FEATURE_DVE021_CAM_LIGHT_PREVENT_PEEPING)
static void led_brightness_set_prevent_peeping(int device_no, enum led_brightness value)
{
	union u_led_isc_reg					set_brigh;
	struct led_request_prevent_peeping	set_onoff;
	int									ret;
	
	memset( &set_onoff, 0, sizeof( set_onoff ) );
	memset( &set_brigh, 0, sizeof( set_brigh ) );
	
	
	if( value > LED_OFF ){
		
		set_brigh.st2.set_flag = ADP8861_LED_ON;
		set_brigh.st2.scr = pg_led_congig->prevent_peeping_config.brightness.st2.scr;
		set_brigh.st2.scd = pg_led_congig->prevent_peeping_config.brightness.st2.scd;
		set_onoff.set_1 = ADP8861_LED_ON;
	}
	else{
		
		set_brigh.st2.set_flag = ADP8861_LED_OFF;
		set_brigh.st2.scr = ADP8861_LED_OFF;
		set_brigh.st2.scd = ADP8861_LED_OFF;
		
		set_onoff.set_1 = ADP8861_LED_OFF;
	}
	ret = adp8861_prevent_peeping_led_bright( &set_brigh );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_brightness_set_prevent_peeping: adp8861_prevent_peeping_led_bright = %d \n",ret);
	}
	
	ret = adp8861_prevent_peeping_led_set( &set_onoff );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_brightness_set_prevent_peeping: adp8861_prevent_peeping_led_set(%d) = %d \n",set_onoff.set_1,ret);
	}
}
#endif 









static int drv_ic_led_get_config(void)
{
	if( !pg_led_congig )
	{
		pg_led_congig = (struct drv_ic_led_configs *)kzalloc(sizeof(struct drv_ic_led_configs), GFP_KERNEL);
		if (pg_led_congig == NULL) {
			printk(KERN_ERR "drv_ic_led_get_config: no memory for device\n");
			return -ENOMEM;
		}
	}
	
	
	
	pg_led_congig->rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_1].brightness[LED_DRV_IC_DEVICE_ID_RGB_RED]	=LEDS_LED_ILLU_RED_LUMIN1;
	pg_led_congig->rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_1].brightness[LED_DRV_IC_DEVICE_ID_RGB_GREEN]=LEDS_LED_ILLU_GREEN_LUMIN1;
	pg_led_congig->rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_1].brightness[LED_DRV_IC_DEVICE_ID_RGB_BLUE] =LEDS_LED_ILLU_BLUE_LUMIN1;
	
	pg_led_congig->rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_2].brightness[LED_DRV_IC_DEVICE_ID_RGB_RED]	=LEDS_LED_ILLU_RED_LUMIN2;
	pg_led_congig->rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_2].brightness[LED_DRV_IC_DEVICE_ID_RGB_GREEN]=LEDS_LED_ILLU_GREEN_LUMIN2;
	pg_led_congig->rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_2].brightness[LED_DRV_IC_DEVICE_ID_RGB_BLUE] =LEDS_LED_ILLU_BLUE_LUMIN2;
	
	pg_led_congig->rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_3].brightness[LED_DRV_IC_DEVICE_ID_RGB_RED]	=LEDS_LED_ILLU_RED_LUMIN3;
	pg_led_congig->rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_3].brightness[LED_DRV_IC_DEVICE_ID_RGB_GREEN]=LEDS_LED_ILLU_GREEN_LUMIN3;
	pg_led_congig->rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_3].brightness[LED_DRV_IC_DEVICE_ID_RGB_BLUE] =LEDS_LED_ILLU_BLUE_LUMIN3;
	

	
	pg_led_congig->er_rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_1].brightness[LED_DRV_IC_DEVICE_ID_RGB_RED]	=LEDS_LED_ILLU_RED_LUMIN1;
	pg_led_congig->er_rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_1].brightness[LED_DRV_IC_DEVICE_ID_RGB_GREEN]=LEDS_LED_ILLU_GREEN_LUMIN1;
	pg_led_congig->er_rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_1].brightness[LED_DRV_IC_DEVICE_ID_RGB_BLUE] =LEDS_LED_ILLU_BLUE_LUMIN1;

	pg_led_congig->er_rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_2].brightness[LED_DRV_IC_DEVICE_ID_RGB_RED]	=LEDS_LED_ILLU_RED_LUMIN2;
	pg_led_congig->er_rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_2].brightness[LED_DRV_IC_DEVICE_ID_RGB_GREEN]=LEDS_LED_ILLU_GREEN_LUMIN2;
	pg_led_congig->er_rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_2].brightness[LED_DRV_IC_DEVICE_ID_RGB_BLUE] =LEDS_LED_ILLU_BLUE_LUMIN2;

	pg_led_congig->er_rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_3].brightness[LED_DRV_IC_DEVICE_ID_RGB_RED]	=LEDS_LED_ILLU_RED_LUMIN3;
	pg_led_congig->er_rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_3].brightness[LED_DRV_IC_DEVICE_ID_RGB_GREEN]=LEDS_LED_ILLU_GREEN_LUMIN3;
	pg_led_congig->er_rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_3].brightness[LED_DRV_IC_DEVICE_ID_RGB_BLUE] =LEDS_LED_ILLU_BLUE_LUMIN3;


	
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 

	pg_led_congig->keybl_config.right_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_1]=LEDS_LED_KEYBL1;
	pg_led_congig->keybl_config.right_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_2]=LEDS_LED_KEYBL2;
	pg_led_congig->keybl_config.right_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_3]=LEDS_LED_KEYBL3;

	pg_led_congig->keybl_config.left_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_1]=LEDS_LED_KEYBL1;
	pg_led_congig->keybl_config.left_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_2]=LEDS_LED_KEYBL2;
	pg_led_congig->keybl_config.left_brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_3]=LEDS_LED_KEYBL3;





#endif 
	
	

	pg_led_congig->flash_config[ID_0].brightness[LED_DRV_IC_FLASH_TYPE_STILL].st3.uc1 =LEDS_SND_CAM_FLASH_LED1;
	pg_led_congig->flash_config[ID_0].brightness[LED_DRV_IC_FLASH_TYPE_MOVIE].st3.uc1 =LEDS_SND_CAM_FLASH_LED2;
	pg_led_congig->flash_config[ID_0].brightness[LED_DRV_IC_FLASH_TYPE_IDLE ].st3.uc1 =LEDS_SND_CAM_FLASH_LED3;
	pg_led_congig->flash_config[ID_1].brightness[LED_DRV_IC_FLASH_TYPE_STILL].st3.uc1 =LEDS_SND_CAM_SUB_FLASH_LED1;
	pg_led_congig->flash_config[ID_1].brightness[LED_DRV_IC_FLASH_TYPE_MOVIE].st3.uc1 =LEDS_SND_CAM_SUB_FLASH_LED2;
	pg_led_congig->flash_config[ID_1].brightness[LED_DRV_IC_FLASH_TYPE_IDLE ].st3.uc1 =LEDS_SND_CAM_SUB_FLASH_LED3;





	
#if defined(LOCAL_CONFIG_FEATURE_DVE021_CAM_LIGHT_PREVENT_PEEPING)
	
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077)
	pg_led_congig->prevent_peeping_config.brightness.st3.uc1 = 0x12;
#else 



#if defined(LOCAL_CONFIG_FEATURE_DVE021_DVE008)




	pg_led_congig->prevent_peeping_config.brightness.st3.uc1 = 0x15;

#else 
	pg_led_congig->prevent_peeping_config.brightness = 0x15;
#endif 

#endif 
#endif 
	
	return 0;
}








static int drv_ic_led_free_config(void)
{
	if( pg_led_congig ){
		kfree(pg_led_congig);
	}
	pg_led_congig = NULL;

	return 0;
}









static int drv_ic_led_probe(struct platform_device *pdev)
{
	struct drv_ic_data		*pdrvdata;
	int						ret,cnt,cnt_rev;

	
	memset( &g_rgb_brightness, 0, sizeof( struct drv_ic_led_rgb_brightness ));


	memset( &g_er_rgb_brightness, 0, sizeof( struct drv_ic_led_er_rgb_brightness ));


#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 
	memset( &g_key_brightness, 0, sizeof( struct drv_ic_led_keybl_config ));

	mutex_init( &g_key_brightness.m_lock );
#endif 

	ret = drv_ic_led_get_config();							
	if( ret == -ENOMEM ){
		goto err_alloc_failed;
	}

	pdrvdata = (struct drv_ic_data *)kzalloc(sizeof(struct drv_ic_data), GFP_KERNEL);
	if (pdrvdata == NULL) {
		printk(KERN_ERR "drv_ic_led_probe: no memory for device\n");
		ret = -ENOMEM;
		goto err_alloc_failed;
	}

	memset(pdrvdata, 0, sizeof(struct drv_ic_data));

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_RED].name = LED_DRV_IC_DEVICE_NAME_RGB_RED;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_RED].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_RED].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_GREEN].name = LED_DRV_IC_DEVICE_NAME_RGB_GREEN;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_GREEN].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_GREEN].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_BLUE].name = LED_DRV_IC_DEVICE_NAME_RGB_BLUE;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_BLUE].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_BLUE].brightness = LED_OFF;


	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_ER_RGB_RED].name = LED_DRV_IC_DEVICE_NAME_ER_RGB_RED;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_ER_RGB_RED].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_ER_RGB_RED].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_ER_RGB_GREEN].name = LED_DRV_IC_DEVICE_NAME_ER_RGB_GREEN;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_ER_RGB_GREEN].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_ER_RGB_GREEN].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_ER_RGB_BLUE].name = LED_DRV_IC_DEVICE_NAME_ER_RGB_BLUE;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_ER_RGB_BLUE].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_ER_RGB_BLUE].brightness = LED_OFF;


	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_TIME1].name = LED_DRV_IC_DEVICE_NAME_RGB_TIMER1;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_TIME1].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_TIME1].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_TIME2].name = LED_DRV_IC_DEVICE_NAME_RGB_TIMER2;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_TIME2].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_TIME2].brightness = LED_OFF;

#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 
#if defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)


	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_KEY_ILM1].name = LED_DRV_IC_DEVICE_NAME_KEY_ILM1;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_KEY_ILM1].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_KEY_ILM1].brightness = LED_OFF;


	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_KEY_ILM2].name = LED_DRV_IC_DEVICE_NAME_KEY_ILM2;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_KEY_ILM2].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_KEY_ILM2].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_KEY_ILM3].name = LED_DRV_IC_DEVICE_NAME_KEY_ILM3;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_KEY_ILM3].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_KEY_ILM3].brightness = LED_OFF;

#endif 
#endif 

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_TORCH].name = LED_DRV_IC_DEVICE_NAME_FLASH_TORCH;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_TORCH].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_TORCH].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_FLASH_MOVIE].name = LED_DRV_IC_DEVICE_NAME_FLASH_FLASH_MOVIE;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_FLASH_MOVIE].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_FLASH_MOVIE].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_FLASH_STILL].name = LED_DRV_IC_DEVICE_NAME_FLASH_FLASH_STILL;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_FLASH_STILL].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_FLASH_STILL].brightness = LED_OFF;


	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_TORCH_SUB].name = LED_DRV_IC_DEVICE_NAME_SUB_FLASH_TORCH;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_TORCH_SUB].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_TORCH_SUB].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_FLASH_MOVIE_SUB].name = LED_DRV_IC_DEVICE_NAME_SUB_FLASH_FLASH_MOVIE;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_FLASH_MOVIE_SUB].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_FLASH_MOVIE_SUB].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_FLASH_STILL_SUB].name = LED_DRV_IC_DEVICE_NAME_SUB_FLASH_FLASH_STILL;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_FLASH_STILL_SUB].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_FLASH_STILL_SUB].brightness = LED_OFF;


#if defined(LOCAL_CONFIG_FEATURE_DVE021_CAM_LIGHT_PREVENT_PEEPING)
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_PREVENT_PEEPING].name = LED_DRV_IC_DEVICE_NAME_PREVENT_PEEPING;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_PREVENT_PEEPING].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_PREVENT_PEEPING].brightness = LED_OFF;
#endif 

#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_KEYBL].name = LED_DRV_IC_DEVICE_NAME_KEYBL;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_KEYBL].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_KEYBL].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_BTNBL].name = LED_DRV_IC_DEVICE_NAME_BTNBL;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_BTNBL].brightness_set = led_brightness_set_common;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_BTNBL].brightness = LED_OFF;
#endif 

	for (cnt = 0; cnt < LED_DRV_IC_DEVICE_ID_NUM; cnt++) {	
		ret = led_classdev_register(&pdev->dev, &pdrvdata->leds[cnt]);

		printk(KERN_DEBUG "[ADP8861_LED]%s: Resigter (%s)%d \n",
							__func__,
							pdrvdata->leds[cnt].name,
							ret);

		if (ret) {
			printk(KERN_ERR "[ADP8861_LED]%s: Resigter (%s)%d Err\n",
							__func__,
							pdrvdata->leds[cnt].name,
							ret);

			goto err_led_classdev_register_failed;
		}
	}

	dev_set_drvdata(&pdev->dev, pdrvdata);

	return 0;

err_led_classdev_register_failed:
	for (cnt_rev = 0; cnt_rev < cnt; cnt_rev++){
		led_classdev_unregister(&pdrvdata->leds[cnt_rev]);
	}

err_alloc_failed:
	drv_ic_led_free_config();
	return ret;

}








static int __devexit drv_ic_led_remove(struct platform_device *pdev)
{
	struct drv_ic_data		*pdrvdata;
	int						cnt;

	drv_ic_led_free_config();

	pdrvdata = (struct drv_ic_data *)platform_get_drvdata(pdev);

	for (cnt = 0; cnt < LED_DRV_IC_DEVICE_ID_NUM; cnt++) {
		led_classdev_unregister(&pdrvdata->leds[cnt]);
	}

	if( pdrvdata ){
		kfree(pdrvdata);
	}
	return 0;
}




static struct platform_driver g_drv_ic_led_driver = {
	.probe		= drv_ic_led_probe,
	.remove		= __devexit_p(drv_ic_led_remove),
	.driver		= {
		.name	= "leds-adp8861_if",
		.owner	= THIS_MODULE,
	},
};







static int led_class_wrapper_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "led_class_wrapper_open\n");
	return 0;
}

#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077) || defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
static long led_class_wrapper_ioctl(struct file *file, unsigned int iocmd, unsigned long data)
#else 
static int led_class_wrapper_ioctl(struct inode *inode, struct file *file,
	unsigned int iocmd, unsigned long data)
#endif 
{
	int ret = 0;
	int err = 0;
	struct adp8861_led_flash_parame data_nv;
	
	printk(KERN_DEBUG "[led_class_wrapper_ioctl]%s: ioctl Enter (iocmd:0x%02X)\n", __func__,iocmd);
	
	switch(iocmd){
	  case ADP8861_CUSTOM_N40 :











		err = copy_from_user( &data_nv,
							  (unsigned char *)data, 
							  sizeof(struct adp8861_led_flash_parame) );
		

		pg_led_congig->flash_config[ID_0].brightness[LED_DRV_IC_FLASH_TYPE_STILL].st3.uc1 = data_nv.still;
		pg_led_congig->flash_config[ID_0].brightness[LED_DRV_IC_FLASH_TYPE_MOVIE].st3.uc1 = data_nv.video;
		pg_led_congig->flash_config[ID_0].brightness[LED_DRV_IC_FLASH_TYPE_IDLE].st3.uc1  = data_nv.torch;





		
		











		if( err ){
			return -EINVAL;
		}
		break;
		
	  case ADP8861_CUSTOM_N43 :



		err = copy_from_user( &pg_led_congig->prevent_peeping_config.brightness, 
							  (unsigned char *)data, 
							  sizeof(struct adp8861_led_prevent_peeping_parame) );



		if( err ){
			return -EINVAL;
		}
		break;


	  case ADP8861_CUSTOM_N44 :





		err = copy_from_user( &data_nv,
							  (unsigned char *)data,
							  sizeof(struct adp8861_led_flash_parame) );
		pg_led_congig->flash_config[ID_1].brightness[LED_DRV_IC_FLASH_TYPE_STILL].st3.uc1 = data_nv.still;
		pg_led_congig->flash_config[ID_1].brightness[LED_DRV_IC_FLASH_TYPE_MOVIE].st3.uc1 = data_nv.video;
		pg_led_congig->flash_config[ID_1].brightness[LED_DRV_IC_FLASH_TYPE_IDLE].st3.uc1  = data_nv.torch;






		break;



	  case ADP8861_CUSTOM_N45 :
		err = copy_from_user( &adp8861_3color_led_parame_nv.illu_red_lumin,
							  (unsigned char *)data,
							  sizeof(struct adp8861_3color_led_parame) );
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.illu_red_lumin.one %02x \n", adp8861_3color_led_parame_nv.illu_red_lumin.one);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.illu_red_lumin.two %02x \n", adp8861_3color_led_parame_nv.illu_red_lumin.two);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.illu_red_lumin.three %02x \n", adp8861_3color_led_parame_nv.illu_red_lumin.three);
		if( err ){
			return -EINVAL;
		}
		break;
	  case ADP8861_CUSTOM_N46 :
		err = copy_from_user( &adp8861_3color_led_parame_nv.illu_green_lumin,
							  (unsigned char *)data,
							  sizeof(struct adp8861_3color_led_parame) );
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.illu_green_lumin.one %02x \n", adp8861_3color_led_parame_nv.illu_green_lumin.one);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.illu_green_lumin.two %02x \n", adp8861_3color_led_parame_nv.illu_green_lumin.two);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.illu_green_lumin.three %02x \n", adp8861_3color_led_parame_nv.illu_green_lumin.three);
		if( err ){
			return -EINVAL;
		}
		break;
	  case ADP8861_CUSTOM_N47 :
		err = copy_from_user( &adp8861_3color_led_parame_nv.illu_blue_lumin,
							  (unsigned char *)data,
							  sizeof(struct adp8861_3color_led_parame) );
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.illu_blue_lumin.one %02x \n", adp8861_3color_led_parame_nv.illu_blue_lumin.one);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.illu_blue_lumin.two %02x \n", adp8861_3color_led_parame_nv.illu_blue_lumin.two);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.illu_blue_lumin.three %02x \n", adp8861_3color_led_parame_nv.illu_blue_lumin.three);
		if( err ){
			return -EINVAL;
		}
		break;
	  case ADP8861_CUSTOM_N48 :
		err = copy_from_user( &adp8861_3color_led_parame_nv.keybl1_red_lumin,
							  (unsigned char *)data,
							  sizeof(struct adp8861_3color_led_parame) );
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl1_red_lumin.one %02x \n", adp8861_3color_led_parame_nv.keybl1_red_lumin.one);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl1_red_lumin.two %02x \n", adp8861_3color_led_parame_nv.keybl1_red_lumin.two);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl1_red_lumin.three %02x \n", adp8861_3color_led_parame_nv.keybl1_red_lumin.three);
		if( err ){
			return -EINVAL;
		}
		break;
	  case ADP8861_CUSTOM_N49 :
		err = copy_from_user( &adp8861_3color_led_parame_nv.keybl1_green_lumin,
							  (unsigned char *)data,
							  sizeof(struct adp8861_3color_led_parame) );
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl1_green_lumin.one %02x \n", adp8861_3color_led_parame_nv.keybl1_green_lumin.one);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl1_green_lumin.two %02x \n", adp8861_3color_led_parame_nv.keybl1_green_lumin.two);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl1_green_lumin.three %02x \n", adp8861_3color_led_parame_nv.keybl1_green_lumin.three);
		if( err ){
			return -EINVAL;
		}
		break;
	  case ADP8861_CUSTOM_N50 :
		err = copy_from_user( &adp8861_3color_led_parame_nv.keybl1_blue_lumin,
							  (unsigned char *)data,
							  sizeof(struct adp8861_3color_led_parame) );
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl1_blue_lumin.one %02x \n", adp8861_3color_led_parame_nv.keybl1_blue_lumin.one);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl1_blue_lumin.two %02x \n", adp8861_3color_led_parame_nv.keybl1_blue_lumin.two);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl1_blue_lumin.three %02x \n", adp8861_3color_led_parame_nv.keybl1_blue_lumin.three);
		if( err ){
			return -EINVAL;
		}
		break;
	  case ADP8861_CUSTOM_N51 :
		err = copy_from_user( &adp8861_3color_led_parame_nv.keybl2_red_lumin,
							  (unsigned char *)data,
							  sizeof(struct adp8861_3color_led_parame) );
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl2_red_lumin.one %02x \n", adp8861_3color_led_parame_nv.keybl2_red_lumin.one);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl2_red_lumin.two %02x \n", adp8861_3color_led_parame_nv.keybl2_red_lumin.two);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl2_red_lumin.three %02x \n", adp8861_3color_led_parame_nv.keybl2_red_lumin.three);
		if( err ){
			return -EINVAL;
		}
		break;
	  case ADP8861_CUSTOM_N52 :
		err = copy_from_user( &adp8861_3color_led_parame_nv.keybl2_green_lumin,
							  (unsigned char *)data,
							  sizeof(struct adp8861_3color_led_parame) );
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl2_green_lumin.one %02x \n", adp8861_3color_led_parame_nv.keybl2_green_lumin.one);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl2_green_lumin.two %02x \n", adp8861_3color_led_parame_nv.keybl2_green_lumin.two);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl2_green_lumin.three %02x \n", adp8861_3color_led_parame_nv.keybl2_green_lumin.three);
		if( err ){
			return -EINVAL;
		}
		break;
	  case ADP8861_CUSTOM_N53 :
		err = copy_from_user( &adp8861_3color_led_parame_nv.keybl2_blue_lumin,
							  (unsigned char *)data,
							  sizeof(struct adp8861_3color_led_parame) );
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl2_blue_lumin.one %02x \n", adp8861_3color_led_parame_nv.keybl2_blue_lumin.one);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl2_blue_lumin.two %02x \n", adp8861_3color_led_parame_nv.keybl2_blue_lumin.two);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.keybl2_blue_lumin.three %02x \n", adp8861_3color_led_parame_nv.keybl2_blue_lumin.three);
		if( err ){
			return -EINVAL;
		}
		break;
	  case ADP8861_CUSTOM_N54 :
		err = copy_from_user( &adp8861_3color_led_parame_nv.hsj_red_lumin,
							  (unsigned char *)data,
							  sizeof(struct adp8861_3color_led_parame) );
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.hsj_red_lumin.one %02x \n", adp8861_3color_led_parame_nv.hsj_red_lumin.one);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.hsj_red_lumin.two %02x \n", adp8861_3color_led_parame_nv.hsj_red_lumin.two);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.hsj_red_lumin.three %02x \n", adp8861_3color_led_parame_nv.hsj_red_lumin.three);
		if( err ){
			return -EINVAL;
		}
		break;
	  case ADP8861_CUSTOM_N55 :
		err = copy_from_user( &adp8861_3color_led_parame_nv.hsj_green_lumin,
							  (unsigned char *)data,
							  sizeof(struct adp8861_3color_led_parame) );
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.hsj_green_lumin.one %02x \n", adp8861_3color_led_parame_nv.hsj_green_lumin.one);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.hsj_green_lumin.two %02x \n", adp8861_3color_led_parame_nv.hsj_green_lumin.two);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.hsj_green_lumin.three %02x \n", adp8861_3color_led_parame_nv.hsj_green_lumin.three);
		if( err ){
			return -EINVAL;
		}
		break;
	  case ADP8861_CUSTOM_N56 :
		err = copy_from_user( &adp8861_3color_led_parame_nv.hsj_blue_lumin,
							  (unsigned char *)data,
							  sizeof(struct adp8861_3color_led_parame) );
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.hsj_blue_lumin.one %02x \n", adp8861_3color_led_parame_nv.hsj_blue_lumin.one);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.hsj_blue_lumin.two %02x \n", adp8861_3color_led_parame_nv.hsj_blue_lumin.two);
		printk(KERN_INFO "[led_class_wrapper_ioctl] adp8861_3color_led_parame_nv.hsj_blue_lumin.three %02x \n", adp8861_3color_led_parame_nv.hsj_blue_lumin.three);
		if( err ){
			return -EINVAL;
		}
		break;

		
	  default:
		ret = ADP8861_LED_SET_NG;
		break;
	}
	printk(KERN_DEBUG "[led_class_wrapper_ioctl]%s: ioctl Exit\n", __func__);
	return ret;
}



int adp8861_get_3color_nv(struct adp8861_3color_led_parame_nv *three_color_led_parame)
{
	int ret = 0;

	if(three_color_led_parame == NULL)
		return -1;

	memcpy( three_color_led_parame, &adp8861_3color_led_parame_nv, sizeof( struct adp8861_3color_led_parame_nv) );

	return ret;
}




int adp8861_i2c_smbus_write(u8 command, u8 value, enum device_id_num id_num)



{
	s32	ret ;

    unsigned char alrm_info[4];
    static int alrm_cnt = 0;

	unsigned char event_id    = 0x00;

	if(adp8861.driver_data[id_num].led_starte != LED_STATE_IDLE ){



		printk(KERN_WARNING
			   "ADP8861_LED: adp8861 It isn't initialized.\n" );
		return ADP8861_LED_SET_NG;
	}
	

	
	ret = i2c_smbus_write_byte_data(adp8861.driver_data[id_num].adp8861_client, command, value);




	
	if(ret < 0){

		if(id_num == 0)
		{
			event_id = 0x6E;
		}
		else
		{
			event_id = 0x6D;
		}



		printk(KERN_WARNING
			   "ADP8861_LED: i2c_smbus_write_byte_data Error %x\n", ret );

        if( alrm_cnt == 0 )
        {
            alrm_info[0] = 0x09;        
            alrm_info[1] = (unsigned char)(ret * -1);
            alrm_info[2] = command;     
            alrm_info[3] = value;       

            printk(KERN_ERR "[T][ARM]Event:0x%02X Info:0x%02X%02X%02X%02X\n",
                            event_id,alrm_info[0],alrm_info[1],alrm_info[2],alrm_info[3]);
            alrm_cnt = 1;
        }


	}

    else
    {
        alrm_cnt = 0;
    }

	return ret;
}
EXPORT_SYMBOL(adp8861_i2c_smbus_write);


int adp8861_i2c_smbus_read(u8 command, u8 *value, enum device_id_num  id_num)



{
	s32	ret ;

    unsigned char alrm_info[4];
    static int alrm_cnt = 0;


	unsigned char event_id    = 0x00;

	
	ret = i2c_smbus_read_byte_data(adp8861.driver_data[id_num].adp8861_client, command);




	
	if(ret < 0){

		if(id_num == 0)
		{
			event_id = 0x6E;
		}
		else
		{
			event_id = 0x6D;
		}



		printk(KERN_ERR
			   "ADP8861_LED: i2c_smbus_read_byte_data Error %x\n", ret );

        if( alrm_cnt == 0 )
        {
            alrm_info[0] = 0x06;        
            alrm_info[1] = (unsigned char)(ret * -1);
            alrm_info[2] = *value;      
            alrm_info[3] = 0;           

            printk(KERN_ERR "[T][ARM]Event:0x%02X Info:0x%02X%02X%02X%02X\n",
                            event_id,alrm_info[0],alrm_info[1],alrm_info[2],alrm_info[3]);
            alrm_cnt = 1;
        }


	}
	else{
		*value = (u8)( ret & 0xFF );

        alrm_cnt = 0;

	}
	return ret;
}
EXPORT_SYMBOL(adp8861_i2c_smbus_read);

#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 
static void adp8861_pm_obs_a_keybacklight(void)
{
#if !defined(LED_PM_OBS_DISABLE)
	


	if( ( adp8861.driver_data[ID_1].adp8861_reg.iscc.st.sc1_en == ADP8861_LED_ON )
	 || ( adp8861.driver_data[ID_1].adp8861_reg.iscc.st.sc4_en == ADP8861_LED_ON )
	 || ( adp8861.driver_data[ID_1].adp8861_reg.iscc.st.sc2_en == ADP8861_LED_ON )
	 || ( adp8861.driver_data[ID_1].adp8861_reg.iscc.st.sc5_en == ADP8861_LED_ON )
	 || ( adp8861.driver_data[ID_1].adp8861_reg.iscc.st.sc3_en == ADP8861_LED_ON )
	 || ( adp8861.driver_data[ID_1].adp8861_reg.iscc.st.sc6_en == ADP8861_LED_ON ) ){








		
		
		pm_obs_a_keybacklight(PM_OBS_KEYBACKLIGHT_MODE, TRUE);




	}
	else{
		
		pm_obs_a_keybacklight(PM_OBS_KEYBACKLIGHT_MODE, FALSE);




	}
#endif 
	return;
}
#endif 

static void adp8861_pm_obs_a_camlight(struct led_request *request)
{
#if !defined(LED_PM_OBS_DISABLE)
	
	if( request->flash_flag == ADP8861_LED_ON ){
		if( request->set == ADP8861_LED_ON ){
			pm_obs_a_camlight( PM_OBS_CAMERALIGHT_MODE, TRUE );
			printk(KERN_INFO
				   "ADP8861_LED: pm_obs_a_camlight( %d, %d ) \n", PM_OBS_CAMERALIGHT_MODE, TRUE );
		}
		else{
			pm_obs_a_camlight( PM_OBS_CAMERALIGHT_MODE, FALSE );
			printk(KERN_INFO
				   "ADP8861_LED: pm_obs_a_camlight( %d, %d ) \n", PM_OBS_CAMERALIGHT_MODE, FALSE );
		}
	}
	else{
		if( request->set == ADP8861_LED_ON ){
			pm_obs_a_camlight( PM_OBS_MOBILELIGHT_MODE, TRUE );
			printk(KERN_INFO
				   "ADP8861_LED: pm_obs_a_camlight( %d, %d ) \n", PM_OBS_MOBILELIGHT_MODE, TRUE );
		}
		else{
			pm_obs_a_camlight( PM_OBS_MOBILELIGHT_MODE, FALSE );
			printk(KERN_INFO
				   "ADP8861_LED: pm_obs_a_camlight( %d, %d ) \n", PM_OBS_MOBILELIGHT_MODE, FALSE );
		}
	}
#endif 
	return;
}


int adp8861_er_rgb_led_set(struct led_request_rgb *request)
{
	int						ret;
	u8						rw_adr = LED_REG_ISCC ;
	union u_led_iscc_reg	rw_data;


	DEFINE_SPINLOCK(spin_lock_status);

	spin_lock(&spin_lock_status);

	rw_data.uc = adp8861.driver_data[ID_0].adp8861_reg.iscc.uc ;

	rw_data.st.sc4_en = request->set_r;
	rw_data.st.sc5_en = request->set_g;
	rw_data.st.sc6_en = request->set_b;
	spin_unlock(&spin_lock_status);

	adp8861_init_reg(ID_0);

	
	ret = adp8861_i2c_smbus_write(rw_adr, rw_data.uc, ID_0);

	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
		return ADP8861_LED_SET_NG;
	}
	
	spin_lock(&spin_lock_status);
	adp8861.driver_data[ID_0].adp8861_reg.iscc.st.sc4_en = request->set_r;
	adp8861.driver_data[ID_0].adp8861_reg.iscc.st.sc5_en = request->set_g;
	adp8861.driver_data[ID_0].adp8861_reg.iscc.st.sc6_en = request->set_b;
	spin_unlock(&spin_lock_status);

	if( adp8861.driver_data[ID_0].adp8861_reg.iscc.uc == LED_REG_ISCC_INI ){
		rw_adr = LED_REG_MDCR ;
		rw_data.uc = LED_REG_MDCR_INI_POFF ;
		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.uc, ID_0);
		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}
	return ADP8861_LED_SET_OK;
}
EXPORT_SYMBOL(adp8861_er_rgb_led_set);


int adp8861_er_rgb_led_bright( union u_led_isc_reg *red_bright,
							union u_led_isc_reg *green_bright,
							union u_led_isc_reg *blue_bright)
{
	int					ret;
	u8					rw_adr;
	union u_led_isc_reg	rw_data;

	if( red_bright->st2.set_flag == ADP8861_LED_ON ){
		rw_adr = LED_REG_ISC4 ;
		rw_data.us = 0 ;
		rw_data.st2.scd = red_bright->st2.scd ;
		if( rw_data.st2.scd > LED_REG_ISC4_MAX ){
			rw_data.st2.scd = LED_REG_ISC4_MAX;
		}








		
		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.st3.uc1, ID_0);

		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}
	if( green_bright->st2.set_flag == ADP8861_LED_ON ){
		rw_adr = LED_REG_ISC5 ;
		rw_data.st2.set_flag = 0 ;
		rw_data.st2.scd = green_bright->st2.scd ;
		if( rw_data.st2.scd > LED_REG_ISC5_MAX ){
			rw_data.st2.scd = LED_REG_ISC5_MAX;
		}








		
		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.st3.uc1, ID_0);

		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}
	if( blue_bright->st2.set_flag == ADP8861_LED_ON ){
		rw_adr = LED_REG_ISC6 ;
		rw_data.st2.set_flag = 0 ;
		rw_data.st2.scd = blue_bright->st2.scd ;
		if( rw_data.st2.scd > LED_REG_ISC6_MAX ){
			rw_data.st2.scd = LED_REG_ISC6_MAX;
		}








		
		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.st3.uc1, ID_0);

		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}
	
	return ADP8861_LED_SET_OK;
}
EXPORT_SYMBOL(adp8861_er_rgb_led_bright);


int adp8861_rgb_led_set(struct led_request_rgb *request)
{
	int						ret;
	u8						rw_adr = LED_REG_ISCC ;
	union u_led_iscc_reg	rw_data;

	
	DEFINE_SPINLOCK(spin_lock_status);
	
	spin_lock(&spin_lock_status);

	rw_data.uc = adp8861.driver_data[ID_0].adp8861_reg.iscc.uc ;



	rw_data.st.sc1_en = request->set_r;
	rw_data.st.sc2_en = request->set_g;
	rw_data.st.sc3_en = request->set_b;
	spin_unlock(&spin_lock_status);


	adp8861_init_reg(ID_0);







	
	ret = adp8861_i2c_smbus_write(rw_adr, rw_data.uc, ID_0);




	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
		return ADP8861_LED_SET_NG;
	}
	
	spin_lock(&spin_lock_status);

	adp8861.driver_data[ID_0].adp8861_reg.iscc.st.sc1_en = request->set_r;
	adp8861.driver_data[ID_0].adp8861_reg.iscc.st.sc2_en = request->set_g;
	adp8861.driver_data[ID_0].adp8861_reg.iscc.st.sc3_en = request->set_b;





	spin_unlock(&spin_lock_status);
	


	if( adp8861.driver_data[ID_0].adp8861_reg.iscc.uc == LED_REG_ISCC_INI ){



		rw_adr = LED_REG_MDCR ;
		rw_data.uc = LED_REG_MDCR_INI_POFF ;

		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.uc, ID_0);



		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}

	return ADP8861_LED_SET_OK;
}
EXPORT_SYMBOL(adp8861_rgb_led_set);

int adp8861_rgb_led_bright( union u_led_isc_reg *red_bright,
							union u_led_isc_reg *green_bright,
							union u_led_isc_reg *blue_bright)
{
	int					ret;
	u8					rw_adr;
	union u_led_isc_reg	rw_data;
	
	if( red_bright->st2.set_flag == ADP8861_LED_ON ){
		rw_adr = LED_REG_ISC1 ;
		rw_data.us = 0 ;
		rw_data.st2.scd = red_bright->st2.scd ;
		if( rw_data.st2.scd > LED_REG_ISC1_MAX ){
			rw_data.st2.scd = LED_REG_ISC1_MAX;
		}








		
		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.st3.uc1, ID_0);




		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}
	if( green_bright->st2.set_flag == ADP8861_LED_ON ){
		rw_adr = LED_REG_ISC2 ;
		rw_data.st2.set_flag = 0 ;
		rw_data.st2.scd = green_bright->st2.scd ;
		if( rw_data.st2.scd > LED_REG_ISC2_MAX ){
			rw_data.st2.scd = LED_REG_ISC2_MAX;
		}








		
		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.st3.uc1, ID_0);




		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}
	if( blue_bright->st2.set_flag == ADP8861_LED_ON ){
		rw_adr = LED_REG_ISC3 ;
		rw_data.st2.set_flag = 0 ;
		rw_data.st2.scd = blue_bright->st2.scd ;
		if( rw_data.st2.scd > LED_REG_ISC3_MAX ){
			rw_data.st2.scd = LED_REG_ISC3_MAX;
		}








		
		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.st3.uc1, ID_0);




		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}
	
	return ADP8861_LED_SET_OK;	
}
EXPORT_SYMBOL(adp8861_rgb_led_bright);

































#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 



int adp8861_left_key_led_set(struct led_request_rgb *request)



{
	int						ret = ADP8861_LED_OFF ;
	u8						rw_adr = LED_REG_ISCC;
	union u_led_iscc_reg	rw_data;

	
	DEFINE_SPINLOCK(spin_lock_status);
	





	
	spin_lock(&spin_lock_status);

	rw_data.uc = adp8861.driver_data[ID_1].adp8861_reg.iscc.uc ;





	rw_data.st.sc4_en = request->set_r;
	rw_data.st.sc5_en = request->set_g;
	rw_data.st.sc6_en = request->set_b;








	spin_unlock(&spin_lock_status);
	

	adp8861_init_reg(ID_1);







	
	ret = adp8861_i2c_smbus_write(rw_adr, rw_data.uc, ID_1);




	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
		return ADP8861_LED_SET_NG;
	}
	
	spin_lock(&spin_lock_status);

	adp8861.driver_data[ID_1].adp8861_reg.iscc.st.sc4_en = request->set_r;
	adp8861.driver_data[ID_1].adp8861_reg.iscc.st.sc5_en = request->set_g;
	adp8861.driver_data[ID_1].adp8861_reg.iscc.st.sc6_en = request->set_b;














	spin_unlock(&spin_lock_status);
	


	if( adp8861.driver_data[ID_1].adp8861_reg.iscc.uc == LED_REG_ISCC_INI ){



		rw_adr = LED_REG_MDCR ;
		rw_data.uc = LED_REG_MDCR_INI_POFF ;

		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.uc, ID_1);



		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}

	adp8861_pm_obs_a_keybacklight();
	
	return ADP8861_LED_SET_OK;
}

EXPORT_SYMBOL(adp8861_left_key_led_set);





int adp8861_right_key_led_set(struct led_request_rgb *request)
{
	int						ret = ADP8861_LED_OFF ;
	u8						rw_adr = LED_REG_ISCC;
	union u_led_iscc_reg	rw_data;


	DEFINE_SPINLOCK(spin_lock_status);







	spin_lock(&spin_lock_status);

	rw_data.uc = adp8861.driver_data[ID_1].adp8861_reg.iscc.uc ;

	rw_data.st.sc1_en = request->set_r;
	rw_data.st.sc2_en = request->set_g;
	rw_data.st.sc3_en = request->set_b;

	spin_unlock(&spin_lock_status);

	adp8861_init_reg(ID_1);

	
	ret = adp8861_i2c_smbus_write(rw_adr, rw_data.uc, ID_1);

	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
		return ADP8861_LED_SET_NG;
	}
	
	spin_lock(&spin_lock_status);

	adp8861.driver_data[ID_1].adp8861_reg.iscc.st.sc1_en = request->set_r;
	adp8861.driver_data[ID_1].adp8861_reg.iscc.st.sc2_en = request->set_g;
	adp8861.driver_data[ID_1].adp8861_reg.iscc.st.sc3_en = request->set_b;

	spin_unlock(&spin_lock_status);

	if( adp8861.driver_data[ID_1].adp8861_reg.iscc.uc == LED_REG_ISCC_INI ){

		rw_adr = LED_REG_MDCR ;
		rw_data.uc = LED_REG_MDCR_INI_POFF ;

		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.uc, ID_1);

		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}

	adp8861_pm_obs_a_keybacklight();

	return ADP8861_LED_SET_OK;
}
EXPORT_SYMBOL(adp8861_right_key_led_set);







int adp8861_left_key_led_bright( union u_led_isc_reg *led_key_bright1,
                            union u_led_isc_reg *led_key_bright2,
                            union u_led_isc_reg *led_key_bright3)



{
	int	ret;
	u8	rw_adr ;
	union u_led_isc_reg	rw_data;
	





	
	if( led_key_bright1->st2.set_flag == ADP8861_LED_ON ){
		rw_adr = LED_REG_ISC4 ;
		rw_data.us = 0 ;
		rw_data.st2.scd = led_key_bright1->st2.scd ;
		if( rw_data.st2.scd > LED_REG_ISC4_1_MAX ){
			rw_data.st2.scd = LED_REG_ISC4_1_MAX;
		}



		

		
		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.st3.uc1, ID_1);




		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}

	if( led_key_bright2->st2.set_flag == ADP8861_LED_ON ){
		rw_adr = LED_REG_ISC5 ;
		rw_data.us = 0 ;
		rw_data.st2.scd = led_key_bright2->st2.scd ;
		if( rw_data.st2.scd > LED_REG_ISC5_1_MAX ){
			rw_data.st2.scd = LED_REG_ISC5_1_MAX;
		}



		

		
		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.st3.uc1, ID_1);




		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}
	if( led_key_bright3->st2.set_flag == ADP8861_LED_ON ){
		rw_adr = LED_REG_ISC6 ;
		rw_data.us = 0 ;
		rw_data.st2.scd = led_key_bright3->st2.scd ;
		if( rw_data.st2.scd > LED_REG_ISC6_1_MAX ){
			rw_data.st2.scd = LED_REG_ISC6_1_MAX;
		}



		

		
		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.st3.uc1, ID_1);




		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}

	
	return ADP8861_LED_SET_OK;
}

EXPORT_SYMBOL(adp8861_left_key_led_bright);



#endif 


int adp8861_right_key_led_bright( union u_led_isc_reg *led_key_bright1,
                            union u_led_isc_reg *led_key_bright2,
                            union u_led_isc_reg *led_key_bright3)
{
	int	ret;
	u8	rw_adr ;
	union u_led_isc_reg	rw_data;







	if( led_key_bright1->st2.set_flag == ADP8861_LED_ON ){
		rw_adr = LED_REG_ISC1 ;
		rw_data.us = 0 ;
		rw_data.st2.scd = led_key_bright1->st2.scd ;
		if( rw_data.st2.scd > LED_REG_ISC1_1_MAX ){
			rw_data.st2.scd = LED_REG_ISC1_1_MAX;
		}




		
		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.st3.uc1, ID_1);

		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}

	if( led_key_bright2->st2.set_flag == ADP8861_LED_ON ){
		rw_adr = LED_REG_ISC2 ;
		rw_data.us = 0 ;
		rw_data.st2.scd = led_key_bright2->st2.scd ;
		if( rw_data.st2.scd > LED_REG_ISC2_1_MAX ){
			rw_data.st2.scd = LED_REG_ISC2_1_MAX;
		}




		
		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.st3.uc1, ID_1);

		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}
	if( led_key_bright3->st2.set_flag == ADP8861_LED_ON ){
		rw_adr = LED_REG_ISC3 ;
		rw_data.us = 0 ;
		rw_data.st2.scd = led_key_bright3->st2.scd ;
		if( rw_data.st2.scd > LED_REG_ISC3_1_MAX ){
			rw_data.st2.scd = LED_REG_ISC3_1_MAX;
		}




		
		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.st3.uc1, ID_1);

		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}
	
	return ADP8861_LED_SET_OK;
}
EXPORT_SYMBOL(adp8861_right_key_led_bright);


#if defined(LOCAL_CONFIG_FEATURE_DVE021_CAM_LIGHT_PREVENT_PEEPING)
int adp8861_prevent_peeping_led_set(struct led_request_prevent_peeping *request)
{
	int						ret = ADP8861_LED_OFF ;
	u8						rw_adr = LED_REG_ISCC;
	union u_led_iscc_reg	rw_data;

	
	DEFINE_SPINLOCK(spin_lock_status);
	
	spin_lock(&spin_lock_status);
	rw_data.uc = adp8861_reg.iscc.uc ;
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
	rw_data.st.sc4_en = request->set_1;
#else 



	rw_data.st.sc5_en = request->set_1;

#endif 
	spin_unlock(&spin_lock_status);
	
#if  defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077) || defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
	adp8861_init_reg();
#endif
	
	
	ret = adp8861_i2c_smbus_write(rw_adr, rw_data.uc);
	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
		return ADP8861_LED_SET_NG;
	}
	
	spin_lock(&spin_lock_status);
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
	adp8861_reg.iscc.st.sc4_en = request->set_1;
#else 



	adp8861_reg.iscc.st.sc5_en = request->set_1;

#endif 
	spin_unlock(&spin_lock_status);
	
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077) || defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
	if( adp8861_reg.iscc.uc == LED_REG_ISCC_INI ){
		rw_adr = LED_REG_MDCR ;
		rw_data.uc = LED_REG_MDCR_INI_POFF ;
		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.uc);
		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}
#endif
	return ADP8861_LED_SET_OK;
}
EXPORT_SYMBOL(adp8861_prevent_peeping_led_set);

int adp8861_prevent_peeping_led_bright( union u_led_isc_reg *led_key_bright1)
{
	int	ret;
	u8	rw_adr ;
	union u_led_isc_reg	rw_data;
	
	if( led_key_bright1->st2.set_flag == ADP8861_LED_ON ){
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
		rw_adr = LED_REG_ISC4 ;
		rw_data.us = 0 ;
		rw_data.st2.scd = led_key_bright1->st2.scd ;
		if( rw_data.st2.scd > LED_REG_ISC4_MAX ){
			rw_data.st2.scd = LED_REG_ISC4_MAX;
		}
#else 








		rw_adr = LED_REG_ISC5 ;
		rw_data.us = 0 ;
		rw_data.st2.scd = led_key_bright1->st2.scd ;
		if( rw_data.st2.scd > LED_REG_ISC5_MAX ){
			rw_data.st2.scd = LED_REG_ISC5_MAX;
		}

#endif 
		
		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.st3.uc1);
		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}
	
	return ADP8861_LED_SET_OK;
}
EXPORT_SYMBOL(adp8861_prevent_peeping_led_bright);
#endif 


int adp8861_flash_led_set(struct led_request *request, enum device_id_num id_num)



{
	int						ret;
	u8						rw_adr = LED_REG_ISCC ;

	u8						rw_adr_read = LED_REG_ISCF ;
	u8						rw_iscf = 0;
	u8						rw_iscf_temp = 0;

	union u_led_iscc_reg	rw_data;

	
	DEFINE_SPINLOCK(spin_lock_status);
	
	spin_lock(&spin_lock_status);

	rw_data.uc = adp8861.driver_data[id_num].adp8861_reg.iscc.uc ;



	rw_data.st.sc7_en = request->set;
	spin_unlock(&spin_lock_status);


	ret = adp8861_i2c_smbus_read(rw_adr_read, &rw_iscf, id_num);
	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Read Reg %x Error %x \n", rw_adr_read, ret );
		return ADP8861_LED_SET_NG;
	}

	if( request->set == ADP8861_LED_ON ){
		if( (rw_iscf & 0x0F) != 0 ){
			rw_iscf_temp = ( rw_iscf & 0xF0 );
		}
	}else {
		if( (rw_iscf & 0xF0) != 0 ){
			rw_iscf_temp = ( rw_iscf & 0x0F );
		}
	}
	ret = adp8861_i2c_smbus_write(rw_adr_read, rw_iscf_temp, id_num);
	if(ret < 0){
		printk(KERN_WARNING
			"ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr_read, ret );
		return ADP8861_LED_SET_NG;
	}

	

	adp8861_init_reg(id_num);







	
	ret = adp8861_i2c_smbus_write(rw_adr, rw_data.uc, id_num);




	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
		return ADP8861_LED_SET_NG;
	}
	
	spin_lock(&spin_lock_status);

	adp8861.driver_data[id_num].adp8861_reg.iscc.st.sc7_en = request->set;



	spin_unlock(&spin_lock_status);


	ret = adp8861_i2c_smbus_write(rw_adr_read, rw_iscf, id_num);
	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr_read, ret );
		return ADP8861_LED_SET_NG;
	}

	


	if( adp8861.driver_data[id_num].adp8861_reg.iscc.uc == LED_REG_ISCC_INI ){



		rw_adr = LED_REG_MDCR ;
		rw_data.uc = LED_REG_MDCR_INI_POFF ;

		ret = adp8861_i2c_smbus_write(rw_adr, rw_data.uc, id_num);



		if(ret < 0){
			printk(KERN_WARNING
				   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
			return ADP8861_LED_SET_NG;
		}
	}

	adp8861_pm_obs_a_camlight( request );
	
	return ADP8861_LED_SET_OK;
}
EXPORT_SYMBOL(adp8861_flash_led_set);


int adp8861_flash_led_bright( union u_led_isc_reg *led_flash_bright, enum device_id_num id_num )



{
	int					ret;
	u8					rw_adr;
	union u_led_isc_reg	rw_data;
	
	rw_adr = LED_REG_ISC7 ;
	rw_data.us = 0 ;
	rw_data.st2.scd = led_flash_bright->st2.scd ;
	rw_data.st2.scr = led_flash_bright->st2.scr ;


	if((rw_data.st2.scr == ADP8861_LED_ON ) && ( rw_data.st2.scd > LED_REG_ISC7_D_MAX )){
		rw_data.st2.scd = LED_REG_ISC7_D_MAX;
	}
	else if((rw_data.st2.scr == ADP8861_LED_OFF ) && ( rw_data.st2.scd > LED_REG_ISC7_MAX )){
		rw_data.st2.scd = LED_REG_ISC7_MAX;
	}







	
	ret = adp8861_i2c_smbus_write(rw_adr, rw_data.st3.uc1, id_num);




	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
		return ADP8861_LED_SET_NG;
	}
	
	return ADP8861_LED_SET_OK;
}
EXPORT_SYMBOL(adp8861_flash_led_bright);


int adp8861_led_timer_set1( union u_led_isct1_reg *led_timer_set,
							union u_led_isct1_reg *led_timer_data,enum device_id_num id_num )




{
	int						ret;
	u8						rw_adr;
	union u_led_isct1_reg	rw_data;

	
	DEFINE_SPINLOCK(spin_lock_status);
	
	rw_adr = LED_REG_ISCT1 ;
	spin_lock(&spin_lock_status);

	rw_data.uc = adp8861.driver_data[id_num].adp8861_reg.isct1.uc;



	spin_unlock(&spin_lock_status);
	
	if( led_timer_set->st2.scon == ADP8861_LED_ON ){
		rw_data.st2.scon = led_timer_data->st2.scon ;
	}
	if( led_timer_set->st2.sc7_off == ADP8861_LED_ON ){
		rw_data.st2.sc7_off = led_timer_data->st2.sc7_off ;
	}
	if( led_timer_set->st2.sc6_off == ADP8861_LED_ON ){
		rw_data.st2.sc6_off = led_timer_data->st2.sc6_off ;
	}
	if( led_timer_set->st2.sc5_off == ADP8861_LED_ON ){
		rw_data.st2.sc5_off = led_timer_data->st2.sc5_off ;
	}

	
	ret = adp8861_i2c_smbus_write(rw_adr, rw_data.uc, id_num);




	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
		return ADP8861_LED_SET_NG;
	}

	
	adp8861.driver_data[id_num].adp8861_rgb_time.time1.uc = led_timer_data->uc;




	
	spin_lock(&spin_lock_status);
	if( led_timer_set->st2.scon == ADP8861_LED_ON ){

		adp8861.driver_data[id_num].adp8861_reg.isct1.st2.scon = led_timer_data->st2.scon ;



	}
	if( led_timer_set->st2.sc7_off == ADP8861_LED_ON ){

		adp8861.driver_data[id_num].adp8861_reg.isct1.st2.sc7_off = led_timer_data->st2.sc7_off ;



	}
	if( led_timer_set->st2.sc6_off == ADP8861_LED_ON ){

		adp8861.driver_data[id_num].adp8861_reg.isct1.st2.sc6_off = led_timer_data->st2.sc6_off ;



	}
	if( led_timer_set->st2.sc5_off == ADP8861_LED_ON ){

		adp8861.driver_data[id_num].adp8861_reg.isct1.st2.sc5_off = led_timer_data->st2.sc5_off ;



	}
	spin_unlock(&spin_lock_status);
	return ADP8861_LED_SET_OK;
}
EXPORT_SYMBOL(adp8861_led_timer_set1);


int adp8861_led_timer_set2( union u_led_isct2_reg *led_timer_set,
							union u_led_isct2_reg *led_timer_data, enum device_id_num id_num )




{
	int						ret;
	u8						rw_adr;
	union u_led_isct2_reg	rw_data;

	
	DEFINE_SPINLOCK(spin_lock_status);
	
	rw_adr = LED_REG_ISCT2 ;
	spin_lock(&spin_lock_status);

	rw_data.uc = adp8861.driver_data[id_num].adp8861_reg.isct2.uc;



	spin_unlock(&spin_lock_status);
	
	if( led_timer_set->st2.sc4_off == ADP8861_LED_ON ){
		rw_data.st2.sc4_off = led_timer_data->st2.sc4_off ;
	}
	if( led_timer_set->st2.sc3_off == ADP8861_LED_ON ){
		rw_data.st2.sc3_off = led_timer_data->st2.sc3_off ;
	}
	if( led_timer_set->st2.sc2_off == ADP8861_LED_ON ){
		rw_data.st2.sc2_off = led_timer_data->st2.sc2_off ;
	}
	if( led_timer_set->st2.sc1_off == ADP8861_LED_ON ){
		rw_data.st2.sc1_off = led_timer_data->st2.sc1_off ;
	}

	
	ret = adp8861_i2c_smbus_write(rw_adr, rw_data.uc, id_num);




	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
		return ADP8861_LED_SET_NG;
	}

	
	adp8861.driver_data[id_num].adp8861_rgb_time.time2.uc = led_timer_data->uc;




	
	spin_lock(&spin_lock_status);
	if( led_timer_set->st2.sc4_off == ADP8861_LED_ON ){

		adp8861.driver_data[id_num].adp8861_reg.isct2.st2.sc4_off = led_timer_data->st2.sc4_off ;



	}
	if( led_timer_set->st2.sc3_off == ADP8861_LED_ON ){

		adp8861.driver_data[id_num].adp8861_reg.isct2.st2.sc3_off = led_timer_data->st2.sc3_off ;



	}
	if( led_timer_set->st2.sc2_off == ADP8861_LED_ON ){

		adp8861.driver_data[id_num].adp8861_reg.isct2.st2.sc2_off = led_timer_data->st2.sc2_off ;



	}
	if( led_timer_set->st2.sc1_off == ADP8861_LED_ON ){

		adp8861.driver_data[id_num].adp8861_reg.isct2.st2.sc1_off = led_timer_data->st2.sc1_off ;



	}
	spin_unlock(&spin_lock_status);
	return ADP8861_LED_SET_OK;
}
EXPORT_SYMBOL(adp8861_led_timer_set2);


int adp8861_led_fade_set( union u_led_iscf_reg *led_fade, enum device_id_num id_num )



{
	int						ret;
	u8						rw_adr;
	union u_led_iscf_reg	rw_data;
	
	rw_adr = LED_REG_ISCF ;
	rw_data.uc = led_fade->uc;
	

	
	ret = adp8861_i2c_smbus_write(rw_adr, rw_data.uc, id_num);




	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
		return ADP8861_LED_SET_NG;
	}
	
	return ADP8861_LED_SET_OK;
}
EXPORT_SYMBOL(adp8861_led_fade_set);


int adp8861_reg_write( unsigned char sub2,
					   unsigned char inf1, enum device_id_num id_num )




{
	int	ret;
	

	ret = adp8861_i2c_smbus_write(sub2, (u8)inf1, id_num);



	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Write Reg %x Error %x \n", sub2, ret );
		return ADP8861_LED_REG_WRITE_NG;
	}
	return ADP8861_LED_SET_OK;
}
EXPORT_SYMBOL(adp8861_reg_write);



static int adp8861_init_reg(enum device_id_num id_num)



{
	int			ret ;
	u8			rw_adr ;
	u8			rw_data ;
	
	

	
	rw_adr = LED_REG_MFDVID ;
	rw_data = LED_REG_MFDVID_INI ;

	ret = adp8861_i2c_smbus_write(rw_adr, rw_data, id_num);



	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
		return ADP8861_LED_SET_NG;
	}
	rw_adr = LED_REG_MDCR ;
	rw_data = LED_REG_MDCR_INI ;

	ret = adp8861_i2c_smbus_write(rw_adr, rw_data, id_num);



	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
		return ADP8861_LED_SET_NG;
	}
	rw_adr = LED_REG_BLSEN ;
	rw_data = LED_REG_BLSEN_INI ;

	ret = adp8861_i2c_smbus_write(rw_adr, rw_data, id_num);



	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Write Reg %x Error %x \n", rw_adr, ret );
		return ADP8861_LED_SET_NG;
	}
	
	return ADP8861_LED_SET_OK;
}



static int adp8861_init_client(struct i2c_client *client)
{
	struct led_request request;
	int			ret ;
	u8			rw_adr ;
	u8			rw_data ;
	int			count ;
	

	int *id_num = i2c_get_clientdata(client);


#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
#else 
	adp8861_pm_obs_a_keybacklight();
#endif 
	
	memset( &request, 0, sizeof( request ) );
	adp8861_pm_obs_a_camlight(&request);
	
	rw_adr = LED_REG_ISCC ;
	count = LED_REG_ISCC_COUNT;
	while( count > 0 ){

		ret = adp8861_i2c_smbus_read(rw_adr, &rw_data, *id_num);



		if(ret >= 0){
			break;
		}
		count--;
	}
	if(ret < 0){
		printk(KERN_WARNING
			   "ADP8861_LED: i2c Read Reg %x Error %x \n", rw_adr, ret );
		return ADP8861_LED_SET_NG;
	}

	adp8861.driver_data[*id_num].adp8861_reg.iscc.uc = rw_data;
	adp8861.driver_data[*id_num].adp8861_reg.iscc.st.dmy1 = 0;




	
	
	return ADP8861_LED_SET_OK;
}

static int adp8861_init_client_data( void )
{

	
	DEFINE_SPINLOCK(spin_lock_status);
	
	
	spin_lock(&spin_lock_status);

	adp8861.driver_data[ID_0].adp8861_reg.iscc.uc = LED_REG_ISCC_INI ;
	adp8861.driver_data[ID_1].adp8861_reg.iscc.uc = LED_REG_ISCC_INI ;



	spin_unlock(&spin_lock_status);
	
	
	return ADP8861_LED_SET_OK;
}


static int adp8861_i2c_probe(struct i2c_client *client,
							   const struct i2c_device_id *id)
{
	int rc = 0;
#if defined(LED_REG_GPIO_HWEN_GPIO)
#else 
	struct leds_adp8861_platform_data	*p_data = client->dev.platform_data;
#endif 


	adp8861.driver_data[id->driver_data].id_device_num =id->driver_data;
	adp8861.driver_data[id->driver_data].led_starte = LED_STATE_INITIALIZATION;



	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		rc = -ENOTSUPP;
		goto probe_failure;
	}
	

	adp8861.driver_data[id->driver_data].adp8861_led  =
		kzalloc(sizeof(struct adp8861_work), GFP_KERNEL);






	if( adp8861.driver_data[id->driver_data].adp8861_led == NULL) {



		rc = -ENOMEM;
		goto probe_failure;
	}
#if defined(LED_REG_GPIO_HWEN_GPIO)

	if( id->driver_data == ID_0 ){
		gpio_set_value(LED_REG_GPIO_RESET, ADP8861_LED_ON);
	}else if( id->driver_data == ID_1 ){
		gpio_set_value(CAM_LED_REG_GPIO_RESET, ADP8861_LED_ON);
	}



#else 
	if( p_data ){
		if ( p_data->poweron ){
			rc = p_data->poweron( &client->dev );
			printk(KERN_DEBUG "ADP8861_LED:%s: poweron (%d)\n", __func__,rc);
			if ( rc )
			{
				printk(KERN_ERR
				   "ADP8861_LED: poweron Error %x \n", rc );
				goto probe_failure;
			}
		}
	}
#endif 
	mdelay(LED_REG_GPIO_WAIT);
	

	adp8861.driver_data[id->driver_data].adp8861_client = client;
	i2c_set_clientdata(client, &adp8861.driver_data[id->driver_data].id_device_num);
	adp8861.driver_data[id->driver_data].led_starte = LED_STATE_IDLE;





	adp8861_init_client(client);
	printk(KERN_INFO
		   "ADP8861_LED: adp8861_probe succeeded!\n" );
	return 0;
	
probe_failure:

	adp8861.driver_data[id->driver_data].led_starte = LED_STATE_ENDING;



	printk(KERN_WARNING
		   "ADP8861_LED: adp8861_probe failed!\n" );
	return rc;
}


static int adp8861_i2c_remove(struct i2c_client *client)
{

	int *id_num = i2c_get_clientdata(client);


#if defined(LED_REG_GPIO_HWEN_GPIO)

	if( *id_num == ID_0 ){
		gpio_set_value(LED_REG_GPIO_RESET, ADP8861_LED_OFF);
	}else if( *id_num == ID_1 ){
		gpio_set_value(CAM_LED_REG_GPIO_RESET, ADP8861_LED_OFF);
	}



#endif 


	kfree(adp8861.driver_data[*id_num].adp8861_led);
	adp8861.driver_data[*id_num].adp8861_led = NULL;




	printk(KERN_INFO
		   "ADP8861_LED: adp8861_remove succeeded!\n" );
	return 0;
}

static const struct i2c_device_id adp8861_i2c_id[] = {

	{ "led_adp8861", ID_0},
	{ "led_adp8861_1", ID_1},





	{ },

};


static struct i2c_driver led_adp8861_driver = {
	.id_table = adp8861_i2c_id,
	.probe	  = adp8861_i2c_probe,
	.remove	  = adp8861_i2c_remove,
	
	.driver = {
			.name = "led_adp8861",
			.owner = THIS_MODULE,
	},
};





static const struct file_operations led_class_wrapper_fops = {
	.owner		= THIS_MODULE,
	.open		= led_class_wrapper_open,
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077) || defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE078)
	.unlocked_ioctl	= led_class_wrapper_ioctl,
#else 
	.ioctl		= led_class_wrapper_ioctl,
#endif 
};



static struct miscdevice led_class_wrapper = {
	.fops		= &led_class_wrapper_fops,
	.name		= DEVICEWRPPER_NAME,
	.minor		= MISC_DYNAMIC_MINOR,
};




static void init_driver_data(void)
{
	int i = 0;

	memset( &adp8861_3color_led_parame_nv, 0, sizeof( struct adp8861_3color_led_parame_nv ) );

	adp8861.driver_data[ID_0].led_starte = LED_STATE_INITIALIZATION_WAIT;
	adp8861.driver_data[ID_1].led_starte = LED_STATE_INITIALIZATION_WAIT;

	for(i=0; i<ID_MAX; i++)
	{
		adp8861.driver_data[i].adp8861_reg.iscc.uc = LED_REG_ISCC_INI;
		adp8861.driver_data[i].adp8861_reg.isct1.uc = LED_REG_ISCT1_INI;
		adp8861.driver_data[i].adp8861_reg.isct2.uc = LED_REG_ISCT2_INI;

		adp8861.driver_data[i].adp8861_rgb_time.time1.uc = 0;
		adp8861.driver_data[i].adp8861_rgb_time.time2.uc = 0;
		adp8861.driver_data[i].adp8861_rgb_time.rgb.uc = 0;
	}
}


static int __init adp8861_led_init(void)
{
	int ret;
	misc_register(&led_class_wrapper);
	ret = 0;
	ret = platform_driver_register(&g_drv_ic_led_driver);
	if( ret != 0 ){
	printk(KERN_WARNING
		   "adp8861_led_init: platform_driver_register failed! %d \n", ret );
		return ret;
	}

	init_driver_data();



	ret = i2c_add_driver(&led_adp8861_driver);
	if( ret != 0 ){
	printk(KERN_WARNING
		   "adp8861_led_init: i2c_add_driver failed! %d\n", ret );
	}
	return ret;
}

static void __exit adp8861_led_exit(void)
{

	adp8861.driver_data[ID_0].led_starte = LED_STATE_ENDING;
	adp8861.driver_data[ID_1].led_starte = LED_STATE_ENDING;



	i2c_del_driver(&led_adp8861_driver); 
	adp8861_init_client_data() ;
	
	misc_deregister(&led_class_wrapper);
	platform_driver_unregister(&g_drv_ic_led_driver);
}

MODULE_DESCRIPTION("LED ADP8861 Driver");
MODULE_LICENSE("GPL");

module_init(adp8861_led_init);
module_exit(adp8861_led_exit);



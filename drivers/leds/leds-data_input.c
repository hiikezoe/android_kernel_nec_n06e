/*
 * leds-data_input.c
 *
 * Copyright (C) NEC CASIO Mobile Communications, Ltd.
 *
 */
 
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <asm/page.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/wakelock.h>

#include <linux/module.h>


#if !defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)

#define LOCAL_CONFIG_FEATURE_DVE021_DVE902

#endif 

#if !defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077)
#if  defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)
#define LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077
#endif 
#endif 

#if !defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)
#if  defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077)
#define LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM
#endif
#endif

#if !defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
#if defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)

#define LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES


#else
#define LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES
#endif
#endif 

#if defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)
#include <linux/leds-adp8861.h>
#endif 



#include <linux/leds-data_input.h>



#define LED_LIGHTS_INPUT_NAME         "led-lights-data"
#define LED_DVE028_INPUT_NAME      "led-DVE029-data"



#define LED_DATA_INPUT_OK			0
#define LED_DATA_INPUT_END			1
#define LED_DATA_INPUT_FLIE_END		2
#define LED_DATA_INPUT_NG			-1
#define LED_DATA_INPUT_FLIE_NOT		-2

#define UP_FRAME_DATA_ON(X) ((( up_frame_data[X].s_data.fade.fade & 0x01 ) == 0x00 )  \
							&& (( up_frame_data[X].s_data.u_time.time == 0x12 )  \
							 || ( up_frame_data[X].s_data.u_time.time == 0x16 )  \
							 || ( up_frame_data[X].s_data.u_time.time == 0x18 )  \
							 || ( up_frame_data[X].s_data.u_time.time == 0x1C )) \
							&& (( up_frame_data[X].s_data.brightness[0] != 0 )   \
							 || ( up_frame_data[X].s_data.brightness[1] != 0 )   \
							 || ( up_frame_data[X].s_data.brightness[2] != 0 )))

#define UP_FRAME_DATA_OFF(X) ((( up_frame_data[X].s_data.fade.fade & 0x01 ) == 0x00 ) \
							&& (( up_frame_data[X].s_data.u_time.time == 0x16 )  \
							 || ( up_frame_data[X].s_data.u_time.time == 0x1C )) \
							&& ( up_frame_data[X].s_data.brightness[0] == 0 )    \
							&& ( up_frame_data[X].s_data.brightness[1] == 0 )    \
							&& ( up_frame_data[X].s_data.brightness[2] == 0 ))



#define UP_FRAME_DATA_TWO_LED_ON(X) ((( up_frame_data[X].s_data.fade.fade & 0x01 ) == 0x00 )  \
							&& (( up_frame_data[X].s_data.u_time.time == 0x12 )  \
							 || ( up_frame_data[X].s_data.u_time.time == 0x16 )  \
							 || ( up_frame_data[X].s_data.u_time.time == 0x18 )  \
							 || ( up_frame_data[X].s_data.u_time.time == 0x1C )) \
							&& (( up_frame_data[X].s_data.brightness[0] != 0 )   \
							 || ( up_frame_data[X].s_data.brightness[1] != 0 )   \
							 || ( up_frame_data[X].s_data.brightness[2] != 0 )   \
							 || ( up_frame_data[X].s_data.brightness[3] != 0 )   \
							 || ( up_frame_data[X].s_data.brightness[4] != 0 )   \
							 || ( up_frame_data[X].s_data.brightness[5] != 0 )))

#define UP_FRAME_DATA_TWO_LED_OFF(X) ((( up_frame_data[X].s_data.fade.fade & 0x01 ) == 0x00 ) \
							&& (( up_frame_data[X].s_data.u_time.time == 0x16 )  \
							 || ( up_frame_data[X].s_data.u_time.time == 0x1C )) \
							&& ( up_frame_data[X].s_data.brightness[0] == 0 )    \
							&& ( up_frame_data[X].s_data.brightness[1] == 0 )    \
							&& ( up_frame_data[X].s_data.brightness[2] == 0 )    \
							&& ( up_frame_data[X].s_data.brightness[3] == 0 )    \
							&& ( up_frame_data[X].s_data.brightness[4] == 0 )    \
							&& ( up_frame_data[X].s_data.brightness[5] == 0 ))


enum led_lock_num{
	LED_LOCK_SET = 0,
	LED_UNLOCK_SET
};

enum led_lock_type_num{
	LED_LOCK_TYPE_INCOMING = 0,
	LED_LOCK_TYPE_FRONT,
	LED_LOCK_TYPE_BACK,
	LED_LOCK_TYPE_REFLECT
};

enum led_flag_num{
	LED_FLAG_OFF = 0,
	LED_FLAG_ON,
	LED_FLAG_GET
};

typedef struct led_input_data_struct {
	unsigned char 					*p_patern;
	struct input_data_head_struct	s_data_info;
} led_input_data_type;

typedef struct led_patern_data_struct {
	struct led_input_data_struct	s_set_data;
	struct led_input_data_struct	s_ready_data;
	struct task_struct 				*ps_thread;
	struct mutex	 				data_lock;
	unsigned int					frame_cnt;
	int								active_flag;
	int								loop_flag;
	unsigned int					loop_cnt;
	int								check_flag;

} led_patern_data_type;

#if defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)
typedef union u_battery_data {
	unsigned int			data;
	u8						uc[4];
} u_battery_data_es_type;
#endif 

typedef struct led_patern_data_all_struct {
	led_patern_data_type	incoming;
	led_patern_data_type	front;
	led_patern_data_type	reflect;




#if defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)
	u_battery_data_es_type	battery;
#endif 
	struct mutex	 				func_lock;
} led_patern_data_all_type;

typedef union u_input_frame_es {
	struct input_frame_es_struct	s_data;
	u8								uc[6];
} u_input_frame_es_type;

typedef union u_input_frame_es_two_led {
	struct input_frame_es_two_led_struct	s_data;
	u8								uc[9];
} u_input_frame_es_two_led_type;



struct led_set_brightness {
	unsigned char set_red;
	unsigned char set_green;
	unsigned char set_blue;	
};


struct DVE027_state_t{
	unsigned int	color;	
	int				flashMode;	
	int				flashOnMS;	
	int				flashOffMS;	
	int				brightnessMode;	
	int				flashType;	
	char			*filePath;	
};

typedef struct led_state_struct{
	int					main_state;
	int					sub_state;
	int					req_flag;
	int					req_main_state;
	int					req_sub_state;
}led_state_type;
typedef struct led_data_state_struct{
	led_state_type		ilm3led;
	led_state_type		ilmkey;
	led_state_type		ilmreflect;




}led_data_state_type;


typedef int ( *LED_ILM_FUNC_TBL)(struct DVE027_state_t *, int );

static void led_state_set( int type, int main_state, int sub_state );




static int led_func_error( struct DVE027_state_t *state, int type );
static int led_func_ok( struct DVE027_state_t *state, int type );
static int led_func_stop( struct DVE027_state_t *state, int type );
static int led_func_0_0( struct DVE027_state_t *state, int type );





static int led_func_1_1( struct DVE027_state_t *state, int type );
static int led_func_1_2( struct DVE027_state_t *state, int type );















#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
static int led_func_5_0( struct DVE027_state_t *state, int type );
static int led_func_6_1( struct DVE027_state_t *state, int type );
static int led_func_6_2( struct DVE027_state_t *state, int type );
#endif 
































static int led_func_9_0( struct DVE027_state_t *state, int type );
static int led_func_a_1( struct DVE027_state_t *state, int type );
static int led_func_a_2( struct DVE027_state_t *state, int type );


#if defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)
static u8 *led_patern_data_switch_battery( int type );
static int led_get_frame_data( void *p_set, int type );
static int led_func_1_2_battery( struct DVE027_state_t *state, int type );
static int led_lights_3led_file_ctrl(struct DVE027_state_t *u_state);
static int __init led_data_input_init(void);

static struct miscdevice led_lights_data_input ;

#endif 


static led_patern_data_all_type		gs_patern_data;






















#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077)
static led_input_data_type g_notifications_patern;
static led_input_data_type g_battery_patern;
#endif 

static wait_queue_head_t gs_led_wq;
static led_data_state_type gs_led_state ={
	.ilm3led = {
		.main_state = LED_STATE_STOP,
		.sub_state  = LED_STATE_STOP,





	},
	.ilmkey = {
		.main_state = LED_STATE_STOP,
		.sub_state  = LED_STATE_STOP,





	},
	.ilmreflect = {
		.main_state = LED_STATE_STOP,
		.sub_state  = LED_STATE_STOP,





	},
















};





LED_ILM_FUNC_TBL led_ilm_func_State[13][3]={























	{	led_func_0_0,	led_func_ok,	led_func_ok	},		
	{	led_func_error,	led_func_1_1,	led_func_1_2 },		
	{	led_func_error,	led_func_error,	led_func_error },	
	{	led_func_stop,	led_func_ok,	led_func_stop },	
	{	led_func_ok,	led_func_ok,	led_func_stop },	

#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
	{	led_func_5_0,	led_func_ok,	led_func_stop },		
	{	led_func_error,	led_func_6_1,	led_func_6_2 },		
	{	led_func_ok,	led_func_ok,	led_func_stop },	
	{	led_func_ok,	led_func_ok,	led_func_stop },		
#endif 

	{	led_func_9_0,	led_func_ok,	led_func_stop },		
	{	led_func_error,	led_func_a_1,	led_func_a_2 },		
	{	led_func_ok,	led_func_ok,	led_func_stop },	
	{	led_func_ok,	led_func_ok,	led_func_stop },		


};





















static struct wake_lock led_ilm_wake_lock;
static struct miscdevice led_DVE029_data_input;






static int led_active_flag_ctr( int ctr, int type );

int insert_enable = INSERT_INIT;
struct led_request_rgb stored_illumination_onoff;
struct led_request_rgb stored_illumination_onoff_hard;
union u_led_isc_reg		stored_set_brigh[3];

DEFINE_MUTEX(g_headset_led_state_mutex);

static ssize_t show_insertEnable(struct class *class, struct class_attribute *attr,
	char *buf)
{
	return sprintf(buf, "%u\n",insert_enable);
}

static ssize_t store_insertEnable(struct class *class, struct class_attribute *attr,
	const char *buf, size_t count)
{
	long val;
        char *end;
	u_input_frame_es_type *up_frame_data=NULL;

        val = simple_strtol(buf, &end, 0);

	mutex_lock(&g_headset_led_state_mutex);

	insert_enable = val;

	if( gs_patern_data.reflect.s_set_data.p_patern == NULL)
	{
		memset(&stored_illumination_onoff, 0, sizeof(struct led_request_rgb));
		memset(&stored_illumination_onoff_hard, 0, sizeof(struct led_request_rgb));
		memset( stored_set_brigh,  0, sizeof( stored_set_brigh ) );
		mutex_unlock(&g_headset_led_state_mutex);
		return count;
	}
	up_frame_data = (u_input_frame_es_type *)gs_patern_data.reflect.s_set_data.p_patern;

	if( insert_enable == INSERT_TRUE )
	{

		if( led_active_flag_ctr( LED_FLAG_GET, LED_TYPE_REFLECT ) == LED_FLAG_ON )
		{
			struct led_request_rgb local;
			memset( &local, 0, sizeof(struct led_request_rgb) );
			adp8861_er_rgb_led_set( &local );
		}else{
			if( up_frame_data[0].s_data.fade.hw_act == LED_FLAG_ON )
			{
				struct led_request_rgb local_hard;
				union u_led_isc_reg		local_set_brigh[3];
				memset( &local_hard, 0, sizeof(struct led_request_rgb) );
				memset( local_set_brigh,  0, sizeof( local_set_brigh ) );
				adp8861_er_rgb_led_bright( &local_set_brigh[0], &local_set_brigh[1], &local_set_brigh[2] );
				adp8861_er_rgb_led_set( &local_hard );
			}
		}

	}else if( insert_enable == INSERT_FALSE ){


		if( led_active_flag_ctr( LED_FLAG_GET, LED_TYPE_REFLECT ) == LED_FLAG_ON ) {
			adp8861_er_rgb_led_set( &stored_illumination_onoff );
		}else{
			if( up_frame_data[0].s_data.fade.hw_act == LED_FLAG_ON ) {
				adp8861_er_rgb_led_bright( &stored_set_brigh[0], &stored_set_brigh[1], &stored_set_brigh[2] );
				adp8861_er_rgb_led_set( &stored_illumination_onoff_hard );
			}
		}

		memset(&stored_illumination_onoff, 0, sizeof(struct led_request_rgb));

	}

	mutex_unlock(&g_headset_led_state_mutex);

	return count;
}

static struct class_attribute earphone_insert_attr[] = {
	__ATTR(insert_enable, S_IRUGO | S_IWUSR, show_insertEnable, store_insertEnable),
	__ATTR_NULL
 };

static struct class earphone_insert_class =
	{ .name = "earphone_insert", .owner = THIS_MODULE, .class_attrs =
		(struct class_attribute *) &earphone_insert_attr, };




static void get_led_set_brightness(unsigned char *pattern_brightness, int index, struct adp8861_3color_led_parame_nv *three_color_led_parame_nv, struct led_set_brightness *brightness)
{
	unsigned char brightness_red, brightness_green, brightness_blue;
	int set_cunt = 0;

	brightness_red = pattern_brightness[0] & 0xFF;
	brightness_green = pattern_brightness[1] & 0xFF;
	brightness_blue = pattern_brightness[2] & 0xFF;

	if( brightness_red > 0 ){
		set_cunt++; 
	}
	if( brightness_green > 0 ){
		set_cunt++; 
	}
	if( brightness_blue > 0 ){
		set_cunt++; 
	}
	
	if( set_cunt == 3 ) {
		if( index == 0 ){
			brightness->set_red = brightness_red*three_color_led_parame_nv->illu_red_lumin.three/255;
			brightness->set_green = brightness_green*three_color_led_parame_nv->illu_green_lumin.three/255;
			brightness->set_blue = brightness_blue*three_color_led_parame_nv->illu_blue_lumin.three/255;
		}else if( index == 1 ){
			brightness->set_red = brightness_red*three_color_led_parame_nv->keybl1_red_lumin.three/255;
			brightness->set_green = brightness_green*three_color_led_parame_nv->keybl1_green_lumin.three/255;
			brightness->set_blue = brightness_blue*three_color_led_parame_nv->keybl1_blue_lumin.three/255;
		}else if( index == 2 ){
			brightness->set_red = brightness_red*three_color_led_parame_nv->keybl2_red_lumin.three/255;
			brightness->set_green = brightness_green*three_color_led_parame_nv->keybl2_green_lumin.three/255;
			brightness->set_blue = brightness_blue*three_color_led_parame_nv->keybl2_blue_lumin.three/255;
		}else if( index == 3 ){
			brightness->set_red = brightness_red*three_color_led_parame_nv->hsj_red_lumin.three/255;
			brightness->set_green = brightness_green*three_color_led_parame_nv->hsj_green_lumin.three/255;
			brightness->set_blue = brightness_blue*three_color_led_parame_nv->hsj_blue_lumin.three/255;
		}

	}else if ( set_cunt == 2 ) {
		if( index == 0 ){
			brightness->set_red = brightness_red*three_color_led_parame_nv->illu_red_lumin.two/255;
			brightness->set_green = brightness_green*three_color_led_parame_nv->illu_green_lumin.two/255;
			brightness->set_blue = brightness_blue*three_color_led_parame_nv->illu_blue_lumin.two/255;
		}else if( index == 1 ){
			brightness->set_red = brightness_red*three_color_led_parame_nv->keybl1_red_lumin.two/255;
			brightness->set_green = brightness_green*three_color_led_parame_nv->keybl1_green_lumin.two/255;
			brightness->set_blue = brightness_blue*three_color_led_parame_nv->keybl1_blue_lumin.two/255;
		}else if( index == 2 ){
			brightness->set_red = brightness_red*three_color_led_parame_nv->keybl2_red_lumin.two/255;
			brightness->set_green = brightness_green*three_color_led_parame_nv->keybl2_green_lumin.two/255;
			brightness->set_blue = brightness_blue*three_color_led_parame_nv->keybl2_blue_lumin.two/255;
		}else if( index == 3 ){
			brightness->set_red = brightness_red*three_color_led_parame_nv->hsj_red_lumin.two/255;
			brightness->set_green = brightness_green*three_color_led_parame_nv->hsj_green_lumin.two/255;
			brightness->set_blue = brightness_blue*three_color_led_parame_nv->hsj_blue_lumin.two/255;
		}
	}else if ( set_cunt == 1 ) {
		if( index == 0 ){
			brightness->set_red = brightness_red*three_color_led_parame_nv->illu_red_lumin.one/255;
			brightness->set_green = brightness_green*three_color_led_parame_nv->illu_green_lumin.one/255;
			brightness->set_blue = brightness_blue*three_color_led_parame_nv->illu_blue_lumin.one/255;
		}else if( index == 1 ){
			brightness->set_red = brightness_red*three_color_led_parame_nv->keybl1_red_lumin.one/255;
			brightness->set_green = brightness_green*three_color_led_parame_nv->keybl1_green_lumin.one/255;
			brightness->set_blue = brightness_blue*three_color_led_parame_nv->keybl1_blue_lumin.one/255;
		}else if( index == 2 ){
			brightness->set_red = brightness_red*three_color_led_parame_nv->keybl2_red_lumin.one/255;
			brightness->set_green = brightness_green*three_color_led_parame_nv->keybl2_green_lumin.one/255;
			brightness->set_blue = brightness_blue*three_color_led_parame_nv->keybl2_blue_lumin.one/255;
		}else if( index == 3 ){
			brightness->set_red = brightness_red*three_color_led_parame_nv->hsj_red_lumin.one/255;
			brightness->set_green = brightness_green*three_color_led_parame_nv->hsj_green_lumin.one/255;
			brightness->set_blue = brightness_blue*three_color_led_parame_nv->hsj_blue_lumin.one/255;
		}
	}else if ( set_cunt == 0 ) {
			brightness->set_red = 0;
			brightness->set_green = 0;
			brightness->set_blue = 0;
	}
	return;
	
}









static void led_lock_wrapper( int lock, int locok_type )
{














































	return;
}










static u8 *led_patern_data_switch( int type )
{
	unsigned char *p_ret = NULL;
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_patern_data_switch type = %d\n", type);
#endif 

    switch(type){
	
    case LED_TYPE_INCOMING:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		if( gs_patern_data.incoming.s_ready_data.p_patern != NULL ){
			kfree( gs_patern_data.incoming.s_set_data.p_patern );
			gs_patern_data.incoming.s_set_data.p_patern = gs_patern_data.incoming.s_ready_data.p_patern;
			gs_patern_data.incoming.s_ready_data.p_patern = NULL;
			
			memcpy( &gs_patern_data.incoming.s_set_data.s_data_info, &gs_patern_data.incoming.s_ready_data.s_data_info, sizeof(input_data_head_type) );
		}
		
		p_ret = gs_patern_data.incoming.s_set_data.p_patern;
		
		gs_patern_data.incoming.frame_cnt = 0;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
    	
		break;
		
#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
    case LED_TYPE_FRONT:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		
		if( gs_patern_data.front.s_ready_data.p_patern != NULL ){
			kfree( gs_patern_data.front.s_set_data.p_patern );
			gs_patern_data.front.s_set_data.p_patern = gs_patern_data.front.s_ready_data.p_patern;
			gs_patern_data.front.s_ready_data.p_patern = NULL;
			
			memcpy( &gs_patern_data.front.s_set_data.s_data_info, &gs_patern_data.front.s_ready_data.s_data_info, sizeof(input_data_head_type) );
		}
		
		p_ret = gs_patern_data.front.s_set_data.p_patern;
		
		gs_patern_data.front.frame_cnt = 0;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
    	
		break;
#endif 







































    case LED_TYPE_REFLECT:

		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_REFLECT );

		if( gs_patern_data.reflect.s_ready_data.p_patern != NULL ){
			kfree( gs_patern_data.reflect.s_set_data.p_patern );
			gs_patern_data.reflect.s_set_data.p_patern = gs_patern_data.reflect.s_ready_data.p_patern;
			gs_patern_data.reflect.s_ready_data.p_patern = NULL;

			memcpy( &gs_patern_data.reflect.s_set_data.s_data_info, &gs_patern_data.reflect.s_ready_data.s_data_info, sizeof(input_data_head_type) );
		}

		p_ret = gs_patern_data.reflect.s_set_data.p_patern;

		gs_patern_data.reflect.frame_cnt = 0;

		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_REFLECT );

		break;


	default:
		printk(KERN_INFO "[leds_data_input]led_patern_data_switch NG\n");
		break;
	}
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_patern_data_switch p_ret = %p\n", p_ret);
#endif 

	return p_ret;
}

#if defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)
static u8 *led_patern_data_switch_battery( int type )
{
	unsigned char *p_ret = NULL;
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_patern_data_switch_battery type = %d\n", type);
#endif 

    switch(type)
    {
	
    case LED_TYPE_INCOMING:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		p_ret = gs_patern_data.incoming.s_set_data.p_patern;
		
		gs_patern_data.incoming.frame_cnt = 0;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
    	
		break;
		
    case LED_TYPE_FRONT:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		
		p_ret = gs_patern_data.front.s_set_data.p_patern;
		
		gs_patern_data.front.frame_cnt = 0;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
    	
		break;
	
	default:
		printk(KERN_INFO "[leds_data_input]led_patern_data_switch_battery NG\n");
		break;
	}
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_patern_data_switch_battery p_ret = %p\n", p_ret);
#endif 

	return p_ret;
}









static int led_get_frame_data( void *p_set, int type )
{
	int ret = LED_DATA_INPUT_NG;
	unsigned char *p_shift;
	unsigned int shift_size;
	unsigned int frame_size;
	u_input_frame_es_type *u_frame_data;
	
	if( p_set == NULL )
	{
		printk(KERN_INFO "[leds_data_input]led_get_frame_data input NG\n");
		return ret;
	}
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_get_frame_data type = %d\n", type);
#endif 
	
    switch(type)
    {
	
    case LED_TYPE_INCOMING:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		if( gs_patern_data.incoming.s_set_data.p_patern != NULL )
		{
			frame_size = gs_patern_data.incoming.s_set_data.s_data_info.frame_size;
			shift_size = gs_patern_data.incoming.frame_cnt * frame_size;
			
			if( gs_patern_data.incoming.s_set_data.s_data_info.frame_num  > gs_patern_data.incoming.frame_cnt )
			{
				p_shift = gs_patern_data.incoming.s_set_data.p_patern + shift_size  ;
				
				memcpy( p_set, p_shift, frame_size );
				gs_patern_data.incoming.frame_cnt++;
				
				if( gs_patern_data.battery.data != 0x00000000 )
				{
					u_frame_data = (u_input_frame_es_type *)p_set;
					
					if(( u_frame_data->s_data.brightness[0] == 0 )
					&& ( u_frame_data->s_data.brightness[1] == 0 )
					&& ( u_frame_data->s_data.brightness[2] == 0 ))
					{
						u_frame_data->s_data.brightness[0] = gs_patern_data.battery.uc[2];
						u_frame_data->s_data.brightness[1] = gs_patern_data.battery.uc[1];
						u_frame_data->s_data.brightness[2] = gs_patern_data.battery.uc[0];
					}
					else
					{
						; 
					}
#if defined(LED_DIAG_DEBUG_PLUS)
					printk(KERN_INFO "[leds_data_input]led_get_frame_data s_patern_data.battery.uc = %02x %02x %02x SET \n", 
											gs_patern_data.battery.uc[2],
											gs_patern_data.battery.uc[1],
											gs_patern_data.battery.uc[0]);
#endif 
				}
				else
				{
					; 
				}
				
#if defined(LED_DIAG_DEBUG_PLUS)
				printk(KERN_INFO "[leds_data_input]led_get_frame_data frame_cnt = %d\n", gs_patern_data.incoming.frame_cnt);
#endif 
				
				if( gs_patern_data.incoming.s_set_data.s_data_info.frame_num == gs_patern_data.incoming.frame_cnt )
				{
					if( gs_patern_data.incoming.loop_flag == LED_FLAG_ON )
					{
						gs_patern_data.incoming.frame_cnt = 0;
					}
					else
					{
						gs_patern_data.incoming.loop_cnt--;
						gs_patern_data.incoming.frame_cnt = 0;
					}
				}
				else
				{
					; 
				}
				
				if( (gs_patern_data.incoming.loop_flag != LED_FLAG_ON) && (gs_patern_data.incoming.loop_cnt == 0) )
				{
					ret = LED_DATA_INPUT_FLIE_END;
				}
				else
				{
					ret = LED_DATA_INPUT_OK;
				}
			}
			else
			{
				; 
			}
		}
		else
		{
			ret = LED_DATA_INPUT_FLIE_NOT;
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
    	
		break;
		
#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
    case LED_TYPE_FRONT:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		
		if( gs_patern_data.front.s_set_data.p_patern != NULL )
		{
			frame_size = gs_patern_data.front.s_set_data.s_data_info.frame_size;
			shift_size = gs_patern_data.front.frame_cnt * frame_size;
			
			if( gs_patern_data.front.s_set_data.s_data_info.frame_num  > gs_patern_data.front.frame_cnt )
			{
				p_shift = gs_patern_data.front.s_set_data.p_patern + shift_size ;
				memcpy( p_set, p_shift, frame_size );
				gs_patern_data.front.frame_cnt++;
				
#if defined(LED_DIAG_DEBUG_PLUS)
				printk(KERN_INFO "[leds_data_input]led_get_frame_data frame_cnt = %d\n", gs_patern_data.front.frame_cnt);
#endif 
				
				if( gs_patern_data.front.s_set_data.s_data_info.frame_num == gs_patern_data.front.frame_cnt )
				{
					if( gs_patern_data.front.loop_flag == LED_FLAG_ON )
					{
						gs_patern_data.front.frame_cnt = 0;
					}
					else
					{
						gs_patern_data.front.loop_cnt--;
						gs_patern_data.front.frame_cnt = 0;
					}
				}
				else
				{
					; 
				}
				
				if( (gs_patern_data.front.loop_flag != LED_FLAG_ON) && (gs_patern_data.front.loop_cnt == 0) )
				{
					ret = LED_DATA_INPUT_FLIE_END;
				}
				else
				{
					ret = LED_DATA_INPUT_OK;
				}
			}
			else
			{
				; 
			}
		}
		else
		{
			ret = LED_DATA_INPUT_FLIE_NOT;
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
    	
		break;
#endif 





























































    case LED_TYPE_REFLECT:

		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_REFLECT );

		if( gs_patern_data.reflect.s_set_data.p_patern != NULL )
		{
			frame_size = gs_patern_data.reflect.s_set_data.s_data_info.frame_size;
			shift_size = gs_patern_data.reflect.frame_cnt * frame_size;

			if( gs_patern_data.reflect.s_set_data.s_data_info.frame_num  > gs_patern_data.reflect.frame_cnt )
			{
				p_shift = gs_patern_data.reflect.s_set_data.p_patern + shift_size ;
				memcpy( p_set, p_shift, frame_size );
				gs_patern_data.reflect.frame_cnt++;

#if defined(LED_DIAG_DEBUG_PLUS)
				printk(KERN_INFO "[leds_data_input]led_get_frame_data frame_cnt = %d\n", gs_patern_data.reflect.frame_cnt);
#endif 

				if( gs_patern_data.reflect.s_set_data.s_data_info.frame_num == gs_patern_data.reflect.frame_cnt )
				{
					if( gs_patern_data.reflect.loop_flag == LED_FLAG_ON )
					{
						gs_patern_data.reflect.frame_cnt = 0;
					}
					else
					{
						gs_patern_data.reflect.loop_cnt--;
						gs_patern_data.reflect.frame_cnt = 0;
					}
				}
				else
				{
					; 
				}

				if( (gs_patern_data.reflect.loop_flag != LED_FLAG_ON) && (gs_patern_data.reflect.loop_cnt == 0) )
				{
					ret = LED_DATA_INPUT_FLIE_END;
				}
				else
				{
					ret = LED_DATA_INPUT_OK;
				}
			}
			else
			{
				; 
			}
		}
		else
		{
			ret = LED_DATA_INPUT_FLIE_NOT;
		}

		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_REFLECT );

		break;

	default:
		printk(KERN_INFO "[leds_data_input]led_get_frame_data NG\n");
		break;
	}
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_get_frame_data ret = %d\n", ret);
#endif 
	
	return ret;
}

#endif 
























































static int led_patern_data_del( int type )
{
	int ret = LED_DATA_INPUT_OK;
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_patern_data_del type = %d\n", type);
#endif 

    switch(type){
	
    case LED_TYPE_INCOMING:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		if( gs_patern_data.incoming.s_set_data.p_patern != NULL ){
			kfree( gs_patern_data.incoming.s_set_data.p_patern );
			gs_patern_data.incoming.s_set_data.p_patern = NULL;
		}
		gs_patern_data.incoming.active_flag = LED_FLAG_OFF;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
    	
		break;
		
#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
    case LED_TYPE_FRONT:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		
		if( gs_patern_data.front.s_set_data.p_patern != NULL ){
			kfree( gs_patern_data.front.s_set_data.p_patern );
			gs_patern_data.front.s_set_data.p_patern = NULL;
		}
		gs_patern_data.front.active_flag = LED_FLAG_OFF;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
    	
		break;
#endif 






























    case LED_TYPE_REFLECT:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_REFLECT );
		
		if( gs_patern_data.reflect.s_set_data.p_patern != NULL ){
			kfree( gs_patern_data.reflect.s_set_data.p_patern );
			gs_patern_data.reflect.s_set_data.p_patern = NULL;
		}
		gs_patern_data.reflect.active_flag = LED_FLAG_OFF;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_REFLECT );
    	
		break;

	
	default:
		ret = LED_DATA_INPUT_NG;
		printk(KERN_INFO "[leds_data_input]led_patern_data_del NG\n");
		break;
	}
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_patern_data_del ret = %d\n", ret);
#endif 
	return ret;
}









static int led_active_flag_ctr( int ctr, int type )
{
	int ret = LED_DATA_INPUT_NG;
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_active_flag_ctr ctr = %d, type = %d\n", ctr, type);
#endif 

    switch(type){
	
    case LED_TYPE_INCOMING:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		if( ctr == LED_FLAG_OFF || ctr == LED_FLAG_ON ){
			gs_patern_data.incoming.active_flag = ctr;
		}
		ret = gs_patern_data.incoming.active_flag;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
    	
		break;
		
#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
    case LED_TYPE_FRONT:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		
		if( ctr == LED_FLAG_OFF || ctr == LED_FLAG_ON ){
			gs_patern_data.front.active_flag = ctr;
		}
		ret = gs_patern_data.front.active_flag;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
    	
		break;
#endif 















    case LED_TYPE_REFLECT:

		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_REFLECT );

		if( ctr == LED_FLAG_OFF || ctr == LED_FLAG_ON ){
			gs_patern_data.reflect.active_flag = ctr;
		}
		ret = gs_patern_data.reflect.active_flag;

		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_REFLECT );

		break;

	
	default:
		printk(KERN_INFO "[leds_data_input]led_active_flag_ctr NG\n");
		break;
	}
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_active_flag_ctr ret = %d \n", ret);
#endif 

	return ret;
}









static int led_loop_set( unsigned int ctr, int type )
{
	int ret = LED_DATA_INPUT_OK;
	int				loop_flag;
	unsigned int	loop_cnt = 0;
	
	if( ctr == 0 ){
		loop_flag = LED_FLAG_ON;
	}
	else{
		loop_flag = LED_FLAG_OFF;
		loop_cnt = ctr;
	}
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_loop_set ctr = %d, type = %d\n", ctr, type);
#endif
	
    switch(type){
	
    case LED_TYPE_INCOMING:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		gs_patern_data.incoming.loop_flag = loop_flag;
		gs_patern_data.incoming.loop_cnt = loop_cnt;
		gs_patern_data.incoming.frame_cnt = 0;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
    	
		break;
		
#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
    case LED_TYPE_FRONT:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		
		gs_patern_data.front.loop_flag = loop_flag;
		gs_patern_data.front.loop_cnt = loop_cnt;
		gs_patern_data.front.frame_cnt = 0;
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
    	
		break;
#endif 


























    case LED_TYPE_REFLECT:

		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_REFLECT );

		gs_patern_data.reflect.loop_flag = loop_flag;
		gs_patern_data.reflect.loop_cnt = loop_cnt;
		gs_patern_data.reflect.frame_cnt = 0;

		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_REFLECT );

		break;

	
	default:
		printk(KERN_INFO "[leds_data_input]led_loop_set NG\n");
		ret = LED_DATA_INPUT_NG;
		break;
	}
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_loop_set ret = %d \n", ret);
#endif 
	return ret;
}









static void led_wake_up_thread( int type )
{
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_wake_up_thread IN type = %d\n", type);
#endif
	
    switch(type){
	
    case LED_TYPE_INCOMING:
    
		gs_patern_data.incoming.check_flag++;
		wake_up_interruptible(&gs_led_wq);
    	
		break;
		
#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
    case LED_TYPE_FRONT:
    	
		gs_patern_data.front.check_flag++;
		wake_up_interruptible(&gs_led_wq);
    	
		break;
#endif 









    case LED_TYPE_REFLECT:

		gs_patern_data.reflect.check_flag++;
		wake_up_interruptible(&gs_led_wq);

		break;

	
	default:
		printk(KERN_DEBUG "[leds_data_input]led_wake_up_thread NG\n");
		break;
	}
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_wake_up_thread OUT\n");
#endif 
	
	return ;
}

#if !defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)








static int led_get_frame_data( void *p_set, int type )
{
	int ret = LED_DATA_INPUT_NG;
	unsigned char *p_shift;
	unsigned int shift_size;
	unsigned int frame_size;
	
	if( p_set == NULL ){
		printk(KERN_INFO "[leds_data_input]led_get_frame_data input NG\n");
		return ret;
	}
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_get_frame_data type = %d\n", type);
#endif 
	
    switch(type){
	
    case LED_TYPE_INCOMING:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		if( gs_patern_data.incoming.s_set_data.p_patern != NULL ){
			frame_size = gs_patern_data.incoming.s_set_data.s_data_info.frame_size;
			shift_size = gs_patern_data.incoming.frame_cnt * frame_size;
			
			if( gs_patern_data.incoming.s_set_data.s_data_info.frame_num  > gs_patern_data.incoming.frame_cnt ){
				p_shift = gs_patern_data.incoming.s_set_data.p_patern + shift_size  ;
				
				memcpy( p_set, p_shift, frame_size );
				gs_patern_data.incoming.frame_cnt++;
				
#if defined(LED_DIAG_DEBUG_PLUS)
				printk(KERN_INFO "[leds_data_input]led_get_frame_data frame_cnt = %d\n", gs_patern_data.incoming.frame_cnt);
#endif 
				
				if( gs_patern_data.incoming.s_set_data.s_data_info.frame_num == gs_patern_data.incoming.frame_cnt ){
					if( gs_patern_data.incoming.loop_flag == LED_FLAG_ON ){
						gs_patern_data.incoming.frame_cnt = 0;
					}
					else{
						gs_patern_data.incoming.loop_cnt--;
						gs_patern_data.incoming.frame_cnt = 0;
					}
				}
				
				if( (gs_patern_data.incoming.loop_flag != LED_FLAG_ON) && (gs_patern_data.incoming.loop_cnt == 0) ){
					ret = LED_DATA_INPUT_FLIE_END;
				}
				else {
					ret = LED_DATA_INPUT_OK;
				}
			}
		}
		else{
			ret = LED_DATA_INPUT_FLIE_NOT;
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
    	
		break;
		
#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
    case LED_TYPE_FRONT:
    	
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		
		if( gs_patern_data.front.s_set_data.p_patern != NULL ){
			frame_size = gs_patern_data.front.s_set_data.s_data_info.frame_size;
			shift_size = gs_patern_data.front.frame_cnt * frame_size;
			
			if( gs_patern_data.front.s_set_data.s_data_info.frame_num  > gs_patern_data.front.frame_cnt ){
				p_shift = gs_patern_data.front.s_set_data.p_patern + shift_size ;
				memcpy( p_set, p_shift, frame_size );
				gs_patern_data.front.frame_cnt++;
				
#if defined(LED_DIAG_DEBUG_PLUS)
				printk(KERN_INFO "[leds_data_input]led_get_frame_data frame_cnt = %d\n", gs_patern_data.front.frame_cnt);
#endif 
				
				if( gs_patern_data.front.s_set_data.s_data_info.frame_num == gs_patern_data.front.frame_cnt ){
					if( gs_patern_data.front.loop_flag == LED_FLAG_ON ){
						gs_patern_data.front.frame_cnt = 0;
					}
					else{
						gs_patern_data.front.loop_cnt--;
						gs_patern_data.front.frame_cnt = 0;
					}
				}
				
				if( (gs_patern_data.front.loop_flag != LED_FLAG_ON) && (gs_patern_data.front.loop_cnt == 0) ){
					ret = LED_DATA_INPUT_FLIE_END;
				}
				else {
					ret = LED_DATA_INPUT_OK;
				}
			}
		}
		else{
			ret = LED_DATA_INPUT_FLIE_NOT;
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
    	
		break;
#endif 












































	
	default:
		printk(KERN_INFO "[leds_data_input]led_get_frame_data NG\n");
		break;
	}
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_get_frame_data ret = %d\n", ret);
#endif 
	
	return ret;
}
#endif 

#if defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)








static int led_adp8861_3led_ctrl_reg_time( union u_led_isct1_reg  *led_timer1_data,
										   union u_led_isct2_reg  *led_timer2_data)
{
	int						ret, ret_code = ADP8861_LED_SET_OK;
	union u_led_isct1_reg	led_timer1_set;
	union u_led_isct2_reg	led_timer2_set;
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_reg_time IN\n");
#endif 

	memset( &led_timer2_set,  0, sizeof( led_timer2_set ) );
	led_timer2_set.st2.sc1_off = ADP8861_LED_ON;
	led_timer2_set.st2.sc2_off = ADP8861_LED_ON;
	led_timer2_set.st2.sc3_off = ADP8861_LED_ON;

	ret_code = ret = adp8861_led_timer_set2( &led_timer2_set, led_timer2_data, ID_0 );



	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_3led_ctrl_reg_time: adp8861_led_timer_set2 = %d \n",ret);
	}
	memset( &led_timer1_set,  0, sizeof( led_timer1_set ) );
	led_timer1_set.st2.scon = ADP8861_LED_ON;

	ret_code |= ret = adp8861_led_timer_set1( &led_timer1_set, led_timer1_data, ID_0 );



	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_3led_ctrl_reg_time: adp8861_led_timer_set1 = %d \n",ret);
	}
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_reg_time OUT\n");
#endif 
	return ret_code;
}









static int led_adp8861_3led_ctrl_reg( unsigned char timer_set_flg,
									   union u_led_isct1_reg  *led_timer1_data,
									   union u_led_isct2_reg  *led_timer2_data,
									   u_input_frame_es_type *u_frame_data )
{
	int						ret, ret_code = ADP8861_LED_SET_OK;
	union u_led_isc_reg		set_brigh[3];
	struct led_request_rgb	set_onoff;

	struct adp8861_3color_led_parame_nv  adp8861_3color_led_parame_nv;
	struct led_set_brightness led_brightness;


#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_reg IN\n");
#endif 

	memset( set_brigh,  0, sizeof( set_brigh ) );
	memset( &set_onoff, 0, sizeof( set_onoff ) );

	memset( &adp8861_3color_led_parame_nv, 0, sizeof(  struct adp8861_3color_led_parame_nv ));

	adp8861_get_3color_nv(&adp8861_3color_led_parame_nv);
	get_led_set_brightness(&u_frame_data->s_data.brightness[0], 0, &adp8861_3color_led_parame_nv, &led_brightness);

	

	set_brigh[0].st2.set_flag = ADP8861_LED_ON;
	set_brigh[0].st2.scd	  = led_brightness.set_red;
	if( led_brightness.set_red != 0 ){
		set_onoff.set_r = ADP8861_LED_ON;
	}
	set_brigh[1].st2.set_flag = ADP8861_LED_ON;
	set_brigh[1].st2.scd	  = led_brightness.set_green;
	if( led_brightness.set_green != 0 ){
	set_onoff.set_g =  ADP8861_LED_ON;
	}
	set_brigh[2].st2.set_flag = ADP8861_LED_ON;
	set_brigh[2].st2.scd	  = led_brightness.set_blue;
	if( led_brightness.set_blue != 0 ){
	set_onoff.set_b =  ADP8861_LED_ON;
	}

















	ret_code |= ret = adp8861_rgb_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_3led_ctrl_reg: adp8861_rgb_led_bright = %d \n",ret);
	}
	if( timer_set_flg == ADP8861_LED_ON ){
		ret = led_adp8861_3led_ctrl_reg_time( led_timer1_data, led_timer2_data);
	}
	ret_code |= ret = adp8861_rgb_led_set( &set_onoff );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_3led_ctr_regl: adp8861_rgb_led_set(%d,%d,%d) = %d \n",set_onoff.set_r,set_onoff.set_g,set_onoff.set_b,ret);
	}

#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_reg OUT\n");
#endif 

	return ret_code;
}









int led_adp8861_3led_ctrl_chk( void )
{
	int						ret = 0;
	union u_led_isct1_reg  led_timer1_data;
	union u_led_isct2_reg  led_timer2_data;
	u_input_frame_es_type *up_frame_data;
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_chk IN\n");
#endif 

	memset( &led_timer1_data,  0, sizeof( led_timer1_data ) );
	memset( &led_timer2_data,  0, sizeof( led_timer2_data ) );




		
		up_frame_data = (u_input_frame_es_type *)gs_patern_data.incoming.s_set_data.p_patern ;
		
		if( gs_patern_data.incoming.s_set_data.s_data_info.frame_num == 2 ){
			

			if (( up_frame_data[0].s_data.fade.hw_act == LED_FLAG_ON )
			 && ( up_frame_data[1].s_data.fade.hw_act == LED_FLAG_ON )){
				
				if (( up_frame_data[0].s_data.fade.hw_on_off == LED_FLAG_ON  )
				 && ( up_frame_data[1].s_data.fade.hw_on_off == LED_FLAG_OFF )){
					
					led_timer1_data.st2.scon    = up_frame_data[0].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc1_off = up_frame_data[1].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc2_off = up_frame_data[1].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc3_off = up_frame_data[1].s_data.fade.hw_set_data;
					
					led_adp8861_3led_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[0] );
					
					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_INCOMING );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_chk HRD1 \n");
					ret = 1;
				}
				else if (( up_frame_data[0].s_data.fade.hw_on_off == LED_FLAG_OFF )
					  && ( up_frame_data[1].s_data.fade.hw_on_off == LED_FLAG_ON  )){
					
					led_timer1_data.st2.scon    = up_frame_data[1].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc1_off = up_frame_data[0].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc2_off = up_frame_data[0].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc3_off = up_frame_data[0].s_data.fade.hw_set_data;
					
					led_adp8861_3led_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[1] );
					
					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_INCOMING );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_chk HRD2 \n");
					ret = 1;
				}
			}
			else 

			if(UP_FRAME_DATA_ON(0)){
				if( UP_FRAME_DATA_OFF(1) ){
					if( up_frame_data[0].s_data.u_time.time == 0x12 ){
						led_timer2_data.st2.sc1_off = 0;
						led_timer2_data.st2.sc2_off = 0;
						led_timer2_data.st2.sc3_off = 0;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x16 ){
						led_timer2_data.st2.sc1_off = 1;
						led_timer2_data.st2.sc2_off = 1;
						led_timer2_data.st2.sc3_off = 1;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x18 ){
						led_timer2_data.st2.sc1_off = 2;
						led_timer2_data.st2.sc2_off = 2;
						led_timer2_data.st2.sc3_off = 2;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x1C ){
						led_timer2_data.st2.sc1_off = 3;
						led_timer2_data.st2.sc2_off = 3;
						led_timer2_data.st2.sc3_off = 3;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x16 ){
						led_timer1_data.st2.scon = 1;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x1C ){
						led_timer1_data.st2.scon = 2;
					}
					led_adp8861_3led_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[0] );
					
					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_INCOMING );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_chk HRD1 \n");
					ret = 1;
				}
			}
			else if(UP_FRAME_DATA_ON(1)){
				if( UP_FRAME_DATA_OFF(0) ){
					if( up_frame_data[1].s_data.u_time.time == 0x12 ){
						led_timer2_data.st2.sc1_off = 0;
						led_timer2_data.st2.sc2_off = 0;
						led_timer2_data.st2.sc3_off = 0;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x16 ){
						led_timer2_data.st2.sc1_off = 1;
						led_timer2_data.st2.sc2_off = 1;
						led_timer2_data.st2.sc3_off = 1;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x18 ){
						led_timer2_data.st2.sc1_off = 2;
						led_timer2_data.st2.sc2_off = 2;
						led_timer2_data.st2.sc3_off = 2;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x1C ){
						led_timer2_data.st2.sc1_off = 3;
						led_timer2_data.st2.sc2_off = 3;
						led_timer2_data.st2.sc3_off = 3;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x16 ){
						led_timer1_data.st2.scon = 1;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x1C ){
						led_timer1_data.st2.scon = 2;
					}
					led_adp8861_3led_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[1] );
					
					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_INCOMING );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_chk HRD2 \n");
					ret = 1;
				}
			}
		}



	led_adp8861_3led_ctrl_reg_time( &led_timer1_data, &led_timer2_data );
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_chk OUT\n");
#endif 

	return ret;
}










static void led_adp8861_3led_ctrl( void )
{
	u_input_frame_es_type u_frame_data;
	int get_frame, time_a = 0, time_b = 0, time = 0;
	int wait_ret, ret, count;
	int active_flag;
	union u_led_iscf_reg	led_fade;
	struct DVE027_state_t state;
	union u_led_isct1_reg  led_timer1_data;
	union u_led_isct2_reg  led_timer2_data;
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl IN\n");
#endif
	
	memset( &led_timer1_data,  0, sizeof( led_timer1_data ) );
	memset( &led_timer2_data,  0, sizeof( led_timer2_data ) );
	
	while(1){
		
		mutex_lock( &gs_patern_data.func_lock );
		active_flag = led_active_flag_ctr( LED_FLAG_GET, LED_TYPE_INCOMING );
		
		if( active_flag == LED_FLAG_OFF ){
			memset( &state, 0, sizeof( state ) );
			ret = led_ilm_func_State[4][gs_led_state.ilm3led.main_state](&state, LED_TYPE_INCOMING);
			if( ret != LED_DATA_INPUT_OK ){
				printk(KERN_INFO "led_adp8861_3led_ctrl: led_ilm_func_State(4,%d) = %d \n",gs_led_state.ilm3led.main_state, ret);
			}
			gs_patern_data.incoming.ps_thread = NULL;
			mutex_unlock( &gs_patern_data.func_lock );
			break;
		}
		
		get_frame = led_get_frame_data( u_frame_data.uc, LED_TYPE_INCOMING );
		
		if( get_frame == LED_DATA_INPUT_OK || get_frame == LED_DATA_INPUT_FLIE_END ){
			
			if( gs_patern_data.incoming.frame_cnt == 0 ){
				if( led_adp8861_3led_ctrl_chk() != 0 ){
					gs_patern_data.incoming.ps_thread = NULL;
					mutex_unlock( &gs_patern_data.func_lock );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl hw end\n");
					break;
				}
				else{
					led_adp8861_3led_ctrl_reg_time( &led_timer1_data, &led_timer2_data);
				}
			}
			if(( u_frame_data.s_data.fade.fade & 0x01 ) == 0x01 ){
				
				
				

				led_fade.uc = u_frame_data.s_data.fade_time.time ;




				ret = adp8861_led_fade_set( &led_fade, ID_0 );
				led_adp8861_3led_ctrl_reg( ADP8861_LED_OFF, &led_timer1_data, &led_timer2_data, &u_frame_data );
				if( 0 != (int)u_frame_data.s_data.u_time.time ){
					
					count = (int)u_frame_data.s_data.u_time.s_tmie.time_a;
					time_a = 1;
					while( count > 0){
						time_a *= 10;
						count-- ;
					}
					time_b = (int)u_frame_data.s_data.u_time.s_tmie.time_b;
					time = time_a * time_b * 10;
				}





			}
			else{
				if( 0 != (int)u_frame_data.s_data.u_time.time ){
					
					count = (int)u_frame_data.s_data.u_time.s_tmie.time_a;
					time_a = 1;
					while( count > 0){
						time_a *= 10;
						count-- ;
					}
					time_b = (int)u_frame_data.s_data.u_time.s_tmie.time_b;
					time = time_a * time_b * 10;
				}

				led_fade.uc = 0;
				adp8861_led_fade_set( &led_fade, ID_0 );
				
#if defined(LED_DIAG_DEBUG_PLUS)
				printk(KERN_INFO "[leds_data_input]wait tmie : %d mse \n",time);
#endif 
				
				
				
				
				
				
				
				led_adp8861_3led_ctrl_reg( ADP8861_LED_OFF, &led_timer1_data, &led_timer2_data, &u_frame_data );
			}
			
			if( get_frame == LED_DATA_INPUT_FLIE_END ){
				led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_INCOMING );
			}
			
			gs_patern_data.incoming.check_flag = 0;
			mutex_unlock( &gs_patern_data.func_lock );
			
			wait_ret = 0;
			if(time != 0 ){
				wait_ret = wait_event_interruptible_timeout( gs_led_wq, ( gs_patern_data.incoming.check_flag != 0 ), msecs_to_jiffies(time) );
			}
			if( wait_ret != 0 )
				printk(KERN_INFO "[leds_data_input]wait_event_interruptible_timeout : %d \n",wait_ret);
		}
		else
		{
			led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_INCOMING );
			mutex_unlock( &gs_patern_data.func_lock );
		}
	}
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_DEBUG "[leds_data_input]led_adp8861_3led_ctrl OUT\n");
#endif
	
	return ;
}
#endif 










static int led_adp8861_keyilm_ctrl_reg_time( union u_led_isct1_reg  *led_timer1_data,
										   union u_led_isct2_reg  *led_timer2_data)
{
	int						ret, ret_code = ADP8861_LED_SET_OK;
	union u_led_isct1_reg	led_timer1_set;
	union u_led_isct2_reg	led_timer2_set;
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl_reg_time IN\n");
#endif 

	memset( &led_timer2_set,  0, sizeof( led_timer2_set ) );

	led_timer2_set.st2.sc1_off = ADP8861_LED_ON;
	led_timer2_set.st2.sc2_off = ADP8861_LED_ON;
	led_timer2_set.st2.sc3_off = ADP8861_LED_ON;

	led_timer2_set.st2.sc4_off = ADP8861_LED_ON;

	ret_code = ret = adp8861_led_timer_set2( &led_timer2_set, led_timer2_data, ID_1 );



	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_keyilm_ctrl_reg_time: adp8861_led_timer_set2 = %d \n",ret);
	}
	memset( &led_timer1_set,  0, sizeof( led_timer1_set ) );
	led_timer1_set.st2.sc5_off = ADP8861_LED_ON;
	led_timer1_set.st2.sc6_off = ADP8861_LED_ON;
	led_timer1_set.st2.scon = ADP8861_LED_ON;

	ret_code |= ret = adp8861_led_timer_set1( &led_timer1_set, led_timer1_data, ID_1 );



	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_keyilm_ctrl_reg_time: adp8861_led_timer_set1 = %d \n",ret);
	}
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl_reg_time OUT\n");
#endif 
	return ret_code;
}










static int led_adp8861_keyilm_ctrl_reg( unsigned char timer_set_flg,
									   union u_led_isct1_reg  *led_timer1_data,
									   union u_led_isct2_reg  *led_timer2_data,
									   u_input_frame_es_two_led_type *u_frame_data )






{
	int						ret, ret_code = ADP8861_LED_SET_OK;
	union u_led_isc_reg		set_brigh[3];
	struct led_request_rgb	set_onoff;

	struct adp8861_3color_led_parame_nv  adp8861_3color_led_parame_nv;
	struct led_set_brightness led_brightness;


#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl_reg IN\n");
#endif 

	memset( set_brigh,  0, sizeof( set_brigh ) );
	memset( &set_onoff, 0, sizeof( set_onoff ) );

	memset( &adp8861_3color_led_parame_nv, 0, sizeof(  struct adp8861_3color_led_parame_nv ));

	adp8861_get_3color_nv(&adp8861_3color_led_parame_nv);
	get_led_set_brightness(&u_frame_data->s_data.brightness[0], 1, &adp8861_3color_led_parame_nv, &led_brightness);



	set_brigh[0].st2.set_flag = ADP8861_LED_ON;
	set_brigh[0].st2.scd	  = led_brightness.set_red;
	if( led_brightness.set_red != 0 ){
		set_onoff.set_r = ADP8861_LED_ON;
	}
	set_brigh[1].st2.set_flag = ADP8861_LED_ON;
	set_brigh[1].st2.scd	  = led_brightness.set_green;
	if( led_brightness.set_green != 0 ){
	set_onoff.set_g =  ADP8861_LED_ON;
	}
	set_brigh[2].st2.set_flag = ADP8861_LED_ON;
	set_brigh[2].st2.scd	  = led_brightness.set_blue;
	if( led_brightness.set_blue != 0 ){
	set_onoff.set_b =  ADP8861_LED_ON;
	}


















	ret_code |= ret = adp8861_right_key_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );

	memset( set_brigh,  0, sizeof( set_brigh ) );
	memset( &led_brightness,  0, sizeof( led_brightness ) );
	get_led_set_brightness(&u_frame_data->s_data.brightness[3], 2, &adp8861_3color_led_parame_nv, &led_brightness);

	set_brigh[0].st2.set_flag = ADP8861_LED_ON;
	set_brigh[0].st2.scd	  = led_brightness.set_red;

	set_brigh[1].st2.set_flag = ADP8861_LED_ON;
	set_brigh[1].st2.scd	  = led_brightness.set_green;

	set_brigh[2].st2.set_flag = ADP8861_LED_ON;
	set_brigh[2].st2.scd	  = led_brightness.set_blue;

	ret_code |= ret = adp8861_left_key_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );



	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_3led_ctrl: adp8861_key_led_bright = %d \n",ret);
	}
	if( timer_set_flg == ADP8861_LED_ON ){
		ret = led_adp8861_keyilm_ctrl_reg_time( led_timer1_data, led_timer2_data);
	}

	ret_code |= ret = adp8861_right_key_led_set( &set_onoff );

	memset( &set_onoff, 0, sizeof( set_onoff ) );

	if( led_brightness.set_red != 0 ){
		set_onoff.set_r = ADP8861_LED_ON;
	}

	if( led_brightness.set_green != 0 ){
	set_onoff.set_g =  ADP8861_LED_ON;
	}

	if( led_brightness.set_blue != 0 ){
	set_onoff.set_b =  ADP8861_LED_ON;
	}
	ret_code |= ret = adp8861_left_key_led_set( &set_onoff );



	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_3led_ctrl: adp8861_key_led_set(%d,%d,%d) = %d \n",set_onoff.set_r,set_onoff.set_g,set_onoff.set_b,ret);
	}

#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl_reg OUT\n");
#endif 

	return ret_code;
}









int led_adp8861_keyilm_ctrl_chk( void )
{
	int						ret = 0;
	union u_led_isct1_reg  led_timer1_data;
	union u_led_isct2_reg  led_timer2_data;

	u_input_frame_es_two_led_type *up_frame_data;



	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl_chk IN\n");
#endif 

	memset( &led_timer1_data,  0, sizeof( led_timer1_data ) );
	memset( &led_timer2_data,  0, sizeof( led_timer2_data ) );




		

		up_frame_data = (u_input_frame_es_two_led_type *)gs_patern_data.front.s_set_data.p_patern ;



		
		if( gs_patern_data.front.s_set_data.s_data_info.frame_num == 2 ){
			
			if (( up_frame_data[0].s_data.fade.hw_act == LED_FLAG_ON )
			 && ( up_frame_data[1].s_data.fade.hw_act == LED_FLAG_ON )){
				
				if (( up_frame_data[0].s_data.fade.hw_on_off == LED_FLAG_ON  )
				 && ( up_frame_data[1].s_data.fade.hw_on_off == LED_FLAG_OFF )){
					
					led_timer1_data.st2.scon    = up_frame_data[0].s_data.fade.hw_set_data;

					led_timer2_data.st2.sc1_off = up_frame_data[1].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc2_off = up_frame_data[1].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc3_off = up_frame_data[1].s_data.fade.hw_set_data;


					led_timer2_data.st2.sc4_off = up_frame_data[1].s_data.fade.hw_set_data;
					led_timer1_data.st2.sc5_off = up_frame_data[1].s_data.fade.hw_set_data;
					led_timer1_data.st2.sc6_off = up_frame_data[1].s_data.fade.hw_set_data;
					
					led_adp8861_keyilm_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[0] );
					
					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_FRONT );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl_chk HRD1 \n");
					ret = 1;
				}
				else if (( up_frame_data[0].s_data.fade.hw_on_off == LED_FLAG_OFF )
					  && ( up_frame_data[1].s_data.fade.hw_on_off == LED_FLAG_ON  )){
					
					led_timer1_data.st2.scon    = up_frame_data[1].s_data.fade.hw_set_data;

					led_timer2_data.st2.sc1_off = up_frame_data[0].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc2_off = up_frame_data[0].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc3_off = up_frame_data[0].s_data.fade.hw_set_data;

					led_timer2_data.st2.sc4_off = up_frame_data[0].s_data.fade.hw_set_data;
					led_timer1_data.st2.sc5_off = up_frame_data[0].s_data.fade.hw_set_data;
					led_timer1_data.st2.sc6_off = up_frame_data[0].s_data.fade.hw_set_data;
					
					led_adp8861_keyilm_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[1] );
					
					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_FRONT );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl_chk HRD2 \n");
					ret = 1;
				}
			}

			else if(UP_FRAME_DATA_TWO_LED_ON(0)){
				if( UP_FRAME_DATA_TWO_LED_OFF(1) ){




					if( up_frame_data[0].s_data.u_time.time == 0x12 ){

						led_timer2_data.st2.sc1_off = 0;
						led_timer2_data.st2.sc2_off = 0;
						led_timer2_data.st2.sc3_off = 0;

						led_timer2_data.st2.sc4_off = 0;
						led_timer1_data.st2.sc5_off = 0;
						led_timer1_data.st2.sc6_off = 0;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x16 ){

						led_timer2_data.st2.sc1_off = 1;
						led_timer2_data.st2.sc2_off = 1;
						led_timer2_data.st2.sc3_off = 1;

						led_timer2_data.st2.sc4_off = 1;
						led_timer1_data.st2.sc5_off = 1;
						led_timer1_data.st2.sc6_off = 1;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x18 ){

						led_timer2_data.st2.sc1_off = 2;
						led_timer2_data.st2.sc2_off = 2;
						led_timer2_data.st2.sc3_off = 2;

						led_timer2_data.st2.sc4_off = 2;
						led_timer1_data.st2.sc5_off = 2;
						led_timer1_data.st2.sc6_off = 2;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x1C ){

						led_timer2_data.st2.sc1_off = 3;
						led_timer2_data.st2.sc2_off = 3;
						led_timer2_data.st2.sc3_off = 3;

						led_timer2_data.st2.sc4_off = 3;
						led_timer1_data.st2.sc5_off = 3;
						led_timer1_data.st2.sc6_off = 3;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x16 ){
						led_timer1_data.st2.scon = 1;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x1C ){
						led_timer1_data.st2.scon = 2;
					}
					led_adp8861_keyilm_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[0] );
					
					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_FRONT );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl_chk HRD1 \n");
					ret = 1;
				}
			}

			else if(UP_FRAME_DATA_TWO_LED_ON(1)){
				if( UP_FRAME_DATA_TWO_LED_OFF(0) ){




					if( up_frame_data[1].s_data.u_time.time == 0x12 ){

						led_timer2_data.st2.sc1_off = 0;
						led_timer2_data.st2.sc2_off = 0;
						led_timer2_data.st2.sc3_off = 0;

						led_timer2_data.st2.sc4_off = 0;
						led_timer1_data.st2.sc5_off = 0;
						led_timer1_data.st2.sc6_off = 0;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x16 ){

						led_timer2_data.st2.sc1_off = 1;
						led_timer2_data.st2.sc2_off = 1;
						led_timer2_data.st2.sc3_off = 1;

						led_timer2_data.st2.sc4_off = 1;
						led_timer1_data.st2.sc5_off = 1;
						led_timer1_data.st2.sc6_off = 1;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x18 ){

						led_timer2_data.st2.sc1_off = 2;
						led_timer2_data.st2.sc2_off = 2;
						led_timer2_data.st2.sc3_off = 2;

						led_timer2_data.st2.sc4_off = 2;
						led_timer1_data.st2.sc5_off = 2;
						led_timer1_data.st2.sc6_off = 2;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x1C ){

						led_timer2_data.st2.sc1_off = 3;
						led_timer2_data.st2.sc2_off = 3;
						led_timer2_data.st2.sc3_off = 3;

						led_timer2_data.st2.sc4_off = 3;
						led_timer1_data.st2.sc5_off = 3;
						led_timer1_data.st2.sc6_off = 3;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x16 ){
						led_timer1_data.st2.scon = 1;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x1C ){
						led_timer1_data.st2.scon = 2;
					}
					led_adp8861_keyilm_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[1] );
					
					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_FRONT );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl_chk HRD2 \n");
					ret = 1;
				}
			}
		}



	led_adp8861_keyilm_ctrl_reg_time( &led_timer1_data, &led_timer2_data );
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl_chk OUT\n");
#endif 

	return ret;
}


#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
#if defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)








static void led_adp8861_keyilm_ctrl( void )
{


	union u_input_frame_es_two_led u_frame_data;



	int get_frame, time_a = 0, time_b = 0, time = 0;
	int wait_ret, ret, count;
	int active_flag;






	union u_led_iscf_reg	led_fade;
	struct DVE027_state_t state;

	union u_led_isct1_reg  led_timer1_data;
	union u_led_isct2_reg  led_timer2_data;

	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl IN\n");
#endif
	

	memset( &led_timer1_data,  0, sizeof( led_timer1_data ) );
	memset( &led_timer2_data,  0, sizeof( led_timer2_data ) );

	
	led_brightness_set_keyilm_status(ADP8861_LED_ON);
	
	wake_lock(&led_ilm_wake_lock);
	
	while(1){
		
		mutex_lock( &gs_patern_data.func_lock );
		
		active_flag = led_active_flag_ctr( LED_FLAG_GET, LED_TYPE_FRONT );
		
		if( active_flag == LED_FLAG_OFF ){
			memset( &state, 0, sizeof( state ) );
			ret = led_ilm_func_State[8][gs_led_state.ilmkey.main_state](&state, LED_TYPE_FRONT);
			if( ret != LED_DATA_INPUT_OK ){
				printk(KERN_INFO "led_adp8861_keyilm_ctrl: led_ilm_func_State(8,%d) = %d \n",gs_led_state.ilmkey.main_state, ret);
			}
			gs_patern_data.front.ps_thread = NULL;
			mutex_unlock( &gs_patern_data.func_lock );
			
			led_brightness_set_keyilm_status(ADP8861_LED_OFF);
			break;
		}
		
		get_frame = led_get_frame_data( u_frame_data.uc, LED_TYPE_FRONT );
		
		if( get_frame == LED_DATA_INPUT_OK || get_frame == LED_DATA_INPUT_FLIE_END ){
			

			if( gs_patern_data.front.frame_cnt == 0 ){
				if( led_adp8861_keyilm_ctrl_chk() != 0 ){
					gs_patern_data.front.ps_thread = NULL;
					mutex_unlock( &gs_patern_data.func_lock );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_keyilm_ctrl hw end\n");
					break;
				}
				else{
					led_adp8861_keyilm_ctrl_reg_time( &led_timer1_data, &led_timer2_data);
				}
			}

			
			if(( u_frame_data.s_data.fade.fade & 0x01 ) == 0x01 ){
				
				
				

				led_fade.uc = u_frame_data.s_data.fade_time.time ;




				ret = adp8861_led_fade_set( &led_fade, ID_1 );

				led_adp8861_keyilm_ctrl_reg( ADP8861_LED_OFF, &led_timer1_data, &led_timer2_data, &u_frame_data );

				if( 0 != (int)u_frame_data.s_data.u_time.time ){
					
					count = (int)u_frame_data.s_data.u_time.s_tmie.time_a;
					time_a = 1;
					while( count > 0){
						time_a *= 10;
						count-- ;
					}
					time_b = (int)u_frame_data.s_data.u_time.s_tmie.time_b;
					time = time_a * time_b * 10;
				}




			}
			else{
				if( 0 != (int)u_frame_data.s_data.u_time.time ){
					
					count = (int)u_frame_data.s_data.u_time.s_tmie.time_a;
					time_a = 1;
					while( count > 0){
						time_a *= 10;
						count-- ;
					}
					time_b = (int)u_frame_data.s_data.u_time.s_tmie.time_b;
					time = time_a * time_b * 10;
				}

				led_fade.uc = 0;
				adp8861_led_fade_set( &led_fade, ID_1 );

#if defined(LED_DIAG_DEBUG_PLUS)
				printk(KERN_INFO "[leds_data_input]wait tmie : %d mse \n",time);
#endif 
				
				
				
				

				led_adp8861_keyilm_ctrl_reg( ADP8861_LED_OFF, &led_timer1_data, &led_timer2_data, &u_frame_data );



























































































			}
			
			if( get_frame == LED_DATA_INPUT_FLIE_END ){
				led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_FRONT );
			}
			gs_patern_data.front.check_flag = 0;
			mutex_unlock( &gs_patern_data.func_lock );
			
			wait_ret = 0;
			if(time != 0 ){
				wait_ret =  wait_event_interruptible_timeout( gs_led_wq, ( gs_patern_data.front.check_flag != 0 ), msecs_to_jiffies(time) );
			}
			if( wait_ret != 0 )
				printk(KERN_INFO "[leds_data_input]wait_event_interruptible_timeout : %d \n",wait_ret);
		}
		else {
			led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_FRONT );
			mutex_unlock( &gs_patern_data.func_lock );
		}
	}
	







	
	wake_unlock(&led_ilm_wake_lock);
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_DEBUG "[leds_data_input]led_adp8861_keyilm_ctrl OUT\n");
#endif
	
	return ;
}
#endif 
#endif 





















































































































































































































































































































































static int led_adp8861_reflectilm_ctrl_reg_time( union u_led_isct1_reg  *led_timer1_data,
										   union u_led_isct2_reg  *led_timer2_data)
{
	int						ret, ret_code = ADP8861_LED_SET_OK;
	union u_led_isct1_reg	led_timer1_set;
	union u_led_isct2_reg	led_timer2_set;

#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_reflectilm_ctrl_reg_time IN\n");
#endif 

	memset( &led_timer2_set,  0, sizeof( led_timer2_set ) );
	led_timer2_set.st2.sc4_off = ADP8861_LED_ON;

	ret_code = ret = adp8861_led_timer_set2( &led_timer2_set, led_timer2_data, ID_0 );

	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_reflectilm_ctrl_reg_time: adp8861_led_timer_set2 = %d \n",ret);
	}
	memset( &led_timer1_set,  0, sizeof( led_timer1_set ) );
	led_timer1_set.st2.sc5_off = ADP8861_LED_ON;
	led_timer1_set.st2.sc6_off = ADP8861_LED_ON;
	led_timer1_set.st2.scon = ADP8861_LED_ON;

	ret_code |= ret = adp8861_led_timer_set1( &led_timer1_set, led_timer1_data, ID_0 );

	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_reflectilm_ctrl_reg_time: adp8861_led_timer_set1 = %d \n",ret);
	}

#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_reflectilm_ctrl_reg_time OUT\n");
#endif 
	return ret_code;
}









static int led_adp8861_reflectilm_ctrl_reg( unsigned char timer_set_flg,
									   union u_led_isct1_reg  *led_timer1_data,
									   union u_led_isct2_reg  *led_timer2_data,
									   u_input_frame_es_type *u_frame_data )
{
	int						ret, ret_code = ADP8861_LED_SET_OK;
	union u_led_isc_reg		set_brigh[3];
	struct led_request_rgb	set_onoff;
	struct adp8861_3color_led_parame_nv  adp8861_3color_led_parame_nv;
	struct led_set_brightness led_brightness;

#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_reflectilm_ctrl_reg IN\n");
#endif 

	memset( set_brigh,  0, sizeof( set_brigh ) );
	memset( &set_onoff, 0, sizeof( set_onoff ) );
	memset( &adp8861_3color_led_parame_nv, 0, sizeof(  struct adp8861_3color_led_parame_nv ));

	adp8861_get_3color_nv(&adp8861_3color_led_parame_nv);
	get_led_set_brightness(&u_frame_data->s_data.brightness[0], 3, &adp8861_3color_led_parame_nv, &led_brightness);
	
	set_brigh[0].st2.set_flag = ADP8861_LED_ON;
	set_brigh[0].st2.scd	  = led_brightness.set_red;
	if( led_brightness.set_red != 0 ){
		set_onoff.set_r = ADP8861_LED_ON;
	}
	set_brigh[1].st2.set_flag = ADP8861_LED_ON;
	set_brigh[1].st2.scd	  = led_brightness.set_green;
	if( led_brightness.set_green != 0 ){
	set_onoff.set_g =  ADP8861_LED_ON;
	}
	set_brigh[2].st2.set_flag = ADP8861_LED_ON;
	set_brigh[2].st2.scd	  = led_brightness.set_blue;
	if( led_brightness.set_blue != 0 ){
	set_onoff.set_b =  ADP8861_LED_ON;
	}
	mutex_lock(&g_headset_led_state_mutex);
	memcpy( &stored_set_brigh, &set_brigh, sizeof(set_brigh) );
	if ( insert_enable == INSERT_TRUE ) {
		
		memset( &set_brigh, 0, sizeof( set_brigh ) );
	}
	mutex_unlock(&g_headset_led_state_mutex);
	ret_code |= ret = adp8861_er_rgb_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_reflectilm_ctrl_reg: adp8861_er_rgb_led_bright = %d \n",ret);
	}
	if( timer_set_flg == ADP8861_LED_ON ){
		ret = led_adp8861_reflectilm_ctrl_reg_time( led_timer1_data, led_timer2_data);
	}
	mutex_lock(&g_headset_led_state_mutex);
	memcpy( &stored_illumination_onoff, &set_onoff, sizeof(struct led_request_rgb) );
	memcpy( &stored_illumination_onoff_hard, &set_onoff, sizeof(struct led_request_rgb) );
	if ( insert_enable == INSERT_TRUE ) {
		
		memset( &set_onoff, 0, sizeof( set_onoff ) );
	}
	mutex_unlock(&g_headset_led_state_mutex);
	
	ret_code |= ret = adp8861_er_rgb_led_set( &set_onoff );
	if( ret != ADP8861_LED_SET_OK ){
		printk(KERN_INFO "led_adp8861_reflectilm_ctr_reg: adp8861_er_rgb_led_set(%d,%d,%d) = %d \n",set_onoff.set_r,set_onoff.set_g,set_onoff.set_b,ret);
	}

#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_reflectilm_ctrl_reg OUT\n");
#endif 

	return ret_code;
}









int led_adp8861_reflectilm_ctrl_chk( void )
{
	int						ret = 0;
	union u_led_isct1_reg  led_timer1_data;
	union u_led_isct2_reg  led_timer2_data;
	u_input_frame_es_type *up_frame_data;

#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_chk IN\n");
#endif 

	memset( &led_timer1_data,  0, sizeof( led_timer1_data ) );
	memset( &led_timer2_data,  0, sizeof( led_timer2_data ) );





		up_frame_data = (u_input_frame_es_type *)gs_patern_data.reflect.s_set_data.p_patern ;

		if( gs_patern_data.reflect.s_set_data.s_data_info.frame_num == 2 ){

			if (( up_frame_data[0].s_data.fade.hw_act == LED_FLAG_ON )
			 && ( up_frame_data[1].s_data.fade.hw_act == LED_FLAG_ON )){

				if (( up_frame_data[0].s_data.fade.hw_on_off == LED_FLAG_ON  )
				 && ( up_frame_data[1].s_data.fade.hw_on_off == LED_FLAG_OFF )){

					led_timer1_data.st2.scon    = up_frame_data[0].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc4_off = up_frame_data[1].s_data.fade.hw_set_data;
					led_timer1_data.st2.sc5_off = up_frame_data[1].s_data.fade.hw_set_data;
					led_timer1_data.st2.sc6_off = up_frame_data[1].s_data.fade.hw_set_data;

					led_adp8861_reflectilm_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[0] );

					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_REFLECT );

					printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_chk HRD1 \n");
					ret = 1;
				}
				else if (( up_frame_data[0].s_data.fade.hw_on_off == LED_FLAG_OFF )
					  && ( up_frame_data[1].s_data.fade.hw_on_off == LED_FLAG_ON  )){

					led_timer1_data.st2.scon    = up_frame_data[1].s_data.fade.hw_set_data;
					led_timer2_data.st2.sc4_off = up_frame_data[0].s_data.fade.hw_set_data;
					led_timer1_data.st2.sc5_off = up_frame_data[0].s_data.fade.hw_set_data;
					led_timer1_data.st2.sc6_off = up_frame_data[0].s_data.fade.hw_set_data;

					led_adp8861_reflectilm_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[1] );

					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_REFLECT );

					printk(KERN_INFO "[leds_data_input]led_adp8861_3led_ctrl_chk HRD2 \n");
					ret = 1;
				}
			}
			else if(UP_FRAME_DATA_ON(0)){
				if( UP_FRAME_DATA_OFF(1) ){
					if( up_frame_data[0].s_data.u_time.time == 0x12 ){
						led_timer2_data.st2.sc4_off = 0;
						led_timer1_data.st2.sc5_off = 0;
						led_timer1_data.st2.sc6_off = 0;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x16 ){
						led_timer2_data.st2.sc4_off = 1;
						led_timer1_data.st2.sc5_off = 1;
						led_timer1_data.st2.sc6_off = 1;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x18 ){
						led_timer2_data.st2.sc4_off = 2;
						led_timer1_data.st2.sc5_off = 2;
						led_timer1_data.st2.sc6_off = 2;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x1C ){
						led_timer2_data.st2.sc4_off = 3;
						led_timer1_data.st2.sc5_off = 3;
						led_timer1_data.st2.sc6_off = 3;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x16 ){
						led_timer1_data.st2.scon = 1;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x1C ){
						led_timer1_data.st2.scon = 2;
					}
					led_adp8861_reflectilm_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[0] );

					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_REFLECT );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_reflectilm_ctrl_chk HRD1 \n");
					ret = 1;
				}
			}
			else if(UP_FRAME_DATA_ON(1)){
				if( UP_FRAME_DATA_OFF(0) ){
					if( up_frame_data[1].s_data.u_time.time == 0x12 ){
						led_timer2_data.st2.sc4_off = 0;
						led_timer1_data.st2.sc5_off = 0;
						led_timer1_data.st2.sc6_off = 0;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x16 ){
						led_timer2_data.st2.sc4_off = 1;
						led_timer1_data.st2.sc5_off = 1;
						led_timer1_data.st2.sc6_off = 1;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x18 ){
						led_timer2_data.st2.sc4_off = 2;
						led_timer1_data.st2.sc5_off = 2;
						led_timer1_data.st2.sc6_off = 2;
					}
					if( up_frame_data[1].s_data.u_time.time == 0x1C ){
						led_timer2_data.st2.sc4_off = 3;
						led_timer1_data.st2.sc5_off = 3;
						led_timer1_data.st2.sc6_off = 3;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x16 ){
						led_timer1_data.st2.scon = 1;
					}
					if( up_frame_data[0].s_data.u_time.time == 0x1C ){
						led_timer1_data.st2.scon = 2;
					}
					led_adp8861_reflectilm_ctrl_reg( ADP8861_LED_ON, &led_timer1_data, &led_timer2_data, &up_frame_data[1] );

					led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_REFLECT );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_reflectilm_ctrl_chk HRD2 \n");
					ret = 1;
				}
			}
		}



	led_adp8861_reflectilm_ctrl_reg_time( &led_timer1_data, &led_timer2_data );

#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_reflectilm_ctrl_chk OUT\n");
#endif 

	return ret;
}









static void led_adp8861_reflectilm_ctrl( void )
{
	union u_input_frame_es u_frame_data;
	int get_frame, time_a = 0, time_b = 0, time = 0;
	int wait_ret, ret, count;
	int active_flag;





	union u_led_iscf_reg	led_fade;
	struct DVE027_state_t state;

	union u_led_isct1_reg  led_timer1_data;
	union u_led_isct2_reg  led_timer2_data;

#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_adp8861_reflectilm_ctrl IN\n");
#endif

	memset( &led_timer1_data,  0, sizeof( led_timer1_data ) );
	memset( &led_timer2_data,  0, sizeof( led_timer2_data ) );

	wake_lock(&led_ilm_wake_lock);

	while(1){

		mutex_lock( &gs_patern_data.func_lock );

		active_flag = led_active_flag_ctr( LED_FLAG_GET, LED_TYPE_REFLECT);

		if( active_flag == LED_FLAG_OFF ){
			memset(&stored_illumination_onoff, 0, sizeof(struct led_request_rgb));
			memset( &state, 0, sizeof( state ) );
			ret = led_ilm_func_State[12][gs_led_state.ilmreflect.main_state](&state, LED_TYPE_REFLECT);
			if( ret != LED_DATA_INPUT_OK ){
				printk(KERN_INFO "led_adp8861_reflectilm_ctrl: led_ilm_func_State(8,%d) = %d \n",gs_led_state.ilmreflect.main_state, ret);
			}
			gs_patern_data.reflect.ps_thread = NULL;
			mutex_unlock( &gs_patern_data.func_lock );

			break;
		}

		get_frame = led_get_frame_data( u_frame_data.uc, LED_TYPE_REFLECT );

		if( get_frame == LED_DATA_INPUT_OK || get_frame == LED_DATA_INPUT_FLIE_END ){

			if( gs_patern_data.reflect.frame_cnt == 0 ){
				if( led_adp8861_reflectilm_ctrl_chk() != 0 ){
					gs_patern_data.reflect.ps_thread = NULL;
					mutex_unlock( &gs_patern_data.func_lock );
					
					printk(KERN_INFO "[leds_data_input]led_adp8861_reflectilm_ctrl hw end\n");
					break;
				}
				else{
					led_adp8861_reflectilm_ctrl_reg_time( &led_timer1_data, &led_timer2_data);
				}
			}

			if(( u_frame_data.s_data.fade.fade & 0x01 ) == 0x01 ){
				
				
				

				led_fade.uc = u_frame_data.s_data.fade_time.time ;

				ret = adp8861_led_fade_set( &led_fade, ID_0 );

				led_adp8861_reflectilm_ctrl_reg( ADP8861_LED_OFF, &led_timer1_data, &led_timer2_data, &u_frame_data );
				if( 0 != (int)u_frame_data.s_data.u_time.time ){
					
					count = (int)u_frame_data.s_data.u_time.s_tmie.time_a;
					time_a = 1;
					while( count > 0){
						time_a *= 10;
						count-- ;
					}
					time_b = (int)u_frame_data.s_data.u_time.s_tmie.time_b;
					time = time_a * time_b * 10;
				}





			}
			else{
				if( 0 != (int)u_frame_data.s_data.u_time.time ){
					
					count = (int)u_frame_data.s_data.u_time.s_tmie.time_a;
					time_a = 1;
					while( count > 0){
						time_a *= 10;
						count-- ;
					}
					time_b = (int)u_frame_data.s_data.u_time.s_tmie.time_b;
					time = time_a * time_b * 10;
				}

				led_fade.uc = 0;
				adp8861_led_fade_set( &led_fade, ID_0 );

#if defined(LED_DIAG_DEBUG_PLUS)
				printk(KERN_INFO "[leds_data_input]wait tmie : %d mse \n",time);
#endif 

				
				
				

				led_adp8861_reflectilm_ctrl_reg( ADP8861_LED_OFF, &led_timer1_data, &led_timer2_data, &u_frame_data );






























			}

			if( get_frame == LED_DATA_INPUT_FLIE_END ){
				led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_REFLECT );
			}
			gs_patern_data.reflect.check_flag = 0;
			mutex_unlock( &gs_patern_data.func_lock );

			wait_ret = 0;
			if(time != 0 ){
				wait_ret =  wait_event_interruptible_timeout( gs_led_wq, ( gs_patern_data.reflect.check_flag != 0 ), msecs_to_jiffies(time) );
			}
			if( wait_ret != 0 )
				printk(KERN_INFO "[leds_data_input]wait_event_interruptible_timeout : %d \n",wait_ret);
		}
		else {
			led_active_flag_ctr( LED_FLAG_OFF, LED_TYPE_REFLECT );
			mutex_unlock( &gs_patern_data.func_lock );
		}
	}

	wake_unlock(&led_ilm_wake_lock);

#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_DEBUG "[leds_data_input]led_adp8861_reflectilm_ctrl OUT\n");
#endif

	return ;
}









static int led_run_stop( int type )
{
	int ret = 0;
#if defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)
	union u_led_iscf_reg	led_fade;
	union u_led_isc_reg set_brigh[3];


	union u_led_isct1_reg  led_timer1_data;
	union u_led_isct2_reg  led_timer2_data;
	struct led_request_rgb	set_onoff1;
	struct led_request_key	set_onoff2;
#endif 




#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_run_stop IN\n");
#endif 
	
#if defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)
	memset( &led_fade,  0, sizeof( led_fade  ) );
	memset( &set_brigh, 0, sizeof( set_brigh ) );
	memset( &set_onoff1, 0, sizeof( set_onoff1 ) );
	memset( &set_onoff2, 0, sizeof( set_onoff2 ) );
	memset( &led_timer1_data,  0, sizeof( led_timer1_data ) );
	memset( &led_timer2_data,  0, sizeof( led_timer2_data ) );
#endif 
	
	switch(type){
		
	  case LED_TYPE_INCOMING:
#if defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)

		ret = adp8861_led_fade_set( &led_fade, ID_0 );



		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_run_stop: adp8861_led_fade_set = %d \n",ret);
		}
		set_brigh[0].st2.set_flag = ADP8861_LED_ON;
		set_brigh[1].st2.set_flag = ADP8861_LED_ON;
		set_brigh[2].st2.set_flag = ADP8861_LED_ON;
		ret |= adp8861_rgb_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );
		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_run_stop: adp8861_rgb_led_bright = %d \n",ret);
		}
		led_adp8861_3led_ctrl_reg_time( &led_timer1_data, &led_timer2_data);
		
		ret |= adp8861_rgb_led_set( &set_onoff1 );
		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_run_stop: adp8861_rgb_led_set(%d,%d,%d) = %d \n",set_onoff1.set_r,set_onoff1.set_g,set_onoff1.set_b,ret);
		}
#endif 







		break;
		
#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
	  case LED_TYPE_FRONT:

#if defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)

		ret = adp8861_led_fade_set( &led_fade, ID_1 );



		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_run_stop: adp8861_led_fade_set = %d \n",ret);
		}
		set_brigh[0].st2.set_flag = ADP8861_LED_ON;
		set_brigh[1].st2.set_flag = ADP8861_LED_ON;
		set_brigh[2].st2.set_flag = ADP8861_LED_ON;

		ret = adp8861_right_key_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );
		ret = adp8861_left_key_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );



		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_run_stop: adp8861_rgb_led_bright = %d \n",ret);
		}
		led_adp8861_keyilm_ctrl_reg_time( &led_timer1_data, &led_timer2_data);

		ret = adp8861_right_key_led_set( &set_onoff1 );
		ret = adp8861_left_key_led_set( &set_onoff1 );



		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_run_stop: adp8861_rgb_led_set(%d,%d,%d) = %d \n",set_onoff1.set_r,set_onoff1.set_g,set_onoff1.set_b,ret);
		}
		led_brightness_set_keyilm_status(ADP8861_LED_OFF);
		
#endif 











		break;
#endif 













	  case LED_TYPE_REFLECT:
		ret = adp8861_led_fade_set( &led_fade, ID_0 );
		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_run_stop: adp8861_led_fade_set = %d \n",ret);
		}
		set_brigh[0].st2.set_flag = ADP8861_LED_ON;
		set_brigh[1].st2.set_flag = ADP8861_LED_ON;
		set_brigh[2].st2.set_flag = ADP8861_LED_ON;

		ret = adp8861_er_rgb_led_bright( &set_brigh[0], &set_brigh[1], &set_brigh[2] );
		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_run_stop: adp8861_er_rgb_led_bright = %d \n",ret);
		}

		ret = adp8861_er_rgb_led_set( &set_onoff1 );
		if( ret != ADP8861_LED_SET_OK ){
			printk(KERN_INFO "led_run_stop: adp8861_er_rgb_led_set(%d,%d,%d) = %d \n",set_onoff1.set_r,set_onoff1.set_g,set_onoff1.set_b,ret);
		}
		break;


	  default:
		ret = -EINVAL;
        break;
    }
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_run_stop OUT ret = %d\n", ret );
#endif 
	
	return ret;
}









static void led_incoming_thread( void )
{



#if defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)
	led_adp8861_3led_ctrl();
#endif 
	return ;
}

#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)








static void led_front_thread( void )
{



#if defined(LOCAL_CONFIG_FEATURE_DVE021_ELEGANT_SLIM)
	led_adp8861_keyilm_ctrl();
#endif 
	return ;
}
#endif 



























static void led_reflect_thread( void )
{
	led_adp8861_reflectilm_ctrl();

	return ;
}










static int led_thread_create( int type )
{
	int ret = 0;
	
#if defined(LED_DIAG_DEBUG_PLUS)
    printk(KERN_INFO "[leds_data_input]led_thread_create IN type = %d\n", type );
#endif 
	
	switch(type){
	
    case LED_TYPE_INCOMING:
    
    	led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
    	
		if( gs_patern_data.incoming.ps_thread == NULL ){
			gs_patern_data.incoming.ps_thread = kthread_run((void*)*led_incoming_thread, NULL, "led_incoming_thread" );
			
			if( IS_ERR(gs_patern_data.incoming.ps_thread) ){
				printk(KERN_INFO "[leds_data_input]kthread_run NG\n" );
				gs_patern_data.incoming.ps_thread = NULL;
				ret = -EINVAL;
			}
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		break;
		
#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
    case LED_TYPE_FRONT:
    
    	led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
    	
		if( gs_patern_data.front.ps_thread == NULL ){
			 gs_patern_data.front.ps_thread = kthread_run((void*)*led_front_thread, NULL, "led_front_thread" );
			
			if( IS_ERR(gs_patern_data.front.ps_thread) ){
				printk(KERN_INFO "[leds_data_input]kthread_run NG\n" );
				gs_patern_data.front.ps_thread = NULL;
				ret = -EINVAL;
			}
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
    
		break;
#endif 




















    case LED_TYPE_REFLECT:

	led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_REFLECT );

		if( gs_patern_data.reflect.ps_thread == NULL ){
			 gs_patern_data.reflect.ps_thread = kthread_run((void*)*led_reflect_thread, NULL, "led_reflect_thread" );

			if( IS_ERR(gs_patern_data.reflect.ps_thread) ){
				printk(KERN_INFO "[leds_data_input]kthread_run NG\n" );
				gs_patern_data.reflect.ps_thread = NULL;
				ret = -EINVAL;
			}
		}

		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_REFLECT );

		break;

    default:

		ret = -EINVAL;
        break;
    }
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_thread_create OUT ret = %d\n", ret );	
#endif 
	return ret;
}









static int led_thread_stop( int type )
{
	int ret = 0;
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_thread_stop IN type = %d\n", type );
#endif 
	
	switch(type){
	
    case LED_TYPE_INCOMING:
    
    	led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
    	
		if( gs_patern_data.incoming.ps_thread != NULL ){


			gs_patern_data.incoming.active_flag = LED_FLAG_OFF;
			led_wake_up_thread( type );
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		break;
		
#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
    case LED_TYPE_FRONT:
    
    	led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
    	
		if( gs_patern_data.front.ps_thread != NULL ){


			gs_patern_data.front.active_flag = LED_FLAG_OFF;
			led_wake_up_thread( type );
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
    
		break;
#endif 

















    case LED_TYPE_REFLECT:
    
    	led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_REFLECT );
    	
		if( gs_patern_data.reflect.ps_thread != NULL ){
			gs_patern_data.reflect.active_flag = LED_FLAG_OFF;
			led_wake_up_thread( type );
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_REFLECT );
		
		break;

    default:
		
		ret = -EINVAL;
        break;
    }
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_DEBUG "[leds_data_input]led_thread_stop OUT ret = %d\n", ret );
#endif 
	
	return ret;
}









static int led_patern_data_read( unsigned char *p_data )
{
	struct input_data_head_struct s_head_data;
	void *p_patern_data;
	int err = 0;
	int switch_type;
	struct DVE027_state_t state;
	
#if defined(LED_DIAG_DEBUG_PLUS)
    printk(KERN_INFO "[leds_data_input]led_patern_data_read IN\n");
#endif 

	memcpy( &s_head_data, p_data, sizeof(s_head_data) );
	memset( &state, 0, sizeof( state ) );
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]data_size = %d led_type = %d led_num = %d frame_num = %d frame_size = %d\n",
			s_head_data.data_size, s_head_data.led_type, s_head_data.led_num, s_head_data.frame_num, s_head_data.frame_size );
#endif
 	
	if( ( s_head_data.frame_size == 0 ) || 
		( s_head_data.frame_num == 0 )  || 
		( s_head_data.data_size != s_head_data.frame_num * s_head_data.frame_size ) ){
		printk(KERN_INFO "[leds_data_input]led_patern_data_read head_data NG\n");
		return -EINVAL;
	}
	
	p_patern_data = kzalloc( s_head_data.data_size, GFP_KERNEL );
	
	if( p_patern_data == NULL ){
		printk(KERN_INFO "[leds_data_input]led_patern_data_read p_data kzalloc NG\n");
		return -EINVAL;
	}
	
	memcpy( p_patern_data, p_data + sizeof(s_head_data), s_head_data.data_size );
	
	switch_type = s_head_data.led_type;
	










	
    switch(switch_type){
	
    case LED_TYPE_INCOMING:
		
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );
		
		if( gs_patern_data.incoming.s_set_data.p_patern == NULL ){
			gs_patern_data.incoming.s_set_data.p_patern = p_patern_data;
			memcpy( &gs_patern_data.incoming.s_set_data.s_data_info, &s_head_data, sizeof(s_head_data) );
		}
		else{
			if( gs_patern_data.incoming.s_ready_data.p_patern != NULL ){
				kfree( gs_patern_data.incoming.s_ready_data.p_patern );
			}
			gs_patern_data.incoming.s_ready_data.p_patern = p_patern_data;
			memcpy( &gs_patern_data.incoming.s_ready_data.s_data_info, &s_head_data, sizeof(s_head_data) );
		}
		
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
		
#if defined(LED_DIAG_DEBUG_PLUS)
		printk(KERN_INFO "[leds_data_input]led_patern_data_read LED_TYPE_INCOMING main = %p\n", gs_patern_data.incoming.s_set_data.p_patern );
		printk(KERN_INFO "[leds_data_input]led_patern_data_read LED_TYPE_INCOMING sub  = %p\n", gs_patern_data.incoming.s_ready_data.p_patern );
#endif 

		err = led_ilm_func_State[0][gs_led_state.ilm3led.main_state](&state, s_head_data.led_type);
		
        break;
        
#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
    case LED_TYPE_FRONT:
		
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		
		if( gs_patern_data.front.s_set_data.p_patern == NULL ){
			gs_patern_data.front.s_set_data.p_patern = p_patern_data;
			memcpy( &gs_patern_data.front.s_set_data.s_data_info, &s_head_data, sizeof(s_head_data) );
		}
		else{
			if( gs_patern_data.front.s_ready_data.p_patern != NULL ){
				kfree( gs_patern_data.front.s_ready_data.p_patern );
			}
			gs_patern_data.front.s_ready_data.p_patern = p_patern_data;
			memcpy( &gs_patern_data.front.s_ready_data.s_data_info, &s_head_data, sizeof(s_head_data) );
		}
		
#if defined(LED_DIAG_DEBUG_PLUS)
		printk(KERN_INFO "[leds_data_input]led_patern_data_read LED_TYPE_FRONT main = %p\n", gs_patern_data.front.s_set_data.p_patern );
		printk(KERN_INFO "[leds_data_input]led_patern_data_read LED_TYPE_FRONT sub  = %p\n", gs_patern_data.front.s_ready_data.p_patern );
#endif 

		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
		
		err = led_ilm_func_State[5][gs_led_state.ilmkey.main_state](&state, s_head_data.led_type);
		
        break;
#endif 

    case LED_TYPE_REFLECT:

		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_REFLECT );

		if( gs_patern_data.reflect.s_set_data.p_patern == NULL ){
			gs_patern_data.reflect.s_set_data.p_patern = p_patern_data;
			memcpy( &gs_patern_data.reflect.s_set_data.s_data_info, &s_head_data, sizeof(s_head_data) );
		}
		else{
			if( gs_patern_data.reflect.s_ready_data.p_patern != NULL ){
				kfree( gs_patern_data.reflect.s_ready_data.p_patern );
			}
			gs_patern_data.reflect.s_ready_data.p_patern = p_patern_data;
			memcpy( &gs_patern_data.reflect.s_ready_data.s_data_info, &s_head_data, sizeof(s_head_data) );
		}

#if defined(LED_DIAG_DEBUG_PLUS)
		printk(KERN_INFO "[leds_data_input]led_patern_data_read LED_TYPE_REFLECT main = %p\n", gs_patern_data.reflect.s_set_data.p_patern );
		printk(KERN_INFO "[leds_data_input]led_patern_data_read LED_TYPE_REFLECT sub  = %p\n", gs_patern_data.reflect.s_ready_data.p_patern );
#endif 

		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_REFLECT );

		err = led_ilm_func_State[9][gs_led_state.ilmreflect.main_state](&state, s_head_data.led_type);

	break;


































































    default:
		
		kfree( p_patern_data );
		return -EINVAL;
        break;
    }
    
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_patern_data_read OUT\n");
#endif 

	return err;
}









static int led_run_loop( int type )
{
	int ret = 0;
	
	led_active_flag_ctr(LED_FLAG_ON, type );
	ret = led_thread_create( type );
	
	return ret;
}










static int led_data_input_open(struct inode *inode, struct file *file)
{
#if defined(LED_DIAG_DEBUG_PLUS)
    printk(KERN_INFO "[leds_data_input]led_data_input_open\n");
#endif 
    return 0;
}





































































void led_vma_open( struct vm_area_struct *vma )
{
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_vma_open: virt=%lx, phys=%d \n",
			vma->vm_start, vma->vm_page_prot << PAGE_SHIFT );
#endif 
	return ;
}









void led_vma_close( struct vm_area_struct *vma )
{
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_vma_close \n");
#endif 
	return ;
}

static struct vm_operations_struct led_remap_vm_ops = {
    .open       = led_vma_open,
    .close      = led_vma_close,
};









static void led_state_set( int type, int main_state, int sub_state )
{
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_state_set type %d, main %d, sub %d \n", type, main_state, sub_state );
#endif
	
	switch(type){
	  case LED_TYPE_INCOMING:
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_INCOMING );










		gs_led_state.ilm3led.main_state = main_state;
		gs_led_state.ilm3led.sub_state = sub_state;
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_INCOMING );
		break;
#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
	  case LED_TYPE_FRONT:
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_FRONT );
		gs_led_state.ilmkey.main_state = main_state;
		gs_led_state.ilmkey.sub_state = sub_state;
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_FRONT );
        break;
#endif 

















	  case LED_TYPE_REFLECT:
		led_lock_wrapper( LED_LOCK_SET, LED_LOCK_TYPE_REFLECT );
		gs_led_state.ilmreflect.main_state = main_state;
		gs_led_state.ilmreflect.sub_state = sub_state;
		led_lock_wrapper( LED_UNLOCK_SET, LED_LOCK_TYPE_REFLECT );
        break;

	  default:
		break;
	}
}









































































static int led_func_error( struct DVE027_state_t *state, int type )
{
	printk(KERN_INFO "[leds_data_input]led_func_error \n");
	
	return -EINVAL;
}

static int led_func_ok( struct DVE027_state_t *state, int type )
{
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_DEBUG "[leds_data_input]led_func_ok \n");
#endif
	
	return 0;
}

static int led_func_stop( struct DVE027_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_func_stop type = %d \n", type);
#endif
	
	
	ret = led_thread_stop( type );
	ret |= led_patern_data_del( type );
	set_patern = led_patern_data_switch( type );
	ret |= led_run_stop( type );
	
	if( set_patern == NULL ){
		led_state_set( type, LED_STATE_STOP, 0 );
	}
	else{
		led_state_set( type, LED_STATE_PATERN, 0 );
	}
	
	return ret;
}

static int led_func_0_0( struct DVE027_state_t *state, int type )
{
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_func_0_0 type = %d \n", type);
#endif
	
	led_state_set( LED_TYPE_INCOMING, LED_STATE_PATERN, LED_STATE_SUB_3LED );
	
	return LED_DATA_INPUT_OK;
}














static int led_func_1_1( struct DVE027_state_t *state, int type )
{
	int ret = 0;
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_func_1_1 type = %d \n", type);
#endif
	
	led_patern_data_switch( type );
	
	ret = led_run_loop( LED_TYPE_INCOMING );
	if( ret == 0 ){
		led_state_set( LED_TYPE_INCOMING, LED_STATE_LOCAL_OPE, LED_STATE_SUB_3LED );
	}
	return ret;
}

static int led_func_1_2( struct DVE027_state_t *state, int type )
{
	int ret = LED_DATA_INPUT_OK;
	u8	*set_patern = NULL;
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_func_1_2 type = %d \n", type);
#endif
	
	led_active_flag_ctr( LED_FLAG_ON, type );
	set_patern = led_patern_data_switch( type );
	led_wake_up_thread( type );
	
	if( set_patern == NULL ){
		ret = LED_DATA_INPUT_NG;
	}
	else {
		ret = led_thread_create( type );
	}
	
	return ret;
}

#if defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)
static int led_func_1_2_battery( struct DVE027_state_t *state, int type )
{
	int ret = LED_DATA_INPUT_OK;
	u8	*set_patern = NULL;
	
	printk(KERN_INFO "[leds_data_input]led_func_1_2_battery type = %d \n", type);
	
	led_active_flag_ctr( LED_FLAG_ON, type );
	set_patern = led_patern_data_switch_battery( type );
	led_wake_up_thread( type );
	
	if( set_patern == NULL )
	{
		ret = LED_DATA_INPUT_NG;
	}
	else
	{
		ret = led_thread_create( type );
	}
	
	return ret;
}
#endif 




































































































































































































































































#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
static int led_func_5_0( struct DVE027_state_t *state, int type )
{
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_func_5_0 type = %d \n", type);
#endif
	
	led_state_set( LED_TYPE_FRONT, LED_STATE_PATERN, LED_STATE_SUB_3LED );
	
	return LED_DATA_INPUT_OK;
}

static int led_func_6_1( struct DVE027_state_t *state, int type )
{
	int ret = 0;
	
	ret = led_run_loop( LED_TYPE_FRONT );
	if( ret == 0 ){
		led_state_set( LED_TYPE_FRONT, LED_STATE_LOCAL_OPE, LED_STATE_SUB_3LED );
	}
	return ret;
}

static int led_func_6_2( struct DVE027_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_func_6_2 type = %d \n", type);
#endif
	
	led_active_flag_ctr( LED_FLAG_ON, type );
	set_patern = led_patern_data_switch( type );
	led_wake_up_thread( type );
	
	if( set_patern == NULL ){
		ret = LED_DATA_INPUT_NG;
	}
	else {
		ret = led_thread_create( type );
	}
	
	return ret;
}
#endif 




















































































































































































































































































































































































































































































































































































static int led_func_9_0( struct DVE027_state_t *state, int type )
{
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_func_9_0 type = %d \n", type);
#endif

	led_state_set( LED_TYPE_REFLECT, LED_STATE_PATERN, LED_STATE_SUB_3LED );

	return LED_DATA_INPUT_OK;
}

static int led_func_a_1( struct DVE027_state_t *state, int type )
{
	int ret = 0;

	ret = led_run_loop( LED_TYPE_REFLECT );
	if( ret == 0 ){
		led_state_set( LED_TYPE_REFLECT, LED_STATE_LOCAL_OPE, LED_STATE_SUB_3LED );
	}
	return ret;
}

static int led_func_a_2( struct DVE027_state_t *state, int type )
{
	int ret = 0;
	u8	*set_patern = NULL;

#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_func_a_2 type = %d \n", type);
#endif

	led_active_flag_ctr( LED_FLAG_ON, type );
	set_patern = led_patern_data_switch( type );
	led_wake_up_thread( type );

	if( set_patern == NULL ){
		ret = LED_DATA_INPUT_NG;
	}
	else {
		ret = led_thread_create( type );
	}

	return ret;
}










static int led_lights_3led(struct DVE027_state_t *u_state)
{
	int err = 0;
	struct DVE027_state_t state;
	int type = LED_TYPE_INCOMING;
	
#if defined(LED_DIAG_DEBUG_PLUS)
    printk(KERN_INFO "[leds_data_input]led_lights_3led IN\n");
#endif 

	err = copy_from_user( &state, u_state, sizeof(struct DVE027_state_t) );
	if( err ){
		printk(KERN_INFO "[leds_data_input]led_lights_3led state read NG\n");
		return -EINVAL;
	}
	
	
	if( state.color == 0 ){
		err = led_ilm_func_State[3][gs_led_state.ilm3led.main_state](&state, type);
	}










	else{
		led_loop_set( state.flashOnMS, type );
		err = led_ilm_func_State[1][gs_led_state.ilm3led.main_state](&state, type);
	}


#if defined(LED_DIAG_DEBUG_PLUS)
    printk(KERN_INFO "[leds_data_input]led_lights_3led OUT\n");
#endif 

    return err;
}

#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)








static int led_DVE029_keyilm(struct DVE027_state_t *u_state)
{



	int err = 0;
	struct DVE027_state_t state;
	int type = LED_TYPE_FRONT;
	
#if defined(LED_DIAG_DEBUG_PLUS)
    printk(KERN_INFO "[leds_data_input]led_DVE029_keyilm\n");
#endif 
	
	err = copy_from_user( &state, u_state, sizeof(struct DVE027_state_t) );
	if( err ){
		printk(KERN_INFO "[leds_data_input]led_DVE029_keyilm state read NG\n");
		return -EINVAL;
	}
	
	led_loop_set( state.flashOnMS, type );
	
	
	if( state.flashMode == 2 ){				
		err = led_ilm_func_State[6][gs_led_state.ilmkey.main_state](&state, type);
	}
	else if( state.flashMode == 3 ){		
		err = led_ilm_func_State[7][gs_led_state.ilmkey.main_state](&state, type);
	}
	
#if defined(LED_DIAG_DEBUG_PLUS)
    printk(KERN_INFO "[leds_data_input]led_DVE029_keyilm OUT\n");
#endif 
	
    return err;

}
#endif 















































































































































































































































































static int led_DVE029_reflectilm(struct DVE027_state_t *u_state)
{
	int err = 0;
	struct DVE027_state_t state;
	int type = LED_TYPE_REFLECT;

#if defined(LED_DIAG_DEBUG_PLUS)
    printk(KERN_INFO "[leds_data_input]led_DVE029_reflectilm\n");
#endif 

	err = copy_from_user( &state, u_state, sizeof(struct DVE027_state_t) );
	if( err ){
		printk(KERN_INFO "[leds_data_input]led_DVE029_reflectilm state read NG\n");
		return -EINVAL;
	}

	led_loop_set( state.flashOnMS, type );

	
	if( state.flashMode == 2 ){				
		err = led_ilm_func_State[10][gs_led_state.ilmreflect.main_state](&state, type);
	}
	else if( state.flashMode == 3 ){		
		err = led_ilm_func_State[11][gs_led_state.ilmreflect.main_state](&state, type);
	}

#if defined(LED_DIAG_DEBUG_PLUS)
    printk(KERN_INFO "[leds_data_input]led_DVE029_reflectilm OUT\n");
#endif 

	return err;
}


#if defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)








static int led_lights_3led_file_ctrl(struct DVE027_state_t *u_state) {
	
	int err = 0;
	struct DVE027_state_t state;
	void *p_patern_data = NULL;
	int	patern_size = 0;
	
#if defined(LED_DIAG_DEBUG_PLUS)
    printk(KERN_INFO "[leds_data_input]led_lights_3led_file_ctrl IN\n");
#endif 

	err = copy_from_user( &state, u_state, sizeof(struct DVE027_state_t) );
	if( err )
	{
#if defined(LED_DIAG_DEBUG_PLUS)
		printk(KERN_INFO "[leds_data_input]led_lights_3led_file_ctrl state read NG\n");
#endif 
		return -EINVAL;
	}
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_lights_3led_file_ctrl = %d \n", state.flashMode );
#endif 

	switch(state.flashMode)
	{
	
	  case LED_TYPE_N_KEEP:
	  
		if( g_notifications_patern.p_patern != NULL )
		{
			kfree( g_notifications_patern.p_patern );
			g_notifications_patern.p_patern = NULL;
		}
		else
		{
			; 
		}
		
		if( gs_patern_data.incoming.s_ready_data.p_patern != NULL )
		{
			p_patern_data = gs_patern_data.incoming.s_ready_data.p_patern;
			patern_size = gs_patern_data.incoming.s_ready_data.s_data_info.data_size;
			memcpy( &g_notifications_patern.s_data_info, &gs_patern_data.incoming.s_ready_data.s_data_info, sizeof(input_data_head_type) );
		}
		else
		{
			p_patern_data = gs_patern_data.incoming.s_set_data.p_patern;
			patern_size = gs_patern_data.incoming.s_set_data.s_data_info.data_size;
			memcpy( &g_notifications_patern.s_data_info, &gs_patern_data.incoming.s_set_data.s_data_info, sizeof(input_data_head_type) );
		}
		
		if( p_patern_data != NULL )
		{
			g_notifications_patern.p_patern = kzalloc( patern_size, GFP_KERNEL );
			if( g_notifications_patern.p_patern == NULL )
			{
#if defined(LED_DIAG_DEBUG_PLUS)
				printk(KERN_INFO "[leds_data_input]led_lights_3led_file_ctrl kzalloc NG\n");
#endif 
				return -EINVAL;
			}
			else
			{
				; 
			}
			memcpy( g_notifications_patern.p_patern, p_patern_data, patern_size );
		}
		else
		{
			; 
		}
		
		break;
		
	  case LED_TYPE_N_SET:
		
		if( g_notifications_patern.p_patern != NULL )
		{
			
			p_patern_data = kzalloc( g_notifications_patern.s_data_info.data_size, GFP_KERNEL );
			
			if( p_patern_data == NULL )
			{
#if defined(LED_DIAG_DEBUG_PLUS)
				printk(KERN_INFO "[leds_data_input]led_lights_3led_file_ctrl kzalloc NG\n");
#endif 
				return -EINVAL;
			}
			else
			{
				; 
			}
			
			memcpy( p_patern_data, g_notifications_patern.p_patern, g_notifications_patern.s_data_info.data_size );
			
			if( gs_patern_data.incoming.s_set_data.p_patern == NULL )
			{
				gs_patern_data.incoming.s_set_data.p_patern = p_patern_data;
				memcpy( &gs_patern_data.incoming.s_set_data.s_data_info, &g_notifications_patern.s_data_info, sizeof(input_data_head_type) );
			}
			else
			{
				if( gs_patern_data.incoming.s_ready_data.p_patern != NULL )
				{
					kfree( gs_patern_data.incoming.s_ready_data.p_patern );
				}
				gs_patern_data.incoming.s_ready_data.p_patern = p_patern_data ;
				memcpy( &gs_patern_data.incoming.s_ready_data.s_data_info, &g_notifications_patern.s_data_info, sizeof(input_data_head_type) );
			}
			
			err = led_ilm_func_State[0][gs_led_state.ilm3led.main_state](&state, LED_TYPE_INCOMING);
		}
		else
		{
			; 
		}
		
		break;
		
	  case LED_TYPE_B_KEEP:

		if( g_battery_patern.p_patern != NULL )
		{
			kfree( g_battery_patern.p_patern );
			g_battery_patern.p_patern = NULL;
		}
		else
		{
			; 
		}

		if( gs_patern_data.incoming.s_ready_data.p_patern != NULL )
		{
			p_patern_data = gs_patern_data.incoming.s_ready_data.p_patern;
			patern_size = gs_patern_data.incoming.s_ready_data.s_data_info.data_size;
			memcpy( &g_battery_patern.s_data_info, &gs_patern_data.incoming.s_ready_data.s_data_info, sizeof(input_data_head_type) );
		}
		else
		{
			p_patern_data = gs_patern_data.incoming.s_set_data.p_patern;
			patern_size = gs_patern_data.incoming.s_set_data.s_data_info.data_size;
			memcpy( &g_battery_patern.s_data_info, &gs_patern_data.incoming.s_set_data.s_data_info, sizeof(input_data_head_type) );
		}
		
		if( p_patern_data != NULL )
		{
			g_battery_patern.p_patern = kzalloc( patern_size, GFP_KERNEL );
			if( g_battery_patern.p_patern == NULL )
			{
#if defined(LED_DIAG_DEBUG_PLUS)
				printk(KERN_INFO "[leds_data_input]led_lights_3led_file_ctrl kzalloc NG\n");
#endif 
				return -EINVAL;
			}
			else
			{
				; 
			}
			memcpy( g_battery_patern.p_patern, p_patern_data, patern_size );
		}
		else
		{
			; 
		}
		
		break;
		
	  case LED_TYPE_B_SET:
	  
		if( g_battery_patern.p_patern != NULL ){
			
			p_patern_data = kzalloc( g_battery_patern.s_data_info.data_size, GFP_KERNEL );
			
			if( p_patern_data == NULL )
			{
#if defined(LED_DIAG_DEBUG_PLUS)
				printk(KERN_INFO "[leds_data_input]led_lights_3led_file_ctrl kzalloc NG\n");
#endif 
				return -EINVAL;
			}
			else
			{
				; 
			}
			
			memcpy( p_patern_data, g_battery_patern.p_patern, g_battery_patern.s_data_info.data_size );
			
			if( gs_patern_data.incoming.s_set_data.p_patern == NULL )
			{
				gs_patern_data.incoming.s_set_data.p_patern = p_patern_data;
				memcpy( &gs_patern_data.incoming.s_set_data.s_data_info, &g_battery_patern.s_data_info, sizeof(input_data_head_type) );
			}
			else
			{
				if( gs_patern_data.incoming.s_ready_data.p_patern != NULL )
				{
					kfree( gs_patern_data.incoming.s_ready_data.p_patern );
				}
				else
				{
					; 
				}
				gs_patern_data.incoming.s_ready_data.p_patern = p_patern_data ;
				memcpy( &gs_patern_data.incoming.s_ready_data.s_data_info, &g_battery_patern.s_data_info, sizeof(input_data_head_type) );
			}
			
			err = led_ilm_func_State[0][gs_led_state.ilm3led.main_state](&state, LED_TYPE_INCOMING);
		}
		else
		{
			; 
		}

		break;
	
	  case LED_TYPE_SUB_SET:
		gs_patern_data.battery.data = (state.color & 0x00FFFFFF) ;
#if defined(LED_DIAG_DEBUG_PLUS)
		printk(KERN_INFO "[leds_data_input]state.color = %08x\n", state.color);
		printk(KERN_INFO "[leds_data_input]gs_patern_data.battery.data = %08x\n", gs_patern_data.battery.data);
#endif 


		switch(gs_led_state.ilm3led.main_state)
		{
		  case 	LED_STATE_STOP:
			break;
			
		  case 	LED_STATE_PATERN:
			break;
			
		  case 	LED_STATE_LOCAL_OPE:
			
			err = led_func_1_2_battery( &state, LED_TYPE_INCOMING );
			break;
			
		  case 	LED_STATE_SYNC_OPE_WAIT:
			break;
			
		  case 	LED_STATE_SYNC_OPE:
			break;
			
		  case 	LED_STATE_SYNC_OPE_CANCEL:
			break;
			
		  default:
			break;
		}


        break;
        
	  case LED_TYPE_SUB_CLR:
		gs_patern_data.battery.data = 0 ;
#if defined(LED_DIAG_DEBUG_PLUS)
		printk(KERN_INFO "[leds_data_input]state.color = %08x\n", state.color);
		printk(KERN_INFO "[leds_data_input]gs_patern_data.battery.data = %08x\n", gs_patern_data.battery.data);
#endif 


		switch(gs_led_state.ilm3led.main_state)
		{
		  case 	LED_STATE_STOP:
			break;
			
		  case 	LED_STATE_PATERN:
			break;
			
		  case 	LED_STATE_LOCAL_OPE:
			
			err = led_func_1_2_battery( &state, LED_TYPE_INCOMING );
			break;
			
		  case 	LED_STATE_SYNC_OPE_WAIT:
			break;
			
		  case 	LED_STATE_SYNC_OPE:
			break;
			
		  case 	LED_STATE_SYNC_OPE_CANCEL:
			break;
			
		  default:
			break;
		}


        break;
	
	  default:
        err = -EINVAL;
        break;
    }
	
    return err;
}
#endif 

#if defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)








static int __init led_data_input_init(void)
{
	int ret;
	
	printk(KERN_INFO "[leds_data_input]led_data_input_ini \n");
	
	mutex_init( &gs_patern_data.func_lock );
	
	wake_lock_init(&led_ilm_wake_lock, WAKE_LOCK_SUSPEND, "leds_data_input");
	
	memset( &gs_patern_data.incoming, 0, sizeof(gs_patern_data.incoming) );
	mutex_init( &gs_patern_data.incoming.data_lock );

	
	memset( &gs_patern_data.front, 0, sizeof(gs_patern_data.front) );
	mutex_init( &gs_patern_data.front.data_lock );

	










	memset( &g_notifications_patern, 0, sizeof(g_notifications_patern) );
	memset( &g_battery_patern, 0, sizeof(g_battery_patern) );

	gs_patern_data.battery.data = 0 ;

	init_waitqueue_head(&gs_led_wq);

	ret = misc_register(&led_lights_data_input);
	if( ret != 0 )
	{
		return ret;
	}
	else
	{
		; 
	}
	ret = misc_register(&led_DVE029_data_input);
	if( ret != 0 )
	{
		misc_deregister(&led_lights_data_input);
	}
	else
	{
		; 
	}


	ret = class_register(&earphone_insert_class);


	return ret;
}

#endif 

#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077)
#if !defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)








static int led_lights_3led_file_ctrl(struct DVE027_state_t *u_state) {
	
	int err = 0;
	struct DVE027_state_t state;
	void *p_patern_data = NULL;
	int	patern_size = 0;
	
#if defined(LED_DIAG_DEBUG_PLUS)
    printk(KERN_INFO "[leds_data_input]led_lights_3led_file_ctrl IN\n");
#endif 

	err = copy_from_user( &state, u_state, sizeof(struct DVE027_state_t) );
	if( err ){
#if defined(LED_DIAG_DEBUG_PLUS)
		printk(KERN_INFO "[leds_data_input]led_lights_3led_file_ctrl state read NG\n");
#endif 
		return -EINVAL;
	}
	
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_lights_3led_file_ctrl = %d \n", state.flashMode );
#endif 

	switch(state.flashMode){
	
	  case LED_TYPE_N_KEEP:
	  
		if( g_notifications_patern.p_patern != NULL ){
			kfree( g_notifications_patern.p_patern );
			g_notifications_patern.p_patern = NULL;
		}
		
		if( gs_patern_data.incoming.s_ready_data.p_patern != NULL ) {
			p_patern_data = gs_patern_data.incoming.s_ready_data.p_patern;
			patern_size = gs_patern_data.incoming.s_ready_data.s_data_info.data_size;
			memcpy( &g_notifications_patern.s_data_info, &gs_patern_data.incoming.s_ready_data.s_data_info, sizeof(input_data_head_type) );
		}
		else
		{
			p_patern_data = gs_patern_data.incoming.s_set_data.p_patern;
			patern_size = gs_patern_data.incoming.s_set_data.s_data_info.data_size;
			memcpy( &g_notifications_patern.s_data_info, &gs_patern_data.incoming.s_set_data.s_data_info, sizeof(input_data_head_type) );
		}
		
		if( p_patern_data != NULL ){
			g_notifications_patern.p_patern = kzalloc( patern_size, GFP_KERNEL );
			if( g_notifications_patern.p_patern == NULL ){
#if defined(LED_DIAG_DEBUG_PLUS)
				printk(KERN_INFO "[leds_data_input]led_lights_3led_file_ctrl kzalloc NG\n");
#endif 
				return -EINVAL;
			}
			memcpy( g_notifications_patern.p_patern, p_patern_data, patern_size );
		}
		
		break;
		
	  case LED_TYPE_N_SET:
		
		if( g_notifications_patern.p_patern != NULL ){
			
			p_patern_data = kzalloc( g_notifications_patern.s_data_info.data_size, GFP_KERNEL );
			
			if( p_patern_data == NULL ){
#if defined(LED_DIAG_DEBUG_PLUS)
				printk(KERN_INFO "[leds_data_input]led_lights_3led_file_ctrl kzalloc NG\n");
#endif 
				return -EINVAL;
			}
			
			memcpy( p_patern_data, g_notifications_patern.p_patern, g_notifications_patern.s_data_info.data_size );
			
			if( gs_patern_data.incoming.s_set_data.p_patern == NULL ){
				gs_patern_data.incoming.s_set_data.p_patern = p_patern_data;
				memcpy( &gs_patern_data.incoming.s_set_data.s_data_info, &g_notifications_patern.s_data_info, sizeof(input_data_head_type) );
			}
			else{
				if( gs_patern_data.incoming.s_ready_data.p_patern != NULL ){
					kfree( gs_patern_data.incoming.s_ready_data.p_patern );
				}
				gs_patern_data.incoming.s_ready_data.p_patern = p_patern_data ;
				memcpy( &gs_patern_data.incoming.s_ready_data.s_data_info, &g_notifications_patern.s_data_info, sizeof(input_data_head_type) );
			}
			
			err = led_ilm_func_State[0][gs_led_state.ilm3led.main_state](&state, LED_TYPE_INCOMING);
		}
		
		break;
		
	  case LED_TYPE_B_KEEP:

		if( g_battery_patern.p_patern != NULL ){
			kfree( g_battery_patern.p_patern );
			g_battery_patern.p_patern = NULL;
		}

		if( gs_patern_data.incoming.s_ready_data.p_patern != NULL ) {
			p_patern_data = gs_patern_data.incoming.s_ready_data.p_patern;
			patern_size = gs_patern_data.incoming.s_ready_data.s_data_info.data_size;
			memcpy( &g_battery_patern.s_data_info, &gs_patern_data.incoming.s_ready_data.s_data_info, sizeof(input_data_head_type) );
		}
		else
		{
			p_patern_data = gs_patern_data.incoming.s_set_data.p_patern;
			patern_size = gs_patern_data.incoming.s_set_data.s_data_info.data_size;
			memcpy( &g_battery_patern.s_data_info, &gs_patern_data.incoming.s_set_data.s_data_info, sizeof(input_data_head_type) );
		}
		
		if( p_patern_data != NULL ){
			g_battery_patern.p_patern = kzalloc( patern_size, GFP_KERNEL );
			if( g_battery_patern.p_patern == NULL ){
#if defined(LED_DIAG_DEBUG_PLUS)
				printk(KERN_INFO "[leds_data_input]led_lights_3led_file_ctrl kzalloc NG\n");
#endif 
				return -EINVAL;
			}
			memcpy( g_battery_patern.p_patern, p_patern_data, patern_size );
		}
		
		break;
		
	  case LED_TYPE_B_SET:
	  
		if( g_battery_patern.p_patern != NULL ){
			
			p_patern_data = kzalloc( g_battery_patern.s_data_info.data_size, GFP_KERNEL );
			
			if( p_patern_data == NULL ){
#if defined(LED_DIAG_DEBUG_PLUS)
				printk(KERN_INFO "[leds_data_input]led_lights_3led_file_ctrl kzalloc NG\n");
#endif 
				return -EINVAL;
			}
			
			memcpy( p_patern_data, g_battery_patern.p_patern, g_battery_patern.s_data_info.data_size );
			
			if( gs_patern_data.incoming.s_set_data.p_patern == NULL ){
				gs_patern_data.incoming.s_set_data.p_patern = p_patern_data;
				memcpy( &gs_patern_data.incoming.s_set_data.s_data_info, &g_battery_patern.s_data_info, sizeof(input_data_head_type) );
			}
			else{
				if( gs_patern_data.incoming.s_ready_data.p_patern != NULL ){
					kfree( gs_patern_data.incoming.s_ready_data.p_patern );
				}
				gs_patern_data.incoming.s_ready_data.p_patern = p_patern_data ;
				memcpy( &gs_patern_data.incoming.s_ready_data.s_data_info, &g_battery_patern.s_data_info, sizeof(input_data_head_type) );
			}
			
			err = led_ilm_func_State[0][gs_led_state.ilm3led.main_state](&state, LED_TYPE_INCOMING);
		}

		break;
	
	  default:
        err = -EINVAL;
        break;
    }
	
    return err;
}
#endif 
#endif 








#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077)
static long led_data_input_ioctl(struct file *file, unsigned int iocmd, unsigned long data)
#else 
static int led_data_input_ioctl(struct inode *inode, struct file *file,
    unsigned int iocmd, unsigned long data)
#endif 
{
	int ret = 0;
	struct input_data_head_struct *sp_head_data;
	


#if defined(LED_DIAG_DEBUG_PLUS)
    printk(KERN_INFO "[leds_data_input]led_data_input_ioctl IN iocmd = %d ilm3led_state = %d ilmkey_state = %d \n", iocmd, gs_led_state.ilm3led.main_state, gs_led_state.ilmkey.main_state);
#endif
    








    switch(iocmd){
	  case LED_IOC_LIGHTS_3LED:
		mutex_lock( &gs_patern_data.func_lock );
		ret = led_lights_3led((struct DVE027_state_t *)data);
		mutex_unlock( &gs_patern_data.func_lock );
		break;
		


#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
	  case LED_IOC_DVE028_KEYILM:
		mutex_lock( &gs_patern_data.func_lock );
		ret = led_DVE029_keyilm((struct DVE027_state_t *)data);
		mutex_unlock( &gs_patern_data.func_lock );
		break;
#endif 

		
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077)
	  case LED_IOC_FILE_CTRL:
		mutex_lock( &gs_patern_data.func_lock );
		ret = led_lights_3led_file_ctrl((struct DVE027_state_t *)data);
		mutex_unlock( &gs_patern_data.func_lock );
		break;
#endif 















	  case LED_IOC_LIGHTS_REFLECTILM:
		mutex_lock( &gs_patern_data.func_lock );
		ret = led_DVE029_reflectilm((struct DVE027_state_t *)data);
		mutex_unlock( &gs_patern_data.func_lock );
		break;

		
	  case LED_IOC_DATA_SET:
		sp_head_data = (struct input_data_head_struct *)data;
		if(sp_head_data->led_type == LED_TYPE_INCOMING ){
			mutex_lock( &gs_patern_data.func_lock );
			ret = led_patern_data_read( (unsigned char*)data );
			mutex_unlock( &gs_patern_data.func_lock );
		}


#if defined(LOCAL_FEATURE_DVE021_KEY_BACKLIGHT_USES)
		else if(sp_head_data->led_type == LED_TYPE_FRONT ){
			mutex_lock( &gs_patern_data.func_lock );
			ret = led_patern_data_read( (unsigned char*)data );
			mutex_unlock( &gs_patern_data.func_lock );
		}
#endif 


		else if(sp_head_data->led_type == LED_TYPE_REFLECT ){
			mutex_lock( &gs_patern_data.func_lock );
			ret = led_patern_data_read( (unsigned char*)data );
			mutex_unlock( &gs_patern_data.func_lock );
		}













        break;

	  default:
        ret = -EINVAL;
        break;
    }
	

    



	

#if defined(LED_DIAG_DEBUG_PLUS)
    printk(KERN_DEBUG "[leds_data_input]led_data_input_ioctl OUT ret = %d ilm3led_state = %d ilmkey_state = %d \n", ret, gs_led_state.ilm3led.main_state, gs_led_state.ilmkey.main_state);
#endif
   
    return ret;
}









static int led_data_input_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret = 0;
	unsigned long size;
	
	size = vma->vm_end - vma->vm_start ;
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_data_input_mmap IN size = %ld PAGE_SIZE = %ld\n", size, PAGE_SIZE );
#endif 
	ret = remap_pfn_range( vma, vma->vm_start, vma->vm_pgoff, size, vma->vm_page_prot );
	
	if( ret ){
		return -EAGAIN;
	}
	
	vma->vm_ops = &led_remap_vm_ops;
	
	led_vma_open(vma);
#if defined(LED_DIAG_DEBUG_PLUS)
	printk(KERN_INFO "[leds_data_input]led_data_input_mmap OUT\n");
#endif 
	return 0;
}

static const struct file_operations led_data_input_fops = {
    .owner      = THIS_MODULE,
    .open       = led_data_input_open,
#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077)
    .unlocked_ioctl = led_data_input_ioctl,
#else 
    .ioctl      = led_data_input_ioctl,
#endif 
    .mmap       = led_data_input_mmap,
};

static struct miscdevice led_lights_data_input = {
    .fops       = &led_data_input_fops,
    .name       = LED_LIGHTS_INPUT_NAME,
    .minor      = MISC_DYNAMIC_MINOR,
};

static struct miscdevice led_DVE029_data_input = {
    .fops       = &led_data_input_fops,
    .name       = LED_DVE028_INPUT_NAME,
    .minor      = MISC_DYNAMIC_MINOR,
};

#if !defined(LOCAL_CONFIG_FEATURE_DVE021_DVE902)








static int __init led_data_input_init(void)
{
	int ret;
	
	printk(KERN_INFO "[leds_data_input]led_data_input_ini \n");
	
	mutex_init( &gs_patern_data.func_lock );
	
	wake_lock_init(&led_ilm_wake_lock, WAKE_LOCK_SUSPEND, "leds_data_input");
	
	memset( &gs_patern_data.incoming, 0, sizeof(gs_patern_data.incoming) );
	mutex_init( &gs_patern_data.incoming.data_lock );

	
	memset( &gs_patern_data.front, 0, sizeof(gs_patern_data.front) );
	mutex_init( &gs_patern_data.front.data_lock );

	










#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077)
	memset( &g_notifications_patern, 0, sizeof(g_notifications_patern) );
	memset( &g_battery_patern, 0, sizeof(g_battery_patern) );
#endif 


	memset( &gs_patern_data.reflect, 0, sizeof(gs_patern_data.reflect) );
	mutex_init( &gs_patern_data.reflect.data_lock );


	init_waitqueue_head(&gs_led_wq);

	ret = misc_register(&led_lights_data_input);
	if( ret != 0 ){
		return ret;
	}
	ret = misc_register(&led_DVE029_data_input);
	if( ret != 0 ){
		misc_deregister(&led_lights_data_input);
	}
	return ret;
}
#endif 









static void __exit led_data_input_exit(void)
{
	
	
	misc_deregister(&led_lights_data_input);
	misc_deregister(&led_DVE029_data_input);
	
	if( gs_patern_data.incoming.ps_thread != NULL ){
		kthread_stop( gs_patern_data.incoming.ps_thread );
	}
	
	if( gs_patern_data.front.ps_thread != NULL ){
		kthread_stop( gs_patern_data.front.ps_thread );
	}
	
	if( gs_patern_data.incoming.s_set_data.p_patern != NULL ){
		kfree( gs_patern_data.incoming.s_set_data.p_patern );
	}
	
	if( gs_patern_data.incoming.s_ready_data.p_patern != NULL ){
		kfree( gs_patern_data.incoming.s_ready_data.p_patern );
	}
	
	if( gs_patern_data.front.s_set_data.p_patern != NULL ){
		kfree( gs_patern_data.front.s_set_data.p_patern );
	}
	
	if( gs_patern_data.front.s_ready_data.p_patern != NULL ){
		kfree( gs_patern_data.front.s_ready_data.p_patern );
	}
























#if defined(LOCAL_CONFIG_FEATURE_DVE021_LED_DVE077)
	if( g_notifications_patern.p_patern != NULL ){
		kfree( g_notifications_patern.p_patern );
	}
	
	if( g_battery_patern.p_patern != NULL ){
		kfree( g_battery_patern.p_patern );
	}
#endif 

	if( gs_patern_data.reflect.s_set_data.p_patern != NULL ){
		kfree( gs_patern_data.reflect.s_set_data.p_patern );
	}
	
	if( gs_patern_data.reflect.s_ready_data.p_patern != NULL ){
		kfree( gs_patern_data.reflect.s_ready_data.p_patern );
	}

	
	wake_lock_destroy(&led_ilm_wake_lock);
	
	return;
}

module_init(led_data_input_init);
module_exit(led_data_input_exit);



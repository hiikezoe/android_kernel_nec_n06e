/*
 * leds-adp8861.h - platform data structure for adp8861 led controller
 *
 * Copyright (C) 2011 NEC NEC Corporation
 *
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/
 
#if !defined(__LINUX_ADP8861_H)
#define __LINUX_ADP8861_H

#include <linux/leds.h>



#define ADP8861_LED_SET_OK	0
#define ADP8861_LED_SET_NG	(-1)
#define ADP8861_LED_SET_RESERVED	(1)

#define ADP8861_LED_REG_WRITE_NG	(0x80)

#define ADP8861_LED_ON		(1)
#define ADP8861_LED_OFF		(0)


struct leds_adp8861_platform_data {
	int (*poweron)(struct device *);
	int (*poweroff)(struct device *);
};


struct led_mfdvid_reg{
	unsigned char	bit0:1			;
	unsigned char	bit1:1			;
	unsigned char	bit2:1			;
	unsigned char	bit3:1			;
	unsigned char	bit4:1			;
	unsigned char	bit5:1			;
	unsigned char	bit6:1			;
	unsigned char	bit7:1			;
};
union u_led_mfdvid_reg{
	struct led_mfdvid_reg	st;
	u8						uc;
};


struct led_mdcr_reg{
	unsigned char	bl_en:1			;
	unsigned char	dmy1:1			;
	unsigned char	sis_en:1		;
	unsigned char	gdwn_dis:1		;
	unsigned char	dim_en:1		;
	unsigned char	nstby:1			;
	unsigned char	int_cfg:1		;
	unsigned char	dmy2:1			;
};
union u_led_mdcr_reg{
	struct led_mdcr_reg		st;
	u8						uc;
};


struct led_mdcr2_reg{
	unsigned char	dmy1:2			;
	unsigned char	ovp_int:1		;
	unsigned char	tsd_int:1		;
	unsigned char	short_int:1		;
	unsigned char	dmy2:3			;
};
union u_led_mdcr2_reg{
	struct led_mdcr2_reg	st;
	u8						uc;
};


struct led_intr_en_reg{
	unsigned char	dmy1:2			;
	unsigned char	ovp_ien:1		;
	unsigned char	tsd_ien:1		;
	unsigned char	short_ien:1		;
	unsigned char	dmy2:3			;
};
union u_led_intr_en2_reg{
	struct led_intr_en_reg	st;
	u8						uc;
};


struct led_cfgr_reg_1{
	unsigned char	fovr:1			;
	unsigned char	law_0:1			;
	unsigned char	law_1:1			;
	unsigned char	dmy1:5			;
};
struct led_cfgr_reg_2{
	unsigned char	fovr:1			;
	unsigned char	law:2			;
	unsigned char	dmy1:5			;
};
union u_led_cfgr_reg{
	struct led_cfgr_reg_1	st1;
	struct led_cfgr_reg_2	st2;
	u8						uc;
};


struct led_blsen_reg{
	unsigned char	d1en:1			;
	unsigned char	d2en:1			;
	unsigned char	d3en:1			;
	unsigned char	d4en:1			;
	unsigned char	d5en:1			;
	unsigned char	d6en:1			;
	unsigned char	d7en:1			;
	unsigned char	dmy1:1			;
};
union u_led_blsen_reg{
	struct led_blsen_reg	st;
	u8						uc;
};


struct led_bloff_reg{
	unsigned char	offt_1:1		;
	unsigned char	offt_2:1		;
	unsigned char	offt_3:1		;
	unsigned char	offt_4:1		;
	unsigned char	offt_5:1		;
	unsigned char	offt_6:1		;
	unsigned char	offt_7:1		;
	unsigned char	dmy1:1			;
};
union u_led_bloff_reg{
	struct led_bloff_reg	st;
	u8						uc;
};














struct led_bldim_reg_2{
	unsigned char	dimt:7			;
	unsigned char	dmy1:1			;
};
union u_led_bldim_reg{

	struct led_bldim_reg_2	st2;
	u8						uc;
};














struct led_blfr_reg_2{
	unsigned char	bl_fi:4			;
	unsigned char	bl_fo:4			;
};
union u_led_blfr_reg{

	struct led_blfr_reg_2	st2;
	u8						uc;
};














union u_led_blmx_reg{

	unsigned char			bl_mc;
	u8						uc;
};














struct led_bldm_reg_2{
	unsigned char	bl_dc:7			;
	unsigned char	dmy1:1			;
};
union u_led_bldm_reg{

	struct led_bldm_reg_2	st2;
	u8						uc;
};









struct led_iscfr_reg_2{
	unsigned char	sc_law:2		;
	unsigned char	dmy1:6			;
};
union u_led_iscfr_reg{

	struct led_iscfr_reg_2	st2;
	u8						uc;
};


struct led_iscc_reg{
	unsigned char	sc1_en:1		;
	unsigned char	sc2_en:1		;
	unsigned char	sc3_en:1		;
	unsigned char	sc4_en:1		;
	unsigned char	sc5_en:1		;
	unsigned char	sc6_en:1		;
	unsigned char	sc7_en:1		;
	unsigned char	dmy1:1			;
};
union u_led_iscc_reg{
	struct led_iscc_reg		st;
	u8						uc;
};














struct led_isct1_reg_2{
	unsigned char	sc5_off:2		;
	unsigned char	sc6_off:2		;
	unsigned char	sc7_off:2		;
	unsigned char	scon:2			;
};
union u_led_isct1_reg{

	struct led_isct1_reg_2		st2;
	u8							uc;
};














struct led_isct2_reg_2{
	unsigned char	sc1_off:2		;
	unsigned char	sc2_off:2		;
	unsigned char	sc3_off:2		;
	unsigned char	sc4_off:2		;
};
union u_led_isct2_reg{

	struct led_isct2_reg_2	st2;
	u8						uc;
};














struct led_iscf_reg_2{
	unsigned char	scfi:4			;
	unsigned char	scfo:4			;
};
union u_led_iscf_reg{

	struct led_iscf_reg_2	st2;
	u8						uc;
};

















struct led_isc_reg_2{
	unsigned short	scd:7			;
	unsigned short	scr:1			;
	
	unsigned short	set_flag:1		;
	unsigned short	dmy1:7			;
};
struct led_isc_reg_3{
	unsigned char	uc1;
	unsigned char	uc2;
};
union u_led_isc_reg{

	struct led_isc_reg_2	st2;
	struct led_isc_reg_3	st3;
	unsigned short			us;
};

struct led_request{
	unsigned char	set:1			;
	unsigned short	flash_flag:1	;
	unsigned char	dmy1:6			;
};
struct led_request_rgb{
	unsigned char	set_r:1			;
	unsigned char	set_g:1			;
	unsigned char	set_b:1			;
	unsigned char	dmy1:5			;
};
struct led_request_key{
	unsigned char	set_1:1			;
	unsigned char	dmy1:7			;
};
struct led_request_prevent_peeping{
	unsigned char	set_1:1			;
	unsigned char	dmy1:7			;
};




#define LEDS_CMD_TYPE_RGB_RED		0x01
#define LEDS_CMD_TYPE_RGB_GREEN		0x02
#define LEDS_CMD_TYPE_RGB_BLUE		0x03
#define LEDS_CMD_TYPE_KEY			0x04
#define LEDS_CMD_TYPE_FLASH_MOVIE	0x05
#define LEDS_CMD_TYPE_FLASH_STILL	0x06
#define LEDS_CMD_TYPE_FLASH_TORCH	0x07
#define LEDS_CMD_TYPE_PREVENT_PEEPING	0x08

#define LEDS_CMD_TYPE_ER_RGB_RED		0x09
#define LEDS_CMD_TYPE_ER_RGB_GREEN		0x0A
#define LEDS_CMD_TYPE_ER_RGB_BLUE		0x0B



#define LEDS_CMD_RET_OK				1
#define LEDS_CMD_RET_NG				0

#define LEDS_LED_ILLU_RED_LUMIN1		0x1E
#define LEDS_LED_ILLU_RED_LUMIN2		0x12
#define LEDS_LED_ILLU_RED_LUMIN3		0x1A
#define LEDS_LED_ILLU_GREEN_LUMIN1		0x1E
#define LEDS_LED_ILLU_GREEN_LUMIN2		0x12
#define LEDS_LED_ILLU_GREEN_LUMIN3		0x11
#define LEDS_LED_ILLU_BLUE_LUMIN1		0x1E
#define LEDS_LED_ILLU_BLUE_LUMIN2		0x12
#define LEDS_LED_ILLU_BLUE_LUMIN3		0x0D


#define LEDS_LED_KEYBL1					LEDS_LED_ILLU_RED_LUMIN3
#define LEDS_LED_KEYBL2					LEDS_LED_ILLU_GREEN_LUMIN3
#define LEDS_LED_KEYBL3					LEDS_LED_ILLU_BLUE_LUMIN3


#define LEDS_SND_CAM_FLASH_LED1			0x53
#define LEDS_SND_CAM_FLASH_LED2			0x08
#define LEDS_SND_CAM_FLASH_LED3			0x33

#define LEDS_SND_CAM_SUB_FLASH_LED1		0x2A
#define LEDS_SND_CAM_SUB_FLASH_LED2		0x2A
#define LEDS_SND_CAM_SUB_FLASH_LED3		0x2A
















#define LEDS_LED_CAM_IND				0x15


#define ADP8861_IOCTL_MAGIC 'l'

#define LED_CLASS_WRAPPER_FD_PATH    "/dev/led_class_wrapper"

struct adp8861_led_flash_parame {
	char still;
	char video;
	char torch;
};

struct adp8861_led_prevent_peeping_parame {
	char prevent_peeping ;
};



enum device_id_num{
	ID_0,
	ID_1,
	ID_MAX,
};

struct adp8861_3color_led_parame {
	char one;
	char two;
	char three;
};

typedef struct adp8861_3color_led_parame_nv {
	struct adp8861_3color_led_parame illu_red_lumin;
	struct adp8861_3color_led_parame illu_green_lumin;
	struct adp8861_3color_led_parame illu_blue_lumin;
	struct adp8861_3color_led_parame keybl1_red_lumin;
	struct adp8861_3color_led_parame keybl1_green_lumin;
	struct adp8861_3color_led_parame keybl1_blue_lumin;
	struct adp8861_3color_led_parame keybl2_red_lumin;
	struct adp8861_3color_led_parame keybl2_green_lumin;
	struct adp8861_3color_led_parame keybl2_blue_lumin;
	struct adp8861_3color_led_parame hsj_red_lumin;
	struct adp8861_3color_led_parame hsj_green_lumin;
	struct adp8861_3color_led_parame hsj_blue_lumin;
}adp8861_3color_led_parame_nv_t;

#define INSERT_INIT -1
#define INSERT_FALSE 0
#define INSERT_TRUE 1



#define ADP8861_CUSTOM_N40    _IOW(ADP8861_IOCTL_MAGIC, 0x40, struct adp8861_led_flash_parame *)
#define ADP8861_CUSTOM_N43    _IOW(ADP8861_IOCTL_MAGIC, 0x43, struct adp8861_led_prevent_peeping_parame *)
#define ADP8861_CUSTOM_N44    _IOW(ADP8861_IOCTL_MAGIC, 0x44, struct adp8861_led_flash_parame *)

#define ADP8861_CUSTOM_N45    _IOW(ADP8861_IOCTL_MAGIC, 0x45, struct adp8861_3color_led_parame *)
#define ADP8861_CUSTOM_N46    _IOW(ADP8861_IOCTL_MAGIC, 0x46, struct adp8861_3color_led_parame *)
#define ADP8861_CUSTOM_N47    _IOW(ADP8861_IOCTL_MAGIC, 0x51, struct adp8861_3color_led_parame *)
#define ADP8861_CUSTOM_N48    _IOW(ADP8861_IOCTL_MAGIC, 0x47, struct adp8861_3color_led_parame *)
#define ADP8861_CUSTOM_N49    _IOW(ADP8861_IOCTL_MAGIC, 0x48, struct adp8861_3color_led_parame *)
#define ADP8861_CUSTOM_N50    _IOW(ADP8861_IOCTL_MAGIC, 0x49, struct adp8861_3color_led_parame *)
#define ADP8861_CUSTOM_N51    _IOW(ADP8861_IOCTL_MAGIC, 0x4A, struct adp8861_3color_led_parame *)
#define ADP8861_CUSTOM_N52    _IOW(ADP8861_IOCTL_MAGIC, 0x4B, struct adp8861_3color_led_parame *)
#define ADP8861_CUSTOM_N53    _IOW(ADP8861_IOCTL_MAGIC, 0x4C, struct adp8861_3color_led_parame *)
#define ADP8861_CUSTOM_N54    _IOW(ADP8861_IOCTL_MAGIC, 0x4D, struct adp8861_3color_led_parame *)
#define ADP8861_CUSTOM_N55    _IOW(ADP8861_IOCTL_MAGIC, 0x4F, struct adp8861_3color_led_parame *)
#define ADP8861_CUSTOM_N56    _IOW(ADP8861_IOCTL_MAGIC, 0x50, struct adp8861_3color_led_parame *)


extern int adp8861_rgb_led_bright( union u_led_isc_reg *red_bright,
								   union u_led_isc_reg *green_bright,
								   union u_led_isc_reg *blue_bright);
extern int adp8861_rgb_led_set(struct led_request_rgb *request);




extern int adp8861_left_key_led_bright( union u_led_isc_reg *led_key_bright1,union u_led_isc_reg *led_key_bright2,union u_led_isc_reg *led_key_bright3);
extern int adp8861_left_key_led_set(struct led_request_rgb *request);
extern int adp8861_right_key_led_bright( union u_led_isc_reg *led_key_bright1,union u_led_isc_reg *led_key_bright2,union u_led_isc_reg *led_key_bright3);
extern int adp8861_right_key_led_set(struct led_request_rgb *request);




extern int adp8861_prevent_peeping_led_set(struct led_request_prevent_peeping *request);
extern int adp8861_prevent_peeping_led_bright( union u_led_isc_reg *led_key_bright1);

extern int adp8861_get_3color_nv(struct adp8861_3color_led_parame_nv *three_color_led_parame);
extern int adp8861_er_rgb_led_bright( union u_led_isc_reg *red_bright,
								   union u_led_isc_reg *green_bright,
								   union u_led_isc_reg *blue_bright);
extern int adp8861_er_rgb_led_set(struct led_request_rgb *request);
extern int adp8861_led_fade_set( union u_led_iscf_reg *led_fade, enum device_id_num id_num );
extern int adp8861_led_timer_set1( union u_led_isct1_reg *led_timer_set,
								   union u_led_isct1_reg *led_timer_data, enum device_id_num id_num );
extern int adp8861_led_timer_set2( union u_led_isct2_reg *led_timer_set,
								   union u_led_isct2_reg *led_timer_data, enum device_id_num id_num );
extern int adp8861_flash_led_bright( union u_led_isc_reg *led_flash_bright, enum device_id_num id_num );
extern int adp8861_flash_led_set(struct led_request *request, enum device_id_num id_num);
extern int adp8861_reg_write( unsigned char sub2, unsigned char inf1, enum device_id_num id_num );
extern int adp8861_i2c_smbus_read(u8 command, u8 *value, enum device_id_num id_num);












extern void led_brightness_set_keyilm_status( int status );

#endif 



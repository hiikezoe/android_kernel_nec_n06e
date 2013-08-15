/*
 * leds-lm3532.h - platform data structure for lm3532 led controller
 *
 * Copyright (C) NEC CASIO Mobile Communications, Ltd.
 *
 */
 
#if !defined(__LINUX_LM3532_H)
#define __LINUX_LM3532_H

#include <linux/leds.h>



#define LM3532_LED_SET_OK	0
#define LM3532_LED_SET_NG	(-1)
#define LM3532_LED_SET_RESERVED	(1)

#define LM3532_LED_REG_WRITE_NG	(0x80)

#define LM3532_LED_ON		(1)
#define LM3532_LED_OFF		(0)




struct led_master_en_reg{
	unsigned char	softw_rst:1	;
	unsigned char	group_B_en:1;
	unsigned char	group_A_en:1;
	unsigned char	dmy1:5		;
};
union u_led_master_en_reg{
	struct led_master_en_reg	st;
	u8							uc;
};


struct led_diode_en_reg{
	unsigned char	enD1:1		;
	unsigned char	enD2:1		;
	unsigned char	enD3:1		;
	unsigned char	enD4:1		;
	unsigned char	enD5:1		;
	unsigned char	enD6:1		;
	unsigned char	enD7:1		;
	unsigned char	enD8:1		;
};
union u_led_diode_en_reg{
	struct led_diode_en_reg		st;
	u8							uc;
};


struct led_config_reg{
	unsigned char	pwm_en:1	;
	unsigned char	Pwm_p:1		;
	unsigned char	D5_A:1		;
	unsigned char	D6_A:1		;
	unsigned char	D7_A:1		;
	unsigned char	D8_A:1		;
	unsigned char	lin:1		;
	unsigned char	D7_int:1	;
};
union u_led_config_reg{
	struct led_config_reg		st;
	u8							uc;
};


struct led_options_reg1{
	unsigned char	ru0:1		;
	unsigned char	ru1:1		;
	unsigned char	ru2:1		;
	unsigned char	rd0:1		;
	unsigned char	rd1:1		;
	unsigned char	rd2:1		;
	unsigned char	gt0:1		;
	unsigned char	gt1:1		;
};
struct led_options_reg2{
	unsigned char	ru:3		;
	unsigned char	rd:3		;
	unsigned char	gt:2		;
};
union u_led_led_options_reg{
	struct led_options_reg1		st1;
	struct led_options_reg2		st2;
	u8							uc;
};


struct led_group_a_bri_reg{
	unsigned char	dxa:7		;
	unsigned char	dmy1:1		;
};
union u_led_group_a_bri_reg{
	struct led_group_a_bri_reg	st;
	u8							uc;
};



extern int led_main_lcd_set(unsigned char request, unsigned char pwm);
extern int led_main_lcd_bright (unsigned char lcd_bright);

extern int lm3532_i2c_smbus_write(u8 command, u8 value);
extern int lm3532_i2c_smbus_read(u8 command, u8 *value);
extern int lm3532_main_lcd_don(unsigned char sub2);







#endif 



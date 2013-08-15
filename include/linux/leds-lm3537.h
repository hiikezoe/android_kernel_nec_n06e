/*
 * leds-lm3537.h - platform data structure for lm3537 led controller
 *
 * Copyright (C) NEC CASIO Mobile Communications, Ltd.
 *
 */
 
#if !defined(__LINUX_LM3537_H)
#define __LINUX_LM3537_H

#include <linux/leds.h>



#define LM3537_LED_SET_OK	0
#define LM3537_LED_SET_NG	(-1)
#define LM3537_LED_SET_RESERVED	(1)

#define LM3537_LED_REG_WRITE_NG	(0x80)

#define LM3537_LED_ON		(1)
#define LM3537_LED_OFF		(0)



struct leds_lm3537_platform_data {
	int (*poweron)(struct device *);
	int (*poweroff)(struct device *);
};


extern int led_main_lcd_set(unsigned char request, unsigned char pwm);
extern int led_main_lcd_bright (unsigned char lcd_bright);

extern int lm3537_i2c_smbus_write(u8 command, u8 value);
extern int lm3537_i2c_smbus_read(u8 command, u8 *value);
extern int lm3537_main_lcd_don(unsigned char sub2);







#endif 



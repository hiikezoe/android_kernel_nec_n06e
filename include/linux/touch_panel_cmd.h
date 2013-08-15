/*
 * include/linux/touch_pamel_cmd.h
 *
 * Copyright (C) NEC CASIO Mobile Communications, Ltd.
 *
 */

#if !defined(__LINUX_TOUCH_PANEL_CMD_H)
#define __LINUX_TOUCH_PANEL_CMD_H

typedef struct
{
	unsigned char	reg0;	
	unsigned char	reg1;	
	unsigned char	reg2;	
	unsigned char	reg3;	
	unsigned char	reg4;	
	unsigned char	reg5;	
	unsigned char	reg6;	
	unsigned char	reg7;	
	unsigned char	reg8;	
} touch_panel_cmd_callback_param_coord;

typedef struct{
    unsigned char finger_data_x[2];
    unsigned char finger_data_y[2];
} finger_data;

#define TOUCH_PANEL_FINGER_NUM_MAX				10

typedef struct {
    unsigned char ret;
    finger_data finger[TOUCH_PANEL_FINGER_NUM_MAX];
} touch_diag_result;

#define TOUCH_PANEL_CMD_TYPE_RESET				0x01
#define TOUCH_PANEL_CMD_TYPE_ENTR_SLEEP			0x02
#define TOUCH_PANEL_CMD_TYPE_EXIT_SLEEP			0x03
#define TOUCH_PANEL_CMD_TYPE_ENTR_DEEP			0x04
#define TOUCH_PANEL_CMD_TYPE_EXIT_DEEP			0x05
#define TOUCH_PANEL_CMD_TYPE_GET_REVISION		0x06
#define TOUCH_PANEL_CMD_TYPE_GET_COORD			0x07
#define TOUCH_PANEL_CMD_TYPE_START_TEST			0x08
#define TOUCH_PANEL_CMD_TYPE_GET_TEST_RSLT		0x09
#define TOUCH_PANEL_CMD_TYPE_FW_UPDATE			0x0a
#define TOUCH_PANEL_CMD_TYPE_GET_CHECKSUM		0x0b
#define TOUCH_PANEL_CMD_TYPE_LINE_TEST			0x0c
#define TOUCH_PANEL_CMD_TYPE_POWER_ON			0x0d	
#define TOUCH_PANEL_CMD_TYPE_POWER_OFF			0x0e	
#define TOUCH_PANEL_CMD_TYPE_GLOVE_ON			0x0f	
#define TOUCH_PANEL_CMD_TYPE_GLOVE_OFF			0x10	
#define TOUCH_PANEL_CMD_TYPE_GET_FW_REVISION	0x11	
#define TOUCH_PANEL_CMD_TYPE_WRITE_SPI			0x12
#define TOUCH_PANEL_CMD_TYPE_READ_SPI			0x13
#define TOUCH_PANEL_CMD_TYPE_GET_FW_CONFIG_ID	0x14	
#define TOUCH_PANEL_CMD_TYPE_GET_CONFIG_ID		0x15	

extern unsigned char touch_panel_cmd(unsigned char type, void *val);
extern unsigned char touch_panel_cmd_callback(unsigned char type, unsigned char val, void (*func)(void *));

#endif 

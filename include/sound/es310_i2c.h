/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#if !defined(__ES310_I2C_H)
#define __ES310_I2C_H

struct es310_platform_data {
	int (*dev_setup)(int in_on);
	int (*clk_on)(int in_on);
	int (*pwr_on)(int in_on);
	int reset_gpio;
	int uart_sin_gpio;
	int uart_sout_gpio;
};

#endif

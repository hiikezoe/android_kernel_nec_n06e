/* Copyright (c) 2011-2012, The Linux Foundation. All rights reserved.
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

#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/bootmem.h>
#include <linux/gpio.h>
#include <asm/mach-types.h>
#include <asm/mach/mmc.h>
#include <mach/msm_bus_board.h>
#include <mach/board.h>
#include <mach/gpiomux.h>
#include <mach/socinfo.h>
#include "devices.h"
#include "board-8064.h"

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)










/* The SPI configurations apply to GSBI 5*/
static struct gpiomux_setting gpio_spi_config = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_12MA,
	.pull = GPIOMUX_PULL_NONE,
};

/* The SPI configurations apply to GSBI 5 chip select 2*/
static struct gpiomux_setting gpio_spi_cs2_config = {
	.func = GPIOMUX_FUNC_3,
	.drv = GPIOMUX_DRV_12MA,
	.pull = GPIOMUX_PULL_NONE,
};














#endif
/* Chip selects for SPI clients */
static struct gpiomux_setting gpio_spi_cs_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_12MA,
	.pull = GPIOMUX_PULL_UP,
};












#ifdef CONFIG_MSM_VCAP
static struct gpiomux_setting gpio_vcap_config[] = {
	{
		.func = GPIOMUX_FUNC_GPIO,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
	{
		.func = GPIOMUX_FUNC_1,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
	{
		.func = GPIOMUX_FUNC_2,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
	{
		.func = GPIOMUX_FUNC_3,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
	{
		.func = GPIOMUX_FUNC_4,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
	{
		.func = GPIOMUX_FUNC_5,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
	{
		.func = GPIOMUX_FUNC_6,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
	{
		.func = GPIOMUX_FUNC_7,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
	{
		.func = GPIOMUX_FUNC_8,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
	{
		.func = GPIOMUX_FUNC_9,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},









};

struct msm_gpiomux_config vcap_configs[] = {




































	{
		.gpio = 19,
		.settings = {
			[GPIOMUX_SUSPENDED] =	&gpio_vcap_config[8],
			[GPIOMUX_ACTIVE] =		&gpio_vcap_config[8],
		}
	},






















	{
		.gpio = 12,
		.settings = {
			[GPIOMUX_SUSPENDED] =	&gpio_vcap_config[6],
			[GPIOMUX_ACTIVE] =		&gpio_vcap_config[6],
		}
	},
	{
		.gpio = 18,
		.settings = {
			[GPIOMUX_SUSPENDED] =	&gpio_vcap_config[9],
			[GPIOMUX_ACTIVE] =		&gpio_vcap_config[9],
		}
	},





























	{
		.gpio = 26,
		.settings = {
			[GPIOMUX_SUSPENDED] =	&gpio_vcap_config[1],
			[GPIOMUX_ACTIVE] =		&gpio_vcap_config[1],
		}
	},











	{
		.gpio = 7,
		.settings = {
			[GPIOMUX_SUSPENDED] =	&gpio_vcap_config[7],
			[GPIOMUX_ACTIVE] =		&gpio_vcap_config[7],
		}
	},
	{
		.gpio = 6,
		.settings = {
			[GPIOMUX_SUSPENDED] =	&gpio_vcap_config[7],
			[GPIOMUX_ACTIVE] =		&gpio_vcap_config[7],
		}
	},
	{
		.gpio = 80,
		.settings = {
			[GPIOMUX_SUSPENDED] =	&gpio_vcap_config[2],
			[GPIOMUX_ACTIVE] =		&gpio_vcap_config[2],
		}
	},























	{
		.gpio = 84,
		.settings = {
			[GPIOMUX_SUSPENDED] =	&gpio_vcap_config[3],
			[GPIOMUX_ACTIVE] =		&gpio_vcap_config[3],
		}
	},





































	{
		.gpio = 82,
		.settings = {
			[GPIOMUX_SUSPENDED] =	&gpio_vcap_config[4],
			[GPIOMUX_ACTIVE] =		&gpio_vcap_config[4],
		}
	},
	{
		.gpio = 83,
		.settings = {
			[GPIOMUX_SUSPENDED] =	&gpio_vcap_config[4],
			[GPIOMUX_ACTIVE] =		&gpio_vcap_config[4],
		}
	},
	{
		.gpio = 87,
		.settings = {
			[GPIOMUX_SUSPENDED] =	&gpio_vcap_config[2],
			[GPIOMUX_ACTIVE] =		&gpio_vcap_config[2],
		}
	},
	{
		.gpio = 13,
		.settings = {
			[GPIOMUX_SUSPENDED] =	&gpio_vcap_config[6],
			[GPIOMUX_ACTIVE] =		&gpio_vcap_config[6],
		}
	},
};
#endif














static struct gpiomux_setting gsbi1 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi1_1 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};


static struct gpiomux_setting gsbi2 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting gsbi2_1 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};



static struct gpiomux_setting gsbi3 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi3_1 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};



















static struct gpiomux_setting mbhc_hs_detect = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting cdc_mclk = {
	.func = GPIOMUX_FUNC_1,

	.drv = GPIOMUX_DRV_2MA,



	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting wcnss_5wire_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting wcnss_5wire_active_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_DOWN,
};


static struct gpiomux_setting slimbus = {
	.func = GPIOMUX_FUNC_1,

	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,




};

static struct gpiomux_setting gsbi1_uart_config = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting ext_regulator_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};



static struct gpiomux_setting ncm_2_2_d_l_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting ncm_g_2_d_l_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting ncm_2_2_d_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting ncm_g_2_d_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};































static struct gpiomux_setting gsbi5_i2c_sus_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi5_i2c_act_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};


static struct gpiomux_setting hdmi_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting hdmi_active_1_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};










static struct gpiomux_setting hdmi_active_2_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_UP,
};


















static struct gpiomux_setting sx150x_suspended_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sx150x_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

#ifdef CONFIG_USB_EHCI_MSM_HSIC
static struct gpiomux_setting cyts_sleep_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting cyts_sleep_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting cyts_int_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting cyts_int_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct msm_gpiomux_config cyts_gpio_configs[] __initdata = {
	{	/* TS INTERRUPT */
		.gpio = 6,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cyts_int_act_cfg,
			[GPIOMUX_SUSPENDED] = &cyts_int_sus_cfg,
		},
	},
	{	/* TS SLEEP */
		.gpio = 33,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cyts_sleep_act_cfg,
			[GPIOMUX_SUSPENDED] = &cyts_sleep_sus_cfg,
		},
	},
};
static struct msm_gpiomux_config cyts_gpio_alt_config[] __initdata = {
	{	/* TS INTERRUPT */
		.gpio = 6,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cyts_int_act_cfg,
			[GPIOMUX_SUSPENDED] = &cyts_int_sus_cfg,
		},
	},
	{	/* TS SLEEP */
		.gpio = 12,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cyts_sleep_act_cfg,
			[GPIOMUX_SUSPENDED] = &cyts_sleep_sus_cfg,
		},
	},
};

static struct gpiomux_setting hsic_act_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting hsic_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting hsic_wakeup_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting hsic_wakeup_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config apq8064_hsic_configs[] = {
	{
		.gpio = 88,               /*HSIC_STROBE */
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_sus_cfg,
		},
	},
	{
		.gpio = 89,               /* HSIC_DATA */
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_sus_cfg,
		},
	},
	{
		.gpio = 47,              /* wake up */
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_wakeup_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_wakeup_sus_cfg,
		},
	},
};
#endif

static struct gpiomux_setting mxt_reset_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting mxt_reset_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting mxt_int_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting mxt_int_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};



static struct gpiomux_setting ncm_g_2_n_l_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting ncm_g_2_u_i_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir  = GPIOMUX_IN,
};



static struct msm_gpiomux_config apq8064_hdmi_configs[] __initdata = {
	{
		.gpio = 69,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
	{
		.gpio = 70,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
	{
		.gpio = 71,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
	{
		.gpio = 72,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_2_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
};


static struct gpiomux_setting DVE022_mhl_reset_n_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting DVE022_mhl_int_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir  = GPIOMUX_IN,
};

static struct gpiomux_setting DVE022_mhl_1p2_en_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_OUT_LOW,
	
};


static struct msm_gpiomux_config apq8064_mhl_configs[] __initdata = {


	{
		.gpio = 37,
		.settings = {
			[GPIOMUX_ACTIVE]    = &DVE022_mhl_reset_n_cfg,
			[GPIOMUX_SUSPENDED] = &DVE022_mhl_reset_n_cfg,
		},
	},
	{
		.gpio = 38,
		.settings = {
			[GPIOMUX_ACTIVE]    = &DVE022_mhl_int_cfg,
			[GPIOMUX_SUSPENDED] = &DVE022_mhl_int_cfg,
		},
	},
	{
		.gpio = 81,
		.settings = {
			[GPIOMUX_ACTIVE]    = &DVE022_mhl_1p2_en_cfg,
			[GPIOMUX_SUSPENDED] = &DVE022_mhl_1p2_en_cfg,
		},
	},
















};

static struct msm_gpiomux_config apq8064_gsbi_configs[] __initdata = {
	{
		.gpio      = 8,			/* GSBI3 I2C QUP SDA */
		.settings = {





			[GPIOMUX_SUSPENDED] = &gsbi3,
			[GPIOMUX_ACTIVE] = &gsbi3_1,

		},
	},
	{
		.gpio      = 9,			/* GSBI3 I2C QUP SCL */
		.settings = {





			[GPIOMUX_SUSPENDED] = &gsbi3,
			[GPIOMUX_ACTIVE] = &gsbi3_1,

		},
	},
	{
		.gpio      = 18,		/* GSBI1 UART TX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi1_uart_config,
		},
	},
	{
		.gpio      = 19,		/* GSBI1 UART RX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi1_uart_config,
		},
	},
#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
	{
		.gpio      = 51,		/* GSBI5 QUP SPI_DATA_MOSI */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_config,
		},
	},
	{
		.gpio      = 52,		/* GSBI5 QUP SPI_DATA_MISO */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_config,
		},
	},
	{
		.gpio      = 53,		/* Funny CS0 */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_config,
		},
	},
	{
		.gpio      = 31,		/* GSBI5 QUP SPI_CS2_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_cs2_config,
		},
	},
	{
		.gpio      = 54,		/* GSBI5 QUP SPI_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_config,
		},
	},
#endif
	{
		.gpio      = 30,		/* FP CS */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_cs_config,
		},
	},










	{
		.gpio      = 53,		/* NOR CS */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_cs_config,
		},
	},


	{
		.gpio = 82,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ncm_2_2_d_l_cfg,
			[GPIOMUX_ACTIVE] = &ncm_g_2_d_l_cfg,
		},
	},
	{
		.gpio = 83,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ncm_2_2_d_cfg,
			[GPIOMUX_ACTIVE] = &ncm_g_2_d_cfg,
		},
	},


































	{
		.gpio      = 21,		/* GSBI1 QUP I2C_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi1,
			[GPIOMUX_ACTIVE] = &gsbi1_1,
		},
	},
	{
		.gpio      = 20,		/* GSBI1 QUP I2C_DATA */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi1,
			[GPIOMUX_ACTIVE] = &gsbi1_1,
		},
	},



	{
		.gpio      = 24,	
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi2,
			[GPIOMUX_ACTIVE] = &gsbi2_1,
		},
	},
	{
		.gpio      = 25,	
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi2,
			[GPIOMUX_ACTIVE] = &gsbi2_1,
		},
	},



	{
		.gpio      = 53,		
		.settings = {
			[GPIOMUX_ACTIVE] = &gsbi5_i2c_act_cfg,
			[GPIOMUX_SUSPENDED] = &gsbi5_i2c_sus_cfg,
		},
	},
	{
		.gpio      = 54,		
		.settings = {
			[GPIOMUX_ACTIVE] = &gsbi5_i2c_act_cfg,
			[GPIOMUX_SUSPENDED] = &gsbi5_i2c_sus_cfg,
		},
	},



	{
		.gpio     = 32,         
		.settings = {
			[GPIOMUX_SUSPENDED] = &ncm_g_2_u_i_cfg,
		},
	},
	{
		.gpio     = 28,         
		.settings = {
			[GPIOMUX_SUSPENDED] = &ncm_g_2_n_l_cfg,
		},
	},
	{
		.gpio     = 29,         
		.settings = {
			[GPIOMUX_SUSPENDED] = &ncm_g_2_u_i_cfg,
		},
	},
	{
		.gpio     = 59,         
		.settings = {
			[GPIOMUX_SUSPENDED] = &ncm_g_2_d_l_cfg,
		},
	},


};

static struct msm_gpiomux_config apq8064_slimbus_config[] __initdata = {
	{
		.gpio   = 40,           /* slimbus clk */
		.settings = {
			[GPIOMUX_SUSPENDED] = &slimbus,
		},
	},
	{
		.gpio   = 41,           /* slimbus data */
		.settings = {
			[GPIOMUX_SUSPENDED] = &slimbus,
		},
	},
};


static struct gpiomux_setting sensor_prox_light_int_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting sensor_prox_light_int_suspended_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting sensor_akm_rst_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting sensor_akm_rst_suspended_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};
	
static struct gpiomux_setting sensor_akm_int_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};
	
static struct gpiomux_setting sensor_akm_int_suspended_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};
	
static struct msm_gpiomux_config apq8064_sensor_configs[] __initdata = {
	{
		.gpio = 22,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sensor_prox_light_int_active_cfg,
			[GPIOMUX_SUSPENDED] = &sensor_prox_light_int_suspended_cfg,
		},
	},
	{
		.gpio = 85,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sensor_akm_rst_active_cfg,
			[GPIOMUX_SUSPENDED] = &sensor_akm_rst_suspended_cfg,
		},
	},
	{
		.gpio = 23,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sensor_akm_int_active_cfg,
			[GPIOMUX_SUSPENDED] = &sensor_akm_int_suspended_cfg,
		},
	},
};



static struct gpiomux_setting spkr_i2c = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_KEEPER,
};

static struct msm_gpiomux_config mpq8064_spkr_i2s_config[] __initdata = {
	{
		.gpio   = 47,           /* spkr i2c sck */
		.settings = {
			[GPIOMUX_SUSPENDED] = &spkr_i2c,
		},
	},
	{
		.gpio   = 48,           /* spkr_i2s_ws */
		.settings = {
			[GPIOMUX_SUSPENDED] = &spkr_i2c,
		},
	},
	{
		.gpio   = 49,           /* spkr_i2s_dout */
		.settings = {
			[GPIOMUX_SUSPENDED] = &spkr_i2c,
		},
	},
	{
		.gpio   = 50,           /* spkr_i2s_mclk */
		.settings = {
			[GPIOMUX_SUSPENDED] = &spkr_i2c,
		},
	},
};

static struct msm_gpiomux_config apq8064_audio_codec_configs[] __initdata = {
	{
		.gpio = 38,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mbhc_hs_detect,
		},
	},
	{
		.gpio = 39,
		.settings = {
			[GPIOMUX_SUSPENDED] = &cdc_mclk,
		},
	},
};

static struct gpiomux_setting es310_clk = {
	.func = GPIOMUX_FUNC_3,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};
static struct gpiomux_setting sw_ctl = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};
static struct msm_gpiomux_config audio_es310_gpio_configs[] __initdata = {
	{
		.gpio = 4,
		.settings = {
			[GPIOMUX_SUSPENDED] = &es310_clk,
		},
	},
	{
		.gpio = 7,
		.settings = {
			[GPIOMUX_SUSPENDED] = &sw_ctl,
		},
	},
	{
		.gpio = 45,
		.settings = {
			[GPIOMUX_SUSPENDED] = &sw_ctl,
		},
	},
};


/* External 3.3 V regulator enable */
static struct msm_gpiomux_config apq8064_ext_regulator_configs[] __initdata = {
	{
		.gpio = APQ8064_EXT_3P3V_REG_EN_GPIO,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ext_regulator_config,
		},
	},
};

static struct gpiomux_setting ap2mdm_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting mdm2ap_status_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting mdm2ap_errfatal_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting mdm2ap_pblrdy = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};


static struct gpiomux_setting ap2mdm_soft_reset_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting ap2mdm_wakeup = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_DOWN,
};



static struct gpiomux_setting dtv_reset_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting tsif_cfg_disable_2MA = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_IN,
};

static struct gpiomux_setting tsif_cfg_enable_2MA = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_IN,
};

static struct msm_gpiomux_config dtv_configs[] __initdata = {
	
	{
		.gpio = 31,
		.settings = {
			[GPIOMUX_ACTIVE]    = &dtv_reset_cfg,
			[GPIOMUX_SUSPENDED] = &dtv_reset_cfg,
		},
	},
	
	{
		.gpio = 55,
		.settings = {
			[GPIOMUX_ACTIVE]    = &tsif_cfg_enable_2MA,
			[GPIOMUX_SUSPENDED] = &tsif_cfg_disable_2MA,
		},
	},
	
	{
		.gpio = 56,
		.settings = {
			[GPIOMUX_ACTIVE]    = &tsif_cfg_enable_2MA,
			[GPIOMUX_SUSPENDED] = &tsif_cfg_disable_2MA,
		},
	},
	
	{
		.gpio = 57,
		.settings = {
			[GPIOMUX_ACTIVE]    = &tsif_cfg_enable_2MA,
			[GPIOMUX_SUSPENDED] = &tsif_cfg_disable_2MA,
		},
	},
	
	{
		.gpio = 62,
		.settings = {
			[GPIOMUX_ACTIVE]    = &tsif_cfg_enable_2MA,
			[GPIOMUX_SUSPENDED] = &tsif_cfg_disable_2MA,
		},
	},
};



static struct msm_gpiomux_config mdm_configs[] __initdata = {
	/* AP2MDM_STATUS */
	{
		.gpio = 48,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* MDM2AP_STATUS */
	{
		.gpio = 49,
		.settings = {
			[GPIOMUX_ACTIVE] = &mdm2ap_status_cfg,
			[GPIOMUX_SUSPENDED] = &mdm2ap_status_cfg,
		}
	},
	/* MDM2AP_ERRFATAL */
	{
		.gpio = 19,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_errfatal_cfg,
		}
	},
	/* AP2MDM_ERRFATAL */
	{
		.gpio = 18,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* AP2MDM_SOFT_RESET, aka AP2MDM_PON_RESET_N */
	{
		.gpio = 27,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_soft_reset_cfg,
		}
	},
	/* AP2MDM_WAKEUP */
	{
		.gpio = 35,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_wakeup,
		}
	},
	/* MDM2AP_PBL_READY*/
	{
		.gpio = 46,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_pblrdy,
		}
	},
};

static struct msm_gpiomux_config amdm_configs[] __initdata = {
	/* AP2MDM_STATUS */
	{
		.gpio = 48,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* MDM2AP_STATUS */
	{
		.gpio = 49,
		.settings = {
			[GPIOMUX_ACTIVE] = &mdm2ap_status_cfg,
			[GPIOMUX_SUSPENDED] = &mdm2ap_status_cfg,
		}
	},
	/* MDM2AP_ERRFATAL */
	{
		.gpio = 19,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_errfatal_cfg,
		}
	},
	/* AP2MDM_ERRFATAL */
	{
		.gpio = 18,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* AP2MDM_SOFT_RESET, aka AP2MDM_PON_RESET_N */
	{
		.gpio = 27,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_soft_reset_cfg,
		}
	},
	/* AP2MDM_WAKEUP */
	{
		.gpio = 35,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_wakeup,
		}
	},
	/* MDM2AP_PBL_READY*/
	{
		.gpio = 31,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_pblrdy,
		}
	},
};

static struct msm_gpiomux_config bmdm_configs[] __initdata = {
	/* AP2MDM_STATUS */
	{
		.gpio = 56,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* MDM2AP_STATUS */
	{
		.gpio = 32,
		.settings = {
			[GPIOMUX_ACTIVE] = &mdm2ap_status_cfg,
			[GPIOMUX_SUSPENDED] = &mdm2ap_status_cfg,
		}
	},
	/* MDM2AP_ERRFATAL */
	{
		.gpio = 81,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_errfatal_cfg,
		}
	},
	/* AP2MDM_ERRFATAL */
	{
		.gpio = 18,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* AP2MDM_SOFT_RESET, aka AP2MDM_PON_RESET_N */
	{
		.gpio = 3,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_soft_reset_cfg,
		}
	},
	/* AP2MDM_WAKEUP */
	{
		.gpio = 29,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_wakeup,
		}
	},
};

static struct msm_gpiomux_config mdm_i2s_configs[] __initdata = {
	/* AP2MDM_STATUS */
	{
		.gpio = 48,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* MDM2AP_STATUS */
	{
		.gpio = 49,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_status_cfg,
		}
	},
	/* MDM2AP_ERRFATAL */
	{
		.gpio = 19,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_errfatal_cfg,
		}
	},
	/* AP2MDM_ERRFATAL */
	{
		.gpio = 18,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* AP2MDM_SOFT_RESET, aka AP2MDM_PON_RESET_N */
	{
		.gpio = 0,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_soft_reset_cfg,
		}
	},
	/* AP2MDM_WAKEUP */
	{
		.gpio = 44,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_wakeup,
		}
	},
	/* MDM2AP_PBL_READY*/
	{
		.gpio = 81,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_pblrdy,
		}
	},
};

static struct gpiomux_setting mi2s_act_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting mi2s_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct msm_gpiomux_config mpq8064_mi2s_configs[] __initdata = {
	{
		.gpio	= 27,		/* mi2s ws */
		.settings = {
			[GPIOMUX_ACTIVE]    = &mi2s_act_cfg,
			[GPIOMUX_SUSPENDED] = &mi2s_sus_cfg,
		},
	},


















	{
		.gpio	= 30,		/* mi2s dout2 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &mi2s_act_cfg,
			[GPIOMUX_SUSPENDED] = &mi2s_sus_cfg,
		},
	},

	{
		.gpio	= 31,		/* mi2s dout1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &mi2s_act_cfg,
			[GPIOMUX_SUSPENDED] = &mi2s_sus_cfg,
		},
	},
	{
		.gpio	= 32,		/* mi2s dout0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &mi2s_act_cfg,
			[GPIOMUX_SUSPENDED] = &mi2s_sus_cfg,
		},
	},

	{
		.gpio	= 33,		/* mi2s mclk */
		.settings = {
			[GPIOMUX_ACTIVE]    = &mi2s_act_cfg,
			[GPIOMUX_SUSPENDED] = &mi2s_sus_cfg,
		},
	},
};
static struct msm_gpiomux_config apq8064_mxt_configs[] __initdata = {
	{	/* TS INTERRUPT */
		.gpio = 6,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mxt_int_act_cfg,
			[GPIOMUX_SUSPENDED] = &mxt_int_sus_cfg,
		},
	},
	{	/* TS RESET */
		.gpio = 33,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mxt_reset_act_cfg,
			[GPIOMUX_SUSPENDED] = &mxt_reset_sus_cfg,
		},
	},
};

static struct msm_gpiomux_config wcnss_5wire_interface[] = {
	{
		.gpio = 64,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 65,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 66,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 67,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 68,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
};



static struct gpiomux_setting tp_attn1_act = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting tp_attn1_inact = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config melfas_ncm_configs[] __initdata = {
	{
		.gpio = 6,
		.settings = {
			[GPIOMUX_ACTIVE]    = &tp_attn1_act,
			[GPIOMUX_SUSPENDED] = &tp_attn1_inact,
		},
	},
};

























static struct gpiomux_setting led_reset_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_OUT_HIGH,
};

static struct msm_gpiomux_config led_configs[] __initdata = {
	
	{
		.gpio = 86,
		.settings = {
			[GPIOMUX_ACTIVE]    = &led_reset_cfg,
			[GPIOMUX_SUSPENDED] = &led_reset_cfg,
		},

		.gpio = 33,
		.settings = {
			[GPIOMUX_ACTIVE]    = &led_reset_cfg,
			[GPIOMUX_SUSPENDED] = &led_reset_cfg,
		},

	},
};



static struct gpiomux_setting ir_suspended_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting ir_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config mpq8064_ir_configs[] __initdata = {
	{
		.gpio      = 88,			/* GPIO IR */
		.settings = {
			[GPIOMUX_SUSPENDED] = &ir_suspended_cfg,
			[GPIOMUX_ACTIVE] = &ir_active_cfg,
		},
	},
};

static struct msm_gpiomux_config sx150x_int_configs[] __initdata = {
	{
		.gpio      = 81,
		.settings = {
			[GPIOMUX_SUSPENDED] = &sx150x_suspended_cfg,
			[GPIOMUX_ACTIVE] = &sx150x_active_cfg,
		},
	},
};

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
static struct gpiomux_setting sdc2_clk_active_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sdc2_cmd_data_0_3_active_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting sdc2_suspended_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting sdc2_data_1_suspended_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config apq8064_sdc2_configs[] __initdata = {
	{
		.gpio      = 59,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc2_clk_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc2_suspended_cfg,
		},
	},
	{
		.gpio      = 57,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc2_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc2_suspended_cfg,
		},

	},
	{
		.gpio      = 62,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc2_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc2_suspended_cfg,
		},
	},
	{
		.gpio      = 61,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc2_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc2_data_1_suspended_cfg,
		},
	},
	{
		.gpio      = 60,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc2_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc2_suspended_cfg,
		},
	},
	{
		.gpio      = 58,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc2_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc2_suspended_cfg,
		},
	},
};
#endif


#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
static struct gpiomux_setting sdc4_clk_active_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sdc4_cmd_data_0_3_active_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting sdc4_suspended_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting sdc4_data_1_suspended_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config apq8064_sdc4_configs[] __initdata = {
	{
		.gpio      = 68,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc4_clk_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspended_cfg,
		},
	},
	{
		.gpio      = 67,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc4_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspended_cfg,
		},

	},
	{
		.gpio      = 66,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc4_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspended_cfg,
		},
	},
	{
		.gpio      = 65,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc4_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_data_1_suspended_cfg,
		},
	},
	{
		.gpio      = 64,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc4_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspended_cfg,
		},
	},
	{
		.gpio      = 63,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc4_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspended_cfg,
		},
	},
};
#endif



static struct gpiomux_setting gpio_haptics_active_config = {
    .pull = GPIOMUX_PULL_NONE,
    .drv = GPIOMUX_DRV_8MA,
    .func = GPIOMUX_FUNC_GPIO,
    .dir = GPIOMUX_OUT_HIGH,
};

static struct gpiomux_setting gpio_haptics_deactive_config = {
    .pull = GPIOMUX_PULL_NONE,
    .drv = GPIOMUX_DRV_8MA,
    .func = GPIOMUX_FUNC_GPIO,
    .dir = GPIOMUX_OUT_LOW,
};

struct msm_gpiomux_config haptics_configs[] = {
    {
        .gpio = 43,
        .settings = {
            [GPIOMUX_SUSPENDED] = &gpio_haptics_deactive_config,
            [GPIOMUX_ACTIVE] = &gpio_haptics_active_config,
        }
    },
};



static struct gpiomux_setting apq8064_sdc3_card_det_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config apq8064_sdc3_configs[] __initdata = {
	{
		.gpio      = 26,
		.settings = {
			[GPIOMUX_SUSPENDED] = &apq8064_sdc3_card_det_cfg,
			[GPIOMUX_ACTIVE] = &apq8064_sdc3_card_det_cfg,
		},
	},
};

static struct msm_gpiomux_config sglte2_qsc_configs[] __initdata = {
	/* MDM2AP_STATUS */
	{
		.gpio = 51,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_status_cfg,
		}
	},
	/* MDM2AP_ERRFATAL */
	{
		.gpio = 52,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_errfatal_cfg,
		}
	},
};

static struct gpiomux_setting gsbi4_uartdm_active = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi4_uartdm_suspended = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting gsbi6_uartdm_active = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi6_uartdm_suspended = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct msm_gpiomux_config apq8064_uartdm_gsbi4_configs[] __initdata = {
	{
		.gpio      = 11,        /* GSBI4 UARTDM RX */
		.settings = {
			[GPIOMUX_ACTIVE] = &gsbi4_uartdm_active,
			[GPIOMUX_SUSPENDED] = &gsbi4_uartdm_suspended,
		},
	},
	{
		.gpio      = 10,       /* GSBI4 UARTDM TX */
		.settings = {
			[GPIOMUX_ACTIVE] = &gsbi4_uartdm_active,
			[GPIOMUX_SUSPENDED] = &gsbi4_uartdm_suspended,
		},
	},
	{
		.gpio      = 13,        /* GSBI4 UARTDM RFR */
		.settings = {
			[GPIOMUX_ACTIVE] = &gsbi4_uartdm_active,
			[GPIOMUX_SUSPENDED] = &gsbi4_uartdm_suspended,
		},
	},
	{
		.gpio      = 12,        /* GSBI4 UARTDM CTS */
		.settings = {
			[GPIOMUX_ACTIVE] = &gsbi4_uartdm_active,
			[GPIOMUX_SUSPENDED] = &gsbi4_uartdm_suspended,
		},
	},
};

static struct msm_gpiomux_config mpq8064_uartdm_configs[] __initdata = {
	{ /* UARTDM_TX */
		.gpio      = 14,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi6_uartdm_active,
			[GPIOMUX_SUSPENDED] = &gsbi6_uartdm_suspended,
		},
	},
	{ /* UARTDM_RX */
		.gpio      = 15,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi6_uartdm_active,
			[GPIOMUX_SUSPENDED] = &gsbi6_uartdm_suspended,
		},
	},
	{ /* UARTDM_CTS */
		.gpio      = 16,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi6_uartdm_active,
			[GPIOMUX_SUSPENDED] = &gsbi6_uartdm_suspended,
		},
	},
	{ /* UARTDM_RFR */
		.gpio      = 17,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi6_uartdm_active,
			[GPIOMUX_SUSPENDED] = &gsbi6_uartdm_suspended,
		},
	},
};

void __init apq8064_init_gpiomux(void)
{
	int rc;
	int platform_version = socinfo_get_platform_version();

	rc = msm_gpiomux_init(NR_GPIO_IRQS);
	if (rc) {
		pr_err(KERN_ERR "msm_gpiomux_init failed %d\n", rc);
		return;
	}

	msm_gpiomux_install(wcnss_5wire_interface,
			ARRAY_SIZE(wcnss_5wire_interface));

	if (machine_is_mpq8064_cdp() || machine_is_mpq8064_hrd() ||
		 machine_is_mpq8064_dtv()) {






#ifdef CONFIG_MSM_VCAP
		msm_gpiomux_install(vcap_configs,
				ARRAY_SIZE(vcap_configs));
#endif
		msm_gpiomux_install(sx150x_int_configs,
				ARRAY_SIZE(sx150x_int_configs));
	} else {
		#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)






		#endif

		msm_gpiomux_install(apq8064_gsbi_configs,
				ARRAY_SIZE(apq8064_gsbi_configs));



		msm_gpiomux_install(haptics_configs,
				ARRAY_SIZE(haptics_configs));


	}

	msm_gpiomux_install(apq8064_slimbus_config,
			ARRAY_SIZE(apq8064_slimbus_config));

	msm_gpiomux_install(apq8064_audio_codec_configs,
			ARRAY_SIZE(apq8064_audio_codec_configs));


	msm_gpiomux_install(audio_es310_gpio_configs,
			ARRAY_SIZE(audio_es310_gpio_configs));


	if (machine_is_mpq8064_cdp() || machine_is_mpq8064_hrd() ||
		machine_is_mpq8064_dtv()) {
		msm_gpiomux_install(mpq8064_spkr_i2s_config,
			ARRAY_SIZE(mpq8064_spkr_i2s_config));
	}

	pr_debug("%s(): audio-auxpcm: Include GPIO configs"
		" as audio is not the primary user"
		" for these GPIO Pins\n", __func__);

	if (machine_is_mpq8064_cdp() || machine_is_mpq8064_hrd() ||
		machine_is_mpq8064_dtv())
		msm_gpiomux_install(mpq8064_mi2s_configs,
			ARRAY_SIZE(mpq8064_mi2s_configs));

	msm_gpiomux_install(apq8064_ext_regulator_configs,
			ARRAY_SIZE(apq8064_ext_regulator_configs));

	if (machine_is_apq8064_mtp()) {
		if (socinfo_get_platform_subtype() == PLATFORM_SUBTYPE_DSDA2) {
			msm_gpiomux_install(amdm_configs,
					ARRAY_SIZE(amdm_configs));
			msm_gpiomux_install(bmdm_configs,
				ARRAY_SIZE(bmdm_configs));
		} else if (socinfo_get_platform_subtype() ==
					PLATFORM_SUBTYPE_SGLTE2) {
			msm_gpiomux_install(mdm_configs,
					ARRAY_SIZE(mdm_configs));
			msm_gpiomux_install(sglte2_qsc_configs,
					ARRAY_SIZE(sglte2_qsc_configs));

			/* GSBI4 UART GPIOs for Primary IPC */
			msm_gpiomux_install(apq8064_uartdm_gsbi4_configs,
				ARRAY_SIZE(apq8064_uartdm_gsbi4_configs));
		} else if (SOCINFO_VERSION_MINOR(platform_version) == 1) {
			msm_gpiomux_install(mdm_i2s_configs,
					ARRAY_SIZE(mdm_i2s_configs));
		} else {
			msm_gpiomux_install(mdm_configs,
					ARRAY_SIZE(mdm_configs));
		}
	}

	if (machine_is_apq8064_mtp()) {
		if (SOCINFO_VERSION_MINOR(platform_version) == 1) {
			msm_gpiomux_install(cyts_gpio_alt_config,
					ARRAY_SIZE(cyts_gpio_alt_config));
		} else {
			msm_gpiomux_install(cyts_gpio_configs,
					ARRAY_SIZE(cyts_gpio_configs));
		}
	}

#ifdef CONFIG_USB_EHCI_MSM_HSIC
	if (machine_is_apq8064_mtp())
		msm_gpiomux_install(apq8064_hsic_configs,
				ARRAY_SIZE(apq8064_hsic_configs));
#endif

	if (machine_is_apq8064_cdp() || machine_is_apq8064_liquid())
		msm_gpiomux_install(apq8064_mxt_configs,
			ARRAY_SIZE(apq8064_mxt_configs));

	msm_gpiomux_install(apq8064_hdmi_configs,
			ARRAY_SIZE(apq8064_hdmi_configs));


	msm_gpiomux_install(apq8064_sensor_configs,
			ARRAY_SIZE(apq8064_sensor_configs));
	


	if (apq8064_mhl_display_enabled())
		msm_gpiomux_install(apq8064_mhl_configs,
				ARRAY_SIZE(apq8064_mhl_configs));


	 if (machine_is_mpq8064_cdp())
		msm_gpiomux_install(mpq8064_ir_configs,
				ARRAY_SIZE(mpq8064_ir_configs));

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	 msm_gpiomux_install(apq8064_sdc2_configs,
			     ARRAY_SIZE(apq8064_sdc2_configs));
#endif

#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
	 msm_gpiomux_install(apq8064_sdc4_configs,
			     ARRAY_SIZE(apq8064_sdc4_configs));
#endif



	msm_gpiomux_install(melfas_ncm_configs,
			ARRAY_SIZE(melfas_ncm_configs));





	msm_gpiomux_install(led_configs,
			ARRAY_SIZE(led_configs));





	msm_gpiomux_install(dtv_configs,
			ARRAY_SIZE(dtv_configs));



	msm_gpiomux_install(apq8064_sdc3_configs,
			ARRAY_SIZE(apq8064_sdc3_configs));
	 if (machine_is_mpq8064_hrd() || machine_is_mpq8064_dtv())
		msm_gpiomux_install(mpq8064_uartdm_configs,
				ARRAY_SIZE(mpq8064_uartdm_configs));
}

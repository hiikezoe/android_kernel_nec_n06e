/*---------------------------------------------------------------------------
 * File Name: apds990x.c
 *
 * This file is the device driver for Avago APDS9900/APDS9901 digital proximity
 * and ambient light sensor.
 *
 * (C) NEC Corporation 2011
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *---------------------------------------------------------------------------
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <asm/div64.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/i2c/apds990x_driver.h>
#include <linux/wakelock.h>

#define DRIVER_VERSION		"1.0.0"

MODULE_AUTHOR("NEC Corporation");
MODULE_DESCRIPTION("APDS990x Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

#define CMD_BYTE        	0x80	
#define CMD_WORD        	0xA0	


#define D_I2C_NO_ERR		0
#define D_I2C_ERR		1

#define D_PS_DETECT_STATE_BUSY	2

#define CMD_CLR_PS_INT		0xE5	
#define CMD_CLR_ALS_INT		0xE6	
#define CMD_CLR_PS_ALS_INT	0xE7	

#define ENABLE_MASK_PON		(1 << 0) 
#define ENABLE_MASK_AEN		(1 << 1) 
#define ENABLE_MASK_PEN		(1 << 2) 
#define ENABLE_MASK_WEN		(1 << 3) 
#define ENABLE_MASK_AIEN	(1 << 4) 
#define ENABLE_MASK_PIEN	(1 << 5) 


#define STATE_POWEROFF 		(0x00)


#define STATE_ALS_ON		(ENABLE_MASK_PON | ENABLE_MASK_AEN | ENABLE_MASK_WEN)


#define STATE_PROX_ON		(STATE_ALS_ON | ENABLE_MASK_PEN \
				| ENABLE_MASK_PIEN | ENABLE_MASK_AIEN)


#define STATE_PROX_DIAG_ON	(STATE_ALS_ON | ENABLE_MASK_PEN \
				| ENABLE_MASK_PIEN)


#define NV_ATIME		(g_nv_data.nv_reg[NV_SH_PROX_ALS_ATIME])
#define NV_PTIME		(g_nv_data.nv_reg[NV_SH_PROX_ALS_PTIME])
#define NV_WTIME		(g_nv_data.nv_reg[NV_SH_PROX_ALS_WTIME])
#define NV_WTIME2		(g_nv_data.nv_reg[NV_SH_PROX_ALS_WTIME2])
#define NV_PERS			(g_nv_data.nv_reg[NV_SH_PROX_ALS_PERS])
#define NV_CONFIG		(g_nv_data.nv_reg[NV_SH_PROX_ALS_CONFIG])
#define NV_PPCOUNT		(g_nv_data.nv_reg[NV_SH_PROX_ALS_PPCOUNT])
#define	NV_CONTROL		(g_nv_data.nv_reg[NV_SH_PROX_ALS_CONTROL])
#define NV_PILT			(g_nv_data.nv_reg[NV_SH_PROX_ALS_PILTL] \
				| g_nv_data.nv_reg[NV_SH_PROX_ALS_PILTH] << 8)
#define NV_H_PIHT		(g_nv_data.nv_reg[NV_SH_PROX_ALS_H_PIHTL] \
				| g_nv_data.nv_reg[NV_SH_PROX_ALS_H_PIHTH] << 8)
#define NV_PIHT			(g_nv_data.nv_reg[NV_SH_PROX_ALS_PIHTL] \
				| g_nv_data.nv_reg[NV_SH_PROX_ALS_PIHTH] << 8)
#define NV_L_PILT		(g_nv_data.nv_reg[NV_SH_PROX_ALS_L_PILTL] \
				| g_nv_data.nv_reg[NV_SH_PROX_ALS_L_PILTH] << 8)
#define NV_DET_CANCEL		(g_nv_data.nv_reg[NV_SH_PROX_ALS_DET_CANCEL])
#define NV_CO_B			(g_nv_data.nv_reg[NV_SH_ALS_COEFF_B])
#define NV_CO_C 		(g_nv_data.nv_reg[NV_SH_ALS_COEFF_C])
#define NV_CO_D 		(g_nv_data.nv_reg[NV_SH_ALS_COEFF_D])
#define NV_CO_K			(g_nv_data.nv_reg[NV_SH_ALS_COEFF_K])
#define NV_ALSIT		(2720 * (256 - g_nv_data.nv_reg[NV_SH_PROX_ALS_ATIME]))
#define NV_GA			(g_nv_data.nv_reg[NV_SH_ALS_COEFF_GA])

#define ALS_WAIT		((87 * (256 * 2 - NV_ATIME - NV_WTIME2)) >> 5)
#define ALS_PS_WAIT		((87 * (256 * 3 - NV_ATIME - NV_PTIME - NV_WTIME)) >> 5)
#define ATIME_WAIT		((87 * (256 - NV_ATIME)) >> 5)
#define PTIME_WAIT		((87 * (256 - NV_PTIME)) >> 5)
#define WTIME_WAIT		((87 * (256 - NV_WTIME)) >> 5)
#define WTIME2_WAIT		((87 * (256 - NV_WTIME2)) >> 5)
#define RESTART_WAIT1_BASE ((87 * (256 * 3 - NV_ATIME - NV_PTIME - NV_WTIME)) >> 5)
#define RESTART_WAIT2_BASE ((87 * (256 * 3 - NV_ATIME - NV_PTIME - NV_WTIME2)) >> 5)
#define RESTART_WAIT1 (RESTART_WAIT1_BASE + RESTART_WAIT1_BASE/10)
#define RESTART_WAIT2 (RESTART_WAIT2_BASE + RESTART_WAIT2_BASE/10)

#if !defined(MAX)
#define MAX(a, b)		(((a) > (b)) ? (a) : (b))
#endif


static int diag_calc_restart_wait(void);
static int diag_restart_ready(void);
static int diag_prox_start(void);
static int diag_prox_stop(void);
static int diag_als_start(void);
static int diag_als_stop(void);
static void diag_interrupt_work_func_f1(void);
static void diag_interrupt_work_func_f2(void);
static int diag_get_start(apds990x_diagget_mode mode);
static int diag_get_lux_data(struct apds990x_diag_data_once *data);
static int diag_get_prox_data(struct apds990x_diag_data_once *data);
static int diag_get_data_once(struct apds990x_diag_data_once *data, apds990x_diagget_mode mode);
static int diag_get_end(apds990x_diagget_mode mode);
static int diag_get_datan(struct apds990x_diag_data *data, struct apds990x_diag_data_once *store_data);
static int read_reg_4byte( unsigned char addr, unsigned int *data );

static void als_poll_work_func(struct work_struct *);
static void interrupt_work(struct work_struct *);
static void interrupt_work_func_f1(void);
static void interrupt_work_func_f2(void);
static void interrupt_work_func_f3(void);
static void (*interrupt_work_func)(void);

struct apds990x_data {
	unsigned int ps_n_threshold_l;	
	unsigned int ps_n_threshold_h;
	unsigned int ps_d_threshold_l;	
	unsigned int ps_d_threshold_h;
	unsigned int als_miss_threshold;
	unsigned int als_max_count;
	unsigned int als_saturates;
	unsigned int meas_cycle;
	unsigned int LPC;		
};

static int g_light_err = D_I2C_NO_ERR;	
static int g_prox_err = D_I2C_NO_ERR;	
static int g_distance = 0;		
static int g_is_nv_set = false;		
static int g_als_current = 0;		
static int g_als_open_count = 0;	
static int g_prox_open_count = 0;	
static int g_state = STATE_POWEROFF;	
static int g_i2c_error_alarmed = false; 
static struct i2c_client *g_client = NULL;

static int g_suspend_state = false;
static int g_irq_occurred = false; 


static unsigned int g_ch0_buff[8] = { 0, };


static unsigned int g_ch1_buff[8] = { 0, };


static struct apds990x_data g_data = { 0, };


static struct apds990x_nv_data g_nv_data = {
	.nv_reg = { 0, },
};


DEFINE_SPINLOCK(g_spinlock);


DEFINE_MUTEX(g_mutex);


DECLARE_WAIT_QUEUE_HEAD(g_diag_data_queue);


static DECLARE_WORK(g_interrupt_work, interrupt_work);


static DECLARE_DELAYED_WORK(g_als_poll_work, als_poll_work_func);


DEFINE_SPINLOCK(g_interrupt_spinlock);


static struct wake_lock g_interrupt_wake_lock;










static unsigned int lux_calculate(unsigned int ch0, unsigned int ch1)
{
	u64 temp;
	int IAC, IAC1, IAC2;
	unsigned int lux;

	IAC1 = ch0 * 1000 - ch1 * NV_CO_B;
	IAC2 = ch0 * NV_CO_C - ch1 * NV_CO_D;
	IAC = MAX(MAX(IAC1, IAC2), 0);
	temp = (u64)IAC * g_data.LPC;
	do_div(temp, 1000000);
	lux = (unsigned int)temp;

	return lux;
}












static int als_get_lux(struct apds990x_light_data *light_data)
{
	int i, IAC1, IAC2;
	unsigned int biggest_ch0, biggest_ch1;
	unsigned int smallest_ch0, smallest_ch1;
	unsigned int ch0, ch1, num;
	unsigned int buff_ch0[LIGHT_AVERAGE_NUMBER];
	unsigned int buff_ch1[LIGHT_AVERAGE_NUMBER];

	if (unlikely(light_data == NULL)) {
		pr_err("%s: NULL pointer error!\n", __func__);
		return -EINVAL;
	}

	spin_lock(&g_spinlock);
	num = g_als_current;
	
	if (light_data->mode == D_LIGHT_MODE_LATEST) {
		light_data->lux = lux_calculate(g_ch0_buff[num & 0x07],
						g_ch1_buff[num & 0x07]);
		spin_unlock(&g_spinlock);
		return 0;
	}

	
	if ((g_ch0_buff[LIGHT_AVERAGE_NUMBER - 1] == (unsigned int)(-1))) {
		spin_unlock(&g_spinlock);
		return -EBUSY;
	}

	
	for (i = 0; i < LIGHT_AVERAGE_NUMBER; i++) {
		
		IAC1 = g_ch0_buff[num & 0x07] * 1000 - g_ch1_buff[num & 0x07] * NV_CO_B;
		IAC2 = g_ch0_buff[num & 0x07] * NV_CO_C - g_ch1_buff[num & 0x07] * NV_CO_D;
		if ((g_ch0_buff[num & 0x07] >= g_data.als_saturates)
				|| ((g_ch0_buff[num & 0x07] > 10000) && (IAC1 < 0) && (IAC2 < 0))) {
			buff_ch0[i] = g_data.als_saturates - 1;
			buff_ch1[i] = buff_ch0[i] / 10;
		} else {
			buff_ch0[i] = g_ch0_buff[num & 0x07];
			buff_ch1[i] = g_ch1_buff[num & 0x07];
		}

		num--;
	}
	spin_unlock(&g_spinlock);

	biggest_ch0 = buff_ch0[0];
	biggest_ch1 = buff_ch1[0];
	smallest_ch0 = buff_ch0[0];
	smallest_ch1 = buff_ch1[0];
	ch0 = buff_ch0[0];
	ch1 = buff_ch1[0];
	
	for (i = 1; i < LIGHT_AVERAGE_NUMBER; i++) {
		ch0 += buff_ch0[i];
		ch1 += buff_ch1[i];

		if (buff_ch0[i] > biggest_ch0)
			biggest_ch0 = buff_ch0[i];
		else if (buff_ch0[i] < smallest_ch0)
			smallest_ch0 = buff_ch0[i];

		if (buff_ch1[i] > biggest_ch1)
			biggest_ch1 = buff_ch1[i];
		else if (buff_ch1[i] < smallest_ch1)
			smallest_ch1 = buff_ch1[i];
	}

	ch0 = (ch0 - smallest_ch0 - biggest_ch0) / (LIGHT_AVERAGE_NUMBER - 2);
	ch1 = (ch1 - smallest_ch1 - biggest_ch1) / (LIGHT_AVERAGE_NUMBER - 2);

	
	light_data->lux = lux_calculate(ch0, ch1);

	return 0;
}









static int i2c_rx_data(unsigned char *rxData, int length)
{
	int ret = 0;
	unsigned char info1 = (length > 0) ? 0x03 : 0x01;
	
	char rxData0Bak = (rxData == NULL) ? 0 : rxData[0];
	struct i2c_msg msgs[] = {
		{
			.addr  = g_client->addr,
			.flags = 0,
			.len   = 1,
			.buf   = rxData,
		},
		{
			.addr  = g_client->addr,
			.flags = I2C_M_RD,
			.len   = length,
			.buf   = rxData,
		},
	};

	if (unlikely(rxData == NULL)) {
		pr_err("%s: NULL pointer error!\n", __func__);
		return -EINVAL;
	}

	ret = i2c_transfer(g_client->adapter, msgs, 2);
	if (ret < 0) {
		pr_err("apds990x %s: transfer error\n", __func__);
		if (g_i2c_error_alarmed == false) {
			info1 |= (0x01 << 2);
			pr_err("[T][ARM]Event:0x61 Info:0x%02x%02x%02x%02x\n",
				info1, (-ret) & 0xFF,rxData0Bak, 0);
			g_i2c_error_alarmed = true;
		}
		return -EIO;
	} else {
		g_i2c_error_alarmed = false;
	}

	return 0;
}









static int i2c_tx_data(unsigned char *txData, int length)
{
	int ret = 0;
	unsigned char info1 = 0x01;
	struct i2c_msg msgs[] = {
		{
			.addr  = g_client->addr,
			.flags = 0,
			.len   = length,
			.buf   = txData,
		},
	};

	if (unlikely(txData == NULL)) {
		pr_err("%s: NULL pointer error!\n", __func__);
		return -EINVAL;
	}

	ret = i2c_transfer(g_client->adapter, msgs, 1);
	if (ret < 0) {
		pr_err("apds990x %s: transfer error\n", __func__);
		if (g_i2c_error_alarmed == false) {
			if (length <= 1) {
				info1 |= (0x01 << 2); 
			} else {
				info1 |= (0x02 << 2); 
			}
			pr_err("[T][ARM]Event:0x61 Info:0x%02x%02x%02x%02x\n",
				info1, (-ret) & 0xFF, txData[0],
				((length > 1) ? txData[1] : 0));
			g_i2c_error_alarmed = true;
		}
		return -EIO;
	} else {
		g_i2c_error_alarmed = false;
	}

	return 0;
}










static int write_reg_byte(unsigned char addr, unsigned char data)
{
	unsigned char buff[2];

	buff[0] = CMD_BYTE | (addr & 0x1F);
	buff[1] = data;

	return i2c_tx_data(buff, 2);
}










static int write_reg_word(unsigned char addr, unsigned int data)
{
	unsigned char buff[3];

	buff[0] = CMD_WORD | (addr & 0x1F);
	buff[1] = data;
	buff[2] = (data >> 8);

	return i2c_tx_data(buff, 3);
}










static int read_reg_byte(unsigned char addr, unsigned char *data)
{
	int ret = 0;
	unsigned char buff[1];

	if (unlikely(data == NULL)) {
		pr_err("%s: NULL pointer error!\n", __func__);
		return -EINVAL;
	}

	buff[0] = CMD_BYTE | (addr & 0x1F);

	ret =  i2c_rx_data(buff, 1);

	if (!ret) {
		*data = buff[0];
	}

	return ret;
}










static int read_reg_word(unsigned char addr, unsigned int *data)
{
	int ret = 0;
	unsigned char buff[2];

	if (unlikely(data == NULL)) {
		pr_err("%s: NULL pointer error!\n", __func__);
		return -EINVAL;
	}

	buff[0] = CMD_WORD | (addr & 0x1F);
	buff[1] = 0;

	ret =  i2c_rx_data(buff, 2);
	if (!ret) {
		*data = buff[0] + ((unsigned int)buff[1] << 8);
	}

	return ret;
}










static int read_reg_4byte( unsigned char addr, unsigned int *data )
{
    int ret = 0;
    unsigned char buff[4];
    unsigned int  nTemp, nSize;

    nSize = sizeof( nTemp );

    if( unlikely( data == NULL ) )
    {
        pr_err( "%s: NULL pointer error!\n", __func__ );
        return -EINVAL;
    }

    buff[0] = CMD_WORD | ( addr & 0x1F );
    buff[1] = 0;
    buff[2] = 0;
    buff[3] = 0;

    ret = i2c_rx_data( buff, 4 );
    if( ret == 0 )
    {
        
        data[0] = buff[0] + ( (unsigned int)buff[1] << 8 );
        data[1] = buff[2] + ( (unsigned int)buff[3] << 8 );
    }

    return ret;
}











static int set_state(int state)
{
	int ret = 0;

	switch (state) {
	case STATE_POWEROFF:
		ret = write_reg_byte(APDS990X_ENABLE_REG, STATE_POWEROFF);
		if (ret) {
			pr_err("%s: Failed to set enable register!\n", __func__);
			return ret;
		}
		break;
	case STATE_ALS_ON:
		ret = write_reg_byte(APDS990X_WTIME_REG, NV_WTIME2);
		if (ret) {
			pr_err("%s: Failed to set wtime register!\n", __func__);
			return ret;
		}
		ret = write_reg_byte(APDS990X_PERS_REG, NV_PERS);
		if (ret) {
			pr_err("%s: Failed to set persitence register!\n", __func__);
			return ret;
		}
		ret = write_reg_byte(APDS990X_ENABLE_REG, STATE_ALS_ON);
		if (ret) {
			pr_err("%s: Failed to set enable register!\n", __func__);
			return ret;
		}
		g_data.meas_cycle = ALS_WAIT;
		break;
	case STATE_PROX_ON:
		ret = write_reg_byte(APDS990X_WTIME_REG, NV_WTIME);
		if (ret) {
			pr_err("%s: Failed to set wtime register!\n", __func__);
			return ret;
		}
		
		ret = write_reg_word(APDS990X_AIHTL_REG, g_data.als_miss_threshold);
		if (ret) {
			pr_err("%s: Failed to set aiht register!\n", __func__);
			return ret;
		}
		


		ret = write_reg_byte(APDS990X_PERS_REG, 0x0F);
		if (ret) {
			pr_err("%s: Failed to set persitence register!\n", __func__);
			return ret;
		}

		g_distance = D_PS_DETECT_STATE_BUSY;
		interrupt_work_func = interrupt_work_func_f1;
		g_data.meas_cycle = ALS_PS_WAIT;

		ret = write_reg_byte(APDS990X_ENABLE_REG, STATE_PROX_ON);
		if (ret) {
			pr_err("%s: Failed to set enable register!\n", __func__);
			return ret;
		}
		break;
	default:
		pr_err ("%s: Invalid state: %d!\n", __func__, state);
		return -EINVAL;
	}

	

	g_state = state;

	return 0;
}









static int clear_int(unsigned char command)
{
	unsigned char buff = command;

	return i2c_tx_data(&buff, 1);
}







static void init_data(void)
{
	int AGAIN = 1;

	
	switch (NV_CONTROL & 0x03) {
	case 0x01:
		AGAIN = 8;
		break;
	case 0x02:
		AGAIN = 16;
		break;
	case 0x03:
		AGAIN = 120;
		break;
	}

	
	g_data.als_max_count = 1024 * (256 - NV_ATIME) - 1;
	g_data.als_saturates = (g_data.als_max_count < 65535) ? g_data.als_max_count : 65535;
	g_data.als_miss_threshold = g_data.als_max_count * NV_DET_CANCEL / 100;
	g_data.ps_d_threshold_l = NV_PILT;
	g_data.ps_d_threshold_h = NV_H_PIHT;
	g_data.ps_n_threshold_l = NV_L_PILT;
	g_data.ps_n_threshold_h = NV_PIHT;
	
	if (g_prox_open_count > 0) {
		g_data.meas_cycle = ALS_PS_WAIT;
	} else {
		g_data.meas_cycle = ALS_WAIT;
	}
	g_data.LPC = (NV_GA * NV_CO_K * DEVICE_FACTOR * 10) / (NV_ALSIT * AGAIN);
}








static int als_start(void)
{
	int ret = 0;

	if (g_als_open_count > 0) {
		
		g_als_open_count++;
		return 0;
	}

	if (g_prox_open_count == 0) {
		

		ret = set_state(STATE_ALS_ON);
		if (ret) {
			pr_err("%s: Failed to set state to STATE_ALS_ON!\n", __func__);
			return ret;
		}

		


		msleep(g_data.meas_cycle * 2);
	}

	g_als_open_count = 1;
	

	cancel_delayed_work_sync(&g_als_poll_work);
	
	g_ch0_buff[LIGHT_AVERAGE_NUMBER - 1] = (unsigned int)(-1);
	
	g_als_current = 7;
	
	als_poll_work_func(&g_als_poll_work.work);

	return 0;
}








static int als_stop(void)
{
	int ret = 0;

	g_als_open_count--;
	if (g_als_open_count > 0) {
		
		return 0;
	}

	g_als_open_count = 0;
	
	cancel_delayed_work_sync(&g_als_poll_work);
	if (g_prox_open_count == 0) {
		ret = set_state(STATE_POWEROFF);
		if (ret) {
			pr_err("%s: Failed to set state to STATE_POWEROFF!\n", __func__);
			g_als_open_count++;
			return ret;
		}
	}

	return 0;
}








static int prox_start(void)
{
	int ret = 0;

	if (g_prox_open_count > 0) {
		g_prox_open_count++;
		return 0;
	}

	ret = set_state(STATE_PROX_ON);
	if (ret) {
		pr_err("%s: Failed to set state to STATE_PROX_ON!\n", __func__);
		return ret;
	}

	g_prox_open_count = 1;
	
	msleep(g_data.meas_cycle * 2);

	return 0;
}








static int prox_stop(void)
{
	int ret = 0;

	g_prox_open_count--;
	if (g_prox_open_count > 0) {
		return 0;
	}

	g_prox_open_count = 0;
	if (g_als_open_count == 0) {
		ret = set_state(STATE_POWEROFF);
		if (ret) {
			pr_err("%s: Failed to set state to STATE_POWEROFF!\n", __func__);
			g_prox_open_count++;
		}
	} else {
		ret = set_state(STATE_ALS_ON);
		if (ret) {
			pr_err("%s: Failed to set state to STATE_ALS_ON!\n", __func__);
			g_prox_open_count++;
		}
	}

	return ret;
}














static int diag_calc_restart_wait(void)
{
    if(WTIME_WAIT > WTIME2_WAIT)
    {
        return RESTART_WAIT1;
    }

    return RESTART_WAIT2;
}









static int diag_restart_ready(void)
{
    int ret = 0;

    
    ret = write_reg_byte(APDS990X_ENABLE_REG, STATE_POWEROFF);
    if( ret != 0 )
    {
        pr_err("%s: Failed to set enable register!\n", __func__);
        return ret;
    }

    
    msleep( diag_calc_restart_wait() );
    return ret;
}








static int diag_prox_start(void)
{
    int ret = 0;


    

    ret = diag_restart_ready();
    if( ret != 0 )
    {
        pr_err("%s: diag_restart_ready() Failed. !\n", __func__);
        return ret;
    }

    ret = write_reg_byte(APDS990X_ATIME_REG, NV_ATIME);
    if( ret != 0 )
    {
        pr_err("%s: Failed to set atime register!\n", __func__);
        return ret;
    }

    ret = write_reg_byte(APDS990X_PTIME_REG, NV_PTIME);
    if( ret != 0 )
    {
        pr_err("%s: Failed to set ptime register!\n", __func__);
        return ret;
    }

    ret = write_reg_byte(APDS990X_WTIME_REG, NV_WTIME);
    if( ret != 0 )
    {
        pr_err("%s: Failed to set wtime register!\n", __func__);
        return ret;
    }

    ret = write_reg_byte(APDS990X_CONFIG_REG, NV_CONFIG);
    if( ret != 0 )
    {
        pr_err("%s: Failed to set config register!\n", __func__);
        return ret;
    }

    ret = write_reg_byte(APDS990X_PPCOUNT_REG, NV_PPCOUNT);
    if( ret != 0 )
    {
        pr_err("%s: Failed to set ppcount register!\n", __func__);
        return ret;
    }

    ret = write_reg_byte(APDS990X_CONTROL_REG, NV_CONTROL);
    if( ret != 0 )
    {
        pr_err("%s: Failed to set control register!\n", __func__);
        return ret;
    }

    

    
    ret = write_reg_byte(APDS990X_PERS_REG, 0x00);
    if( ret != 0 )
    {
        pr_err("%s: Failed to set persistance register!\n", __func__);
        return ret;
    }

    interrupt_work_func = diag_interrupt_work_func_f1;

    
    ret = write_reg_byte(APDS990X_ENABLE_REG, STATE_PROX_DIAG_ON);
    if( ret != 0 )
    {
        pr_err("%s: Failed to set enable register!\n", __func__);
        return ret;
    }

    return 0;
}








static int diag_prox_stop(void)
{
    int ret = 0;

    ret = write_reg_byte(APDS990X_ENABLE_REG, STATE_POWEROFF);
    if( ret != 0 )
    {
        pr_err("%s: Failed to set enable register!\n", __func__);
        return ret;
    }

    return 0;
}












static int diag_als_start(void)
{
    int ret = 0;


    

    ret = diag_restart_ready();
    if (ret) {
        pr_err("%s: diag_restart_ready() Failed. !\n", __func__);
        return ret;
    }

    ret = write_reg_byte(APDS990X_ATIME_REG, NV_ATIME);
    if (ret) {
        pr_err("%s: Failed to set atime register!\n", __func__);
        return ret;
    }

    ret = write_reg_byte(APDS990X_PTIME_REG, NV_PTIME);
    if (ret) {
        pr_err("%s: Failed to set ptime register!\n", __func__);
        return ret;
    }

    ret = write_reg_byte(APDS990X_WTIME_REG, NV_WTIME2);
    if (ret) {
        pr_err("%s: Failed to set wtime register!\n", __func__);
        return ret;
    }

    ret = write_reg_byte(APDS990X_CONFIG_REG, NV_CONFIG);
    if (ret) {
        pr_err("%s: Failed to set config register!\n", __func__);
        return ret;
    }

    ret = write_reg_byte(APDS990X_PPCOUNT_REG, NV_PPCOUNT);
    if (ret) {
        pr_err("%s: Failed to set ppcount register!\n", __func__);
        return ret;
    }

    ret = write_reg_byte(APDS990X_CONTROL_REG, NV_CONTROL);
    if (ret) {
        pr_err("%s: Failed to set control register!\n", __func__);
        return ret;
    }

    

    ret = write_reg_byte(APDS990X_PERS_REG, NV_PERS);
    if (ret) {
        pr_err("%s: Failed to set persitence register!\n", __func__);
        return ret;
    }

    ret = write_reg_byte(APDS990X_ENABLE_REG, STATE_ALS_ON);
    if (ret) {
        pr_err("%s: Failed to set enable register!\n", __func__);
        return ret;
    }

    return 0;
}

static int diag_als_stop(void)
{
    int ret = 0;

    ret = write_reg_byte(APDS990X_ENABLE_REG, STATE_POWEROFF);
    if (ret) {
        pr_err("%s: Failed to set enable register!\n", __func__);
        return ret;
    }

    return 0;
}

static void diag_interrupt_work_func_f1(void)
{
    int ret = 0;
    unsigned char status = 0;

    
    ret = read_reg_byte(APDS990X_STATUS_REG, &status);
    if (ret) {
        pr_err("%s: Failed to read status register!\n", __func__);
    } else {
        
        if (status & 0x20) {
            schedule();
            wake_up(&g_diag_data_queue);
        }
    }

    clear_int(CMD_CLR_PS_ALS_INT);
}

static void diag_interrupt_work_func_f2(void)
{
    int ret = 0;
    unsigned char status = 0;

    ret = read_reg_byte(APDS990X_STATUS_REG, &status);
    if(ret) {
        pr_err("%s: Failed to read status register!\n", __func__);
        return;
    }

    if(status & 0x20) {
        schedule();
        wake_up(&g_diag_data_queue);
    } else {
        clear_int(CMD_CLR_PS_ALS_INT);
    }
}

static int diag_get_start(apds990x_diagget_mode mode)
{
    int ret = 0;

    switch(mode) {
        case APDS990X_DIAGGET_PROX:
            ret = diag_prox_start();
            break;

        case APDS990X_DIAGGET_LUX:
            ret = diag_als_start();
            break;

        default:
            break;
    }

    if(ret)
    {
        pr_err("%s: Failed to diag start!\n", __func__);
        return ret;
    }

    return 0;
}

static int diag_get_lux_data(struct apds990x_diag_data_once *data)
{
    int ret;
    unsigned int ch_data[2];

    (void)memset( ch_data, 0, 2 );

    
    ret = read_reg_4byte( APDS990X_CDATAL_REG, ch_data );
    if( ret != 0 )
    {
        pr_err( "%s: Failed to read ch0/1 data!\n", __func__ );
        return ret;
    }
    data->CH0_data = ch_data[0];
    data->CH1_data = ch_data[1];

    return 0;
}

static int diag_get_prox_data(struct apds990x_diag_data_once *data)
{
    int ret;
    unsigned int ch_data[2];

    (void)memset( ch_data, 0, 2 );

    
    ret = read_reg_4byte( APDS990X_CDATAL_REG, ch_data );
    if( ret != 0 )
    {
        pr_err( "%s: Failed to read ch0/1 data!\n", __func__ );
        return ret;
    }
    data->CH0_data = ch_data[0];
    data->CH1_data = ch_data[1];

    
    ret = read_reg_word( APDS990X_PDATAL_REG, &data->prox_data );
    if( ret != 0 )
    {
        pr_err("%s: Failed to read proximity data!\n", __func__);
        return ret;
    }

    return 0;
}


static int diag_get_data_once(struct apds990x_diag_data_once *data, apds990x_diagget_mode mode)
{
	switch(mode) {
		case APDS990X_DIAGGET_PROX:
			return diag_get_prox_data(data);

		case APDS990X_DIAGGET_LUX:
			return diag_get_lux_data(data);

		default:
			break;
	}

	pr_err("%s: diag unknown mode(%d).!\n", __func__, mode);
	return -EINVAL;
}

static int diag_get_end(apds990x_diagget_mode mode)
{


	const int stop_count = 5;
	int i;
	int ret = 0;

	switch(mode) {
		case APDS990X_DIAGGET_PROX:
			for(i = 0; i < stop_count && ret != 0; i++, msleep(1)) {
				ret = diag_prox_stop();
			}
			break;

		case APDS990X_DIAGGET_LUX:
			for(i = 0; i < stop_count && ret != 0; i++, msleep(1)) {
				ret = diag_als_stop();
			}
			break;

		default:
			pr_err("%s: diag unknown mode(%d).!\n", __func__, mode);
			ret = -EINVAL;
			break;
	}

	if(ret)
	{
		pr_err("%s: Failed to diag stop!\n", __func__);
		goto END_LABEL;
	}

END_LABEL:
	ret = set_state(g_state);
	if (ret) {
		pr_err("%s: Failed to set state!\n", __func__);
		return ret;
	}

	return 0;
}

static int diag_get_datan(struct apds990x_diag_data *data, struct apds990x_diag_data_once *store_data)
{
	int ret = 0;
	unsigned int i;

	if (data == NULL || store_data == NULL) {
		pr_err("%s: NULL pointer error!\n", __func__);
		return -EINVAL;
	}

	
	if(g_als_open_count > 0) {
		cancel_delayed_work_sync(&g_als_poll_work);
	}

	
	
	ret = diag_get_start(data->mode);
	if(ret != 0)
	{
		pr_err("%s: diag init failed.!\n", __func__);
		goto END_LABEL;
	}

	
	if(data->mode == APDS990X_DIAGGET_PROX) {

		
		if(interruptible_sleep_on_timeout(&g_diag_data_queue, HZ) == 0) {
			pr_err("%s: interruptible_sleep_on_timeout time out f1\n", __func__);
			ret = -ETIMEDOUT;
			goto END_LABEL;
		}

		interrupt_work_func = diag_interrupt_work_func_f2;
		for(i = 0;;) {
			if(interruptible_sleep_on_timeout(&g_diag_data_queue, HZ) == 0) {
				pr_err("%s: interruptible_sleep_on_timeout time out f2\n", __func__);
				ret = -ETIMEDOUT;
			} else {
				ret = diag_get_data_once(&store_data[i], data->mode);
			}

			if(ret != 0) {
				pr_err("%s: diag_get_datan failed.!\n", __func__);
				goto END_LABEL;
			}

			clear_int(CMD_CLR_PS_ALS_INT);

		    i++;
			if(i >= data->count) {
			    
				break;
			}
		}
	} else if(data->mode == APDS990X_DIAGGET_LUX) {

		
		
		msleep(ALS_WAIT);
		ret = diag_get_data_once(&store_data[0], data->mode);
		if(ret != 0)
		{
			pr_err("%s: diag end failed.!\n", __func__);
			goto END_LABEL;
		}
	} else {
		pr_err("%s: diag unknown mode(%d).!\n", __func__, data->mode);
		ret = -EINVAL;
		goto END_LABEL;
	}

END_LABEL:

	
	if(g_als_open_count > 0) {
		als_poll_work_func(&g_als_poll_work.work);
	}

	if(ret != 0)
	{
		diag_get_end(data->mode);
		pr_err("%s: diag_get_datan failed.!\n", __func__);
		return ret;
	}

	ret = diag_get_end(data->mode);
	if(ret != 0)
	{
		pr_err("%s: diag end failed.!\n", __func__);
		return ret;
	}

	return ret;
}






static int init_device(void)
{
	int ret = 0;

	pr_debug("%s: entry!\n", __func__);
	init_data();
	ret = write_reg_byte(APDS990X_ENABLE_REG, STATE_POWEROFF);
	if (ret) {
		pr_err("%s: Failed to set enable register!\n", __func__);
		return ret;
	}
	ret = write_reg_byte(APDS990X_ATIME_REG, NV_ATIME);
	if (ret) {
		pr_err("%s: Failed to set atime register!\n", __func__);
		return ret;
	}
	ret = write_reg_byte(APDS990X_PTIME_REG, NV_PTIME);
	if (ret) {
		pr_err("%s: Failed to set ptime register!\n", __func__);
		return ret;
	}
	ret = write_reg_byte(APDS990X_PERS_REG, NV_PERS);
	if (ret) {
		pr_err("%s: Failed to set persistance register!\n", __func__);
		return ret;
	}
	ret = write_reg_byte(APDS990X_CONFIG_REG, NV_CONFIG);
	if (ret) {
		pr_err("%s: Failed to set config register!\n", __func__);
		return ret;
	}
	ret = write_reg_byte(APDS990X_PPCOUNT_REG, NV_PPCOUNT);
	if (ret) {
		pr_err("%s: Failed to set ppcount register!\n", __func__);
		return ret;
	}
	ret = write_reg_byte(APDS990X_CONTROL_REG, NV_CONTROL);
	if (ret) {
		pr_err("%s: Failed to set control register!\n", __func__);
		return ret;
	}
	ret = set_state(g_state);
	if (ret) {
		pr_err("%s: Failed to set state!\n", __func__);
		return ret;
	}

	return 0;
}








static void als_poll_work_func(struct work_struct *work)
{
    int ret = 0;
    unsigned int ch_data[2];

    (void)memset( ch_data, 0, 2 );

    
    ret = read_reg_4byte( APDS990X_CDATAL_REG, ch_data );
    if( ret != 0 )
    {
        pr_err( "%s: Failed to read ch0/1 data!\n", __func__ );
        goto exit;
    }

	
	spin_lock(&g_spinlock);
	g_als_current = (g_als_current + 1) & 0x07;
	g_ch0_buff[g_als_current] = ch_data[0];
	g_ch1_buff[g_als_current] = ch_data[1];
	spin_unlock(&g_spinlock);

    if( (ch_data[0] >= g_data.als_miss_threshold ) && ( g_prox_open_count > 0 ) )
    {
		
		write_reg_word(APDS990X_PILTL_REG, g_data.ps_n_threshold_l);
		write_reg_word(APDS990X_PIHTL_REG, g_data.ps_n_threshold_h);
		g_distance = D_PS_DETECT_STATE_NON_DETECT;
	}

exit:
	
	if (ret) {
		g_light_err = D_I2C_ERR;
	} else {
		g_light_err = D_I2C_NO_ERR;
	}

	
	schedule_delayed_work(&g_als_poll_work, msecs_to_jiffies(g_data.meas_cycle));
}




static void interrupt_work(struct work_struct *work)
{
	interrupt_work_func();
}




static void interrupt_work_func_f1(void)
{
	int ret = 0;
	unsigned char status = 0;

	ret = read_reg_byte(APDS990X_STATUS_REG, &status);
	if (ret) {
		pr_err("%s: Failed to read status register!\n", __func__);
		goto exit;
	}

	
	if (status & 0x20) {
		interrupt_work_func = interrupt_work_func_f2;
	}

exit:
	
	if (ret) {
		g_prox_err = D_I2C_ERR;
	} else {
		g_prox_err = D_I2C_NO_ERR;
	}
	


	clear_int(CMD_CLR_PS_ALS_INT);
}




static void interrupt_work_func_f2(void)
{
	int ret = 0;
	unsigned char status = 0;
	unsigned int ch0 = 0;
	unsigned int pdata = 0;

	ret = read_reg_byte(APDS990X_STATUS_REG, &status);
	if (ret) {
		pr_err("%s: Failed to read status register!\n", __func__);
		goto exit;
	}

	
	if (status & 0x20) {
		ret = read_reg_word(APDS990X_CDATAL_REG, &ch0);
		if (ret) {
			pr_err("%s: Failed to read ch0 data register!\n", __func__);
			goto exit;
		}
		ret = read_reg_word(APDS990X_PDATAL_REG, &pdata);
		if (ret) {
			pr_err("%s: Failed to read proximity data register!\n", __func__);
			goto exit;
		}
		
		if ((ch0 < g_data.als_miss_threshold) && (pdata >= NV_PIHT)) {
			
			ret = write_reg_word(APDS990X_PILTL_REG, g_data.ps_d_threshold_l);
			if (ret) {
				pr_err("%s: Failed to set proximity low interrupt threshold!\n", __func__);
				goto exit;
			}
			ret = write_reg_word(APDS990X_PIHTL_REG, g_data.ps_d_threshold_h);
			if (ret) {
				pr_err("%s: Failed to set proximity high interrupt threshold!\n", __func__);
				goto exit;
			}
			g_distance = D_PS_DETECT_STATE_DETECT;
		} else {
			
			ret = write_reg_word(APDS990X_PILTL_REG, g_data.ps_n_threshold_l);
			if (ret) {
				pr_err("%s: Failed to set proximity low interrupt threshold!\n", __func__);
				goto exit;
			}
			ret = write_reg_word(APDS990X_PIHTL_REG, g_data.ps_n_threshold_h);
			if (ret) {
				pr_err("%s: Failed to set proximity high interrupt threshold!\n", __func__);
				goto exit;
			}
			g_distance = D_PS_DETECT_STATE_NON_DETECT;
		}

		ret = write_reg_byte(APDS990X_PERS_REG, NV_PERS);
		if (ret) {
			pr_err("%s: Failed to set persitence register!\n", __func__);
			goto exit;
		}
		interrupt_work_func = interrupt_work_func_f3;
	}

exit:
	
	if (ret) {
		g_prox_err = D_I2C_ERR;
	} else {
		g_prox_err = D_I2C_NO_ERR;
	}
	


	clear_int(CMD_CLR_PS_ALS_INT);
}




static void interrupt_work_func_f3(void)
{
	int ret = 0;
	unsigned char status = 0;
	unsigned int ch0 = 0;
	unsigned int pdata = 0;

	ret = read_reg_byte(APDS990X_STATUS_REG, &status);
	if (ret) {
		pr_err("%s: Failed to read status register!\n", __func__);
		goto exit;
	}

	
	if ((status & 0x20) || (status & 0x10)) {
		ret = read_reg_word(APDS990X_CDATAL_REG, &ch0);
		if (ret) {
			pr_err("%s: Failed to read ch0 data register!\n", __func__);
			goto exit;
		}
		ret = read_reg_word(APDS990X_PDATAL_REG, &pdata);
		if (ret) {
			pr_err("%s: Failed to read proximity data register!\n", __func__);
			goto exit;
		}
		
		if ((ch0 < g_data.als_miss_threshold) && (pdata >= NV_PIHT)) {
			
			ret = write_reg_word(APDS990X_PILTL_REG, g_data.ps_d_threshold_l);
			if (ret) {
				pr_err("%s: Failed to set proximity low interrupt threshold!\n", __func__);
				goto exit;
			}
			ret = write_reg_word(APDS990X_PIHTL_REG, g_data.ps_d_threshold_h);
			if (ret) {
				pr_err("%s: Failed to set proximity high interrupt threshold!\n", __func__);
				goto exit;
			}
			g_distance = D_PS_DETECT_STATE_DETECT;
		} else if ((ch0 >= g_data.als_miss_threshold) || (pdata <= NV_PILT)){
			
			ret = write_reg_word(APDS990X_PILTL_REG, g_data.ps_n_threshold_l);
			if (ret) {
				pr_err("%s: Failed to set proximity low interrupt threshold!\n", __func__);
				goto exit;
			}
			ret = write_reg_word(APDS990X_PIHTL_REG, g_data.ps_n_threshold_h);
			if (ret) {
				pr_err("%s: Failed to set proximity high interrupt threshold!\n", __func__);
				goto exit;
			}
			g_distance = D_PS_DETECT_STATE_NON_DETECT;
		}
	}

exit:
	
	if (ret) {
		g_prox_err = D_I2C_ERR;
		interrupt_work_func = interrupt_work_func_f1;
	} else {
		g_prox_err = D_I2C_NO_ERR;
	}
	


	clear_int(CMD_CLR_PS_ALS_INT);
}




static irqreturn_t apds990x_interrupt_handler(int irq, void *dev)
{

    spin_lock(&g_interrupt_spinlock);
    if (g_suspend_state == false) {
        schedule_work(&g_interrupt_work);
    }
    else {
        pr_debug("%s: start! g_state[0x%x] g_irq_occurred[%d]\n", __func__, g_state, g_irq_occurred);
        if (g_irq_occurred)
            pr_err("%s: g_irq_occurred[%d] is already set!\n", __func__, g_irq_occurred);
        wake_lock(&g_interrupt_wake_lock);
        g_irq_occurred = true;
    }
    spin_unlock(&g_interrupt_spinlock);

	return IRQ_HANDLED;
}




static long apds990x_ioctl(
			struct file *file,
			unsigned int cmd,
			unsigned long arg)
{
	long ret = -EINVAL;
	unsigned char reg_addr = 0;
	unsigned char reg_data = 0;
	void __user *argp = (void __user *)arg;
	struct apds990x_diag_data diag_data;
	struct apds990x_light_data light_data;
	struct apds990x_diag_data_once *diag_store_data;
	size_t diag_data_size;

	pr_debug("%s: entry!\n", __func__);
	if(
		cmd == APDS990X_IOCTL_DIAG_SET_NV ||
		cmd == APDS990X_IOCTL_DIAG_GET_NV ||
		cmd == APDS990X_IOCTL_DIAG_GET_DATA
	) {
		
	} else if (unlikely((g_is_nv_set == false) && (cmd != APDS990X_IOCTL_SET_NV))) {
		pr_err("%s: NV items has not been set!\n", __func__);
		return -EAGAIN;
	}

	switch (cmd) {
	case APDS990X_IOCTL_SET_NV:
		mutex_lock(&g_mutex);
		g_is_nv_set = false;
		ret = copy_from_user(&g_nv_data, argp, sizeof(struct apds990x_nv_data));
		if (ret) {
			pr_err("%s: Failed to copy data from user!\n", __func__);
			mutex_unlock(&g_mutex);
			return -EFAULT;
		}
		ret = init_device();
		if (ret) {
			pr_err("%s: Failed to init device!\n", __func__);
			mutex_unlock(&g_mutex);
			return ret;
		}
		g_is_nv_set = true;
		mutex_unlock(&g_mutex);
		break;

	case APDS990X_IOCTL_GET_NV:
		mutex_lock(&g_mutex);
		ret = copy_to_user(argp, &g_nv_data, sizeof(struct apds990x_nv_data));
		if (ret) {
			pr_err("%s: Failed to copy data to user!\n", __func__);
			mutex_unlock(&g_mutex);
			return -EFAULT;
		}
		mutex_unlock(&g_mutex);
		break;

	case APDS990X_IOCTL_LIGHT_MEAS_START:
		mutex_lock(&g_mutex);
		ret = als_start();
		mutex_unlock(&g_mutex);
		break;

	case APDS990X_IOCTL_LIGHT_MEAS_STOP:
		mutex_lock(&g_mutex);
		ret = als_stop();
		mutex_unlock(&g_mutex);
		break;

	case APDS990X_IOCTL_GET_LUX:
		if (unlikely(g_light_err == D_I2C_ERR)) {
			pr_err("%s: I2C transfer error!\n", __func__);
			return -EIO;
		}
		memset(&light_data, 0, sizeof(struct apds990x_light_data));
		ret = copy_from_user(&light_data, argp, sizeof(struct apds990x_light_data));
		if (ret) {
			pr_err("%s: Failed to copy data from user!\n", __func__);
			return -EFAULT;
		}
		ret = als_get_lux(&light_data);
		if (ret) {
			
			if (ret != -EBUSY) {
				pr_err("%s: Failed to get lux!\n", __func__);
			}
			return ret;
		}
		ret = copy_to_user(argp, &light_data, sizeof(struct apds990x_light_data));
		if (ret) {
			pr_err("%s: Failed to copy data to user!\n", __func__);
			return -EFAULT;
		}
		break;

	case APDS990X_IOCTL_PROX_MEAS_START:
		mutex_lock(&g_mutex);
		ret = prox_start();
		mutex_unlock(&g_mutex);
		break;

	case APDS990X_IOCTL_PROX_MEAS_STOP:
		mutex_lock(&g_mutex);
		ret = prox_stop();
		mutex_unlock(&g_mutex);
		break;

	case APDS990X_IOCTL_GET_DISTANCE:
		if (unlikely(g_prox_err == D_I2C_ERR)) {
			pr_err("%s: I2C transfer error!\n", __func__);
			write_reg_byte(APDS990X_PERS_REG, 0x0F);
			clear_int(CMD_CLR_PS_ALS_INT);
			return -EIO;
		}
		if (g_distance == D_PS_DETECT_STATE_BUSY)
			return -EBUSY;

		ret = copy_to_user(argp, &g_distance, sizeof(int));
		if (ret) {
			pr_err("%s: Failed to copy data to user!\n", __func__);
			return -EFAULT;
		}
		break;

	case APDS990X_IOCTL_DIAG_GET_DATA:
		mutex_lock(&g_mutex);
		diag_store_data = NULL;
		memset(&diag_data, 0, sizeof(struct apds990x_diag_data));

		ret = copy_from_user(&diag_data, argp, sizeof(struct apds990x_diag_data));
		if (ret) {
			pr_err("%s: Failed to copy data from user!\n", __func__);
			ret = -EFAULT;
			goto DIAG_GET_DATA_END;
		}

		diag_data_size = sizeof(struct apds990x_diag_data_once) * diag_data.count;

		diag_store_data = kmalloc(diag_data_size, GFP_KERNEL);
		if (diag_store_data == NULL) {
			pr_err("%s: Failed to kmalloc(%zd)!\n", __func__, diag_data_size);
			ret = -ENOMEM;
			goto DIAG_GET_DATA_END;
		}
		memset(diag_store_data, 0x00, diag_data_size);

		ret = diag_get_datan(&diag_data, diag_store_data);
		if (ret) {
			pr_err("%s: Failed to get diag data!\n", __func__);
			goto DIAG_GET_DATA_END;
		}

		ret = copy_to_user(diag_data.data, diag_store_data, diag_data_size);
		if (ret) {
			pr_err("%s: Failed to copy data to user!\n", __func__);
			ret = -EFAULT;
			goto DIAG_GET_DATA_END;
		}

DIAG_GET_DATA_END:
		if(diag_store_data != NULL) {
			kfree(diag_store_data);
			diag_store_data = NULL;
		}
		mutex_unlock(&g_mutex);
		break;

	case APDS990X_IOCTL_DIAG_GET_REGISTER:
		ret = copy_from_user(&reg_addr, argp, sizeof(unsigned char));
		if (ret) {
			pr_err("%s: Failed to copy data from user!\n", __func__);
			return -EFAULT;
		}
		if (reg_addr > APDS990X_MAX_REG) {
			pr_err("%s: The register address requested is out of range!\n", __func__);
			return -EINVAL;
		}
		ret = read_reg_byte(reg_addr, &reg_data);
		if (ret) {
			pr_err("%s: Failed to read register data!\n", __func__);
			return ret;
		}
		ret = copy_to_user(argp, &reg_data, sizeof(unsigned char));
		if (ret) {
			pr_err("%s: Failed to copy data to user!\n", __func__);
			return -EFAULT;
		}
		break;

	case APDS990X_IOCTL_DIAG_SET_NV:
		
		mutex_lock(&g_mutex);
		ret = copy_from_user(&g_nv_data, argp, sizeof(struct apds990x_nv_data));
		if (ret) {
			pr_err("%s: diag: Failed to copy data from user!\n", __func__);
			mutex_unlock(&g_mutex);
			return -EFAULT;
		}
		mutex_unlock(&g_mutex);
		break;

	case APDS990X_IOCTL_DIAG_GET_NV:
		
		mutex_lock(&g_mutex);
		ret = copy_to_user(argp, &g_nv_data, sizeof(struct apds990x_nv_data));
		if (ret) {
			pr_err("%s: diag: Failed to copy data to user!\n", __func__);
			mutex_unlock(&g_mutex);
			return -EFAULT;
		}
		mutex_unlock(&g_mutex);
		break;

	default:
		pr_err("%s: invalid ioctl command %d\n", __func__, cmd);
	}

	return ret;
}




static int apds990x_open(struct inode *inode, struct file *file)
{
	return 0;
}




static int apds990x_release(struct inode *inode, struct file *file)
{
	return 0;
}

static struct file_operations apds990x_fops = {
	.owner	 = THIS_MODULE,
	.open	 = apds990x_open,
	.release = apds990x_release,
	.unlocked_ioctl	 = apds990x_ioctl,
};

static struct miscdevice apds990x_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "light_prox_sensor",
	.fops  = &apds990x_fops,
};







static int __devinit apds990x_probe(struct i2c_client *client,
				    const struct i2c_device_id *id)
{
	int ret;

	g_client = client;
	i2c_set_clientdata(client, &g_data);
	

	msleep(5);
	ret = set_state(STATE_POWEROFF);
	if (ret) {
		pr_err("%s: Failed to set state to STATE_POWEROFF!", __func__);
		return ret;
	}

	
	interrupt_work_func = interrupt_work_func_f1;

    wake_lock_init(&g_interrupt_wake_lock, WAKE_LOCK_SUSPEND, "apds9900");

	ret = request_irq(client->irq, apds990x_interrupt_handler,
			  IRQ_TYPE_EDGE_FALLING,
			  "apds9900", (void *)client);
	if (ret) {
		pr_err("%s: Could not allocate APDS990x_INT !\n", __func__);
		return ret;
	}

	irq_set_irq_wake(client->irq, 1);

	ret = misc_register(&apds990x_device);
	if (ret) {
		pr_err("%s: misc_register failed\n", __func__);
		free_irq(client->irq, client);
		return ret;
	}
	pr_info("%s: successed!\n", __func__);

	return 0;
}

static int __devexit apds990x_remove(struct i2c_client *client)
{
	int ret;
	ret = set_state(STATE_POWEROFF);
	if (ret) {
		pr_err("%s: Failed to set state to STATE_POWEROFF!\n", __func__);
		return ret;
	}
	misc_deregister(&apds990x_device);
	free_irq(g_client->irq, client);

    wake_lock_destroy(&g_interrupt_wake_lock);

	cancel_work_sync(&g_interrupt_work);
	cancel_delayed_work_sync(&g_als_poll_work);

	return 0;
}


static int apds990x_suspend(struct device *dev)
{
	int ret = 0;

    spin_lock(&g_interrupt_spinlock);
    g_suspend_state = true;
    g_irq_occurred = false;
    spin_unlock(&g_interrupt_spinlock);

	mutex_lock(&g_mutex);

	if (g_state != STATE_POWEROFF) {
		cancel_delayed_work_sync(&g_als_poll_work);









	}
	mutex_unlock(&g_mutex);

	return ret;
}

static int apds990x_resume(struct device *dev)
{
	int ret = 0;
	pr_debug("%s: start! g_state[0x%x] g_irq_occurred[%d]\n", __func__, g_state, g_irq_occurred);

	mutex_lock(&g_mutex);
	if (g_state != STATE_POWEROFF) {









		if (g_als_open_count > 0)
			schedule_delayed_work(&g_als_poll_work,
				msecs_to_jiffies(g_data.meas_cycle * 2));

	}
	mutex_unlock(&g_mutex);

    spin_lock(&g_interrupt_spinlock);
    if (g_irq_occurred == true)
        schedule_work(&g_interrupt_work);

    g_irq_occurred = false;
    g_suspend_state = false;

    if (wake_lock_active(&g_interrupt_wake_lock)) {
        pr_debug("%s: apds wake lock is active.", __func__);
        wake_unlock(&g_interrupt_wake_lock);
    }
    spin_unlock(&g_interrupt_spinlock);

	return ret;
}


static void __devexit apds990x_shutdown(struct i2c_client *client)
{
	apds990x_remove(client);
}

static const struct i2c_device_id apds990x_id[] = {
	{ "apds9900", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, apds990x_id);

static const struct dev_pm_ops apds990x_pm_ops = {

	.suspend = apds990x_suspend,
	.resume = apds990x_resume,

};

static struct i2c_driver apds990x_driver = {
	.probe    = apds990x_probe,
	.remove   = apds990x_remove,




	.shutdown = apds990x_shutdown,
	.id_table = apds990x_id,
	.driver   = {
		.owner = THIS_MODULE,
		.name  = "apds9900",
		.pm   = &apds990x_pm_ops,
	},
};

static int __init apds990x_init(void)
{
	pr_debug("%s: start!\n", __func__);
	return i2c_add_driver(&apds990x_driver);
}

static void __exit apds990x_exit(void)
{
	pr_debug("%s: exit!\n", __func__);
	i2c_del_driver(&apds990x_driver);
}

module_init(apds990x_init);
module_exit(apds990x_exit);

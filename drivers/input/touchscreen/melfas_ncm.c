/*
 * drivers/input/touchscreen/melfas_ncm.c
 *
 * - Touch Screen driver for Melfas MMS Touch Screen sensor IC
 *
 * Copyright (C) NEC CASIO Mobile Communications, Ltd.
 *
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <asm/uaccess.h>

#include <linux/touch_panel_cmd.h>


#include <linux/earlysuspend.h>


#include <linux/melfas_ncm.h>




#include "mms100_cfg_update.h"

#include <linux/oemnc_info.h>




#define MELFAS_NCM_NO_SYSFS





#define MELFAS_NCM_TS_MAX_Z_TOUCH		255
#define MELFAS_NCM_TS_MAX_W_TOUCH		40
#define MELFAS_NCM_TS_MAX_X_COORD 		719
#define MELFAS_NCM_TS_MAX_Y_COORD 		1279
#define MELFAS_NCM_TS_MAX_TOUCH 		11
#define MELFAS_NCM_TS_INPUT_REG_SIZE	6
#define MELFAS_NCM_TS_READ_REGS_LEN		MELFAS_NCM_TS_INPUT_REG_SIZE*MELFAS_NCM_TS_MAX_TOUCH

#define MELFAS_NCM_TS_RTN_VAL_FF		0xFF

#define MELFAS_NCM_TS_FW_MAX_SIZE		64*1024




#define MELFAS_NCM_TS_REG01_ACTIVE		0x02	
#define MELFAS_NCM_TS_REG01_SLEEP		0x00	
#define MELFAS_NCM_TS_REG01_RESET		0x03	

#define MELFAS_NCM_I2C_RETRY_CNT		5
#define MELFAS_PROC_NAME_CTL			"touchpanel_ctl"
#define MELFAS_PROC_NAME_FWUP			"touchpanel_fw_update"


#define MELFAS_REGISTER_MODE_CTL			0x01
#define MELFAS_REGISTER_XY_RESOL			0x02
#define MELFAS_REGISTER_X_RESOL				0x03
#define MELFAS_REGISTER_Y_RESOL				0x04
#define MELFAS_REGISTER_CONTACT_THOLD		0x05
#define MELFAS_REGISTER_MOVING_THOLD		0x06
#define MELFAS_REGISTER_CONFIG_MODE			0x09
#define MELFAS_REGISTER_EXP_POS_MODE		0x0A
#define MELFAS_REGISTER_NUM_OF_TX_CH		0x0B
#define MELFAS_REGISTER_NUM_OF_RX_CH		0x0C
#define MELFAS_REGISTER_NUM_OF_TOUCH_KEY	0x0D
#define MELFAS_REGISTER_INPUT_PACKET_SIZE	0x0F
#define MELFAS_REGISTER_INPUT_INFO			0x10
#define MELFAS_REGISTER_XY_COORDINATE		0x11
#define MELFAS_REGISTER_X_COORDINATE		0x12
#define MELFAS_REGISTER_Y_COORDINATE		0x13
#define MELFAS_REGISTER_TOUCH_WIDTH			0x14
#define MELFAS_REGISTER_TOUCH_STRENGTH		0x15

#define MELFAS_REGISTER_UNIVERSAL_CMD_ID	0xA0
#define MELFAS_REGISTER_UNIVERSAL_CMD_P1	0xA1
#define MELFAS_REGISTER_UNIVERSAL_CMD_P2	0xA2
#define MELFAS_REGISTER_UNIVERSAL_CMD_RESULT_SIZE	0xAE
#define MELFAS_REGISTER_UNIVERSAL_CMD_RESULT		0xAF

#define MELFAS_REGISTER_VENDER_SPEC_CMD_ID	0xB0
#define MELFAS_REGISTER_VENDER_SPEC_CMD_P1	0xB1
#define MELFAS_REGISTER_VENDER_SPEC_CMD_P2	0xB2

#define MELFAS_REGISTER_VENDER_SPEC_CMD_RESULT_SIZE	0xBE
#define MELFAS_REGISTER_VENDER_SPEC_CMD_RESULT		0xBF





























#define MELFAS_STATE_NONE					0x00000000
#define MELFAS_STATE_POWERON				0x00000001
#define MELFAS_STATE_ACTIVE					0x00000010
#define MELFAS_STATE_UPDATE					0x00010000













struct ts_abs_info {
	int			x;
	int			y;
	int			w;
	int			z;
	int			touch;
};

struct melfas_ncm_firmware_struct {
		size_t			size;
		u8				*data;
};


struct melfas_ncm_data_struct {
    struct i2c_client           *p_client;
    struct input_dev            *p_input_dev;
    struct work_struct          work;
    struct ts_abs_info          fingers[MELFAS_NCM_TS_MAX_TOUCH];
    int                         state;

    struct early_suspend        early_suspend;

};





static struct workqueue_struct  	*gp_melfas_ncm_wq = NULL;

static struct i2c_client			*gp_client = NULL;
static void 						(*gp_tp_cmd_callback)(void *) = NULL;
static int							g_diagtype = 0;
static touch_diag_result			g_touch_result;
static struct mutex					g_mutex_state;
static u8							g_touch_mode = 0;   
#if !defined(MELFAS_NCM_NO_SYSFS)
static struct kobject *melfas_ncm_kobj;
#endif 





static int melfas_ncm_soft_reset( struct melfas_ncm_data_struct	*p_ts );




static int melfas_ncm_abs_read( struct melfas_ncm_data_struct	*p_ts);
static int melfas_ncm_poweron( struct melfas_ncm_data_struct	*p_ts );
static int melfas_ncm_poweroff( struct melfas_ncm_data_struct	*p_ts );
static int melfas_ncm_fw_update( struct melfas_ncm_data_struct	*p_ts );
static int melfas_ncm_initial_setting( struct melfas_ncm_data_struct	*p_ts);





unsigned char melfas_ncm_power( unsigned char onoff )
{
	unsigned char						ret = -1;
	struct melfas_ncm_data_struct		*p_ts = NULL;

	if ( gp_client == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: Exit gp_client = NULL \n",
					__func__);
		return ret;
	}

	p_ts = i2c_get_clientdata( gp_client );
	if ( p_ts == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: Exit p_ts = NULL \n",
						__func__);
		return ret;
	}

	mutex_lock( &g_mutex_state );

	if( onoff )
	{
		
		ret = (unsigned char)melfas_ncm_poweron( p_ts );
		if ( ret )
		{
			printk(KERN_DEBUG "[touchpanel]%s: poweron faild \n",__func__);
		}
	}
	else
	{	
		ret = (unsigned char)melfas_ncm_poweroff( p_ts );
		if ( ret )
		{
			printk(KERN_DEBUG "[touchpanel]%s: poweroff faild \n",__func__);
		}
	}
	mutex_unlock( &g_mutex_state );

	return ret;
}
EXPORT_SYMBOL( melfas_ncm_power );




unsigned char touch_panel_cmd( unsigned char type, void *p_val )
{
	unsigned char						ret      = MELFAS_NCM_TS_RTN_VAL_FF; 
	int									touch_index = 0;
	touch_diag_result					*p_result;
	unsigned char						*p_ver;
	struct melfas_ncm_data_struct		*p_ts;
	int									pw_chg = 0;
	tISCFWNCMCInfo_t					ver_info;

	printk(KERN_DEBUG "[touchpanel]%s: Enter type = %d\n",
					__func__, type);
	p_result = (touch_diag_result*)p_val;


	if ( gp_client == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: Exit gp_client = NULL \n",
					__func__);
		return ret;
	}

	p_ts = i2c_get_clientdata( gp_client );
	if ( p_ts == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: Exit p_ts = NULL \n",
						__func__);
		return ret;
	}

	if( p_ts->state & MELFAS_STATE_UPDATE )
	{
		printk(KERN_DEBUG "[touchpanel]%s: F/W Updating... \n",
					__func__);
		return ret;

	}

	printk(KERN_DEBUG "[touchpanel]%s: state: 0x%08X\n",
					__func__, p_ts->state);


	switch ( type )
	{
		
		case TOUCH_PANEL_CMD_TYPE_RESET:
			ret = (unsigned char)melfas_ncm_soft_reset( p_ts );
			break;

		case TOUCH_PANEL_CMD_TYPE_POWER_OFF:
			mutex_lock( &g_mutex_state );

			ret = (unsigned char)melfas_ncm_poweroff( p_ts );

			mutex_unlock( &g_mutex_state );
			break;

		case TOUCH_PANEL_CMD_TYPE_POWER_ON:
			mutex_lock( &g_mutex_state );

			ret = (unsigned char)melfas_ncm_poweron( p_ts );

			mutex_unlock( &g_mutex_state );
			break;

		
		case TOUCH_PANEL_CMD_TYPE_GET_REVISION:

			p_ver  = (unsigned char *)p_val;

			if( p_ver )
			{

				mutex_lock( &g_mutex_state );

				if( !(p_ts->state & MELFAS_STATE_POWERON) )
				{
					melfas_ncm_poweron( p_ts );
					pw_chg = 1;
					printk(KERN_DEBUG "[touchpanel]%s: pw state : off -> on\n",
									__func__);
				}

				ret = mms100_DVE022_get_dev_version( gp_client , &ver_info );
				if( ret == ISC_SUCCESS )
				{




					p_ver[1] = 0;
					p_ver[2] = ver_info.info[SEC_CONFIG].version;

				}
				else
				{
					p_ver[1] = 0xFF;
					p_ver[2] = 0xFF;
				}

				if( pw_chg )
				{
					melfas_ncm_poweroff( p_ts );
					printk(KERN_DEBUG "[touchpanel]%s: pw state : on -> off\n",
									__func__);
				}

				mutex_unlock( &g_mutex_state );
			}
			else
			{
				p_ver[1] = 0xFF;
				p_ver[2] = 0xFF;
			}

			break;

		
		case TOUCH_PANEL_CMD_TYPE_GET_COORD:
			if( p_ts->state &= MELFAS_STATE_POWERON )
			{
				if ( p_result != NULL )
				{
					p_result->ret = 0;

					for ( touch_index = 0; touch_index < TOUCH_PANEL_FINGER_NUM_MAX ; touch_index++ )
					{
						if( p_ts->fingers[touch_index].touch )
						{
							p_result->finger[touch_index].finger_data_x[0] =
											(u8)( p_ts->fingers[touch_index].x >> 8 );
							p_result->finger[touch_index].finger_data_x[1] =
											(u8)( p_ts->fingers[touch_index].x & 0xFF );
							p_result->finger[touch_index].finger_data_y[0] =
											(u8)( p_ts->fingers[touch_index].y >> 8 );
							p_result->finger[touch_index].finger_data_y[1] =
											(u8)( p_ts->fingers[touch_index].y & 0xFF );
						}
						else
						{
							p_result->finger[touch_index].finger_data_x[0] = 0xFF;
							p_result->finger[touch_index].finger_data_x[1] = 0xFF;
							p_result->finger[touch_index].finger_data_y[0] = 0xFF;
							p_result->finger[touch_index].finger_data_y[1] = 0xFF;
						}










					}
				}
				else if ( p_result != NULL )
				{
					printk(KERN_DEBUG "[touchpanel]%s: melfas_ncm_abs_read\n",
							__func__);
					p_result->ret = MELFAS_NCM_TS_RTN_VAL_FF;
					ret = MELFAS_NCM_TS_RTN_VAL_FF;
				}
				else
				{
					printk(KERN_DEBUG "[touchpanel]%s: p_result = NULL Error\n",
							__func__);
					ret = MELFAS_NCM_TS_RTN_VAL_FF;
				}
			}
			else
			{
				if ( p_result != NULL )
				{
					for ( touch_index = 0; touch_index < TOUCH_PANEL_FINGER_NUM_MAX ; touch_index++ )
					{
						p_result->finger[touch_index].finger_data_x[0] = 0xFF;
						p_result->finger[touch_index].finger_data_x[1] = 0xFF;
						p_result->finger[touch_index].finger_data_y[0] = 0xFF;
						p_result->finger[touch_index].finger_data_y[1] = 0xFF;
					}
				}
			}
			break;


		case TOUCH_PANEL_CMD_TYPE_GET_FW_REVISION:
			p_ver  = (unsigned char *)p_val;

			if( p_ver )
			{
				ret = mms100_DVE022_get_file_version( gp_client, &ver_info );

				if( ret == ISC_SUCCESS )
				{




					p_ver[1] = 0;
					p_ver[2] = ver_info.info[SEC_CONFIG].version;

				}
				else
				{
					p_ver[1] = 0xFF;
					p_ver[2] = 0xFF;
				}
			}
			else
			{
				p_ver[1] = 0xFF;
				p_ver[2] = 0xFF;
			}
			break;

		case TOUCH_PANEL_CMD_TYPE_FW_UPDATE:
			mutex_lock( &g_mutex_state );
			if( !(p_ts->state & MELFAS_STATE_POWERON) )
			{
				melfas_ncm_poweron( p_ts );
				pw_chg = 1;
				printk(KERN_DEBUG "[touchpanel]%s: pw state : off -> on\n",__func__);
			}
			mutex_unlock( &g_mutex_state );
			ret = melfas_ncm_fw_update( p_ts );
			if( pw_chg )
			{
				melfas_ncm_poweroff( p_ts );
				printk(KERN_DEBUG "[touchpanel]%s: pw state : on -> off\n",__func__);
			}
			break;

		default:
			printk(KERN_DEBUG "[touchpanel]%s: NG type = %d\n",
							__func__, type);
			break;
	}

	printk(KERN_DEBUG "[touchpanel]%s: Exit ret = %d\n",
					__func__, ret);

	return ret;

}
EXPORT_SYMBOL( touch_panel_cmd );




unsigned char touch_panel_cmd_callback( unsigned char type,
                                        unsigned char val,
                                        void (*func)(void *) )
{
	unsigned char ret = MELFAS_NCM_TS_RTN_VAL_FF; 

	pr_debug("[touchpanel]%s: Enter type = %d\n",
					__func__, type);

	if ( func == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: func = NULL\n",
						__func__);
		return ret;
	}

	switch ( type )
	{
		case TOUCH_PANEL_CMD_TYPE_LINE_TEST:
			gp_tp_cmd_callback = func;
			g_diagtype = TOUCH_PANEL_CMD_TYPE_LINE_TEST;
			ret = 0;
			break;
		default:
			printk(KERN_DEBUG "[touchpanel]%s: NG type = %d\n",
							__func__, type);
			break;
	}

	pr_debug("[touchpanel]%s: Exit ret = %d\n",
					__func__, ret);

	return ret;

}
EXPORT_SYMBOL(touch_panel_cmd_callback);




static void touch_panel_callback( u8 data[] )
{
	int		touch_index = 0;
	struct melfas_ncm_data_struct *p_ts;

	if ( gp_tp_cmd_callback == NULL )
	{
		return;
	}

	printk(KERN_DEBUG "[touchpanel]%s: Enter g_diagtype = %d\n",
			__func__,
			g_diagtype);

	p_ts = i2c_get_clientdata( gp_client );

	switch ( g_diagtype )
	{
		case TOUCH_PANEL_CMD_TYPE_LINE_TEST:
			g_touch_result.ret = 0;

			for ( touch_index = 0; touch_index < TOUCH_PANEL_FINGER_NUM_MAX ; touch_index++ )
			{
				if( p_ts->fingers[touch_index].touch )
				{
					g_touch_result.finger[touch_index].finger_data_x[0] =
									(u8)( p_ts->fingers[touch_index].x >> 8 );
					g_touch_result.finger[touch_index].finger_data_x[1] =
									(u8)( p_ts->fingers[touch_index].x & 0xFF );
					g_touch_result.finger[touch_index].finger_data_y[0] =
									(u8)( p_ts->fingers[touch_index].y >> 8 );
					g_touch_result.finger[touch_index].finger_data_y[1] =
									(u8)( p_ts->fingers[touch_index].y & 0xFF );
				}
				else
				{
					g_touch_result.finger[touch_index].finger_data_x[0] = 0xFF;
					g_touch_result.finger[touch_index].finger_data_x[1] = 0xFF;
					g_touch_result.finger[touch_index].finger_data_y[0] = 0xFF;
					g_touch_result.finger[touch_index].finger_data_y[1] = 0xFF;
				}










			}
			break;
	}

	gp_tp_cmd_callback( &g_touch_result );
	gp_tp_cmd_callback = NULL;
	g_diagtype = 0;

	printk(KERN_DEBUG "[touchpanel]%s: Exit\n",
					__func__);

	return ;

}






static int melfas_ncm_transfer( struct melfas_ncm_data_struct *p_ts,
                                    char reg,
                                    char *tx_buf, int tx_cnt,
                                    char *rx_buf, int rx_cnt )
{
	struct i2c_client	*p_client;
	struct i2c_adapter	*adap;
	struct i2c_msg		msg[2];
	char				buf[32];
	int					ret;
	int					num = 1;
	int					retry_cnt = 0;

    unsigned char		alrm_info[4];
    static int			alrm_cnt = 0;


	if( p_ts == NULL )
	{
        printk(KERN_ERR "[touchpanel]%s: p_ts.\n",
        				__func__);
		return -1;
	}
	p_client = p_ts->p_client;

	if( p_client == NULL )
	{
        printk(KERN_ERR "[touchpanel]%s: p_client.\n",
        				__func__);
		return -1;
	}
	adap = p_client->adapter;

	buf[0] = reg;

	if( tx_cnt )
	{
		memcpy( &buf[1], tx_buf, tx_cnt );
	}

	msg[0].addr = p_client->addr;
	msg[0].flags = p_client->flags & I2C_M_TEN;
	msg[0].len = tx_cnt + 1;
	msg[0].buf = buf;

	if( rx_cnt )
	{
		msg[1].addr = p_client->addr;
		msg[1].flags = p_client->flags & I2C_M_TEN;
		msg[1].flags |= I2C_M_RD;
		msg[1].len = rx_cnt;
		msg[1].buf = rx_buf;
		num = 2;
	}

    for( retry_cnt = 0; retry_cnt < MELFAS_NCM_I2C_RETRY_CNT; retry_cnt++ )
    {
        ret = i2c_transfer(adap, msg, num);

        if ( ret < 0 ) {
            printk(KERN_ERR "[touchpanel]%s: i2c_transfer error ret:%x\n",
            				__func__, ret);

            if( retry_cnt >= MELFAS_NCM_I2C_RETRY_CNT - 1)
            {
                if( alrm_cnt == 0 )
                {
                    if( num == 1 )
                    {
                        alrm_info[0] = 0x09;         
                        alrm_info[2] = buf[0];    
                        if( tx_cnt )
                            alrm_info[3] = buf[1];
                        else
                            alrm_info[3] = 0x00;     
                    }
                    else
                    {
                        alrm_info[0] = 0x0B;        
                        alrm_info[2] = buf[0];   
                        if( tx_cnt )
                            alrm_info[3] = buf[1];
                        else
                            alrm_info[3] = 0x00;     
                    }

                    alrm_info[1] = (unsigned char)(ret * -1);

                    printk(KERN_ERR "[T][ARM]Event:0x7C Info:0x%02X%02X%02X%02X\n",
                                    alrm_info[0],alrm_info[1],alrm_info[2],alrm_info[3]);
                    alrm_cnt = 1;
                }
            }

        }

        else
        {
            alrm_cnt = 0;
            break;
        }

    }


	return ret;

}





































































static int melfas_ncm_poweron( struct melfas_ncm_data_struct	*p_ts )
{
	struct i2c_client					*p_client = NULL;
	struct melfas_ncm_ts_platform_data	*p_data	= NULL;
    int									ret		= 0;

	if( p_ts == NULL )
	{
        printk(KERN_ERR "[touchpanel]%s: p_ts is NULL.\n",
        				__func__);
		return -1;
	}
	p_client = p_ts->p_client;

	if( p_client == NULL )
	{
        printk(KERN_ERR "[touchpanel]%s: p_client is NULL.\n",
        				__func__);
		return -1;
	}

	p_data = p_client->dev.platform_data;

	if( !(p_ts->state & MELFAS_STATE_POWERON) )
	{
		if ( p_data && p_data->poweron )
		{
			ret = p_data->poweron( &p_client->dev );
			if ( ret )
			{
				printk(KERN_ERR "[touchpanel]%s: poweron fail.\n",
								__func__);
				ret = -1;
			}
			else
			{
				if( p_ts )
				{
					ret = melfas_ncm_initial_setting( p_ts );
					if( ret < 0 )
					{
						printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
										__func__, 0x01 );
					}
					else
					{
						ret = 0;
						p_ts->state |= MELFAS_STATE_POWERON ;
					}
				}
				else
				{
					printk(KERN_ERR "[touchpanel]%s: p_ts is NULL.\n",
									__func__);
					ret = -1;
				}
			}
		}
		else
		{
			printk(KERN_ERR "[touchpanel]%s: p_data/powerdown is NULL.\n",
							__func__);
			ret = -1;
		}
	}
	else
	{
		printk(KERN_ERR "[touchpanel]%s: already on.\n",
						__func__);
	}
	return ret;
}



static int melfas_ncm_poweroff( struct melfas_ncm_data_struct	*p_ts )
{
	struct i2c_client					*p_client = NULL;
	struct melfas_ncm_ts_platform_data	*p_data	= NULL;
	int									ret = 0;

	if( p_ts == NULL )
	{
        printk(KERN_ERR "[touchpanel]%s: p_ts is NULL.\n",
        				__func__);
		return -1;
	}
	p_client = p_ts->p_client;

	if( p_client == NULL )
	{
        printk(KERN_ERR "[touchpanel]%s: p_client is NULL.\n",
        				__func__);
		return -1;
	}

	p_data = p_client->dev.platform_data;

	if( p_ts->state & MELFAS_STATE_POWERON )
	{
		if ( p_data && p_data->powerdown )
		{
			p_data->powerdown( &p_client->dev );
			p_ts->state ^= MELFAS_STATE_POWERON;
		}
		else
		{
			printk(KERN_ERR "[touchpanel]%s: p_data/powerdown is NULL.\n",
							__func__);
			ret = -1;
		}
	}
	else
	{
		printk(KERN_ERR "[touchpanel]%s: already off.\n",
						__func__);
	}

	return ret;
}




static int melfas_ncm_soft_reset( struct melfas_ncm_data_struct	*p_ts )
{
	int								ret = 0;	
	int								rc  = 0;
	char							buf[1];






	buf[0] = MELFAS_NCM_TS_REG01_RESET;
	rc = melfas_ncm_transfer( p_ts, MELFAS_REGISTER_MODE_CTL, buf, 1, NULL, 0 );
	if( rc < 0 )
	{
		printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
						__func__, 0x01 );
		ret = MELFAS_NCM_TS_RTN_VAL_FF;
	}






	return ret;
}




































static int melfas_ncm_fw_update( struct melfas_ncm_data_struct	*p_ts)
{
	struct i2c_client	*p_client = NULL;
	int					ret = -1;
	eISCRet_t			ret_val;

	tISCFWNCMCInfo_t    ver_info;

	uint32_t hw_rev = 0;

	hw_rev = hw_revision_read();
	p_client = p_ts->p_client;

	if( p_client == NULL )
	{
        printk(KERN_ERR "[touchpanel]%s: p_client is NULL.\n",__func__);
	}
	else
	{

		ret = mms100_DVE022_get_dev_version( gp_client , &ver_info );
		if(0x54 != ver_info.info[SEC_CORE].version) 
		{
			printk(KERN_DEBUG "[touchpanel]%s: CORE[%02x] is old,F/W not need update \n",__func__,ver_info.info[SEC_CORE].version);
			return 0;
		}

		else if(hw_rev < HW_REV_0P3)
		{
			return 0;
		}
		mutex_lock( &g_mutex_state );
		p_ts->state |= MELFAS_STATE_UPDATE;
		mutex_unlock( &g_mutex_state );



		ret_val = mms100_ISC_download_mbinary(p_client,false);

		if( ret_val == ISC_SUCCESS )
			ret = 0;

		mutex_lock( &g_mutex_state );

		p_ts->state ^= MELFAS_STATE_UPDATE;

		if( !(p_ts->state & MELFAS_STATE_ACTIVE ) )
		{
			ret = melfas_ncm_poweroff( p_ts );
			if ( ret )
			{
				printk(KERN_DEBUG "[touchpanel]%s: poweroff faild \n",__func__);
			}
		}
		mutex_unlock( &g_mutex_state );

	}

	return ret;
}



static int melfas_ncm_mode_read(char *page, char **start, off_t off,
                                   int count, int *eof, void *data)
{
    int  len;

	printk(KERN_DEBUG "[touchpanel]%s: Enter\n",
					__func__);

	len = sprintf(page, "%d\n",g_touch_mode );

    *eof = 1;

	printk(KERN_DEBUG "[touchpanel]%s: Exit\n",
					__func__);
    return len;
}



static u8 melfas_ncm_atoi( u8 str )
{
    u8  num = 0;

    if( '0' <= str && str <= '9' )
    {
        num = str - '0';
    }
    else if( 'a' <= str && str <= 'f' )
    {
        switch( str )
        {
            case 'a':
                num = 10;
                break;
            case 'b':
                num = 11;
                break;
            case 'c':
                num = 12;
                break;
            case 'd':
                num = 13;
                break;
            case 'e':
                num = 14;
                break;
            case 'f':
                num = 15;
                break;
        }
    }
    else if( 'A' <= str && str <= 'F' )
    {
        switch( str )
        {
            case 'A':
                num = 10;
                break;
            case 'B':
                num = 11;
                break;
            case 'C':
                num = 12;
                break;
            case 'D':
                num = 13;
                break;
            case 'E':
                num = 14;
                break;
            case 'F':
                num = 15;
                break;
        }
    }
    return num;
}



static int melfas_ncm_mode_set( u8 value )
{

    


    if( value )
    {
        
        g_touch_mode = 1;
    }
    else
    {
        
        g_touch_mode = 0;
    }

    return 0;
}



static int melfas_ncm_mode_write(struct file* filp, const char* buffer, 
                                   unsigned long count, void* data )
{
    u8				ctl_str[256] ={0};
    u8				ctl_num[256] ={0};
    u8				tmp;
    int				cnt          = 0;
	int				rc  = 0;
	int				len = (int)count;

	printk(KERN_DEBUG "[touchpanel]%s: Enter\n",
					__func__);





    if (count > 256)
    {
        printk(KERN_ERR "[touchpanel]%s: count error (%lu) \n",
        				__func__,count);
        return -EINVAL;
    }

    if (!buffer)
    {
        printk(KERN_ERR "[touchpanel]%s: no buffer \n",
        				__func__);
        return -EINVAL;
    }

    if (copy_from_user(ctl_str, buffer, count))
    {
        printk(KERN_ERR "[touchpanel]%s: data copy error \n",
        				__func__);
        return -EFAULT;
    }

    for(cnt=0; cnt<count; cnt++)
    {
        tmp = melfas_ncm_atoi(ctl_str[cnt]);

        if( cnt % 2 )
        {
            ctl_num[cnt/2] =  ctl_num[cnt/2] + tmp;
        }
        else
        {
            ctl_num[cnt/2] =  tmp << 4;
        }






    }

    printk(KERN_DEBUG "[touchpanel]%s: MODE (%x)\n",
    				__func__, ctl_num[0]);

    rc = melfas_ncm_mode_set( ctl_num[0] );
    if ( rc )
    {
        printk(KERN_ERR "[touchpanel]%s: synaptics_ncm_mode_set Error!\n",
        				__func__);
        return -1;
    }

	printk(KERN_DEBUG "[touchpanel]%s: Exit\n",
					__func__);
    return len;
}





































static void __init melfas_ncm_procfs_init(void)
{
	struct proc_dir_entry *dirp;






	dirp = (struct proc_dir_entry *)create_proc_entry(MELFAS_PROC_NAME_CTL, 0664, 0);
	dirp->read_proc  = (read_proc_t *) melfas_ncm_mode_read;
	dirp->write_proc = (write_proc_t *) melfas_ncm_mode_write;











}
#if !defined(MELFAS_NCM_NO_SYSFS)

static ssize_t reg01_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int								item;
	int								ret = 0;	
	int								rc  = 0;
	char							store_buf[1];
	struct melfas_ncm_data_struct	*p_ts;






	if ( gp_client == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: Exit gp_client = NULL \n",
					__func__);
		return ret;
	}

	p_ts = i2c_get_clientdata( gp_client );
	if ( p_ts == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: Exit p_ts = NULL \n",
						__func__);
		return ret;
	}

	sscanf(buf,"%d\n",&item);	



	store_buf[0] = (char)item;

	printk(KERN_DEBUG "[touchpanel]%s: WRITE DATA 0x%02x \n",
					__func__, store_buf[0]);

	rc = melfas_ncm_transfer( p_ts, 0x01, store_buf, 1, NULL, 0 );
	if( rc < 0 )
	{
		printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
						__func__, 0x01 );
	}






	return count;
}

static ssize_t reg01_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int								ret = 0;
	char							show_buf[1];
	int								rc  = 0;
	struct melfas_ncm_data_struct	*p_ts;






	if ( gp_client == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: Exit gp_client = NULL \n",
					__func__);
		return ret;
	}

	p_ts = i2c_get_clientdata( gp_client );
	if ( p_ts == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: Exit p_ts = NULL \n",
						__func__);
		return ret;
	}


	rc = melfas_ncm_transfer( p_ts, 0x01, NULL, 0, show_buf, 1 );
	if( rc < 0 )
	{
		printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
						__func__, 0x0F );
	}

	printk(KERN_DEBUG "[touchpanel]%s: READ DATA 0x%02x \n",
					__func__, show_buf[0]);

	ret = sprintf(buf, "0x%02x\n", show_buf[0]);






	return ret;
}

static ssize_t reg02_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int								item;
	int								ret = 0;	
	int								rc  = 0;
	char							store_buf[1];
	struct melfas_ncm_data_struct	*p_ts;






	if ( gp_client == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: Exit gp_client = NULL \n",
					__func__);
		return ret;
	}

	p_ts = i2c_get_clientdata( gp_client );
	if ( p_ts == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: Exit p_ts = NULL \n",
						__func__);
		return ret;
	}

	sscanf(buf,"%d\n",&item);	



	store_buf[0] = (char)item;

	printk(KERN_DEBUG "[touchpanel]%s: WRITE DATA 0x%02x \n",
					__func__, store_buf[0]);

	rc = melfas_ncm_transfer( p_ts, 0x02, store_buf, 1, NULL, 0 );
	if( rc < 0 )
	{
		printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
						__func__, 0x02 );
	}






	return count;
}

static ssize_t reg02_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int								ret = 0;
	char							show_buf[1];
	int								rc  = 0;
	struct melfas_ncm_data_struct	*p_ts;






	if ( gp_client == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: Exit gp_client = NULL \n",
					__func__);
		return ret;
	}

	p_ts = i2c_get_clientdata( gp_client );
	if ( p_ts == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: Exit p_ts = NULL \n",
						__func__);
		return ret;
	}


	rc = melfas_ncm_transfer( p_ts, 0x02, NULL, 0, show_buf, 1 );
	if( rc < 0 )
	{
		printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
						__func__, 0x02 );
	}

	printk(KERN_DEBUG "[touchpanel]%s: READ DATA 0x%02x \n",
					__func__, show_buf[0]);

	ret = sprintf(buf, "0x%02x\n", show_buf[0]);






	return ret;
}

static ssize_t reg03_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int								item;
	int								ret = 0;	
	int								rc  = 0;
	char							store_buf[1];
	struct melfas_ncm_data_struct	*p_ts;






	if ( gp_client == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: Exit gp_client = NULL \n",
					__func__);
		return ret;
	}

	p_ts = i2c_get_clientdata( gp_client );
	if ( p_ts == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: Exit p_ts = NULL \n",
						__func__);
		return ret;
	}

	sscanf(buf,"%d\n",&item);	



	store_buf[0] = (char)item;

	printk(KERN_DEBUG "[touchpanel]%s: WRITE DATA 0x%02x \n",
					__func__, store_buf[0]);

	rc = melfas_ncm_transfer( p_ts, 0x03, store_buf, 1, NULL, 0 );
	if( rc < 0 )
	{
		printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
						__func__, 0x03 );
	}






	return count;
}

static ssize_t reg03_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int								ret = 0;
	char							show_buf[1];
	int								rc  = 0;
	struct melfas_ncm_data_struct	*p_ts;






	if ( gp_client == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: Exit gp_client = NULL \n",
					__func__);
		return ret;
	}

	p_ts = i2c_get_clientdata( gp_client );
	if ( p_ts == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: Exit p_ts = NULL \n",
						__func__);
		return ret;
	}


	rc = melfas_ncm_transfer( p_ts, 0x03, NULL, 0, show_buf, 1 );
	if( rc < 0 )
	{
		printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
						__func__, 0x03 );
	}

	printk(KERN_DEBUG "[touchpanel]%s: READ DATA 0x%02x \n",
					__func__, show_buf[0]);

	ret = sprintf(buf, "0x%02x\n", show_buf[0]);






	return ret;
}

static ssize_t reg04_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int								item;
	int								ret = 0;	
	int								rc  = 0;
	char							store_buf[1];
	struct melfas_ncm_data_struct	*p_ts;






	if ( gp_client == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: Exit gp_client = NULL \n",
					__func__);
		return ret;
	}

	p_ts = i2c_get_clientdata( gp_client );
	if ( p_ts == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: Exit p_ts = NULL \n",
						__func__);
		return ret;
	}

	sscanf(buf,"%d\n",&item);	



	store_buf[0] = (char)item;

	printk(KERN_DEBUG "[touchpanel]%s: WRITE DATA 0x%02x \n",
					__func__, store_buf[0]);

	rc = melfas_ncm_transfer( p_ts, 0x04, store_buf, 1, NULL, 0 );
	if( rc < 0 )
	{
		printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
						__func__, 0x04 );
	}






	return count;
}

static ssize_t reg04_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int								ret = 0;
	char							show_buf[1];
	int								rc  = 0;
	struct melfas_ncm_data_struct	*p_ts;






	if ( gp_client == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: Exit gp_client = NULL \n",
					__func__);
		return ret;
	}

	p_ts = i2c_get_clientdata( gp_client );
	if ( p_ts == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: Exit p_ts = NULL \n",
						__func__);
		return ret;
	}


	rc = melfas_ncm_transfer( p_ts, 0x04, NULL, 0, show_buf, 1 );
	if( rc < 0 )
	{
		printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
						__func__, 0x04 );
	}

	printk(KERN_DEBUG "[touchpanel]%s: READ DATA 0x%02x \n",
					__func__, show_buf[0]);

	ret = sprintf(buf, "0x%02x\n", show_buf[0]);






	return ret;
}

static ssize_t reg05_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int								item;
	int								ret = 0;	
	int								rc  = 0;
	char							store_buf[1];
	struct melfas_ncm_data_struct	*p_ts;






	if ( gp_client == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: Exit gp_client = NULL \n",
					__func__);
		return ret;
	}

	p_ts = i2c_get_clientdata( gp_client );
	if ( p_ts == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: Exit p_ts = NULL \n",
						__func__);
		return ret;
	}

	sscanf(buf,"%d\n",&item);	



	store_buf[0] = (char)item;

	printk(KERN_DEBUG "[touchpanel]%s: WRITE DATA 0x%02x \n",
					__func__, store_buf[0]);

	rc = melfas_ncm_transfer( p_ts, 0x05, store_buf, 1, NULL, 0 );
	if( rc < 0 )
	{
		printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
						__func__, 0x05 );
	}






	return count;
}

static ssize_t reg05_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int								ret = 0;
	char							show_buf[1];
	int								rc  = 0;
	struct melfas_ncm_data_struct	*p_ts;






	if ( gp_client == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: Exit gp_client = NULL \n",
					__func__);
		return ret;
	}

	p_ts = i2c_get_clientdata( gp_client );
	if ( p_ts == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: Exit p_ts = NULL \n",
						__func__);
		return ret;
	}


	rc = melfas_ncm_transfer( p_ts, 0x05, NULL, 0, show_buf, 1 );
	if( rc < 0 )
	{
		printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
						__func__, 0x05 );
	}

	printk(KERN_DEBUG "[touchpanel]%s: READ DATA 0x%02x \n",
					__func__, show_buf[0]);

	ret = sprintf(buf, "0x%02x\n", show_buf[0]);






	return ret;
}

static ssize_t reg06_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int								item;
	int								ret = 0;	
	int								rc  = 0;
	char							store_buf[1];
	struct melfas_ncm_data_struct	*p_ts;






	if ( gp_client == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: Exit gp_client = NULL \n",
					__func__);
		return ret;
	}

	p_ts = i2c_get_clientdata( gp_client );
	if ( p_ts == NULL )
	{
		printk(KERN_DEBUG "[touchpanel]%s: Exit p_ts = NULL \n",
						__func__);
		return ret;
	}

	sscanf(buf,"%d\n",&item);	



	store_buf[0] = (char)item;

	printk(KERN_DEBUG "[touchpanel]%s: WRITE DATA 0x%02x \n",
					__func__, store_buf[0]);

	rc = melfas_ncm_transfer( p_ts, 0x06, store_buf, 1, NULL, 0 );
	if( rc < 0 )
	{
		printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
						__func__, 0x06 );
	}






	return count;
}

static ssize_t reg06_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int								ret = 0;
	char							show_buf[1];
	int								rc  = 0;
	struct melfas_ncm_data_struct	*p_ts;






	if ( gp_client == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: Exit gp_client = NULL \n",
					__func__);
		return ret;
	}

	p_ts = i2c_get_clientdata( gp_client );
	if ( p_ts == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: Exit p_ts = NULL \n",
						__func__);
		return ret;
	}


	rc = melfas_ncm_transfer( p_ts, 0x06, NULL, 0, show_buf, 1 );
	if( rc < 0 )
	{
		printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
						__func__, 0x06 );
	}

	printk(KERN_DEBUG "[touchpanel]%s: READ DATA 0x%02x \n",
					__func__, show_buf[0]);

	ret = sprintf(buf, "0x%02x\n", show_buf[0]);






	return ret;
}

static ssize_t state_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct melfas_ncm_data_struct	*p_ts;
	unsigned long value;
	int								err = 0;






	if ( gp_client == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: Exit gp_client = NULL \n",
					__func__);
		return count;
	}

	p_ts = i2c_get_clientdata( gp_client );
	if ( p_ts == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: Exit p_ts = NULL \n",
						__func__);
		return count;
	}

	err = strict_strtoul(buf, 10, &value);
	if (err != 0)
		return count;

	printk(KERN_DEBUG "[touchpanel]%s: 0x%08X\n",
					__func__, (int)value);

	p_ts->state = (int)value;

	return count;

}
static ssize_t state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int								ret = 0;
	struct melfas_ncm_data_struct	*p_ts;






	if ( gp_client == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: Exit gp_client = NULL \n",
					__func__);
		return ret;
	}

	p_ts = i2c_get_clientdata( gp_client );
	if ( p_ts == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: Exit p_ts = NULL \n",
						__func__);
		return ret;
	}

	printk(KERN_DEBUG "[touchpanel]%s: state: 0x%08X\n",
					__func__, p_ts->state);

	ret = sprintf(buf, "0x%08X\n", p_ts->state );






	return ret;

}
static DEVICE_ATTR(reg01, S_IROTH|S_IWOTH, reg01_show, reg01_store);
static DEVICE_ATTR(reg02, S_IROTH|S_IWOTH, reg02_show, reg02_store);
static DEVICE_ATTR(reg03, S_IROTH|S_IWOTH, reg03_show, reg03_store);
static DEVICE_ATTR(reg04, S_IROTH|S_IWOTH, reg04_show, reg04_store);
static DEVICE_ATTR(reg05, S_IROTH|S_IWOTH, reg05_show, reg05_store);
static DEVICE_ATTR(reg06, S_IROTH|S_IWOTH, reg06_show, reg06_store);
static DEVICE_ATTR(state, S_IROTH|S_IWOTH, state_show, state_store);

static struct attribute *melfas_ncm_attributes[] = {
    &dev_attr_reg01.attr,
    &dev_attr_reg02.attr,
    &dev_attr_reg03.attr,
    &dev_attr_reg04.attr,
    &dev_attr_reg05.attr,
    &dev_attr_reg06.attr,
    &dev_attr_state.attr,
    	NULL,
};


static struct attribute_group melfas_ncm_attr_group = {
    .attrs = melfas_ncm_attributes,
};





static int melfas_ncm_sysfs_init( struct melfas_ncm_data_struct *p_ts )
{
	int err = 0;

	melfas_ncm_kobj = kobject_create_and_add("melfas_ncm", kernel_kobj);
	
	err = sysfs_create_group(melfas_ncm_kobj, &melfas_ncm_attr_group);
	if (err !=0)
	{
		printk(KERN_ERR "[touchpanel]%s: create sysfs group error\n",
						__func__);
	}
	return 0;

}
#endif 



static int melfas_ncm_input_init( struct melfas_ncm_data_struct *p_ts )
{
	int			ret = -1;

	if( p_ts == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: p_ts.\n",
						__func__);
		return -1;
	}

	p_ts->p_input_dev = input_allocate_device( );
	if ( p_ts->p_input_dev == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: failed to allocate touchscreen input device\n",
				__func__);
		return -1;
	}

	p_ts->p_input_dev->name = "melfas_ncm_touchscreen";
	input_set_drvdata( p_ts->p_input_dev, p_ts );

	set_bit( EV_SYN, p_ts->p_input_dev->evbit );
	set_bit( EV_ABS, p_ts->p_input_dev->evbit );

	input_set_abs_params( p_ts->p_input_dev, ABS_MT_POSITION_X,
						  0, MELFAS_NCM_TS_MAX_X_COORD, 0, 0 );
	input_set_abs_params( p_ts->p_input_dev, ABS_MT_POSITION_Y,
						  0, MELFAS_NCM_TS_MAX_Y_COORD, 0, 0 );
	input_set_abs_params( p_ts->p_input_dev, ABS_MT_PRESSURE,
						  0, MELFAS_NCM_TS_MAX_Z_TOUCH, 0, 0 );
	input_set_abs_params( p_ts->p_input_dev, ABS_MT_TRACKING_ID,
						  0, MELFAS_NCM_TS_MAX_TOUCH-1, 0, 0);
	input_set_abs_params( p_ts->p_input_dev, ABS_MT_TOUCH_MAJOR,
						  0, MELFAS_NCM_TS_MAX_W_TOUCH, 0, 0);

	ret = input_register_device( p_ts->p_input_dev );

	return ret;
}



static void melfas_ncm_input_report( struct input_dev *dev,
									  int x,
									  int y,
									  int z,
									  int w,
									  int num )
{






	input_report_abs( dev, ABS_MT_TRACKING_ID, num);
	input_report_abs( dev, ABS_MT_POSITION_X, x );
	input_report_abs( dev, ABS_MT_POSITION_Y, y );
	input_report_abs( dev, ABS_MT_PRESSURE, z );
	input_report_abs( dev, ABS_MT_TOUCH_MAJOR, w );
	input_mt_sync( dev);

}



static void melfas_ncm_input_report_all_release( struct melfas_ncm_data_struct	*p_ts)
{
	int				cnt;
	int				report_en = 0;

	for( cnt = 0; cnt < MELFAS_NCM_TS_MAX_TOUCH; cnt++ )
	{
		if( p_ts->fingers[cnt].touch )
		{
			input_mt_sync( p_ts->p_input_dev );
			p_ts->fingers[cnt].touch = 0;
			p_ts->fingers[cnt].x = 0;
			p_ts->fingers[cnt].y = 0;
			p_ts->fingers[cnt].z = 0;
			p_ts->fingers[cnt].w = 0;
			report_en = 1;
		}
		
	}
	if( report_en )
		input_sync( p_ts->p_input_dev );
}



static int melfas_ncm_initial_setting( struct melfas_ncm_data_struct	*p_ts)
{
	char	buf[1] = {0};
	int		ret = 0;

	buf[0] = MELFAS_NCM_TS_REG01_ACTIVE;
	ret = melfas_ncm_transfer( p_ts, MELFAS_REGISTER_MODE_CTL, buf, 1, NULL, 0 );
	if( ret < 0 )
	{
		printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
						__func__, 0x01 );
	}

	return ret;

}




static int melfas_ncm_abs_read( struct melfas_ncm_data_struct	*p_ts)
{
	char	buf[MELFAS_NCM_TS_READ_REGS_LEN] = {0};
	int		ret = 0,	
			rc, pkt_num, read_num, touch_index, cnt,report_en ;














	report_en = 0;
	
	rc = melfas_ncm_transfer( p_ts, MELFAS_REGISTER_INPUT_PACKET_SIZE, NULL, 0, buf, 1 );
	if( rc < 0 )
	{
		printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
						__func__, 0x0F );
		ret = MELFAS_NCM_TS_RTN_VAL_FF;
		goto END_ERR;
	}

	pkt_num = buf[0];











	if(pkt_num > 0 && pkt_num < MELFAS_NCM_TS_READ_REGS_LEN )
	{
		rc = melfas_ncm_transfer( p_ts, MELFAS_REGISTER_INPUT_INFO, NULL, 0, buf, pkt_num );
		if( rc < 0 )
		{
			printk(KERN_ERR "[touchpanel]%s: melfas_ncm_transfer[%02x] failed.\n",
							__func__, 0x10 );
			ret = MELFAS_NCM_TS_RTN_VAL_FF;
			goto END_ERR;
		}









		for( read_num = 0; read_num < pkt_num; read_num = read_num + MELFAS_NCM_TS_INPUT_REG_SIZE)
		{
			touch_index = (buf[read_num] & 0x0F) - 1;

			if( (0 > touch_index) || (MELFAS_NCM_TS_MAX_TOUCH <= touch_index) )
			{
				printk(KERN_ERR "[touchpanel]%s: invalid touch index = %d.\n",__func__, touch_index );
				continue;
			}

			p_ts->fingers[touch_index].x = (uint16_t)(buf[read_num+1] & 0x0F) << 8 | buf[read_num+2];
			p_ts->fingers[touch_index].y = (uint16_t)(buf[read_num+1] & 0xF0) << 4 | buf[read_num+3];


			if( p_ts->fingers[touch_index].x > MELFAS_NCM_TS_MAX_X_COORD )
			{
				printk(KERN_DEBUG "[touchpanel]%s: X size over %d\n",
								__func__, p_ts->fingers[touch_index].x);

				p_ts->fingers[touch_index].x = MELFAS_NCM_TS_MAX_X_COORD;
			}
			if( p_ts->fingers[touch_index].y > MELFAS_NCM_TS_MAX_Y_COORD )
			{
				printk(KERN_DEBUG "[touchpanel]%s: Y size over %d\n",
								__func__, p_ts->fingers[touch_index].x);
				p_ts->fingers[touch_index].y = MELFAS_NCM_TS_MAX_Y_COORD;
			}

			if((buf[read_num] & 0x80)==0)
				p_ts->fingers[touch_index].z = 0;
			else
				p_ts->fingers[touch_index].z = buf[read_num+5];








			p_ts->fingers[touch_index].w = buf[read_num+4];
















		}
	}

	for( cnt = 0; cnt < MELFAS_NCM_TS_MAX_TOUCH; cnt++ )
	{
		if( p_ts->fingers[cnt].z )
		{
			melfas_ncm_input_report( p_ts->p_input_dev,
									 p_ts->fingers[cnt].x,
									 p_ts->fingers[cnt].y,
									 p_ts->fingers[cnt].z,
									 p_ts->fingers[cnt].w,
									 cnt );
			p_ts->fingers[cnt].touch = 1;
			report_en = 1;
		}
		else if( p_ts->fingers[cnt].touch )
		{
			input_mt_sync( p_ts->p_input_dev );
			p_ts->fingers[cnt].x = 0xFFFF;
			p_ts->fingers[cnt].y = 0xFFFF;
			p_ts->fingers[cnt].z = 0;
			p_ts->fingers[cnt].w = 0;
			p_ts->fingers[cnt].touch = 0;
			report_en = 1;
		}
		
	}
	if( report_en )
		input_sync( p_ts->p_input_dev );
























END_ERR:














	return ret;
}





static irqreturn_t melfas_ncm_irq_handler( int irq, void *p_dev_id )
{
	struct melfas_ncm_data_struct *p_ts = p_dev_id;






	if( p_ts )
	{
		queue_work( gp_melfas_ncm_wq, &p_ts->work );
	}
	else
	{
		printk(KERN_ERR "[touchpanel]%s: p_ts is NULL.\n",
						__func__ );
	}






	return IRQ_HANDLED;
}









static void melfas_ncm_do_work( struct work_struct *p_work )
{
	struct melfas_ncm_data_struct	*p_ts = container_of( p_work, struct melfas_ncm_data_struct, work );

	int								ret;








	if( p_ts == NULL )
	{
		printk(KERN_ERR "[touchpanel]%s: p_ts is NULL.\n",
						__func__ );
		goto END_ERR;
	}

	ret = melfas_ncm_abs_read( p_ts );
	if ( ret )
	{
		printk(KERN_ERR "[touchpanel]%s: melfas_ncm_abs_read Error!\n",
						__func__);
		goto END_ERR;
	}

	touch_panel_callback( NULL );

END_ERR:








	return;

}




static int melfas_ncm_suspend( struct i2c_client *p_client, pm_message_t mesg )
{
	struct melfas_ncm_data_struct		*p_ts = i2c_get_clientdata(p_client);

	if( p_ts )
	{
		printk(KERN_DEBUG "[touchpanel]%s Enter: state: 0x%08X\n",
						__func__, p_ts->state);

		mutex_lock( &g_mutex_state );

		if( p_ts->state & MELFAS_STATE_ACTIVE )
		{
			disable_irq(p_client->irq);

			cancel_work_sync( &p_ts->work );

			melfas_ncm_input_report_all_release(p_ts);

			if( !(p_ts->state & MELFAS_STATE_UPDATE ))
				melfas_ncm_poweroff( p_ts );

			p_ts->state ^= MELFAS_STATE_ACTIVE;

		}
		else
		{
			printk(KERN_ERR "[touchpanel]%s: already SLEEP.\n",
							__func__);
		}
		mutex_unlock( &g_mutex_state );

		printk(KERN_DEBUG "[touchpanel]%s Exit: state: 0x%08X\n",
						__func__, p_ts->state);

	}
	else
	{
		printk(KERN_ERR "[touchpanel]%s: p_ts is NULL.\n",
						__func__);
	}

	return 0;
}




static int melfas_ncm_resume( struct i2c_client *p_client )
{
	struct melfas_ncm_data_struct		*p_ts = i2c_get_clientdata(p_client);

	if( p_ts )
	{
	  msleep(200);  
		printk(KERN_DEBUG "[touchpanel]%s Enter: state: 0x%08X\n",
						__func__, p_ts->state);

		mutex_lock( &g_mutex_state );

		if( !(p_ts->state & MELFAS_STATE_ACTIVE) )
		{

			if( !(p_ts->state & MELFAS_STATE_UPDATE ))
				melfas_ncm_poweron(p_ts);

			enable_irq(p_client->irq);

			p_ts->state |= MELFAS_STATE_ACTIVE;
		}
		else
		{
			printk(KERN_ERR "[touchpanel]%s: already ACTIVE.\n",
							__func__);
		}

		mutex_unlock( &g_mutex_state );

		printk(KERN_DEBUG "[touchpanel]%s Exit: state: 0x%08X\n",
						__func__, p_ts->state);

	}
	else
	{
		printk(KERN_ERR "[touchpanel]%s: p_ts is NULL.\n",
						__func__);
	}

	return 0;
}





static void melfas_ncm_early_suspend( struct early_suspend *p_h )
{
    struct melfas_ncm_data_struct	*p_ts	= NULL;






    p_ts = container_of( p_h, struct melfas_ncm_data_struct, early_suspend );
	if( p_ts )
    	melfas_ncm_suspend( p_ts->p_client, PMSG_SUSPEND );
	else
		printk(KERN_DEBUG "[touchpanel]%s: p_ts is NULL.\n",
							__func__);





}




static void melfas_ncm_late_resume( struct early_suspend *p_h )
{
    struct melfas_ncm_data_struct	*p_ts	= NULL;






    p_ts = container_of( p_h, struct melfas_ncm_data_struct, early_suspend );
    melfas_ncm_resume( p_ts->p_client );







}





static int __devinit melfas_ncm_probe( struct i2c_client *p_client,
                                         const struct i2c_device_id *p_id )
{
    struct melfas_ncm_data_struct		*p_ts	= NULL;
    struct melfas_ncm_ts_platform_data	*p_data	= NULL;
    int									ret		= 0;





    if (!i2c_check_functionality(p_client->adapter, I2C_FUNC_I2C))
    {
        printk(KERN_ERR "[touchpanel]%s: i2c_check_functionality.\n",
        				__func__);
        ret = -ENODEV;
        goto END_ERR;
    }

    p_ts = kzalloc( sizeof(struct melfas_ncm_data_struct), GFP_KERNEL );
    if ( p_ts == NULL ) {
        printk(KERN_ERR "[touchpanel]%s: no memory\n",
        				__func__);
        ret = -EIO;
        goto END_ERR;
    }

	p_data = p_client->dev.platform_data;
	
	p_ts->p_client = p_client;
	gp_client      = p_client;

	i2c_set_clientdata(p_client, p_ts);
    mutex_init( &g_mutex_state );

	ret = melfas_ncm_input_init( p_ts );
    if ( ret < 0 ) {
        printk(KERN_ERR "[touchpanel]%s: input register\n",
        				__func__);
        goto END_ERR;
    }

	mutex_lock( &g_mutex_state );

	p_ts->state = MELFAS_STATE_NONE;

	ret = melfas_ncm_poweron( p_ts );

	p_ts->state |= MELFAS_STATE_ACTIVE;

	mutex_unlock( &g_mutex_state );

	if ( ret )
	{
		goto END_ERR;
	}













































	INIT_WORK( &p_ts->work, melfas_ncm_do_work );

	ret = request_irq( p_client->irq, melfas_ncm_irq_handler,
					   IRQF_TRIGGER_FALLING, "melfas_ncm_irq", p_ts);

    if (ret > 0)
    {
        printk(KERN_ERR "[touchpanel]%s: request_irq\n",
        				__func__);
        goto END_ERR;
    }


    p_ts->early_suspend.level   = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
    p_ts->early_suspend.suspend = melfas_ncm_early_suspend;
    p_ts->early_suspend.resume  = melfas_ncm_late_resume;
    register_early_suspend( &p_ts->early_suspend );


#if !defined(MELFAS_NCM_NO_SYSFS)
	melfas_ncm_sysfs_init( p_ts );
#endif 

    return ret;


END_ERR: 
    if ( p_ts != NULL ) {
        kfree( p_ts );
    }

    return ret;

}




static int __devexit melfas_ncm_remove( struct i2c_client *p_client )
{
	struct melfas_ncm_data_struct		*p_ts = i2c_get_clientdata(p_client);
    struct melfas_ncm_ts_platform_data	*p_data	= NULL;

	if( p_ts )
	{

	    unregister_early_suspend( &p_ts->early_suspend );


		free_irq(p_client->irq, p_ts);

		p_data = p_client->dev.platform_data;

		if ( p_data && p_data->powerdown )
			p_data->powerdown( &p_client->dev );
		input_unregister_device(p_ts->p_input_dev);
		kfree(p_ts);
	}

	return 0;

}




static const struct i2c_device_id melfas_ncm_i2c_device_id[] = {
    {MELFAS_NCM_TS_NAME, 0},
    {}
};




static struct i2c_driver melfas_ncm_driver = {
    .probe      = melfas_ncm_probe,
    .remove     = __devexit_p(melfas_ncm_remove),




    .driver     = {
            .name = MELFAS_NCM_TS_NAME,
    },
    .id_table   = melfas_ncm_i2c_device_id,
};




static int __init melfas_ncm_init(void)
{
	int			rc;







	gp_melfas_ncm_wq = create_singlethread_workqueue("melfas_ncm_wq");
	if ( !gp_melfas_ncm_wq )
	{
		return -ENOMEM;
	}

	rc = i2c_add_driver(&melfas_ncm_driver);

	melfas_ncm_procfs_init();






    return rc;
}




static void __exit melfas_ncm_exit(void)
{
	i2c_del_driver(&melfas_ncm_driver);


	if ( gp_melfas_ncm_wq )
	{
		destroy_workqueue( gp_melfas_ncm_wq );
	}


}


module_init(melfas_ncm_init);

module_exit(melfas_ncm_exit);




MODULE_DESCRIPTION("Touch Screen driver for Melfas MMS Touch Screen sensor IC");
MODULE_LICENSE("GPL");

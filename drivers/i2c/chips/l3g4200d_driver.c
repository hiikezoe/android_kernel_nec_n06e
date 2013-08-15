/***********************************************************************
* (C) NCMC Corporation 2011
* 
* No permission to use, copy, modify and distribute this software
* and its documentation for any purpose is granted.
* This software is provided under applicable license agreement only.
*
* File Name:gysd_main.c
* Contents :Gyro Sensor Driver Main module
*
*****************************************************************************/







    #include <linux/errno.h>
    #include <linux/init.h>
    #include <linux/fs.h>
    #include <linux/slab.h>
    #include <linux/kdev_t.h>
    #include <linux/wait.h>
    #include <linux/spinlock.h>
    #include <linux/sched.h>
    #include <linux/timer.h>
    #include <linux/delay.h>
    #include <linux/poll.h>
    #include <linux/wait.h>
    #include <linux/semaphore.h>
    #include <linux/interrupt.h>
    #include <asm/uaccess.h>

    #include <linux/spi/spi.h>


    #include <linux/i2c.h>
    #include <linux/i2c-dev.h>
    #include <linux/miscdevice.h>

    #include <linux/proc_fs.h>

    #include <linux/kthread.h>
    #include <linux/mutex.h>


#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/err.h>




    #include <linux/i2c/gysdcom.h>
    #include <linux/i2c/gysd_define.h>
    #include <linux/i2c/gysd_local.h>
    #include <float.h>












#define DEBUG_PRINT( arg... )





#define DEBUG_PRINT2( arg... )







#define GYRO_REGULATOR_OFF		0
#define GYRO_REGULATOR_ON		1

static int g_gyro_regulator = 0;





static int gyro_i2c_rx_data(unsigned char *p_rxData, int length);
static int init_device(void);
static int stop_device(void);
static int nc_pm8921_vreg_set_enable(void);
static int get_rad_data(struct l3g4200d_rad_data *rad_data);
static int get_lsb_data(struct l3g4200d_lsb_data *lsb_data);
static int read_wait_time(void);
static int write_reg_byte(unsigned char addr, unsigned char data);
static int write_reg_5byte(unsigned char addr, unsigned char *data);


static int read_reg_byte(unsigned char addr, unsigned char *data);
static int read_reg_word(unsigned char addr, short *data);
static int convert_data(short *inputData, float *OutputData);
static int init_device_selfA(void);
static int init_device_selfB(char mode);
static int gyro_self_test(char mode);
static int gyro_offset_calc(unsigned char count,short *OutputData);
static int g_i2c_error_alarmed = false; 




#define D_GYRO_SENSOR_NV_NUM    25
#define D_GYRO_DEBUG_MODE_ON    0x01
#define CMD_WORD        	0xA0	
#define CMD_BYTE        	0x80	
#define PI                   (3.14159265)




























































#define NV_GYRO_DEBUG_MODE         (0x0000)
#define NV_GYRO_CTRL_REG1          (0x004F)
#define NV_GYRO_CTRL_REG2          (0x0029)
#define NV_GYRO_CTRL_REG3          (0x0000)
#define NV_GYRO_CTRL_REG4          (0x00A0)
#define NV_GYRO_CTRL_REG5          (0x0063)
#define NV_GYRO_REFERENCE          (0x0000)
#define NV_GYRO_FIFO_CTRL_REG      (0x0000)
#define NV_GYRO_INT1_CFG           (0x0000)
#define NV_GYRO_INT1_TSH_XH        (0x0000)
#define NV_GYRO_INT1_TSH_XL        (0x0000)
#define NV_GYRO_INT1_TSH_YH        (0x0000)
#define NV_GYRO_INT1_TSH_YL        (0x0000)
#define NV_GYRO_INT1_TSH_ZH        (0x0000)
#define NV_GYRO_INT1_TSH_ZL        (0x0000)
#define NV_GYRO_INT1_DURATION      (0x0000)
#define NV_GYRO_OFFSET_X           (0x0000)
#define NV_GYRO_OFFSET_Y           (0x0000)
#define NV_GYRO_OFFSET_Z           (0x0000)
#define NV_GYRO_THRD_GY_X          (0x0047)
#define NV_GYRO_THRD_GY_Y          (0x0047)
#define NV_GYRO_THRD_GY_Z          (0x0047)
#define NV_GYRO_THRD_GY_X2         (0x0047)
#define NV_GYRO_THRD_GY_Y2         (0x0047)
#define NV_GYRO_THRD_GY_Z2         (0x0047)




struct gysd_nv_data {
    unsigned long nv_reg[D_GYRO_SENSOR_NV_NUM];
};

DEFINE_MUTEX(gyro_mutex);


static struct gysd_nv_data g_nv_data = {
	.nv_reg = { 0, },
};


struct l3g4200d_data {
	struct work_struct work;
};


static struct i2c_client *gyro_client;

















int Gysd_open( struct inode *inode, struct file *file )
{

	return 0;
}










int Gysd_close( struct inode *inode, struct file *file )
{
	return 0;
}

















long Gysd_ioctl( struct file *file_p, unsigned int cmd, unsigned long arg )
{
    long                  ret;
	unsigned char *p_arg = (unsigned char *)arg;
	unsigned char reg_data = 0;
	unsigned char buffer[1];
	unsigned char write_buff[2];
    struct l3g4200d_rad_data rad_data;
    struct l3g4200d_lsb_data diag_data;
	struct l3g4200d_diag_offset_data diag_offset_data;
	struct l3g4200d_diag_read_reg_data read_reg_data;
	struct l3g4200d_diag_write_reg_data write_reg_data;
    unsigned char self_test_count;
    short OutputData[3];

    DEBUG_PRINT( "[gysd] <Gysd_ioctl> START \n");

    ret = -ENODEV;

	mutex_lock(&gyro_mutex);

        switch ( cmd )
        {

            case GYRO_IO_CTL_SET_NV:
				ret = copy_from_user(&g_nv_data, p_arg, sizeof(struct gysd_nv_data));
                if (ret)
                {
                    pr_err("%s: Failed to copy data from user!\n", __func__);
                    mutex_unlock(&gyro_mutex);
                    return -EFAULT;
                }
                break;

            case GYRO_IO_GET_REGISTER:                                                                      

                buffer[0] = GYSD_SENSOR_REG_WHO_AM_I;
                ret = gyro_i2c_rx_data(buffer, 1);

                if (ret)
                {
                    pr_err("%s: gyro_i2c_rx_data error!\n", __func__);
                  mutex_unlock(&gyro_mutex);
                  return ret;
                }

		        reg_data = buffer[0];
                ret = copy_to_user(p_arg, &reg_data, sizeof(unsigned char));

                if (ret)
                {
                  pr_err("%s: Failed to copy data to user!\n", __func__);
                  mutex_unlock(&gyro_mutex);
                  return -EFAULT;
                }

                break;

            case GYRO_IO_DIAG_READ_REGISTER:                                                                
               memset(&read_reg_data, 0, sizeof(struct l3g4200d_diag_read_reg_data));
                ret = copy_from_user(&read_reg_data, p_arg, sizeof(struct l3g4200d_diag_read_reg_data));
                if (ret)
                {
                  pr_err("%s: Failed to copy data from user!\n", __func__);
                  mutex_unlock(&gyro_mutex);
                  return -EFAULT;
                }

                buffer[0] = read_reg_data.id_name;
                ret = gyro_i2c_rx_data(buffer, 1);

                if (ret)
                {
                    pr_err("%s: gyro_i2c_rx_data error!\n", __func__);
                  mutex_unlock(&gyro_mutex);
                  return ret;
                }
		        read_reg_data.id_value = buffer[0];
                ret = copy_to_user(p_arg, &read_reg_data, sizeof(struct l3g4200d_diag_read_reg_data));

                if (ret)
                {
                  pr_err("%s: Failed to copy data to user!\n", __func__);
                  mutex_unlock(&gyro_mutex);
                  return -EFAULT;
                }

                break;

            case GYRO_IO_DIAG_WRITE_REGISTER:                                                               
                memset(&write_reg_data, 0, sizeof(struct l3g4200d_diag_write_reg_data));
                ret = copy_from_user(&write_reg_data, p_arg, sizeof(struct l3g4200d_diag_write_reg_data));
                if (ret)
                {
                  pr_err("%s: Failed to copy data from user!\n", __func__);
                  mutex_unlock(&gyro_mutex);
                  return -EFAULT;
                }

                write_buff[0] = write_reg_data.id_name;
                write_buff[1] = write_reg_data.id_value;
                ret = write_reg_byte(write_buff[0], write_buff[1]);

                if (ret)
                {
                    pr_err("%s: gyro_i2c_tx_data error!\n", __func__);
                  mutex_unlock(&gyro_mutex);
                  return ret;
                }
                break;

            case GYRO_IO_MEASURE_START:                                                                     
                ret = init_device();
                if (ret)
                {
                    pr_err("%s: init_device error!\n", __func__);
                  mutex_unlock(&gyro_mutex);
                  return ret;
                }
                break;

            case GYRO_IO_MEASURE_STOP:                                                                      
                ret = stop_device();
                if (ret)
                {
                    pr_err("%s: stop_device error!\n", __func__);
                  mutex_unlock(&gyro_mutex);
                  return ret;
                }
                break;

            case GYRO_IO_DIAG_GET_RAD_DATA:                                                                 
            case GYRO_IO_READ_RAD_XYZ:

                memset(&rad_data, 0, sizeof(struct l3g4200d_rad_data));

                ret = get_rad_data(&rad_data);
                if (ret)
                {
                    mutex_unlock(&gyro_mutex);
                    return ret;
                }

                ret = copy_to_user(p_arg, &rad_data, sizeof(struct l3g4200d_rad_data));
                if (ret)
                {
                    mutex_unlock(&gyro_mutex);
                    return -EFAULT;
                }
                break;

            case GYRO_IO_DIAG_GET_DATA:                                                                     
                memset(&diag_data, 0, sizeof(struct l3g4200d_lsb_data));
                
                ret = get_lsb_data(&diag_data);
                if (ret)
                {
                    pr_err("%s: Failed to get_lsb_data!\n", __func__);
                    mutex_unlock(&gyro_mutex);
                    return ret;
                }
                ret = copy_to_user(p_arg, &diag_data, sizeof(struct l3g4200d_lsb_data));
                if (ret)
                {
 	            pr_err("%s: Failed to copy data to user!\n", __func__);
                 	mutex_unlock(&gyro_mutex);
 	            return -EFAULT;
                }
                break;


            case GYRO_IO_SELF_TEST_MODE:                                                                    

                ret = copy_from_user(&self_test_count, p_arg, sizeof(unsigned char));
                if (ret)
                {
 	                pr_err("%s: Failed to copy data from user!\n", __func__);
                   	mutex_unlock(&gyro_mutex);
 	                return -EFAULT;
                }

                ret = gyro_self_test(self_test_count);
                if (ret)
                {
 	                pr_err("%s: Failed to gyro_self_test!\n", __func__);
                 	mutex_unlock(&gyro_mutex);
 	                return -EFAULT;
                }

                break;

            case GYRO_IO_DIAG_GET_OFFSET_DATA:                                                              
                memset(&diag_offset_data, 0, sizeof(struct l3g4200d_diag_offset_data));

                ret = copy_from_user(&diag_offset_data, p_arg, sizeof(struct l3g4200d_diag_offset_data));
                if (ret)
                {
 	                pr_err("%s: Failed to copy data from user!\n", __func__);
                   	mutex_unlock(&gyro_mutex);
 	                return -EFAULT;
                }

                ret = gyro_offset_calc(diag_offset_data.count,OutputData);
                if (ret)
                {
 	                pr_err("%s: Failed to gyro_offset_calc!\n", __func__);
                 	mutex_unlock(&gyro_mutex);
 	                return ret;
                }
                    diag_offset_data.lsb_x = OutputData[0];
                    diag_offset_data.lsb_y = OutputData[1];
                    diag_offset_data.lsb_z = OutputData[2];
                
                ret = copy_to_user(p_arg, &diag_offset_data, sizeof(struct l3g4200d_diag_offset_data));
                if (ret)
                {
 	                pr_err("%s: Failed to copy data to user!\n", __func__);
                 	mutex_unlock(&gyro_mutex);
 	                return -EFAULT;
                }

                break;

            default:
                break;
        }
	mutex_unlock(&gyro_mutex);

    DEBUG_PRINT( "[gysd] <Gysd_ioctl> E N D \n");

    return ret;
}









static int gyro_i2c_tx_data(char *p_txData, int length)
{
    int            ret   = 0;       
    unsigned char  info1 = 0x01;    
    struct i2c_msg msg[] =
    {
        {
            .addr = gyro_client->addr,
            .flags = 0,
            .len = length,
            .buf = p_txData,
        },
    };

    
    if( unlikely( p_txData == NULL ) )
    {
        
        pr_err( "L3G4200D[%s]: NULL pointer\n" ,__func__ );
        return -EINVAL;
    }

    
    ret = i2c_transfer( gyro_client->adapter, msg, 1 );
    if( ret < 0 )
    {
        
        pr_err( "L3G4200D[%s]: transfer error\n", __func__ );

        if( g_i2c_error_alarmed == false )
        {
            if( length <= 1 )
            {
                info1 |= (0x01 << 2);   
            }
            else
            {
                info1 |= (0x02 << 2);   
            }

            
            pr_err("[T][ARM]Event:0x73 Info:0x%02x%02x%02x%02x\n",
                info1, (-ret) & 0xFF, p_txData[0], ((length > 1) ? p_txData[1] : 0) );
            g_i2c_error_alarmed = true; 
        }
        
        return -EIO;
    }

    g_i2c_error_alarmed = false;        

    
    return 0;
}





static int gyro_i2c_rx_data(unsigned char *p_rxData, int length)
{
    int            ret   = 0;                                       
    unsigned char  info1 = (length > 0) ? 0x03 : 0x01;              
    char           info3 = (p_rxData == NULL) ? 0 : p_rxData[0];    
	struct i2c_msg msgs[] =
	{
		{
			.addr  = gyro_client->addr,
			.flags = 0,
			.len   = 1,
			.buf   = p_rxData,
		},
		{
			.addr  = gyro_client->addr,
			.flags = I2C_M_RD,
			.len   = length,
			.buf   = p_rxData,
		},
	};

    
    if( unlikely( p_rxData == NULL ) )
    {
        pr_err( "L3G4200D[%s]: NULL pointer\n" ,__func__ );
        
        return -EINVAL;
    }

    
    ret = i2c_transfer( gyro_client->adapter, msgs, 2 );
    if( ret < 0 )
    {
        
        pr_err( "L3G4200D[%s]: transfer error\n", __func__ );

        if( g_i2c_error_alarmed == false )
        {
            info1 |= (0x01 << 2);       

            
            pr_err( "[T][ARM]Event:0x73 Info:0x%02x%02x%02x%02x\n",
                info1, (-ret) & 0xFF, info3, 0 );
            g_i2c_error_alarmed = true; 
        }
        
        return(-EIO);
    }

    g_i2c_error_alarmed = false;        

    
    return 0;
}







static int init_device(void)
{
	int ret = 0;
    unsigned char data[5];




    DEBUG_PRINT2("init_device start \n");
    data[0] = NV_GYRO_CTRL_REG1;
    data[1] = NV_GYRO_CTRL_REG2;
    data[2] = NV_GYRO_CTRL_REG3;
    data[3] = NV_GYRO_CTRL_REG4;
    data[4] = NV_GYRO_CTRL_REG5;

    ret |= write_reg_5byte(GYSD_SENSOR_REG_CTRL_REG1, data);


































    msleep(GYSD_WAKE_WAITTIME);

	return ret;
}

static int stop_device(void)
{
	int ret = 0;
    char read_data = 0;
    char write_data = 0;

	ret |= read_reg_byte(GYSD_SENSOR_REG_CTRL_REG1, &read_data);

    write_data = read_data & 0xF7;
    ret |= write_reg_byte(GYSD_SENSOR_REG_CTRL_REG1, write_data);

	return ret;
}





static int nc_pm8921_vreg_set_enable(void)
{
    int rc;
    static struct regulator *vreg_l16 = NULL;
    
    pr_info("[In]%s\n", __func__);
    
    
    vreg_l16 = regulator_get(NULL, "8921_l16");
    if (IS_ERR(vreg_l16)) {
        pr_err("%s: VREG_L16 failed\n", __func__);
        return -EINVAL;
    }
    
    
    rc = regulator_set_voltage(vreg_l16, 2800000, 2800000);
    if (rc) {
        pr_err("vreg_l16 set voltage failed.\n");
        return -EINVAL;
    }
    
    
    rc = regulator_enable(vreg_l16);
    if (rc) {
        pr_err("vreg_l16 enable failed\n");
        return -EINVAL;
    }
    
    pr_info("[Out]%s\n", __func__);
	
	g_gyro_regulator = GYRO_REGULATOR_ON;
    
    return 0;
}











static int write_reg_byte(unsigned char addr, unsigned char data)
{
	unsigned char buff[2];

	buff[0] = addr;
	buff[1] = data;

	return gyro_i2c_tx_data(buff, 2);
}
























static int write_reg_5byte(unsigned char addr, unsigned char *data)
{
	unsigned char buff[6];

	buff[0] = addr | 0x80;
	buff[1] = data[0];
	buff[2] = data[1];
	buff[3] = data[2];
	buff[4] = data[3];
	buff[5] = data[4];

	return gyro_i2c_tx_data(buff, 6);
}











static int read_reg_byte(unsigned char addr, unsigned char *data)
{
	int ret = 0;
	unsigned char buff[1];

	if (unlikely(data == NULL)) {
		pr_err("%s: NULL pointer error!\n", __func__);
		return -EINVAL;
	}

	buff[0] = addr;

	ret =  gyro_i2c_rx_data(buff, 1);

	if (!ret) {
		*data = buff[0];
	}

	return ret;
}

static int read_reg_word(unsigned char addr, short *data)
{
	int ret = 0;
    unsigned char buff[2];

	if (unlikely(data == NULL)) {
		pr_err("%s: NULL pointer error!\n", __func__);
		return -EINVAL;
	}

	buff[0] = CMD_WORD | (addr & 0x1F);

	ret =  gyro_i2c_rx_data(buff, 2);
	if (!ret) {
		*data = buff[0] + ((unsigned int)buff[1] << 8);
	}

	return ret;
}





static int convert_data(short *inputData, float *OutputData)
{
	char ishpf = 0;
	short offsetX,offsetY,offsetZ;
	double outX,outY,outZ;
	short thrd_X,thrd_Y,thrd_Z;
	char ctrl_reg4;
	char fs_mode;
	double sensitivity;
	double clip;
    DEBUG_PRINT2("convert_data start \n");
	
	ctrl_reg4 = NV_GYRO_CTRL_REG4;
	ishpf = ((NV_GYRO_CTRL_REG5&0x10)>>4);
	
	outX = inputData[0];
	outY = inputData[1];
	outZ = inputData[2];
	
    offsetX = NV_GYRO_OFFSET_X;
    offsetY = NV_GYRO_OFFSET_Y;
    offsetZ = NV_GYRO_OFFSET_Z;

    thrd_X = NV_GYRO_THRD_GY_X;
    thrd_Y = NV_GYRO_THRD_GY_Y;
    thrd_Z = NV_GYRO_THRD_GY_Z;

	if(ishpf == 0)
	{
		
		outX += offsetX;
		outY += offsetY;
		outZ += offsetZ;
		
		
		if(abs(outX) <= thrd_X)
		{
			outX = 0;
		}
		if(abs(outY) <= thrd_Y)
		{
			outY = 0;
		}
		if(abs(outZ) <= thrd_Z)
		{
			outZ = 0;
		}
	}

	
	
	fs_mode = (ctrl_reg4&0x30)>>4;
	
	if(fs_mode == 0)
	{
		sensitivity = 8.75;
		clip = 4.36332313;
	}
	else if(fs_mode == 1)
	{
		sensitivity = 17.5;
		clip = 8.72664626;
	}
	else
	{
		sensitivity = 70.0;
		clip = 34.90658504;
	}
	

	outX = outX * sensitivity / 1000 * PI /180;
	outY = outY * sensitivity / 1000 * PI /180;
	outZ = outZ * sensitivity / 1000 * PI /180;
	
	
	outX = outX;
	outY = outY;
	outZ = outZ;
	
	
	if(outX > clip)
	{
		outX = clip;
	}
	
	if(outX < (-clip))
	{
		outX = (-clip);
	}
	
	if(outY > clip)
	{
		outY = clip;
	}
	
	if(outY < (-clip))
	{
		outY = (-clip);
	}

	if(outZ > clip)
	{
		outZ = clip;
	}
	
	if(outZ < (-clip))
	{
		outZ = (-clip);
	}

    OutputData[0] = outX;
    OutputData[1] = outY;
    OutputData[2] = outZ;

    return 0;
}





static int get_rad_data(struct l3g4200d_rad_data *rad_data)
{

	int ret = 0;
    short inputdata[3];
    float output[3];
    struct l3g4200d_lsb_data lsb_data;
    int wait_time;

    DEBUG_PRINT2("get_rad_data start \n");

    if (rad_data == NULL) {
        pr_err("%s: NULL pointer error!\n", __func__);
        return -EINVAL;
    }

    
    memset(&lsb_data, 0, sizeof(struct l3g4200d_lsb_data));

    
    ret |= write_reg_byte(GYSD_SENSOR_REG_FIFO_CTRL_REG, 0x20);

    
    wait_time = read_wait_time();
    if( wait_time >= 0 )
    {
        DEBUG_PRINT2( "%d ms wait \n", wait_time );
        msleep( wait_time );
    }
    else
    {
        DEBUG_PRINT2( "%d ms wait(error) \n", wait_time );
        msleep( 7 );        
        ret |= (-EINVAL);
    }

    
    ret |= read_reg_word(GYSD_SENSOR_REG_OUT_X_L, &lsb_data.lsb_x);
    ret |= read_reg_word(GYSD_SENSOR_REG_OUT_Y_L, &lsb_data.lsb_y);
    ret |= read_reg_word(GYSD_SENSOR_REG_OUT_Z_L, &lsb_data.lsb_z);

    
    ret |= write_reg_byte(GYSD_SENSOR_REG_FIFO_CTRL_REG, 0x00);

    
    inputdata[0] = lsb_data.lsb_x;
    inputdata[1] = lsb_data.lsb_y;
    inputdata[2] = lsb_data.lsb_z;
    convert_data( inputdata, output );  

    rad_data->rad_x = output[0];
    rad_data->rad_y = output[1];
    rad_data->rad_z = output[2];

    return ret;
}





static int get_lsb_data(struct l3g4200d_lsb_data *lsb_data)
{
    int ret = 0;
    int wait_time;

    DEBUG_PRINT2("get_lsb_data start \n");

    if (lsb_data == NULL) {
        pr_err("%s: NULL pointer error!\n", __func__);
        return -EINVAL;
    }

    
    ret |= write_reg_byte(GYSD_SENSOR_REG_FIFO_CTRL_REG, 0x20);

    
    wait_time = read_wait_time();
    if( wait_time >= 0 )
    {
        DEBUG_PRINT2( "%d ms wait \n", wait_time );
        msleep( wait_time );
    }
    else
    {
        DEBUG_PRINT2( "%d ms wait(error) \n", wait_time );
        msleep( 7 );        
        ret |= (-EINVAL);
    }

    
    ret |= read_reg_word(GYSD_SENSOR_REG_OUT_X_L, &lsb_data->lsb_x);
    ret |= read_reg_word(GYSD_SENSOR_REG_OUT_Y_L, &lsb_data->lsb_y);
    ret |= read_reg_word(GYSD_SENSOR_REG_OUT_Z_L, &lsb_data->lsb_z);

    
    ret |= write_reg_byte(GYSD_SENSOR_REG_FIFO_CTRL_REG, 0x00);

    return ret;
}





static int read_wait_time(void)
{
    int ret;
    int ctrl_reg1;
    unsigned char read_data = 0;

    ret = read_reg_byte(GYSD_SENSOR_REG_CTRL_REG1, &read_data);
    if(!ret)
    {
        ctrl_reg1 = (( read_data >> 6 ) & 0x03);
        if(ctrl_reg1 == 0x00)
        {
            
            ret = 13;
        }
        else if(ctrl_reg1 == 0x01)
        {
            
            ret = 7;
        }
        else if(ctrl_reg1 == 0x02)
        {
            
            ret = 4;
        }
        else
        {
            
            ret = 2;
        }
    }
    else
    {
        ret = (-EINVAL);
    }

    return ret;
}






static int init_device_selfA(void)
{
	int ret = 0;
    unsigned char data[5];

    DEBUG_PRINT2("init_device_selfA start \n");
    data[0] = 0x4F;
    data[1] = NV_GYRO_CTRL_REG2;
    data[2] = NV_GYRO_CTRL_REG3;
    data[3] = NV_GYRO_CTRL_REG4;
    data[4] = 0x63;

    ret = write_reg_5byte(GYSD_SENSOR_REG_CTRL_REG1, data);

    msleep(GYSD_WAKE_WAITTIME);

	return ret;
}





static int init_device_selfB(char mode)
{
	int ret = 0;
	unsigned char reg4 = 0;
    unsigned char data[5];

    DEBUG_PRINT2("init_device_selfB start \n");
	
	reg4 = NV_GYRO_CTRL_REG4;
	
	if(mode == 0)
	{
		
		reg4 = (reg4|0x02);
	}
	else
	{
		
		reg4 = (reg4|0x06);
	}
	
    data[0] = 0x4F;
    data[1] = NV_GYRO_CTRL_REG2;
    data[2] = NV_GYRO_CTRL_REG3;
    data[3] = reg4;
    data[4] = 0x63;
	
    ret = write_reg_5byte(GYSD_SENSOR_REG_CTRL_REG1, data);


    msleep(GYSD_WAKE_WAITTIME);

	return ret;
}





static int gyro_self_test(char mode)
{
    int ret = 1;
    char fs_mode;
    short max,min;
    struct l3g4200d_lsb_data diag_dataA;
    struct l3g4200d_lsb_data diag_dataB;
    unsigned char read_data = 0;
    unsigned char revertdata_reg1 = 0;
    unsigned char revertdata_reg5 = 0;

    DEBUG_PRINT2("gyro_self_test start \n");

	ret = read_reg_byte(GYSD_SENSOR_REG_CTRL_REG1, &revertdata_reg1);
	if(ret)
	{
		pr_err("%s: init_device_selfA read_reg1 error!\n", __func__);
		return 1;
	}
	ret = read_reg_byte(GYSD_SENSOR_REG_CTRL_REG5, &revertdata_reg5);
	if(ret)
	{
		pr_err("%s: init_device_selfA read_reg5 error!\n", __func__);
		return 1;
	}

    ret = init_device_selfA();
    if(ret)
    {
        pr_err("%s: init_device_selfA error!\n", __func__);
        return 1;
    }
    
    memset(&diag_dataA, 0, sizeof(struct l3g4200d_lsb_data));
    
    ret = get_lsb_data(&diag_dataA);
    if(ret)
    {
        pr_err("%s: get_lsb_data A error!\n", __func__);
        return 1;
    }
    
    
    ret = init_device_selfB(mode);
    if(ret)
    {
        pr_err("%s: init_device_selfB error!\n", __func__);
        return 1;
    }
    msleep(100);
    
    memset(&diag_dataB, 0, sizeof(struct l3g4200d_lsb_data));
    
    ret = get_lsb_data(&diag_dataB);
    if(ret)
    {
        pr_err("%s: get_lsb_data B error!\n", __func__);
        return 1;
    }
    
    ret = read_reg_byte(GYSD_SENSOR_REG_CTRL_REG4, &read_data);
    if(ret)
    {
        pr_err("%s: read_reg_byte B error!\n", __func__);
        return 1;
    }
    fs_mode = (( read_data >> 4 ) & 0x03);

	if(fs_mode == 0x00)
	{
		max = 29714;
		min = 1143;
	}
	else if(fs_mode == 0x01)
	{
		max = 20571;
		min = 2286;
	}
	else if(fs_mode == 0x02)
	{
		max = 11571;
		min = 1429;
	}
	else
	{
		max = 11571;
		min = 1429;
	}
	
	
	
	if( (abs(diag_dataB.lsb_x - diag_dataA.lsb_x) > max) ||
		(abs(diag_dataB.lsb_x - diag_dataA.lsb_x) < min) )
	{
		return 1;
	}
	
	if( (abs(diag_dataB.lsb_y - diag_dataA.lsb_y) > max) ||
		(abs(diag_dataB.lsb_y - diag_dataA.lsb_y) < min) )
	{
		return 1;
	}
	
	if( (abs(diag_dataB.lsb_z - diag_dataA.lsb_z) > max) ||
		(abs(diag_dataB.lsb_z - diag_dataA.lsb_z) < min) )
	{
		return 1;
	}
    ret |= write_reg_byte(GYSD_SENSOR_REG_CTRL_REG4, NV_GYRO_CTRL_REG4);
    ret |= write_reg_byte(GYSD_SENSOR_REG_CTRL_REG1, revertdata_reg1);
    ret |= write_reg_byte(GYSD_SENSOR_REG_CTRL_REG5, revertdata_reg5);
	if(ret)
	{
		pr_err("%s: write_reg_byte error!\n", __func__);
		return 1;
	}
	
	return 0;
}







static int gyro_offset_calc(unsigned char count,short *OutputData)
{
    char ishpf = 0;
    int i;
    struct l3g4200d_lsb_data diag_data;
    short max_x,max_y,max_z;
    short min_x,min_y,min_z;
    long sumx=0;
    long sumy=0;
    long sumz=0;
    int ret = 1;
    
    DEBUG_PRINT2("gyro_offset_calc start \n");
    
    if(count == 0)
    {
        return 3;
    }
    
    ishpf = ((NV_GYRO_CTRL_REG5&0x10)>>4);
    
    if(ishpf == 1)
    {
        return 2;
    }
    
    
    max_x = max_y = max_z = -32768;
    
    min_x = min_y = min_z = 32767;
    
    sumx=0;
    sumy=0;
    sumz=0;
    
    for(i=0; i<count ; i++)
    {
        memset(&diag_data, 0, sizeof(struct l3g4200d_lsb_data));
        
        ret = get_lsb_data(&diag_data);
        if(ret)
        {
            pr_err("%s: get_lsb_data error!\n", __func__);
            return 3;
        }
        
        
        if(max_x < diag_data.lsb_x)
        {
            max_x = diag_data.lsb_x;
        }
        if(min_x > diag_data.lsb_x)
        {
            min_x = diag_data.lsb_x;
        }
        sumx += diag_data.lsb_x;

		
		if(max_y < diag_data.lsb_y)
		{
			max_y = diag_data.lsb_y;
		}
		if(min_y > diag_data.lsb_y)
		{
			min_y = diag_data.lsb_y;
		}
		sumy += diag_data.lsb_y;
		
		
		if(max_z < diag_data.lsb_z)
		{
			max_z = diag_data.lsb_z;
		}
		if(min_z > diag_data.lsb_z)
		{
			min_z = diag_data.lsb_z;
		}
		sumz += diag_data.lsb_z;
        msleep(20);
	}
	

	if(abs(max_x - min_x) > (NV_GYRO_THRD_GY_X2 * 2) )
	{
		return 1;
	}
	
	if(abs(max_y - min_y) > (NV_GYRO_THRD_GY_Y2 * 2) )
	{
		return 1;
	}
	
	if(abs(max_z - min_z) > (NV_GYRO_THRD_GY_Z2 * 2) )
	{
		return 1;
	}
	
	
	OutputData[0] = (0 - (sumx/count));
	OutputData[1] = (0 - (sumy/count));
	OutputData[2] = (0 - (sumz/count));
	
	return 0;
}


static const struct file_operations l3g4200d_fops = {
	.owner = THIS_MODULE,
	.open = Gysd_open,
	.release = Gysd_close,
	.unlocked_ioctl = Gysd_ioctl,
};





static struct miscdevice l3g4200d_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gyro_sensor",
	.fops = &l3g4200d_fops,
};

static int __devinit l3g4200d_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct l3g4200d_data *data;
	int err = 0;

    printk( KERN_INFO "l3g4200d_probe start\n" );

	DEBUG_PRINT( "%s\n",__FUNCTION__ );

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
		printk(KERN_ERR "i2c_check_functionality error\n");
		goto exit;
	}

	

	if (!(data = kmalloc(sizeof(struct l3g4200d_data), GFP_KERNEL)))
	{
		err = -ENOMEM;
		printk(KERN_ERR "kmalloc error\n");
		goto exit;
	}
	memset(data, 0, sizeof(struct l3g4200d_data));

	i2c_set_clientdata(client, data);
	gyro_client = client;

	
	err = misc_register(&l3g4200d_device);
	if (err) {
		printk(KERN_ERR "l3g4200d device register failed\n");
		goto exit_kfree;
	}
	printk(KERN_INFO "l3g4200d device create ok\n");
	
	msleep(300);
	
	if(g_gyro_regulator == GYRO_REGULATOR_OFF)
	{
		err = nc_pm8921_vreg_set_enable();
   		if(err != 0)
		{
			printk(KERN_ERR "nc_pm8921_vreg_set failed\n");
			goto exit_kfree;
		}
	}
	
	return 0;
exit_kfree:
	kfree(data);
exit:
	return err;
}

static int l3g4200d_remove(struct i2c_client *client)
{
	struct l3g4200d_data *data;

	DEBUG_PRINT( "%s\n",__FUNCTION__ );

	misc_deregister(&l3g4200d_device);
	data = i2c_get_clientdata(client);
	gyro_client = NULL;

	kfree(data);
	return 0;
}

static const struct i2c_device_id l3g4200d_id[] = {
	{ "l3g4200d", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, l3g4200d_id);
static struct i2c_driver l3g4200d_driver = {
	.probe		= l3g4200d_probe,
	.remove		= l3g4200d_remove,
	.id_table	= l3g4200d_id   ,
	.driver = {
		.owner	= THIS_MODULE,	
		.name	= "l3g4200d",
	},
};

static int __init l3g4200d_init(void)
{
	DEBUG_PRINT( "%s\n",__FUNCTION__ );

	return i2c_add_driver(&l3g4200d_driver);
}

static void __exit l3g4200d_exit(void)
{
	DEBUG_PRINT( "%s\n",__FUNCTION__ );
	i2c_del_driver(&l3g4200d_driver);
}


MODULE_DESCRIPTION("L3G4200D driver");

module_init(l3g4200d_init);
module_exit(l3g4200d_exit);



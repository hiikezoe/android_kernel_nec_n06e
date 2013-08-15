/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/







#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>





#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>

#include <linux/firmware.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <sound/soc.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include <linux/input.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <sound/es310_i2c.h>
#include <linux/mfd/pm8xxx/pm8921.h>
#include <linux/gpio.h>

#include "es310_configs.h"
#include "es310.h"

#include <mach/rpm-regulator-8960.h>
extern int nc_pm8921_ldo_set_lpm(unsigned int ldo_id);
extern int nc_pm8921_ldo_set_npm(unsigned int ldo_id);

#define ES310_ACK_VAL(reg,val) (((reg&0xffff)<<16)|val)
#define ES310_BOOT_CMD 0x0001
#define ES310_BOOTACK_CMD 0x0101

#define ES310_SYNC_REG	0x8000
#define ES310_SYNC_BOOT_VAL 0x0000
#define ES310_SYNC_WAKEUP_VAL 0x0001
#define ES310_SYNC_ACK_VAL 0x80000000
#define ES310_SLEEP_REG	0x8010
#define ES310_SLEEP_REG_VAL 0x0001
#define ES310_SLEEP_ACK_VAL 0x80100001

#define ES310_SET_ALGORITHM_PARMID_CMD  0x8017
#define ES310_SET_ALGORITHM_PARMVAL_CMD 0x8018
#define ES310_HYPER_CLEAR_VOICE_PARMID  0x0009
#define ES310_SLOW_VOICE_PARMID         0x0010
#define ES310_EQ_SELECT_VOICE_PARMID    0x001F

#define ES310_WRITE_DATA_BLOCK_CMD      0x802F

#define ONETIME_WRITE_SIZE 0x2800
#define ES310_FW_READ_ATTEMPTS 15
#define ES310_FW_READ_TIMEOUT 2000000



static int es310_sleep(void);
static int es310_wakeup(void);
static int es310_boot_cmd(void);

typedef enum {
	ES310_OFF,
	ES310_CT,
	ES310_FT,
	ES310_HPnHMT,
	ES310_MOVIEMAINMIC,
	ES310_MOVIESUBMIC,
	ES310_VRECORD,
	ES310_VRECOGNITION,
	ES310_VRECOGNITION_NS,

	ES310_DIAG_MAINMIC,
	ES310_DIAG_SUBMIC,

	ES310_MAX_MODE
} es310_mode_type;

#define ES310_STD_PARAM_FILES_FOLDER "/etc/es310_macros/"
#define ES310_PARAM_FILE_PATH_MAX_LENGTH 128
#define ES310_PARAM_FILE_MAX_SIZE 1024

static char *es310_std_param_filename[ES310_MAX_MODE] = {
	NULL,
	"CT_Value.txt",
	"FT_Value.txt",
	"HPnHMT_Value.txt",
	"MWMain-MIC_Value.txt",
	"MWSub-MIC_Value.txt",
	"Vrecoding_Value.txt",
	"VRecognition_Value.txt",
	"VRecognition_ns_Value.txt",

	"DiagWithMain_Value.txt",
	"DiagWithSub_Value.txt",

};

static const char *es310_mode_ctl[ES310_MAX_MODE] = {
	"OFF",              
	"CT",               
	"FT",               
	"HPnHMT",           
	"MOVIEMAINMIC",     
	"MOVIESUBMIC",      
	"VRECORD",          
	"VRECOGNITION",     
	"VRECOGNITION_NS",  

	"DIAGMAINMIC",      
	"DIAGSUBMIC",       

};
static const char *es310_hyper_clear_voice[] = {
	"OFF",
	"ON",
};
static const char *es310_slow_voice[] = {
	"OFF",
	"ON",
};
static const char *es310_eq_select_voice[] = {
	"OFF",
	"SOFTEN_HIGH",
	"SOFTEN_MID",
	"SOFTEN_LOW",
	"RESOUND_HIGH",
	"RESOUND_MID",
	"RESOUND_LOW",
};
static const char *es310_eq_enable[] = {
	"OFF",
	"ON",
};


static const char *es310_noisesuppressor[] = {
	"OFF",
	"ON",
};



static const char *es310_rx_enable[] = {
	"OFF",
	"ON",
};
static const char *es310_tx_enable[] = {
	"OFF",
	"ON",
};
static const char *es310_rxtx_enable[] = {
	"OFF",
	"ON",
};
static int es310_rx_enable_state = 0;
static int es310_tx_enable_state = 0;
static int es310_rxtx_enable_state = 0;




static const char *es310_bootup[] = {
	"BOOTUP",
};



static unsigned short *es310_mode_value[ES310_MAX_MODE] = {NULL};

enum es310_eq_select_voice_mode_type {
	ES310_EQ_VOICE_OFF,
	ES310_EQ_SOFTEN_HIGH,
	ES310_EQ_SOFTEN_MID,
	ES310_EQ_SOFTEN_LOW,
	ES310_EQ_RESOUND_HIGH,
	ES310_EQ_RESOUND_MID,
	ES310_EQ_RESOUND_LOW,
	ES310_EQ_VOICE_MAX_MODE,
};

#define ES310_EQ_SELECT_VOICE_FILE_FOLDER "/etc/es310_eq_patterns/"

static const char *es310_eq_select_voice_filename[ES310_EQ_VOICE_MAX_MODE] = {
	"off.bin",           
	"yawaraka_high.bin", 
	"yawaraka_mid.bin",  
	"yawaraka_low.bin",  
	"hakkiri_high.bin",  
	"hakkiri_mid.bin",   
	"hakkiri_low.bin",   
};

enum es310_anc_mode {
	ANC_ACTIVE = 0,
	ANC_BYPASS = 1,
	ANC_STANDBY = 2,
	ANC_OFF = 3,
};

struct es310_priv {
	struct i2c_client *i2c;
	int anc_download_size;
	char *anc_download;
	int mode_data_size;
	unsigned short *mode_data;
	int (*clk_on)(int in_on);
	int (*pwr_on)(int in_on);
	int reset_gpio;
	int uart_sin_gpio;
	int uart_sout_gpio;
	struct mutex	mutext_lock;
	unsigned char rxdata[2];
	struct delayed_work firmware_dwork;
};

static struct i2c_client *es310_i2c;
static int es310_mode_control;

static int firmware_download_complete=0;


static unsigned short es310_noisesuppressor_state = 0;
static int get_noisesuppressor(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);
static int set_noisesuppressor(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);

static unsigned short es310_hyper_clear_voice_state = 1;
static unsigned short es310_slow_voice_state = 0;
static enum es310_eq_select_voice_mode_type es310_eq_select_voice_mode = ES310_EQ_VOICE_OFF;
static unsigned int es310_eq_enable_state = 0;
static int is_sleeping = 0;
static int es310_load_eq_select_voice_data(enum es310_eq_select_voice_mode_type mode);
static int es310_set_mode(es310_mode_type mode);
static int es310_write_data_block(unsigned char buf[], unsigned short len);
static int es310_read_conf_user_files(void *);
static int es310_read_conf_user_file(es310_mode_type);
static unsigned short convert_hexa_string_to_integer(char *);
static struct task_struct *task1;
static int g_i2c_error_alarmed = 0;


static int get_bootup(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);
static int set_bootup(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);








static struct pm_gpio es310_wakeup_gpio_config = {
	.direction      = PM_GPIO_DIR_OUT,
	.pull           = PM_GPIO_PULL_NO,
	.out_strength   = PM_GPIO_STRENGTH_HIGH,
	.function       = PM_GPIO_FUNC_NORMAL,
	.inv_int_pol    = 0,
	.vin_sel        = PM_GPIO_VIN_S4,
	.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
	.output_value   = 0,
};
static int i2c_master_send_wrapper(const struct i2c_client *client, const char *buf, int count)
{
	unsigned char info1 = 0x01; 
	int ret = 0;
	ret = i2c_master_send(client, buf, count);
	if (ret == count) {
		g_i2c_error_alarmed = 0;
	} else {
		if (g_i2c_error_alarmed == 0) {
			if (1 < count) {
				info1 |= (0x02 << 2); 
				pr_err( "[T][ARM]Event:0x7A Info:0x%02x%02x%02x%02x\n",info1, (-ret) & 0xFF, buf[0], buf[1]);
			} else {
				info1 |= (0x01 << 2); 
				pr_err( "[T][ARM]Event:0x7A Info:0x%02x%02x%02x%02x\n",info1, (-ret) & 0xFF, buf[0], 0x00);
			}
			g_i2c_error_alarmed = 1;
		}
	}
	return ret;
}

static int i2c_master_recv_wrapper(const struct i2c_client *client, char *buf, int count)
{
	unsigned char info1 = 0x02; 
	int ret;
	ret = i2c_master_recv(client, buf, count);
	if (ret == count) {
		g_i2c_error_alarmed = 0;
	} else {
		if (g_i2c_error_alarmed == 0) {
			info1 |= (0x00 << 2); 
			pr_err( "[T][ARM]Event:0x7A Info:0x%02x%02x%02x%02x\n",info1, (-ret) & 0xFF, 0x00, 0x00);
			g_i2c_error_alarmed = 1;
		}
	}
	return ret;
}


static int es310_i2c_write_reg(struct i2c_client *i2c, unsigned short reg_address)
{
	int ret;
	unsigned char txdata[2];
	struct es310_priv *es310 = i2c_get_clientdata(i2c);

	mutex_lock(&es310->mutext_lock);
	txdata[0] = (reg_address >> 8) & 0xFF;
	txdata[1] = reg_address & 0xFF;	

	ret = i2c_master_send_wrapper(i2c, txdata, 2);
	mutex_unlock(&es310->mutext_lock);

	if (ret == 2) {
		return 0;
	} else if (ret < 0) {
		return ret;
	} else {
		return -EIO;
	}
}


static int es310_i2c_write_reg_16(struct i2c_client *i2c, unsigned short reg_address,unsigned short value)
{
	struct es310_priv *es310 = i2c_get_clientdata(i2c);
	int ret;
	unsigned char txdata[4];

	mutex_lock(&es310->mutext_lock);
	txdata[0] = (reg_address >> 8) & 0xFF;
	txdata[1] = reg_address & 0xFF;
	txdata[2] = (value >> 8) & 0xFF;
	txdata[3] = value & 0xFF;
	pr_debug("%s: send 0x%02x%02x 0x%02x%02x", __func__,
		txdata[0], txdata[1], txdata[2], txdata[3]);
	ret = i2c_master_send_wrapper(i2c, txdata, 4);
	mutex_unlock(&es310->mutext_lock);

	if (ret == 4) {
		return 0;
	} else if (ret < 0) {
		return ret;
	} else {
		return -EIO;
	}
}


static int es310_i2c_read_reg_16(struct i2c_client *i2c,unsigned short *val)
{
	struct es310_priv *es310 = i2c_get_clientdata(i2c);
	int ret;
	unsigned char rxdata[2]={0,};

	mutex_lock(&es310->mutext_lock);
	ret = i2c_master_recv_wrapper(i2c, rxdata, 2);
	mutex_unlock(&es310->mutext_lock);	
	if (ret == 2) {
		*val = (rxdata[0] << 8) | rxdata[1];
		return 0;
	} else if (ret < 0) {
		return ret;
	} else {
		return -EIO;	
	}
}


static int es310_i2c_read_reg_32(struct i2c_client *i2c,unsigned int *val)
{
	struct es310_priv *es310 = i2c_get_clientdata(i2c);
	int ret;
	unsigned char rxdata[4]={0,};




	mutex_lock(&es310->mutext_lock);
	ret = i2c_master_recv_wrapper(i2c, rxdata, 4);
	mutex_unlock(&es310->mutext_lock);	
	if (ret == 4) {
		*val = (rxdata[0] << 24) | (rxdata[1] << 16) | (rxdata[2] << 8) | rxdata[3];



		pr_debug("%s: val:0x%08x", __func__, *val);
		return 0;
	} else if (ret < 0) {
		return ret;
	} else {
		return -EIO;
	}
}

static int es310_i2c_write_ndata(struct i2c_client *i2c) 
{
	int ret;
	int count = 0;
	int index_16, total_cmd;
	int ack;
	struct es310_priv *es310 = i2c_get_clientdata(i2c);
	index_16 = es310->mode_data_size/sizeof(unsigned short);
	total_cmd = index_16/2;
	do{
		ret = es310_i2c_write_reg_16(i2c,es310->mode_data[count*2],es310->mode_data[(count*2)+1]);
		es310_i2c_read_reg_32(i2c,&ack);
		count++;
	} while(count<total_cmd);
	return 0;
}

static int get_mode_control(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	pr_debug("%s: mode_control = %d", __func__, es310_mode_control);
	ucontrol->value.integer.value[0] = es310_mode_control;
	return 0;
}

static int set_mode_control(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	pr_debug("%s: cur mode : %d, new mode: %ld", __func__, es310_mode_control, ucontrol->value.integer.value[0]);

	if (ES310_OFF == ucontrol->value.integer.value[0]) {
		if (es310_rx_enable_state
			|| es310_tx_enable_state
			|| es310_rxtx_enable_state)
		{
			pr_debug("%s: we will not set es310 to OFF while rx,tx or rxtx state is on", __func__);
			return 1;
		}
	}

	
	if (es310_mode_control == ucontrol->value.integer.value[0]) {
		return 1;
	}

	es310_mode_control = ucontrol->value.integer.value[0];
	
	 if(es310_mode_control==ES310_VRECOGNITION){
		if(es310_noisesuppressor_state){
			es310_set_mode(ES310_VRECOGNITION_NS);
		}else
		{
			es310_set_mode(ES310_VRECOGNITION);
		}
	}else
	{
		es310_set_mode(es310_mode_control);
	}
	return 1;


}

static int apply_hyper_clear_voice(struct i2c_client *client, unsigned short state)
{
	int ack;
	pr_debug("%s: %d", __func__, state);

	es310_i2c_write_reg_16(client, ES310_SET_ALGORITHM_PARMID_CMD,
		ES310_HYPER_CLEAR_VOICE_PARMID);
	es310_i2c_read_reg_32(client, &ack);
	es310_i2c_write_reg_16(client, ES310_SET_ALGORITHM_PARMVAL_CMD, state);
	es310_i2c_read_reg_32(client, &ack);

	return 0;
}
static int get_hyper_clear_voice(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	pr_debug("%s: ", __func__);
	ucontrol->value.integer.value[0] = es310_hyper_clear_voice_state;
	return 0;
}
static int set_hyper_clear_voice(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct i2c_client *client = es310_i2c;
	pr_debug("%s: cur: %d new: %ld", __func__, 
		es310_hyper_clear_voice_state,
		ucontrol->value.integer.value[0]);

	
	if (es310_hyper_clear_voice_state == ucontrol->value.integer.value[0]) {
		return 1;
	}

	es310_hyper_clear_voice_state = (unsigned short)ucontrol->value.integer.value[0];
	if (es310_eq_enable_state) {
		apply_hyper_clear_voice(client, es310_hyper_clear_voice_state);
	}

	return 1;
}

static int apply_slow_voice(struct i2c_client *client, unsigned short state)
{
	int ack;
	pr_debug("%s: %d", __func__, state);

	es310_i2c_write_reg_16(client, ES310_SET_ALGORITHM_PARMID_CMD,
		ES310_SLOW_VOICE_PARMID);
	es310_i2c_read_reg_32(client, &ack);
	es310_i2c_write_reg_16(client, ES310_SET_ALGORITHM_PARMVAL_CMD, state);
	es310_i2c_read_reg_32(client, &ack);

	return 0;
}
static int get_slow_voice(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = es310_slow_voice_state;
	pr_debug("%s: ", __func__);
	return 0;
}
static int set_slow_voice(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct i2c_client *client = es310_i2c;
	pr_debug("%s: cur: %d new: %ld", __func__, 
		es310_slow_voice_state,
		ucontrol->value.integer.value[0]);

	
	if (es310_slow_voice_state == ucontrol->value.integer.value[0]) {
		return 1;
	}

	es310_slow_voice_state = (unsigned short)ucontrol->value.integer.value[0];
	if (es310_eq_enable_state) {
		apply_slow_voice(client, es310_slow_voice_state);
	}

	return 1;
}

static int apply_eq_select_voice(struct i2c_client *client, unsigned short mode)
{
	int ack;
	pr_debug("%s: %d", __func__, mode);

	
	es310_i2c_write_reg_16(client, ES310_SET_ALGORITHM_PARMID_CMD,
		ES310_EQ_SELECT_VOICE_PARMID);
	es310_i2c_read_reg_32(client, &ack);
	es310_i2c_write_reg_16(client, ES310_SET_ALGORITHM_PARMVAL_CMD,
		0x0000); 
	es310_i2c_read_reg_32(client, &ack);

	
	es310_load_eq_select_voice_data(mode);

	
	es310_i2c_write_reg_16(client, ES310_SET_ALGORITHM_PARMID_CMD,
		ES310_EQ_SELECT_VOICE_PARMID);
	es310_i2c_read_reg_32(client, &ack);
	es310_i2c_write_reg_16(client, ES310_SET_ALGORITHM_PARMVAL_CMD,
		0x0001); 
	es310_i2c_read_reg_32(client, &ack);
	return 1;
}
static int get_eq_select_voice(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = es310_eq_select_voice_mode;
	pr_debug("%s: ", __func__);
	return 0;
}
static int set_eq_select_voice(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct i2c_client *client = es310_i2c;
	pr_debug("%s: cur: %d new: %ld", __func__, 
		es310_eq_select_voice_mode,
		ucontrol->value.integer.value[0]);

	if (ucontrol->value.integer.value[0] < ES310_EQ_VOICE_OFF
		|| ucontrol->value.integer.value[0] >= ES310_EQ_VOICE_MAX_MODE)
	{
		pr_err("invalid es310 EQ select voice mode:%lu\n", ucontrol->value.integer.value[0]);
		return 1;
	}
	
	if (es310_eq_select_voice_mode == ucontrol->value.integer.value[0]) {
		return 1;
	}

	es310_eq_select_voice_mode = (unsigned short)ucontrol->value.integer.value[0];

	if (es310_eq_enable_state) {
		apply_eq_select_voice(client, es310_eq_select_voice_mode);
	}

	return 1;
}

static int es310_load_eq_select_voice_data(enum es310_eq_select_voice_mode_type mode)
{
	char file_path[ES310_PARAM_FILE_PATH_MAX_LENGTH];
	unsigned char *config_value = NULL;
	struct file *f = NULL;
	mm_segment_t fs;
	int ret = 0;
	int len = 0;
	if (mode < ES310_EQ_VOICE_OFF || mode >= ES310_EQ_VOICE_MAX_MODE)
	{
		pr_err("Invalid es310 eq voice mode: %d\n", mode);
		return -1;
	}
	
	sprintf(file_path, "%s/%s", ES310_EQ_SELECT_VOICE_FILE_FOLDER, es310_eq_select_voice_filename[mode]);
	f = filp_open(file_path, O_RDONLY, 0);
	if (IS_ERR(f)) {
		pr_err("fail to open es310 eq pattern file: %s errno: %lu.\n", file_path, -PTR_ERR(f));
		return -1;
	}
	config_value = kmalloc(ES310_PARAM_FILE_MAX_SIZE+1, GFP_KERNEL);
	if (NULL == config_value) {
		pr_err("%s: Out of memory\n", __func__);
		filp_close(f, NULL);
		return -ENOMEM;
	}
	
	fs = get_fs();
	
	set_fs(get_ds());
	len = f->f_op->read(f, config_value, ES310_PARAM_FILE_MAX_SIZE, &f->f_pos);
	if (0 == len) {
		pr_warn("es310 eq pattern file %s is empty!\n", file_path);
	} else if (len < 0) {
		pr_err("es310 eq pattern file %s read error(%d)\n", file_path, -ret);
	} else {
		pr_debug("es310 eq pattern file %s len:%d\n", file_path, len);
		
		es310_write_data_block(config_value, len);
	}
	kfree(config_value);
	set_fs(fs);
	filp_close(f,NULL);

	return 0;
}

static int get_eq_enable(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = es310_eq_enable_state;
	pr_debug("%s: ", __func__);
	return 0;
}
static int set_eq_enable(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct i2c_client *client = es310_i2c;
	pr_debug("%s: cur: %d new: %ld", __func__, 
		es310_eq_enable_state,
		ucontrol->value.integer.value[0]);

	
	if (es310_eq_enable_state == ucontrol->value.integer.value[0]) {
		return 1;
	}

	es310_eq_enable_state = ucontrol->value.integer.value[0];

	if (es310_eq_enable_state) {
		
		if (es310_slow_voice_state) {
			apply_slow_voice(client, es310_slow_voice_state);
		}
		if (es310_hyper_clear_voice_state) {
			apply_hyper_clear_voice(client, es310_hyper_clear_voice_state);
		}

		

			apply_eq_select_voice(client, es310_eq_select_voice_mode);

		


	} else {
		
		
		if (es310_slow_voice_state) {
			apply_slow_voice(client, 0);
		}
		if (es310_hyper_clear_voice_state) {
			apply_hyper_clear_voice(client, 0);
		}
		if (es310_eq_select_voice_mode) {
			apply_eq_select_voice(client, 0);
		}
	}

	return 1;
}


static int get_rx_enable(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = es310_rx_enable_state;
	pr_debug("%s: ", __func__);
	return 0;
}
static int set_rx_enable(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	pr_debug("%s: cur: %d new: %ld", __func__,
		es310_rx_enable_state,
		ucontrol->value.integer.value[0]);
	es310_rx_enable_state = ucontrol->value.integer.value[0];

	return 1;
}
static int get_tx_enable(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = es310_tx_enable_state;
	pr_debug("%s: ", __func__);
	return 0;
}
static int set_tx_enable(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	pr_debug("%s: cur: %d new: %ld", __func__,
		es310_tx_enable_state,
		ucontrol->value.integer.value[0]);
	es310_tx_enable_state = ucontrol->value.integer.value[0];

	return 1;
}
static int get_rxtx_enable(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = es310_rxtx_enable_state;
	pr_debug("%s: ", __func__);
	return 0;
}
static int set_rxtx_enable(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	pr_debug("%s: cur: %d new: %ld", __func__,
		es310_rxtx_enable_state,
		ucontrol->value.integer.value[0]);
	es310_rxtx_enable_state = ucontrol->value.integer.value[0];

	return 1;
}


static const struct soc_enum es310_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(es310_mode_ctl),es310_mode_ctl),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(es310_hyper_clear_voice),es310_hyper_clear_voice),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(es310_slow_voice),es310_slow_voice),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(es310_eq_select_voice),es310_eq_select_voice),
	
        SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(es310_noisesuppressor),es310_noisesuppressor),
	


	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(es310_bootup),es310_bootup),


	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(es310_eq_enable),es310_eq_enable),

	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(es310_rx_enable),es310_rx_enable),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(es310_tx_enable),es310_tx_enable),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(es310_rxtx_enable),es310_rxtx_enable),

};

static const struct snd_kcontrol_new es310_controls[] = {
	SOC_ENUM_EXT("eS310mode", es310_enum[0], get_mode_control,
		set_mode_control),
	SOC_ENUM_EXT("eS310HyperClearVoice", es310_enum[1], get_hyper_clear_voice,
		set_hyper_clear_voice),
	SOC_ENUM_EXT("eS310SlowVoice", es310_enum[2], get_slow_voice,
		set_slow_voice),
	SOC_ENUM_EXT("eS310EQSelectVoice", es310_enum[3], get_eq_select_voice,
		set_eq_select_voice),
        
        SOC_ENUM_EXT("eS310NoiseSuppressor", es310_enum[4], get_noisesuppressor,
		 set_noisesuppressor),
	



	SOC_ENUM_EXT("eS310BootUp", es310_enum[5], get_bootup,
		set_bootup),


	SOC_ENUM_EXT("eS310EQEnable", es310_enum[6], get_eq_enable,
		set_eq_enable),

	SOC_ENUM_EXT("eS310RX", es310_enum[7], get_rx_enable,
		set_rx_enable),
	SOC_ENUM_EXT("eS310TX", es310_enum[8], get_tx_enable,
		set_tx_enable),
	SOC_ENUM_EXT("eS310RXTX", es310_enum[9], get_rxtx_enable,
		set_rxtx_enable),

};


int es310_add_controls(struct snd_soc_codec *codec)
{
	pr_debug( "func = %s\n",__func__);
	if (!es310_i2c) {
		pr_err("ES310 not yet probed\n");
		return -ENODEV;
	}
	return snd_soc_add_codec_controls(codec, es310_controls,
			ARRAY_SIZE(es310_controls));
}
EXPORT_SYMBOL_GPL(es310_add_controls);


int es310_isactive_noisesuppressor(void)
{
	if (es310_noisesuppressor_state) {
		if(es310_mode_control==ES310_VRECOGNITION){
			return 1;
		}
	}
	return 0;
}


static int es310_boot_cmd(void)
{
	struct i2c_client *client = es310_i2c;
	struct es310_priv *es310 = i2c_get_clientdata(client);
	unsigned short buf = 0;
	int count=0;
	int ret;
	int retry;
	
	pr_debug("%s start !!!\n", __func__);
	retry=0;
	do
	{
		es310_i2c_write_reg(client,ES310_BOOT_CMD);
		mdelay(3);
		do{
			count++;
			mdelay(1);
			ret = es310_i2c_read_reg_16(client,&buf);
		}while((buf != ES310_BOOTACK_CMD)&&(count<100));
		if(buf != ES310_BOOTACK_CMD)
		{
			gpio_set_value_cansleep(es310->reset_gpio, 0);
			pr_debug("%s: sleep 10 ms low reset \n", __func__);
			mdelay(10);
			gpio_set_value_cansleep(es310->reset_gpio, 1);
			pr_debug("%s: sleep 50 ms hight reset \n", __func__);
			mdelay(50);
			pr_warn("boot_cmd error retry %d\n",retry);
		}else
		{
			 break;
		}
		retry++;
	}while(retry<10);
	if(retry >= 10)
	{
		pr_err("%s Read Boot Ack failed\n", __func__);
		gpio_set_value_cansleep(es310->reset_gpio, 0);
		nc_pm8921_ldo_set_lpm(RPM_VREG_ID_PM8921_L8);
		return -EINVAL;
	}
	pr_debug("%s Read Boot Ack success\n", __func__);
	return 0;
}




static int get_noisesuppressor(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = es310_noisesuppressor_state;
	pr_debug("%s: ", __func__);
	return 0;
}

static int set_noisesuppressor(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	pr_debug( "%s set_noisesuppressor line = %d \n", __func__,__LINE__);
	
	if ( es310_noisesuppressor_state == ucontrol->value.integer.value[0]) {
		return 1;
	}
	pr_debug( "noisesuppressor_state %d,es310_mode_control %d line = %d \n",es310_noisesuppressor_state,es310_mode_control,__LINE__);
	es310_noisesuppressor_state = (unsigned short)ucontrol->value.integer.value[0];
	if (es310_noisesuppressor_state) {
		if(es310_mode_control==ES310_VRECOGNITION)
		{
			es310_set_mode(ES310_VRECOGNITION_NS);
		}
	} else
	{
		if(es310_mode_control==ES310_VRECOGNITION)
		{
			es310_set_mode(es310_mode_control);
		}
	}
	return 1;
}




int es310_set_mode(es310_mode_type mode)
{
	int i;
	struct i2c_client *client = es310_i2c;
	struct es310_priv *es310 = i2c_get_clientdata(client);
	pr_debug( "%s start mode %d line = %d \n", __func__,mode,__LINE__);

	if (ES310_OFF == mode) {
		es310->mode_data_size = sizeof(ES310_MODE_OFF);
		es310->mode_data = ES310_MODE_OFF;
		es310_i2c_write_ndata(client);
		es310_sleep();
	} else if (ES310_OFF < mode && ES310_MAX_MODE > mode) {
		es310_wakeup();
		if (NULL != es310_mode_value[mode])
		{
			es310->mode_data_size = es310_mode_value[mode][0]*2;
			es310->mode_data = &es310_mode_value[mode][1];
			for(i=1;i<=es310_mode_value[mode][0];i++)
			{
				pr_debug( "%x\n", es310_mode_value[mode][i]);
			}
			es310_i2c_write_ndata(client);
		} else {
			pr_warn("%s the config value for mode %d does not exist!",
					__func__, mode);
		}
	} else {
		
	}
	return 0;
}

static int es310_write_data_block(unsigned char buf[], unsigned short len)
{
	struct i2c_client *client = es310_i2c;
	unsigned int cmdRealAck, cmdExpectedAck;
	
	unsigned int dataRealAck, dataExpectedAck;
	char count=0;
	unsigned short cmd = ES310_WRITE_DATA_BLOCK_CMD;
	int ret;

	cmdExpectedAck = ES310_ACK_VAL(cmd, len);
	
	ret = es310_i2c_write_reg_16(client, cmd, len);
	if(ret != 0)
	{
		pr_err("es310: WDB cmd fail\n");
		return -EIO;
	}
	
	do{
		count++;
		msleep(1);
		ret = es310_i2c_read_reg_32(client,&cmdRealAck);
	} while ((cmdExpectedAck != cmdRealAck) && (count<20));
	if(ret != 0 || count >= 20)
	{
		pr_err("es310: fail to get WDB cmd ack\n");
		return -EIO;
	}

	
	msleep(10);

	
	ret = i2c_master_send_wrapper(client, buf, len);

	
	msleep(10);

	
	dataExpectedAck = ES310_ACK_VAL(cmd, 0x0000);
	ret = es310_i2c_read_reg_32(client,&dataRealAck);
	if (ret != 0)
	{
		pr_err("es310: fail to get WDB data ack\n");
		return -EIO;
	}
	if (dataRealAck != dataExpectedAck)
	{
		pr_err("es310: WDB data ack incorrect: 0x%08x\n", dataRealAck);
	}

	return ret;
}

static int es310_read_conf_user_files(void* p)
{
	es310_mode_type t = ES310_OFF+1;
	for (; t < ES310_MAX_MODE; t++)
	{
		es310_read_conf_user_file(t);
	}
	return 0;
}


static int es310_read_conf_user_file(es310_mode_type type)
{
	struct file *f=NULL;
	char buf[7],string[7], buf_temp;
	mm_segment_t fs;
	unsigned char i,j=0,k,index=1;
	unsigned int loop_count = 0;
	unsigned short *config_value = NULL;
	char file_path[ES310_PARAM_FILE_PATH_MAX_LENGTH];

	pr_debug("%s() : type:%d \n", __func__, type);

	if (ES310_OFF >= type || ES310_MAX_MODE <= type)
	{
		pr_err("%s() : invalid es310 mode type: %d", __func__, type);
		return -1;
	}

	for(i=0;i<7;i++)
		buf[i] = 0;

	sprintf(file_path, "%s/%s", ES310_STD_PARAM_FILES_FOLDER, es310_std_param_filename[type]);
	f = filp_open(file_path, O_RDONLY, 0);
	if (IS_ERR(f)) {
		pr_err("fail to open es310 eq pattern file: %s errno: %lu.\n", file_path, -PTR_ERR(f));
	} else {
		config_value = kmalloc(ES310_PARAM_FILE_MAX_SIZE+1, GFP_KERNEL);
		if (NULL == config_value) {
			pr_err("%s: Out of memory\n", __func__);
			filp_close(f,NULL);
			return -ENOMEM;
		}
		
		fs = get_fs();
		
		set_fs(get_ds());
		i=0;
		while(loop_count < 800){
			loop_count++;
			
			if(!(f->f_op->read(f, &buf[i], 1, &f->f_pos)))
			{
				pr_debug("End of file. loop_count = %d\n",loop_count);
				break;
			}
			if(buf[i] == ';')
			{
				buf[i]='\0';
				i=0;
				j=0;
				while(j<600) {
					f->f_op->read(f, &buf_temp,1, &f->f_pos);
					if(buf_temp == '\n')
					{
						break;
					}
					j++;
				}
			}
			else if(buf[i] == ' ')
			{
				if(i==6)
				{
					for(k=0;k<6;k++)
						string[k] = buf[k];
					string[k] = '\0';
					config_value[index] = convert_hexa_string_to_integer(string);
					index++;
				}
				i=0;
			}
			else if (buf[i] == '\n')
			{
				i = 0;
			}
			else
			{
				i++;
			}
		}
		config_value[0] = (unsigned short) (index - 1);
		
		set_fs(fs);
		filp_close(f,NULL);

		if (NULL != es310_mode_value[type]) {
			kfree(es310_mode_value[type]);
			es310_mode_value[type] = NULL;
		}
		es310_mode_value[type] = config_value;

		pr_debug("Total Elements -  %d\n",(unsigned short)config_value[0]);
	}

	return 0;
}

static unsigned short convert_hexa_string_to_integer(char *c)
{
	int num,i=2,length=0;
	unsigned short integerVal = 0,multiplier =1;

	pr_debug("convert() : %s\n",c);
	if(c[1] != 'x')
		return 0;
	
	while(c[i] != '\0')
	{
		length++;
		
		i++;
		if(i>10)
		{
			
			break;
		}
	}
	
	while(length != 0)
	{
		if(c[length+1] >= 48 && c[length+1] <= 57)
		{
			num = c[length+1]- 48;
			integerVal = integerVal + num * multiplier;
		}
		else if(c[length+1] >= 65 && c[length+1] <= 70)
		{
			num = c[length+1]- 55;
			integerVal = integerVal + num * multiplier;
		}
		else
		{
			pr_err("Well, something is Wrong. \n");
		}
	multiplier <<= 4;
	length--;
	}
	pr_debug("value - %d\n",integerVal);
	return integerVal;
}

static int es310_firmware_download(void)
{
	struct i2c_client *client = es310_i2c;
	struct es310_priv *es310 = i2c_get_clientdata(client);
	int ret=0;
	char *anc_download;
	int download_size=0;
	unsigned char i=0;
	int count=0,pos_next=0;
	int remain=0;
	pr_debug("%s start\n", __func__);

	if(is_sleeping == 2)
	{
		pr_debug("%s eS310 not boot! start\n", __func__);
		return -EIO;
	}

	anc_download = es310->anc_download;
	download_size = es310->anc_download_size;
	count = download_size/ONETIME_WRITE_SIZE;
	remain = download_size%ONETIME_WRITE_SIZE;
	pr_debug("Downloading %d bytes count %d remain %d\n",es310->anc_download_size,count,remain);
	mutex_lock(&es310->mutext_lock);
	for(i=0;i<count;i++)
	{
		ret = i2c_master_send_wrapper(client,anc_download+pos_next,ONETIME_WRITE_SIZE);
		if(ret == ONETIME_WRITE_SIZE )
		{
			pr_info("FW download #%d OK\n",i);
		}else{
			pr_err("FW download aborted.\n");
			goto FWDL_exit;
		}
		pos_next += ret;
	}
	if(remain)
	{
		ret = i2c_master_send_wrapper(client,anc_download+pos_next,remain);
		if(ret == remain)
		{
			pr_debug("FW download #last OK\n");
		}else{
			pr_warn("FW download error #last code=%d\n",ret);
			pr_err("FW download aborted.\n");
			goto FWDL_exit;
		}
		pos_next += ret;
	}
FWDL_exit:
	mutex_unlock(&es310->mutext_lock);

	if (pos_next != es310->anc_download_size) {
		dev_err(&client->dev, "i2c_transfer() failed, %d != %d\n",
			ret, es310->anc_download_size);
		return -EIO;
	}
	pr_debug("%s complete\n", __func__);
	return 0;
}

static int es310_sync(unsigned short reg,unsigned short val)
{
	struct i2c_client *client = es310_i2c;
	unsigned int synAck,synVal;
	char count = 0;
	int ret = 0;
	
	synVal = ES310_ACK_VAL(reg,val);
	ret = es310_i2c_write_reg_16(client,reg,val);
	if(ret != 0)
	{
		pr_err("%s Sync write fail\n",__func__);
		return -EIO;
	}
	do{
		count++;
		msleep(1);
		ret = es310_i2c_read_reg_32(client,&synAck);
	}while((synAck != synVal)&&(count<20));
	if(ret != 0 || count >= 20)
	{
		pr_err("%s Sync ack fail\n",__func__);
		return -EIO;
	}

	return ret;

}

static int es310_sleep(void)
{
	struct i2c_client *client = es310_i2c;
	struct es310_priv *es310 = i2c_get_clientdata(client);
	if(!firmware_download_complete)
	{
		return 0;
	}
	if(is_sleeping)
		return 0;

	is_sleeping = 1;
	pr_debug("%s start\n", __func__);

	es310_i2c_write_reg_16(client,ES310_SLEEP_REG,ES310_SLEEP_REG_VAL);
	mdelay(20);
	pr_debug("%s: sleep 20 ms after Ctl \n", __func__);

	es310->clk_on(0);

	nc_pm8921_ldo_set_lpm(RPM_VREG_ID_PM8921_L8);


	return 0;
}

static int es310_wakeup(void)
{
	int ret = 0;

	struct i2c_client *client = es310_i2c;
	struct es310_priv *es310 = i2c_get_clientdata(client);
	if(!firmware_download_complete)
	{
		return 0;
	}
	if(!is_sleeping)
		return 0;
	is_sleeping = 0;
	pr_debug("%s start\n", __func__);

	nc_pm8921_ldo_set_npm(RPM_VREG_ID_PM8921_L8);

	es310->clk_on(1);
	pr_debug("%s: sleep 10 ms after Ctl \n", __func__);
	mdelay(40);

	
	gpio_set_value(es310->uart_sin_gpio, 1);
	gpio_set_value(es310->uart_sin_gpio, 0);
	pr_debug("%s: sleep 30 ms after Ctl \n", __func__);
	mdelay(30);

	ret = es310_sync(ES310_SYNC_REG,ES310_SYNC_WAKEUP_VAL);
	if(ret < 0) {
		pr_err("%s ret %d \n",__func__,ret);
		return ret;
	}

	return ret;
}



























static struct pm_gpio es310_uart_sout_gpio_config = {
	.direction      = PM_GPIO_DIR_IN,
	.pull           = PM_GPIO_PULL_NO,
	.out_strength   = PM_GPIO_STRENGTH_NO,
	.function       = PM_GPIO_FUNC_NORMAL,
	.inv_int_pol    = 0,
	.vin_sel        = PM_GPIO_VIN_S4,
	.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
	.output_value   = 0,
};

static void es310_fw_read(struct work_struct *work)
{
	struct delayed_work *dwork;
	struct es310_priv *es310;
	const struct firmware *fw;
	int ret = -1, retry = 0;
		
	pr_debug("%s start \n",__func__);
	dwork = to_delayed_work(work);
	es310 = container_of(dwork, struct es310_priv,firmware_dwork);

	while (retry < ES310_FW_READ_ATTEMPTS) {
		retry++;
		pr_debug("%s:Attempt %d to request ES310 firmware\n",
			__func__, retry);
		ret = request_firmware(&fw, "es310_fw.bin",&es310->i2c->dev);

		if (ret != 0) {
			usleep_range(ES310_FW_READ_TIMEOUT,
					ES310_FW_READ_TIMEOUT);
		} else {
			pr_debug("%s: ES310 Firmware read succesful\n", __func__);
			break;
		}
	}
	if (ret != 0) {
		pr_err("%s: Cannot load ES310 firmware use default cal\n",
			__func__);
		
	}else{

		pr_debug("%s fw_size %d \n",__func__,fw->size);
		es310->anc_download_size = fw->size;
		es310->anc_download = kmalloc(es310->anc_download_size, GFP_KERNEL);
		if (es310->anc_download == NULL) {
			dev_err(&es310->i2c->dev, "Out of memory\n");
			ret = -ENOMEM;
		}
		memcpy(es310->anc_download, fw->data, fw->size);
		release_firmware(fw);
		{
			int err,retry;
			retry=0;
			do{
				err = es310_firmware_download();
				if(err)
				{
					pr_debug("Firmware DL retry %d\n", retry);
					pr_debug("%s: reset:%d \n", __func__, es310->reset_gpio	);
					
					gpio_set_value_cansleep(es310->reset_gpio, 0);
					es310->pwr_on(1);
					pr_debug("%s: sleep 1 ms after Ctl \n", __func__);
					mdelay(1);
					es310->clk_on(1);
					pr_debug("%s: sleep 10 ms after Ctl \n", __func__);
					mdelay(10);
					gpio_set_value_cansleep(es310->reset_gpio, 1);
					pr_debug("%s: sleep 50 ms after Ctl \n", __func__);
					mdelay(50);
					pr_debug("es310_boot_cmd()\n");
					err = es310_boot_cmd();
				}else{
					break;
				}
				retry++;
			}while(retry<10);

			kfree(es310->anc_download);
			if(retry >= 10)
			{
				gpio_set_value_cansleep(es310->reset_gpio, 0);
				nc_pm8921_ldo_set_lpm(RPM_VREG_ID_PM8921_L8);
				pr_err("firmware download error\n");
				return;
			}else
			{
				pr_debug("firmware_download_complete retry:%d,\n",retry);
				firmware_download_complete=1;
				msleep(120);
				ret = es310_sync(ES310_SYNC_REG,ES310_SYNC_BOOT_VAL);
				
				pm8xxx_gpio_config(es310->uart_sout_gpio, &es310_uart_sout_gpio_config);
				es310_sleep();
			}
		}
	}
	task1 = kthread_run(es310_read_conf_user_files, NULL, "eS310-conf-file");
	if (IS_ERR(task1)) {
		task1 = NULL;
		pr_err("eS310-conf-file failed to create the thread...\n");
	}
	pr_debug("%s end \n",__func__);










}


static int __devinit es310_i2c_probe(struct i2c_client *client,
					  const struct i2c_device_id *i2c_id)
{
	struct device *dev;
	struct es310_priv *es310;
	struct es310_platform_data *pdata;
	int ret;

	pr_debug("%s start !!!\n", __func__);
	pr_debug("%s:%s:0x%02x:%s\n", client->adapter->name, client->name, client->addr, i2c_id->name);
	dev = &client->dev;

	if (client->dev.platform_data == NULL) {
		return -ENODEV;
	}
	
	if (es310_i2c) {
		pr_err("%s Another es310 is already registered\n", __func__);
		return -EINVAL;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("%s i2c_check_functionality failed\n", __func__);
		return -EINVAL;
	}
	pr_debug("%s i2c_check_functionality success\n", __func__);
	es310_i2c = client;
	
	es310 = kzalloc(sizeof(struct es310_priv), GFP_KERNEL);
	if (es310 == NULL) {
		pr_err("%s Unable to allocate private data\n", __func__);
		return -ENOMEM;
	}
	i2c_set_clientdata(es310_i2c, es310);

	pdata = client->dev.platform_data;

	ret = pdata->dev_setup(1);
	if(ret != 0)
	{
		printk(KERN_ERR"%s Unable to setup gpio/power\n", __func__);	
		goto err_fw;
	}

	es310->pwr_on         = pdata->pwr_on;
	es310->clk_on         = pdata->clk_on;
	es310->reset_gpio     = pdata->reset_gpio;
	es310->uart_sin_gpio  = pdata->uart_sin_gpio;
	es310->uart_sout_gpio = pdata->uart_sout_gpio;

	es310->i2c = client;
	INIT_DELAYED_WORK(&es310->firmware_dwork, es310_fw_read);
	mutex_init(&es310->mutext_lock);

	pr_debug("%s: reset:%d \n", __func__, pdata->reset_gpio);
	
	gpio_set_value_cansleep(es310->reset_gpio, 0);

	es310->pwr_on(1);
	pr_debug("%s: sleep 1 ms after Ctl \n", __func__);
	mdelay(1);

	es310->clk_on(1);
	pr_debug("%s: sleep 10 ms after Ctl \n", __func__);
	mdelay(10);

	gpio_set_value_cansleep(es310->reset_gpio, 1);
	pr_debug("%s: sleep 50 ms after Ctl \n", __func__);
	mdelay(50);

	
	ret = pm8xxx_gpio_config(es310->uart_sin_gpio, &es310_wakeup_gpio_config);
	if (ret) {
		pr_err("%s: pm8921 gpio %d config failed(%d)\n",
				__func__, es310->uart_sin_gpio, ret);
		return ret;
	}

	ret = es310_boot_cmd();
	if(ret != 0)
	{
		dev_err(&client->dev, "Failed to es310_boot_cmd : %d\n", ret);
		is_sleeping = 2;
		goto err_fw;
	}

	schedule_delayed_work(&es310->firmware_dwork,usecs_to_jiffies(ES310_FW_READ_TIMEOUT));
	pr_debug("%s  end !!!\n", __func__);
	return 0;
err_fw:
	kfree(es310);
	i2c_set_clientdata(es310_i2c, NULL);
	es310_i2c = NULL;
	return ret;

}

static __devexit int es310_i2c_remove(struct i2c_client *client)
{
	struct es310_priv *es310 = i2c_get_clientdata(client);
	struct es310_platform_data *pdata;

	pr_debug("%s es310_i2c_remove !!!\n", __func__);

	pdata = client->dev.platform_data;
	pdata->dev_setup(0);

	es310_i2c = NULL;
	kfree(es310->anc_download);
	kfree(es310);

	return 0;
}

static void es310_i2c_shutdown(struct i2c_client *i2c)
{
	pr_debug("%s es310_i2c_shutdown !!!\n", __func__);
}



























static int get_bootup(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	pr_debug("%s: ", __func__);
	return 0;
}

static int set_bootup(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct i2c_client *client = es310_i2c;
	struct es310_priv *es310 = i2c_get_clientdata(client);
	const struct firmware *fw;
	int ret = 0;
	pr_debug("%s: ", __func__);

	if (ucontrol->value.integer.value[0]!=0) {
		pr_err("%s: invalid value:%ld\n", __func__, ucontrol->value.integer.value[0]);
		return 1;
	}

	
	
	gpio_set_value_cansleep(es310->reset_gpio, 0);

	
	es310->pwr_on(1);
	pr_debug("%s: sleep 1 ms after Ctl \n", __func__);
	mdelay(1);

	
	es310->clk_on(1);
	pr_debug("%s: sleep 10 ms after Ctl \n", __func__);
	mdelay(10);

	
	gpio_set_value_cansleep(es310->reset_gpio, 1);
	pr_debug("%s: sleep 50 ms after Ctl \n", __func__);
	mdelay(50);

	
	ret = pm8xxx_gpio_config(es310->uart_sin_gpio, &es310_wakeup_gpio_config);
	if (ret) {
		pr_err("%s: pm8921 gpio %d config failed(%d)\n",
				__func__, es310->uart_sin_gpio, ret);
		return ret;
	}

	
	ret = es310_boot_cmd();
	if(ret != 0)
	{
		pr_err("Failed to es310_boot_cmd : %d\n", ret);
		is_sleeping = 2;
		return ret;
	}

	
	ret = request_firmware(&fw, "es310_fw.bin",&es310->i2c->dev);
	if (ret != 0) {
		pr_err("Failed to request firmware: %d\n", ret);
		return ret;
	}

	pr_debug("%s fw_size %d \n",__func__,fw->size);
	es310->anc_download_size = fw->size;
	es310->anc_download = kmalloc(es310->anc_download_size, GFP_KERNEL);
	if (es310->anc_download == NULL) {
		pr_err("Out of memory\n");
		ret = -ENOMEM;
		return ret;
	}
	memcpy(es310->anc_download, fw->data, fw->size);
	release_firmware(fw);
	es310_firmware_download();
	kfree(es310->anc_download);
	msleep(120);
	ret = es310_sync(ES310_SYNC_REG,ES310_SYNC_BOOT_VAL);
	
	pm8xxx_gpio_config(es310->uart_sout_gpio, &es310_uart_sout_gpio_config);

	return 1;
}



static const struct i2c_device_id es310_i2c_id[] = {
	{ "es310", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, es310_i2c_id);

static struct i2c_driver es310_i2c_driver = {
	.driver = {
		.name = "es310",
		.owner = THIS_MODULE,



	},
	.probe = es310_i2c_probe,
	.remove = __devexit_p(es310_i2c_remove),
	.shutdown = es310_i2c_shutdown,
	.id_table = es310_i2c_id,
};

static int __init es310_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&es310_i2c_driver);
	if (ret != 0) {
		printk(KERN_ERR "Failed to register es310 I2C driver: %d\n",
			ret);
	}

	return ret;
}
module_init(es310_init);

static void __exit es310_exit(void)
{

	i2c_del_driver(&es310_i2c_driver);

}
module_exit(es310_exit);

MODULE_DESCRIPTION("ASoC es310 driver");
MODULE_AUTHOR("Mitsuaki Iwasaki <m-iwasaki@wr.cnt.ncmobile.jp.nec.com>, Jyunji Yamashina<j-yamashina@wr.cnt.ncmobile.jp.nec.com>");
MODULE_LICENSE("GPL");

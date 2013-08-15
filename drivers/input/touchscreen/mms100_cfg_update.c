/*
 * drivers/input/touchscreen/mms100_isc.c - ISC(In-system programming via I2C) enalbes  sensor to be programmed while installed in a complete system.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
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
#include <asm/unaligned.h>
#include "mms100_cfg_update.h"



#define DEFAULT_SLAVE_ADDR                      	0x48

#define SECTION_NUM                           		3
#define SECTION_NAME_LEN                	        5

#define MBIN_NAME_LEN							12


#define PAGE_HEADER                         		3
#define PAGE_DATA                              		1024
#define PAGE_TAIL                            		2
#define PACKET_SIZE                           		(PAGE_HEADER + PAGE_DATA + PAGE_TAIL)

#define TIMEOUT_CNT                          		10

#define TS_WRITE_REGS_LEN	PACKET_SIZE




#define MIP_ADDR_INPUT_INFORMATION           		 0x01

#define ISC_ADDR_VERSION							 0xE1
#define ISC_ADDR_SECTION_PAGE_INFO					 0xE5




#define ISC_CMD_ENTER_ISC							0x5F
#define ISC_CMD_ENTER_ISC_PARA1					0x01
#define ISC_CMD_UPDATE_MODE						0xAE
#define ISC_SUBCMD_ENTER_UPDATE					0x55
#define ISC_SUBCMD_DATA_WRITE						0XF1
#define ISC_SUBCMD_LEAVE_UPDATE_PARA1			0x0F
#define ISC_SUBCMD_LEAVE_UPDATE_PARA2			0xF0
#define ISC_CMD_CONFIRM_STATUS					0xAF

#define ISC_STATUS_UPDATE_MODE					0x01
#define ISC_STATUS_CRC_CHECK_SUCCESS				0x03

#define ISC_CHAR_2_BCD(num)	\
	(((num/10)<<4) + (num%10))
#define ISC_MAX(x, y)		( ((x) > (y))? (x) : (y) )

typedef enum {
	EC_NONE = -1,
	EC_DEPRECATED = 0,
	EC_BOOTLOADER_RUNNING = 1,
	EC_BOOT_ON_SUCCEEDED = 2,
	EC_ERASE_END_MARKER_ON_SLAVE_FINISHED = 3,
	EC_SLAVE_DOWNLOAD_STARTS = 4,
	EC_SLAVE_DOWNLOAD_FINISHED = 5,
	EC_2CHIP_HANDSHAKE_FAILED = 0x0E,
	EC_ESD_PATTERN_CHECKED = 0x0F,
	EC_LIMIT
} eErrCode_t;





















static char* mbin_path = "./";

static const char section_name[SECTION_NUM][SECTION_NAME_LEN] = { "BOOT",
		"CORE", "CONF" };


static const char mbin_name[SECTION_NUM][MBIN_NAME_LEN] =
{ "melfas_boot", "melfas_core", "melfas_conf" };


static const unsigned char crc0_buf[31] = { 0x1D, 0x2C, 0x05, 0x34, 0x95, 0xA4,
		0x8D, 0xBC, 0x59, 0x68, 0x41, 0x70, 0xD1, 0xE0, 0xC9, 0xF8, 0x3F, 0x0E,
		0x27, 0x16, 0xB7, 0x86, 0xAF, 0x9E, 0x7B, 0x4A, 0x63, 0x52, 0xF3, 0xC2,
		0xEB };

static const unsigned char crc1_buf[31] = { 0x1E, 0x9C, 0xDF, 0x5D, 0x76, 0xF4,
		0xB7, 0x35, 0x2A, 0xA8, 0xEB, 0x69, 0x42, 0xC0, 0x83, 0x01, 0x04, 0x86,
		0xC5, 0x47, 0x6C, 0xEE, 0xAD, 0x2F, 0x30, 0xB2, 0xF1, 0x73, 0x58, 0xDA,
		0x99 };

static tISCFWInfo_t mbin_info[SECTION_NUM];
static tISCFWInfo_t ts_info[SECTION_NUM];
static bool section_update_flag[SECTION_NUM];

const struct firmware *fw_mbin[SECTION_NUM];

static unsigned char g_wr_buf[1024 + 3 + 2];


extern unsigned char melfas_ncm_power( unsigned char onoff );


static int mms100_i2c_read(struct i2c_client *client, u16 addr, u16 length,
		u8 *value) {
	struct i2c_adapter *adapter = client->adapter;
	struct i2c_msg msg;
	int ret = -1;

	msg.addr = client->addr;
	msg.flags = 0x00;
	msg.len = 1;
	msg.buf = (u8 *) &addr;

	ret = i2c_transfer(adapter, &msg, 1);

	if (ret >= 0) {
		msg.addr = client->addr;
		msg.flags = I2C_M_RD;
		msg.len = length;
		msg.buf = (u8 *) value;

		ret = i2c_transfer(adapter, &msg, 1);
	}

	if (ret < 0) {
		pr_err("[TSP] : read error : [%d]", ret);
	}

	return ret;
}


static char g_wdata[TS_WRITE_REGS_LEN];

static int mms100_i2c_write(struct i2c_client *client, char *buf, int length) {
	int i;




	if (length > TS_WRITE_REGS_LEN) {
		pr_err("[TSP] %s :size error \n", __FUNCTION__);
		return -EINVAL;
	}





    for (i = 0; i < length; i++)
    {
        g_wdata[i] = *buf++;

    }





    i = i2c_master_send(client, (char *) g_wdata, length);


	if (i == length)
		return length;
	else {
		pr_err("[TSP] :write error : [%d]", i);
		return -EIO;
	}
}

static eISCRet_t mms100_reset(struct i2c_client *_client) {










    pr_info("[touchpanel]%s\n", __func__);

	melfas_ncm_power(0);
	mms100_msdelay(20);
	melfas_ncm_power(1);
	mms100_msdelay(100);


	return ISC_SUCCESS;
}

static eISCRet_t mms100_check_operating_mode(struct i2c_client *_client,
		const eErrCode_t _error_code) {
	int ret;
	unsigned char rd_buf = 0x00;

	pr_info("[TSP ISC] %s\n", __func__);

	
	ret = mms100_i2c_read(_client, ISC_ADDR_VERSION, 1, &rd_buf);

	if (ret < 0) {
		pr_info("[TSP ISC] %s,%d: i2c read fail[%d] \n", __FUNCTION__, __LINE__,
				ret);
		return _error_code;
	}

	pr_info("End mms100_check_operating_mode()\n");

	return ISC_SUCCESS;
}

static eISCRet_t mms100_get_version_info(struct i2c_client *_client) {
	int i, ret;
	unsigned char rd_buf[8];

	pr_info("[TSP ISC] %s\n", __func__);

	
	ret = mms100_i2c_read(_client, ISC_ADDR_VERSION, SECTION_NUM, rd_buf);

	if (ret < 0) {
		pr_info("[TSP ISC] %s,%d: i2c read fail[%d] \n", __FUNCTION__, __LINE__,
				ret);
		return ISC_I2C_ERROR;
	}

	for (i = 0; i < SECTION_NUM; i++)
		ts_info[i].version = rd_buf[i];

	ts_info[SEC_CORE].compatible_version = ts_info[SEC_BOOTLOADER].version;
	ts_info[SEC_CONFIG].compatible_version = ts_info[SEC_CORE].version;

	ret = mms100_i2c_read(_client, ISC_ADDR_SECTION_PAGE_INFO, 8, rd_buf);

	if (ret < 0) {
		pr_info("[TSP ISC] %s,%d: i2c read fail[%d] \n", __FUNCTION__, __LINE__,
				ret);
		return ISC_I2C_ERROR;
	}

	for (i = 0; i < SECTION_NUM; i++) {
		ts_info[i].start_addr = rd_buf[i];

		




		ts_info[i].end_addr = rd_buf[i + SECTION_NUM + 1];
	}

	for (i = 0; i < SECTION_NUM; i++) {
		pr_info("\tTS : Section(%d) version: 0x%02X\n", i, ts_info[i].version);
		pr_info("\tTS : Section(%d) Start Address: 0x%02X\n", i,
				ts_info[i].start_addr);
		pr_info("\tTS : Section(%d) End Address: 0x%02X\n", i,
				ts_info[i].end_addr);
		pr_info("\tTS : Section(%d) Compatibility: 0x%02X\n", i,
				ts_info[i].compatible_version);
	}

	pr_info("End mms100_get_version_info()\n");

	return ISC_SUCCESS;
}

static eISCRet_t mms100_seek_section_info(void) {
#define STRING_BUF_LEN		100

	int i;

	char str_buf[STRING_BUF_LEN];
	char name_buf[SECTION_NAME_LEN];
	int version;
	int page_num;
	int offset[SECTION_NUM] = {0x03f0, 0x53f0 - 0x0400, 0x7bf0 - 0x5400};

	const unsigned char *buf;
	int next_ptr;

	pr_info("[TSP ISC] %s\n", __func__);

	for (i = 0; i < SECTION_NUM; i++) {

        if( fw_mbin[i] )
        {

		buf = fw_mbin[i]->data;

		if (buf == NULL) {
			mbin_info[i].version = ts_info[i].version;
			mbin_info[i].compatible_version = ts_info[i].compatible_version;
			mbin_info[i].start_addr = ts_info[i].start_addr;
			mbin_info[i].end_addr = ts_info[i].end_addr;
		} else {
			next_ptr = 0;

			do {

                if( fw_mbin[i]->size <= next_ptr )
                {
                    pr_info("[TSP ISC] %s,%d : %d > %d: size error. \n", __FUNCTION__, __LINE__, fw_mbin[i]->size , next_ptr);
                    return ISC_FILE_FORMAT_ERROR;
                }

				sscanf(buf + next_ptr, "%s", str_buf);
				next_ptr += strlen(str_buf) + 1;
			} while (!strstr(str_buf, "SECTION_NAME"));
			sscanf(buf + next_ptr, "%s%s", str_buf, name_buf);
			if (strncmp(section_name[i], name_buf, SECTION_NAME_LEN))
				return ISC_FILE_FORMAT_ERROR;

			do {

                if( fw_mbin[i]->size <= next_ptr )
                {
                    pr_info("[TSP ISC] %s,%d : %d > %d: size error. \n", __FUNCTION__, __LINE__, fw_mbin[i]->size , next_ptr);
                    return ISC_FILE_FORMAT_ERROR;
                }

				sscanf(buf + next_ptr, "%s", str_buf);
				next_ptr += strlen(str_buf) + 1;
			} while (!strstr(str_buf, "SECTION_VERSION"));
			sscanf(buf + next_ptr, "%s%d", str_buf, &version);
			mbin_info[i].version = ISC_CHAR_2_BCD(version);

			do {

                if( fw_mbin[i]->size <= next_ptr )
                {
                    pr_info("[TSP ISC] %s,%d : %d > %d: size error. \n", __FUNCTION__, __LINE__, fw_mbin[i]->size , next_ptr);
                    return ISC_FILE_FORMAT_ERROR;
                }

				sscanf(buf + next_ptr, "%s", str_buf);
				next_ptr += strlen(str_buf) + 1;
			} while (!strstr(str_buf, "START_PAGE_ADDR"));
			sscanf(buf + next_ptr, "%s%d", str_buf, &page_num);
			mbin_info[i].start_addr = page_num;

			do {

                if( fw_mbin[i]->size <= next_ptr )
                {
                    pr_info("[TSP ISC] %s,%d : %d > %d: size error. \n", __FUNCTION__, __LINE__, fw_mbin[i]->size , next_ptr);
                    return ISC_FILE_FORMAT_ERROR;
                }

				sscanf(buf + next_ptr, "%s", str_buf);
				next_ptr += strlen(str_buf) + 1;
			} while (!strstr(str_buf, "END_PAGE_ADDR"));
			sscanf(buf + next_ptr, "%s%d", str_buf, &page_num);
			mbin_info[i].end_addr = page_num;

			do {

                if( fw_mbin[i]->size <= next_ptr )
                {
                    pr_info("[TSP ISC] %s,%d : %d > %d: size error. \n", __FUNCTION__, __LINE__, fw_mbin[i]->size , next_ptr);
                    return ISC_FILE_FORMAT_ERROR;
                }

				sscanf(buf + next_ptr, "%s", str_buf);
				next_ptr += strlen(str_buf) + 1;
			} while (!strstr(str_buf, "COMPATIBLE_VERSION"));
			sscanf(buf + next_ptr, "%s%d", str_buf, &version);
			mbin_info[i].compatible_version = ISC_CHAR_2_BCD(version);


















			{
				char *bin_header = "[Binary]";
				int cnt = strlen(bin_header);
				int j;

				for (j = 0; j < cnt;) {
					if (*(buf + next_ptr) == bin_header[j]) {
						j++;
					} else {
						j = 0;
					}
					next_ptr++;
				}
			}
			next_ptr++;

			mbin_info[i].bin_offset = next_ptr;

			mbin_info[i].crc = get_unaligned_le32(buf + next_ptr + offset[i]);

			if (mbin_info[i].version == 0xFF)
				return ISC_FILE_FORMAT_ERROR;
		}

        }
        else
        {
            mbin_info[i].version = ts_info[i].version;
            mbin_info[i].compatible_version = ts_info[i].compatible_version;
            mbin_info[i].start_addr = ts_info[i].start_addr;
            mbin_info[i].end_addr = ts_info[i].end_addr;
        }


	}

	for (i = 0; i < SECTION_NUM; i++) {
		pr_info("\tMBin : Section(%d) Version: 0x%02X\n", i,
				mbin_info[i].version);
		pr_info("\tMBin : Section(%d) Start Address: 0x%02X\n", i,
				mbin_info[i].start_addr);
		pr_info("\tMBin : Section(%d) End Address: 0x%02X\n", i,
				mbin_info[i].end_addr);
		pr_info("\tMBin : Section(%d) Compatibility: 0x%02X\n", i,
				mbin_info[i].compatible_version);
		pr_info("\tMBin : Section(%d) crc : 0x%02X\n", i,
				mbin_info[i].crc);
	}

	pr_info("End mms100_seek_section_info()\n");

	return ISC_SUCCESS;
}

static eISCRet_t mms100_compare_version_info(struct i2c_client *_client) {
	int i;
	
	int target_ver[SECTION_NUM];
	int fw_up_to_date = true;

	pr_info("[TSP ISC] %s\n", __func__);

	if (mms100_get_version_info(_client) != ISC_SUCCESS)
		return ISC_I2C_ERROR;

	mms100_seek_section_info();

	for (i = 0; i < SECTION_NUM; i++) {



		if ((mbin_info[i].version != ts_info[i].version) && fw_mbin[i]) {

			fw_up_to_date = false;
			section_update_flag[i] = true;
			target_ver[i] = mbin_info[i].version;
		} else {
			target_ver[i] = ts_info[i].version;
		}
	}

	if (fw_up_to_date) {
		pr_info("mms_ts firmware version is up to date\n");
		return ISC_NO_NEED_UPDATE_ERROR;
	}






























	for (i = 1; i < SECTION_NUM; i++) {
		if (target_ver[i - 1] != mbin_info[i].compatible_version) {
			pr_info("compatibility version mismatch(%d), 0x%02x, 0x%02x\n", i,
					target_ver[i - 1], mbin_info[i].compatible_version);
			return ISC_COMPATIVILITY_ERROR;
		}
	}



	pr_info("End mms100_compare_version_info()\n");

	return ISC_SUCCESS;
}

static eISCRet_t mms100_enter_ISC_mode(struct i2c_client *_client) {
	int ret;
	unsigned char wr_buf[2];

	pr_info("[TSP ISC] %s\n", __func__);

	wr_buf[0] = ISC_CMD_ENTER_ISC;          
	wr_buf[1] = ISC_CMD_ENTER_ISC_PARA1;    

	ret = mms100_i2c_write(_client, wr_buf, 2);

	if (ret < 0) {
		pr_info("[TSP ISC] %s,%d: i2c write fail[%d] \n", __FUNCTION__,
				__LINE__, ret);
		return ISC_I2C_ERROR;
	}

	mms100_msdelay(50);

	pr_info("End mms100_enter_ISC_mode()\n");

	return ISC_SUCCESS;
}

static eISCRet_t mms100_enter_config_update(struct i2c_client *_client) {
	int ret;
	unsigned char wr_buf[10] = { 0, };
	unsigned char rd_buf;

	pr_info("[TSP ISC] %s\n", __func__);

	wr_buf[0] = ISC_CMD_UPDATE_MODE;
	wr_buf[1] = ISC_SUBCMD_ENTER_UPDATE;

	ret = mms100_i2c_write(_client, wr_buf, 10);

	if (ret < 0) {
		pr_info("[TSP ISC] %s,%d: i2c write fail[%d] \n", __FUNCTION__,
				__LINE__, ret);
		return ISC_I2C_ERROR;
	}

	ret = mms100_i2c_read(_client, ISC_CMD_CONFIRM_STATUS, 1, &rd_buf);

	if (ret < 0) {
		pr_info("[TSP ISC] %s,%d: i2c read fail[%d] \n", __FUNCTION__, __LINE__,
				ret);
		return ISC_I2C_ERROR;
	}

	if (rd_buf != ISC_STATUS_UPDATE_MODE)
		return ISC_UPDATE_MODE_ENTER_ERROR;

	pr_info("End mms100_enter_config_update()\n");

	return ISC_SUCCESS;
}

static eISCRet_t mms100_ISC_clear_page(struct i2c_client *_client,
		unsigned char _page_addr) {
	int ret;
	unsigned char rd_buf;

	pr_info("[TSP ISC] %s\n", __func__);

	
	memset(&g_wr_buf[3], 0xFF, PAGE_DATA);

	g_wr_buf[0] = ISC_CMD_UPDATE_MODE;        
	g_wr_buf[1] = ISC_SUBCMD_DATA_WRITE;       
	g_wr_buf[2] = _page_addr;

	g_wr_buf[PAGE_HEADER + PAGE_DATA] = crc0_buf[_page_addr];
	g_wr_buf[PAGE_HEADER + PAGE_DATA + 1] = crc1_buf[_page_addr];

	ret = mms100_i2c_write(_client, g_wr_buf, PACKET_SIZE);

	if (ret < 0) {
		pr_info("[TSP ISC] %s,%d: i2c write fail[%d] \n", __FUNCTION__,
				__LINE__, ret);
		return ISC_I2C_ERROR;
	}

	ret = mms100_i2c_read(_client, ISC_CMD_CONFIRM_STATUS, 1, &rd_buf);

	if (ret < 0) {
		pr_info("[TSP ISC] %s,%d: i2c read fail[%d] \n", __FUNCTION__, __LINE__,
				ret);
		return ISC_I2C_ERROR;
	}

	if (rd_buf != ISC_STATUS_CRC_CHECK_SUCCESS)
		return ISC_UPDATE_MODE_ENTER_ERROR;

	pr_info("End mms100_ISC_clear_page()\n");

	return ISC_SUCCESS;

}

static eISCRet_t mms100_ISC_clear_validate_markers(struct i2c_client *_client) {
	eISCRet_t ret_msg;
	int i, j;
	bool is_matched_address;

	pr_info("[TSP ISC] %s\n", __func__);

	for (i = SEC_CORE; i <= SEC_CONFIG; i++) {
		if (section_update_flag[i]) {
			if (ts_info[i].end_addr <= 30 && ts_info[i].end_addr > 0) {
				ret_msg = mms100_ISC_clear_page(_client, ts_info[i].end_addr);

				if (ret_msg != ISC_SUCCESS)
					return ret_msg;
			}
		}
	}

	for (i = SEC_CORE; i <= SEC_CONFIG; i++) {
		if (section_update_flag[i]) {
			is_matched_address = false;
			for (j = SEC_CORE; j <= SEC_CONFIG; j++) {
				if (mbin_info[i].end_addr == ts_info[i].end_addr) {
					is_matched_address = true;
					break;
				}
			}

			if (!is_matched_address) {
				if (mbin_info[i].end_addr <= 30 && mbin_info[i].end_addr > 0) {
					ret_msg = mms100_ISC_clear_page(_client,
							mbin_info[i].end_addr);

					if (ret_msg != ISC_SUCCESS)
						return ret_msg;
				}
			}
		}
	}

	pr_info("End mms100_ISC_clear_validate_markers()\n");

	return ISC_SUCCESS;
}

static void mms100_calc_crc(unsigned char *crc, int page_addr,
		unsigned char* ptr_fw)	
		{
	int i, j;

	unsigned char ucData;

	unsigned short SeedValue;
	unsigned short CRC_check_buf;
	unsigned short CRC_send_buf;
	unsigned short IN_data;
	unsigned short XOR_bit_1;
	unsigned short XOR_bit_2;
	unsigned short XOR_bit_3;

	

	CRC_check_buf = 0xFFFF;
	SeedValue = (unsigned short) page_addr;

	for (i = 7; i >= 0; i--) {
		IN_data = (SeedValue >> i) & 0x01;
		XOR_bit_1 = (CRC_check_buf & 0x0001) ^ IN_data;
		XOR_bit_2 = XOR_bit_1 ^ (CRC_check_buf >> 11 & 0x01);
		XOR_bit_3 = XOR_bit_1 ^ (CRC_check_buf >> 4 & 0x01);
		CRC_send_buf = (XOR_bit_1 << 4) | (CRC_check_buf >> 12 & 0x0F);
		CRC_send_buf = (CRC_send_buf << 7) | (XOR_bit_2 << 6)
				| (CRC_check_buf >> 5 & 0x3F);
		CRC_send_buf = (CRC_send_buf << 4) | (XOR_bit_3 << 3)
				| (CRC_check_buf >> 1 & 0x0007);
		CRC_check_buf = CRC_send_buf;
	}

	for (i = 0; i < 1024; i++) {
		ucData = ptr_fw[i];

		for (j = 7; j >= 0; j--) {
			IN_data = (ucData >> j) & 0x0001;
			XOR_bit_1 = (CRC_check_buf & 0x0001) ^ IN_data;
			XOR_bit_2 = XOR_bit_1 ^ (CRC_check_buf >> 11 & 0x01);
			XOR_bit_3 = XOR_bit_1 ^ (CRC_check_buf >> 4 & 0x01);
			CRC_send_buf = (XOR_bit_1 << 4) | (CRC_check_buf >> 12 & 0x0F);
			CRC_send_buf = (CRC_send_buf << 7) | (XOR_bit_2 << 6)
					| (CRC_check_buf >> 5 & 0x3F);
			CRC_send_buf = (CRC_send_buf << 4) | (XOR_bit_3 << 3)
					| (CRC_check_buf >> 1 & 0x0007);
			CRC_check_buf = CRC_send_buf;
		}
	}

	crc[0] = (unsigned char) ((CRC_check_buf >> 8) & 0xFF);
	crc[1] = (unsigned char) ((CRC_check_buf >> 0) & 0xFF);
}

static eISCRet_t mms100_update_section_data(struct i2c_client *_client) {
#define STRING_BUF_LEN		100

	int i, j, ret;					
	unsigned char rd_buf;

	unsigned char crc[2];			

	const unsigned char *ptr_fw;



	int page_addr;

	pr_info("[TSP ISC] %s\n", __func__);

	

	for (i = 0; i < SECTION_NUM; i++) {
		pr_info("update flag (%d)\n", section_update_flag[i]);
		if (section_update_flag[i]) {
			ptr_fw = fw_mbin[i]->data;
			ptr_fw += mbin_info[i].bin_offset;







			pr_info("binary found\n");

			for (page_addr = mbin_info[i].start_addr;
					page_addr <= mbin_info[i].end_addr; page_addr++) {
				if (page_addr - mbin_info[i].start_addr > 0)
					
					ptr_fw += 1024;								

				
				

				g_wr_buf[0] = ISC_CMD_UPDATE_MODE;				
				g_wr_buf[1] = ISC_SUBCMD_DATA_WRITE;
				g_wr_buf[2] = (unsigned char) page_addr;

				for (j = 0; j < 1024; j += 4) {
					g_wr_buf[3 + j] = ptr_fw[j + 3];
					g_wr_buf[3 + j + 1] = ptr_fw[j + 2];
					g_wr_buf[3 + j + 2] = ptr_fw[j + 1];
					g_wr_buf[3 + j + 3] = ptr_fw[j + 0];
				}

				mms100_calc_crc(crc, page_addr, &g_wr_buf[3]);

				g_wr_buf[1027] = crc[0];
				g_wr_buf[1028] = crc[1];

				
				pr_info("crc val : %X%X\n", crc[0], crc[1]);
				ret = mms100_i2c_write(_client, g_wr_buf, PACKET_SIZE);	

				if (ret < 0) {
					pr_info("[TSP ISC] %s,%d: i2c write fail[%d] \n",
							__FUNCTION__, __LINE__, ret);
					return ISC_I2C_ERROR;
				}

				ret = mms100_i2c_read(_client, ISC_CMD_CONFIRM_STATUS, 1,
						&rd_buf);

				if (ret < 0) {
					pr_info("[TSP ISC] %s,%d: i2c read fail[%d] \n",
							__FUNCTION__, __LINE__, ret);
					return ISC_I2C_ERROR;
				}

				
				

				section_update_flag[i] = false;
				pr_info("section(%d) updated.\n", i);
			}
		}
	}

	pr_info("End mms100_update_section_data()\n");

	return ISC_SUCCESS;
}

static eISCRet_t mms100_open_mbinary(struct i2c_client *_client) {
	int i;
	char *fw_name;

	pr_info("[TSP ISC] %s\n", __func__);

	for (i = 0; i < SECTION_NUM; i++) {
		



		fw_name = kasprintf(GFP_KERNEL, "%s%s.mbin", mbin_path,mbin_name[i]);

		request_firmware(&fw_mbin[i], fw_name, &_client->dev);
	}

	pr_info("End mms100_open_mbinary()\n");

	return ISC_SUCCESS;
}

static eISCRet_t mms100_close_mbinary(void) {
	int i;

	pr_info("[TSP ISC] %s\n", __func__);

	for (i = 0; i < SECTION_NUM; i++)
		if (fw_mbin[i] != NULL)
			release_firmware(fw_mbin[i]);

	pr_info("End mms100_close_mbinary()\n");

	return ISC_SUCCESS;
}




eISCRet_t mms100_ISC_download_mbinary(struct i2c_client *_client, bool force_update) {
	eISCRet_t ret_msg = ISC_NONE;

	pr_info("[TSP ISC] %s\n", __func__);

	
	mms100_reset(_client);

	
	ret_msg = mms100_check_operating_mode(_client, EC_BOOT_ON_SUCCEEDED);
	if (ret_msg != ISC_SUCCESS)
		goto ISC_ERROR_HANDLE;

	
	
	ret_msg = mms100_open_mbinary(_client);
	if (ret_msg != ISC_SUCCESS)
		goto ISC_ERROR_HANDLE;

	
	if (force_update) {
		int i;
		for (i = 0; i < SECTION_NUM; i++)
			section_update_flag[i] = true;
	} else {
		ret_msg = mms100_compare_version_info(_client);
		if (ret_msg != ISC_SUCCESS)
			goto ISC_ERROR_HANDLE;
	}

	ret_msg = mms100_enter_ISC_mode(_client);
	if (ret_msg != ISC_SUCCESS)
		goto ISC_ERROR_HANDLE;

	ret_msg = mms100_enter_config_update(_client);
	if (ret_msg != ISC_SUCCESS)
		goto ISC_ERROR_HANDLE;

	
	ret_msg = mms100_ISC_clear_validate_markers(_client);
	if (ret_msg != ISC_SUCCESS)
		goto ISC_ERROR_HANDLE;

	ret_msg = mms100_update_section_data(_client);
	if (ret_msg != ISC_SUCCESS)
		goto ISC_ERROR_HANDLE;

	mms100_reset(_client);

	pr_info("FIRMWARE_UPDATE_FINISHED!!!\n");

	ret_msg = ISC_SUCCESS;

ISC_ERROR_HANDLE: 
	if (ret_msg != ISC_SUCCESS)
		pr_info("ISC_ERROR_CODE: %d\n", ret_msg);

	mms100_reset(_client);
	mms100_close_mbinary();

	return ret_msg;
}


eISCRet_t mms100_DVE022_get_dev_version(struct i2c_client *_client, tISCFWNCMCInfo_t *DVE022_info )
{
    int i, ret;
    unsigned char rd_buf[8];

    pr_info("[TSP ISC] %s\n", __func__);

    if( DVE022_info == NULL )
    {
        pr_info("[TSP ISC] %s,%d: invalid DVE022_info \n", __FUNCTION__, __LINE__);
        return ISC_WRITE_BUFFER_ERROR;
    }

    
    ret = mms100_i2c_read(_client, ISC_ADDR_VERSION, SECTION_NUM, rd_buf);

    if (ret < 0)
    {
        pr_info("[TSP ISC] %s,%d: i2c read fail[%d] \n", __FUNCTION__, __LINE__, ret);
        return ISC_I2C_ERROR;
    }
	

    for (i = 0; i < SECTION_NUM; i++)
        DVE022_info->info[i].version = rd_buf[i];
   
    DVE022_info->info[SEC_CORE].compatible_version = DVE022_info->info[SEC_BOOTLOADER].version;
    DVE022_info->info[SEC_CONFIG].compatible_version = DVE022_info->info[SEC_CORE].version;

    ret = mms100_i2c_read(_client, ISC_ADDR_SECTION_PAGE_INFO, 8, rd_buf); 

    if (ret < 0)
    {
        pr_info("[TSP ISC] %s,%d: i2c read fail[%d] \n", __FUNCTION__, __LINE__, ret);
        return ISC_I2C_ERROR;
    }
    
    for (i = 0; i < SECTION_NUM; i++)
    {
        DVE022_info->info[i].start_addr = rd_buf[i];
        DVE022_info->info[i].end_addr = rd_buf[i + SECTION_NUM + 1];
    }
    
    for (i = 0; i < SECTION_NUM; i++)
    {
        pr_info("\tTS : Section(%d) version: 0x%02X\n", i, DVE022_info->info[i].version);
        pr_info("\tTS : Section(%d) Start Address: 0x%02X\n", i, DVE022_info->info[i].start_addr);
        pr_info("\tTS : Section(%d) End Address: 0x%02X\n", i, DVE022_info->info[i].end_addr);
        pr_info("\tTS : Section(%d) Compatibility: 0x%02X\n", i, DVE022_info->info[i].compatible_version);
    }

    pr_info("[TSP ISC] %s End\n", __func__);

	return ISC_SUCCESS;
}

eISCRet_t mms100_DVE022_get_file_version( struct i2c_client *_client, tISCFWNCMCInfo_t *DVE022_info )
{
    eISCRet_t ret_msg = ISC_NONE;
    int i;

    ret_msg = mms100_open_mbinary(_client);
    if (ret_msg != ISC_SUCCESS)
    {
        pr_info("[TSP ISC] %s,%d: Open mbin error[%d] \n", __FUNCTION__, __LINE__, ret_msg);
        return ret_msg;

	}
	memset(ts_info, 0x00, sizeof(ts_info));

	mms100_seek_section_info();

	memcpy(DVE022_info, mbin_info, sizeof( tISCFWNCMCInfo_t ) );

	mms100_close_mbinary();

    for (i = 0; i < SECTION_NUM; i++)
    {
        pr_info("\tTS : Section(%d) version: 0x%02X\n", i, DVE022_info->info[i].version);
        pr_info("\tTS : Section(%d) Start Address: 0x%02X\n", i, DVE022_info->info[i].start_addr);
        pr_info("\tTS : Section(%d) End Address: 0x%02X\n", i, DVE022_info->info[i].end_addr);
        pr_info("\tTS : Section(%d) Compatibility: 0x%02X\n", i, DVE022_info->info[i].compatible_version);
    }

	return ISC_SUCCESS;
}




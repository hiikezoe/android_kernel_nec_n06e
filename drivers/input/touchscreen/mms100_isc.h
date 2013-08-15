/*
 * include/linux/mms100_isc.h - ISC(In-system programming via I2C) enalbes MMS-100 Series sensor to be programmed while installed in a complete system.
 *
 * Copyright (C) 2012 Melfas, Inc.
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

#if !defined(__MMS100_ISC_H__)
#define __MMS100_ISC_H__

#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/earlysuspend.h>
















#define MMS100_DEBUG_MSG_PRINT                      0





#define MMS100_DISABLE_BASEBAND_ISR()					disable_irq(OMAP_GPIO_IRQ(35))
#define MMS100_DISABLE_WATCHDOG_TIMER_RESET()		

#define MMS100_ROLLBACK_BASEBAND_ISR()				enable_irq(OMAP_GPIO_IRQ(35))
#define MMS100_ROLLBACK_WATCHDOG_TIMER_RESET()		























#define mms100_usdelay(x)                           do{(x) > (MAX_UDELAY_MS * 1000) ? mdelay((x)/1000) : udelay(x);}while(0) 
#define mms100_msdelay(x)                           mdelay(x) 


















typedef enum
{
    ISC_NONE = -1,
    ISC_SUCCESS = 0,
    ISC_FILE_OPEN_ERROR,
    ISC_FILE_CLOSE_ERROR,
    ISC_FILE_FORMAT_ERROR,
    ISC_WRITE_BUFFER_ERROR,
    ISC_I2C_ERROR,
    ISC_UPDATE_MODE_ENTER_ERROR,
    ISC_CRC_ERROR,
    ISC_VALIDATION_ERROR,
    ISC_COMPATIVILITY_ERROR,
    ISC_UPDATE_SECTION_ERROR,
    ISC_SLAVE_ERASE_ERROR,
    ISC_SLAVE_DOWNLOAD_ERROR,
    ISC_DOWNLOAD_WHEN_SLAVE_IS_UPDATED_ERROR,
    ISC_INITIAL_PACKET_ERROR,



    ISC_NO_NEED_UPDATE_ERROR,

    ISC_LIMIT
} eISCRet_t;




typedef enum
{
    SEC_NONE = -1,
    SEC_BOOTLOADER = 0,
    SEC_CORE,
    SEC_PRIVATE_CONFIG,
    SEC_PUBLIC_CONFIG,
    SEC_LIMIT
} eSectionType_t;

typedef struct
{
	unsigned char version;
	unsigned char compatible_version;
	unsigned char start_addr;
	unsigned char end_addr;
} tISCFWInfo_t;



typedef struct
{
	tISCFWInfo_t	info[SEC_LIMIT];
} tISCFWNCMCInfo_t;









eISCRet_t mms100_ISC_download_mbinary(struct i2c_client *_client);



eISCRet_t mms100_DVE022_get_dev_version(struct i2c_client *_client, tISCFWNCMCInfo_t *info );
eISCRet_t mms100_DVE022_get_file_version(struct i2c_client *_client, tISCFWNCMCInfo_t *info );


#endif 


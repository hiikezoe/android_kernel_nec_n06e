/*
 * include/linux/melfas_ncm.h
 *
 * - Touch Screen driver for Melfas MMS Touch Screen sensor IC
 *
 * Copyright (C) NEC CASIO Mobile Communications, Ltd.
 *
 */

#if !defined(_LINUX_MELFASE_NCM_H)
#define _LINUX_MELFASE_NCM_H

#define MELFAS_NCM_TS_NAME "melfas_ncm_ts"

struct melfas_ncm_ts_platform_data {
	int (*poweron)(struct device *);
	void (*powerdown)(struct device *);
};

#endif 

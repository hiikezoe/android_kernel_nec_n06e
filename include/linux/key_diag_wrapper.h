/*
 * key_diag_wrapper.h
 *
 * Copyright (C) 2010 NEC Corporation
 *
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#if !defined(KEY_DIAGWRAPPER_SHARED)
#define KEY_DIAGWRAPPER_SHARED




#define DIAG_KEY_WRAPPER_FD_PATH         "/dev/key_diag_wrapper"

#define DIAG_WRAPPER_IOCTL_OK   0
#define DIAG_WRAPPER_IOCTL_NG   1

#define DIAG_WRAPPER_IOCTL_A01  0x0101
#define DIAG_WRAPPER_IOCTL_A02  0x0102
#define DIAG_WRAPPER_IOCTL_A03  0x0103
#define DIAG_WRAPPER_IOCTL_A04  0x0104

#define KEY_DIAG_EMULATION_KEY     1
#define KEY_DIAG_EMULATION_HANDSET 2

#endif 

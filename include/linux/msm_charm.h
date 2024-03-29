/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/
#ifndef _ARCH_ARM_MACH_MSM_MDM_IOCTLS_H
#define _ARXH_ARM_MACH_MSM_MDM_IOCTLS_H


#define CHARM_CODE		0xCC
#define WAKE_CHARM		_IO(CHARM_CODE, 1)
#define RESET_CHARM		_IO(CHARM_CODE, 2)
#define CHECK_FOR_BOOT		_IOR(CHARM_CODE, 3, int)
#define WAIT_FOR_BOOT		_IO(CHARM_CODE, 4)
#define NORMAL_BOOT_DONE	_IOW(CHARM_CODE, 5, int)
#define RAM_DUMP_DONE		_IOW(CHARM_CODE, 6, int)
#define WAIT_FOR_RESTART	_IOR(CHARM_CODE, 7, int)
#define GET_DLOAD_STATUS	_IOR(CHARM_CODE, 8, int)
#define IMAGE_UPGRADE		_IOW(CHARM_CODE, 9, int)
#define SHUTDOWN_CHARM		_IOW(CHARM_CODE, 10, int)

#define GET_FATAL_FLAG		_IOR(CHARM_CODE, 11, unsigned int)
#define SET_FATAL_FLAG		_IOW(CHARM_CODE,12, unsigned int)
#define DUMP_COMP_REBOOT	_IO(CHARM_CODE, 13)


enum charm_boot_type {
	CHARM_NORMAL_BOOT = 0,
	CHARM_RAM_DUMPS,
};

enum image_upgrade_type {
	APQ_CONTROLLED_UPGRADE = 0,
	MDM_CONTROLLED_UPGRADE,
};
#endif

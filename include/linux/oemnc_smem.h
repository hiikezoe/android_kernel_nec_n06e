#if !defined(_LINUX_DVE035_SMEM_H)
#define _LINUX_DVE035_SMEM_H

/* ========================================================================
FILE: oemnc_smem.h
All Right Reserved, Copyright(c) NEC Casio Mobile Communications Co.,Ltd.
===========================================================================*/























struct smem_board_info_v3 {
	unsigned format;
	unsigned msm_id;
	unsigned msm_version;
	char build_id[32];
	unsigned raw_msm_id;
	unsigned raw_msm_version;
	unsigned hw_platform;
};

struct smem_board_info_v7 {
	struct smem_board_info_v3 board_info_v3;
	unsigned platform_version;
	unsigned fused_chip;
	unsigned platform_subtype;
	unsigned pmic_type;
	unsigned pmic_version;
	unsigned buffer_align;	
};

typedef struct
{
	struct smem_board_info_v7	smem_board_info;
}smem_id_vendor0;




#endif 


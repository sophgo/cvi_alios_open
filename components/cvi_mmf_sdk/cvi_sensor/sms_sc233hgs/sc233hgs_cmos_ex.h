#ifndef __SC233HGS_CMOS_EX_H_
#define __SC233HGS_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

enum sc233hgs_linear_regs_e {
	LINEAR_SHS1_0_ADDR,
	LINEAR_SHS1_1_ADDR,
	LINEAR_SHS1_2_ADDR,
	LINEAR_AGAIN_0_ADDR,
	LINEAR_AGAIN_1_ADDR,
	LINEAR_DGAIN_0_ADDR,
	LINEAR_DGAIN_1_ADDR,
	LINEAR_VMAX_0_ADDR,
	LINEAR_VMAX_1_ADDR,
	LINEAR_REGS_NUM
};

enum sc233hgs_dol2_regs_e {
	WDR2_SHS1_0_ADDR,
	WDR2_SHS1_1_ADDR,
	WDR2_SHS1_2_ADDR,

	WDR2_SHS2_0_ADDR,
	WDR2_SHS2_1_ADDR,
	WDR2_SHS2_2_ADDR,

	WDR2_AGAIN1_0_ADDR,
	WDR2_AGAIN1_1_ADDR,

	WDR2_DGAIN1_0_ADDR,
	WDR2_DGAIN1_1_ADDR,

	WDR2_AGAIN2_0_ADDR,
	WDR2_AGAIN2_1_ADDR,

	WDR2_DGAIN2_0_ADDR,
	WDR2_DGAIN2_1_ADDR,

	WDR2_VMAX_0_ADDR,
	WDR2_VMAX_1_ADDR,

	// WDR2_MAXSEXP_0_ADDR,
	// WDR2_MAXSEXP_1_ADDR,

	WDR2_REGS_NUM
};

typedef enum _SC233HGS_MODE_E {
	SC233HGS_MODE_1080P60 = 0,
	SC233HGS_MODE_LINEAR_NUM,
	SC233HGS_MODE_1080P60_WDR = SC233HGS_MODE_LINEAR_NUM,
	SC233HGS_MODE_NUM
} SC233HGS_MODE_E;

typedef struct _SC233HGS_STATE_S {
	CVI_U32		u32Sexp_MAX;	/* (2*{16’h3e23,16’h3e24} – 'd10)/2 */
} SC233HGS_STATE_S;

typedef struct _SC233HGS_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	CVI_U16 u16SexpMaxReg;		/* {16’h3e23,16’h3e24} */
	char name[64];
} SC233HGS_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastSC233HGS[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunSC233HGS_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunSC233HGS_AddrInfo[];
extern CVI_U16 g_au16SC233HGS_GainMode[];
extern CVI_U16 g_au16SC233HGS_L2SMode[];
extern const CVI_U8 sc233hgs_i2c_addr;
extern const CVI_U32 sc233hgs_addr_byte;
extern const CVI_U32 sc233hgs_data_byte;
extern void sc233hgs_init(VI_PIPE ViPipe);
extern void sc233hgs_exit(VI_PIPE ViPipe);
extern int  sc233hgs_i2c_exit(VI_PIPE ViPipe);
extern void sc233hgs_standby(VI_PIPE ViPipe);
extern void sc233hgs_restart(VI_PIPE ViPipe);
extern int  sc233hgs_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  sc233hgs_read_register(VI_PIPE ViPipe, int addr);
extern void sc233hgs_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  sc233hgs_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __SC233HGS_CMOS_EX_H_ */

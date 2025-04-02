#ifndef __MIS2032_CMOS_EX_H_
#define __MIS2032_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef ARCH_CV182X
#include "cvi_vip_cif_uapi.h"
#else
#include "cif_uapi.h"
#endif
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define syslog(level, fmt, ...)            \
do {                                                   \
	printf(fmt, ##__VA_ARGS__);                \
} while (0)


enum mis2032_linear_regs_e {
	LINEAR_EXP1_0,
	LINEAR_EXP1_1,
	LINEAR_AGAIN1_0,
	LINEAR_AGAIN1_1,
	LINEAR_DGAIN1_0,
	LINEAR_DGAIN1_1,
	LINEAR_VTS_0,
	LINEAR_VTS_1,
	LINEAR_HOLD_END,
	LINEAR_REGS_NUM
};

enum mis2032_wdr2_regs_e {
	WDR2_EXP1_0 = 0,
	WDR2_EXP1_1,
	WDR2_EXP2_0,
	WDR2_EXP2_1,
	WDR2_AGAIN1_0,
	WDR2_AGAIN1_1,
	WDR2_AGAIN2_0,
	WDR2_AGAIN2_1,
	WDR2_DGAIN1_0,
	WDR2_DGAIN1_1,
	WDR2_DGAIN2_0,
	WDR2_DGAIN2_1,
	WDR2_VTS_0,
	WDR2_VTS_1,
	WDR2_HOLD_END,
	WDR2_REGS_NUM
};

typedef enum _MIS2032_MODE_E {
	MIS2032_MODE_1080P25 = 0,
	MIS2032_MODE_1080P25_WDR,
	MIS2032_MODE_NUM
} MIS2032_MODE_E;

typedef struct _MIS2032_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_LARGE_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} MIS2032_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastMIS2032[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunMIS2032_BusInfo[];
extern CVI_U16 g_au16MIS2032_GainMode[];
extern CVI_U16 g_au16MIS2032_L2SMode[];
extern CVI_U8 mis2032_i2c_addr;
extern const CVI_U32 mis2032_addr_byte;
extern const CVI_U32 mis2032_data_byte;
extern void mis2032_init(VI_PIPE ViPipe);
extern void mis2032_exit(VI_PIPE ViPipe);
extern void mis2032_standby(VI_PIPE ViPipe);
extern void mis2032_restart(VI_PIPE ViPipe);
extern int  mis2032_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  mis2032_read_register(VI_PIPE ViPipe, int addr);
extern void mis2032_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  mis2032_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __MIS2032_CMOS_EX_H_ */

#ifndef __OS08B10_CMOS_EX_H_
#define __OS08B10_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"


enum os08b10_linear_regs_e {
	LINEAR_HOLD_START = 0,
	LINEAR_EXP_0,
	LINEAR_EXP_1,
	LINEAR_AGAIN_0,
	LINEAR_AGAIN_1,
	LINEAR_DGAIN_0,
	LINEAR_DGAIN_1,
	LINEAR_DGAIN_2,
	LINEAR_HCG,
	LINEAR_VTS_0,
	LINEAR_VTS_1,
	LINEAR_HOLD_END,
	LINEAR_LAUNCH_0,
	LINEAR_LAUNCH_1,
	LINEAR_REGS_NUM
};

enum os08b10_wdr2_regs_e {
	WDR2_HOLD_START = 0,
	WDR2_EXP1_0,
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
	WDR2_LAUNCH_0,
	WDR2_LAUNCH_1,
	WDR2_HCG,
	WDR2_REGS_NUM
};

typedef enum _OS08B10_MODE_E {
	OS08B10_MODE_3840X2160P30 = 0,
	OS08B10_MODE_LINEAR_NUM,
	OS08B10_MODE_3840X2160P30_WDR = OS08B10_MODE_LINEAR_NUM,
	OS08B10_MODE_NUM
} OS08B10_MODE_E;

typedef struct _OS08B10_STATE_S {
	CVI_U32		u32Sexp_MAX;
} OS08B10_STATE_S;

typedef struct _OS08B10_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
	CVI_U32 u32L2S_offset;
	CVI_U32 u32IspResTime;
	CVI_U32 u32VStart;
	CVI_U32 u32VEnd;
} OS08B10_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastOs08b10[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunOs08b10_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunOs08b10_AddrInfo[];
extern CVI_U16 g_au16Os08b10_GainMode[];
extern CVI_U16 g_au16Os08b10_L2SMode[VI_MAX_PIPE_NUM];
extern const CVI_U8 os08b10_i2c_addr;
extern const CVI_U32 os08b10_addr_byte;
extern const CVI_U32 os08b10_data_byte;
extern void os08b10_init(VI_PIPE ViPipe);
extern void os08b10_exit(VI_PIPE ViPipe);
extern int  os08b10_i2c_exit(VI_PIPE ViPipe);
extern void os08b10_standby(VI_PIPE ViPipe);
extern void os08b10_restart(VI_PIPE ViPipe);
extern int  os08b10_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  os08b10_read_register(VI_PIPE ViPipe, int addr);
extern void os08b10_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  os08b10_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __os08b10_CMOS_EX_H_ */

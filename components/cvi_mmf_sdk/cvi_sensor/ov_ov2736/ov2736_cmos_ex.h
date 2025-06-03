#ifndef __OV2736_CMOS_EX_H_
#define __OV2736_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif
enum ov2736_linear_regs_e {
	LINEAR_HOLD_START = 0,
	LINEAR_VTS_0,
	LINEAR_VTS_1,
	LINEAR_EXP_0,
	LINEAR_EXP_1,
	LINEAR_EXP_2,
	LINEAR_AGAIN_0,
	LINEAR_AGAIN_1,
	LINEAR_DGAIN_0,
	LINEAR_DGAIN_1,
	LINEAR_HOLD_END,
	LINEAR_LAUNCH_0,
	LINEAR_LAUNCH_1,
	LINEAR_REGS_NUM
};

enum os04c10_wdr2_regs_e {
	WDR2_HOLD_START = 0,
	WDR2_EXP1_0,
	WDR2_EXP1_1,
	WDR2_EXP1_2,
	WDR2_EXP2_0,
	WDR2_EXP2_1,
	WDR2_EXP2_2,
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
	WDR2_REGS_NUM
};

typedef enum _OV2736_MODE_E {
	OV2736_MODE_1920X1080P30 = 0,
	OV2736_MODE_LINEAR_NUM,
	OV2736_MODE_1920X1080P30_WDR = OV2736_MODE_LINEAR_NUM,
	OV2736_MODE_NUM
} OV2736_MODE_E;

typedef struct _OV2736_STATE_S {
	CVI_U32		u32Sexp_MAX;
} OV2736_STATE_S;

typedef struct _OV2736_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} OV2736_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastOv2736[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunOv2736_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunOv2736_AddrInfo[];
extern CVI_U16 g_au16Ov2736_GainMode[];
extern CVI_U16 g_au16Ov2736_L2SMode[VI_MAX_PIPE_NUM];
extern CVI_U8 ov2736_i2c_addr;
extern const CVI_U32 ov2736_addr_byte;
extern const CVI_U32 ov2736_data_byte;
extern void ov2736_init(VI_PIPE ViPipe);
extern void ov2736_exit(VI_PIPE ViPipe);
extern void ov2736_standby(VI_PIPE ViPipe);
extern void ov2736_restart(VI_PIPE ViPipe);
extern int  ov2736_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  ov2736_read_register(VI_PIPE ViPipe, int addr);
extern void ov2736_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int ov2736_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __OV2736_CMOS_EX_H_ */

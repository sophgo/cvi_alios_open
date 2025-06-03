#ifndef __OV9282_CMOS_EX_H_
#define __OV9282_CMOS_EX_H_

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

enum ov9282_linear_regs_e {
	LINEAR_HOLD_START = 0,
	LINEAR_EXP_0,
	LINEAR_EXP_1,
	LINEAR_EXP_2,
	LINEAR_AGAIN_0,
	LINEAR_VTS_0,
	LINEAR_VTS_1,
	LINEAR_HOLD_END,
	LINEAR_LAUNCH_0,
	LINEAR_LAUNCH_1,
	LINEAR_REGS_NUM
};

typedef enum _OV9282_MODE_E {
	OV9282_MODE_800P15_2L_MASTER_10BIT = 0,
	OV9282_MODE_800P15_2L_SLAVE_10BIT,
	OV9282_MODE_LINEAR_NUM,
	OV9282_MODE_NUM
} OV9282_MODE_E;

typedef struct _OV9282_STATE_S {
	CVI_U32		u32Sexp_MAX;
} OV9282_STATE_S;

typedef struct _OV9282_MODE_S {
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
} OV9282_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastOv9282[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunOv9282_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunOv9282_AddrInfo[];
extern CVI_U16 g_au16Ov9282_GainMode[];
extern CVI_U16 g_au16Ov9282_UseHwSync[VI_MAX_PIPE_NUM];
extern CVI_U8 ov9282_i2c_addr;
extern const CVI_U32 ov9282_addr_byte;
extern const CVI_U32 ov9282_data_byte;
extern void ov9282_init(VI_PIPE ViPipe);
extern void ov9282_exit(VI_PIPE ViPipe);
extern int ov9282_i2c_exit(VI_PIPE ViPipe);
extern void ov9282_standby(VI_PIPE ViPipe);
extern void ov9282_restart(VI_PIPE ViPipe);
extern void ov9282_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  ov9282_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  ov9282_read_register(VI_PIPE ViPipe, int addr);
extern int  ov9282_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __OV9282_CMOS_EX_H_ */

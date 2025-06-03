#ifndef __OS05A20_CMOS_EX_H_
#define __OS05A20_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

enum os05a20_linear_regs_e {
	LINEAR_HOLD_START = 0,
	LINEAR_EXP_0,
	LINEAR_EXP_1,
	LINEAR_AGAIN_0,
	LINEAR_AGAIN_1,
	LINEAR_VTS_0,
	LINEAR_VTS_1,
	LINEAR_HOLD_END,
	LINEAR_LAUNCH_0,
	LINEAR_LAUNCH_1,
	LINEAR_REGS_NUM
};

typedef enum _OS05A20_MODE_E {
	OS05A20_MODE_1944P15_12BIT = 0,
	OS05A20_MODE_LINEAR_NUM,
	OS05A20_MODE_NUM
} OS05A20_MODE_E;

typedef struct _OS05A20_STATE_S {
	CVI_U32		u32Sexp_MAX;
} OS05A20_STATE_S;

typedef struct _OS05A20_MODE_S {
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
} OS05A20_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastOs05a20[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunOs05a20_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunOs05a20_AddrInfo[];
extern CVI_U16 g_au16os05a20_GainMode[];
extern CVI_U16 g_au16os05a20_UseHwSync[VI_MAX_PIPE_NUM];
extern CVI_U8 os05a20_i2c_addr;
extern const CVI_U32 os05a20_addr_byte;
extern const CVI_U32 os05a20_data_byte;
extern void os05a20_init(VI_PIPE ViPipe);
extern void os05a20_exit(VI_PIPE ViPipe);
extern int os05a20_i2c_exit(VI_PIPE ViPipe);
extern void os05a20_standby(VI_PIPE ViPipe);
extern void os05a20_restart(VI_PIPE ViPipe);
extern void os05a20_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  os05a20_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  os05a20_read_register(VI_PIPE ViPipe, int addr);
extern int  os05a20_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __OS05A20_CMOS_EX_H_ */

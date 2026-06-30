#ifndef __OS02G10_CMOS_EX_H_
#define __OS02G10_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "cvi_comm_cif.h"
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define syslog(level, fmt, ...)            \
do {                                                   \
	printf(fmt, ##__VA_ARGS__);                \
} while (0)


enum os02g10_linear_regs_e {
	LINEAR_PAGE_START = 0,
	LINEAR_EXP_0,
	LINEAR_EXP_1,
	LINEAR_AGAIN,
	LINEAR_DGAIN_0,
	LINEAR_DGAIN_1,
	LINEAR_VTS_0,
	LINEAR_VTS_1,
	LINEAR_FLIP_MIRROR,
	LINEAR_LAUNCH,
	LINEAR_PAGE_END,
	LINEAR_REGS_NUM
};

typedef enum _OS02G10_MODE_E {
	OS02G10_MODE_1920X1080P30 = 0,
	OS02G10_MODE_LINEAR_NUM,
	OS02G10_MODE_NUM
} OS02G10_MODE_E;

typedef struct _OS02G10_STATE_S {
	CVI_U32		u32Sexp_MAX;
} OS02G10_STATE_S;

typedef struct _OS02G10_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	CVI_U16 u16L2sOffset;
	CVI_U16 u16TopBoundary;
	CVI_U16 u16BotBoundary;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	CVI_U32 u32L2S_offset;
	CVI_U32 u32IspResTime;
	CVI_U32 u32HdrMargin;
	char name[64];
} OS02G10_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastOs02g10[VI_MAX_PIPE_NUM];
extern ISP_SNS_MIRRORFLIP_TYPE_E g_aeOs02g10_MirrorFlip[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunOs02g10_BusInfo[];
extern CVI_U8 os02g10_i2c_addr;
extern const CVI_U32 os02g10_addr_byte;
extern const CVI_U32 os02g10_data_byte;
extern void os02g10_init(VI_PIPE ViPipe);
extern void os02g10_exit(VI_PIPE ViPipe);
extern void os02g10_standby(VI_PIPE ViPipe);
extern void os02g10_restart(VI_PIPE ViPipe);
extern int  os02g10_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  os02g10_read_register(VI_PIPE ViPipe, int addr);
extern void os02g10_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int os02g10_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __OS02G10_CMOS_EX_H_ */

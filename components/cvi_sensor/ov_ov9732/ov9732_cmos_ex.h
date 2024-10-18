#ifndef __OV9732_CMOS_EX_H_
#define __OV9732_CMOS_EX_H_

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

enum ov9732_linear_regs_e {
	LINEAR_EXP_0,
	LINEAR_EXP_1,
        LINEAR_EXP_2,
	LINEAR_AGAIN,
        LINEAR_DGAIN_0,
        LINEAR_DGAIN_1,
        LINEAR_DGAIN_2,
        LINEAR_DGAIN_3,
        LINEAR_DGAIN_4,
        LINEAR_DGAIN_5,
	LINEAR_VTS_0,
	LINEAR_VTS_1,
	LINEAR_REGS_NUM
};

typedef enum _OV9732_MODE_E {
	OV9732_MODE_1280X720P30 = 0,
	OV9732_MODE_LINEAR_NUM,
	OV9732_MODE_NUM
} OV9732_MODE_E;

typedef struct _OV9732_STATE_S {
	CVI_U32		u32Sexp_MAX;
} OV9732_STATE_S;

typedef struct _OV9732_MODE_S {
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
} OV9732_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastOv9732[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunOv9732_BusInfo[];
extern CVI_U16 g_au16Ov9732_GainMode[];
extern CVI_U16 g_au16Ov9732_UseHwSync[VI_MAX_PIPE_NUM];
extern CVI_U8 ov9732_i2c_addr;
extern const CVI_U32 ov9732_addr_byte;
extern const CVI_U32 ov9732_data_byte;
extern void ov9732_init(VI_PIPE ViPipe);
extern void ov9732_exit(VI_PIPE ViPipe);
extern void ov9732_standby(VI_PIPE ViPipe);
extern void ov9732_restart(VI_PIPE ViPipe);
extern int  ov9732_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  ov9732_read_register(VI_PIPE ViPipe, int addr);
extern void ov9732_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int ov9732_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __OV9732_CMOS_EX_H_ */


#ifndef __MIS5001_CMOS_EX_H_
#define __MIS5001_CMOS_EX_H_

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


enum mis5001_linear_regs_e {
	LINEAR_EXP1_0,
	LINEAR_EXP1_1,
	LINEAR_AGAIN1,
	LINEAR_DGAIN1_0,
	LINEAR_DGAIN1_1,
	LINEAR_VTS_0,
	LINEAR_VTS_1,
	LINEAR_REGS_NUM
};

typedef enum _MIS5001_MODE_E {
	MIS5001_MODE_1080P15 = 0,
	MIS5001_MODE_NUM
} MIS5001_MODE_E;

typedef struct _MIS5001_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_LARGE_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} MIS5001_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastMIS5001[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunMIS5001_BusInfo[];
extern CVI_U16 g_au16MIS5001_GainMode[];
extern CVI_U8 mis5001_i2c_addr;
extern const CVI_U32 mis5001_addr_byte;
extern const CVI_U32 mis5001_data_byte;
extern void mis5001_init(VI_PIPE ViPipe);
extern void mis5001_exit(VI_PIPE ViPipe);
extern void mis5001_standby(VI_PIPE ViPipe);
extern void mis5001_restart(VI_PIPE ViPipe);
extern int  mis5001_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  mis5001_read_register(VI_PIPE ViPipe, int addr);
extern void mis5001_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  mis5001_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __MIS5001_CMOS_EX_H_ */

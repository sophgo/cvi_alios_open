#ifndef __SP2509_CMOS_EX_H_
#define __SP2509_CMOS_EX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cif_uapi.h"
#include "cvi_sns_ctrl.h"
#include "cvi_type.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define syslog(level, fmt, ...)     \
	do {                            \
		printf(fmt, ##__VA_ARGS__); \
	} while (0)

enum sp2509_linear_regs_e {
	LINEAR_PAGE_1,  // sp2509 page 1 is sensor register
	LINEAR_EXP_H,
	LINEAR_EXP_L,
	LINEAR_AGAIN_H,
	LINEAR_AGAIN_L,
	LINEAR_TRIGGER,
	LINEAR_DGAIN_H,
	LINEAR_DGAIN_L,
	LINEAR_VTS_H,
	LINEAR_VTS_L,
	LINEAR_MIRROR_FLIP,

	LINEAR_REGS_NUM
};

typedef enum _SP2509_MODE_E {
	SP2509_MODE_1600X1200P30 = 0,
	SP2509_MODE_800X600P30,
	SP2509_MODE_LINEAR_NUM,
	SP2509_MODE_NUM
} SP2509_MODE_E;

typedef struct _SP2509_STATE_S {
	CVI_U32 u32Sexp_MAX;
} SP2509_STATE_S;

typedef struct _SP2509_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} SP2509_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastSp2509[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunSp2509_BusInfo[];
extern ISP_SNS_MIRRORFLIP_TYPE_E g_aeSp2509_MirrorFip[VI_MAX_PIPE_NUM];
extern CVI_U8 sp2509_i2c_addr;
extern const CVI_U32 sp2509_addr_byte;
extern const CVI_U32 sp2509_data_byte;
extern void sp2509_init(VI_PIPE ViPipe);
extern void sp2509_exit(VI_PIPE ViPipe);
extern void sp2509_standby(VI_PIPE ViPipe);
extern void sp2509_restart(VI_PIPE ViPipe);
extern int sp2509_write_register(VI_PIPE ViPipe, int addr, int data);
extern int sp2509_read_register(VI_PIPE ViPipe, int addr);
extern int sp2509_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* __SP2509_CMOS_EX_H_ */

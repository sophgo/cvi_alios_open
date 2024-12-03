#ifndef __GC3003_CMOS_EX_H_
#define __GC3003_CMOS_EX_H_

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

enum gc3003_linear_regs_e {
	LINEAR_EXP_H = 0,
	LINEAR_EXP_L,
	LINEAR_PGA_GAIN_H,
	LINEAR_PGA_GAIN_L,
	LINEAR_COL_AGAIN_H,
	LINEAR_COL_AGAIN_L,
	LINEAR_AGAIN_0,
	LINEAR_AGAIN_1,
	LINEAR_AGAIN_2,
	LINEAR_PREGAIN_H,
	LINEAR_PREGAIN_L,
	LINEAR_VTS_H,
	LINEAR_VTS_L,
	LINEAR_REGS_NUM
};


typedef enum _GC3003_MODE_E {
	GC3003_MODE_2304X1296P30 = 0,
	GC3003_MODE_NUM
} GC3003_SLAVE_MODE_E;

typedef struct _GC3003_STATE_S {
	CVI_U32		u32Sexp_MAX;
} GC3003_STATE_S;

typedef struct _GC3003_MODE_S {
	ISP_WDR_SIZE_S stImg;
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp;
	SNS_ATTR_LARGE_S stAgain;
	SNS_ATTR_LARGE_S stDgain;
	char name[64];
} GC3003_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastGc3003[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunGc3003_BusInfo[];
extern ISP_SNS_MIRRORFLIP_TYPE_E g_aeGc3003_MirrorFip[VI_MAX_PIPE_NUM];
extern CVI_U8 gc3003_i2c_addr;
extern const CVI_U32 gc3003_addr_byte;
extern const CVI_U32 gc3003_data_byte;
extern void gc3003_init(VI_PIPE ViPipe);
extern void gc3003_exit(VI_PIPE ViPipe);
extern void gc3003_standby(VI_PIPE ViPipe);
extern void gc3003_restart(VI_PIPE ViPipe);
extern int  gc3003_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  gc3003_read_register(VI_PIPE ViPipe, int addr);
extern void gc3003_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  gc3003_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __GC3003_CMOS_EX_H_ */


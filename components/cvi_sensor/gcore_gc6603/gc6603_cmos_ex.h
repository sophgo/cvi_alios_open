#ifndef __GC6603_CMOS_EX_H_
#define __GC6603_CMOS_EX_H_

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

enum gc6603_linear_regs_e {
	LINEAR_EXP_H,
	LINEAR_EXP_L,
	LINEAR_AGAIN_PGA1,
	LINEAR_AGAIN_PGA2,
	LINEAR_AGAIN_PGA3,
	LINEAR_AGAIN_MAG1,
	LINEAR_AGAIN_MAG2,
	LINEAR_AGAIN_MAG3,
	LINEAR_AGAIN_MAG4,
	LINEAR_AGAIN_MAG5,
	LINEAR_AGAIN_MAG6,
	LINEAR_DGAIN_H,
	LINEAR_DGAIN_L,
	LINEAR_VTS_H,
	LINEAR_VTS_L,
	LINEAR_FLIP_MIRROR,
	LINEAR_REGS_NUM
};

typedef enum _GC6603_MODE_E {
	GC6603_MODE_2L_2048X2048P15 = 0,
	GC6603_MODE_2L_2592x1944P15,
	GC6603_MODE_LINEAR_NUM,
	GC6603_MODE_NUM
} GC6603_MODE_E;

typedef struct _GC6603_STATE_S {
	CVI_U32		u32Sexp_MAX;
} GC6603_STATE_S;

typedef struct _GC6603_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} GC6603_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastGc6603[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunGc6603_BusInfo[];
extern ISP_SNS_MIRRORFLIP_TYPE_E g_aeGc6603_MirrorFip[VI_MAX_PIPE_NUM];
extern CVI_U8 gc6603_i2c_addr;
extern const CVI_U32 gc6603_addr_byte;
extern const CVI_U32 gc6603_data_byte;
extern void gc6603_init(VI_PIPE ViPipe);
extern void gc6603_exit(VI_PIPE ViPipe);
extern void gc6603_standby(VI_PIPE ViPipe);
extern void gc6603_restart(VI_PIPE ViPipe);
extern int  gc6603_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  gc6603_read_register(VI_PIPE ViPipe, int addr);
extern int  gc6603_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __GC6603_CMOS_EX_H_ */


#ifndef __GC2083_1L_CMOS_EX_H_
#define __GC2083_1L_CMOS_EX_H_

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

enum gc2083_1l_linear_regs_e {
	LINEAR_EXP_H = 0,   //03
	LINEAR_EXP_L,       //04
	LINEAR_AGAIN_L,     //b3
	LINEAR_AGAIN_HOLD,  //0x031d
	LINEAR_AGAIN_H,
	LINEAR_AGAIN_REL,   //0x031d
	LINEAR_COL_AGAIN_H, //b8
	LINEAR_COL_AGAIN_L, //b9
	LINEAR_AGAIN_MAG1,  //0x155
	LINEAR_AGAIN_MAG2,  //0xc2
	LINEAR_AGAIN_MAG3,  //0xcf
	LINEAR_AGAIN_MAG4,  //0xd9
	LINEAR_AGAIN_MAG5,
	LINEAR_AGAIN_MAG6,
	LINEAR_AGAIN_MAG7,
	LINEAR_AGAIN_MAG8,
	LINEAR_AGAIN_MAG9,
	LINEAR_DGAIN_H,     //b1
	LINEAR_DGAIN_L,     //b2
	LINEAR_VTS_H,       //0x41  (frame length)
	LINEAR_VTS_L,       //0x42
	LINEAR_REGS_NUM
};
typedef enum _GC2083_1L_MODE_E {
	GC2083_1L_MODE_1920X1080P30 = 0,
	GC2083_1L_MODE_LINEAR_NUM,
	GC2083_1L_MODE_1920X1080P30_WDR = GC2083_1L_MODE_LINEAR_NUM,
	GC2083_1L_MODE_NUM
} GC2083_1L_MODE_E;

typedef struct _GC2083_1L_STATE_S {
	CVI_U32		u32Sexp_MAX;
} GC2083_1L_STATE_S;

typedef struct _GC2083_1L_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} GC2083_1L_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastGc2083_1L[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunGc2083_1L_BusInfo[];
extern ISP_SNS_MIRRORFLIP_TYPE_E g_aeGc2083_1L_MirrorFip[VI_MAX_PIPE_NUM];
extern CVI_U8 gc2083_1l_i2c_addr;
extern const CVI_U32 gc2083_1l_addr_byte;
extern const CVI_U32 gc2083_1l_data_byte;
extern void gc2083_1l_init(VI_PIPE ViPipe);
extern void gc2083_1l_exit(VI_PIPE ViPipe);
extern void gc2083_1l_standby(VI_PIPE ViPipe);
extern void gc2083_1l_restart(VI_PIPE ViPipe);
extern int  gc2083_1l_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  gc2083_1l_read_register(VI_PIPE ViPipe, int addr);
extern void gc2083_1l_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  gc2083_1l_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __GC2083_1L_CMOS_EX_H_ */


#ifndef __GC0403_CMOS_EX_H_
#define __GC0403_CMOS_EX_H_

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

enum gc0403_linear_regs_e {
	LINEAR_EXP_RST = 0,
	LINEAR_EXP_H,
	LINEAR_EXP_L,
	LINEAR_AGAIN,
	LINEAR_DGAIN_H,
	LINEAR_DGAIN_L,
	LINEAR_VTS_H,
	LINEAR_VTS_L,
	LINEAR_REGS_NUM
};


typedef enum _GC0403_MODE_E {
	GC0403_MODE_768X576P60 = 0,
	GC0403_MODE_NUM
} GC0403_SLAVE_MODE_E;

typedef struct _GC0403_STATE_S {
	CVI_U32		u32Sexp_MAX;
} GC0403_STATE_S;

typedef struct _GC0403_MODE_S {
	ISP_WDR_SIZE_S stImg;
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp;
	SNS_ATTR_LARGE_S stAgain;
	SNS_ATTR_LARGE_S stDgain;
	char name[64];
} GC0403_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastGc0403[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunGc0403_BusInfo[];
extern ISP_SNS_MIRRORFLIP_TYPE_E g_aeGc0403_MirrorFip[VI_MAX_PIPE_NUM];
extern CVI_U8 gc0403_i2c_addr_write;
extern const CVI_U32 gc0403_addr_byte;
extern const CVI_U32 gc0403_data_byte;
extern void gc0403_init(VI_PIPE ViPipe);
extern void gc0403_exit(VI_PIPE ViPipe);
extern void gc0403_standby(VI_PIPE ViPipe);
extern void gc0403_restart(VI_PIPE ViPipe);
extern int  gc0403_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  gc0403_read_register(VI_PIPE ViPipe, int addr);
extern void gc0403_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  gc0403_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __GC0403_CMOS_EX_H_ */


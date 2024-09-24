#ifndef __GC2053_CMOS_EX_H_
#define __GC2053_CMOS_EX_H_

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

enum gc2053_linear_regs_e {
	LINEAR_EXP_H = 0,//03
	LINEAR_EXP_L,	//04
	LINEAR_AGAIN_H, //b4
	LINEAR_AGAIN_L, //b3
	LINEAR_COL_AGAIN_H, //b8
	LINEAR_COL_AGAIN_L, //b9
	LINEAR_DGAIN_H, //b1
	LINEAR_DGAIN_L, //b2
	LINEAR_VTS_H, //0x41  (frame length)
	LINEAR_VTS_L,//0x42
	LINEAR_REGS_NUM
};


typedef enum _GC2053_MODE_E {
	GC2053_MODE_1920X1080P30 = 0,
	GC2053_MODE_NUM
} GC2053_SLAVE_MODE_E;

typedef struct _GC2053_STATE_S {
	CVI_U32		u32Sexp_MAX;
} GC2053_STATE_S;

typedef struct _GC2053_MODE_S {
	ISP_WDR_SIZE_S stImg;
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp;
	SNS_ATTR_LARGE_S stAgain;
	SNS_ATTR_LARGE_S stDgain;
	char name[64];
} GC2053_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastGc2053[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunGc2053_BusInfo[];
extern ISP_SNS_MIRRORFLIP_TYPE_E g_aeGc2053_MirrorFip[VI_MAX_PIPE_NUM];
extern CVI_U8 gc2053_i2c_addr;
extern const CVI_U32 gc2053_addr_byte;
extern const CVI_U32 gc2053_data_byte;
extern void gc2053_init(VI_PIPE ViPipe);
extern void gc2053_exit(VI_PIPE ViPipe);
extern void gc2053_standby(VI_PIPE ViPipe);
extern void gc2053_restart(VI_PIPE ViPipe);
extern int  gc2053_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  gc2053_read_register(VI_PIPE ViPipe, int addr);
extern void gc2053_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  gc2053_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __GC2053_CMOS_EX_H_ */


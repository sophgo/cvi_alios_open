#ifndef __GC4663_CMOS_EX_H_
#define __GC4663_CMOS_EX_H_

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

enum gc4663_linear_regs_e {
	LINEAR_EXP_H,       // 0x0202 Exposure_long[13:8] 6bit
	LINEAR_EXP_L,       // 0x0203 Exposure_long[7:0]  8bit
	LINEAR_AGAIN_L,     // 0x02b3
	LINEAR_AGAIN_H,     // 0x02b4
	LINEAR_COL_AGAIN_H, // 0x02b8
	LINEAR_COL_AGAIN_L, // 0x02b9
	LINEAR_AGAIN_MAG1,  // 0x0515
	LINEAR_AGAIN_MAG2,  // 0x0519
	LINEAR_AGAIN_MAG3,  // 0x02d9
	LINEAR_DGAIN_H,     // 0x020e
	LINEAR_DGAIN_L,     // 0x020f
	LINEAR_VTS_H,       // 0x0340
	LINEAR_VTS_L,       // 0x0341
	LINEAR_FLIP_MIRROR, // 0x0101
	LINEAR_REGS_NUM
};

enum gc4663_wdr_regs_e {
	WDR_EXP_LONG_H,     // 0x0202
	WDR_EXP_LONG_L,     // 0x0203
	WDR_EXP_SHORT_H,    // 0x0200
	WDR_EXP_SHORT_L,    // 0x0201
	WDR_AGAIN_L,        // 0x02b3
	WDR_AGAIN_H,        // 0x02b4
	WDR_COL_AGAIN_H,    // 0x02b8
	WDR_COL_AGAIN_L,    // 0x02b9
	WDR_AGAIN_MAG1,     // 0x0515
	WDR_AGAIN_MAG2,     // 0x0519
	WDR_AGAIN_MAG3,     // 0x02d9
	WDR_DGAIN_H,        // 0x020e
	WDR_DGAIN_L,        // 0x020f
	WDR_VTS_H,          // 0x0340
	WDR_VTS_L,          // 0x0341
	WDR_FLIP_MIRROR,    // 0x0101
	WDR_REGS_NUM
};

typedef enum _GC4663_MODE_E {
	GC4663_MODE_2560X1440P30 = 0,
	GC4663_MODE_LINEAR_NUM,
	GC4663_MODE_2560X1440P30_WDR = GC4663_MODE_LINEAR_NUM,
	GC4663_MODE_NUM
} GC4663_MODE_E;

typedef struct _GC4663_STATE_S {
	CVI_U32		u32Sexp_MAX;
} GC4663_STATE_S;

typedef struct _GC4663_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} GC4663_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastGc4663[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunGc4663_BusInfo[];
extern ISP_SNS_MIRRORFLIP_TYPE_E g_aeGc4663_MirrorFip[VI_MAX_PIPE_NUM];
extern CVI_U8 gc4663_i2c_addr;
extern const CVI_U32 gc4663_addr_byte;
extern const CVI_U32 gc4663_data_byte;
extern void gc4663_init(VI_PIPE ViPipe);
extern void gc4663_exit(VI_PIPE ViPipe);
extern void gc4663_standby(VI_PIPE ViPipe);
extern void gc4663_restart(VI_PIPE ViPipe);
extern int  gc4663_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  gc4663_read_register(VI_PIPE ViPipe, int addr);
extern int  gc4663_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __GC4663_CMOS_EX_H_ */

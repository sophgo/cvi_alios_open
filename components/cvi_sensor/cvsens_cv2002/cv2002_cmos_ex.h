#ifndef __CV2002_CMOS_EX_H_
#define __CV2002_CMOS_EX_H_

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

enum cv2002_linear_regs_e {
	LINEAR_EXP_0,       // 0x304a bit[19:16]
	LINEAR_EXP_1,       // 0x30a9
	LINEAR_EXP_2,       // 0x3048
	LINEAR_GAINENABLE,  // 0x3141 bit[0]
	LINEAR_AGAIN,       // 0x3154 bit[7:0]
	LINEAR_DGAIN_H,     // 0x314d bit[15:8]
	LINEAR_DGAIN_L,     // 0x314c bit[7:0]
	LINEAR_VTS_0,       // 0x3022 bit[19:16]
	LINEAR_VTS_1,       // 0x3021
	LINEAR_VTS_2,       // 0x3020
	LINEAR_FLIP_MIRROR, // 0x3028
	LINEAR_REGS_NUM
};

enum cv2002_wdr2_regs_e {
	WDR2_EXP1_0,		// 0x304a bit[19:16]
	WDR2_EXP1_1,		// 0x30a9
	WDR2_EXP1_2,		// 0x3048
	WDR2_EXP2_0,		// 0x304e
	WDR2_EXP2_1,		// 0x304d
	WDR2_EXP2_2,		// 0x304c
	WDR2_GAINENABLE, 	// 0x3141 bit[0]
	WDR2_AGAIN1,		// 0x3154
	WDR2_AGAIN2,		// 0x3155
	WDR2_DGAIN1_H,		// 0x314d bit[15:8]
	WDR2_DGAIN1_L,		// 0x314c bit[7:0]
	WDR2_DGAIN2_H,		// 0x314f bit[15:8]
	WDR2_DGAIN2_L,		// 0x314e bit[7:0]
	WDR2_VTS_0,			// 0x3022 bit[19:16]
	WDR2_VTS_1,			// 0x3021
	WDR2_VTS_2,			// 0x3020
	WDR2_FLIP_MIRROR,	// 0x3028
	WDR2_REGS_NUM
};

typedef enum _CV2002_MODE_E {
	CV2002_MODE_1920X1080P30 = 0,
	CV2002_MODE_1280X720P30,
	CV2002_MODE_LINEAR_NUM,
	CV2002_MODE_1920X1080P30_WDR = CV2002_MODE_LINEAR_NUM,
	CV2002_MODE_1280X720P30_WDR,
	CV2002_MODE_NUM
} CV2002_MODE_E;

typedef struct _CV2002_STATE_S {
	CVI_U32		u32Sexp_MAX;
} CV2002_STATE_S;

typedef struct _CV2002_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	CVI_U32 u32IspResTime;
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} CV2002_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastCV2002[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunCV2002_BusInfo[];
extern ISP_SNS_MIRRORFLIP_TYPE_E g_aeCV2002_MirrorFip[VI_MAX_PIPE_NUM];
extern CVI_U8 cv2002_i2c_addr;
extern const CVI_U32 cv2002_addr_byte;
extern const CVI_U32 cv2002_data_byte;
extern void cv2002_init(VI_PIPE ViPipe);
extern void cv2002_exit(VI_PIPE ViPipe);
extern void cv2002_standby(VI_PIPE ViPipe);
extern void cv2002_restart(VI_PIPE ViPipe);
extern int  cv2002_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  cv2002_read_register(VI_PIPE ViPipe, int addr);
extern int  cv2002_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __CV2002_CMOS_EX_H_ */


#ifndef __OG01A1B_CMOS_EX_H_
#define __OG01A1B_CMOS_EX_H_

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

enum og01a1b_linear_regs_e {
	LINEAR_EXP_H = 0,
	LINEAR_EXP_L,
	LINEAR_AGAIN_H,
	LINEAR_AGAIN_L,
	LINEAR_DGAIN_H,
	LINEAR_DGAIN_M,
	LINEAR_DGAIN_L,
	LINEAR_VTS_H,
	LINEAR_VTS_L,
	LINEAR_REGS_NUM
};


typedef enum _OG01A1B_MODE_E {
	OG01A1B_MODE_1280X1024P30 = 0,
	OG01A1B_MODE_NUM
} OG01A1B_MODE_E;

typedef struct _OG01A1B_STATE_S {
	CVI_U32		u32Sexp_MAX;
} OG01A1B_STATE_S;

typedef struct _OG01A1B_MODE_S {
	ISP_WDR_SIZE_S stImg;
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp;
	SNS_ATTR_LARGE_S stAgain;
	SNS_ATTR_LARGE_S stDgain;
	char name[64];
} OG01A1B_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastOg01a1b[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunOg01a1b_BusInfo[];
extern ISP_SNS_MIRRORFLIP_TYPE_E g_aeOg01a1b_MirrorFip[VI_MAX_PIPE_NUM];
extern CVI_U8 og01a1b_i2c_addr;
extern const CVI_U32 og01a1b_addr_byte;
extern const CVI_U32 og01a1b_data_byte;
extern void og01a1b_init(VI_PIPE ViPipe);
extern void og01a1b_exit(VI_PIPE ViPipe);
extern void og01a1b_standby(VI_PIPE ViPipe);
extern void og01a1b_restart(VI_PIPE ViPipe);
extern int  og01a1b_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  og01a1b_read_register(VI_PIPE ViPipe, int addr);
extern void og01a1b_mirror_flip(VI_PIPE ViPipe,
				ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  og01a1b_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __OG01A1B_CMOS_EX_H_ */


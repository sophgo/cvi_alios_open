#ifndef __BF314A_CMOS_EX_H_
#define __BF314A_CMOS_EX_H_

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


enum bf314a_linear_regs_e {
	LINEAR_SHS1_0_ADDR,
	LINEAR_SHS1_1_ADDR,
	LINEAR_AGAIN_ADDR,
	LINEAR_DGAIN_0_ADDR,
	LINEAR_DL_0_ADDR,	//dummy line high 8bit
	LINEAR_DL_1_ADDR,	//dummy line low 8bit
	LINEAR_FLIP_MIRROR_ADDR,
	LINEAR_REGS_NUM
};

typedef enum _BF314A_MODE_E {
	BF314A_MODE_1280X720P30 = 0,
	BF314A_MODE_LINEAR_NUM,
	BF314A_MODE_NUM
} BF314A_MODE_E;

typedef struct _BF314A_STATE_S {
	CVI_U32		u32Sexp_MAX;
} BF314A_STATE_S;

typedef struct _BF314A_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} BF314A_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastBf314a[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunBf314a_BusInfo[];
extern CVI_U8 bf314a_i2c_addr;
extern const CVI_U32 bf314a_addr_byte;
extern const CVI_U32 bf314a_data_byte;
extern void bf314a_init(VI_PIPE ViPipe);
extern void bf314a_exit(VI_PIPE ViPipe);
extern void bf314a_standby(VI_PIPE ViPipe);
extern void bf314a_restart(VI_PIPE ViPipe);
extern int  bf314a_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  bf314a_read_register(VI_PIPE ViPipe, int addr);
extern int  bf314a_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __BF314A_CMOS_EX_H_ */


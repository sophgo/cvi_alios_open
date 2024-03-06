#ifndef __SC230AI_CMOS_EX_H_
#define __SC230AI_CMOS_EX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cif_uapi.h"
#include "cvi_sns_ctrl.h"
#include "cvi_type.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define syslog(level, fmt, ...) printf(fmt, ##__VA_ARGS__)

#define SC230AI_I2C_ADDR_1 0x30
#define SC230AI_I2C_ADDR_2 0x32

enum sc230ai_linear_regs_e {
	LINEAR_SHS1_0_ADDR,
	LINEAR_SHS1_1_ADDR,
	LINEAR_SHS1_2_ADDR,
	LINEAR_AGAIN_ADDR,
	LINEAR_DGAIN_0_ADDR,
	LINEAR_DGAIN_1_ADDR,
	LINEAR_VMAX_0_ADDR,
	LINEAR_VMAX_1_ADDR,

	LINEAR_REGS_NUM  // detect the
};

typedef enum _SC230AI_MODE_E {
	SC230AI_MODE_1920X1080P30 = 0,
	SC230AI_MODE_LINEAR_NUM,
	SC230AI_MODE_NUM
} SC230AI_MODE_E;

typedef struct _SC230AI_STATE_S {
	CVI_U32 u32Sexp_MAX;
} SC230AI_STATE_S;

typedef struct _SC230AI_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} SC230AI_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastSc230ai[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunSc230ai_BusInfo[];
extern ISP_SNS_MIRRORFLIP_TYPE_E g_aeSc230ai_MirrorFip[VI_MAX_PIPE_NUM];
extern CVI_U8 sc230ai_i2c_addr;
extern CVI_U8 sc230ai_i2c_addr_list[];
extern CVI_S8 sc230ai_i2c_dev_list[];
extern CVI_U8 sc230ai_cur_idx;
extern const CVI_U32 sc230ai_addr_byte;
extern const CVI_U32 sc230ai_data_byte;
extern void sc230ai_init(VI_PIPE ViPipe);
extern void sc230ai_exit(VI_PIPE ViPipe);
extern void sc230ai_standby(VI_PIPE ViPipe);
extern void sc230ai_restart(VI_PIPE ViPipe);
extern void sc230ai_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int sc230ai_write_register(VI_PIPE ViPipe, int addr, int data);
extern int sc230ai_read_register(VI_PIPE ViPipe, int addr);
extern int sc230ai_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* __SC230AI_CMOS_EX_H_ */

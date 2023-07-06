#ifndef __SC031IOT_RAW_CMOS_EX_H_
#define __SC031IOT_RAW_CMOS_EX_H_

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

#define syslog(level, fmt, ...)	printf(fmt, ##__VA_ARGS__)

enum sc031iot_raw_linear_regs_e {

	LINEAR_PAGE_EXP_ADDR,
	LINEAR_EXP_0_ADDR,
	LINEAR_EXP_1_ADDR,
	LINEAR_PAGE_AGAIN_ADDR,
	LINEAR_AGAIN_0_ADDR,
	LINEAR_AGAIN_1_ADDR,
	LINEAR_PAGE_DGAIN_ADDR,
	LINEAR_DGAIN_0_ADDR,
	LINEAR_PAGE_VMAX_ADDR,
	LINEAR_VMAX_0_ADDR,
	LINEAR_VMAX_1_ADDR,

	LINEAR_REGS_NUM
};

typedef enum _SC031IOT_RAW_MODE_E {
	SC031IOT_RAW_MODE_640X480P30 = 0,
	SC031IOT_RAW_MODE_LINEAR_NUM,
	SC031IOT_RAW_MODE_NUM
} SC031IOT_RAW_MODE_E;

typedef struct _SC031IOT_RAW_STATE_S {
	CVI_U32		u32Sexp_MAX;
} SC031IOT_RAW_STATE_S;

typedef struct _SC031IOT_RAW_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} SC031IOT_RAW_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastSc031iot_raw[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunSc031iot_raw_BusInfo[];
extern ISP_SNS_MIRRORFLIP_TYPE_E g_aeSc031iot_raw_MirrorFip[VI_MAX_PIPE_NUM];
extern CVI_U8 sc031iot_raw_i2c_addr;
extern const CVI_U32 sc031iot_raw_addr_byte;
extern const CVI_U32 sc031iot_raw_data_byte;
extern void sc031iot_raw_init(VI_PIPE ViPipe);
extern void sc031iot_raw_exit(VI_PIPE ViPipe);
extern void sc031iot_raw_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  sc031iot_raw_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  sc031iot_raw_read_register(VI_PIPE ViPipe, int addr);
extern int  sc031iot_raw_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __SC031IOT_RAW_CMOS_EX_H_ */
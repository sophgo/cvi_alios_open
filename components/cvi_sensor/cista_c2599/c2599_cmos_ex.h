#ifndef __C2599_CMOS_EX_H_
#define __C2599_CMOS_EX_H_

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

enum c2599_linear_regs_e {
	LINEAR_SHS1_0_ADDR,
	LINEAR_SHS1_1_ADDR,
	LINEAR_AGAIN_MAGIC_1,
	LINEAR_AGAIN_MAGIC_2,
	LINEAR_AGAIN_MAGIC_3,
	C2599_WRITE_GROUP_ADDR,
	LINEAR_DGAIN_ADDR,
	LINEAR_D_FINEGAIN_ADDR,
	LINEAR_VMAX_0_ADDR,
	LINEAR_VMAX_1_ADDR,
	LINEAR_REGS_NUM
};

typedef enum _C2599_MODE_E {
	C2599_MODE_1200P30 = 0,
	C2599_MODE_NUM
} C2599_MODE_E;

typedef struct _C2599_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_LARGE_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} C2599_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastC2599[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunC2599_BusInfo[];
extern CVI_U16 g_au16C2599_GainMode[];
extern CVI_U16 g_au16C2599_L2SMode[];
extern CVI_U8 c2599_i2c_addr;
extern const CVI_U32 c2599_addr_byte;
extern const CVI_U32 c2599_data_byte;
extern void c2599_init(VI_PIPE ViPipe);
extern void c2599_exit(VI_PIPE ViPipe);
extern void c2599_standby(VI_PIPE ViPipe);
extern void c2599_restart(VI_PIPE ViPipe);
extern int  c2599_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  c2599_read_register(VI_PIPE ViPipe, int addr);
extern void c2599_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  c2599_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __C2599_CMOS_EX_H_ */

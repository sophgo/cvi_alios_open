#ifndef __SC2356_CMOS_EX_H_
#define __SC2356_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "cif_uapi.h"
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define syslog(level, fmt, ...)            \
do {                                                   \
	printf(fmt, ##__VA_ARGS__);                \
} while (0)

enum sc2356_linear_regs_e {
	LINEAR_HOLD,
	LINEAR_EXP_H_ADDR,
	LINEAR_EXP_M_ADDR,
	LINEAR_EXP_L_ADDR,
	LINEAR_AGAIN_ADDR,
	LINEAR_DGAIN_ADDR,
	LINEAR_DGAIN_FINE_ADDR,
	LINEAR_VMAX_H_ADDR,
	LINEAR_VMAX_L_ADDR,
	LINEAR_REL,
	LINEAR_REGS_NUM
};

typedef enum _SC2356_MODE_E {
	SC2356_MODE_496X360P93 = 0,
	SC2356_MODE_1600X1200P30,
	SC2356_MODE_LINEAR_NUM,
	SC2356_MODE_496X360P93_WDR = SC2356_MODE_LINEAR_NUM,
	SC2356_MODE_NUM
} SC2356_MODE_E;

typedef struct _SC2356_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_S stAgain[2];
	SNS_ATTR_S stDgain[2];
	char name[64];
} SC2356_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastSC2356[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunSC2356_BusInfo[];
extern CVI_U16 g_au16SC2356_GainMode[];
extern CVI_U16 g_au16SC2356_L2SMode[];
extern const CVI_U8 sc2356_i2c_addr;
extern const CVI_U32 sc2356_addr_byte;
extern const CVI_U32 sc2356_data_byte;
extern void sc2356_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern void sc2356_init(VI_PIPE ViPipe);
extern void sc2356_exit(VI_PIPE ViPipe);
extern void sc2356_standby(VI_PIPE ViPipe);
extern void sc2356_restart(VI_PIPE ViPipe);
extern int  sc2356_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  sc2356_read_register(VI_PIPE ViPipe, int addr);
extern int  sc2356_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __SC2356_CMOS_EX_H_ */

#ifndef __sc020HGS_CMOS_EX_H_
#define __sc020HGS_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

enum sc020hgs_linear_regs_e {
	LINEAR_EXP_H_ADDR,
	LINEAR_EXP_M_ADDR,
	LINEAR_EXP_L_ADDR,
	LINEAR_AGAIN_H_ADDR,
	LINEAR_AGAIN_L_ADDR,
	LINEAR_DGAIN_H_ADDR,
	LINEAR_DGAIN_L_ADDR,
	LINEAR_VMAX_H_ADDR,
	LINEAR_VMAX_L_ADDR,
	LINEAR_REGS_NUM
};

typedef enum _SC020HGS_MODE_E {
	SC020HGS_MODE_400X400P120 = 0,
	SC020HGS_MODE_LINEAR_NUM,
	SC020HGS_MODE_NUM
} SC020HGS_MODE_E;

typedef struct _sc020HGS_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} SC020HGS_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastSC020HGS[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunSC020HGS_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunSC020HGS_AddrInfo[];
extern CVI_U16 g_au16SC020HGS_GainMode[];
extern CVI_U16 g_au16SC020HGS_L2SMode[];
extern const CVI_U8 sc020hgs_i2c_addr;
extern const CVI_U32 sc020hgs_addr_byte;
extern const CVI_U32 sc020hgs_data_byte;
extern void sc020hgs_init(VI_PIPE ViPipe);
extern void sc020hgs_exit(VI_PIPE ViPipe);
extern int  sc020hgs_i2c_exit(VI_PIPE ViPipe);
extern void sc020hgs_standby(VI_PIPE ViPipe);
extern void sc020hgs_restart(VI_PIPE ViPipe);
extern int  sc020hgs_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  sc020hgs_read_register(VI_PIPE ViPipe, int addr);
extern void sc020hgs_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __sc020HGS_CMOS_EX_H_ */

#ifndef __SC438AI_CMOS_EX_H_
#define __SC438AI_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

enum sc438ai_linear_regs_e {
	LINEAR_HOLD_START,
	LINEAR_SHS1_0_ADDR,
	LINEAR_SHS1_1_ADDR,
	LINEAR_SHS1_2_ADDR,
	LINEAR_AGAIN_ADDR,
	LINEAR_A_FINEGAIN_ADDR,
	LINEAR_DGAIN_ADDR,
	LINEAR_D_FINEGAIN_ADDR,
	LINEAR_VMAX_0_ADDR,
	LINEAR_VMAX_1_ADDR,
	LINEAR_HOLD_END,
	LINEAR_REGS_NUM
};

typedef enum _SC438AI_MODE_E {
	SC438AI_MODE_1440P30_2L = 0,
	SC438AI_MODE_1440P30_2L_MASTER,
	SC438AI_MODE_1440P30_2L_SLAVE,
	SC438AI_MODE_NUM
} SC438AI_MODE_E;

typedef struct _SC438AI_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_LARGE_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} SC438AI_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastSC438AI[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunSC438AI_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunSC438AI_AddrInfo[];
extern CVI_U16 g_au16SC438AI_GainMode[];
extern CVI_U16 g_au16SC438AI_L2SMode[];
extern CVI_U8 sc438ai_i2c_addr;
extern const CVI_U32 sc438ai_addr_byte;
extern const CVI_U32 sc438ai_data_byte;
extern void sc438ai_init(VI_PIPE ViPipe);
extern void sc438ai_exit(VI_PIPE ViPipe);
extern int  sc438ai_i2c_exit(VI_PIPE ViPipe);
extern void sc438ai_standby(VI_PIPE ViPipe);
extern void sc438ai_restart(VI_PIPE ViPipe);
extern int  sc438ai_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  sc438ai_read_register(VI_PIPE ViPipe, int addr);
extern void sc438ai_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  sc438ai_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __SC438AI_CMOS_EX_H_ */

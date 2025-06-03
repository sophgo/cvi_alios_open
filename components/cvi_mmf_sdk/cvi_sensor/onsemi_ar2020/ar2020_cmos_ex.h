#ifndef __AR2020_CMOS_EX_H_
#define __AR2020_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"


enum ar2020_linear_regs_e {
	LINEAR_SHS1_0_ADDR,
	LINEAR_AGAIN_ADDR,
	LINEAR_VMAX_0_ADDR,
	LINEAR_VMAX_1_ADDR,
	LINEAR_REGS_NUM
};

typedef enum _AR2020_MODE_E {
	AR2020_MODE_3840P30 = 0,
	AR2020_MODE_NUM
} AR2020_MODE_E;

typedef struct _AR2020_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_LARGE_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} AR2020_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastAR2020[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunAR2020_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunAr2020_AddrInfo[];
extern CVI_U16 g_au16AR2020_GainMode[];
extern CVI_U16 g_au16AR2020_L2SMode[];
extern CVI_U8 ar2020_i2c_addr;
extern const CVI_U32 ar2020_addr_byte;
extern const CVI_U32 ar2020_data_byte;
extern void ar2020_init(VI_PIPE ViPipe);
extern void ar2020_exit(VI_PIPE ViPipe);
extern int ar2020_i2c_exit(VI_PIPE ViPipe);
extern void ar2020_standby(VI_PIPE ViPipe);
extern void ar2020_restart(VI_PIPE ViPipe);
extern int  ar2020_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  ar2020_read_register(VI_PIPE ViPipe, int addr);
extern void ar2020_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  ar2020_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __AR2020_CMOS_EX_H_ */

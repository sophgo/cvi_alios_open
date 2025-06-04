#ifndef __IMX327_2L_CMOS_EX_H_
#define __IMX327_2L_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"


enum imx327_2l_linear_regs_e {
	LINEAR_HOLD = 0,
	LINEAR_SHS1_0,
	LINEAR_SHS1_1,
	LINEAR_SHS1_2,
	LINEAR_GAIN,
	LINEAR_HCG,
	LINEAR_VMAX_0,
	LINEAR_VMAX_1,
	LINEAR_VMAX_2,
	LINEAR_REL,
	LINEAR_REGS_NUM
};

enum imx327_2l_dol2_regs_e {
	DOL2_HOLD = 0,
	DOL2_SHS1_0,
	DOL2_SHS1_1,
	DOL2_SHS1_2,
	DOL2_GAIN,
	DOL2_HCG,
	DOL2_GAIN1,
	DOL2_RHS1_0,
	DOL2_RHS1_1,
	DOL2_RHS1_2,
	DOL2_SHS2_0,
	DOL2_SHS2_1,
	DOL2_SHS2_2,
	DOL2_VMAX_0,
	DOL2_VMAX_1,
	DOL2_VMAX_2,
	DOL2_YOUT_SIZE_0,
	DOL2_YOUT_SIZE_1,
	DOL2_REL,
	DOL2_REGS_NUM
};

typedef enum _IMX327_2L_MODE_E {
	IMX327_2L_MODE_1080P30 = 0,
	IMX327_2L_MODE_LINEAR_NUM,
	IMX327_2L_MODE_1080P30_WDR = IMX327_2L_MODE_LINEAR_NUM,
	IMX327_2L_MODE_NUM
} IMX327_2L_MODE_E;

typedef struct _IMX327_2L_STATE_S {
	CVI_U8       u8Hcg;
	CVI_U32      u32BRL;
	CVI_U32      u32RHS1;
	CVI_U32      u32RHS1_MAX;
} IMX327_2L_STATE_S;

typedef struct _IMX327_2L_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_S stAgain[2];
	SNS_ATTR_S stDgain[2];
	CVI_U16 u16RHS1;
	CVI_U16 u16BRL;
	CVI_U16 u16OpbSize;
	CVI_U16 u16MarginVtop;
	CVI_U16 u16MarginVbot;
	char name[64];
} IMX327_2L_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastImx327_2l[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunImx327_2l_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunImx327_2l_AddrInfo[];
extern CVI_U16 g_au16Imx327_2l_GainMode[];
extern const CVI_U8 imx327_2l_i2c_addr;
extern const CVI_U32 imx327_2l_addr_byte;
extern const CVI_U32 imx327_2l_data_byte;
extern void imx327_2l_init(VI_PIPE ViPipe);
extern void imx327_2l_exit(VI_PIPE ViPipe);
extern int imx327_2l_i2c_exit(VI_PIPE ViPipe);
extern void imx327_2l_standby(VI_PIPE ViPipe);
extern void imx327_2l_restart(VI_PIPE ViPipe);
extern int  imx327_2l_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  imx327_2l_read_register(VI_PIPE ViPipe, int addr);
extern void imx327_2l_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  imx327_2l_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __IMX327_2L_CMOS_EX_H_ */

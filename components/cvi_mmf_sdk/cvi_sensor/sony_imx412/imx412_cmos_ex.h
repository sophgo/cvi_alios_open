#ifndef __IMX412_CMOS_EX_H_
#define __IMX412_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

enum imx412_linear_regs_e {
	LINEAR_HOLD = 0,      //0x3001
	LINEAR_SHR_L,         //0x3059[19:0]
	LINEAR_SHR_M,
	LINEAR_SHR_H,
	LINEAR_GAIN_L,        //0x30E8[10:0]
	LINEAR_GAIN_H,
	LINEAR_HCG,
	LINEAR_VMAX_L,        //0x3030[19:0]
	LINEAR_VMAX_M,
	LINEAR_VMAX_H,
	LINEAR_REGS_NUM
};

enum imx412_dol2_regs_e {
	DOL_REG_HOLD = 0,
	DOL_REG_REL,
	DOL_FRM_OFFSET_H,
	DOL_FRM_OFFSET_L,
	DOL_CIT1_H,
	DOL_CIT1_L,
	DOL_CIT2_H,
	DOL_CIT2_L,
	DOL_AGAIN1_H,
	DOL_AGAIN1_L,
	DOL_AGAIN2_H,
	DOL_AGAIN2_L,
	DOL_DGAIN1_H,
	DOL_DGAIN1_L,
	DOL_DGAIN2_H,
	DOL_DGAIN2_L,
	DOL_FRM_LINES_H,
	DOL_FRM_LINES_L,
	DOL2_REGS_NUM
};

typedef enum _IMX412_MODE_E {
	IMX412_MODE_12M30 = 0,
	IMX412_MODE_LINEAR_NUM,
	IMX412_MODE_12M30_WDR = IMX412_MODE_LINEAR_NUM,
	IMX412_MODE_8M30_WDR,
	IMX412_MODE_NUM
} IMX412_MODE_E;

typedef struct _IMX412_STATE_S {
	CVI_U8       u8Hcg;
	CVI_U32      u32BRL;
	CVI_U32      u32RHS1;
	CVI_U32      u32RHS1_MAX;
} IMX412_STATE_S;

typedef struct _IMX412_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	CVI_U16 u16RHS1;
	CVI_U16 u16BRL;
	CVI_U16 u16OpbSize;
	CVI_U16 u16MarginVtop;
	CVI_U16 u16MarginVbot;
	char name[64];
} IMX412_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastImx412[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunImx412_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunImx412_AddrInfo[];
extern CVI_U16 g_au16Imx412_GainMode[];
extern const CVI_U32 imx412_addr_byte;
extern const CVI_U32 imx412_data_byte;
extern void imx412_init(VI_PIPE ViPipe);
extern void imx412_exit(VI_PIPE ViPipe);
extern int imx412_i2c_exit(VI_PIPE ViPipe);
extern void imx412_standby(VI_PIPE ViPipe);
extern void imx412_restart(VI_PIPE ViPipe);
extern int  imx412_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  imx412_read_register(VI_PIPE ViPipe, int addr);
extern void imx412_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __IMX412_CMOS_EX_H_ */

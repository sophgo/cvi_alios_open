#ifndef __IMX678_CMOS_EX_H_
#define __IMX678_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

/* [TODO] ======== Temporarily definitions end ========*/
enum imx678_linear_regs_e {
	LINEAR_HOLD = 0,
	LINEAR_SHR0_0,
	LINEAR_SHR0_1,
	LINEAR_SHR0_2,
	LINEAR_GAIN_L,
	LINEAR_GAIN_H,
	LINEAR_HCG,
	LINEAR_VMAX_0,
	LINEAR_VMAX_1,
	LINEAR_VMAX_2,
	LINEAR_REL,
	LINEAR_REGS_NUM
};

enum imx678_dol2_regs_e {
	DOL2_HOLD = 0,
	DOL2_SHR0_0,
	DOL2_SHR0_1,
	DOL2_SHR0_2,
	DOL2_GAIN,
	DOL2_HCG,
	DOL2_GAIN1,
	DOL2_RHS1_0,
	DOL2_RHS1_1,
	DOL2_RHS1_2,
	DOL2_SHR1_0,
	DOL2_SHR1_1,
	DOL2_SHR1_2,
	DOL2_VMAX_0,
	DOL2_VMAX_1,
	DOL2_VMAX_2,
	DOL2_REL,
	DOL2_REGS_NUM
};

typedef enum _IMX678_MODE_E {
	IMX678_MODE_8M30 = 0,
	IMX678_MODE_LINEAR_NUM,
	IMX678_MODE_5M25_WDR = IMX678_MODE_LINEAR_NUM,
	IMX678_MODE_NUM
} IMX678_MODE_E;

typedef struct _IMX678_STATE_S {
	CVI_U8       u8Hcg;
	CVI_U32      u32BRL;
	CVI_U32      u32SHR1;
	CVI_U32      u32RHS1;
	CVI_U32      u32RHS1_MAX;
} IMX678_STATE_S;

typedef struct _IMX678_MODE_S {
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
} IMX678_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastImx678[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunImx678_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunImx678_AddrInfo[];
extern CVI_U16 g_au16Imx678_GainMode[];
extern const CVI_U8 imx678_i2c_addr;
extern const CVI_U32 imx678_addr_byte;
extern const CVI_U32 imx678_data_byte;
extern void imx678_init(VI_PIPE ViPipe);
extern void imx678_exit(VI_PIPE ViPipe);
extern void imx678_standby(VI_PIPE ViPipe);
extern void imx678_restart(VI_PIPE ViPipe);
extern int imx678_i2c_exit(VI_PIPE ViPipe);
extern int imx678_write_register(VI_PIPE ViPipe, int addr, int data);
extern int imx678_read_register(VI_PIPE ViPipe, int addr);
extern void imx678_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int imx678_probe(VI_PIPE ViPipe);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __IMX678_CMOS_EX_H_ */

#ifndef __IMX415_CMOS_EX_H_
#define __IMX415_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

enum imx415_linear_regs_e {
	LINEAR_SHR_L = 0,
	LINEAR_SHR_M,
	LINEAR_SHR_H,
	LINEAR_GAIN_L,
	LINEAR_GAIN_H,
	LINEAR_VMAX_L,
	LINEAR_VMAX_M,
	LINEAR_VMAX_H,
	LINEAR_REGS_NUM
};

enum imx415_wdr_regs_e {
	WDR_SHR0_L = 0,
	WDR_SHR0_H,
	WDR_SHR1_L,
	WDR_SHR1_H,
	WDR_GAIN_L,
	WDR_GAIN_H,
	WDR_GAIN_SHORT_L,
	WDR_GAIN_SHORT_H,
	WDR_VMAX_L,
	WDR_VMAX_H,
	WDR_REGS_NUM
};

typedef enum _IMX415_MODE_E {
	IMX415_MODE_4M25 = 0,
	IMX415_MODE_8M25,
	IMX415_MODE_8M30,
	IMX415_MODE_5M25,
	IMX415_MODE_2M60,
	IMX415_MODE_LINEAR_NUM,
	IMX415_MODE_4M25_WDR = IMX415_MODE_LINEAR_NUM,
	IMX415_MODE_NUM
} IMX415_MODE_E;

typedef struct _IMX415_MODE_S {
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
} IMX415_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastImx415[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunImx415_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunImx415_AddrInfo[];
extern CVI_U16 g_au16Imx415_GainMode[];
extern const CVI_U8 imx415_i2c_addr;
extern const CVI_U32 imx415_addr_byte;
extern const CVI_U32 imx415_data_byte;
extern void imx415_init(VI_PIPE ViPipe);
extern void imx415_exit(VI_PIPE ViPipe);
extern int imx415_i2c_exit(VI_PIPE ViPipe);
extern void imx415_standby(VI_PIPE ViPipe);
extern void imx415_restart(VI_PIPE ViPipe);
extern int  imx415_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  imx415_read_register(VI_PIPE ViPipe, int addr);
extern void imx415_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  imx415_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __IMX415_CMOS_EX_H_ */

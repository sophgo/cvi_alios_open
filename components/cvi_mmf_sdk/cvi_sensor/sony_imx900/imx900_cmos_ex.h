#ifndef __IMX900_CMOS_EX_H_
#define __IMX900_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

enum imx900_linear_regs_e {
	LINEAR_HOLD = 0,      //0x3001
	LINEAR_GAIN_L,        //0x30E8[10:0]
	LINEAR_GAIN_H,
	LINEAR_HCG,
	LINEAR_VMAX_L,        //0x3030[19:0]
	LINEAR_VMAX_M,
	LINEAR_VMAX_H,
	LINEAR_REL,
	LINEAR_REGS_NUM
};

typedef enum _IMX900_MODE_E {
	IMX900_MODE_3M70_COLOR = 0,
	IMX900_MODE_3M70_MONO,
	IMX900_MODE_LINEAR_NUM,
	IMX900_MODE_NUM
} IMX900_MODE_E;

typedef struct _IMX900_STATE_S {
	CVI_U8       u8Hcg;
	CVI_U32      u32BRL;
	CVI_U32      u32RHS1;
	CVI_U32      u32RHS1_MAX;
} IMX900_STATE_S;

typedef struct _IMX900_MODE_S {
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
} IMX900_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastImx900[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunImx900_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunImx900_AddrInfo[];
extern CVI_U16 g_au16Imx900_GainMode[];
extern const CVI_U8 imx900_i2c_addr;
extern const CVI_U32 imx900_addr_byte;
extern const CVI_U32 imx900_data_byte;
extern void imx900_init(VI_PIPE ViPipe);
extern void imx900_exit(VI_PIPE ViPipe);
extern int imx900_i2c_exit(VI_PIPE ViPipe);
extern void imx900_standby(VI_PIPE ViPipe);
extern void imx900_restart(VI_PIPE ViPipe);
extern int  imx900_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  imx900_read_register(VI_PIPE ViPipe, int addr);
extern void imx900_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __IMX900_CMOS_EX_H_ */

#ifndef __IMX307_SLAVE_CMOS_EX_H_
#define __IMX307_SLAVE_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

/* [TODO] ======== Temporarily definitions end ========*/
enum imx307_slave_linear_regs_e {
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

enum imx307_slave_dol2_regs_e {
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

typedef enum _IMX307_SLAVE_MODE_E {
	IMX307_SLAVE_MODE_1080P30 = 0,
	IMX307_SLAVE_MODE_LINEAR_NUM,
	IMX307_SLAVE_MODE_1080P30_WDR = IMX307_SLAVE_MODE_LINEAR_NUM,
	IMX307_SLAVE_MODE_NUM
} IMX307_SLAVE_MODE_E;

typedef struct _IMX307_SLAVE_STATE_S {
	CVI_U8       u8Hcg;
	CVI_U32      u32BRL;
	CVI_U32      u32RHS1;
	CVI_U32      u32RHS1_MAX;
} IMX307_SLAVE_STATE_S;

typedef struct _IMX307_SLAVE_MODE_S {
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
} IMX307_SLAVE_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastImx307_Slave[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunImx307_Slave_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunImx307_Slave_AddrInfo[];
extern CVI_U16 g_au16Imx307_Slave_UseHwSync[];
extern CVI_U16 g_au16Imx307_Slave_GainMode[];
extern CVI_U8 imx307_slave_i2c_addr;
extern const CVI_U32 imx307_slave_addr_byte;
extern const CVI_U32 imx307_slave_data_byte;
extern void imx307_slave_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern void imx307_slave_init(VI_PIPE ViPipe);
extern void imx307_slave_exit(VI_PIPE ViPipe);
extern int imx307_slave_i2c_exit(VI_PIPE ViPipe);
extern void imx307_slave_standby(VI_PIPE ViPipe);
extern void imx307_slave_restart(VI_PIPE ViPipe);
extern int  imx307_slave_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  imx307_slave_read_register(VI_PIPE ViPipe, int addr);
extern int  imx307_slave_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __IMX307_SLAVE_CMOS_EX_H_ */

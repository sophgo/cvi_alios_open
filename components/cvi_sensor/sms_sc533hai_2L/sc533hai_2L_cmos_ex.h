#ifndef __SC533HAI_2L_CMOS_EX_H_
#define __SC533HAI_2L_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#ifdef ARCH_CV182X
#include "cvi_vip_cif_uapi.h"
#else
#include "cif_uapi.h"
#endif
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define syslog(level, fmt, ...)            \
do {                                                   \
	printf(fmt, ##__VA_ARGS__);                \
} while (0)

enum sc533hai_2l_linear_regs_e {
	LINEAR_HOLD_START,
	LINEAR_SHS0_H_ADDR,
	LINEAR_SHS0_M_ADDR,
	LINEAR_SHS0_L_ADDR,
	LINEAR_AGAIN_ADDR,
	LINEAR_A_FINEGAIN_ADDR,
	LINEAR_DGAIN_ADDR,
	LINEAR_D_FINEGAIN_ADDR,
	LINEAR_VMAX_H_ADDR,
	LINEAR_VMAX_M_ADDR,
	LINEAR_VMAX_L_ADDR,
	LINEAR_HOLD_END,
	LINEAR_REGS_NUM
};

enum sc533hai_2l_wdr_regs_e {
	WDR2_HOLD_START,
	WDR2_SHS0_H_ADDR,
	WDR2_SHS0_M_ADDR,
	WDR2_SHS0_L_ADDR,
	WDR2_SHS1_H_ADDR,
	WDR2_SHS1_M_ADDR,
	WDR2_SHS1_L_ADDR,
	WDR2_AGAIN0_ADDR,
	WDR2_A_FINEGAIN0_ADDR,
	WDR2_DGAIN0_ADDR,
	WDR2_D_FINEGAIN0_ADDR,
	WDR2_AGAIN1_ADDR,
	WDR2_A_FINEGAIN1_ADDR,
	WDR2_DGAIN1_ADDR,
	WDR2_D_FINEGAIN1_ADDR,
	WDR2_VMAX_H_ADDR,
	WDR2_VMAX_M_ADDR,
	WDR2_VMAX_L_ADDR,
	WDR2_MAXSEXP_H_ADDR,
	WDR2_MAXSEXP_M_ADDR,
	WDR2_MAXSEXP_L_ADDR,
	WDR2_HOLD_END,
	WDR2_REGS_NUM
};

typedef enum _SC533HAI_2L_MODE_E {
	SC533HAI_2L_MODE_1620P30 = 0,
	SC533HAI_2L_LINEAR_NUM,
	SC533HAI_2L_MODE_1620P30_WDR = SC533HAI_2L_LINEAR_NUM,
	SC533HAI_2L_MODE_NUM
} SC533HAI_2L_MODE_E;

typedef struct _SC533hAI_STATE_S {
	CVI_U32		u32Sexp_MAX;
} SC533HAI_2L_STATE_S;

typedef struct _SC533HAI_2L_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_LARGE_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	CVI_U16 u16SexpMaxReg;
	char name[64];
} SC533HAI_2L_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastSC533HAI_2L[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunSC533HAI_2L_BusInfo[];
extern CVI_U16 g_au16SC533HAI_2L_GainMode[];
extern CVI_U16 g_au16SC533HAI_2L_L2SMode[];
extern CVI_U8 sc533hai_2l_i2c_addr;
extern const CVI_U32 sc533hai_2l_addr_byte;
extern const CVI_U32 sc533hai_2l_data_byte;
extern void sc533hai_2l_init(VI_PIPE ViPipe);
extern void sc533hai_2l_exit(VI_PIPE ViPipe);
extern int  sc533hai_2l_i2c_exit(VI_PIPE ViPipe);
extern void sc533hai_2l_standby(VI_PIPE ViPipe);
extern void sc533hai_2l_restart(VI_PIPE ViPipe);
extern int  sc533hai_2l_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  sc533hai_2l_read_register(VI_PIPE ViPipe, int addr);
extern void sc533hai_2l_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  sc533hai_2l_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __SC533HAI_2L_CMOS_EX_H_ */

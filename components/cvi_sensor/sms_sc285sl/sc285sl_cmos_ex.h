#ifndef __SC285SL_CMOS_EX_H_
#define __SC285SL_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "cvi_comm_cif.h"
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define syslog(level, fmt, ...)            \
do {                                                   \
	printf(fmt, ##__VA_ARGS__);                \
} while (0)

#define SC285SL_I2C_ADDR_1 0x30
#define SC285SL_I2C_ADDR_2 0x32

enum sc285sl_linear_regs_e {
	LINEAR_HOLD_START = 0,
	LINEAR_SHS1_0_ADDR,
	LINEAR_SHS1_1_ADDR,
	LINEAR_SHS1_2_ADDR,
	LINEAR_SHS1_3_ADDR,
	LINEAR_AGAIN_ADDR,
	LINEAR_AGAIN_FINE_ADDR,
	LINEAR_DGAIN_ADDR,
	LINEAR_DGAIN_FINE_ADDR,
	LINEAR_VMAX_0_ADDR,
	LINEAR_VMAX_1_ADDR,
	LINEAR_VMAX_2_ADDR,
	LINEAR_HOLD_END,
	LINEAR_REGS_NUM
};

enum sc285sl_wdr_regs_e {
	WDR_HOLD_START = 0,
	WDR_SHS1_0_ADDR,
	WDR_SHS1_1_ADDR,
	WDR_SHS1_2_ADDR,
	WDR_SHS1_3_ADDR,
	WDR_SHS2_0_ADDR,
	WDR_SHS2_1_ADDR,
	WDR_SHS2_2_ADDR,
	WDR_SHS2_3_ADDR,
	WDR_AGAIN_L_ADDR,
	WDR_AGAIN_L_FINE_ADDR,
	WDR_DGAIN_L_ADDR,
	WDR_DGAIN_L_FINE_ADDR,
	WDR_AGAIN_S_ADDR,
	WDR_AGAIN_S_FINE_ADDR,
	WDR_DGAIN_S_ADDR,
	WDR_DGAIN_S_FINE_ADDR,
	WDR_VMAX_0_ADDR,
	WDR_VMAX_1_ADDR,
	WDR_VMAX_2_ADDR,
	WDR2_MAXSEXP_H_ADDR,
	WDR2_MAXSEXP_M_ADDR,
	WDR2_MAXSEXP_L_ADDR,
	WDR_HOLD_END,
	WDR_REGS_NUM
};

typedef enum _SC285SL_MODE_E {
	SC285SL_MODE_1920X1080P30 = 0,
	SC285SL_MODE_LINEAR_NUM,
	SC285SL_MODE_1920X1080P30_WDR = SC285SL_MODE_LINEAR_NUM,
	SC285SL_MODE_NUM
} SC285SL_MODE_E;

typedef struct _SC285SL_STATE_S {
	CVI_U32 u32Sexp_MAX;
} SC285SL_STATE_S;

typedef struct _SC285SL_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	CVI_U16 u16SexpMaxReg;
	char name[64];
} SC285SL_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastSc285sl[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunSc285sl_BusInfo[];
extern ISP_SNS_MIRRORFLIP_TYPE_E g_aeSc285sl_MirrorFip[VI_MAX_PIPE_NUM];
extern CVI_U8 sc285sl_i2c_addr;
extern const CVI_U32 sc285sl_addr_byte;
extern const CVI_U32 sc285sl_data_byte;
extern void sc285sl_init(VI_PIPE ViPipe);
extern void sc285sl_exit(VI_PIPE ViPipe);
extern void sc285sl_standby(VI_PIPE ViPipe);
extern void sc285sl_restart(VI_PIPE ViPipe);
extern void sc285sl_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int sc285sl_write_register(VI_PIPE ViPipe, int addr, int data);
extern int sc285sl_read_register(VI_PIPE ViPipe, int addr);
extern int sc285sl_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* __SC285SL_CMOS_EX_H_ */

#ifndef __GC8613_CMOS_EX_H_
#define __GC8613_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef ARCH_CV182X
#include <linux/cvi_vip_cif.h>
#include <linux/cvi_vip_snsr.h>
#include "cvi_type.h"
#else
#include <cvi_comm_cif.h>
#include <cvi_type.h>
#endif
#include "cvi_sns_ctrl.h"

#define GC8613_I2C_ADDR_1 0x31
#define GC8613_I2C_ADDR_2 0x7f
#define GC8613_ID 0x8613

enum gc8613_linear_regs_e {
	LINEAR_SHR_H = 0,
	LINEAR_SHR_L,
	LINEAR_HOLD,
	LINEAR_GAIN_L,
	LINEAR_GAIN_H,
	LINEAR_REL,
	LINEAR_GAIN_F,
	LINEAR_MAG_1,
	LINEAR_MAG_2,
	LINEAR_COL_GAIN_H,
	LINEAR_COL_GAIN_L,
	LINEAR_MAG_3,
	LINEAR_PRE_GAIN_H,
	LINEAR_PRE_GAIN_L,
	LINEAR_VMAX_H,
	LINEAR_VMAX_L,
	LINEAR_REGS_NUM
};

enum gc8613_wdr_regs_e {
	WDR_SHR1_H = 0,
	WDR_SHR1_L,
	WDR_SHR0_H,
	WDR_SHR0_L,
	WDR_HOLD,
	WDR_GAIN_L,
	WDR_GAIN_H,
	WDR_REL,
	WDR_GAIN_F,
	WDR_MAG_1,
	WDR_MAG_2,
	WDR_COL_GAIN_H,
	WDR_COL_GAIN_L,
	WDR_MAG_3,
	WDR_PRE_GAIN_L,
	WDR_PRE_GAIN_H,
	WDR_PRE_GAIN1_L,
	WDR_PRE_GAIN1_H,
	WDR_VMAX_H,
	WDR_VMAX_L,
	WDR_REGS_NUM
};

typedef enum _GC8613_MODE_E {
	GC8613_MODE_8M30 = 0,
	GC8613_MODE_8M25,
	GC8613_MODE_2M30,
	GC8613_MODE_8M60_30,
	GC8613_MODE_LINEAR_NUM,
	GC8613_MODE_8M30_WDR = GC8613_MODE_LINEAR_NUM,
	GC8613_MODE_NUM
} GC8613_MODE_E;

typedef struct _GC8613_MODE_S {
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
} GC8613_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastGc8613[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunGc8613_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunGc8613_AddrInfo[];
extern CVI_U16 g_au16Gc8613_GainMode[];
extern const CVI_U8 gc8613_i2c_addr;
extern const CVI_U32 gc8613_addr_byte;
extern const CVI_U32 gc8613_data_byte;
extern void gc8613_init(VI_PIPE ViPipe);
extern void gc8613_exit(VI_PIPE ViPipe);
extern int gc8613_i2c_exit(VI_PIPE ViPipe);
extern void gc8613_standby(VI_PIPE ViPipe);
extern void gc8613_restart(VI_PIPE ViPipe);
extern int  gc8613_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  gc8613_read_register(VI_PIPE ViPipe, int addr);
extern void gc8613_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  gc8613_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __GC8613_CMOS_EX_H_ */

#ifndef __C4390_CMOS_EX_H_
#define __C4390_CMOS_EX_H_

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

enum c4390_linear_regs_e {
	LINEAR_SHS1_0_ADDR,
	LINEAR_SHS1_1_ADDR,
	LINEAR_AGAIN_ADDR,
	LINEAR_DGAIN_ADDR,
	LINEAR_D_FINEGAIN_ADDR,
	LINEAR_VMAX_0_ADDR,
	LINEAR_VMAX_1_ADDR,
	LINEAR_REGS_NUM
};

typedef enum _C4390_MODE_E {
	C4390_MODE_1440P30 = 0,
	C4390_MODE_NUM
} C4390_MODE_E;

typedef struct _C4390_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_LARGE_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} C4390_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastC4390[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunC4390_BusInfo[];
extern CVI_U16 g_au16C4390_GainMode[];
extern CVI_U16 g_au16C4390_L2SMode[];
extern CVI_U8 c4390_i2c_addr;
extern const CVI_U32 c4390_addr_byte;
extern const CVI_U32 c4390_data_byte;
extern void c4390_init(VI_PIPE ViPipe);
extern void c4390_exit(VI_PIPE ViPipe);
extern void c4390_standby(VI_PIPE ViPipe);
extern void c4390_restart(VI_PIPE ViPipe);
extern int  c4390_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  c4390_read_register(VI_PIPE ViPipe, int addr);
extern void c4390_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  c4390_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __C4390_CMOS_EX_H_ */

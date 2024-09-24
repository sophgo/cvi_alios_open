#ifndef __BF2253L_CMOS_EX_H_
#define __BF2253L_CMOS_EX_H_

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

enum bf2253l_linear_regs_e {
	LINEAR_SHS1_0_ADDR,
	LINEAR_SHS1_1_ADDR,
	LINEAR_AGAIN_0_ADDR,
	LINEAR_DGAIN_0_ADDR,
	LINEAR_DL_0_ADDR,	//dummy line high 8bit
	LINEAR_DL_1_ADDR,	//dummy line low 8bit
	LINEAR_FLIP_MIRROR_ADDR,
	LINEAR_REGS_NUM
};


typedef enum _BF2253L_MODE_E {
	BF2253L_MODE_1200P10 = 0,
	BF2253L_MODE_LINEAR_NUM,
	BF2253L_MODE_NUM
} BF2253L_MODE_E;

typedef struct _BF2253L_STATE_S {
	CVI_U32		u32Sexp_MAX;
} BF2253L_STATE_S;

typedef struct _BF2253L_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_S stAgain[2];
	SNS_ATTR_S stDgain[2];
	CVI_U16 u16SexpMaxReg;
	char name[64];
} BF2253L_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastBF2253L[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunBF2253L_BusInfo[];
extern CVI_U16 g_au16BF2253L_GainMode[];
extern CVI_U16 g_au16BF2253L_L2SMode[];
extern const CVI_U8 bf2253l_i2c_addr;
extern const CVI_U32 bf2253l_addr_byte;
extern const CVI_U32 bf2253l_data_byte;
extern void bf2253l_init(VI_PIPE ViPipe);
extern void bf2253l_exit(VI_PIPE ViPipe);
extern void bf2253l_standby(VI_PIPE ViPipe);
extern void bf2253l_restart(VI_PIPE ViPipe);
extern int  bf2253l_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  bf2253l_read_register(VI_PIPE ViPipe, int addr);
extern void bf2253l_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  bf2253l_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __BF2253L_CMOS_EX_H_ */

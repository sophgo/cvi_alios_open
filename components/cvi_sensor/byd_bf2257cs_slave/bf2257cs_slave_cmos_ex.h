#ifndef __BF2257CS_SLAVE_CMOS_EX_H_
#define __BF2257CS_SLAVE_CMOS_EX_H_

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


enum bf2257cs_slave_linear_regs_e {
	LINEAR_SHS1_0_ADDR,
	LINEAR_SHS1_1_ADDR,
	LINEAR_AGAIN_0_ADDR,
	LINEAR_DL_0_ADDR,	//dummy line high 8bit
	LINEAR_DL_1_ADDR,	//dummy line low 8bit
	LINEAR_FLIP_MIRROR_ADDR,
	LINEAR_REGS_NUM
};


typedef enum _BF2257CS_SLAVE_MODE_E {
	BF2257CS_SLAVE_MODE_1200P30 = 0,
	BF2257CS_SLAVE_MODE_LINEAR_NUM,
	BF2257CS_SLAVE_MODE_NUM
} BF2257CS_SLAVE_MODE_E;

typedef struct _BF2257CS_SLAVE_STATE_S {
	CVI_U32		u32Sexp_MAX;
} BF2257CS_SLAVE_STATE_S;

typedef struct _BF2257CS_SLAVE_MODE_S {
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
} BF2257CS_SLAVE_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastBF2257CS_Slave[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunBF2257CS_Slave_BusInfo[];
extern CVI_U16 g_au16BF2257CS_Slave_GainMode[];
extern CVI_U16 g_au16BF2257CSL_Slave_L2SMode[];
extern  CVI_U8 bf2257cs_slave_i2c_addr;
extern const CVI_U32 bf2257cs_slave_addr_byte;
extern const CVI_U32 bf2257cs_slave_data_byte;
extern void bf2257cs_slave_init(VI_PIPE ViPipe);
extern void bf2257cs_slave_exit(VI_PIPE ViPipe);
extern void bf2257cs_slave_standby(VI_PIPE ViPipe);
extern void bf2257cs_slave_restart(VI_PIPE ViPipe);
extern int  bf2257cs_slave_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  bf2257cs_slave_read_register(VI_PIPE ViPipe, int addr);
extern void bf2257cs_slave_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  bf2257cs_slave_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __BF2257CS_SLAVE_CMOS_EX_H_ */

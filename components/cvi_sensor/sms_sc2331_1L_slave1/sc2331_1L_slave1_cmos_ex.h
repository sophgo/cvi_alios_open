#ifndef __SC2331_1L_SLAVE1_CMOS_EX_H_
#define __SC2331_1L_SLAVE1_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "cvi_comm_cif.h"
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"

#define syslog(level, fmt, ...)            \
do {                                                   \
	printf(fmt, ##__VA_ARGS__);                \
} while (0)

enum sc2331_1L_slave1_linear_regs_e {
	LINEAR_EXP_H_ADDR,
	LINEAR_EXP_M_ADDR,
	LINEAR_EXP_L_ADDR,
	LINEAR_AGAIN_H_ADDR,
	LINEAR_AGAIN_L_ADDR,
	LINEAR_DGAIN_H_ADDR,
	LINEAR_DGAIN_L_ADDR,
	LINEAR_VMAX_H_ADDR,
	LINEAR_VMAX_L_ADDR,
	LINEAR_REGS_NUM
};

typedef enum _SC2331_1L_SLAVE1_MODE_E {
	SC2331_1L_SLAVE1_MODE_1920X1080P30 = 0,
	SC2331_1L_SLAVE1_MODE_LINEAR_NUM,
	SC2331_1L_SLAVE1_MODE_NUM
} SC2331_1L_SLAVE1_MODE_E;

typedef struct _SC2331_1L_SLAVE1_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_S stAgain[2];
	SNS_ATTR_S stDgain[2];
	char name[64];
} SC2331_1L_SLAVE1_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastSC2331_1L_Slave1[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunSC2331_1L_Slave1_BusInfo[];
extern CVI_U16 g_au16SC2331_1L_GainMode[];
extern CVI_U16 g_au16SC2331_1L_L2SMode[];
extern CVI_U8 sc2331_1L_slave1_i2c_addr;
extern CVI_U32 sc2331_1L_slave1_addr_byte;
extern CVI_U32 sc2331_1L_slave1_data_byte;
extern void sc2331_1L_slave1_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern void sc2331_1L_slave1_init(VI_PIPE ViPipe);
extern void sc2331_1L_slave1_exit(VI_PIPE ViPipe);
extern void sc2331_1L_slave1_standby(VI_PIPE ViPipe);
extern void sc2331_1L_slave1_restart(VI_PIPE ViPipe);
extern int  sc2331_1L_slave1_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  sc2331_1L_slave1_read_register(VI_PIPE ViPipe, int addr);
extern int  sc2331_1L_slave1_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __SC2331_1L_SLAVE1_CMOS_EX_H_ */

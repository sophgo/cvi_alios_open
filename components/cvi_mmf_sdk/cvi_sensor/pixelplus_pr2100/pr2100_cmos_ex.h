#ifndef __PR2100_CMOS_EX_H_
#define __PR2100_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

typedef enum _PR2100_MODE_E {
	PR2100_MODE_NONE,
	PR2100_MODE_1080P25,
	PR2100_MODE_1080P25_2CH,
	PR2100_MODE_1080P25_2CH_2L,
	PR2100_MODE_1080P25_4CH,
	PR2100_MODE_1080P30_4CH,
	PR2100_MODE_NUM
} PR2100_MODE_E;

typedef struct _PR2100_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_S stAgain[2];
	SNS_ATTR_S stDgain[2];
	CVI_U8 u8DgainReg;
	char name[64];
} PR2100_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastPr2100[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunPr2100_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunPr2100_AddrInfo[];
extern const CVI_U8 pr2100_i2c_addr;
extern const CVI_U32 pr2100_addr_byte;
extern const CVI_U32 pr2100_data_byte;
extern void pr2100_init(VI_PIPE ViPipe);
extern void pr2100_exit(VI_PIPE ViPipe);
extern int  pr2100_i2c_exit(VI_PIPE ViPipe);
extern void pr2100_standby(VI_PIPE ViPipe);
extern void pr2100_restart(VI_PIPE ViPipe);
extern int  pr2100_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  pr2100_read_register(VI_PIPE ViPipe, int addr);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __PR2100_CMOS_EX_H_ */

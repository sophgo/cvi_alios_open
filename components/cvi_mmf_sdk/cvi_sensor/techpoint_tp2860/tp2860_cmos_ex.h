#ifndef __TP2860_CMOS_EX_H_
#define __TP2860_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
typedef enum _TP2860_MODE_E {
	TP2860_MODE_1080P_25P,
	TP2860_MODE_NUM
} TP2860_MODE_E;
typedef struct _TP2860_MODE_S {
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
} TP2860_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pasttp2860[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_auntp2860_BusInfo[];
extern ISP_SNS_COMMADDR_U g_auntp2860_AddrInfo[];
extern const CVI_U8 tp2860_i2c_addr;
extern const CVI_U32 tp2860_addr_byte;
extern const CVI_U32 tp2860_data_byte;
extern void tp2860_init(VI_PIPE ViPipe);
extern void tp2860_exit(VI_PIPE ViPipe);
extern int  tp2860_i2c_exit(VI_PIPE ViPipe);
extern void tp2860_standby(VI_PIPE ViPipe);
extern void tp2860_restart(VI_PIPE ViPipe);
extern int  tp2860_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  tp2860_read_register(VI_PIPE ViPipe, int addr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __tp2860_CMOS_EX_H_ */


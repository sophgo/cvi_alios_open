#ifndef __nc021_CMOS_EX_H_
#define __nc021_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

typedef enum _NC021_MODE_E {
	NC021_MODE_NONE,
	NC021_MODE_1080P,
	NC021_MODE_NUM
} NC021_MODE_E;

typedef struct _NC021_MODE_S {
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
} NC021_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastnc021[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunnc021_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunnc021_AddrInfo[];
extern const CVI_U8 nc021_i2c_addr;
extern const CVI_U32 nc021_addr_byte;
extern const CVI_U32 nc021_data_byte;
extern void nc021_init(VI_PIPE ViPipe);
extern void nc021_exit(VI_PIPE ViPipe);
extern int  nc021_i2c_exit(VI_PIPE ViPipe);
extern void nc021_standby(VI_PIPE ViPipe);
extern void nc021_restart(VI_PIPE ViPipe);
extern int  nc021_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  nc021_read_register(VI_PIPE ViPipe, int addr);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __nc021_CMOS_EX_H_ */

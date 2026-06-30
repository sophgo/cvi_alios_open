#ifndef __XS9950_CMOS_EX_H_
#define __XS9950_CMOS_EX_H_

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

typedef enum _XS9950_MODE_E {
	XS9950_MODE_NONE,
	XS9950_MODE_720H_NTSC,
	XS9950_MODE_720H_PAL,
	XS9950_MODE_720P_25,
	XS9950_MODE_720P_30,
	XS9950_MODE_720P_50,
	XS9950_MODE_720P_60,
	XS9950_MODE_1080P_25,
	XS9950_MODE_1080P_30,
	XS9950_MODE_NUM
} XS9950_MODE_E;

typedef struct _XS9950_MODE_S {
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
} XS9950_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastXs9950[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunXs9950_BusInfo[];
extern const CVI_U8 xs9950_i2c_addr;
extern const CVI_U32 xs9950_addr_byte;
extern const CVI_U32 xs9950_data_byte;
extern void xs9950_init(VI_PIPE ViPipe);
extern void xs9950_exit(VI_PIPE ViPipe);
extern void xs9950_standby(VI_PIPE ViPipe);
extern void xs9950_restart(VI_PIPE ViPipe);
extern int  xs9950_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  xs9950_read_register(VI_PIPE ViPipe, int addr);
extern int  xs9950_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __XS9950_CMOS_EX_H_ */
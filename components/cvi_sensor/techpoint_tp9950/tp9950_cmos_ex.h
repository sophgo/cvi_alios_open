#ifndef __TP9950_CMOS_EX_H_
#define __TP9950_CMOS_EX_H_

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

enum tp9950_linear_regs_e {
	LINEAR_REGS_NUM
};

typedef enum _TP9950_MODE_E {
	TP9950_MODE_NONE,
	TP9950_MODE_720P_25P,
	TP9950_MODE_720P_30P,
	TP9950_MODE_1080P_25P,
	TP9950_MODE_1080P_30P,
	TP9950_MODE_NUM
} TP9950_MODE_E;

typedef struct _TP9950_STATE_S {
	CVI_U32		u8SexpReg;
	CVI_U32		u32Sexp_MAX;
} TP9950_STATE_S;

typedef struct _TP9950_MODE_S {
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
} TP9950_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastTP9950[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunTP9950_BusInfo[];
extern CVI_U16 g_au16TP9950_GainMode[];
extern CVI_U8 tp9950_i2c_addr;
extern const CVI_U32 tp9950_addr_byte;
extern const CVI_U32 tp9950_data_byte;
extern void tp9950_init(VI_PIPE ViPipe);
extern void tp9950_exit(VI_PIPE ViPipe);
extern void tp9950_standby(VI_PIPE ViPipe);
extern void tp9950_restart(VI_PIPE ViPipe);
extern int  tp9950_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  tp9950_read_register(VI_PIPE ViPipe, int addr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __TP9950_CMOS_EX_H_ */

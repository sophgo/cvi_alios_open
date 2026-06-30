#ifndef __TP9951_CMOS_EX_H_
#define __TP9951_CMOS_EX_H_

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

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

enum tp9951_linear_regs_e {
	LINEAR_REGS_NUM
};

typedef enum _TP9951_MODE_E {
	TP9951_MODE_NONE,
	TP9951_MODE_720P_25P,
	TP9951_MODE_720P_30P,
	TP9951_MODE_1080P_25P,
	TP9951_MODE_1080P_30P,
	TP9951_MODE_NUM
} TP9951_MODE_E;

typedef struct _TP9951_STATE_S {
	CVI_U32		u8SexpReg;
	CVI_U32		u32Sexp_MAX;
} TP9951_STATE_S;

typedef struct _TP9951_MODE_S {
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
} TP9951_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastTP9951[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunTP9951_BusInfo[];
extern CVI_U16 g_au16TP9951_GainMode[];
extern CVI_U8 tp9951_i2c_addr;
extern const CVI_U32 tp9951_addr_byte;
extern const CVI_U32 tp9951_data_byte;
extern void tp9951_init(VI_PIPE ViPipe);
extern void tp9951_exit(VI_PIPE ViPipe);
extern int  tp9951_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  tp9951_read_register(VI_PIPE ViPipe, int addr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __TP9951_CMOS_EX_H_ */

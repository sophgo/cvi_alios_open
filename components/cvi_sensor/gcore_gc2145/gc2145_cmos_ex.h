#ifndef __GC2145_CMOS_EX_H_
#define __GC2145_CMOS_EX_H_

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

typedef enum _GC2145_MODE_E {
	GC2145_MODE_1600X1200P12 = 0,
	GC2145_MODE_NUM
} GC2145_SLAVE_MODE_E;

typedef struct _GC2145_MODE_S {
	ISP_WDR_SIZE_S stImg;
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp;
	SNS_ATTR_LARGE_S stAgain;
	SNS_ATTR_LARGE_S stDgain;
	char name[64];
} GC2145_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastGc2145[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunGc2145_BusInfo[];
extern CVI_U8 gc2145_i2c_addr;
extern const CVI_U32 gc2145_addr_byte;
extern const CVI_U32 gc2145_data_byte;
extern void gc2145_init(VI_PIPE ViPipe);
extern void gc2145_exit(VI_PIPE ViPipe);
extern void gc2145_standby(VI_PIPE ViPipe);
extern void gc2145_restart(VI_PIPE ViPipe);
extern int  gc2145_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  gc2145_read_register(VI_PIPE ViPipe, int addr);
extern void gc2145_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  gc2145_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __GC2145_CMOS_EX_H_ */


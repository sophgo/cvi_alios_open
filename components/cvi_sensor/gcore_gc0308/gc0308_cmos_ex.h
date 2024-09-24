#ifndef __GC0308_CMOS_EX_H_
#define __GC0308_CMOS_EX_H_

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

typedef enum _GC0308_MODE_E {
	GC0308_MODE_640X480P30 = 0,
	GC0308_MODE_NUM
} GC0308_SLAVE_MODE_E;

typedef struct _GC0308_MODE_S {
	ISP_WDR_SIZE_S stImg;
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp;
	SNS_ATTR_LARGE_S stAgain;
	SNS_ATTR_LARGE_S stDgain;
	char name[64];
} GC0308_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastGc0308[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunGc0308_BusInfo[];
extern CVI_U8 gc0308_i2c_addr;
extern const CVI_U32 gc0308_addr_byte;
extern const CVI_U32 gc0308_data_byte;
extern void gc0308_init(VI_PIPE ViPipe);
extern void gc0308_exit(VI_PIPE ViPipe);
extern void gc0308_standby(VI_PIPE ViPipe);
extern void gc0308_restart(VI_PIPE ViPipe);
extern int  gc0308_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  gc0308_read_register(VI_PIPE ViPipe, int addr);
extern void gc0308_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  gc0308_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __GC0308_CMOS_EX_H_ */


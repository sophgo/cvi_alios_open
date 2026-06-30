#ifndef __CV2005_CMOS_EX_H_
#define __CV2005_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define syslog(level, fmt, ...)            \
do {                                                   \
	printf(fmt, ##__VA_ARGS__);                \
} while (0)

enum cv2005_linear_regs_e {
	LINEAR_EXP_0,       //0x3062 bit[19:16]
	LINEAR_EXP_1,       //0x3061
	LINEAR_EXP_2,       //0x3060
	LINEAR_GAINENABLE,  //0x3180 bit[7:0]
	LINEAR_AGAIN,       //0x3180 bit[7:0]
	LINEAR_DGAIN_H,     //0x3179 bit[15:8]
	LINEAR_DGAIN_L,     //0x3178 bit[7:0]
	LINEAR_VTS_0,       //0x302A bit[19:16]
	LINEAR_VTS_1,       //0x3029
	LINEAR_VTS_2,       //0x3028
	LINEAR_FLIP_MIRROR, //0x3034
	LINEAR_REGS_NUM
};

typedef enum _CV2005_MODE_E {
	CV2005_MODE_1920X1080P15 = 0,
	CV2005_MODE_1920X1080P12P5 = 1,
	CV2005_MODE_LINEAR_NUM,
	CV2005_MODE_1920X1080P15_WDR = CV2005_MODE_LINEAR_NUM,
	CV2005_MODE_NUM
} CV2005_MODE_E;

typedef struct _CV2005_STATE_S {
	CVI_U32		u32Sexp_MAX;
} CV2005_STATE_S;

typedef struct _CV2005_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	CVI_U32 u32IspResTime;
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} CV2005_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastCV2005[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunCV2005_BusInfo[];
extern ISP_SNS_MIRRORFLIP_TYPE_E g_aeCV2005_MirrorFip[VI_MAX_PIPE_NUM];
extern CVI_U8 cv2005_i2c_addr;
extern const CVI_U32 cv2005_addr_byte;
extern const CVI_U32 cv2005_data_byte;
extern void cv2005_init(VI_PIPE ViPipe);
extern void cv2005_exit(VI_PIPE ViPipe);
extern int cv2005_i2c_exit(VI_PIPE ViPipe);
extern void cv2005_standby(VI_PIPE ViPipe);
extern void cv2005_restart(VI_PIPE ViPipe);
extern int  cv2005_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  cv2005_read_register(VI_PIPE ViPipe, int addr);
extern int  cv2005_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __CV2005_CMOS_EX_H_ */

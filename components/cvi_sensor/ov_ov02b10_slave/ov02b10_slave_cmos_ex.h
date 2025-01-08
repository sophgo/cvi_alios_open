#ifndef __OV02B10_SLAVE_CMOS_EX_H_
#define __OV02B10_SLAVE_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef ARCH_CV182X
#include "cvi_vip_cif_uapi.h"
#else
#include "cif_uapi.h"
#endif
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define syslog(level, fmt, ...)            \
do {                                                   \
	printf(fmt, ##__VA_ARGS__);                \
} while (0)

enum ov02b10_slave_linear_regs_e {
    LINEAR_PAGE_1,
    LINEAR_EXP_0,
    LINEAR_EXP_1,
    LINEAR_AGAIN_0,
    LINEAR_VTS_0,
    LINEAR_VTS_1,
    LINEAR_RESTART_0,
    LINEAR_PAGE_3,
    LINEAR_DGAIN_0,
    LINEAR_REGS_NUM
};

typedef enum _OV02B10_SLAVE_MODE_E {
	OV02B10_SLAVE_MODE_1600X1200P30 = 0,
	OV02B10_SLAVE_MODE_800X600P60,
	OV02B10_SLAVE_MODE_LINEAR_NUM,
	OV02B10_SLAVE_MODE_NUM
} OV02B10_SLAVE_MODE_E;

typedef struct _OV02B10_SLAVE_STATE_S {
	CVI_U32		u32Sexp_MAX;
} OV02B10_SLAVE_STATE_S;

typedef struct _OV02B10_SLAVE_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	CVI_U16 u16L2sOffset;
	CVI_U16 u16TopBoundary;
	CVI_U16 u16BotBoundary;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	CVI_U32 u32L2S_offset;
	CVI_U32 u32IspResTime;
	CVI_U32 u32HdrMargin;
	char name[64];
} OV02B10_SLAVE_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastOv02b10_slave[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunOv02b10_slave_BusInfo[];
extern CVI_U16 g_au16Ov02b10_slave_GainMode[];
extern CVI_U16 g_au16Ov02b10_slave_UseHwSync[VI_MAX_PIPE_NUM];
extern CVI_U8 ov02b10_slave_i2c_addr;
extern const CVI_U32 ov02b10_slave_addr_byte;
extern const CVI_U32 ov02b10_slave_data_byte;
extern void ov02b10_slave_init(VI_PIPE ViPipe);
extern void ov02b10_slave_exit(VI_PIPE ViPipe);
extern void ov02b10_slave_standby(VI_PIPE ViPipe);
extern void ov02b10_slave_restart(VI_PIPE ViPipe);
extern int  ov02b10_slave_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  ov02b10_slave_read_register(VI_PIPE ViPipe, int addr);
extern void ov02b10_slave_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int ov02b10_slave_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __OV02B10_SLAVE_CMOS_EX_H_ */


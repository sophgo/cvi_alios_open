#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "cif_uapi.h"
#include "cvi_ae.h"
#include "cvi_ae_comm.h"
#include "cvi_awb.h"
#include "cvi_awb_comm.h"
#include "cvi_comm_sns.h"
#include "cvi_comm_video.h"
#include "cvi_debug.h"
#include "cvi_isp.h"
#include "cvi_sns_ctrl.h"
#include "cvi_type.h"
#include "sp2509_cmos_ex.h"
#include "sp2509_cmos_param.h"

#define DIV_0_TO_1(a) ((0 == (a)) ? 1 : (a))
#define DIV_0_TO_1_FLOAT(a) ((((a) < 1E-10) && ((a) > -1E-10)) ? 1 : (a))

#define SP2509_CHIP_ID 0x2509
#define SP2509_I2C_ADDR 0x3d

// Judge Macro
#define IS_SP2509_I2C_ADDR_VALID(addr) ((addr) == SP2509_I2C_ADDR)
#define IS_SP2509_RES_1200P(w, h) ((w) == 1600 && (h) == 1200)
#define IS_SP2509_RES_600P(w, h) ((w) == 800 && (h) == 600)

/****************************************************************************
 * global variables                                                         *
 ***************************************************************************/
ISP_SNS_STATE_S *g_pastSp2509[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define SP2509_SENSOR_GET_CTX(dev, pstCtx) (pstCtx = g_pastSp2509[dev])
#define SP2509_SENSOR_SET_CTX(dev, pstCtx) (g_pastSp2509[dev] = pstCtx)
#define SP2509_SENSOR_RESET_CTX(dev) (g_pastSp2509[dev] = CVI_NULL)

ISP_SNS_COMMBUS_U g_aunSp2509_BusInfo[VI_MAX_PIPE_NUM] = {
	[0] = {.s8I2cDev = 3}, [1 ... VI_MAX_PIPE_NUM - 1] = {.s8I2cDev = -1}};

ISP_SNS_MIRRORFLIP_TYPE_E g_aeSp2509_MirrorFip[VI_MAX_PIPE_NUM] = {ISP_SNS_NORMAL};

CVI_U16 g_au16Sp2509_GainMode[VI_MAX_PIPE_NUM] = {0};
CVI_U16 g_au16Sp2509_UseHwSync[VI_MAX_PIPE_NUM] = {0};

/****************************************************************************
 * local variables and functions                                            *
 ***************************************************************************/
static ISP_FSWDR_MODE_E genFSWDRMode[VI_MAX_PIPE_NUM] = {[0 ... VI_MAX_PIPE_NUM - 1] =
															 ISP_FSWDR_NORMAL_MODE};

static CVI_U32 gu32MaxTimeGetCnt[VI_MAX_PIPE_NUM] = {0};
static CVI_U32 g_au32InitExposure[VI_MAX_PIPE_NUM] = {0};
static CVI_U32 g_au32LinesPer500ms[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16InitWBGain[VI_MAX_PIPE_NUM][3] = {{0}};
static CVI_U16 g_au16SampleRgain[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16SampleBgain[VI_MAX_PIPE_NUM] = {0};
static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg);

/*****Sp2509 Lines Range*****/
#define SP2509_FULL_LINES_MAX (0x3fff)

// Page Selection
#define SP2509_PAGE_FLG_ADDR_L 0xfd  // bit[3:0]	def_val:0x00
/*
 * 0x000: PAGE_0	system register
 * 0x001: PAGE_1	sensor register
 * 0x010: PAGE_2	ISP register
 * */

/*****Sp2509 Register Address*****/

// Sensor Register
// Address:P1
#define SP2509_EXP_ADDR_H 0x03  // bit[7:0]	def_val:0x01
#define SP2509_EXP_ADDR_L 0x04  // bit[7:0]	def_val:0x68

#define SP2509_VBLANK_ADDR_H 0x05  // bit[7:0]	def_val:0x00
#define SP2509_VBLANK_ADDR_L 0x06  // bit[7:0]	def_val:0x00

#define SP2509_HBLANK_ADDR_H 0x09  // bit[7:0]	def_val:0x00
#define SP2509_HBLANK_ADDR_L 0x0a  // bit[7:0]	def_val:0x00

#define SP2509_VTS_ADDR_H 0x4e  // bit[7:0]	def_val:0x04
#define SP2509_VTS_ADDR_L 0x4f  // bit[7:0]	def_val:0xf2

#define SP2509_HTS_ADDR_H 0x8c  // bit[7:0]	def_val:0x04
#define SP2509_HTS_ADDR_L 0x8d  // bit[7:0]	def_val:0x02

#define SP2509_AGAIN_ADDR_H 0x37  // bit[0]	def_val:0x00
#define SP2509_AGAIN_ADDR_L 0x24  // bit[7:0]	def_val:0x24
/*
 * AGAIN_ADDR_L should be set firstly
 */

#define SP2509_EXP_LIMIT_ADDR_H 0x42  // bit[7:0]	def_val:0x00
#define SP2509_EXP_LIMIT_ADDR_L 0x43  // bit[7:0]	def_val:0x00

#define SP2509_MIRROR_FLIP_ADDR 0x3f  // bit[1:0]	def_val:0x00
/*
 * bit[1]	vertical upside/down
 * bit[0]	horizontal mirror
 */
#define SP2509_TRIGGER_ADDR 0x01  // bit[1]
/*
 * 1: enable
 */

static CVI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft) {
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	SP2509_MODE_E u8ImgMode;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	SP2509_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FlickerFreq = 50 * 256;
	pstAeSnsDft->u32FullLinesMax = SP2509_FULL_LINES_MAX;
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * 30);

	pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
	pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
	pstAeSnsDft->stIntTimeAccu.f32Offset = 0;
	pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stAgainAccu.f32Accuracy = 1;
	pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stDgainAccu.f32Accuracy = 1;

	pstAeSnsDft->u32ISPDgainShift = 8;
	pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
	pstAeSnsDft->u32MaxISPDgainTarget = 2 << pstAeSnsDft->u32ISPDgainShift;

	if (g_au32LinesPer500ms[ViPipe] == 0)
		pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * 30 / 2;
	else
		pstAeSnsDft->u32LinesPer500ms = g_au32LinesPer500ms[ViPipe];
	switch (pstSnsState->enWDRMode) {
	default:
	case WDR_MODE_NONE: /*linear mode*/
		u8ImgMode = pstSnsState->u8ImgMode;
		if (u8ImgMode >= SP2509_MODE_LINEAR_NUM) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport u8ImgMode: %d\n", pstSnsState->enWDRMode);
			return CVI_FAILURE;
		}
		pstAeSnsDft->f32Fps = g_astSp2509_mode[u8ImgMode].f32MaxFps;
		pstAeSnsDft->f32MinFps = g_astSp2509_mode[u8ImgMode].f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xd;
		pstAeSnsDft->au8HistThresh[1] = 0x28;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxAgain = g_astSp2509_mode[u8ImgMode].stAgain[0].u32Max;
		pstAeSnsDft->u32MinAgain = 1024;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = g_astSp2509_mode[u8ImgMode].stDgain[0].u32Max;
		pstAeSnsDft->u32MinDgain = 1024;
		pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
		pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

		pstAeSnsDft->u8AeCompensation = 40;
		pstAeSnsDft->u32InitAESpeed = 64;
		pstAeSnsDft->u32InitAETolerance = 5;
		pstAeSnsDft->u32AEResponseFrame = 4;
		pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe]
										   ? g_au32InitExposure[ViPipe]
										   : g_astSp2509_mode[u8ImgMode].stExp[0].u16Def;

		pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 8;
		pstAeSnsDft->u32MinIntTime = g_astSp2509_mode[u8ImgMode].stExp[0].u16Min;
		pstAeSnsDft->u32MaxIntTimeTarget = 65535;
		pstAeSnsDft->u32MinIntTimeTarget = 1;

		pstAeSnsDft->u32IntAgain = 2048;
		pstAeSnsDft->u32IntDgain = 1024;
		pstAeSnsDft->u32IntExposure = 0x168;

		break;
	}

	return CVI_SUCCESS;
}

/* the function of sensor set fps */
static CVI_S32 cmos_fps_set(VI_PIPE ViPipe, CVI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft) {
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	CVI_U32 u32VMAX;
	CVI_FLOAT f32MaxFps = 0;
	CVI_FLOAT f32MinFps = 0;
	CVI_U32 u32Vts = 0;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	SP2509_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32Vts = g_astSp2509_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	f32MaxFps = g_astSp2509_mode[pstSnsState->u8ImgMode].f32MaxFps;
	f32MinFps = g_astSp2509_mode[pstSnsState->u8ImgMode].f32MinFps;

	if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
		u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport Fps: %f\n", f32Fps);
		return CVI_FAILURE;
	}

	u32VMAX = (u32VMAX > SP2509_FULL_LINES_MAX) ? SP2509_FULL_LINES_MAX : u32VMAX;

	if (pstSnsState->u8ImgMode == SP2509_MODE_800X600P30) {
		u32VMAX = ((u32VMAX >> 1) << 1);
	}

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstSnsRegsInfo->astI2cData[LINEAR_VTS_H].u32Data = ((u32VMAX & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[LINEAR_VTS_L].u32Data = (u32VMAX & 0xFF);
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport WDRMode: %d\n", pstSnsState->enWDRMode);
		return CVI_FAILURE;
	}

	pstSnsState->u32FLStd = u32VMAX;

	pstAeSnsDft->f32Fps = f32Fps;
	pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * f32Fps / 2;
	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 8;
	pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * DIV_0_TO_1_FLOAT(f32Fps));

	return CVI_SUCCESS;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static CVI_S32 cmos_inttime_update(VI_PIPE ViPipe, CVI_U32 *u32IntTime) {
	static CVI_U32 u32PrevIntTimeH;
	static CVI_U32 u32PrevIntTimeL;
	static CVI_U8 u8FrameCnt;

	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U32 expLine = u32IntTime[0];

	SP2509_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(u32IntTime);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	if (pstSnsState->u8ImgMode == SP2509_MODE_800X600P30) {
		expLine = ((expLine >> 1) << 1);
		u32IntTime[0] = expLine;
	}

	pstSnsRegsInfo->astI2cData[LINEAR_EXP_H].u32Data = ((expLine >> 8) & 0x3F);
	pstSnsRegsInfo->astI2cData[LINEAR_EXP_L].u32Data = (expLine & 0xFF);
	pstSnsRegsInfo->astI2cData[LINEAR_TRIGGER].u32Data = 0x1;

	static CVI_U32 count_exp = 0;
	if (count_exp % 20 == 0) {
		// printf("change exp and trigger, exp_h val = 0x%x, exp_l val = 0x%x\n",
		// pstSnsRegsInfo->astI2cData[LINEAR_EXP_H].u32Data,
		// pstSnsRegsInfo->astI2cData[LINEAR_EXP_L].u32Data);
	}
	++count_exp;

	if (u8FrameCnt < 3) {
		if (u32PrevIntTimeH != pstSnsRegsInfo->astI2cData[LINEAR_EXP_H].u32Data) {
			sp2509_write_register(ViPipe, SP2509_EXP_ADDR_H,
								  pstSnsRegsInfo->astI2cData[LINEAR_EXP_H].u32Data);

			u32PrevIntTimeH = pstSnsRegsInfo->astI2cData[LINEAR_EXP_H].u32Data;
		}

		if (u32PrevIntTimeL != pstSnsRegsInfo->astI2cData[LINEAR_EXP_L].u32Data) {
			sp2509_write_register(ViPipe, SP2509_EXP_ADDR_L,
								  pstSnsRegsInfo->astI2cData[LINEAR_EXP_L].u32Data);

			u32PrevIntTimeL = pstSnsRegsInfo->astI2cData[LINEAR_EXP_L].u32Data;
		}
		u8FrameCnt++;
	}

	return CVI_SUCCESS;
}

static CVI_U32 regValTable[] = {
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x21, 0x23, 0x25, 0x27, 0x29, 0x2b, 0x2d, 0x2f, 0x31, 0x33, 0x35, 0x37, 0x39, 0x3b, 0x3d, 0x3f,
	0x43, 0x47, 0x4b, 0x4f, 0x53, 0x57, 0x5b, 0x5f, 0x63, 0x67, 0x6b, 0x6f, 0x73, 0x77, 0x7b, 0x7f,
	0x87, 0x8f, 0x97, 0x9f, 0xa7, 0xaf, 0xb7, 0xbf, 0xc7, 0xcf, 0xd7, 0xdf, 0xe7, 0xef, 0xf7, 0xff,
};

static CVI_U32 gain_table[] = {
	1024,  1088,  1152,  1216,  1280,  1344,  1408,  1472,  1536,  1600,  1664,  1728,  1792,
	1856,  1920,  1984,  2048,  2176,  2304,  2432,  2560,  2688,  2816,  2944,  3072,  3200,
	3328,  3456,  3584,  3712,  3840,  3968,  4096,  4352,  4608,  4864,  5120,  5376,  5632,
	5888,  6144,  6400,  6656,  6912,  7168,  7424,  7680,  7936,  8192,  8704,  9216,  9728,
	10240, 10752, 11264, 11776, 12288, 12800, 13312, 13824, 14336, 14848, 15360, 15872,
};

static CVI_U32 dgainRegValTable[] = {
	1024,
};

static CVI_S32 cmos_again_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32AgainLin, CVI_U32 *pu32AgainDb) {
	int i;
	UNUSED(ViPipe);
	CMOS_CHECK_POINTER(pu32AgainLin);
	CMOS_CHECK_POINTER(pu32AgainDb);
	int total = sizeof(gain_table) / sizeof(gain_table[0]);

	if (*pu32AgainLin >= gain_table[total - 1]) {
		*pu32AgainDb = total - 1;
		*pu32AgainLin = gain_table[total - 1];
		return CVI_SUCCESS;
	}

	for (i = 0; i < total; i++) {
		if (*pu32AgainLin < gain_table[i]) {
			*pu32AgainDb = i - 1;
			break;
		}
	}
	*pu32AgainLin = gain_table[i - 1];
	return CVI_SUCCESS;
}

static CVI_S32 cmos_dgain_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32DgainLin, CVI_U32 *pu32DgainDb) {
	int i;
	CMOS_CHECK_POINTER(pu32DgainLin);
	CMOS_CHECK_POINTER(pu32DgainDb);
	UNUSED(ViPipe);

	int total = sizeof(dgainRegValTable) / sizeof(dgainRegValTable[0]);

	if (*pu32DgainLin >= dgainRegValTable[total - 1]) {
		*pu32DgainDb = total - 1;
		*pu32DgainLin = dgainRegValTable[total - 1];
		return CVI_SUCCESS;
	}

	for (i = 0; i < total; i++) {
		if (*pu32DgainLin < dgainRegValTable[i]) {
			*pu32DgainDb = i - 1;
			break;
		}
	}
	*pu32DgainLin = dgainRegValTable[i - 1];
	return CVI_SUCCESS;
}

static CVI_S32 cmos_gains_update(VI_PIPE ViPipe, CVI_U32 *pu32Again, CVI_U32 *pu32Dgain) {
	static CVI_U32 u32PreAgain;
	static CVI_U8 u8FrameCnt;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U32 u32Again;
	CVI_U32 u32Dgain;

	SP2509_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pu32Again);
	CMOS_CHECK_POINTER(pu32Dgain);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	/* only surpport linear mode */
	u32Again = pu32Again[0];
	u32Dgain = pu32Dgain[0];

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_L].u32Data = regValTable[u32Again];
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_H].u32Data = 0;
		pstSnsRegsInfo->astI2cData[LINEAR_TRIGGER].u32Data = 0x1;

		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_H].u32Data = dgainRegValTable[u32Dgain] >> 8;
		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_L].u32Data = dgainRegValTable[u32Dgain] & 0xff;
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport WDRMode: %d\n", pstSnsState->enWDRMode);
		return CVI_FAILURE;
	}
	if (u8FrameCnt < 3) {
		if (u32PreAgain != pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_L].u32Data) {
			sp2509_write_register(ViPipe, SP2509_AGAIN_ADDR_L,
								  pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_L].u32Data);

			u32PreAgain = pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_L].u32Data;
		}
		u8FrameCnt++;
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_inttime_max(VI_PIPE ViPipe, CVI_U16 u16ManRatioEnable, CVI_U32 *au32Ratio,
									CVI_U32 *au32IntTimeMax, CVI_U32 *au32IntTimeMin,
									CVI_U32 *pu32LFMaxIntTime) {
	UNUSED(ViPipe);
	UNUSED(u16ManRatioEnable);
	UNUSED(au32Ratio);
	UNUSED(au32IntTimeMax);
	UNUSED(au32IntTimeMin);
	UNUSED(pu32LFMaxIntTime);

	CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport WDRMode\n");
	return CVI_SUCCESS;
}

/* Only used in LINE_WDR mode */
static CVI_S32 cmos_ae_fswdr_attr_set(VI_PIPE ViPipe, AE_FSWDR_ATTR_S *pstAeFSWDRAttr) {
	CMOS_CHECK_POINTER(pstAeFSWDRAttr);

	genFSWDRMode[ViPipe] = pstAeFSWDRAttr->enFSWDRMode;
	gu32MaxTimeGetCnt[ViPipe] = 0;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs) {
	CMOS_CHECK_POINTER(pstExpFuncs);

	memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));
	pstExpFuncs->pfn_cmos_get_ae_default = cmos_get_ae_default;
	pstExpFuncs->pfn_cmos_fps_set = cmos_fps_set;
	pstExpFuncs->pfn_cmos_inttime_update = cmos_inttime_update;
	pstExpFuncs->pfn_cmos_gains_update = cmos_gains_update;
	pstExpFuncs->pfn_cmos_again_calc_table = cmos_again_calc_table;
	pstExpFuncs->pfn_cmos_dgain_calc_table = cmos_dgain_calc_table;
	pstExpFuncs->pfn_cmos_get_inttime_max = cmos_get_inttime_max;
	pstExpFuncs->pfn_cmos_ae_fswdr_attr_set = cmos_ae_fswdr_attr_set;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_awb_default(VI_PIPE ViPipe, AWB_SENSOR_DEFAULT_S *pstAwbSnsDft) {
	CMOS_CHECK_POINTER(pstAwbSnsDft);
	UNUSED(ViPipe);
	memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

	pstAwbSnsDft->u16InitGgain = 1024;
	pstAwbSnsDft->u8AWBRunInterval = 1;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs) {
	CMOS_CHECK_POINTER(pstExpFuncs);

	memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

	pstExpFuncs->pfn_cmos_get_awb_default = cmos_get_awb_default;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_isp_default(VI_PIPE ViPipe, ISP_CMOS_DEFAULT_S *pstDef) {
	UNUSED(ViPipe);
	memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_blc_default(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *pstBlc) {
	CMOS_CHECK_POINTER(pstBlc);
	UNUSED(ViPipe);
	memset(pstBlc, 0, sizeof(ISP_CMOS_BLACK_LEVEL_S));

	memcpy(pstBlc, &g_stIspBlcCalibratio, sizeof(ISP_CMOS_BLACK_LEVEL_S));

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg) {
	const SP2509_MODE_S *pstMode = CVI_NULL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SP2509_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstMode = &g_astSp2509_mode[pstSnsState->u8ImgMode];

	if (pstSnsState->enWDRMode != WDR_MODE_NONE) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport WDRMode: %d\n", pstSnsState->enWDRMode);
	} else {
		pstIspCfg->frm_num = 1;
		memcpy(&pstIspCfg->img_size[0], &pstMode->astImg[0], sizeof(ISP_WDR_SIZE_S));
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_wdr_mode(VI_PIPE ViPipe, CVI_U8 u8Mode) {
	UNUSED(ViPipe);
	UNUSED(u8Mode);
	CVI_TRACE_SNS(CVI_DBG_INFO, "Unsupport sensor mode!\n");
	return CVI_SUCCESS;
}

static CVI_U32 sensor_cmp_wdr_size(ISP_SNS_ISP_INFO_S *pstWdr1, ISP_SNS_ISP_INFO_S *pstWdr2) {
	CVI_U32 i;

	if (pstWdr1->frm_num != pstWdr2->frm_num)
		goto _mismatch;
	for (i = 0; i < 2; i++) {
		if (pstWdr1->img_size[i].stSnsSize.u32Width != pstWdr2->img_size[i].stSnsSize.u32Width)
			goto _mismatch;
		if (pstWdr1->img_size[i].stSnsSize.u32Height != pstWdr2->img_size[i].stSnsSize.u32Height)
			goto _mismatch;
		if (pstWdr1->img_size[i].stWndRect.s32X != pstWdr2->img_size[i].stWndRect.s32X)
			goto _mismatch;
		if (pstWdr1->img_size[i].stWndRect.s32Y != pstWdr2->img_size[i].stWndRect.s32Y)
			goto _mismatch;
		if (pstWdr1->img_size[i].stWndRect.u32Width != pstWdr2->img_size[i].stWndRect.u32Width)
			goto _mismatch;
		if (pstWdr1->img_size[i].stWndRect.u32Height != pstWdr2->img_size[i].stWndRect.u32Height)
			goto _mismatch;
	}

	return 0;
_mismatch:
	return 1;
}

static CVI_S32 cmos_get_sns_regs_info(VI_PIPE ViPipe, ISP_SNS_SYNC_INFO_S *pstSnsSyncInfo) {
	CVI_U32 i;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	ISP_SNS_SYNC_INFO_S *pstCfg0 = CVI_NULL;
	ISP_SNS_SYNC_INFO_S *pstCfg1 = CVI_NULL;
	ISP_I2C_DATA_S *pstI2c_data = CVI_NULL;

	CMOS_CHECK_POINTER(pstSnsSyncInfo);
	SP2509_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstSnsRegsInfo = &pstSnsSyncInfo->snsCfg;
	pstCfg0 = &pstSnsState->astSyncInfo[0];
	pstCfg1 = &pstSnsState->astSyncInfo[1];
	pstI2c_data = pstCfg0->snsCfg.astI2cData;

	if ((pstSnsState->bSyncInit == CVI_FALSE) || (pstSnsRegsInfo->bConfig == CVI_FALSE)) {
		pstCfg0->snsCfg.enSnsType = SNS_I2C_TYPE;
		pstCfg0->snsCfg.unComBus.s8I2cDev = g_aunSp2509_BusInfo[ViPipe].s8I2cDev;
		pstCfg0->snsCfg.u8Cfg2ValidDelayMax = 0;
		pstCfg0->snsCfg.use_snsr_sram = CVI_TRUE;
		pstCfg0->snsCfg.u32RegNum = LINEAR_REGS_NUM;

		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			pstI2c_data[i].bUpdate = CVI_TRUE;
			pstI2c_data[i].u8DevAddr = sp2509_i2c_addr;
			pstI2c_data[i].u32AddrByteNum = sp2509_addr_byte;
			pstI2c_data[i].u32DataByteNum = sp2509_data_byte;
		}

		pstI2c_data[LINEAR_PAGE_1].u32RegAddr = SP2509_PAGE_FLG_ADDR_L;
		pstI2c_data[LINEAR_PAGE_1].u32Data = 0x1;
		pstI2c_data[LINEAR_EXP_H].u32RegAddr = SP2509_EXP_ADDR_H;
		pstI2c_data[LINEAR_EXP_L].u32RegAddr = SP2509_EXP_ADDR_L;
		pstI2c_data[LINEAR_AGAIN_H].u32RegAddr = SP2509_AGAIN_ADDR_H;
		pstI2c_data[LINEAR_AGAIN_L].u32RegAddr = SP2509_AGAIN_ADDR_L;
		pstI2c_data[LINEAR_TRIGGER].u32RegAddr = SP2509_TRIGGER_ADDR;
		pstI2c_data[LINEAR_VTS_H].u32RegAddr = SP2509_VTS_ADDR_H;
		pstI2c_data[LINEAR_VTS_L].u32RegAddr = SP2509_VTS_ADDR_L;
		pstI2c_data[LINEAR_MIRROR_FLIP].u32RegAddr = SP2509_MIRROR_FLIP_ADDR;
		pstSnsState->bSyncInit = CVI_TRUE;
		pstCfg0->snsCfg.need_update = CVI_TRUE;
		/* recalcualte WDR size */
		cmos_get_wdr_size(ViPipe, &pstCfg0->ispCfg);
		pstCfg0->ispCfg.need_update = CVI_TRUE;
	} else {
		CVI_U32 gainsUpdate = 0, shutterUpdate = 0, vtsUpdate = 0;

		pstCfg0->snsCfg.need_update = CVI_FALSE;
		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			if (pstCfg0->snsCfg.astI2cData[i].u32Data == pstCfg1->snsCfg.astI2cData[i].u32Data) {
				pstCfg0->snsCfg.astI2cData[i].bUpdate = CVI_FALSE;
			} else {
				if ((i == LINEAR_AGAIN_H) || (i == LINEAR_AGAIN_L) || (i == LINEAR_DGAIN_H) ||
					(i == LINEAR_DGAIN_L))
					gainsUpdate = 1;

				if (i <= LINEAR_EXP_L && i >= LINEAR_EXP_H)
					shutterUpdate = 1;

				if ((i >= LINEAR_VTS_H) && (i <= LINEAR_VTS_L))
					vtsUpdate = 1;

				pstCfg0->snsCfg.astI2cData[i].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.need_update = CVI_TRUE;
			}
		}

		if (gainsUpdate) {
			pstCfg0->snsCfg.astI2cData[LINEAR_AGAIN_H].bUpdate = CVI_TRUE;
			pstCfg0->snsCfg.astI2cData[LINEAR_AGAIN_L].bUpdate = CVI_TRUE;
			pstCfg0->snsCfg.astI2cData[LINEAR_DGAIN_H].bUpdate = CVI_TRUE;
			pstCfg0->snsCfg.astI2cData[LINEAR_DGAIN_L].bUpdate = CVI_TRUE;
			pstCfg0->snsCfg.astI2cData[LINEAR_TRIGGER].bUpdate = CVI_TRUE;
		}
		if (shutterUpdate) {
			pstCfg0->snsCfg.astI2cData[LINEAR_EXP_H].bUpdate = CVI_TRUE;
			pstCfg0->snsCfg.astI2cData[LINEAR_EXP_L].bUpdate = CVI_TRUE;
			pstCfg0->snsCfg.astI2cData[LINEAR_TRIGGER].bUpdate = CVI_TRUE;
		}
		if (vtsUpdate) {
			pstCfg0->snsCfg.astI2cData[LINEAR_VTS_H].bUpdate = CVI_TRUE;
			pstCfg0->snsCfg.astI2cData[LINEAR_VTS_L].bUpdate = CVI_TRUE;
			pstCfg0->snsCfg.astI2cData[LINEAR_TRIGGER].bUpdate = CVI_TRUE;
		}
		/* check update isp crop or not */
		pstCfg0->ispCfg.need_update =
			(sensor_cmp_wdr_size(&pstCfg0->ispCfg, &pstCfg1->ispCfg) ? CVI_TRUE : CVI_FALSE);
	}

	pstSnsRegsInfo->bConfig = CVI_FALSE;
	memcpy(pstSnsSyncInfo, &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));
	memcpy(&pstSnsState->astSyncInfo[1], &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));
	pstSnsState->au32FL[1] = pstSnsState->au32FL[0];
	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_image_mode(VI_PIPE ViPipe,
								   ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode) {
	CVI_U8 u8SensorImageMode = 0;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstSensorImageMode);
	SP2509_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8SensorImageMode = pstSnsState->u8ImgMode;
	pstSnsState->bSyncInit = CVI_FALSE;

	if (pstSensorImageMode->f32Fps <= 30) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (IS_SP2509_RES_1200P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = SP2509_MODE_1600X1200P30;
			else if (IS_SP2509_RES_600P(pstSensorImageMode->u16Width,
										pstSensorImageMode->u16Height))
				u8SensorImageMode = SP2509_MODE_800X600P30;
			else {
				CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
							  pstSensorImageMode->u16Width, pstSensorImageMode->u16Height,
							  pstSensorImageMode->f32Fps, pstSnsState->enWDRMode);
				return CVI_FAILURE;
			}
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
						  pstSensorImageMode->u16Width, pstSensorImageMode->u16Height,
						  pstSensorImageMode->f32Fps, pstSnsState->enWDRMode);
			return CVI_FAILURE;
		}
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support this Fps:%f\n", pstSensorImageMode->f32Fps);
		return CVI_FAILURE;
	}

	if ((pstSnsState->bInit == CVI_TRUE) && (u8SensorImageMode == pstSnsState->u8ImgMode)) {
		/* Don't need to switch SensorImageMode */
		return CVI_FAILURE;
	}

	pstSnsState->u8ImgMode = u8SensorImageMode;

	return CVI_SUCCESS;
}

static CVI_VOID sensor_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip) {
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U8 value;

	SP2509_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	/* Apply the setting on the fly  */
	if (pstSnsState->bInit == CVI_TRUE && g_aeSp2509_MirrorFip[ViPipe] != eSnsMirrorFlip) {
		switch (eSnsMirrorFlip) {
		case ISP_SNS_NORMAL:
			value = 0;
			break;
		case ISP_SNS_MIRROR:
			value = 0x01;
			break;
		case ISP_SNS_FLIP:
			value = 0x02;
			break;
		case ISP_SNS_MIRROR_FLIP:
			value = 0x03;
			break;
		default:
			return;
		}

		pstSnsRegsInfo->astI2cData[LINEAR_MIRROR_FLIP].u32Data = value;
		pstSnsRegsInfo->astI2cData[LINEAR_TRIGGER].u32Data = value;
		pstSnsRegsInfo->astI2cData[LINEAR_MIRROR_FLIP].bUpdate = CVI_TRUE;
		pstSnsRegsInfo->astI2cData[LINEAR_TRIGGER].bUpdate = CVI_TRUE;

		g_aeSp2509_MirrorFip[ViPipe] = eSnsMirrorFlip;
	}
}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe) {
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SP2509_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsState->bInit = CVI_FALSE;
	pstSnsState->bSyncInit = CVI_FALSE;
	pstSnsState->u8ImgMode = SP2509_MODE_1600X1200P30;
	pstSnsState->enWDRMode = WDR_MODE_NONE;
	pstSnsState->u32FLStd = g_astSp2509_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[0] = g_astSp2509_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[1] = g_astSp2509_mode[pstSnsState->u8ImgMode].u32VtsDef;

	memset(&pstSnsState->astSyncInfo[0], 0, sizeof(ISP_SNS_SYNC_INFO_S));
	memset(&pstSnsState->astSyncInfo[1], 0, sizeof(ISP_SNS_SYNC_INFO_S));
}

static CVI_S32 sensor_rx_attr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *pstRxAttr) {
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SP2509_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pstRxAttr);

	memcpy(pstRxAttr, &sp2509_rx_attr, sizeof(*pstRxAttr));

	pstRxAttr->img_size.width =
		g_astSp2509_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Width;
	pstRxAttr->img_size.height =
		g_astSp2509_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Height;
	if (pstSnsState->enWDRMode == WDR_MODE_NONE)
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_patch_rx_attr(RX_INIT_ATTR_S *pstRxInitAttr) {
	SNS_COMBO_DEV_ATTR_S *pstRxAttr = &sp2509_rx_attr;
	int i;

	CMOS_CHECK_POINTER(pstRxInitAttr);

	if (pstRxInitAttr->stMclkAttr.bMclkEn)
		pstRxAttr->mclk.cam = pstRxInitAttr->stMclkAttr.u8Mclk;

	if (pstRxInitAttr->MipiDev >= 2)
		return CVI_SUCCESS;
	pstRxAttr->devno = pstRxInitAttr->MipiDev;

	if (pstRxAttr->input_mode == INPUT_MODE_MIPI) {
		struct mipi_dev_attr_s *attr = &pstRxAttr->mipi_attr;

		for (i = 0; i < MIPI_LANE_NUM + 1; i++) {
			attr->lane_id[i] = pstRxInitAttr->as16LaneId[i];
			attr->pn_swap[i] = pstRxInitAttr->as8PNSwap[i];
		}
	} else {
		struct lvds_dev_attr_s *attr = &pstRxAttr->lvds_attr;

		for (i = 0; i < MIPI_LANE_NUM + 1; i++) {
			attr->lane_id[i] = pstRxInitAttr->as16LaneId[i];
			attr->pn_swap[i] = pstRxInitAttr->as8PNSwap[i];
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc) {
	CMOS_CHECK_POINTER(pstSensorExpFunc);

	memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

	pstSensorExpFunc->pfn_cmos_sensor_init = sp2509_init;
	pstSensorExpFunc->pfn_cmos_sensor_exit = sp2509_exit;
	pstSensorExpFunc->pfn_cmos_sensor_global_init = sensor_global_init;
	pstSensorExpFunc->pfn_cmos_set_image_mode = cmos_set_image_mode;
	pstSensorExpFunc->pfn_cmos_set_wdr_mode = cmos_set_wdr_mode;
	pstSensorExpFunc->pfn_cmos_get_isp_default = cmos_get_isp_default;
	pstSensorExpFunc->pfn_cmos_get_isp_black_level = cmos_get_blc_default;
	pstSensorExpFunc->pfn_cmos_get_sns_reg_info = cmos_get_sns_regs_info;

	return CVI_SUCCESS;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
static CVI_VOID sensor_patch_i2c_addr(CVI_S32 s32I2cAddr) {
	if (IS_SP2509_I2C_ADDR_VALID(s32I2cAddr))
		sp2509_i2c_addr = s32I2cAddr;
}

static CVI_S32 sp2509_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo) {
	g_aunSp2509_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe) {
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	SP2509_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

	if (pastSnsStateCtx == CVI_NULL) {
		pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
		if (pastSnsStateCtx == CVI_NULL) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
			return -ENOMEM;
		}
	}

	memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

	SP2509_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

	return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe) {
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	SP2509_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
	SENSOR_FREE(pastSnsStateCtx);
	SP2509_SENSOR_RESET_CTX(ViPipe);
}

static CVI_S32 sensor_register_callback(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib) {
	CVI_S32 s32Ret;
	ISP_SENSOR_REGISTER_S stIspRegister;
	AE_SENSOR_REGISTER_S stAeRegister;
	AWB_SENSOR_REGISTER_S stAwbRegister;
	ISP_SNS_ATTR_INFO_S stSnsAttrInfo;

	CMOS_CHECK_POINTER(pstAeLib);
	CMOS_CHECK_POINTER(pstAwbLib);

	s32Ret = sensor_ctx_init(ViPipe);

	if (s32Ret != CVI_SUCCESS)
		return CVI_FAILURE;

	stSnsAttrInfo.eSensorId = SP2509_CHIP_ID;

	s32Ret = cmos_init_sensor_exp_function(&stIspRegister.stSnsExp);
	s32Ret |= CVI_ISP_SensorRegCallBack(ViPipe, &stSnsAttrInfo, &stIspRegister);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function failed!\n");
		return s32Ret;
	}

	s32Ret = cmos_init_ae_exp_function(&stAeRegister.stAeExp);
	s32Ret |= CVI_AE_SensorRegCallBack(ViPipe, pstAeLib, &stSnsAttrInfo, &stAeRegister);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret = cmos_init_awb_exp_function(&stAwbRegister.stAwbExp);
	s32Ret |= CVI_AWB_SensorRegCallBack(ViPipe, pstAwbLib, &stSnsAttrInfo, &stAwbRegister);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function to awb lib failed!\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 sensor_unregister_callback(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib,
										  ALG_LIB_S *pstAwbLib) {
	CVI_S32 s32Ret = CVI_SUCCESS;

	CMOS_CHECK_POINTER(pstAeLib);
	CMOS_CHECK_POINTER(pstAwbLib);

	s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, SP2509_CHIP_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, SP2509_CHIP_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, SP2509_CHIP_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to awb lib failed!\n");
		return s32Ret;
	}

	sensor_ctx_exit(ViPipe);

	return s32Ret;
}

static CVI_S32 sensor_set_init(VI_PIPE ViPipe, ISP_INIT_ATTR_S *pstInitAttr) {
	CMOS_CHECK_POINTER(pstInitAttr);

	g_au32InitExposure[ViPipe] = pstInitAttr->u32Exposure;
	g_au32LinesPer500ms[ViPipe] = pstInitAttr->u32LinesPer500ms;
	g_au16InitWBGain[ViPipe][0] = pstInitAttr->u16WBRgain;
	g_au16InitWBGain[ViPipe][1] = pstInitAttr->u16WBGgain;
	g_au16InitWBGain[ViPipe][2] = pstInitAttr->u16WBBgain;
	g_au16SampleRgain[ViPipe] = pstInitAttr->u16SampleRgain;
	g_au16SampleBgain[ViPipe] = pstInitAttr->u16SampleBgain;
	g_au16Sp2509_GainMode[ViPipe] = pstInitAttr->enGainMode;
	g_au16Sp2509_UseHwSync[ViPipe] = pstInitAttr->u16UseHwSync;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_probe(VI_PIPE ViPipe) { return sp2509_probe(ViPipe); }

ISP_SNS_OBJ_S stSnsSp2509_Obj = {
	.pfnRegisterCallback = sensor_register_callback,
	.pfnUnRegisterCallback = sensor_unregister_callback,
	.pfnStandby = sp2509_standby,
	.pfnRestart = sp2509_restart,
	.pfnWriteReg = sp2509_write_register,
	.pfnReadReg = sp2509_read_register,
	.pfnSetBusInfo = sp2509_set_bus_info,
	.pfnSetInit = sensor_set_init,
	.pfnMirrorFlip = sensor_mirror_flip,
	.pfnPatchRxAttr = sensor_patch_rx_attr,
	.pfnPatchI2cAddr = sensor_patch_i2c_addr,
	.pfnGetRxAttr = sensor_rx_attr,
	.pfnExpSensorCb = cmos_init_sensor_exp_function,
	.pfnExpAeCb = cmos_init_ae_exp_function,
	.pfnSnsProbe = sensor_probe,
};

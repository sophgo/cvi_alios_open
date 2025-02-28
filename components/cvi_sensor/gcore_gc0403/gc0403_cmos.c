#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include "cvi_type.h"
#include "cvi_debug.h"
#include "cvi_comm_sns.h"
// #include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_isp.h"

#include "gc0403_cmos_ex.h"
#include "gc0403_cmos_param.h"
#ifdef ARCH_CV182X
#include "cvi_vip_cif_uapi.h"
#else
#include "cif_uapi.h"
#endif

#define DIV_0_TO_1(a)   ((0 == (a)) ? 1 : (a))
#define DIV_0_TO_1_FLOAT(a) ((((a) < 1E-10) && ((a) > -1E-10)) ? 1 : (a))
#define GC0403_ID 0x0403
#define GC0403_I2C_ADDR_1 0x3c
#define GC0403_I2C_ADDR_2 0x3c
#define GC0403_I2C_ADDR_IS_VALID(addr)      ((addr) == GC0403_I2C_ADDR_1 || (addr) == GC0403_I2C_ADDR_2)

/****************************************************************************
 * global variables                                                            *
 ****************************************************************************/

ISP_SNS_STATE_S *g_pastGc0403[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define GC0403_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastGc0403[dev])
#define GC0403_SENSOR_SET_CTX(dev, pstCtx)   (g_pastGc0403[dev] = pstCtx)
#define GC0403_SENSOR_RESET_CTX(dev)         (g_pastGc0403[dev] = CVI_NULL)

ISP_SNS_COMMBUS_U g_aunGc0403_BusInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cDev = 0},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

GC0403_STATE_S g_astGc0403_State[VI_MAX_PIPE_NUM] = { {0} };
ISP_SNS_MIRRORFLIP_TYPE_E g_aeGc0403_MirrorFip[VI_MAX_PIPE_NUM] = {0};

CVI_U16 g_au16Gc0403_GainMode[VI_MAX_PIPE_NUM] = {0};
CVI_U16 g_au16Gc0403_L2SMode[VI_MAX_PIPE_NUM] = {0};

/****************************************************************************
 * local variables and functions                                                           *
 ****************************************************************************/
static CVI_U32 g_au32InitExposure[VI_MAX_PIPE_NUM]  = {0};
static CVI_U32 g_au32LinesPer500ms[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16InitWBGain[VI_MAX_PIPE_NUM][3] = {{0} };
static CVI_U16 g_au16SampleRgain[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16SampleBgain[VI_MAX_PIPE_NUM] = {0};
static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg);
/*****Gc0403 Lines Range*****/
#define GC0403_FULL_LINES_MAX  (663-0x46+0x1fff)

/*****Gc0403 Register Address*****/
#define GC0403_EXP_RST_ADDR			0xfe
#define GC0403_EXP_H_ADDR			0x03
#define GC0403_EXP_L_ADDR			0x04
#define GC0403_AGAIN_ADDR    			0xb6
#define GC0403_DGAIN_H_ADDR    			0xb1
#define GC0403_DGAIN_L_ADDR    			0xb2
#define GC0403_VTS_H_ADDR			0x07	//(frame length)
#define GC0403_VTS_L_ADDR			0x08

#define GC0403_RES_IS_576P(w, h)      ((w) == 768 && (h) == 576)

static CVI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	GC0403_SLAVE_MODE_E u8ImgMode;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	GC0403_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8ImgMode = pstSnsState->u8ImgMode;
	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FlickerFreq = 50 * 256;
	pstAeSnsDft->u32FullLinesMax = GC0403_FULL_LINES_MAX;
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * g_stGc0403_mode[u8ImgMode].f32MaxFps);

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
		pstAeSnsDft->u32LinesPer500ms = g_stGc0403_mode[u8ImgMode].f32MaxFps / 2;
	else
		pstAeSnsDft->u32LinesPer500ms = g_au32LinesPer500ms[ViPipe];

	switch (pstSnsState->enWDRMode) {
	default:
	case WDR_MODE_NONE:   /*linear mode*/
		pstAeSnsDft->f32Fps = g_stGc0403_mode[u8ImgMode].f32MaxFps;
		pstAeSnsDft->f32MinFps = g_stGc0403_mode[u8ImgMode].f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xd;
		pstAeSnsDft->au8HistThresh[1] = 0x28;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxAgain = g_stGc0403_mode[u8ImgMode].stAgain.u32Max;
		pstAeSnsDft->u32MinAgain = g_stGc0403_mode[u8ImgMode].stAgain.u32Min;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = g_stGc0403_mode[u8ImgMode].stDgain.u32Max;
		pstAeSnsDft->u32MinDgain = g_stGc0403_mode[u8ImgMode].stDgain.u32Min;
		pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
		pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

		pstAeSnsDft->u8AeCompensation = 40;
		pstAeSnsDft->u32InitAESpeed = 64;
		pstAeSnsDft->u32InitAETolerance = 5;
		pstAeSnsDft->u32AEResponseFrame = 5;
		pstAeSnsDft->u32SnsResponseFrame = 4;
		pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe] ? g_au32InitExposure[ViPipe] :
			g_stGc0403_mode[u8ImgMode].stExp.u16Def;

		pstAeSnsDft->u32MaxIntTime = g_stGc0403_mode[u8ImgMode].stExp.u16Max;
		pstAeSnsDft->u32MinIntTime = g_stGc0403_mode[u8ImgMode].stExp.u16Min;
		pstAeSnsDft->u32MaxIntTimeTarget = 65535;
		pstAeSnsDft->u32MinIntTimeTarget = 1;

		pstAeSnsDft->u32IntAgain = g_stGc0403_mode[u8ImgMode].stAgain.u32Def;
		pstAeSnsDft->u32IntDgain = g_stGc0403_mode[u8ImgMode].stDgain.u32Def;
		pstAeSnsDft->u32IntExposure = g_stGc0403_mode[u8ImgMode].stExp.u16Def;
		break;

	case WDR_MODE_2To1_LINE:
		break;
	}

	return CVI_SUCCESS;
}

/* the function of sensor set fps */
static CVI_S32 cmos_fps_set(VI_PIPE ViPipe, CVI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	CVI_U32 u32VMAX = 0;
	CVI_U32 u32VB = 0;
	CVI_FLOAT f32MaxFps = 0;
	CVI_FLOAT f32MinFps = 0;
	CVI_U32 u32Vts = 0;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	GC0403_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32Vts = g_stGc0403_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	f32MaxFps = g_stGc0403_mode[pstSnsState->u8ImgMode].f32MaxFps;
	f32MinFps = g_stGc0403_mode[pstSnsState->u8ImgMode].f32MinFps;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
			u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport Fps: %f\n", f32Fps);
			return CVI_FAILURE;
		}
		u32VMAX = (u32VMAX > GC0403_FULL_LINES_MAX) ? GC0403_FULL_LINES_MAX : u32VMAX;

		u32VB = u32VMAX - 663 + 0x46;
		pstSnsRegsInfo->astI2cData[LINEAR_VTS_H].u32Data = ((u32VB & 0x1F00) >> 8);
		pstSnsRegsInfo->astI2cData[LINEAR_VTS_L].u32Data = (u32VB & 0xFF);
	}

	pstSnsState->u32FLStd = u32VMAX;

	pstAeSnsDft->f32Fps = f32Fps;
	pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * f32Fps / 2;
	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 1;
	pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * DIV_0_TO_1_FLOAT(f32Fps));

	return CVI_SUCCESS;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static CVI_S32 cmos_inttime_update(VI_PIPE ViPipe, CVI_U32 *u32IntTime)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;


	GC0403_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(u32IntTime);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	pstSnsRegsInfo->astI2cData[LINEAR_EXP_RST].u32Data = 0x00;
	pstSnsRegsInfo->astI2cData[LINEAR_EXP_H].u32Data = (u32IntTime[0] >> 8);
	pstSnsRegsInfo->astI2cData[LINEAR_EXP_L].u32Data = (u32IntTime[0] & 0xFF);

	return CVI_SUCCESS;
}

static CVI_U32 gain_table[] = {
	16 * 64,
	1040,
	1056,
	1072,
	1088,
	1104,
	1120,
	1136,
	1152,
	1168,
	1184,
	1200,
	1216,
	1232,
	1248,
	1264,
	1280,
	1296,
	1312,
	1328,
	1344,
	1360,
	1376,
	1392,
	1408,
	1424,
	16 * 90,
	1463,
	1485,
	1508,
	1530,
	1553,
	1575,
	1598,
	1620,
	1643,
	1665,
	1688,
	1710,
	1733,
	1755,
	1778,
	1800,
	1823,
	1845,
	1868,
	1890,
	1913,
	1935,
	1958,
	1980,
	2003,
	2025,
	2048,
	2070,
	2093,
	2115,
	2138,
	2160,
	2183,
	2205,
	2228,
	2250,
	2273,
	2295,
	2318,
	2340,
	2363,
	2385,
	2408,
	2430,
	2453,
	2475,
	2498,
	2520,
	2543,
	16 * 160,
	2600,
	2640,
	2680,
	2720,
	2760,
	2800,
	2840,
	2880,
	2920,
	2960,
	3000,
	3040,
	3080,
	3120,
	3160,
	3200,
	3240,
	3280,
	3320,
	3360,
	3400,
	3440,
	3480,
	3520,
	3560,
	3600,
	16 * 226,
	3673,
	3729,
	3786,
	3842,
	3899,
	3955,
	4012,
	4068,
	4125,
	4181,
	4238,
	4294,
	4351,
	4407,
	4464,
	4520,
	4577,
	4633,
	4690,
	4746,
	4803,
	4859,
	4916,
	4972,
	5029,
	16 * 314,
	5103,
	5181,
	5260,
	5338,
	5417,
	5495,
	5574,
	5652,
	5731,
	5809,
	5888,
	5966,
	6045,
	6123,
	6202,
	6280,
	6359,
	6437,
	6516,
	6594,
	6673,
	6751,
	6830,
	6908,
	6987,
	7065,
	16 * 442,
	7183,
	7293,
	7404,
	7514,
	7625,
	7735,
	7846,
	7956,
	8067,
	8177,
	8288,
	8398,
	8509,
	8619,
	8730,
	8840,
	8951,
	9061,
	9172,
	9282,
	9393,
	9503,
	9614,
	9724,
	9835,
	16 * 620,
	10075,
	10230,
	10385,
	10540,
	10695,
	10850,
	11005,
	11160,
	11315,
	11470,
	11625,
	11780,
	11935,
	12090,
	12245,
	12400,
	12555,
	12710,
	12865,
	13020,
	13175,
	13330,
	13485,
	13640,
	13795,
	13950,
	16 * 872,
	14170,
	14388,
	14606,
	14824,
	15042,
	15260,
	15478,
	15696,
	15914,
	16132,
	16350,
	16568,
	16786,
	17004,
	17222,
	17440,
	17658,
	17876,
	18094,
	18312,
	18530,
	18748,
	18966,
	19184,
	19402,
	19620,
	19838,
	16 * 1244,
	20215,
	20526,
	20837,
	21148,
	21459,
	21770,
	22081,
	22392,
	22703,
	23014,
	23325,
	23636,
	23947,
	24258,
	24569,
	24880,
	25191,
	25502,
	25813,
	26124,
	26435,
	26746,
	27057,
	27368,
	16 * 1730,
	28113,
	28545,
	28978,
	29410,
	29843,
	30275,
	30708,
	31140,
	31573,
	32005,
	32438,
	32870,
	33303,
	33735,
	34168,
	34600,
	35033,
	35465,
	35898,
	36330,
	36763,
	37195,
	37628,
	38060,
	38493,
	38925,
	39358,
	39790,
	16 * 2489,
	0xffffffff,
};
static CVI_U32 again_table[] = {
	16 * 64,
	16 * 90,
	16 * 160,
	16 * 226,
	16 * 314,
	16 * 442,
	16 * 620,
	16 * 872,
	16 * 1244,
	16 * 1730,
	16 * 2489,
	0xffffffff,
};
static CVI_U32 dgainRegValTable[] = {
	1024,
	2032,
	2512,
	3072,
	3616,
	4144,
	4592,
	5088,
	5696,
	6256,
	6704,
	7200,
	7680,
	8208,
	9216,
};
static CVI_S32 cmos_again_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32AgainLin, CVI_U32 *pu32AgainDb)
{
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

static CVI_S32 cmos_dgain_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32DgainLin, CVI_U32 *pu32DgainDb)
{
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
	*pu32DgainDb = 0;
	*pu32DgainLin = 1024;
	return CVI_SUCCESS;
}

static CVI_S32 cmos_gains_update(VI_PIPE ViPipe, CVI_U32 *pu32Again, CVI_U32 *pu32Dgain)
{
	CVI_U32 tol_dig_gain = 0;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U32 u32Again;
	int i;
	int total = sizeof(again_table) / sizeof(again_table[0]);

	GC0403_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pu32Again);
	CMOS_CHECK_POINTER(pu32Dgain);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	/* only surpport linear mode */
	u32Again = gain_table[pu32Again[0]];

	for(i = 0; i < total; i++)
	{
		if((again_table[i] <= u32Again)&&(u32Again < again_table[i+1]))
			break;
	}

	tol_dig_gain = u32Again*64/again_table[i];
	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN].u32Data = i;
		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_H].u32Data = (tol_dig_gain) >> 6;
		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_L].u32Data = (tol_dig_gain & 0x3f) << 2;
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport WDRMode: %d\n", pstSnsState->enWDRMode);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}
static CVI_S32 cmos_get_inttime_max(VI_PIPE ViPipe, CVI_U16 u16ManRatioEnable, CVI_U32 *au32Ratio,
				    CVI_U32 *au32IntTimeMax, CVI_U32 *au32IntTimeMin, CVI_U32 *pu32LFMaxIntTime)
{
	UNUSED(ViPipe);
	UNUSED(u16ManRatioEnable);
	UNUSED(au32Ratio);
	UNUSED(au32IntTimeMax);
	UNUSED(au32IntTimeMin);
	UNUSED(pu32LFMaxIntTime);

	CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport WDRMode\n");
	return CVI_SUCCESS;
}
static CVI_S32 cmos_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	CMOS_CHECK_POINTER(pstExpFuncs);

	memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));
	pstExpFuncs->pfn_cmos_get_ae_default    = cmos_get_ae_default;
	pstExpFuncs->pfn_cmos_fps_set           = cmos_fps_set;
	pstExpFuncs->pfn_cmos_inttime_update    = cmos_inttime_update;
	pstExpFuncs->pfn_cmos_gains_update      = cmos_gains_update;
	pstExpFuncs->pfn_cmos_again_calc_table  = cmos_again_calc_table;
	pstExpFuncs->pfn_cmos_dgain_calc_table  = cmos_dgain_calc_table;
	pstExpFuncs->pfn_cmos_get_inttime_max   = cmos_get_inttime_max;
	//pstExpFuncs->pfn_cmos_ae_fswdr_attr_set = cmos_ae_fswdr_attr_set;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_awb_default(VI_PIPE ViPipe, AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
	CMOS_CHECK_POINTER(pstAwbSnsDft);
	UNUSED(ViPipe);

	memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

	pstAwbSnsDft->u16InitGgain = 1024;
	pstAwbSnsDft->u8AWBRunInterval = 1;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	CMOS_CHECK_POINTER(pstExpFuncs);

	memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

	pstExpFuncs->pfn_cmos_get_awb_default = cmos_get_awb_default;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_isp_default(VI_PIPE ViPipe, ISP_CMOS_DEFAULT_S *pstDef)
{
	UNUSED(ViPipe);
	memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_blc_default(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *pstBlc)
{
	CMOS_CHECK_POINTER(pstBlc);
	UNUSED(ViPipe);

	memset(pstBlc, 0, sizeof(ISP_CMOS_BLACK_LEVEL_S));

	memcpy(pstBlc,
		&g_stIspBlcCalibratio, sizeof(ISP_CMOS_BLACK_LEVEL_S));
	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg)
{
	const GC0403_MODE_S *pstMode = CVI_NULL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	GC0403_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstMode = &g_stGc0403_mode[pstSnsState->u8ImgMode];
	if (pstSnsState->enWDRMode != WDR_MODE_NONE) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport WDRMode: %d\n", pstSnsState->enWDRMode);
	} else {
		pstIspCfg->frm_num = 1;
		memcpy(&pstIspCfg->img_size[0], &pstMode->stImg, sizeof(ISP_WDR_SIZE_S));
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_wdr_mode(VI_PIPE ViPipe, CVI_U8 u8Mode)
{
	UNUSED(ViPipe);
	UNUSED(u8Mode);
	CVI_TRACE_SNS(CVI_DBG_INFO, "Unsupport sensor mode!\n");
	return CVI_SUCCESS;
}

static CVI_U32 sensor_cmp_wdr_size(ISP_SNS_ISP_INFO_S *pstWdr1, ISP_SNS_ISP_INFO_S *pstWdr2)
{
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

static CVI_S32 cmos_get_sns_regs_info(VI_PIPE ViPipe, ISP_SNS_SYNC_INFO_S *pstSnsSyncInfo)
{
	CVI_U32 i;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	ISP_SNS_SYNC_INFO_S *pstCfg0 = CVI_NULL;
	ISP_SNS_SYNC_INFO_S *pstCfg1 = CVI_NULL;
	ISP_I2C_DATA_S *pstI2c_data = CVI_NULL;

	CMOS_CHECK_POINTER(pstSnsSyncInfo);
	GC0403_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstSnsRegsInfo = &pstSnsSyncInfo->snsCfg;
	pstCfg0 = &pstSnsState->astSyncInfo[0];
	pstCfg1 = &pstSnsState->astSyncInfo[1];
	pstI2c_data = pstCfg0->snsCfg.astI2cData;

	if ((pstSnsState->bSyncInit == CVI_FALSE) || (pstSnsRegsInfo->bConfig == CVI_FALSE)) {
		pstCfg0->snsCfg.enSnsType = SNS_I2C_TYPE;
		pstCfg0->snsCfg.unComBus.s8I2cDev = g_aunGc0403_BusInfo[ViPipe].s8I2cDev;
		pstCfg0->snsCfg.u8Cfg2ValidDelayMax = 0;
		pstCfg0->snsCfg.use_snsr_sram = CVI_TRUE;
		pstCfg0->snsCfg.u32RegNum = LINEAR_REGS_NUM;

		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			pstI2c_data[i].bUpdate = CVI_TRUE;
			pstI2c_data[i].u8DevAddr = gc0403_i2c_addr_write;
			pstI2c_data[i].u32AddrByteNum = gc0403_addr_byte;
			pstI2c_data[i].u32DataByteNum = gc0403_data_byte;
		}

		pstI2c_data[LINEAR_EXP_RST].u32RegAddr = GC0403_EXP_RST_ADDR;
		pstI2c_data[LINEAR_EXP_H].u32RegAddr = GC0403_EXP_H_ADDR;
		pstI2c_data[LINEAR_EXP_L].u32RegAddr = GC0403_EXP_L_ADDR;
		pstI2c_data[LINEAR_AGAIN].u32RegAddr     = GC0403_AGAIN_ADDR;
		pstI2c_data[LINEAR_DGAIN_H].u32RegAddr     = GC0403_DGAIN_H_ADDR;
		pstI2c_data[LINEAR_DGAIN_L].u32RegAddr     = GC0403_DGAIN_L_ADDR;
		pstI2c_data[LINEAR_VTS_H].u32RegAddr = GC0403_VTS_H_ADDR;
		pstI2c_data[LINEAR_VTS_L].u32RegAddr = GC0403_VTS_L_ADDR;

		pstSnsState->bSyncInit = CVI_TRUE;
		pstCfg0->snsCfg.need_update = CVI_TRUE;
		/* recalcualte WDR size */
		cmos_get_wdr_size(ViPipe, &pstCfg0->ispCfg);
		pstCfg0->ispCfg.need_update = CVI_TRUE;
	} else {
		pstCfg0->snsCfg.need_update = CVI_FALSE;
		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			if (pstCfg0->snsCfg.astI2cData[i].u32Data == pstCfg1->snsCfg.astI2cData[i].u32Data) {
				pstCfg0->snsCfg.astI2cData[i].bUpdate = CVI_FALSE;

			} else {
				pstCfg0->snsCfg.astI2cData[i].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.need_update = CVI_TRUE;
			}
		}
		/* check update isp crop or not */
		pstCfg0->ispCfg.need_update = (sensor_cmp_wdr_size(&pstCfg0->ispCfg, &pstCfg1->ispCfg) ?
				CVI_TRUE : CVI_FALSE);
	}

	pstSnsRegsInfo->bConfig = CVI_FALSE;
	memcpy(pstSnsSyncInfo, &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));
	memcpy(&pstSnsState->astSyncInfo[1], &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));
	pstSnsState->au32FL[1] = pstSnsState->au32FL[0];

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_image_mode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
	CVI_U8 u8SensorImageMode = 0;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstSensorImageMode);
	GC0403_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8SensorImageMode = pstSnsState->u8ImgMode;
	pstSnsState->bSyncInit = CVI_FALSE;

	if (pstSensorImageMode->f32Fps <= 60) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (GC0403_RES_IS_576P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = GC0403_MODE_768X576P60;
			else {
				CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
					      pstSensorImageMode->u16Width,
					      pstSensorImageMode->u16Height,
					      pstSensorImageMode->f32Fps,
					      pstSnsState->enWDRMode);
				return CVI_FAILURE;
			}
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
				      pstSensorImageMode->u16Width,
				      pstSensorImageMode->u16Height,
				      pstSensorImageMode->f32Fps,
				      pstSnsState->enWDRMode);
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

static CVI_VOID sensor_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	GC0403_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);
	/* Apply the setting on the fly  */
	if (pstSnsState->bInit == CVI_TRUE && g_aeGc0403_MirrorFip[ViPipe] != eSnsMirrorFlip) {
		gc0403_mirror_flip(ViPipe, eSnsMirrorFlip);
		g_aeGc0403_MirrorFip[ViPipe] = eSnsMirrorFlip;
	}
}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	GC0403_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsState->bInit = CVI_FALSE;
	pstSnsState->bSyncInit = CVI_FALSE;
	pstSnsState->u8ImgMode = GC0403_MODE_768X576P60;
	pstSnsState->enWDRMode = WDR_MODE_NONE;
	pstSnsState->u32FLStd  = g_stGc0403_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[0] = g_stGc0403_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[1] = g_stGc0403_mode[pstSnsState->u8ImgMode].u32VtsDef;

	memset(&pstSnsState->astSyncInfo[0], 0, sizeof(ISP_SNS_SYNC_INFO_S));
	memset(&pstSnsState->astSyncInfo[1], 0, sizeof(ISP_SNS_SYNC_INFO_S));
}

static CVI_S32 sensor_rx_attr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *pstRxAttr)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	GC0403_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pstRxAttr);

	memcpy(pstRxAttr, &gc0403_rx_attr, sizeof(*pstRxAttr));

	pstRxAttr->img_size.width = g_stGc0403_mode[pstSnsState->u8ImgMode].stImg.stSnsSize.u32Width;
	pstRxAttr->img_size.height = g_stGc0403_mode[pstSnsState->u8ImgMode].stImg.stSnsSize.u32Height;
	if (pstSnsState->enWDRMode == WDR_MODE_NONE)
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;

	return CVI_SUCCESS;

}

static CVI_S32 sensor_patch_rx_attr(RX_INIT_ATTR_S *pstRxInitAttr)
{
	SNS_COMBO_DEV_ATTR_S *pstRxAttr = &gc0403_rx_attr;
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

static CVI_S32 cmos_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
	CMOS_CHECK_POINTER(pstSensorExpFunc);

	memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

	pstSensorExpFunc->pfn_cmos_sensor_init = gc0403_init;
	pstSensorExpFunc->pfn_cmos_sensor_exit = gc0403_exit;
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
static CVI_VOID sensor_patch_i2c_addr(CVI_S32 s32I2cAddr)
{
	if (GC0403_I2C_ADDR_IS_VALID(s32I2cAddr))
		gc0403_i2c_addr_write = s32I2cAddr;
}

static CVI_S32 gc0403_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
	g_aunGc0403_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	GC0403_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

	if (pastSnsStateCtx == CVI_NULL) {
		pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
		if (pastSnsStateCtx == CVI_NULL) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
			return -ENOMEM;
		}
	}

	memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

	GC0403_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

	return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	GC0403_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
	SENSOR_FREE(pastSnsStateCtx);
	GC0403_SENSOR_RESET_CTX(ViPipe);
}

static CVI_S32 sensor_register_callback(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib)
{
	CVI_S32 s32Ret;
	ISP_SENSOR_REGISTER_S stIspRegister;
	AE_SENSOR_REGISTER_S  stAeRegister;
	AWB_SENSOR_REGISTER_S stAwbRegister;
	ISP_SNS_ATTR_INFO_S   stSnsAttrInfo;

	CMOS_CHECK_POINTER(pstAeLib);
	CMOS_CHECK_POINTER(pstAwbLib);

	s32Ret = sensor_ctx_init(ViPipe);

	if (s32Ret != CVI_SUCCESS)
		return CVI_FAILURE;

	stSnsAttrInfo.eSensorId = GC0403_ID;

	s32Ret  = cmos_init_sensor_exp_function(&stIspRegister.stSnsExp);
	s32Ret |= CVI_ISP_SensorRegCallBack(ViPipe, &stSnsAttrInfo, &stIspRegister);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function failed!\n");
		return s32Ret;
	}

	s32Ret  = cmos_init_ae_exp_function(&stAeRegister.stAeExp);
	s32Ret |= CVI_AE_SensorRegCallBack(ViPipe, pstAeLib, &stSnsAttrInfo, &stAeRegister);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret  = cmos_init_awb_exp_function(&stAwbRegister.stAwbExp);
	s32Ret |= CVI_AWB_SensorRegCallBack(ViPipe, pstAwbLib, &stSnsAttrInfo, &stAwbRegister);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function to awb lib failed!\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 sensor_unregister_callback(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CMOS_CHECK_POINTER(pstAeLib);
	CMOS_CHECK_POINTER(pstAwbLib);
#if 0
	s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, GC0403_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, GC0403_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, GC0403_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to awb lib failed!\n");
		return s32Ret;
	}
#endif
	sensor_ctx_exit(ViPipe);

	return s32Ret;
}

static CVI_S32 sensor_set_init(VI_PIPE ViPipe, ISP_INIT_ATTR_S *pstInitAttr)
{
	CMOS_CHECK_POINTER(pstInitAttr);

	g_au32InitExposure[ViPipe] = pstInitAttr->u32Exposure;
	g_au32LinesPer500ms[ViPipe] = pstInitAttr->u32LinesPer500ms;
	g_au16InitWBGain[ViPipe][0] = pstInitAttr->u16WBRgain;
	g_au16InitWBGain[ViPipe][1] = pstInitAttr->u16WBGgain;
	g_au16InitWBGain[ViPipe][2] = pstInitAttr->u16WBBgain;
	g_au16SampleRgain[ViPipe] = pstInitAttr->u16SampleRgain;
	g_au16SampleBgain[ViPipe] = pstInitAttr->u16SampleBgain;
	g_au16Gc0403_GainMode[ViPipe] = pstInitAttr->enGainMode;
	g_au16Gc0403_L2SMode[ViPipe] = pstInitAttr->enL2SMode;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_probe(VI_PIPE ViPipe)
{
	return gc0403_probe(ViPipe);
}

ISP_SNS_OBJ_S stSnsGc0403_Obj = {
	.pfnRegisterCallback    = sensor_register_callback,
	.pfnUnRegisterCallback  = sensor_unregister_callback,
	.pfnStandby             = gc0403_standby,
	.pfnRestart             = gc0403_restart,
	.pfnWriteReg            = gc0403_write_register,
	.pfnReadReg             = gc0403_read_register,
	.pfnSetBusInfo          = gc0403_set_bus_info,
	.pfnSetInit             = sensor_set_init,
	.pfnMirrorFlip          = sensor_mirror_flip,
	.pfnPatchRxAttr         = sensor_patch_rx_attr,
	.pfnPatchI2cAddr        = sensor_patch_i2c_addr,
	.pfnGetRxAttr           = sensor_rx_attr,
	.pfnExpSensorCb         = cmos_init_sensor_exp_function,
	.pfnExpAeCb             = cmos_init_ae_exp_function,
	.pfnSnsProbe            = sensor_probe,
};


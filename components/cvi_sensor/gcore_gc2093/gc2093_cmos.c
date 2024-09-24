#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include "cvi_type.h"
#include "cvi_debug.h"
#include "cvi_comm_sns.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_isp.h"

#include "gc2093_cmos_ex.h"
#include "gc2093_cmos_param.h"

#include "cvi_comm_cif.h"

#define DIV_0_TO_1(a)   ((0 == (a)) ? 1 : (a))
#define DIV_0_TO_1_FLOAT(a) ((((a) < 1E-10) && ((a) > -1E-10)) ? 1 : (a))
#define GC2093_ID 2093
/****************************************************************************
 * global variables                                                         *
 ***************************************************************************/

ISP_SNS_STATE_S *g_pastGc2093[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define GC2093_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastGc2093[dev])
#define GC2093_SENSOR_SET_CTX(dev, pstCtx)   (g_pastGc2093[dev] = pstCtx)
#define GC2093_SENSOR_RESET_CTX(dev)         (g_pastGc2093[dev] = CVI_NULL)

ISP_SNS_COMMBUS_U g_aunGc2093_BusInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cDev = 0},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

ISP_SNS_MIRRORFLIP_TYPE_E g_aeGc2093_MirrorFip[VI_MAX_PIPE_NUM] = {0};

CVI_U16 g_au16Gc2093_GainMode[VI_MAX_PIPE_NUM] = {0};

/****************************************************************************
 * local variables and functions                                            *
 ***************************************************************************/
static ISP_FSWDR_MODE_E genFSWDRMode[VI_MAX_PIPE_NUM] = {
	[0 ... VI_MAX_PIPE_NUM - 1] = ISP_FSWDR_NORMAL_MODE
};

static CVI_U32 gu32MaxTimeGetCnt[VI_MAX_PIPE_NUM] = {0};
static CVI_U32 g_au32InitExposure[VI_MAX_PIPE_NUM]  = {0};
static CVI_U32 g_au32LinesPer500ms[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16InitWBGain[VI_MAX_PIPE_NUM][3] = {{0} };
static CVI_U16 g_au16SampleRgain[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16SampleBgain[VI_MAX_PIPE_NUM] = {0};
static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg);
/*****Gc2093 Lines Range*****/
#define GC2093_FULL_LINES_MAX  (0x3fff)

/*****Gc2093 Register Address*****/
#define GC2093_EXP_H_ADDR			0x0003
#define GC2093_EXP_L_ADDR			0x0004
#define GC2093_SEXP_H_ADDR			0x0001
#define GC2093_SEXP_L_ADDR			0x0002

#define GC2093_AGAIN_H_ADDR			0x00b4
#define GC2093_AGAIN_L_ADDR			0x00b3
#define GC2093_COL_AGAIN_H_ADDR			0x00b8
#define GC2093_COL_AGAIN_L_ADDR			0x00b9
#define GC2093_AGAIN_MAG1			0x0155
#define GC2093_AGAIN_HOLD			0x031d
#define GC2093_AGAIN_MAG2			0x00c2
#define GC2093_AGAIN_MAG3			0x00cf
#define GC2093_AGAIN_MAG4			0x00d9

#define GC2093_DGAIN_H_ADDR			0x00b1
#define GC2093_DGAIN_L_ADDR			0x00b2
#define GC2093_VTS_H_ADDR			0x0041	//(frame length)
#define GC2093_VTS_L_ADDR			0x0042

#define GC2093_RES_IS_1080P(w, h)      ((w) <= 1920 && (h) <= 1080)

static CVI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	GC2093_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FlickerFreq = 50 * 256;
	pstAeSnsDft->u32FullLinesMax = GC2093_FULL_LINES_MAX;
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
#if 0
	pstAeSnsDft->enMaxIrisFNO = ISP_IRIS_F_NO_1_0;
	pstAeSnsDft->enMinIrisFNO = ISP_IRIS_F_NO_32_0;

	pstAeSnsDft->bAERouteExValid = CVI_FALSE;
	pstAeSnsDft->stAERouteAttr.u32TotalNum = 0;
	pstAeSnsDft->stAERouteAttrEx.u32TotalNum = 0;
#endif
	switch (pstSnsState->enWDRMode) {
	default:
	case WDR_MODE_NONE:   /*linear mode*/
		pstAeSnsDft->f32Fps = g_astGc2093_mode[GC2093_MODE_1920X1080P30].f32MaxFps;
		pstAeSnsDft->f32MinFps = g_astGc2093_mode[GC2093_MODE_1920X1080P30].f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xd;
		pstAeSnsDft->au8HistThresh[1] = 0x28;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxAgain = 74976;
		pstAeSnsDft->u32MinAgain = 1024;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = 10240;
		pstAeSnsDft->u32MinDgain = 1024;
		pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
		pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

		pstAeSnsDft->u8AeCompensation = 40;
		pstAeSnsDft->u32InitAESpeed = 64;
		pstAeSnsDft->u32InitAETolerance = 5;
		pstAeSnsDft->u32AEResponseFrame = 4;
		pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe] ?
			g_au32InitExposure[ViPipe] : g_astGc2093_mode[GC2093_MODE_1920X1080P30].stExp[0].u16Def;

		pstAeSnsDft->u32MaxIntTime = g_astGc2093_mode[GC2093_MODE_1920X1080P30].stExp[0].u16Max;
		pstAeSnsDft->u32MinIntTime = g_astGc2093_mode[GC2093_MODE_1920X1080P30].stExp[0].u16Min;
		pstAeSnsDft->u32MaxIntTimeTarget = 65535;
		pstAeSnsDft->u32MinIntTimeTarget = 1;
		break;

	case WDR_MODE_2To1_LINE:
		pstAeSnsDft->f32Fps = g_astGc2093_mode[GC2093_MODE_1920X1080P30_WDR].f32MaxFps;
		pstAeSnsDft->f32MinFps = g_astGc2093_mode[GC2093_MODE_1920X1080P30_WDR].f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xC;
		pstAeSnsDft->au8HistThresh[1] = 0x18;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 3;
		pstAeSnsDft->u32MinIntTime = 1;

		pstAeSnsDft->u32MaxIntTimeTarget = 65535;
		pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;

		pstAeSnsDft->u32MaxAgain = 74976;
		pstAeSnsDft->u32MinAgain = 1024;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = 10240;
		pstAeSnsDft->u32MinDgain = 1024;
		pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
		pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;
		pstAeSnsDft->u32MaxISPDgainTarget = 16 << pstAeSnsDft->u32ISPDgainShift;

		pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe] ? g_au32InitExposure[ViPipe] : 52000;
		pstAeSnsDft->u32InitAESpeed = 64;
		pstAeSnsDft->u32InitAETolerance = 5;
		pstAeSnsDft->u32AEResponseFrame = 4;
		if (genFSWDRMode[ViPipe] == ISP_FSWDR_LONG_FRAME_MODE) {
			pstAeSnsDft->u8AeCompensation = 64;
			pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		} else {
			pstAeSnsDft->u8AeCompensation = 40;
			pstAeSnsDft->enAeExpMode = AE_EXP_LOWLIGHT_PRIOR;
			/* [TODO] */
#if 0
			pstAeSnsDft->u16ManRatioEnable = CVI_TRUE;
			pstAeSnsDft->au32Ratio[0] = 0x400;
			pstAeSnsDft->au32Ratio[1] = 0x40;
			pstAeSnsDft->au32Ratio[2] = 0x40;
#endif
		}
		break;
	}

	return CVI_SUCCESS;
}

/* the function of sensor set fps */
static CVI_S32 cmos_fps_set(VI_PIPE ViPipe, CVI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	CVI_U32 u32VMAX;
	CVI_FLOAT f32MaxFps = 0;
	CVI_FLOAT f32MinFps = 0;
	CVI_U32 u32Vts = 0;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	GC2093_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32Vts = g_astGc2093_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	f32MaxFps = g_astGc2093_mode[pstSnsState->u8ImgMode].f32MaxFps;
	f32MinFps = g_astGc2093_mode[pstSnsState->u8ImgMode].f32MinFps;

	if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
		u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport Fps: %f\n", f32Fps);
		return CVI_FAILURE;
	}

	u32VMAX = (u32VMAX > GC2093_FULL_LINES_MAX) ? GC2093_FULL_LINES_MAX : u32VMAX;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstSnsRegsInfo->astI2cData[LINEAR_VTS_H].u32Data = ((u32VMAX & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[LINEAR_VTS_L].u32Data = (u32VMAX & 0xFF);
	} else {
		pstSnsRegsInfo->astI2cData[WDR_VTS_H].u32Data = ((u32VMAX & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[WDR_VTS_L].u32Data = (u32VMAX & 0xFF);
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
static CVI_S32 cmos_inttime_update(VI_PIPE ViPipe, CVI_U32 *u32IntTime)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;

	GC2093_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(u32IntTime);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) {
		CVI_U32 u32ShortIntTime = u32IntTime[0];
		CVI_U32 u32LongIntTime = u32IntTime[1];
		CVI_U16 u16Sexp, u16Lexp;
		CVI_U32 u32MaxRangeLExp, u32MaxRangeSExp;

		/*
		 *exp_short<vb - 8,exp_short+long_exp<frame length - 16; vb=framelengt-window_height-20
		 */
		/* long exposure reg range:
		 * min : 1
		 * max : frame length - exp_short - 16
		 * step : 1
		 */
		/* short exposure reg range:
		 * min : 1
		 * max : frame length-window_height-20 -1 - 8
		 * step : 1
		 */
		// ratio[0]:長幀曝光/短幀曝光比 * 64
		u32MaxRangeSExp = pstSnsState->au32FL[0] - 1088 - 20 - 1 - 8;
		pstSnsState->au32WDRIntTime[0] = (u32ShortIntTime <= u32MaxRangeSExp) ?
			u32ShortIntTime : u32MaxRangeSExp;
		if (!pstSnsState->au32WDRIntTime[0])
			pstSnsState->au32WDRIntTime[0] = 1;
		u16Sexp = (CVI_U16)pstSnsState->au32WDRIntTime[0];

		u32MaxRangeLExp = pstSnsState->au32FL[0] - u16Sexp;
		pstSnsState->au32WDRIntTime[1] = (u32LongIntTime <= u32MaxRangeLExp) ?
			u32LongIntTime : u32MaxRangeLExp;
		if (!pstSnsState->au32WDRIntTime[1])
			pstSnsState->au32WDRIntTime[1] = 1;
		u16Lexp = (CVI_U16)pstSnsState->au32WDRIntTime[1];

		pstSnsRegsInfo->astI2cData[WDR_LEXP_H].u32Data = ((u16Lexp >> 8) & 0x3F);
		pstSnsRegsInfo->astI2cData[WDR_LEXP_L].u32Data = (u16Lexp & 0xFF);

		pstSnsRegsInfo->astI2cData[WDR_SEXP_H].u32Data = ((u16Sexp >> 8) & 0x3F);
		pstSnsRegsInfo->astI2cData[WDR_SEXP_L].u32Data = (u16Sexp & 0xFF);

		//Fixed blooming problem that would occur in highlighted environments
		//sync solution from gc simon
		if (u16Sexp < 0x4)
			gc2093_write_register(ViPipe, 0x0032, 0xfd);
		else
			gc2093_write_register(ViPipe, 0x0032, 0xf8);
	} else {
		pstSnsRegsInfo->astI2cData[LINEAR_EXP_H].u32Data = ((u32IntTime[0] >> 8) & 0x3F);
		pstSnsRegsInfo->astI2cData[LINEAR_EXP_L].u32Data = (u32IntTime[0] & 0xFF);
	}

	return CVI_SUCCESS;
}

static CVI_U32 regValTable[25][7] = {
	//0xb3 0xb8  0xb9  0x155 0xc2  0xcf  0xd9
	{0x00, 0x01, 0x00, 0x08, 0x10, 0x08, 0x0a},
	{0x10, 0x01, 0x0c, 0x08, 0x10, 0x08, 0x0a},
	{0x20, 0x01, 0x1b, 0x08, 0x11, 0x08, 0x0c},
	{0x30, 0x01, 0x2c, 0x08, 0x12, 0x08, 0x0e},
	{0x40, 0x01, 0x3f, 0x08, 0x14, 0x08, 0x12},
	{0x50, 0x02, 0x16, 0x08, 0x15, 0x08, 0x14},
	{0x60, 0x02, 0x35, 0x08, 0x17, 0x08, 0x18},
	{0x70, 0x03, 0x16, 0x08, 0x18, 0x08, 0x1a},
	{0x80, 0x04, 0x02, 0x08, 0x1a, 0x08, 0x1e},
	{0x90, 0x04, 0x31, 0x08, 0x1b, 0x08, 0x20},
	{0xa0, 0x05, 0x32, 0x08, 0x1d, 0x08, 0x24},
	{0xb0, 0x06, 0x35, 0x08, 0x1e, 0x08, 0x26},
	{0xc0, 0x08, 0x04, 0x08, 0x20, 0x08, 0x2a},
	{0x5a, 0x09, 0x19, 0x08, 0x1e, 0x08, 0x2a},
	{0x83, 0x0b, 0x0f, 0x08, 0x1f, 0x08, 0x2a},
	{0x93, 0x0d, 0x12, 0x08, 0x21, 0x08, 0x2e},
	{0x84, 0x10, 0x00, 0x0b, 0x22, 0x08, 0x30},
	{0x94, 0x12, 0x3a, 0x0b, 0x24, 0x08, 0x34},
	{0x5d, 0x1a, 0x02, 0x0b, 0x26, 0x08, 0x34},
	{0x9b, 0x1b, 0x20, 0x0b, 0x26, 0x08, 0x34},
	{0x8c, 0x20, 0x0f, 0x0b, 0x26, 0x08, 0x34},
	{0x9c, 0x26, 0x07, 0x12, 0x26, 0x08, 0x34},
	{0xB6, 0x36, 0x21, 0x12, 0x26, 0x08, 0x34},
	{0xad, 0x37, 0x3a, 0x12, 0x26, 0x08, 0x34},
	{0xbd, 0x3d, 0x02, 0x12, 0x26, 0x08, 0x34},
};

static CVI_U32 regValTable_WDR[25][7] = {
	//0xb3 0xb8  0xb9  0x155 0xc2  0xcf  0xd9
	{0x00, 0x01, 0x00, 0x08, 0x10, 0x08, 0x0a},
	{0x10, 0x01, 0x0c, 0x08, 0x10, 0x08, 0x0a},
	{0x20, 0x01, 0x1b, 0x08, 0x11, 0x08, 0x0c},
	{0x30, 0x01, 0x2c, 0x08, 0x12, 0x08, 0x0e},
	{0x40, 0x01, 0x3f, 0x08, 0x14, 0x08, 0x12},
	{0x50, 0x02, 0x16, 0x08, 0x15, 0x08, 0x14},
	{0x60, 0x02, 0x35, 0x08, 0x17, 0x08, 0x18},
	{0x70, 0x03, 0x16, 0x08, 0x18, 0x08, 0x1a},
	{0x80, 0x04, 0x02, 0x08, 0x1a, 0x08, 0x1e},
	{0x90, 0x04, 0x31, 0x08, 0x1b, 0x08, 0x20},
	{0xa0, 0x05, 0x32, 0x08, 0x1d, 0x08, 0x24},
	{0xb0, 0x06, 0x35, 0x08, 0x1e, 0x08, 0x26},
	{0xc0, 0x08, 0x04, 0x08, 0x20, 0x08, 0x2a},
	{0x5a, 0x09, 0x19, 0x08, 0x1e, 0x08, 0x2a},
	{0x83, 0x0b, 0x0f, 0x08, 0x1f, 0x08, 0x2a},
	{0x93, 0x0d, 0x12, 0x08, 0x21, 0x08, 0x2e},
	{0x84, 0x10, 0x00, 0x0b, 0x22, 0x08, 0x30},
	{0x94, 0x12, 0x3a, 0x0b, 0x24, 0x08, 0x34},
	{0x5d, 0x1a, 0x02, 0x0b, 0x26, 0x08, 0x34},
	{0x9b, 0x1b, 0x20, 0x0b, 0x26, 0x08, 0x34},
	{0x8c, 0x20, 0x0f, 0x0b, 0x26, 0x08, 0x34},
	{0x9c, 0x26, 0x07, 0x12, 0x26, 0x08, 0x34},
	{0xB6, 0x36, 0x21, 0x12, 0x26, 0x08, 0x34},
	{0xad, 0x37, 0x3a, 0x12, 0x26, 0x08, 0x34},
	{0xbd, 0x3d, 0x02, 0x12, 0x26, 0x08, 0x34},
};

static CVI_U32 gain_table[25] = {
	1024, 1216, 1456, 1712, 2000, 2352, 2832, 3376, 3968, 4752, 5696, 6800, 8064,
	9584, 11344, 13376, 15648, 18448, 26352, 26416, 30960, 36672, 51824, 63344, 74976,
};

static CVI_S32 cmos_again_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32AgainLin, CVI_U32 *pu32AgainDb)
{
	int i, total;
	CVI_U32 pregain;

	CMOS_CHECK_POINTER(pu32AgainLin);
	CMOS_CHECK_POINTER(pu32AgainDb);
	UNUSED(ViPipe);
	total = sizeof(gain_table) / sizeof(CVI_U32);

	if (*pu32AgainLin >= gain_table[total - 1]) {
		*pu32AgainLin = *pu32AgainDb = gain_table[total - 1];
		return CVI_SUCCESS;
	}

	for (i = 1; i < total; i++) {
		if (*pu32AgainLin < gain_table[i])
			break;
	}
	i--;
	// find the pregain
	pregain = *pu32AgainLin * 64 / gain_table[i];
	// set the Db as the AE algo gain, we need this to do gain update
	*pu32AgainDb = *pu32AgainLin;
	// set the Lin as the closest sensor gain for AE algo reference
	*pu32AgainLin = pregain * gain_table[i] / 64;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_dgain_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32DgainLin, CVI_U32 *pu32DgainDb)
{
	CVI_U32 pregain;

	CMOS_CHECK_POINTER(pu32DgainLin);
	CMOS_CHECK_POINTER(pu32DgainDb);
	UNUSED(ViPipe);
	// find the pregain
	pregain = *pu32DgainLin * 64 / 1024;
	// set the Db as the AE algo gain, we need this to do gain update
	*pu32DgainDb = *pu32DgainLin;
	// set the Lin as the closest sensor gain for AE algo reference
	*pu32DgainLin = pregain * 16;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_gains_update(VI_PIPE ViPipe, CVI_U32 *pu32Again, CVI_U32 *pu32Dgain)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U32 u32Again;
	CVI_U32 u32Dgain;
	int i, total;

	total = sizeof(gain_table) / sizeof(CVI_U32);

	GC2093_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pu32Again);
	CMOS_CHECK_POINTER(pu32Dgain);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	/* only surpport linear mode */
	u32Again = pu32Again[0];
	/* To kepp the linearity. we assume AE algo adjusts the dgain only when the again reachs the maximum value */
	if (u32Again < (74976)) {
		for (i = 1; i < total; i++) {
			if (*pu32Again < gain_table[i])
				break;
		}
		i--;
		// find the pregain
		u32Dgain = u32Again * 64 / gain_table[i];
		u32Again = i;
	} else {
		u32Again = total - 1;
		// find the pregain
		u32Dgain = pu32Dgain[0] * 64 / 1024;
	}

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_L].u32Data = regValTable[u32Again][0];
		pstSnsRegsInfo->astI2cData[LINEAR_COL_AGAIN_H].u32Data = regValTable[u32Again][1];
		pstSnsRegsInfo->astI2cData[LINEAR_COL_AGAIN_L].u32Data = regValTable[u32Again][2];
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_MAG1].u32Data = regValTable[u32Again][3];
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_HOLD].u32Data = 0X2D;
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_MAG2].u32Data = regValTable[u32Again][4];
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_MAG3].u32Data = regValTable[u32Again][5];
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_MAG4].u32Data = regValTable[u32Again][6];
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_REL].u32Data = 0X28;

		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_H].u32Data = (u32Dgain >> 6);
		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_L].u32Data = (u32Dgain & 0x3F) << 2;
	} else {
		pstSnsRegsInfo->astI2cData[WDR_AGAIN_L].u32Data = regValTable_WDR[u32Again][0];
		pstSnsRegsInfo->astI2cData[WDR_COL_AGAIN_H].u32Data = regValTable_WDR[u32Again][1];
		pstSnsRegsInfo->astI2cData[WDR_COL_AGAIN_L].u32Data = regValTable_WDR[u32Again][2];
		pstSnsRegsInfo->astI2cData[WDR_AGAIN_MAG1].u32Data = regValTable_WDR[u32Again][3];
		pstSnsRegsInfo->astI2cData[WDR_AGAIN_HOLD].u32Data = 0X2D;
		pstSnsRegsInfo->astI2cData[WDR_AGAIN_MAG2].u32Data = regValTable_WDR[u32Again][4];
		pstSnsRegsInfo->astI2cData[WDR_AGAIN_MAG3].u32Data = regValTable_WDR[u32Again][5];
		pstSnsRegsInfo->astI2cData[WDR_AGAIN_MAG4].u32Data = regValTable_WDR[u32Again][6];
		pstSnsRegsInfo->astI2cData[WDR_AGAIN_HOLD].u32Data = 0X28;

		pstSnsRegsInfo->astI2cData[WDR_DGAIN_H].u32Data = (u32Dgain >> 6);
		pstSnsRegsInfo->astI2cData[WDR_DGAIN_L].u32Data = (u32Dgain & 0x3F) << 2;
	}
	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_inttime_max(VI_PIPE ViPipe, CVI_U16 u16ManRatioEnable, CVI_U32 *au32Ratio,
				    CVI_U32 *au32IntTimeMax, CVI_U32 *au32IntTimeMin, CVI_U32 *pu32LFMaxIntTime)
{
	CVI_U32 u32IntTimeMaxTmp  = 0, u32IntTimeMaxTmp0 = 0;
	CVI_U32 u32RatioTmp = 0x40;
	CVI_U32 u32ShortTimeMinLimit = 1;
	CVI_U32 u32ShortTimeMaxLimit = 0;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(au32Ratio);
	CMOS_CHECK_POINTER(au32IntTimeMax);
	CMOS_CHECK_POINTER(au32IntTimeMin);
	CMOS_CHECK_POINTER(pu32LFMaxIntTime);
	GC2093_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	/*
	 *exp_short<vb - 8,exp_short+long_exp<frame length - 16; vb=framelengt-window_height-20
	 */
	/* long exposure reg range:
	 * min : 1
	 * max : frame length - exp_short -16
	 * step : 1
	 */
	/* short exposure reg range:
	 * min : 1
	 * max : frame length-window_height-20 -1 - 8
	 * step : 1
	 */
	// ratio[0]:長幀曝光/短幀曝光比 * 64
	/*use old lexp calculate new sexp*/
	u32IntTimeMaxTmp0 = ((pstSnsState->au32FL[1] - pstSnsState->au32WDRIntTime[0]) * 0x40) /
			DIV_0_TO_1(au32Ratio[0]);
	/*expect new max sexp*/
	u32IntTimeMaxTmp  = ((pstSnsState->au32FL[0] - 1) * 0x40) / DIV_0_TO_1(au32Ratio[0] + 0x40);
	u32ShortTimeMaxLimit = pstSnsState->au32FL[0] - 1088 - 20 - 1 - 8;
	/*real new max sexp*/
	u32IntTimeMaxTmp = (u32IntTimeMaxTmp > u32IntTimeMaxTmp0) ? u32IntTimeMaxTmp0 : u32IntTimeMaxTmp;
	/*we need to make sure sexp <= frame length-window_height-20 -1*/
	u32IntTimeMaxTmp  = (u32IntTimeMaxTmp <= u32ShortTimeMaxLimit) ? u32IntTimeMaxTmp : u32ShortTimeMaxLimit;
	u32IntTimeMaxTmp  = (!u32IntTimeMaxTmp) ? 1 : u32IntTimeMaxTmp;

	if (u32IntTimeMaxTmp >= u32ShortTimeMinLimit) {
		if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
			au32IntTimeMax[0] = u32IntTimeMaxTmp;
			au32IntTimeMax[1] = au32IntTimeMax[0] * au32Ratio[0] >> 6;
			au32IntTimeMax[2] = au32IntTimeMax[1] * au32Ratio[1] >> 6;
			au32IntTimeMax[3] = au32IntTimeMax[2] * au32Ratio[2] >> 6;
			au32IntTimeMin[0] = u32ShortTimeMinLimit;
			au32IntTimeMin[1] = au32IntTimeMin[0] * au32Ratio[0] >> 6;
			au32IntTimeMin[2] = au32IntTimeMin[1] * au32Ratio[1] >> 6;
			au32IntTimeMin[3] = au32IntTimeMin[2] * au32Ratio[2] >> 6;
			CVI_TRACE_SNS(CVI_DBG_DEBUG, "sexp[%d, %d], lexp[%d, %d], ratio:%d\n",
				au32IntTimeMin[0], au32IntTimeMax[0],
				au32IntTimeMin[1], au32IntTimeMax[1], au32Ratio[0]);
		} else {
		}
	} else {
		if (u16ManRatioEnable) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Manaul ExpRatio is too large!\n");
			return CVI_FAILURE;
		}
		u32IntTimeMaxTmp = u32ShortTimeMinLimit;

		if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
			u32RatioTmp = 0xFFF;
			au32IntTimeMax[0] = u32IntTimeMaxTmp;
			au32IntTimeMax[1] = au32IntTimeMax[0] * u32RatioTmp >> 6;
		} else {
		}
		au32IntTimeMin[0] = au32IntTimeMax[0];
		au32IntTimeMin[1] = au32IntTimeMax[1];
		au32IntTimeMin[2] = au32IntTimeMax[2];
		au32IntTimeMin[3] = au32IntTimeMax[3];
	}
	return CVI_SUCCESS;
}

/* Only used in LINE_WDR mode */
static CVI_S32 cmos_ae_fswdr_attr_set(VI_PIPE ViPipe, AE_FSWDR_ATTR_S *pstAeFSWDRAttr)
{
	CMOS_CHECK_POINTER(pstAeFSWDRAttr);

	genFSWDRMode[ViPipe] = pstAeFSWDRAttr->enFSWDRMode;
	gu32MaxTimeGetCnt[ViPipe] = 0;

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
	pstExpFuncs->pfn_cmos_ae_fswdr_attr_set = cmos_ae_fswdr_attr_set;

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
	const GC2093_MODE_S *pstMode = CVI_NULL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	GC2093_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstMode = &g_astGc2093_mode[pstSnsState->u8ImgMode];

	if (pstSnsState->enWDRMode != WDR_MODE_NONE) {
		pstIspCfg->frm_num = 2;
		memcpy(&pstIspCfg->img_size[0], &pstMode->astImg[0], sizeof(ISP_WDR_SIZE_S));
		memcpy(&pstIspCfg->img_size[1], &pstMode->astImg[1], sizeof(ISP_WDR_SIZE_S));
	} else {
		pstIspCfg->frm_num = 1;
		memcpy(&pstIspCfg->img_size[0], &pstMode->astImg[0], sizeof(ISP_WDR_SIZE_S));
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_wdr_mode(VI_PIPE ViPipe, CVI_U8 u8Mode)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	GC2093_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstSnsState->bSyncInit = CVI_FALSE;

	switch (u8Mode) {
	case WDR_MODE_NONE:
		if (pstSnsState->u8ImgMode == GC2093_MODE_1920X1080P30_WDR)
			pstSnsState->u8ImgMode = GC2093_MODE_1920X1080P30;

		pstSnsState->enWDRMode = WDR_MODE_NONE;
		pstSnsState->u32FLStd = g_astGc2093_mode[pstSnsState->u8ImgMode].u32VtsDef;
		//syslog(LOG_INFO, "WDR_MODE_NONE\n");
		break;
	case WDR_MODE_2To1_LINE:
		if (pstSnsState->u8ImgMode == GC2093_MODE_1920X1080P30)
			pstSnsState->u8ImgMode = GC2093_MODE_1920X1080P30_WDR;

		pstSnsState->enWDRMode = WDR_MODE_2To1_LINE;
		pstSnsState->u32FLStd = g_astGc2093_mode[pstSnsState->u8ImgMode].u32VtsDef;
		//syslog(LOG_INFO, "2to1 line WDR 1080p mode(60fps->30fps)\n");
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_WARN, "Unsupport sensor mode!\n");
		return CVI_FAILURE;
	}

	pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
	pstSnsState->au32FL[1] = pstSnsState->au32FL[0];
	memset(pstSnsState->au32WDRIntTime, 0, sizeof(pstSnsState->au32WDRIntTime));

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
	GC2093_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstSnsRegsInfo = &pstSnsSyncInfo->snsCfg;
	pstCfg0 = &pstSnsState->astSyncInfo[0];
	pstCfg1 = &pstSnsState->astSyncInfo[1];
	pstI2c_data = pstCfg0->snsCfg.astI2cData;

	if ((pstSnsState->bSyncInit == CVI_FALSE) || (pstSnsRegsInfo->bConfig == CVI_FALSE)) {
		pstCfg0->snsCfg.enSnsType = SNS_I2C_TYPE;
		pstCfg0->snsCfg.unComBus.s8I2cDev = g_aunGc2093_BusInfo[ViPipe].s8I2cDev;
		pstCfg0->snsCfg.u8Cfg2ValidDelayMax = 0;
		pstCfg0->snsCfg.use_snsr_sram = CVI_TRUE;
		pstCfg0->snsCfg.u32RegNum = (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) ?
					WDR_REGS_NUM : LINEAR_REGS_NUM;

		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			pstI2c_data[i].bUpdate = CVI_TRUE;
			pstI2c_data[i].u8DevAddr = gc2093_i2c_addr;
			pstI2c_data[i].u32AddrByteNum = gc2093_addr_byte;
			pstI2c_data[i].u32DataByteNum = gc2093_data_byte;
		}

		switch (pstSnsState->enWDRMode) {
		case WDR_MODE_2To1_LINE:
			//WDR Mode Regs

			pstI2c_data[WDR_LEXP_H].u32RegAddr = GC2093_EXP_H_ADDR;
			pstI2c_data[WDR_LEXP_L].u32RegAddr = GC2093_EXP_L_ADDR;
			pstI2c_data[WDR_SEXP_H].u32RegAddr = GC2093_SEXP_H_ADDR;
			pstI2c_data[WDR_SEXP_L].u32RegAddr = GC2093_SEXP_L_ADDR;
			pstI2c_data[WDR_AGAIN_L].u32RegAddr = GC2093_AGAIN_L_ADDR;
			pstI2c_data[WDR_COL_AGAIN_H].u32RegAddr = GC2093_COL_AGAIN_H_ADDR;
			pstI2c_data[WDR_COL_AGAIN_L].u32RegAddr = GC2093_COL_AGAIN_L_ADDR;
			pstI2c_data[WDR_AGAIN_MAG1].u32RegAddr = GC2093_AGAIN_MAG1;
			pstI2c_data[WDR_AGAIN_HOLD].u32RegAddr = GC2093_AGAIN_HOLD;
			pstI2c_data[WDR_AGAIN_MAG2].u32RegAddr = GC2093_AGAIN_MAG2;
			pstI2c_data[WDR_AGAIN_MAG3].u32RegAddr = GC2093_AGAIN_MAG3;
			pstI2c_data[WDR_AGAIN_MAG4].u32RegAddr = GC2093_AGAIN_MAG4;
			pstI2c_data[WDR_AGAIN_REL].u32RegAddr = GC2093_AGAIN_HOLD;
			pstI2c_data[WDR_DGAIN_H].u32RegAddr = GC2093_DGAIN_H_ADDR;
			pstI2c_data[WDR_DGAIN_L].u32RegAddr = GC2093_DGAIN_L_ADDR;
			pstI2c_data[WDR_VTS_H].u32RegAddr = GC2093_VTS_H_ADDR;
			pstI2c_data[WDR_VTS_H].u8DelayFrmNum = 1;
			pstI2c_data[WDR_VTS_L].u32RegAddr = GC2093_VTS_L_ADDR;
			pstI2c_data[WDR_VTS_L].u8DelayFrmNum = 1;
			break;
		default:
			pstI2c_data[LINEAR_EXP_H].u32RegAddr = GC2093_EXP_H_ADDR;
			pstI2c_data[LINEAR_EXP_L].u32RegAddr = GC2093_EXP_L_ADDR;
			pstI2c_data[LINEAR_AGAIN_L].u32RegAddr = GC2093_AGAIN_L_ADDR;
			pstI2c_data[LINEAR_COL_AGAIN_H].u32RegAddr = GC2093_COL_AGAIN_H_ADDR;
			pstI2c_data[LINEAR_COL_AGAIN_L].u32RegAddr = GC2093_COL_AGAIN_L_ADDR;
			pstI2c_data[LINEAR_AGAIN_MAG1].u32RegAddr = GC2093_AGAIN_MAG1;
			pstI2c_data[LINEAR_AGAIN_HOLD].u32RegAddr = GC2093_AGAIN_HOLD;
			pstI2c_data[LINEAR_AGAIN_MAG2].u32RegAddr = GC2093_AGAIN_MAG2;
			pstI2c_data[LINEAR_AGAIN_MAG3].u32RegAddr = GC2093_AGAIN_MAG3;
			pstI2c_data[LINEAR_AGAIN_MAG4].u32RegAddr = GC2093_AGAIN_MAG4;
			pstI2c_data[LINEAR_AGAIN_REL].u32RegAddr = GC2093_AGAIN_HOLD;
			pstI2c_data[LINEAR_DGAIN_H].u32RegAddr = GC2093_DGAIN_H_ADDR;
			pstI2c_data[LINEAR_DGAIN_L].u32RegAddr = GC2093_DGAIN_L_ADDR;
			pstI2c_data[LINEAR_VTS_H].u32RegAddr = GC2093_VTS_H_ADDR;
			pstI2c_data[LINEAR_VTS_H].u8DelayFrmNum = 1;
			pstI2c_data[LINEAR_VTS_L].u32RegAddr = GC2093_VTS_L_ADDR;
			pstI2c_data[LINEAR_VTS_L].u8DelayFrmNum = 1;
		}
		pstSnsState->bSyncInit = CVI_TRUE;
		pstCfg0->snsCfg.need_update = CVI_TRUE;
		/* recalcualte WDR size */
		cmos_get_wdr_size(ViPipe, &pstCfg0->ispCfg);
		pstCfg0->ispCfg.need_update = CVI_TRUE;
	} else {

		CVI_U32 gainsUpdate = 0;

		pstCfg0->snsCfg.need_update = CVI_FALSE;
		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			if (pstCfg0->snsCfg.astI2cData[i].u32Data == pstCfg1->snsCfg.astI2cData[i].u32Data) {
				pstCfg0->snsCfg.astI2cData[i].bUpdate = CVI_FALSE;
			} else {

				if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
					if ((i >= LINEAR_AGAIN_L) && (i <= LINEAR_DGAIN_L))
						gainsUpdate = 1;
				} else {
					if ((i >= WDR_AGAIN_L) && (i <= WDR_DGAIN_L))
						gainsUpdate = 1;
				}
				pstCfg0->snsCfg.astI2cData[i].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.need_update = CVI_TRUE;
			}
		}

		if (gainsUpdate) {
			if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
				pstCfg0->snsCfg.astI2cData[LINEAR_AGAIN_L].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_COL_AGAIN_H].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_COL_AGAIN_L].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_AGAIN_MAG1].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_AGAIN_HOLD].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_AGAIN_MAG2].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_AGAIN_MAG3].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_AGAIN_MAG4].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_AGAIN_REL].bUpdate = CVI_TRUE;

				pstCfg0->snsCfg.astI2cData[LINEAR_DGAIN_H].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_DGAIN_L].bUpdate = CVI_TRUE;
			} else {
				pstCfg0->snsCfg.astI2cData[WDR_AGAIN_L].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_COL_AGAIN_H].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_COL_AGAIN_L].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_AGAIN_MAG1].bUpdate = CVI_TRUE;

				pstCfg0->snsCfg.astI2cData[WDR_AGAIN_HOLD].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_AGAIN_MAG2].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_AGAIN_MAG3].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_AGAIN_MAG4].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_AGAIN_REL].bUpdate = CVI_TRUE;

				pstCfg0->snsCfg.astI2cData[WDR_DGAIN_H].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_DGAIN_L].bUpdate = CVI_TRUE;
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
	GC2093_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8SensorImageMode = pstSnsState->u8ImgMode;
	pstSnsState->bSyncInit = CVI_FALSE;

	if (pstSensorImageMode->f32Fps <= 30) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (GC2093_RES_IS_1080P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = GC2093_MODE_1920X1080P30;
			else {
				CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
					      pstSensorImageMode->u16Width,
					      pstSensorImageMode->u16Height,
					      pstSensorImageMode->f32Fps,
					      pstSnsState->enWDRMode);
				return CVI_FAILURE;
			}
		} else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
			if (GC2093_RES_IS_1080P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height)) {
				u8SensorImageMode = GC2093_MODE_1920X1080P30_WDR;
			} else {
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

	GC2093_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);
	/* Apply the setting on the fly  */
	if (pstSnsState->bInit == CVI_TRUE && g_aeGc2093_MirrorFip[ViPipe] != eSnsMirrorFlip) {
		gc2093_mirror_flip(ViPipe, eSnsMirrorFlip);
		g_aeGc2093_MirrorFip[ViPipe] = eSnsMirrorFlip;
	}
}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	GC2093_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsState->bInit = CVI_FALSE;
	pstSnsState->bSyncInit = CVI_FALSE;
	pstSnsState->u8ImgMode = GC2093_MODE_1920X1080P30;
	pstSnsState->enWDRMode = WDR_MODE_NONE;
	pstSnsState->u32FLStd  = g_astGc2093_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[0] = g_astGc2093_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[1] = g_astGc2093_mode[pstSnsState->u8ImgMode].u32VtsDef;

	memset(&pstSnsState->astSyncInfo[0], 0, sizeof(ISP_SNS_SYNC_INFO_S));
	memset(&pstSnsState->astSyncInfo[1], 0, sizeof(ISP_SNS_SYNC_INFO_S));
}

static CVI_S32 sensor_rx_attr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *pstRxAttr)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	GC2093_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pstRxAttr);

	memcpy(pstRxAttr, &gc2093_rx_attr, sizeof(*pstRxAttr));

	pstRxAttr->img_size.width = g_astGc2093_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Width;
	pstRxAttr->img_size.height = g_astGc2093_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Height;
	if (pstSnsState->enWDRMode == WDR_MODE_NONE)
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;

	return CVI_SUCCESS;

}

static CVI_S32 sensor_patch_rx_attr(RX_INIT_ATTR_S *pstRxInitAttr)
{
	SNS_COMBO_DEV_ATTR_S *pstRxAttr = &gc2093_rx_attr;
	int i;

	CMOS_CHECK_POINTER(pstRxInitAttr);

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

	pstSensorExpFunc->pfn_cmos_sensor_init = gc2093_init;
	pstSensorExpFunc->pfn_cmos_sensor_exit = gc2093_exit;
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

static CVI_S32 gc2093_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
	g_aunGc2093_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	GC2093_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

	if (pastSnsStateCtx == CVI_NULL) {
		pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
		if (pastSnsStateCtx == CVI_NULL) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
			return -ENOMEM;
		}
	}

	memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

	GC2093_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

	return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	GC2093_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
	SENSOR_FREE(pastSnsStateCtx);
	GC2093_SENSOR_RESET_CTX(ViPipe);
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

	stSnsAttrInfo.eSensorId = GC2093_ID;

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

	// s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, GC2093_ID);
	// if (s32Ret != CVI_SUCCESS) {
	// 	CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
	// 	return s32Ret;
	// }

	// s32Ret = CVI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, GC2093_ID);
	// if (s32Ret != CVI_SUCCESS) {
	// 	CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
	// 	return s32Ret;
	// }

	// s32Ret = CVI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, GC2093_ID);
	// if (s32Ret != CVI_SUCCESS) {
	// 	CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to awb lib failed!\n");
	// 	return s32Ret;
	// }

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
	g_au16Gc2093_GainMode[ViPipe] = pstInitAttr->enGainMode;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_probe(VI_PIPE ViPipe)
{
	return gc2093_probe(ViPipe);
}

ISP_SNS_OBJ_S stSnsGc2093_Obj = {
	.pfnRegisterCallback    = sensor_register_callback,
	.pfnUnRegisterCallback  = sensor_unregister_callback,
	.pfnStandby             = gc2093_standby,
	.pfnRestart             = gc2093_restart,
	.pfnWriteReg            = gc2093_write_register,
	.pfnReadReg             = gc2093_read_register,
	.pfnSetBusInfo          = gc2093_set_bus_info,
	.pfnSetInit             = sensor_set_init,
	.pfnMirrorFlip          = sensor_mirror_flip,
	.pfnPatchRxAttr		= sensor_patch_rx_attr,
	.pfnGetRxAttr		= sensor_rx_attr,
	.pfnExpSensorCb		= cmos_init_sensor_exp_function,
	.pfnExpAeCb		= cmos_init_ae_exp_function,
	.pfnSnsProbe		= sensor_probe,
};


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <xil_assert.h>
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

#include "imx307_cmos_ex.h"
#include "imx307_cmos_param.h"
//#include <linux/cvi_vip_snsr.h>
#include "cvi_comm_cif.h"

#define DIV_0_TO_1(a)   ((0 == (a)) ? 1 : (a))
#define DIV_0_TO_1_FLOAT(a) ((((a) < 1E-10) && ((a) > -1E-10)) ? 1 : (a))
#define IMX307_ID 307
#define SENSOR_IMX307_WIDTH 1920
#define SENSOR_IMX307_HEIGHT 1080
/****************************************************************************
 * global variables                                                            *
 ****************************************************************************/

ISP_SNS_STATE_S *g_pastImx307[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define IMX307_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastImx307[dev])
#define IMX307_SENSOR_SET_CTX(dev, pstCtx)   (g_pastImx307[dev] = pstCtx)
#define IMX307_SENSOR_RESET_CTX(dev)         (g_pastImx307[dev] = CVI_NULL)

ISP_SNS_COMMBUS_U g_aunImx307_BusInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cDev = 0},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

CVI_U16 g_au16Imx307_GainMode[VI_MAX_PIPE_NUM] = {0};

IMX307_STATE_S g_astImx307_State[VI_MAX_PIPE_NUM] = {{0} };
ISP_SNS_MIRRORFLIP_TYPE_E g_aeImx307_MirrorFip[VI_MAX_PIPE_NUM] = {0};

/****************************************************************************
 * local variables and functions                                                           *
 ****************************************************************************/
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
/*****Imx307 Lines Range*****/
#define IMX307_FULL_LINES_MAX  (0x3FFFF)
#define IMX307_FULL_LINES_MAX_2TO1_WDR  (0x3FFFF)    // considering the YOUT_SIZE and bad frame
#define IMX307_VMAX_1080P30_LINEAR	1125

/*****Imx307 Register Address*****/
#define IMX307_HOLD_ADDR		0x3001
#define IMX307_SHS1_ADDR		0x3020
#define IMX307_SHS2_ADDR		0x3024
#define IMX307_GAIN_ADDR		0x3014
#define IMX307_GAIN1_ADDR		0x30F2
#define IMX307_HCG_ADDR			0x3009
#define IMX307_VMAX_ADDR		0x3018
#define IMX307_YOUT_ADDR		0x3418
#define IMX307_RHS1_ADDR		0x3030
#define IMX307_TABLE_END		0xffff

#define IMX307_RES_IS_1080P(w, h)      ((w) <= 1920 && (h) <= 1080)

static CVI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	IMX307_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
#if 0
	memset(&pstAeSnsDft->stAERouteAttr, 0, sizeof(ISP_AE_ROUTE_S));
#endif
	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FlickerFreq = 50 * 256;
	pstAeSnsDft->u32FullLinesMax = IMX307_FULL_LINES_MAX;
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
	pstAeSnsDft->u32SnsStableFrame = 8;
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
		pstAeSnsDft->f32Fps = g_astImx307_mode[IMX307_MODE_1080P30].f32MaxFps;
		pstAeSnsDft->f32MinFps = g_astImx307_mode[IMX307_MODE_1080P30].f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xd;
		pstAeSnsDft->au8HistThresh[1] = 0x28;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxAgain = 45740;
		pstAeSnsDft->u32MinAgain = 1024;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = 128914;
		pstAeSnsDft->u32MinDgain = 1024;
		pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
		pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

		pstAeSnsDft->u8AeCompensation = 40;
		pstAeSnsDft->u32InitAESpeed = 64;
		pstAeSnsDft->u32InitAETolerance = 5;
		pstAeSnsDft->u32AEResponseFrame = 4;
		pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe] ? g_au32InitExposure[ViPipe] : 76151;

		pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 2;
		pstAeSnsDft->u32MinIntTime = 1;
		pstAeSnsDft->u32MaxIntTimeTarget = 65535;
		pstAeSnsDft->u32MinIntTimeTarget = 1;
		break;

	case WDR_MODE_2To1_LINE:
		pstAeSnsDft->f32Fps = g_astImx307_mode[IMX307_MODE_1080P30_WDR].f32MaxFps;
		pstAeSnsDft->f32MinFps = g_astImx307_mode[IMX307_MODE_1080P30_WDR].f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xC;
		pstAeSnsDft->au8HistThresh[1] = 0x18;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 2;
		pstAeSnsDft->u32MinIntTime = 2;

		pstAeSnsDft->u32MaxIntTimeTarget = 65535;
		pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;

		pstAeSnsDft->u32MaxAgain = 45740;
		pstAeSnsDft->u32MinAgain = 1024;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = 128914;
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

static CVI_S32 cmos_fps_set(VI_PIPE ViPipe, CVI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	CVI_U32 u32VMAX = IMX307_VMAX_1080P30_LINEAR;
	CVI_FLOAT f32MaxFps = 0;
	CVI_FLOAT f32MinFps = 0;
	CVI_U32 u32Vts = 0;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	IMX307_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32Vts = g_astImx307_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	f32MaxFps = g_astImx307_mode[pstSnsState->u8ImgMode].f32MaxFps;
	f32MinFps = g_astImx307_mode[pstSnsState->u8ImgMode].f32MinFps;

	switch (pstSnsState->u8ImgMode) {
	case IMX307_MODE_1080P30_WDR:
		if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
			u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport Fps: %f\n", f32Fps);
			return CVI_FAILURE;
		}
		u32VMAX = (u32VMAX > IMX307_FULL_LINES_MAX_2TO1_WDR) ? IMX307_FULL_LINES_MAX_2TO1_WDR : u32VMAX;
		break;

	case IMX307_MODE_1080P30:
		if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
			u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport Fps: %f\n", f32Fps);
			return CVI_FAILURE;
		}
		u32VMAX = (u32VMAX > IMX307_FULL_LINES_MAX) ? IMX307_FULL_LINES_MAX : u32VMAX;
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_INFO, "Unsupport sensor mode: %d\n", pstSnsState->u8ImgMode);
		return CVI_FAILURE;
	}

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_0].u32Data = (u32VMAX & 0xFF);
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_1].u32Data = ((u32VMAX & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_2].u32Data = ((u32VMAX & 0xF0000) >> 16);
	} else {
		pstSnsRegsInfo->astI2cData[DOL2_VMAX_0].u32Data = (u32VMAX & 0xFF);
		pstSnsRegsInfo->astI2cData[DOL2_VMAX_1].u32Data = ((u32VMAX & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[DOL2_VMAX_2].u32Data = ((u32VMAX & 0xF0000) >> 16);
	}

	if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) {
		pstSnsState->u32FLStd = u32VMAX * 2;
		g_astImx307_State[ViPipe].u32RHS1_MAX = (u32VMAX - g_astImx307_State[ViPipe].u32BRL) * 2 - 21;
	} else {
		pstSnsState->u32FLStd = u32VMAX;
	}

	pstAeSnsDft->f32Fps = f32Fps;
	pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * f32Fps / 2;
	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 2;
	pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * DIV_0_TO_1_FLOAT(f32Fps));

	return CVI_SUCCESS;
}

static CVI_S32 cmos_inttime_update(VI_PIPE ViPipe, CVI_U32 *u32IntTime)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U32 u32Value = 0;
	CVI_U32 u32RHS1 = 0;
	CVI_U32 u32SHS1 = 0;
	CVI_U32 u32SHS2 = 0;
	CVI_U32 u32YOUTSIZE;
	CVI_U32 u16BRL = 0;

	IMX307_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(u32IntTime);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	u16BRL = g_astImx307_mode[pstSnsState->u8ImgMode].u16BRL;

	if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) {
		CVI_U32 u32ShortIntTime = u32IntTime[0];
		CVI_U32 u32LongIntTime = u32IntTime[1];

		if (pstSnsState->au32FL[1] < u32LongIntTime - 1) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "FL %d is smaller than  = %d\n",
					pstSnsState->au32FL[1], u32LongIntTime - 1);
			return CVI_FAILURE;
		}
		u32SHS2 = pstSnsState->au32FL[1] - u32LongIntTime - 1;

		u32SHS1 = (u32ShortIntTime % 2) + 2;
		u32RHS1 = u32ShortIntTime + u32SHS1 + 1;
		g_astImx307_State[ViPipe].u32RHS1 = u32RHS1;

		/* short exposure */
		pstSnsState->au32WDRIntTime[0] = u32RHS1 - (u32SHS1 + 1);
		/* long exposure */
		pstSnsState->au32WDRIntTime[1] = pstSnsState->au32FL[1] - (u32SHS2 + 1);
		/* Return the actual exposure lines*/
		u32IntTime[0] = pstSnsState->au32WDRIntTime[0];
		u32IntTime[1] = pstSnsState->au32WDRIntTime[1];

		u32YOUTSIZE = (u16BRL + (u32RHS1 - 1) / 2) * 2;
		u32YOUTSIZE = (u32YOUTSIZE >= 0x1FFF) ? 0x1FFF : u32YOUTSIZE;

		CVI_TRACE_SNS(CVI_DBG_DEBUG, "u32ShortIntTime = %d u32SHS1 = %d\n", u32ShortIntTime, u32SHS1);
		CVI_TRACE_SNS(CVI_DBG_DEBUG, "ViPipe = %d RHS1 = %d u32YOUTSIZE = %d\n", ViPipe, u32RHS1, u32YOUTSIZE);

		pstSnsRegsInfo->astI2cData[DOL2_SHS1_0].u32Data = (u32SHS1 & 0xFF);
		pstSnsRegsInfo->astI2cData[DOL2_SHS1_1].u32Data = ((u32SHS1 & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[DOL2_SHS1_2].u32Data = ((u32SHS1 & 0xF0000) >> 16);

		pstSnsRegsInfo->astI2cData[DOL2_SHS2_0].u32Data = (u32SHS2 & 0xFF);
		pstSnsRegsInfo->astI2cData[DOL2_SHS2_1].u32Data = ((u32SHS2 & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[DOL2_SHS2_2].u32Data = ((u32SHS2 & 0xF0000) >> 16);

		pstSnsRegsInfo->astI2cData[DOL2_RHS1_0].u32Data = (u32RHS1 & 0xFF);
		pstSnsRegsInfo->astI2cData[DOL2_RHS1_1].u32Data = ((u32RHS1 & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[DOL2_RHS1_2].u32Data = ((u32RHS1 & 0xF0000) >> 16);

		pstSnsRegsInfo->astI2cData[DOL2_YOUT_SIZE_0].u32Data = (u32YOUTSIZE & 0xFF);
		pstSnsRegsInfo->astI2cData[DOL2_YOUT_SIZE_1].u32Data = ((u32YOUTSIZE & 0x1F00) >> 8);
		/* update isp */
		cmos_get_wdr_size(ViPipe, &pstSnsState->astSyncInfo[0].ispCfg);
	} else {
		if (pstSnsState->au32FL[0] < *u32IntTime - 1) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "FL %d is smaller than  = %d\n",
					pstSnsState->au32FL[0], *u32IntTime - 1);
			return CVI_FAILURE;
		}
		u32Value = pstSnsState->au32FL[0] - *u32IntTime - 1;

		pstSnsRegsInfo->astI2cData[LINEAR_SHS1_0].u32Data = (u32Value & 0xFF);
		pstSnsRegsInfo->astI2cData[LINEAR_SHS1_1].u32Data = ((u32Value & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[LINEAR_SHS1_2].u32Data = ((u32Value & 0x30000) >> 16);
	}

	return CVI_SUCCESS;

}

static CVI_U32 gain_table[231] = {
	1024, 1059, 1097, 1135, 1175, 1217, 1259, 1304, 1349, 1397, 1446, 1497, 1549, 1604, 1660, 1719, 1779, 1842,
	1906, 1973, 2043, 2114, 2189, 2266, 2345, 2428, 2513, 2602, 2693, 2788, 2886, 2987, 3092, 3201, 3313, 3430,
	3550, 3675, 3804, 3938, 4076, 4219, 4368, 4521, 4680, 4845, 5015, 5191, 5374, 5562, 5758, 5960, 6170, 6387,
	6611, 6843, 7084, 7333, 7591, 7857, 8134, 8419, 8715, 9022, 9339, 9667, 10007, 10358, 10722, 11099, 11489,
	11893, 12311, 12743, 13191, 13655, 14135, 14631, 15146, 15678, 16229, 16799, 17390, 18001, 18633, 19288,
	19966, 20668, 21394, 22146, 22924, 23730, 24564, 25427, 26320, 27245, 28203, 29194, 30220, 31282, 32381,
	33519, 34697, 35917, 37179, 38485, 39838, 41238, 42687, 44187, 45740, 47347, 49011, 50734, 52517, 54362, 56272,
	58250, 60297, 62416, 64610, 66880, 69231, 71663, 74182, 76789, 79487, 82281, 85172, 88165, 91264, 94471, 97791,
	101227, 104785, 108467, 112279, 116225,	120309, 124537, 128913, 133444, 138133, 142988, 148013, 153214, 158599,
	164172, 169941, 175913, 182095, 188495,	195119, 201976, 209073, 216421, 224026, 231899, 240049, 248485, 257217,
	266256, 275613, 285298, 295324, 305703,	320110, 327567, 339078, 350994, 363329, 376097, 389314, 402995, 417157,
	431817, 446992, 462700, 478960, 495792,	513215, 531251, 549920, 569246, 589250, 609958, 631393, 653581, 676550,
	700325, 724936, 750412, 776783, 804081,	832338, 861588, 891866, 923208, 955652, 989236, 1024000, 1059985,
	1097235, 1135795, 1175709, 1217026, 1259795, 1304067, 1349894, 1397333, 1446438, 1497269, 1549886, 1604353,
	1660733, 1719095, 1779508, 1842043, 1906777, 1913785, 2043148, 2114949, 2189273, 2266208, 2345848, 2428286,
	2513621, 2601956, 2662563, 2788046, 2886024
};

static CVI_S32 cmos_again_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32AgainLin, CVI_U32 *pu32AgainDb)
{
	int i;

	(void) ViPipe;

	CMOS_CHECK_POINTER(pu32AgainLin);
	CMOS_CHECK_POINTER(pu32AgainDb);

	if (*pu32AgainLin >= gain_table[230]) {
		*pu32AgainLin = gain_table[230];
		*pu32AgainDb = 230;
		return CVI_SUCCESS;
	}

	for (i = 1; i < 231; i++) {
		if (*pu32AgainLin < gain_table[i]) {
			*pu32AgainLin = gain_table[i - 1];
			*pu32AgainDb = i - 1;
			break;
		}
	}
	return CVI_SUCCESS;
}

static CVI_S32 cmos_dgain_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32DgainLin, CVI_U32 *pu32DgainDb)
{
	int i;

	(void) ViPipe;

	CMOS_CHECK_POINTER(pu32DgainLin);
	CMOS_CHECK_POINTER(pu32DgainDb);

	if (*pu32DgainLin >= gain_table[140]) {
		*pu32DgainLin = gain_table[140];
		*pu32DgainDb = 140;
		return CVI_SUCCESS;
	}

	for (i = 1; i < 141; i++) {
		if (*pu32DgainLin < gain_table[i]) {
			*pu32DgainLin = gain_table[i - 1];
			*pu32DgainDb = i - 1;
			break;
		}
	}
	return CVI_SUCCESS;
}

static CVI_S32 cmos_gains_update(VI_PIPE ViPipe, CVI_U32 *pu32Again, CVI_U32 *pu32Dgain)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U32 u32HCG = g_astImx307_State[ViPipe].u8Hcg;
	CVI_U32 u16Mode = g_au16Imx307_GainMode[ViPipe];
	CVI_U32 u32Tmp;
	CVI_U32 u32Again;
	CVI_U32 u32Dgain;

	IMX307_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pu32Again);
	CMOS_CHECK_POINTER(pu32Dgain);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		/* linear mode */
		u32Again = pu32Again[0];
		u32Dgain = pu32Dgain[0];

		if (u32Again >= 20) {
			/* hcg bit[4]*/
			u32HCG = u32HCG | 0x10;
			u32Again = u32Again - 20;
		}

		u32Tmp = u32Again + u32Dgain;

		pstSnsRegsInfo->astI2cData[LINEAR_GAIN].u32Data = (u32Tmp & 0xFF);
		pstSnsRegsInfo->astI2cData[LINEAR_HCG].u32Data = (u32HCG & 0xFF);
	} else {
		/* DOL mode */
		if (u16Mode == SNS_GAIN_MODE_WDR_2F) {
			/* don't support gain conversion in this mode. */
			u32Again = pu32Again[1];
			u32Dgain = pu32Dgain[1];

			u32Tmp = u32Again + u32Dgain;
			if (u32Tmp > 0xFF) {
				u32Tmp = 0xFF;
			}
			if (u32HCG > 0xFF) {
				u32HCG = 0xFF;
			}
			pstSnsRegsInfo->astI2cData[DOL2_GAIN].u32Data = (u32Tmp & 0xFF);
			pstSnsRegsInfo->astI2cData[DOL2_HCG].u32Data = (u32HCG & 0xFF);

			u32Again = pu32Again[0];
			u32Dgain = pu32Dgain[0];

			u32Tmp = u32Again + u32Dgain;
			if (u32Tmp > 0xFF) {
				u32Tmp = 0xFF;
			}
			pstSnsRegsInfo->astI2cData[DOL2_GAIN1].u32Data = (u32Tmp & 0xFF);
		} else if (u16Mode == SNS_GAIN_MODE_SHARE) {
			u32Again = pu32Again[0];
			u32Dgain = pu32Dgain[0];

			if (u32Again >= 20) {
				/* hcg bit[4]*/
				u32HCG = u32HCG | 0x10;
				u32Again = u32Again - 20;
			}

			u32Tmp = u32Again + u32Dgain;
			if (u32Tmp > 0xFF) {
				u32Tmp = 0xFF;
			}
			if (u32HCG > 0xFF) {
				u32HCG = 0xFF;
			}
			pstSnsRegsInfo->astI2cData[DOL2_GAIN].u32Data = (u32Tmp & 0xFF);
			pstSnsRegsInfo->astI2cData[DOL2_HCG].u32Data = (u32HCG & 0xFF);
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_inttime_max(VI_PIPE ViPipe, CVI_U16 u16ManRatioEnable, CVI_U32 *au32Ratio,
		CVI_U32 *au32IntTimeMax, CVI_U32 *au32IntTimeMin, CVI_U32 *pu32LFMaxIntTime)
{
	CVI_U32 u32IntTimeMaxTmp = 0, u32IntTimeMaxTmp0 = 0;
	CVI_U32 u32RatioTmp = 0x40;
	CVI_U32 u32ShortTimeMinLimit = 0;

	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(au32Ratio);
	CMOS_CHECK_POINTER(au32IntTimeMax);
	CMOS_CHECK_POINTER(au32IntTimeMin);
	CMOS_CHECK_POINTER(pu32LFMaxIntTime);
	IMX307_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32ShortTimeMinLimit = (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) ? 2 : 2;

	if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) {
		if (genFSWDRMode[ViPipe] == ISP_FSWDR_LONG_FRAME_MODE) {
			u32IntTimeMaxTmp = pstSnsState->au32FL[0] - 10;
			au32IntTimeMax[0] = u32IntTimeMaxTmp;
			au32IntTimeMin[0] = u32ShortTimeMinLimit;
			return CVI_SUCCESS;
		}
		u32IntTimeMaxTmp0 = ((pstSnsState->au32FL[1] - 6 - pstSnsState->au32WDRIntTime[0]) * 0x40) /
						DIV_0_TO_1(au32Ratio[0]);
		u32IntTimeMaxTmp  = ((pstSnsState->au32FL[0] - 6) * 0x40)  / DIV_0_TO_1(au32Ratio[0] + 0x40);
		u32IntTimeMaxTmp = (u32IntTimeMaxTmp > u32IntTimeMaxTmp0) ? u32IntTimeMaxTmp0 : u32IntTimeMaxTmp;
		u32IntTimeMaxTmp  = (u32IntTimeMaxTmp > (g_astImx307_State[ViPipe].u32RHS1_MAX - 3)) ?
						(g_astImx307_State[ViPipe].u32RHS1_MAX - 3) : u32IntTimeMaxTmp;
		u32IntTimeMaxTmp  = (!u32IntTimeMaxTmp) ? 1 : u32IntTimeMaxTmp;

	}

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
		} else {
		}
	} else {
		if (u16ManRatioEnable) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Manaul ExpRatio out of range!\n");
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
	//pstExpFuncs->pfn_cmos_slow_framerate_set = cmos_slow_framerate_set;
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
	(void) ViPipe;

	memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));

//	memcpy(pstDef->stNoiseCalibration.CalibrationCoef,
//		&g_stIspNoiseCalibratio, sizeof(ISP_CMOS_NOISE_CALIBRATION_S));
	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_blc_default(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *pstBlc)
{
	(void) ViPipe;

	CMOS_CHECK_POINTER(pstBlc);

	memset(pstBlc, 0, sizeof(ISP_CMOS_BLACK_LEVEL_S));

	memcpy(pstBlc,
		&g_stIspBlcCalibratio, sizeof(ISP_CMOS_BLACK_LEVEL_S));
	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg)
{
	const IMX307_MODE_S *pstMode = CVI_NULL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	IMX307_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstMode = &g_astImx307_mode[pstSnsState->u8ImgMode];

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

	IMX307_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstSnsState->bSyncInit = CVI_FALSE;

	switch (u8Mode) {
	case WDR_MODE_NONE:
		if (pstSnsState->u8ImgMode == IMX307_MODE_1080P30_WDR)
			pstSnsState->u8ImgMode = IMX307_MODE_1080P30;
		pstSnsState->enWDRMode = WDR_MODE_NONE;
		pstSnsState->u32FLStd = g_astImx307_mode[pstSnsState->u8ImgMode].u32VtsDef;
		g_astImx307_State[ViPipe].u8Hcg = 0x2;
		CVI_TRACE_SNS(CVI_DBG_INFO, "WDR_MODE_NONE\n");
		break;

	case WDR_MODE_2To1_LINE:
		if (pstSnsState->u8ImgMode == IMX307_MODE_1080P30)
			pstSnsState->u8ImgMode = IMX307_MODE_1080P30_WDR;
		pstSnsState->enWDRMode = WDR_MODE_2To1_LINE;
		g_astImx307_State[ViPipe].u8Hcg    = 0x1;
		if (pstSnsState->u8ImgMode == IMX307_MODE_1080P30_WDR) {
			pstSnsState->u32FLStd = g_astImx307_mode[pstSnsState->u8ImgMode].u32VtsDef * 2;
			g_astImx307_State[ViPipe].u32BRL  = g_astImx307_mode[pstSnsState->u8ImgMode].u16BRL;
			CVI_TRACE_SNS(CVI_DBG_INFO, "WDR_MODE_2To1_LINE 1080p\n");
		}
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unknown mode!\n");
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
	IMX307_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstSnsRegsInfo = &pstSnsSyncInfo->snsCfg;
	pstCfg0 = &pstSnsState->astSyncInfo[0];
	pstCfg1 = &pstSnsState->astSyncInfo[1];
	pstI2c_data = pstCfg0->snsCfg.astI2cData;

	if ((pstSnsState->bSyncInit == CVI_FALSE) || (pstSnsRegsInfo->bConfig == CVI_FALSE)) {
		pstCfg0->snsCfg.enSnsType = SNS_I2C_TYPE;
		pstCfg0->snsCfg.unComBus.s8I2cDev = g_aunImx307_BusInfo[ViPipe].s8I2cDev;
		pstCfg0->snsCfg.u8Cfg2ValidDelayMax = 2;
		pstCfg0->snsCfg.use_snsr_sram = CVI_TRUE;
		pstCfg0->snsCfg.u32RegNum = (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) ?
					DOL2_REGS_NUM : LINEAR_REGS_NUM;

		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			pstI2c_data[i].bUpdate = CVI_TRUE;
			pstI2c_data[i].u8DevAddr = imx307_i2c_addr;
			pstI2c_data[i].u32AddrByteNum = imx307_addr_byte;
			pstI2c_data[i].u32DataByteNum = imx307_data_byte;
		}

		switch (pstSnsState->enWDRMode) {
		case WDR_MODE_2To1_LINE:
			pstI2c_data[DOL2_HOLD].u32RegAddr = IMX307_HOLD_ADDR;
			pstI2c_data[DOL2_HOLD].u32Data = 1;
			pstI2c_data[DOL2_SHS1_0].u32RegAddr = IMX307_SHS1_ADDR;
			pstI2c_data[DOL2_SHS1_1].u32RegAddr = IMX307_SHS1_ADDR + 1;
			pstI2c_data[DOL2_SHS1_2].u32RegAddr = IMX307_SHS1_ADDR + 2;

			pstI2c_data[DOL2_GAIN].u32RegAddr = IMX307_GAIN_ADDR;
			pstI2c_data[DOL2_HCG].u32RegAddr = IMX307_HCG_ADDR;
			pstI2c_data[DOL2_HCG].u8DelayFrmNum = 1;
			pstI2c_data[DOL2_GAIN1].u32RegAddr = IMX307_GAIN1_ADDR;
			pstI2c_data[DOL2_RHS1_0].u32RegAddr = IMX307_RHS1_ADDR;
			pstI2c_data[DOL2_RHS1_0].u8DelayFrmNum = 1;
			pstI2c_data[DOL2_RHS1_1].u32RegAddr = IMX307_RHS1_ADDR + 1;
			pstI2c_data[DOL2_RHS1_1].u8DelayFrmNum = 1;
			pstI2c_data[DOL2_RHS1_2].u32RegAddr = IMX307_RHS1_ADDR + 2;
			pstI2c_data[DOL2_RHS1_2].u8DelayFrmNum = 1;
			pstI2c_data[DOL2_SHS2_0].u32RegAddr = IMX307_SHS2_ADDR;
			pstI2c_data[DOL2_SHS2_1].u32RegAddr = IMX307_SHS2_ADDR + 1;
			pstI2c_data[DOL2_SHS2_2].u32RegAddr = IMX307_SHS2_ADDR + 2;
			pstI2c_data[DOL2_VMAX_0].u32RegAddr = IMX307_VMAX_ADDR;
			pstI2c_data[DOL2_VMAX_0].u8DelayFrmNum = 1;
			pstI2c_data[DOL2_VMAX_1].u32RegAddr = IMX307_VMAX_ADDR + 1;
			pstI2c_data[DOL2_VMAX_1].u8DelayFrmNum = 1;
			pstI2c_data[DOL2_VMAX_2].u32RegAddr = IMX307_VMAX_ADDR + 2;
			pstI2c_data[DOL2_VMAX_2].u8DelayFrmNum = 1;

			pstI2c_data[DOL2_YOUT_SIZE_0].u32RegAddr = IMX307_YOUT_ADDR;
			pstI2c_data[DOL2_YOUT_SIZE_0].u8DelayFrmNum = 1;
			pstI2c_data[DOL2_YOUT_SIZE_1].u32RegAddr = IMX307_YOUT_ADDR + 1;
			pstI2c_data[DOL2_YOUT_SIZE_1].u8DelayFrmNum = 1;
			pstI2c_data[DOL2_REL].u32RegAddr = IMX307_HOLD_ADDR;
			pstI2c_data[DOL2_REL].u32Data = 0;
			pstCfg0->ispCfg.u8DelayFrmNum = 1;
			break;
		default:
			pstI2c_data[LINEAR_HOLD].u32RegAddr = IMX307_HOLD_ADDR;
			pstI2c_data[LINEAR_HOLD].u32Data = 1;
			pstI2c_data[LINEAR_SHS1_0].u32RegAddr = IMX307_SHS1_ADDR;
			pstI2c_data[LINEAR_SHS1_1].u32RegAddr = IMX307_SHS1_ADDR + 1;
			pstI2c_data[LINEAR_SHS1_2].u32RegAddr = IMX307_SHS1_ADDR + 2;
			pstI2c_data[LINEAR_GAIN].u32RegAddr = IMX307_GAIN_ADDR;
			pstI2c_data[LINEAR_HCG].u32RegAddr = IMX307_HCG_ADDR;
			pstI2c_data[LINEAR_HCG].u8DelayFrmNum = 1;
			pstI2c_data[LINEAR_VMAX_0].u32RegAddr = IMX307_VMAX_ADDR;
			pstI2c_data[LINEAR_VMAX_1].u32RegAddr = IMX307_VMAX_ADDR + 1;
			pstI2c_data[LINEAR_VMAX_2].u32RegAddr = IMX307_VMAX_ADDR + 2;
			pstI2c_data[LINEAR_REL].u32RegAddr = IMX307_HOLD_ADDR;
			pstI2c_data[LINEAR_REL].u32Data = 0;
			pstCfg0->ispCfg.u8DelayFrmNum = 0;
			break;
		}
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
		if (pstCfg0->snsCfg.need_update == CVI_TRUE) {
			if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
				pstI2c_data[DOL2_HOLD].u32Data = 1;
				pstI2c_data[DOL2_HOLD].bUpdate = CVI_TRUE;
				pstI2c_data[DOL2_REL].u32Data = 0;
				pstI2c_data[DOL2_REL].bUpdate = CVI_TRUE;
			} else {
				pstI2c_data[LINEAR_HOLD].u32Data = 1;
				pstI2c_data[LINEAR_HOLD].bUpdate = CVI_TRUE;
				pstI2c_data[LINEAR_REL].u32Data = 0;
				pstI2c_data[LINEAR_REL].bUpdate = CVI_TRUE;
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
	IMX307_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8SensorImageMode = pstSnsState->u8ImgMode;
	pstSnsState->bSyncInit = CVI_FALSE;

	if (pstSensorImageMode->f32Fps <= 30) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (IMX307_RES_IS_1080P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height)) {
				u8SensorImageMode = IMX307_MODE_1080P30;
			} else {
				CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
				       pstSensorImageMode->u16Width,
				       pstSensorImageMode->u16Height,
				       pstSensorImageMode->f32Fps,
				       pstSnsState->enWDRMode);
				return CVI_FAILURE;
			}
		} else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
			if (IMX307_RES_IS_1080P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height)) {
				u8SensorImageMode = IMX307_MODE_1080P30_WDR;
				g_astImx307_State[ViPipe].u32BRL = 1109;
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
	}

	if ((pstSnsState->bInit == CVI_TRUE) && (u8SensorImageMode == pstSnsState->u8ImgMode)) {
		return CVI_FAILURE;
	}

	pstSnsState->u8ImgMode = u8SensorImageMode;

	return CVI_SUCCESS;
}

static CVI_VOID sensor_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	IMX307_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);
	if (pstSnsState->bInit == CVI_TRUE && g_aeImx307_MirrorFip[ViPipe] != eSnsMirrorFlip) {
		imx307_mirror_flip(ViPipe, eSnsMirrorFlip);
		g_aeImx307_MirrorFip[ViPipe] = eSnsMirrorFlip;
	}
}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	IMX307_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsState->bInit = CVI_FALSE;
	pstSnsState->bSyncInit = CVI_FALSE;
	pstSnsState->u8ImgMode = IMX307_MODE_1080P30;
	pstSnsState->enWDRMode = WDR_MODE_NONE;
	pstSnsState->u32FLStd  = IMX307_VMAX_1080P30_LINEAR;
	pstSnsState->au32FL[0] = IMX307_VMAX_1080P30_LINEAR;
	pstSnsState->au32FL[1] = IMX307_VMAX_1080P30_LINEAR;

	memset(&pstSnsState->astSyncInfo[0], 0, sizeof(ISP_SNS_SYNC_INFO_S));
	memset(&pstSnsState->astSyncInfo[1], 0, sizeof(ISP_SNS_SYNC_INFO_S));
}

static CVI_S32 sensor_rx_attr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *pstRxAttr)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	IMX307_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pstRxAttr);

	memcpy(pstRxAttr, &imx307_rx_attr, sizeof(*pstRxAttr));

	pstRxAttr->img_size.width = g_astImx307_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Width;
	pstRxAttr->img_size.height = g_astImx307_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Height;
	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;
	}

	return CVI_SUCCESS;

}

static CVI_S32 sensor_patch_rx_attr(RX_INIT_ATTR_S *pstRxInitAttr)
{
	SNS_COMBO_DEV_ATTR_S *pstRxAttr = &imx307_rx_attr;
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

	pstSensorExpFunc->pfn_cmos_sensor_init = imx307_init;
	pstSensorExpFunc->pfn_cmos_sensor_exit = imx307_exit;
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

static CVI_S32 imx307_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
	g_aunImx307_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	IMX307_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

	if (pastSnsStateCtx == CVI_NULL) {
		pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
		if (pastSnsStateCtx == CVI_NULL) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
			return -ENOMEM;
		}
	}

	memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

	IMX307_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

	return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	IMX307_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
	SENSOR_FREE(pastSnsStateCtx);
	IMX307_SENSOR_RESET_CTX(ViPipe);
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

	stSnsAttrInfo.eSensorId = IMX307_ID;

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

	// s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, IMX307_ID);
	// if (s32Ret != CVI_SUCCESS) {
	// 	CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
	// 	return s32Ret;
	// }

	// s32Ret = CVI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, IMX307_ID);
	// if (s32Ret != CVI_SUCCESS) {
	// 	CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
	// 	return s32Ret;
	// }

	// s32Ret = CVI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, IMX307_ID);
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
	g_au16Imx307_GainMode[ViPipe] = pstInitAttr->enGainMode;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_probe(VI_PIPE ViPipe)
{
	return imx307_probe(ViPipe);
}

ISP_SNS_OBJ_S stSnsImx307_Obj = {
	.pfnRegisterCallback    = sensor_register_callback,
	.pfnUnRegisterCallback  = sensor_unregister_callback,
	.pfnStandby             = imx307_standby,
	.pfnRestart             = imx307_restart,
	.pfnMirrorFlip          = sensor_mirror_flip,
	.pfnWriteReg            = imx307_write_register,
	.pfnReadReg             = imx307_read_register,
	.pfnSetBusInfo          = imx307_set_bus_info,
	.pfnSetInit             = sensor_set_init,
	.pfnPatchRxAttr         = sensor_patch_rx_attr,
	.pfnPatchI2cAddr        = CVI_NULL,
	.pfnGetRxAttr           = sensor_rx_attr,
	.pfnExpSensorCb         = cmos_init_sensor_exp_function,
	.pfnExpAeCb             = cmos_init_ae_exp_function,
	.pfnSnsProbe            = sensor_probe,
};


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#include "cvi_type.h"
#include "cvi_debug.h"
#include "cvi_comm_sns.h"
#include "cvi_sns_ctrl.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_isp.h"

#include "sc285sl_cmos_ex.h"
#include "sc285sl_cmos_param.h"
#include "cvi_comm_cif.h"

#define DIV_0_TO_1(a) ((0 == (a)) ? 1 : (a))
#define DIV_0_TO_1_FLOAT(a) ((((a) < 1E-10) && ((a) > -1E-10)) ? 1 : (a))
#define SC285SL_ID 0x0285
#define SC285SL_I2C_ADDR_IS_VALID(addr) \
	((addr) == SC285SL_I2C_ADDR_1 || (addr) == SC285SL_I2C_ADDR_2)

/****************************************************************************
 * global variables                                                         *
 ***************************************************************************/
ISP_SNS_STATE_S *g_pastSc285sl[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define SC285SL_SENSOR_GET_CTX(dev, pstCtx) (pstCtx = g_pastSc285sl[dev])
#define SC285SL_SENSOR_SET_CTX(dev, pstCtx) (g_pastSc285sl[dev] = pstCtx)
#define SC285SL_SENSOR_RESET_CTX(dev) (g_pastSc285sl[dev] = CVI_NULL)

ISP_SNS_COMMBUS_U g_aunSc285sl_BusInfo[VI_MAX_PIPE_NUM] = {
	[0] = {.s8I2cDev = 2}, [1 ... VI_MAX_PIPE_NUM - 1] = {.s8I2cDev = -1}};

SC285SL_STATE_S g_astSc285sl_State[VI_MAX_PIPE_NUM] = {{0} };

ISP_SNS_MIRRORFLIP_TYPE_E g_aeSc285sl_MirrorFip[VI_MAX_PIPE_NUM] = {ISP_SNS_NORMAL};

CVI_U16 g_au16Sc285sl_GainMode[VI_MAX_PIPE_NUM] = {0};
CVI_U16 g_au16Sc285sl_UseHwSync[VI_MAX_PIPE_NUM] = {0};

/****************************************************************************
 * local variables and functions                                            *
 ***************************************************************************/
static ISP_FSWDR_MODE_E genFSWDRMode[VI_MAX_PIPE_NUM] = {
	[0 ... VI_MAX_PIPE_NUM - 1] = ISP_FSWDR_NORMAL_MODE
};
static CVI_U32 gu32MaxTimeGetCnt[VI_MAX_PIPE_NUM] = {0};
static CVI_U32 g_au32InitExposure[VI_MAX_PIPE_NUM] = {0};
static CVI_U32 g_au32LinesPer500ms[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16InitWBGain[VI_MAX_PIPE_NUM][3] = {{0}};
static CVI_U16 g_au16SampleRgain[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16SampleBgain[VI_MAX_PIPE_NUM] = {0};
static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg);
/*****Sc285sl Lines Range*****/
#define SC285SL_FULL_LINES_MAX (0x3FFFF0)

/*****Sc285sl Register Address*****/
/*****HDR long exposure & linear exposure regs*****/
#define SC285SL_SHS1_0_ADDR 0x3E20
#define SC285SL_SHS1_1_ADDR 0x3E00
#define SC285SL_SHS1_2_ADDR 0x3E01
#define SC285SL_SHS1_3_ADDR 0x3E02

/*****HDR short exp Register Address*****/
#define SC285SL_SHS2_0_ADDR 0x3E21
#define SC285SL_SHS2_1_ADDR 0x3E22
#define SC285SL_SHS2_2_ADDR 0x3E04
#define SC285SL_SHS2_3_ADDR 0x3E05

#define SC285SL_AGAIN_ADDR 0x3E08
#define SC285SL_AGAIN_FINE_ADDR 0x3E09
#define SC285SL_DGAIN_ADDR 0x3E06
#define SC285SL_DGAIN_FINE_ADDR 0x3E07

#define SC285SL_WDR_AGAIN_ADDR 0x3E12
#define SC285SL_WDR_AGAIN_FINE_ADDR 0x3E13
#define SC285SL_WDR_DGAIN_ADDR 0x3E10
#define SC285SL_WDR_DGAIN_FINE_ADDR 0x3E11

#define SC285SL_VMAX_1_ADDR 0x326D
#define SC285SL_VMAX_2_ADDR 0x320E
#define SC285SL_VMAX_3_ADDR 0x320F

#define SC533HAI_MAXSEXP_H_ADDR		0x3033 //(0x3033[0:0],0x3E23[7:0],0x3E24[7:0])
#define SC533HAI_MAXSEXP_M_ADDR		0x3E23
#define SC533HAI_MAXSEXP_L_ADDR		0x3E24

#define SC285SL_GROUP_HOLD_ADDR	0x3812

#define SC285SL_RES_IS_1080P(w, h) ((w) <= 1920 && (h) <= 1080)

static CVI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	const SC285SL_MODE_S *pstMode;

	CVI_U32 FpsMax;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	SC285SL_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstMode = &g_astSc285sl_mode[pstSnsState->u8ImgMode];
	FpsMax = g_astSc285sl_mode[pstSnsState->u8ImgMode].f32MaxFps;

	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FlickerFreq = 50 * 256;
	pstAeSnsDft->u32FullLinesMax = SC285SL_FULL_LINES_MAX;
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * FpsMax);

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
		pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * FpsMax / 2;
	else
		pstAeSnsDft->u32LinesPer500ms = g_au32LinesPer500ms[ViPipe];
	switch (pstSnsState->enWDRMode) {
	default:
	case WDR_MODE_NONE: /*linear mode*/
		pstAeSnsDft->f32Fps = pstMode->f32MaxFps;
		pstAeSnsDft->f32MinFps = pstMode->f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xd;
		pstAeSnsDft->au8HistThresh[1] = 0x28;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxAgain = pstMode->stAgain[0].u32Max;
		pstAeSnsDft->u32MinAgain = pstMode->stAgain[0].u32Min;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = pstMode->stDgain[0].u32Max;
		pstAeSnsDft->u32MinDgain = pstMode->stDgain[0].u32Min;
		pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
		pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

		pstAeSnsDft->u8AeCompensation = 40;
		pstAeSnsDft->u32InitAESpeed = 64;
		pstAeSnsDft->u32InitAETolerance = 5;
		pstAeSnsDft->u32AEResponseFrame = 3; //AEResponseFrame measurement = 3
		pstAeSnsDft->u32SnsResponseFrame = 5;
		pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		pstAeSnsDft->u32InitExposure =
			g_au32InitExposure[ViPipe] ? g_au32InitExposure[ViPipe] : pstMode->stExp[0].u16Def;

		pstAeSnsDft->u32MaxIntTime = pstMode->stExp[0].u16Max;
		pstAeSnsDft->u32MinIntTime = pstMode->stExp[0].u16Min;
		pstAeSnsDft->u32MaxIntTimeTarget = 65535;
		pstAeSnsDft->u32MinIntTimeTarget = 1;

		break;
	case WDR_MODE_2To1_LINE:
		pstAeSnsDft->f32Fps = pstMode->f32MaxFps;
		pstAeSnsDft->f32MinFps = pstMode->f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0x0c;
		pstAeSnsDft->au8HistThresh[1] = 0x18;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxAgain = pstMode->stAgain[0].u32Max;
		pstAeSnsDft->u32MinAgain = pstMode->stAgain[0].u32Min;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = pstMode->stDgain[0].u32Max;
		pstAeSnsDft->u32MinDgain = pstMode->stDgain[0].u32Min;
		pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
		pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

		pstAeSnsDft->u8AeCompensation = 40;
		pstAeSnsDft->u32InitAESpeed = 64;
		pstAeSnsDft->u32InitAETolerance = 5;
		pstAeSnsDft->u32AEResponseFrame = 4;//AEResponseFrame measurement = 4
		pstAeSnsDft->u32SnsResponseFrame = 4;
		pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		pstAeSnsDft->u32InitExposure =
			g_au32InitExposure[ViPipe] ?
			g_au32InitExposure[ViPipe] : pstMode->stExp[0].u16Def;

		pstAeSnsDft->u32MaxIntTime = pstMode->stExp[0].u16Max;
		pstAeSnsDft->u32MinIntTime = pstMode->stExp[0].u16Min;
		pstAeSnsDft->u32MaxIntTimeTarget = 65535;
		pstAeSnsDft->u32MinIntTimeTarget = 2;
		if (genFSWDRMode[ViPipe] == ISP_FSWDR_LONG_FRAME_MODE) {
			pstAeSnsDft->u8AeCompensation = 64;
			pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		} else {
			pstAeSnsDft->u8AeCompensation = 40;
			pstAeSnsDft->enAeExpMode = AE_EXP_LOWLIGHT_PRIOR;
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
	CVI_U16 u16MaxSexpReg;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	SC285SL_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32Vts = g_astSc285sl_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	f32MaxFps = g_astSc285sl_mode[pstSnsState->u8ImgMode].f32MaxFps;
	f32MinFps = g_astSc285sl_mode[pstSnsState->u8ImgMode].f32MinFps;
	u16MaxSexpReg = g_astSc285sl_mode[pstSnsState->u8ImgMode].u16SexpMaxReg;

	if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
		u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport Fps: %f\n", f32Fps);
		return CVI_FAILURE;
	}

	u32VMAX = (u32VMAX > SC285SL_FULL_LINES_MAX) ? SC285SL_FULL_LINES_MAX : u32VMAX;

	if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) {
		u16MaxSexpReg = u16MaxSexpReg * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
		g_astSc285sl_State[ViPipe].u32Sexp_MAX = u16MaxSexpReg;
	}

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_0_ADDR].u32Data = ((u32VMAX & 0x3F0000) >> 16);
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_1_ADDR].u32Data = ((u32VMAX & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_2_ADDR].u32Data = (u32VMAX & 0xFF);
	} else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
		pstSnsRegsInfo->astI2cData[WDR_VMAX_0_ADDR].u32Data = ((u32VMAX & 0x3F0000) >> 16);
		pstSnsRegsInfo->astI2cData[WDR_VMAX_1_ADDR].u32Data = ((u32VMAX & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[WDR_VMAX_2_ADDR].u32Data = (u32VMAX & 0xFF);
		pstSnsRegsInfo->astI2cData[WDR2_MAXSEXP_H_ADDR].u32Data = ((u16MaxSexpReg & 0x10000) >> 16);
		pstSnsRegsInfo->astI2cData[WDR2_MAXSEXP_M_ADDR].u32Data = ((u16MaxSexpReg & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[WDR2_MAXSEXP_L_ADDR].u32Data = (u16MaxSexpReg & 0xFF);
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport Mode: %d\n", pstSnsState->enWDRMode);
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

static CVI_S32 cmos_inttime_update(VI_PIPE ViPipe, CVI_U32 *u32IntTime)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;

	SC285SL_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(u32IntTime);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		CVI_U32 u32TmpIntTime = u32IntTime[0];
		CVI_U32 maxExp = pstSnsState->au32FL[0] - 8;

		/* linear exposure reg range:
		 * min : 1
		 * max : vts - 8
		 * step : 1
		 */
		u32TmpIntTime = (u32TmpIntTime > maxExp) ? maxExp : u32TmpIntTime;
		if (u32TmpIntTime < 1)
			u32TmpIntTime = 1;
		u32IntTime[0] = u32TmpIntTime;

		pstSnsRegsInfo->astI2cData[LINEAR_SHS1_0_ADDR].u32Data = (u32IntTime[0] >> 20) & 0x03;	// [21:20]
		pstSnsRegsInfo->astI2cData[LINEAR_SHS1_1_ADDR].u32Data = (u32IntTime[0] >> 12) & 0xFF;	// [19:12]
		pstSnsRegsInfo->astI2cData[LINEAR_SHS1_2_ADDR].u32Data = (u32IntTime[0] >> 4) & 0xFF;	// [11:4]
		pstSnsRegsInfo->astI2cData[LINEAR_SHS1_3_ADDR].u32Data = (u32IntTime[0] & 0x0F) << 4;	// [3:0]
	} else {
		CVI_U32 u32ShortIntTime = u32IntTime[0];
		CVI_U32 u32LongIntTime = u32IntTime[1];
		CVI_U16 u16SexpReg, u16LexpReg;
		CVI_U32 u32MaxLExp;
		CVI_U32 u32MaxSexp = g_astSc285sl_State[ViPipe].u32Sexp_MAX - 11;

		/* short exposure reg range:
		 * min : 2
		 * max : reg_sexp_max - 11
		 * step : 2
		 */

		pstSnsState->au32WDRIntTime[0] = (u32ShortIntTime > u32MaxSexp) ? u32MaxSexp : u32ShortIntTime;
		if (pstSnsState->au32WDRIntTime[0] < 2)
			pstSnsState->au32WDRIntTime[0] = 2;
		u16SexpReg = ((CVI_U16)pstSnsState->au32WDRIntTime[0] & ~0x1) + 1;
		pstSnsState->au32WDRIntTime[0] =  u16SexpReg;
		u32IntTime[0] = pstSnsState->au32WDRIntTime[0];
		/* long exposure reg range:
		 * min : 2
		 * max : (vts - reg_sexp_max) - 13
		 * step : 2
		 */
		u32MaxLExp = (pstSnsState->au32FL[0] - g_astSc285sl_State[ViPipe].u32Sexp_MAX) - 13;
		pstSnsState->au32WDRIntTime[1] = (u32LongIntTime > u32MaxLExp) ? u32MaxLExp : u32LongIntTime;
		if (pstSnsState->au32WDRIntTime[1] < 2)
			pstSnsState->au32WDRIntTime[1] = 2;
		u16LexpReg = ((CVI_U16)pstSnsState->au32WDRIntTime[1] & ~0x1);
		pstSnsState->au32WDRIntTime[1] = u16LexpReg;
		u32IntTime[1] = pstSnsState->au32WDRIntTime[1];

		/* short exposure regs: {3e21[1:0],3e22,3e04,3e05[7:4]} */
		pstSnsRegsInfo->astI2cData[WDR_SHS2_0_ADDR].u32Data = (u16SexpReg >> 20) & 0x03;
		pstSnsRegsInfo->astI2cData[WDR_SHS2_1_ADDR].u32Data = (u16SexpReg >> 12) & 0xFF;
		pstSnsRegsInfo->astI2cData[WDR_SHS2_2_ADDR].u32Data = (u16SexpReg >> 4) & 0xFF;
		pstSnsRegsInfo->astI2cData[WDR_SHS2_3_ADDR].u32Data = (u16SexpReg & 0x0F) << 4;

		/* long exposure regs: {3e20[1:0],3e00,3e01,3e02[7:4]} */
		pstSnsRegsInfo->astI2cData[WDR_SHS1_0_ADDR].u32Data = (u16LexpReg >> 20) & 0x03;
		pstSnsRegsInfo->astI2cData[WDR_SHS1_1_ADDR].u32Data = (u16LexpReg >> 12) & 0xFF;
		pstSnsRegsInfo->astI2cData[WDR_SHS1_2_ADDR].u32Data = (u16LexpReg >> 4) & 0xFF;
		pstSnsRegsInfo->astI2cData[WDR_SHS1_3_ADDR].u32Data = (u16LexpReg & 0x0F) << 4;
	}

	return CVI_SUCCESS;
}

typedef struct gain_tbl_info_s {
	CVI_U32 gainMax;
	CVI_U16 idxBase;
	CVI_U8 regGain;
	CVI_U8 regGainFineBase;
	CVI_U8 regGainFineStep;
} gain_tbl_info_s;

static struct gain_tbl_info_s DgainInfo[] = {
	{
		.gainMax = 2032,
		.idxBase = 0,
		.regGain = 0x00,
		.regGainFineBase = 0x80,
		.regGainFineStep = 2,
	},
	{
		.gainMax = 4064,
		.idxBase = 64,
		.regGain = 0x01,
		.regGainFineBase = 0x80,
		.regGainFineStep = 2,
	},
	{
		.gainMax = 8128,
		.idxBase = 128,
		.regGain = 0x03,
		.regGainFineBase = 0x80,
		.regGainFineStep = 2,
	},
	{
		.gainMax = 16256,
		.idxBase = 192,
		.regGain = 0x07,
		.regGainFineBase = 0x80,
		.regGainFineStep = 2,
	},
};

static CVI_U32 Dgain_table[] = {
	1024, 1040, 1056, 1072, 1088, 1104, 1120, 1136, 1152, 1168, 1184, 1200, 1216, 1232, 1248, 1264,
	1280, 1296, 1312, 1328, 1344, 1360, 1376, 1392, 1408, 1424, 1440, 1456, 1472, 1488, 1504, 1520,
	1536, 1552, 1568, 1584, 1600, 1616, 1632, 1648, 1664, 1680, 1696, 1712, 1728, 1744, 1760, 1776,
	1792, 1808, 1824, 1840, 1856, 1872, 1888, 1904, 1920, 1936, 1952, 1968, 1984, 2000, 2016, 2032,
	2048, 2080, 2112, 2144, 2176, 2208, 2240, 2272, 2304, 2336, 2368, 2400, 2432, 2464, 2496, 2528,
	2560, 2592, 2624, 2656, 2688, 2720, 2752, 2784, 2816, 2848, 2880, 2912, 2944, 2976, 3008, 3040,
	3072, 3104, 3136, 3168, 3200, 3232, 3264, 3296, 3328, 3360, 3392, 3424, 3456, 3488, 3520, 3552,
	3584, 3616, 3648, 3680, 3712, 3744, 3776, 3808, 3840, 3872, 3904, 3936, 3968, 4000, 4032, 4064,
	4096, 4160, 4224, 4288, 4352, 4416, 4480, 4544, 4608, 4672, 4736, 4800, 4864, 4928, 4992, 5056,
	5120, 5184, 5248, 5312, 5376, 5440, 5504, 5568, 5632, 5696, 5760, 5824, 5888, 5952, 6016, 6080,
	6144, 6208, 6272, 6336, 6400, 6464, 6528, 6592, 6656, 6720, 6784, 6848, 6912, 6976, 7040, 7104,
	7168, 7232, 7296, 7360, 7424, 7488, 7552, 7616, 7680, 7744, 7808, 7872, 7936, 8000, 8064, 8128,
	8192, 8320, 8448, 8576, 8704, 8832, 8960, 9088, 9216, 9344, 9472, 9600, 9728, 9856, 9984, 10112,
	10240, 10368, 10496, 10624, 10752, 10880, 11008, 11136, 11264, 11392, 11520, 11648, 11776, 11904, 12032, 12160,
	12288, 12416, 12544, 12672, 12800, 12928, 13056, 13184, 13312, 13440, 13568, 13696, 13824, 13952, 14080, 14208,
	14336, 14464, 14592, 14720, 14848, 14976, 15104, 15232, 15360, 15488, 15616, 15744, 15872, 16000, 16128, 16256
};

static struct gain_tbl_info_s AgainInfo[] = {
	{
		.gainMax = 2016,
		.idxBase = 0,
		.regGain = 0x00,
		.regGainFineBase = 0x20,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 4032,
		.idxBase = 32,
		.regGain = 0x01,
		.regGainFineBase = 0x20,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 5504,
		.idxBase = 64,
		.regGain = 0x03,
		.regGainFineBase = 0x20,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 10886,
		.idxBase = 76,
		.regGain = 0x80,
		.regGainFineBase = 0x20,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 21773,
		.idxBase = 108,
		.regGain = 0x81,
		.regGainFineBase = 0x20,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 43545,
		.idxBase = 140,
		.regGain = 0x83,
		.regGainFineBase = 0x20,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 87091,
		.idxBase = 172,
		.regGain = 0x87,
		.regGainFineBase = 0x20,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 174182,
		.idxBase = 204,
		.regGain = 0x8f,
		.regGainFineBase = 0x80,
		.regGainFineStep = 1,
	},
};

static CVI_U32 Again_table[] = {
	/*Again 1x ~ 2x*/
	1024, 1056, 1088, 1120, 1152, 1183, 1216, 1248,
	1280, 1311, 1344, 1376, 1408, 1439, 1472, 1504,
	1536, 1567, 1600, 1632, 1664, 1695, 1728, 1760,
	1792, 1823, 1856, 1888, 1920, 1951, 1984, 2016,
	/*Again 2x ~ 4x*/
	2048, 2112, 2176, 2240, 2304, 2368, 2432, 2496,
	2560, 2624, 2688, 2752, 2816, 2880, 2944, 3008,
	3072, 3136, 3200, 3264, 3328, 3392, 3456, 3520,
	3584, 3648, 3712, 3776, 3840, 3904, 3968, 4032,
	/*Again 4x ~ 8x*/
	4096, 4224, 4352, 4480, 4608, 4736, 4864, 4992,
	5120, 5248, 5376, 5504, 5536, 5702, 5875, 6047,
	6220, 6393, 6566, 6738, 6912, 7085, 7258, 7430,
	7603, 7776, 7949, 8121, 8294, 8467, 8640, 8812,
	/*Again 8x ~ 16x*/
	8985, 9158, 9331, 9503, 9676, 9849, 10022, 10194,
	10368, 10541, 10714, 10886, 11059, 11405, 11750, 12096,
	12441, 12787, 13132, 13478, 13824, 14170, 14515, 14861,
	15206, 15552, 15897, 16243, 
	/*Again 16x ~ 32x*/
	16588, 16934, 17280, 17626, 17971, 18317, 18662, 19008,
	19353, 19699, 20044, 20390, 20736, 21082, 21427, 21773,
	22118, 22809, 23500, 24192, 24883, 25574, 26265, 26956,
	27648, 28339, 29030, 29721, 30412, 31104, 31795, 32486,
	/*Again 32x ~ 64x*/
	33177, 33868, 34560, 35251,
	35942, 36633, 37324, 38016, 38707, 39398, 40089, 40780,
	41472, 42163, 42854, 43545, 44236, 45619, 47001, 48384,
	49766, 51148, 52531, 53913, 55296, 56678, 58060, 59443,
	60825, 62208, 63590, 64972,
	/*Again 64x ~ 128x*/
	66355, 67737, 69120, 70502, 71884, 73267, 74649, 76032,
	77414, 78796, 80179, 81561, 82944, 84326, 85708, 87091,
	88473, 91238, 94003, 96768, 99532, 102297, 105062, 107827,
	110592, 113356, 116121, 118886, 121651, 124416, 127180, 129945,
	/*Again 128x ~ 170x*/
	132710, 135475, 138240, 141004, 143769, 146534, 149299,
	152064, 154828, 157593, 160358, 163123, 165888, 168652,
	171417, 174182
};

static const CVI_U32 again_table_size = ARRAY_SIZE(Again_table);
static const CVI_U32 dgain_table_size = ARRAY_SIZE(Dgain_table);

static CVI_S32 cmos_again_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32AgainLin, CVI_U32 *pu32AgainDb)
{
	CVI_U32 i;

	UNUSED(ViPipe);

	CMOS_CHECK_POINTER(pu32AgainLin);
	CMOS_CHECK_POINTER(pu32AgainDb);

	if (*pu32AgainLin >= Again_table[again_table_size - 1]) {
		*pu32AgainLin = Again_table[again_table_size - 1];
		*pu32AgainDb = again_table_size - 1;
		return CVI_SUCCESS;
	}

	for (i = 1; i < again_table_size; i++) {
		if (*pu32AgainLin < Again_table[i]) {
			*pu32AgainLin = Again_table[i - 1];
			*pu32AgainDb = i - 1;
			break;
		}
	}
	return CVI_SUCCESS;
}

static CVI_S32 cmos_dgain_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32DgainLin, CVI_U32 *pu32DgainDb)
{
	int i;

	UNUSED(ViPipe);

	CMOS_CHECK_POINTER(pu32DgainLin);
	CMOS_CHECK_POINTER(pu32DgainDb);

	if (*pu32DgainLin >= Dgain_table[dgain_table_size - 1]) {
		*pu32DgainLin = Dgain_table[dgain_table_size - 1];
		*pu32DgainDb = dgain_table_size -1;
		return CVI_SUCCESS;
	}

	for (i = 1; i < dgain_table_size; i++) {
		if (*pu32DgainLin < Dgain_table[i]) {
			*pu32DgainLin = Dgain_table[i - 1];
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
	CVI_U32 u32Again;
	CVI_U32 u32Dgain;
	CVI_U32 u32AgainFine;
	CVI_U32 u32DgainFine;
	struct gain_tbl_info_s *info;
	int i, tbl_num;

	SC285SL_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pu32Again);
	CMOS_CHECK_POINTER(pu32Dgain);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	u32Again = pu32Again[0];
	u32Dgain = pu32Dgain[0];

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		/* find Again register setting. */
		tbl_num = sizeof(AgainInfo)/sizeof(struct gain_tbl_info_s);
		for (i = tbl_num - 1; i >= 0; i--) {
			info = &AgainInfo[i];

			if (u32Again >= info->idxBase)
				break;
		}

		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_ADDR].u32Data = (info->regGain & 0xFF);
		u32AgainFine = info->regGainFineBase + (u32Again - info->idxBase) * info->regGainFineStep;
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_FINE_ADDR].u32Data = (u32AgainFine & 0xFF);

		/* find Dgain register setting. */
		tbl_num = sizeof(DgainInfo) / sizeof(struct gain_tbl_info_s);
		for (i = tbl_num - 1; i >= 0; i--) {
			info = &DgainInfo[i];

			if (u32Dgain >= info->idxBase)
				break;
		}

		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_ADDR].u32Data = (info->regGain & 0xFF);
		u32DgainFine = info->regGainFineBase + (u32Dgain - info->idxBase) * info->regGainFineStep;
		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_FINE_ADDR].u32Data = (u32DgainFine & 0xFF);
	} else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
		/* find SEF Again register setting. */
		tbl_num = sizeof(AgainInfo)/sizeof(struct gain_tbl_info_s);
		for (i = tbl_num - 1; i >= 0; i--) {
			info = &AgainInfo[i];

			if (u32Again >= info->idxBase)
				break;
		}

		u32AgainFine = info->regGainFineBase + (u32Again - info->idxBase) * info->regGainFineStep;
		/* short exposure gain */
		pstSnsRegsInfo->astI2cData[WDR_AGAIN_S_ADDR].u32Data = (info->regGain & 0xFF);
		pstSnsRegsInfo->astI2cData[WDR_AGAIN_S_FINE_ADDR].u32Data = (u32AgainFine & 0xFF);

		/* find SEF Dgain register setting. */
		tbl_num = sizeof(DgainInfo) / sizeof(struct gain_tbl_info_s);
		for (i = tbl_num - 1; i >= 0; i--) {
			info = &DgainInfo[i];
			if (u32Dgain >= info->idxBase)
				break;
		}
		u32DgainFine = info->regGainFineBase + (u32Dgain - info->idxBase) * info->regGainFineStep;
		/* short exposure dgain */
		pstSnsRegsInfo->astI2cData[WDR_DGAIN_S_ADDR].u32Data = (info->regGain & 0xFF);
		pstSnsRegsInfo->astI2cData[WDR_DGAIN_S_FINE_ADDR].u32Data = (u32DgainFine & 0xFF);

		u32Again = pu32Again[1];
		u32Dgain = pu32Dgain[1];
		/* find LEF Again register setting. */
		tbl_num = sizeof(AgainInfo)/sizeof(struct gain_tbl_info_s);
		for (i = tbl_num - 1; i >= 0; i--) {
			info = &AgainInfo[i];

			if (u32Again >= info->idxBase)
				break;
		}
		/* long exposure gain */
		u32AgainFine = info->regGainFineBase + (u32Again - info->idxBase) * info->regGainFineStep;
		pstSnsRegsInfo->astI2cData[WDR_AGAIN_L_ADDR].u32Data = (info->regGain & 0xFF);
		pstSnsRegsInfo->astI2cData[WDR_AGAIN_L_FINE_ADDR].u32Data = (u32AgainFine & 0xFF);

		/* find LEF Dgain register setting. */
		tbl_num = sizeof(DgainInfo)/sizeof(struct gain_tbl_info_s);
		for (i = tbl_num - 1; i >= 0; i--) {
			info = &DgainInfo[i];

			if (u32Dgain >= info->idxBase)
				break;
		}
		/* long exposure dgain */
		u32DgainFine = info->regGainFineBase + (u32Dgain - info->idxBase) * info->regGainFineStep;
		pstSnsRegsInfo->astI2cData[WDR_DGAIN_L_ADDR].u32Data = (info->regGain & 0xFF);
		pstSnsRegsInfo->astI2cData[WDR_DGAIN_L_FINE_ADDR].u32Data = (u32DgainFine & 0xFF);
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support WDR: %d\n", pstSnsState->enWDRMode);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_inttime_max(VI_PIPE ViPipe, CVI_U16 u16ManRatioEnable, CVI_U32 *au32Ratio,
		CVI_U32 *au32IntTimeMax, CVI_U32 *au32IntTimeMin, CVI_U32 *pu32LFMaxIntTime)
{
	CVI_U32 u32IntTimeMaxTmp  = 0, u32IntTimeMaxTmp0 = 0;
	CVI_U32 u32ShortTimeMinLimit = 4;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	CVI_U32 u32MaxSexp = g_astSc285sl_State[ViPipe].u32Sexp_MAX - 11;

	(void) u16ManRatioEnable;

	CMOS_CHECK_POINTER(au32Ratio);
	CMOS_CHECK_POINTER(au32IntTimeMax);
	CMOS_CHECK_POINTER(au32IntTimeMin);
	CMOS_CHECK_POINTER(pu32LFMaxIntTime);
	SC285SL_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32ShortTimeMinLimit = 2;
	/*
	 * Long exp + Short exp < VTS - 25
	 */
	u32IntTimeMaxTmp0 = ((pstSnsState->au32FL[1] - 24 - pstSnsState->au32WDRIntTime[0]) * 0x40) /
					DIV_0_TO_1(au32Ratio[0]);
	u32IntTimeMaxTmp  = ((pstSnsState->au32FL[0] - 24) * 0x40)  / DIV_0_TO_1(au32Ratio[0] + 0x40);
	u32IntTimeMaxTmp = (u32IntTimeMaxTmp > u32IntTimeMaxTmp0) ? u32IntTimeMaxTmp0 : u32IntTimeMaxTmp;

	u32IntTimeMaxTmp  = (u32MaxSexp < u32IntTimeMaxTmp) ?
					u32MaxSexp : u32IntTimeMaxTmp;
	u32IntTimeMaxTmp  = (!u32IntTimeMaxTmp) ? u32ShortTimeMinLimit : u32IntTimeMaxTmp;
	CVI_TRACE_SNS(CVI_DBG_DEBUG, "Max inttime0 = %u\n", u32IntTimeMaxTmp);

	if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
		au32IntTimeMax[0] = u32IntTimeMaxTmp;
		au32IntTimeMax[1] = au32IntTimeMax[0] * au32Ratio[0] >> 6;
		au32IntTimeMax[2] = au32IntTimeMax[1] * au32Ratio[1] >> 6;
		au32IntTimeMax[3] = au32IntTimeMax[2] * au32Ratio[2] >> 6;
		au32IntTimeMin[0] = u32ShortTimeMinLimit;
		au32IntTimeMin[1] = au32IntTimeMin[0] * au32Ratio[0] >> 6;
		au32IntTimeMin[2] = au32IntTimeMin[1] * au32Ratio[1] >> 6;
		au32IntTimeMin[3] = au32IntTimeMin[2] * au32Ratio[2] >> 6;
		CVI_TRACE_SNS(CVI_DBG_DEBUG, "ViPipe = %d ratio = %d, (%d, %d)\n", ViPipe, au32Ratio[0],
				u32IntTimeMaxTmp, u32ShortTimeMinLimit);
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
	pstExpFuncs->pfn_cmos_get_ae_default = cmos_get_ae_default;
	pstExpFuncs->pfn_cmos_fps_set = cmos_fps_set;
	pstExpFuncs->pfn_cmos_inttime_update = cmos_inttime_update;
	pstExpFuncs->pfn_cmos_gains_update = cmos_gains_update;
	pstExpFuncs->pfn_cmos_again_calc_table = cmos_again_calc_table;
	pstExpFuncs->pfn_cmos_dgain_calc_table = cmos_dgain_calc_table;
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

	memcpy(pstBlc, &g_stIspBlcCalibratio, sizeof(ISP_CMOS_BLACK_LEVEL_S));
	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg)
{
	const SC285SL_MODE_S *pstMode = CVI_NULL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC285SL_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstMode = &g_astSc285sl_mode[pstSnsState->u8ImgMode];

	if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
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

	SC285SL_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstSnsState->bSyncInit = CVI_FALSE;

	switch (u8Mode) {
	case WDR_MODE_NONE:
		pstSnsState->enWDRMode = WDR_MODE_NONE;
		pstSnsState->u8ImgMode = SC285SL_MODE_1920X1080P30;
		break;
	case WDR_MODE_2To1_LINE:
		pstSnsState->enWDRMode = WDR_MODE_2To1_LINE;
		pstSnsState->u8ImgMode = SC285SL_MODE_1920X1080P30_WDR;
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport sensor mode: %d\n", u8Mode);
		return CVI_FAILURE;
	}

	pstSnsState->u32FLStd = g_astSc285sl_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
	pstSnsState->au32FL[1] = pstSnsState->u32FLStd;

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

static CVI_U32 sensor_cmp_cif_wdr(ISP_SNS_CIF_INFO_S *pstWdr1, ISP_SNS_CIF_INFO_S *pstWdr2)
{
	if (pstWdr1->wdr_manual.l2s_distance != pstWdr2->wdr_manual.l2s_distance)
		goto _mismatch;
	if (pstWdr1->wdr_manual.lsef_length != pstWdr2->wdr_manual.lsef_length)
		goto _mismatch;

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
	SC285SL_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstSnsRegsInfo = &pstSnsSyncInfo->snsCfg;
	pstCfg0 = &pstSnsState->astSyncInfo[0];
	pstCfg1 = &pstSnsState->astSyncInfo[1];
	pstI2c_data = pstCfg0->snsCfg.astI2cData;

	if ((pstSnsState->bSyncInit == CVI_FALSE) || (pstSnsRegsInfo->bConfig == CVI_FALSE)) {
		pstCfg0->snsCfg.enSnsType = SNS_I2C_TYPE;
		pstCfg0->snsCfg.unComBus.s8I2cDev = g_aunSc285sl_BusInfo[ViPipe].s8I2cDev;
		pstCfg0->snsCfg.u8Cfg2ValidDelayMax = 0;
		pstCfg0->snsCfg.use_snsr_sram = CVI_TRUE;
		pstCfg0->snsCfg.u32RegNum = (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) ?
					WDR_REGS_NUM : LINEAR_REGS_NUM;

		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			pstI2c_data[i].bUpdate = CVI_TRUE;
			pstI2c_data[i].u8DevAddr = sc285sl_i2c_addr;
			pstI2c_data[i].u32AddrByteNum = sc285sl_addr_byte;
			pstI2c_data[i].u32DataByteNum = sc285sl_data_byte;
		}

		switch (pstSnsState->enWDRMode) {
		case WDR_MODE_NONE:
			// Linear Mode Regs
			pstI2c_data[WDR_HOLD_START].u32RegAddr     = SC285SL_GROUP_HOLD_ADDR;
			pstI2c_data[WDR_HOLD_START].u32Data = 0x00;
			pstI2c_data[LINEAR_SHS1_0_ADDR].u32RegAddr = SC285SL_SHS1_0_ADDR;
			pstI2c_data[LINEAR_SHS1_1_ADDR].u32RegAddr = SC285SL_SHS1_1_ADDR;
			pstI2c_data[LINEAR_SHS1_2_ADDR].u32RegAddr = SC285SL_SHS1_2_ADDR;
			pstI2c_data[LINEAR_SHS1_3_ADDR].u32RegAddr = SC285SL_SHS1_3_ADDR;
			pstI2c_data[LINEAR_AGAIN_ADDR].u32RegAddr = SC285SL_AGAIN_ADDR;
			pstI2c_data[LINEAR_AGAIN_FINE_ADDR].u32RegAddr = SC285SL_AGAIN_FINE_ADDR;
			pstI2c_data[LINEAR_DGAIN_ADDR].u32RegAddr = SC285SL_DGAIN_ADDR;
			pstI2c_data[LINEAR_DGAIN_FINE_ADDR].u32RegAddr = SC285SL_DGAIN_FINE_ADDR;
			pstI2c_data[LINEAR_VMAX_0_ADDR].u32RegAddr = SC285SL_VMAX_1_ADDR;
			pstI2c_data[LINEAR_VMAX_1_ADDR].u32RegAddr = SC285SL_VMAX_2_ADDR;
			pstI2c_data[LINEAR_VMAX_2_ADDR].u32RegAddr = SC285SL_VMAX_3_ADDR;
			pstI2c_data[WDR_HOLD_END].u32RegAddr     = SC285SL_GROUP_HOLD_ADDR;
			pstI2c_data[WDR_HOLD_END].u32Data = 0xa0;
			break;
		case WDR_MODE_2To1_LINE:
			pstI2c_data[WDR_HOLD_START].u32RegAddr     = SC285SL_GROUP_HOLD_ADDR;
			pstI2c_data[WDR_HOLD_START].u32Data = 0x00;
			/* long exposure regs */
			pstI2c_data[WDR_SHS1_0_ADDR].u32RegAddr = SC285SL_SHS1_0_ADDR;
			pstI2c_data[WDR_SHS1_1_ADDR].u32RegAddr = SC285SL_SHS1_1_ADDR;
			pstI2c_data[WDR_SHS1_2_ADDR].u32RegAddr = SC285SL_SHS1_2_ADDR;
			pstI2c_data[WDR_SHS1_3_ADDR].u32RegAddr = SC285SL_SHS1_3_ADDR;
			/* short exposure regs */
			pstI2c_data[WDR_SHS2_0_ADDR].u32RegAddr = SC285SL_SHS2_0_ADDR;
			pstI2c_data[WDR_SHS2_1_ADDR].u32RegAddr = SC285SL_SHS2_1_ADDR;
			pstI2c_data[WDR_SHS2_2_ADDR].u32RegAddr = SC285SL_SHS2_2_ADDR;
			pstI2c_data[WDR_SHS2_3_ADDR].u32RegAddr = SC285SL_SHS2_3_ADDR;

			/* long exposure gains */
			pstI2c_data[WDR_AGAIN_L_ADDR].u32RegAddr = SC285SL_AGAIN_ADDR;
			pstI2c_data[WDR_AGAIN_L_FINE_ADDR].u32RegAddr = SC285SL_AGAIN_FINE_ADDR;
			pstI2c_data[WDR_DGAIN_L_ADDR].u32RegAddr = SC285SL_DGAIN_ADDR;
			pstI2c_data[WDR_DGAIN_L_FINE_ADDR].u32RegAddr = SC285SL_DGAIN_FINE_ADDR;
			/* short exposure gains */
			pstI2c_data[WDR_AGAIN_S_ADDR].u32RegAddr = SC285SL_WDR_AGAIN_ADDR;
			pstI2c_data[WDR_AGAIN_S_FINE_ADDR].u32RegAddr = SC285SL_WDR_AGAIN_FINE_ADDR;
			pstI2c_data[WDR_DGAIN_S_ADDR].u32RegAddr = SC285SL_WDR_DGAIN_ADDR;
			pstI2c_data[WDR_DGAIN_S_FINE_ADDR].u32RegAddr = SC285SL_WDR_DGAIN_FINE_ADDR;

			pstI2c_data[WDR_VMAX_0_ADDR].u32RegAddr = SC285SL_VMAX_1_ADDR;
			pstI2c_data[WDR_VMAX_1_ADDR].u32RegAddr = SC285SL_VMAX_2_ADDR;
			pstI2c_data[WDR_VMAX_2_ADDR].u32RegAddr = SC285SL_VMAX_3_ADDR;
			pstI2c_data[WDR_HOLD_END].u32RegAddr     = SC285SL_GROUP_HOLD_ADDR;
			pstI2c_data[WDR_HOLD_END].u32Data = 0xa0;
			break;
		default:
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support WDR: %d\n", pstSnsState->enWDRMode);
			return CVI_FAILURE;
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

		/* check update isp crop or not */
		pstCfg0->ispCfg.need_update =
			(sensor_cmp_wdr_size(&pstCfg0->ispCfg, &pstCfg1->ispCfg) ? CVI_TRUE : CVI_FALSE);

		/* check update cif wdr manual or not */
		pstCfg0->cifCfg.need_update =
			(sensor_cmp_cif_wdr(&pstCfg0->cifCfg, &pstCfg1->cifCfg) ? CVI_TRUE : CVI_FALSE);
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
	SC285SL_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8SensorImageMode = pstSnsState->u8ImgMode;
	pstSnsState->bSyncInit = CVI_FALSE;

	if (pstSensorImageMode->f32Fps <= 30) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (SC285SL_RES_IS_1080P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = SC285SL_MODE_1920X1080P30;
			else {
				CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
							  pstSensorImageMode->u16Width, pstSensorImageMode->u16Height,
							  pstSensorImageMode->f32Fps, pstSnsState->enWDRMode);
				return CVI_FAILURE;
			}
		} else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
			if (SC285SL_RES_IS_1080P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = SC285SL_MODE_1920X1080P30_WDR;
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

static CVI_VOID sensor_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC285SL_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);
	/* Apply the setting on the fly  */
	if (pstSnsState->bInit == CVI_TRUE && g_aeSc285sl_MirrorFip[ViPipe] != eSnsMirrorFlip) {
		sc285sl_mirror_flip(ViPipe, eSnsMirrorFlip);
		g_aeSc285sl_MirrorFip[ViPipe] = eSnsMirrorFlip;
	}
}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC285SL_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsState->bInit = CVI_FALSE;
	pstSnsState->bSyncInit = CVI_FALSE;
	pstSnsState->u8ImgMode = SC285SL_MODE_1920X1080P30;
	pstSnsState->enWDRMode = WDR_MODE_NONE;
	pstSnsState->u32FLStd = g_astSc285sl_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[0] = g_astSc285sl_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[1] = g_astSc285sl_mode[pstSnsState->u8ImgMode].u32VtsDef;

	memset(&pstSnsState->astSyncInfo[0], 0, sizeof(ISP_SNS_SYNC_INFO_S));
	memset(&pstSnsState->astSyncInfo[1], 0, sizeof(ISP_SNS_SYNC_INFO_S));
}

static CVI_S32 sensor_rx_attr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *pstRxAttr)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC285SL_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pstRxAttr);

	memcpy(pstRxAttr, &sc285sl_rx_attr, sizeof(*pstRxAttr));

	pstRxAttr->img_size.width =
		g_astSc285sl_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Width;
	pstRxAttr->img_size.height =
		g_astSc285sl_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Height;
	if (pstSnsState->enWDRMode == WDR_MODE_NONE)
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;
	else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE)
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_VC;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_patch_rx_attr(RX_INIT_ATTR_S *pstRxInitAttr)
{
	SNS_COMBO_DEV_ATTR_S *pstRxAttr = &sc285sl_rx_attr;
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

	pstSensorExpFunc->pfn_cmos_sensor_init = sc285sl_init;
	pstSensorExpFunc->pfn_cmos_sensor_exit = sc285sl_exit;
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
	if (SC285SL_I2C_ADDR_IS_VALID(s32I2cAddr))
		sc285sl_i2c_addr = s32I2cAddr;
}

static CVI_S32 sc285sl_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
	g_aunSc285sl_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	SC285SL_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

	if (pastSnsStateCtx == CVI_NULL) {
		pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
		if (pastSnsStateCtx == CVI_NULL) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
			return -ENOMEM;
		}
	}

	memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

	SC285SL_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

	return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	SC285SL_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
	SENSOR_FREE(pastSnsStateCtx);
	SC285SL_SENSOR_RESET_CTX(ViPipe);
	g_aeSc285sl_MirrorFip[ViPipe] = ISP_SNS_NORMAL;
}

static CVI_S32 sensor_register_callback(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib)
{
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

	stSnsAttrInfo.eSensorId = SC285SL_ID;

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

static CVI_S32 sensor_unregister_callback(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CMOS_CHECK_POINTER(pstAeLib);
	CMOS_CHECK_POINTER(pstAwbLib);

	s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, SC285SL_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, SC285SL_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, SC285SL_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to awb lib failed!\n");
		return s32Ret;
	}

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
	g_au16Sc285sl_GainMode[ViPipe] = pstInitAttr->enGainMode;
	g_au16Sc285sl_UseHwSync[ViPipe] = pstInitAttr->u16UseHwSync;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_probe(VI_PIPE ViPipe)
{
	return sc285sl_probe(ViPipe);
}

ISP_SNS_OBJ_S stSnsSC285SL_Obj = {
	.pfnRegisterCallback = sensor_register_callback,
	.pfnUnRegisterCallback = sensor_unregister_callback,
	.pfnStandby = sc285sl_standby,
	.pfnRestart = sc285sl_restart,
	.pfnWriteReg = sc285sl_write_register,
	.pfnReadReg = sc285sl_read_register,
	.pfnSetBusInfo = sc285sl_set_bus_info,
	.pfnSetInit = sensor_set_init,
	.pfnMirrorFlip = sensor_mirror_flip,
	.pfnPatchRxAttr = sensor_patch_rx_attr,
	.pfnPatchI2cAddr = sensor_patch_i2c_addr,
	.pfnGetRxAttr = sensor_rx_attr,
	.pfnExpSensorCb = cmos_init_sensor_exp_function,
	.pfnExpAeCb = cmos_init_ae_exp_function,
	.pfnSnsProbe = sensor_probe,
};

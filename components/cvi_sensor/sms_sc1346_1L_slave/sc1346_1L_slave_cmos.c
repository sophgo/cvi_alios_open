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

#include "sc1346_1L_slave_cmos_ex.h"
#include "sc1346_1L_slave_cmos_param.h"
#include "cvi_comm_cif.h"

#define DIV_0_TO_1(a)   ((0 == (a)) ? 1 : (a))
#define DIV_0_TO_1_FLOAT(a) ((((a) < 1E-10) && ((a) > -1E-10)) ? 1 : (a))
#define SC1346_1L_SLAVE_ID 35
#define SENSOR_SC1346_1L_SLAVE_WIDTH 1280
#define SENSOR_SC1346_1L_SLAVE_HEIGHT 720
#define SC1346_1L_SLAVE_I2C_ADDR_1 0x30
#define SC1346_1L_SLAVE_I2C_ADDR_2 0x32
#define SC1346_1L_SLAVE_I2C_ADDR_IS_VALID(addr)	((addr) == SC1346_1L_SLAVE_I2C_ADDR_1 || (addr) == SC1346_1L_SLAVE_I2C_ADDR_2)
/****************************************************************************
 * global variables                                                            *
 ****************************************************************************/

ISP_SNS_STATE_S *g_pastSC1346_1L_Slave[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define SC1346_1L_SLAVE_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastSC1346_1L_Slave[dev])
#define SC1346_1L_SLAVE_SENSOR_SET_CTX(dev, pstCtx)   (g_pastSC1346_1L_Slave[dev] = pstCtx)
#define SC1346_1L_SLAVE_SENSOR_RESET_CTX(dev)         (g_pastSC1346_1L_Slave[dev] = CVI_NULL)

ISP_SNS_COMMBUS_U g_aunSC1346_1L_Slave_BusInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cDev = 0},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

CVI_U16 g_au16SC1346_1L_Slave_GainMode[VI_MAX_PIPE_NUM] = {0};
CVI_U16 g_au16SC1346_1L_Slave_L2SMode[VI_MAX_PIPE_NUM] = {0};

SC1346_1L_SLAVE_STATE_S g_astSC1346_1L_Slave_State[VI_MAX_PIPE_NUM] = {{0} };
ISP_SNS_MIRRORFLIP_TYPE_E g_aeSc1346_Slave_MirrorFip[VI_MAX_PIPE_NUM] = {ISP_SNS_NORMAL};
/****************************************************************************
 * local variables and functions                                                           *
 ****************************************************************************/

static CVI_U32 g_au32InitExposure[VI_MAX_PIPE_NUM]  = {0};
static CVI_U32 g_au32LinesPer500ms[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16InitWBGain[VI_MAX_PIPE_NUM][3] = {{0} };
static CVI_U16 g_au16SampleRgain[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16SampleBgain[VI_MAX_PIPE_NUM] = {0};
static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg);
/*****SC1346_1L_SLAVE Lines Range*****/
#define SC1346_1L_SLAVE_FULL_LINES_MAX  (0x3FFF)

/*****SC1346_1L_SLAVE Register Address*****/
#define SC1346_1L_SLAVE_SHS1_0_ADDR		0x3E00
#define SC1346_1L_SLAVE_SHS1_1_ADDR		0x3E01
#define SC1346_1L_SLAVE_SHS1_2_ADDR		0x3E02
#define SC1346_1L_SLAVE_AGAIN_ADDR		0x3E09
#define SC1346_1L_SLAVE_DGAIN0_ADDR		0x3E06
#define SC1346_1L_SLAVE_VMAX_ADDR			0x320E
#define SC1346_1L_SLAVE_FLIP_MIRROR_ADDR  0x3221
#define SC1346_1L_SLAVE_TABLE_END			0xFFFF

#define SC1346_1L_SLAVE_RES_IS_720P(w, h)      ((w) <= 1280 && (h) <= 720)

static CVI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	const SC1346_1L_SLAVE_MODE_S *pstMode;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	CVI_U32 fps = 30;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	SC1346_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstMode = &g_astSC1346_1L_Slave_mode[pstSnsState->u8ImgMode];
	if (pstSnsState->u8ImgMode == SC1346_1L_SLAVE_MODE_720P60)
		fps = 60;
#if 0
	memset(&pstAeSnsDft->stAERouteAttr, 0, sizeof(ISP_AE_ROUTE_S));
#endif
	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FlickerFreq = 50 * 256;
	pstAeSnsDft->u32FullLinesMax = SC1346_1L_SLAVE_FULL_LINES_MAX;
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * fps);

	pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
	pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
	pstAeSnsDft->stIntTimeAccu.f32Offset = 0;

	pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_DB;
	pstAeSnsDft->stAgainAccu.f32Accuracy = 1;

	pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stDgainAccu.f32Accuracy = 1;

	pstAeSnsDft->u32ISPDgainShift = 8;
	pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
	pstAeSnsDft->u32MaxISPDgainTarget = 2 << pstAeSnsDft->u32ISPDgainShift;

	if (g_au32LinesPer500ms[ViPipe] == 0)
		pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * fps / 2;
	else
		pstAeSnsDft->u32LinesPer500ms = g_au32LinesPer500ms[ViPipe];
	pstAeSnsDft->u32SnsStableFrame = 0;
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
		pstAeSnsDft->f32Fps = pstMode->f32MaxFps;
		pstAeSnsDft->f32MinFps = pstMode->f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xd;
		pstAeSnsDft->au8HistThresh[1] = 0x28;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxAgain = pstMode->stAgain[0].u16Max;
		pstAeSnsDft->u32MinAgain = pstMode->stAgain[0].u16Min;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = pstMode->stDgain[0].u16Max;
		pstAeSnsDft->u32MinDgain = pstMode->stDgain[0].u16Min;
		pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
		pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

		pstAeSnsDft->u8AeCompensation = 40;
		pstAeSnsDft->u32InitAESpeed = 64;
		pstAeSnsDft->u32InitAETolerance = 5;
		pstAeSnsDft->u32AEResponseFrame = 4;
		pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe] ? g_au32InitExposure[ViPipe] : 76151;

		pstAeSnsDft->u32MaxIntTime = pstMode->stExp[0].u16Max;
		pstAeSnsDft->u32MinIntTime = pstMode->stExp[0].u16Min;
		pstAeSnsDft->u32MaxIntTimeTarget = 65535;
		pstAeSnsDft->u32MinIntTimeTarget = 1;
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
	SC1346_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32Vts = g_astSC1346_1L_Slave_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	f32MaxFps = g_astSC1346_1L_Slave_mode[pstSnsState->u8ImgMode].f32MaxFps;
	f32MinFps = g_astSC1346_1L_Slave_mode[pstSnsState->u8ImgMode].f32MinFps;

	switch (pstSnsState->u8ImgMode) {
	case SC1346_1L_SLAVE_MODE_720P30:
	case SC1346_1L_SLAVE_MODE_720P60:
		if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
			u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
			return CVI_FAILURE;
		}
		u32VMAX = (u32VMAX > SC1346_1L_SLAVE_FULL_LINES_MAX) ? SC1346_1L_SLAVE_FULL_LINES_MAX : u32VMAX;
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support sensor mode: %d\n", pstSnsState->u8ImgMode);
		return CVI_FAILURE;
	}

	pstSnsState->u32FLStd = u32VMAX;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_0_ADDR].u32Data = ((u32VMAX & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_1_ADDR].u32Data = (u32VMAX & 0xFF);
	}

	pstAeSnsDft->f32Fps = f32Fps;
	pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * f32Fps / 2;
	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 6;
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
	CVI_U32 u32TmpIntTime, u32MinTime, u32MaxTime;

	SC1346_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(u32IntTime);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	if (WDR_MODE_2To1_LINE != pstSnsState->enWDRMode) {
		/* linear exposure reg range:
		 * min : 2
		 * max : (vts - 6)
		 * step : 1
		 */

		u32MinTime = 2;
		u32MaxTime = pstSnsState->au32FL[0] - 6;
		u32TmpIntTime = (u32IntTime[0] > u32MaxTime) ? u32MaxTime : u32IntTime[0];
		u32TmpIntTime = (u32TmpIntTime < u32MinTime) ? u32MinTime : u32TmpIntTime;

		pstSnsRegsInfo->astI2cData[LINEAR_SHS1_0_ADDR].u32Data = ((u32TmpIntTime & 0xF000) >> 12);
		pstSnsRegsInfo->astI2cData[LINEAR_SHS1_1_ADDR].u32Data = ((u32TmpIntTime & 0x0FF0) >> 4);
		pstSnsRegsInfo->astI2cData[LINEAR_SHS1_2_ADDR].u32Data = (u32TmpIntTime & 0xF) << 4;
	}

	return CVI_SUCCESS;

}

struct gain_tbl_info_s {
	CVI_U16	gainMax;
	CVI_U16	idxBase;
	CVI_U8	regGain;
	CVI_U8	regGainFineBase;
	CVI_U8	regGainFineStep;
};

static struct gain_tbl_info_s AgainInfo[] = {
	{
		.gainMax = 2016,
		.idxBase = 0,
		.regGain = 0x00,
		.regGainFineBase = 0x80,
		.regGainFineStep = 4,
	},
	{
		.gainMax = 4032,
		.idxBase = 32,
		.regGain = 0x08,
		.regGainFineBase = 0x80,
		.regGainFineStep = 4,
	},
	{
		.gainMax = 8064,
		.idxBase = 64,
		.regGain = 0x09,
		.regGainFineBase = 0x80,
		.regGainFineStep = 4,
	},
	{
		.gainMax = 16128,
		.idxBase = 96,
		.regGain = 0x0b,
		.regGainFineBase = 0x80,
		.regGainFineStep = 4,
	},
	{
		.gainMax = 32256,
		.idxBase = 128,
		.regGain = 0x0f,
		.regGainFineBase = 0x80,
		.regGainFineStep = 4,
	},
	{
		.gainMax = 32768,
		.idxBase = 160,
		.regGain = 0x1F,
		.regGainFineBase = 0x80,
		.regGainFineStep = 4,
	},
};

static CVI_U32 Again_table[] = {
	1024,	1056,	1088,	1120,	1152,	1184,	1216,	1248,
	1280,	1312,	1344,	1376,	1408,	1440,	1472,	1504,
	1536,	1568,	1600,	1632,	1664,	1696,	1728,	1760,
	1792,	1824,	1856,	1888,	1920,	1952,	1984,	2016,

	2048,	2112,	2176,	2240,	2304,	2368,	2432,	2496,
	2560,	2624,	2688,	2752,	2816,	2880,	2944,	3008,
	3072,	3136,	3200,	3264,	3328,	3392,	3456,	3520,
	3584,	3648,	3712,	3776,	3840,	3904,	3968,	4032,

	4096,	4224,	4352,	4480,	4608,	4736,	4864,	4992,
	5120,	5248,	5376,	5504,	5632,	5760,	5888,	6016,
	6144,	6272,	6400,	6528,	6656,	6784,	6912,	7040,
	7168,	7296,	7424,	7552,	7680,	7808,	7936,	8064,

	8192,	8448,	8704,	8960,	9216,	9472,	9728,	9984,
	10240,	10496,	10752,	11008,	11264,	11520,	11776,	12032,
	12288,	12544,	12800,	13056,	13312,	13568,	13824,	14080,
	14336,	14592,	14848,	15104,	15360,	15616,	15872,	16128,

	16384,	16896,	17408,	17920,	18432,	18944,	19456,	19968,
	20480,	20992,	21504,	22016,	22528,	23040,	23552,	24064,
	24576,	25088,	25600,	26112,	26624,	27136,	27648,	28160,
	28672,	29184,	29696,	30208,	30720,	31232,	31744,	32256,

	32768,
};

static struct gain_tbl_info_s DgainInfo[] = {
	{
		.gainMax = 2016,
		.idxBase = 0,
		.regGain = 0x00,
		.regGainFineBase = 0x80,
		.regGainFineStep = 4,
	},
	{
		.gainMax = 4032,
		.idxBase = 32,
		.regGain = 0x01,
		.regGainFineBase = 0x80,
		.regGainFineStep = 4,
	},
};

static CVI_U32 Dgain_table[] = {
	/* 1x-2x */
	1024, 1056, 1088, 1120, 1152, 1184, 1216, 1248,
	1280, 1312, 1344, 1376, 1408, 1440, 1472, 1504,
	1536, 1568, 1600, 1632, 1664, 1696, 1728, 1760,
	1792, 1824, 1856, 1888, 1920, 1952, 1984, 2016,
	/* 2x-4x */
	2048, 2112, 2176, 2240, 2304, 2368, 2432, 2496,
	2560, 2624, 2688, 2752, 2816, 2880, 2944, 3008,
	3072, 3136, 3200, 3264, 3328, 3392, 3456, 3520,
	3584, 3648, 3712, 3776, 3840, 3904, 3968, 4032,
};

static CVI_S32 cmos_again_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32AgainLin, CVI_U32 *pu32AgainDb)
{
	CVI_U32 tableSize = sizeof(Again_table) / sizeof(CVI_U32);

	(void)ViPipe;

	CMOS_CHECK_POINTER(pu32AgainLin);
	CMOS_CHECK_POINTER(pu32AgainDb);

	if (*pu32AgainLin >= Again_table[tableSize - 1]) {
		*pu32AgainLin = Again_table[tableSize - 1];
		*pu32AgainDb = tableSize - 1;
		return CVI_SUCCESS;
	}

	for (CVI_U32 i = 1; i < tableSize; i++) {
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
	CVI_U32 tableSize = sizeof(Dgain_table) / sizeof(CVI_U32);

	(void)ViPipe;

	CMOS_CHECK_POINTER(pu32DgainLin);
	CMOS_CHECK_POINTER(pu32DgainDb);

	if (*pu32DgainLin >= Dgain_table[tableSize - 1]) {
		*pu32DgainLin = Dgain_table[tableSize - 1];
		*pu32DgainDb = tableSize - 1;
		return CVI_SUCCESS;
	}

	for (CVI_U32 i = 1; i < tableSize; i++) {
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
	struct gain_tbl_info_s *info;
	CVI_S32 i = 0, tbl_num = 0;

	SC1346_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pu32Again);
	CMOS_CHECK_POINTER(pu32Dgain);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	u32Again = pu32Again[0];
	u32Dgain = pu32Dgain[0];

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		/* linear mode */

		/* Again. */
		tbl_num = sizeof(AgainInfo)/sizeof(struct gain_tbl_info_s);
		for (i = tbl_num - 1; i >= 0; i--) {
			info = &AgainInfo[i];

			if (u32Again >= info->idxBase)
				break;
		}

		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_0_ADDR].u32Data = (info->regGain & 0xFF);
		u32Again = info->regGainFineBase + (u32Again - info->idxBase) * info->regGainFineStep;
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_1_ADDR].u32Data = (u32Again & 0xFF);

		/* find Dgain register setting. */
		tbl_num = sizeof(DgainInfo)/sizeof(struct gain_tbl_info_s);
		for (i = tbl_num - 1; i >= 0; i--) {
			info = &DgainInfo[i];

			if (u32Dgain >= info->idxBase)
				break;
		}

		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_0_ADDR].u32Data = (info->regGain & 0xFF);
		u32Dgain = info->regGainFineBase + (u32Dgain - info->idxBase) * info->regGainFineStep;
		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_1_ADDR].u32Data = (u32Dgain & 0xFF);
	}
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
	// pstExpFuncs->pfn_cmos_get_inttime_max   = cmos_get_inttime_max;
	// pstExpFuncs->pfn_cmos_ae_fswdr_attr_set = cmos_ae_fswdr_attr_set;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_awb_default(VI_PIPE ViPipe, AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
	(void) ViPipe;

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
	const SC1346_1L_SLAVE_MODE_S *pstMode = CVI_NULL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC1346_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstMode = &g_astSC1346_1L_Slave_mode[pstSnsState->u8ImgMode];

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

	SC1346_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstSnsState->bSyncInit = CVI_FALSE;

	switch (u8Mode) {
	case WDR_MODE_NONE:
		pstSnsState->enWDRMode = WDR_MODE_NONE;
		pstSnsState->u32FLStd = g_astSC1346_1L_Slave_mode[pstSnsState->u8ImgMode].u32VtsDef;
		syslog(LOG_INFO, "linear mode\n");
		break;

	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "NOT support this mode!\n");
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
	SC1346_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstSnsRegsInfo = &pstSnsSyncInfo->snsCfg;
	pstCfg0 = &pstSnsState->astSyncInfo[0];
	pstCfg1 = &pstSnsState->astSyncInfo[1];
	pstI2c_data = pstCfg0->snsCfg.astI2cData;

	if ((pstSnsState->bSyncInit == CVI_FALSE) || (pstSnsRegsInfo->bConfig == CVI_FALSE)) {
		pstCfg0->snsCfg.enSnsType = SNS_I2C_TYPE;
		pstCfg0->snsCfg.unComBus.s8I2cDev = g_aunSC1346_1L_Slave_BusInfo[ViPipe].s8I2cDev;
		pstCfg0->snsCfg.u8Cfg2ValidDelayMax = 0;
		pstCfg0->snsCfg.use_snsr_sram = CVI_TRUE;
		pstCfg0->snsCfg.u32RegNum = LINEAR_REGS_NUM;

		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			pstI2c_data[i].bUpdate = CVI_TRUE;
			pstI2c_data[i].u8DevAddr = sc1346_1L_slave_i2c_addr;
			pstI2c_data[i].u32AddrByteNum = sc1346_1L_slave_addr_byte;
			pstI2c_data[i].u32DataByteNum = sc1346_1L_slave_data_byte;
		}

		//DOL 2t1 Mode Regs
		switch (pstSnsState->enWDRMode) {
		case WDR_MODE_2To1_LINE:
			break;
		default:
			//Linear Mode Regs
			pstI2c_data[LINEAR_SHS1_0_ADDR].u32RegAddr = SC1346_1L_SLAVE_SHS1_0_ADDR;
			pstI2c_data[LINEAR_SHS1_1_ADDR].u32RegAddr = SC1346_1L_SLAVE_SHS1_1_ADDR;
			pstI2c_data[LINEAR_SHS1_2_ADDR].u32RegAddr = SC1346_1L_SLAVE_SHS1_2_ADDR;
			pstI2c_data[LINEAR_AGAIN_0_ADDR].u32RegAddr = SC1346_1L_SLAVE_AGAIN_ADDR;
			pstI2c_data[LINEAR_AGAIN_1_ADDR].u32RegAddr = SC1346_1L_SLAVE_DGAIN0_ADDR + 1;
			pstI2c_data[LINEAR_DGAIN_0_ADDR].u32RegAddr = SC1346_1L_SLAVE_DGAIN0_ADDR;
			pstI2c_data[LINEAR_DGAIN_1_ADDR].u32RegAddr = SC1346_1L_SLAVE_DGAIN0_ADDR + 1;
			pstI2c_data[LINEAR_VMAX_0_ADDR].u32RegAddr = SC1346_1L_SLAVE_VMAX_ADDR;
			pstI2c_data[LINEAR_VMAX_1_ADDR].u32RegAddr = SC1346_1L_SLAVE_VMAX_ADDR + 1;
			pstI2c_data[LINEAR_FLIP_MIRROR_ADDR].u32RegAddr = SC1346_1L_SLAVE_FLIP_MIRROR_ADDR;

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
		/* check update isp crop or not */
		pstCfg0->ispCfg.need_update = (sensor_cmp_wdr_size(&pstCfg0->ispCfg, &pstCfg1->ispCfg) ?
				CVI_TRUE : CVI_FALSE);

		/* check update cif wdr manual or not */
		pstCfg0->cifCfg.need_update = (sensor_cmp_cif_wdr(&pstCfg0->cifCfg, &pstCfg1->cifCfg) ?
				CVI_TRUE : CVI_FALSE);
	}

	pstSnsRegsInfo->bConfig = CVI_FALSE;
	memcpy(pstSnsSyncInfo, &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));
	memcpy(&pstSnsState->astSyncInfo[1], &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));
	pstSnsState->au32FL[1] = pstSnsState->au32FL[0];
	pstCfg0->snsCfg.astI2cData[LINEAR_FLIP_MIRROR_ADDR].bDropFrm = CVI_FALSE;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_image_mode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
	CVI_U8 u8SensorImageMode = 0;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstSensorImageMode);
	SC1346_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8SensorImageMode = pstSnsState->u8ImgMode;
	pstSnsState->bSyncInit = CVI_FALSE;

	if (pstSensorImageMode->f32Fps <= 30) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (SC1346_1L_SLAVE_RES_IS_720P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height)) {
				u8SensorImageMode = SC1346_1L_SLAVE_MODE_720P30;
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
	} else if (pstSensorImageMode->f32Fps <= 60) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (SC1346_1L_SLAVE_RES_IS_720P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height)){
				u8SensorImageMode = SC1346_1L_SLAVE_MODE_720P60;
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
	}

	if ((pstSnsState->bInit == CVI_TRUE) && (u8SensorImageMode == pstSnsState->u8ImgMode)) {
		/* Don't need to switch SensorImageMode */
		return CVI_FAILURE;
	}

	pstSnsState->u8ImgMode = u8SensorImageMode;

	return CVI_SUCCESS;
}

static CVI_VOID sc1346_1L_slave_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U8 value = 0;

	SC1346_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	/* Apply the setting on the fly  */
	if (pstSnsState->bInit == CVI_TRUE && g_aeSc1346_Slave_MirrorFip[ViPipe] != eSnsMirrorFlip) {
		switch (eSnsMirrorFlip) {
		case ISP_SNS_NORMAL:
			value |= 0;
			break;
		case ISP_SNS_MIRROR:
			value |= 0x6;
			break;
		case ISP_SNS_FLIP:
			value |= 0x60;
			break;
		case ISP_SNS_MIRROR_FLIP:
			value |= 0x66;
			break;
		default:
			return;
		}

		pstSnsRegsInfo->astI2cData[LINEAR_FLIP_MIRROR_ADDR].u32Data = value;
		pstSnsRegsInfo->astI2cData[LINEAR_FLIP_MIRROR_ADDR].bDropFrm = 1;
		pstSnsRegsInfo->astI2cData[LINEAR_FLIP_MIRROR_ADDR].u8DropFrmNum = 1;
		g_aeSc1346_Slave_MirrorFip[ViPipe] = eSnsMirrorFlip;
	}
}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	const SC1346_1L_SLAVE_MODE_S *pstMode = CVI_NULL;

	SC1346_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsState->bInit = CVI_FALSE;
	pstSnsState->bSyncInit = CVI_FALSE;
	pstSnsState->u8ImgMode = SC1346_1L_SLAVE_MODE_720P30;
	pstSnsState->enWDRMode = WDR_MODE_NONE;
	pstMode = &g_astSC1346_1L_Slave_mode[pstSnsState->u8ImgMode];
	pstSnsState->u32FLStd  = pstMode->u32VtsDef;
	pstSnsState->au32FL[0] = pstMode->u32VtsDef;
	pstSnsState->au32FL[1] = pstMode->u32VtsDef;

	memset(&pstSnsState->astSyncInfo[0], 0, sizeof(ISP_SNS_SYNC_INFO_S));
	memset(&pstSnsState->astSyncInfo[1], 0, sizeof(ISP_SNS_SYNC_INFO_S));
}

static CVI_S32 sensor_rx_attr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *pstRxAttr)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC1346_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pstRxAttr);

	memcpy(pstRxAttr, &sc1346_1L_slave_rx_attr, sizeof(*pstRxAttr));

	pstRxAttr->img_size.width = g_astSC1346_1L_Slave_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Width;
	pstRxAttr->img_size.height = g_astSC1346_1L_Slave_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Height;
	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;
	}

	return CVI_SUCCESS;

}

static CVI_S32 sensor_patch_rx_attr(RX_INIT_ATTR_S *pstRxInitAttr)
{
	SNS_COMBO_DEV_ATTR_S *pstRxAttr = &sc1346_1L_slave_rx_attr;
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

	pstSensorExpFunc->pfn_cmos_sensor_init = sc1346_1L_slave_init;
	pstSensorExpFunc->pfn_cmos_sensor_exit = sc1346_1L_slave_exit;
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
	if (SC1346_1L_SLAVE_I2C_ADDR_IS_VALID(s32I2cAddr))
		sc1346_1L_slave_i2c_addr = s32I2cAddr;
}

static CVI_S32 sc1346_1L_slave_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
	g_aunSC1346_1L_Slave_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	SC1346_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

	if (pastSnsStateCtx == CVI_NULL) {
		pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
		if (pastSnsStateCtx == CVI_NULL) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
			return -ENOMEM;
		}
	}

	memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

	SC1346_1L_SLAVE_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

	return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	SC1346_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
	SENSOR_FREE(pastSnsStateCtx);
	SC1346_1L_SLAVE_SENSOR_RESET_CTX(ViPipe);
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

	stSnsAttrInfo.eSensorId = SC1346_1L_SLAVE_ID;

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
	CVI_S32 s32Ret;

	CMOS_CHECK_POINTER(pstAeLib);
	CMOS_CHECK_POINTER(pstAwbLib);

	s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, SC1346_1L_SLAVE_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, SC1346_1L_SLAVE_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, SC1346_1L_SLAVE_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to awb lib failed!\n");
		return s32Ret;
	}

	sensor_ctx_exit(ViPipe);

	return CVI_SUCCESS;
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
	g_au16SC1346_1L_Slave_GainMode[ViPipe] = pstInitAttr->enGainMode;
	g_au16SC1346_1L_Slave_L2SMode[ViPipe] = pstInitAttr->enL2SMode;

	return CVI_SUCCESS;
}

ISP_SNS_OBJ_S stSnsSC1346_1L_Slave_Obj = {
	.pfnRegisterCallback    = sensor_register_callback,
	.pfnUnRegisterCallback  = sensor_unregister_callback,
	.pfnStandby             = sc1346_1L_slave_standby,
	.pfnRestart             = sc1346_1L_slave_restart,
	.pfnMirrorFlip          = sc1346_1L_slave_mirror_flip,
	.pfnWriteReg            = sc1346_1L_slave_write_register,
	.pfnReadReg             = sc1346_1L_slave_read_register,
	.pfnSetBusInfo          = sc1346_1L_slave_set_bus_info,
	.pfnSetInit             = sensor_set_init,
	.pfnPatchRxAttr		= sensor_patch_rx_attr,
	.pfnPatchI2cAddr	= sensor_patch_i2c_addr,
	.pfnGetRxAttr		= sensor_rx_attr,
	.pfnExpSensorCb		= cmos_init_sensor_exp_function,
	.pfnExpAeCb		= cmos_init_ae_exp_function,
	.pfnSnsProbe		= sc1346_1L_slave_probe,
};


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

#include "sc035hgs_1L_slave_cmos_ex.h"
#include "sc035hgs_1L_slave_cmos_param.h"
#include "cvi_comm_cif.h"

#define DIV_0_TO_1(a)   ((0 == (a)) ? 1 : (a))
#define DIV_0_TO_1_FLOAT(a) ((((a) < 1E-10) && ((a) > -1E-10)) ? 1 : (a))
#define SC035HGS_1L_SLAVE_ID 35
/****************************************************************************
 * global variables                                                            *
 ****************************************************************************/

ISP_SNS_STATE_S *g_pastSC035HGS_1L_SLAVE[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define SC035HGS_1L_SLAVE_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastSC035HGS_1L_SLAVE[dev])
#define SC035HGS_1L_SLAVE_SENSOR_SET_CTX(dev, pstCtx)   (g_pastSC035HGS_1L_SLAVE[dev] = pstCtx)
#define SC035HGS_1L_SLAVE_SENSOR_RESET_CTX(dev)         (g_pastSC035HGS_1L_SLAVE[dev] = CVI_NULL)
#define SC035HGS_1L_SLAVE_I2C_ADDR_1 0x30
#define SC035HGS_1L_SLAVE_I2C_ADDR_2 0x31
#define SC035HGS_1L_SLAVE_I2C_ADDR_3 0x32
#define SC035HGS_1L_SLAVE_I2C_ADDR_4 0x33
#define SC035HGS_1L_SLAVE_I2C_ADDR_IS_VALID(addr)	((addr) == SC035HGS_1L_SLAVE_I2C_ADDR_1 || \
											 (addr) == SC035HGS_1L_SLAVE_I2C_ADDR_2 || \
											 (addr) == SC035HGS_1L_SLAVE_I2C_ADDR_3 || \
											 (addr) == SC035HGS_1L_SLAVE_I2C_ADDR_4)

ISP_SNS_COMMBUS_U g_aunSC035HGS_1L_SLAVE_BusInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cDev = 2},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

CVI_U16 g_au16SC035HGS_1L_SLAVE_GainMode[VI_MAX_PIPE_NUM] = {0};
CVI_U16 g_au16SC035HGS_1L_SLAVE_L2SMode[VI_MAX_PIPE_NUM] = {0};

ISP_SNS_MIRRORFLIP_TYPE_E g_aeSc035hgs_slave_MirrorFip[VI_MAX_PIPE_NUM] = {0};

/****************************************************************************
 * local variables and functions                                            *
 ****************************************************************************/
static CVI_U32 g_au32InitExposure[VI_MAX_PIPE_NUM]  = {0};
static CVI_U32 g_au32LinesPer500ms[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16InitWBGain[VI_MAX_PIPE_NUM][3] = {{0} };
static CVI_U16 g_au16SampleRgain[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16SampleBgain[VI_MAX_PIPE_NUM] = {0};
static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg);
/*****SC035HGS_1L Lines Range*****/
#define SC035HGS_1L_SLAVE_FULL_LINES_MAX  (0xFFFF)

/*****SC035HGS_1L Register Address*****/
#define SC035HGS_1L_SLAVE_EXP_H_ADDR                     (0x3e01)
#define SC035HGS_1L_SLAVE_EXP_L_ADDR                     (0x3e02)

#define SC035HGS_1L_SLAVE_AGAIN_H_ADDR                   (0x3e08)
#define SC035HGS_1L_SLAVE_AGAIN_L_ADDR                   (0x3e09)

#define SC035HGS_1L_SLAVE_DGAIN_H_ADDR                   (0x3e06)
#define SC035HGS_1L_SLAVE_DGAIN_L_ADDR                   (0x3e07)

#define SC035HGS_1L_SLAVE_VMAX_H_ADDR                    (0x320e)
#define SC035HGS_1L_SLAVE_VMAX_L_ADDR                    (0x320f)

#define SC035HGS_1L_SLAVE_GAIN_MAGIC_0_ADDR              (0x3314)
#define SC035HGS_1L_SLAVE_GAIN_MAGIC_1_ADDR              (0x3317)
#define SC035HGS_1L_SLAVE_GAIN_MAGIC_2_ADDR              (0x3631)
#define SC035HGS_1L_SLAVE_GAIN_MAGIC_3_ADDR              (0x3630)

#define SC035HGS_1L_SLAVE_RES_IS_480P(w, h)      ((w) <= 640 && (h) <= 480)

static CVI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	const SC035HGS_1L_SLAVE_MODE_S *pstMode;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	SC035HGS_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstMode = &g_astSC035HGS_1L_SLAVE_mode[pstSnsState->u8ImgMode];
#if 0
	memset(&pstAeSnsDft->stAERouteAttr, 0, sizeof(ISP_AE_ROUTE_S));
#endif
	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FlickerFreq = 50 * 256;
	pstAeSnsDft->u32FullLinesMax = SC035HGS_1L_SLAVE_FULL_LINES_MAX;
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * 120);

	pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
	pstAeSnsDft->stIntTimeAccu.f32Accuracy = 0.0625; /* unit = 1/16 line */
	pstAeSnsDft->stIntTimeAccu.f32Offset = 0;

	pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stAgainAccu.f32Accuracy = 1;

	pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stDgainAccu.f32Accuracy = 1;

	pstAeSnsDft->u32ISPDgainShift = 8;
	pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
	pstAeSnsDft->u32MaxISPDgainTarget = 2 << pstAeSnsDft->u32ISPDgainShift;

	if (g_au32LinesPer500ms[ViPipe] == 0)
		pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * 120 / 2;
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
	case WDR_MODE_NONE:   /*linear mode*/
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
		pstAeSnsDft->u32AEResponseFrame = 4;
		pstAeSnsDft->u32SnsResponseFrame = 4;
		pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe] ? g_au32InitExposure[ViPipe] : 76151;

		pstAeSnsDft->u32MaxIntTime = pstMode->stExp[0].u16Max;
		pstAeSnsDft->u32MinIntTime = pstMode->stExp[0].u16Min;
		pstAeSnsDft->u32MaxIntTimeTarget = pstAeSnsDft->u32MaxIntTime;
		pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support WDR: %d\n", pstSnsState->enWDRMode);
		return CVI_FAILURE;
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
	SC035HGS_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32Vts = g_astSC035HGS_1L_SLAVE_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	f32MaxFps = g_astSC035HGS_1L_SLAVE_mode[pstSnsState->u8ImgMode].f32MaxFps;
	f32MinFps = g_astSC035HGS_1L_SLAVE_mode[pstSnsState->u8ImgMode].f32MinFps;

	switch (pstSnsState->u8ImgMode) {
	case SC035HGS_1L_SLAVE_MODE_640X480P120:
		if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
			u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
			return CVI_FAILURE;
		}
		u32VMAX = (u32VMAX > SC035HGS_1L_SLAVE_FULL_LINES_MAX) ? SC035HGS_1L_SLAVE_FULL_LINES_MAX : u32VMAX;
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support sensor mode: %d\n", pstSnsState->u8ImgMode);
		return CVI_FAILURE;
	}

	pstSnsState->u32FLStd = u32VMAX;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_H_ADDR].u32Data = ((u32VMAX & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_L_ADDR].u32Data = (u32VMAX & 0xFF);
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support WDR: %d\n", pstSnsState->enWDRMode);
		return CVI_FAILURE;
	}

	pstAeSnsDft->f32Fps = f32Fps;
	pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * f32Fps / 2;
	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32MaxIntTime = (pstSnsState->u32FLStd - 6) << 4;
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

	SC035HGS_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(u32IntTime);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		/* linear exposure reg range:
		 * min : 1
		 * max : 16 * (vts - 6)
		 * step : 1
		 */
		u32MinTime = 1;
		u32MaxTime = (pstSnsState->au32FL[0] - 6) << 4;

		u32TmpIntTime = (u32IntTime[0] > u32MaxTime) ? u32MaxTime : u32IntTime[0];
		u32TmpIntTime = (u32TmpIntTime < u32MinTime) ? u32MinTime : u32TmpIntTime;

		pstSnsRegsInfo->astI2cData[LINEAR_EXP_H_ADDR].u32Data = ((u32TmpIntTime & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[LINEAR_EXP_L_ADDR].u32Data = (u32TmpIntTime & 0xFF);
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support WDR: %d\n", pstSnsState->enWDRMode);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;

}

struct gain_tbl_info_s {
	CVI_U32	gainMax;
	CVI_U32	idxBase;
	CVI_U8	regGain;
	CVI_U8	regGainFineBase;
	CVI_U8	regGainFineStep;
};

static struct gain_tbl_info_s AgainInfo[4] = {
	{
		.gainMax = 1984,
		.idxBase = 0,
		.regGain = 0x00,
		.regGainFineBase = 0x10,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 3968,
		.idxBase = 16,
		.regGain = 0x01,
		.regGainFineBase = 0x10,
		.regGainFineStep = 1,
	},
/* vendor suggest the maximum again shall not be larger than 4X*/
	{
		.gainMax = 7936,
		.idxBase = 32,
		.regGain = 0x03,
		.regGainFineBase = 0x10,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 15872,
		.idxBase = 48,
		.regGain = 0x07,
		.regGainFineBase = 0x10,
		.regGainFineStep = 1,
	},
};

static CVI_U32 Again_table[] = {
	1024, 1088, 1152, 1216, 1280, 1344, 1408, 1472, 1536, 1600, 1664, 1728, 1792,
	1856, 1920, 1984, 2048, 2176, 2304, 2432, 2560, 2688, 2816, 2944, 3072, 3200,
	3328, 3456, 3584, 3712, 3840, 3968, 4096, 4352, 4608, 4864, 5120, 5376, 5632,
	5888, 6144, 6400, 6656, 6912, 7168, 7424, 7680, 7936, 8192, 8704, 9216, 9728,
	10240, 10752, 11264, 11776, 12288, 12800, 13312, 13824, 14336, 14848, 15360, 15872
 };

static const CVI_U32 again_table_size = ARRAY_SIZE(Again_table);

static CVI_S32 cmos_again_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32AgainLin, CVI_U32 *pu32AgainDb)
{
	CVI_U32 i;

	(void) ViPipe;

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

static struct gain_tbl_info_s DgainInfo[3] = {
	{
		.gainMax = 1984,
		.idxBase = 0,
		.regGain = 0x00,
		.regGainFineBase = 0x80,
		.regGainFineStep = 8,
	},
	{
		.gainMax = 3968,
		.idxBase = 16,
		.regGain = 0x01,
		.regGainFineBase = 0x80,
		.regGainFineStep = 8,
	},
	{
		.gainMax = 7936,
		.idxBase = 32,
		.regGain = 0x03,
		.regGainFineBase = 0x80,
		.regGainFineStep = 8,
	},
};

static CVI_U32 Dgain_table[] = {
	1024, 1088, 1152, 1216, 1280, 1344, 1408, 1472, 1536, 1600, 1664, 1728, 1792,
	1856, 1920, 1984, 2048, 2176, 2304, 2432, 2560, 2688, 2816, 2944, 3072, 3200,
	3328, 3456, 3584, 3712, 3840, 3968, 4096, 4352, 4608, 4864, 5120, 5376, 5632,
	5888, 6144, 6400, 6656, 6912, 7168, 7424, 7680, 7936
};

static const CVI_U32 dgain_table_size = ARRAY_SIZE(Dgain_table);

static CVI_S32 cmos_dgain_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32DgainLin, CVI_U32 *pu32DgainDb)
{
	CVI_U32 i;

	(void) ViPipe;

	CMOS_CHECK_POINTER(pu32DgainLin);
	CMOS_CHECK_POINTER(pu32DgainDb);

	if (*pu32DgainLin >= Dgain_table[dgain_table_size - 1]) {
		*pu32DgainLin = Dgain_table[dgain_table_size - 1];
		*pu32DgainDb = dgain_table_size - 1;
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
	struct gain_tbl_info_s *info;
	int i, tbl_num;

	SC035HGS_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pu32Again);
	CMOS_CHECK_POINTER(pu32Dgain);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	u32Again = pu32Again[0];
	u32Dgain = pu32Dgain[0];

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		/* linear mode */

		/* find Again register setting. */
		tbl_num = sizeof(AgainInfo)/sizeof(struct gain_tbl_info_s);
		for (i = tbl_num - 1; i >= 0; i--) {
			info = &AgainInfo[i];

			if (u32Again >= info->idxBase)
				break;
		}

		/* gain logic setting. */
		if (u32Again<16) {//gain<2
			pstSnsRegsInfo->astI2cData[LINEAR_GAIN_MAGIC_0_ADDR].u32Data = 0x6b;
			pstSnsRegsInfo->astI2cData[LINEAR_GAIN_MAGIC_1_ADDR].u32Data = 0x14;
			pstSnsRegsInfo->astI2cData[LINEAR_GAIN_MAGIC_2_ADDR].u32Data = 0x58;
			pstSnsRegsInfo->astI2cData[LINEAR_GAIN_MAGIC_3_ADDR].u32Data = 0x4a;
		} else if (u32Again < 32) {//[gain<4]
			pstSnsRegsInfo->astI2cData[LINEAR_GAIN_MAGIC_0_ADDR].u32Data = 0x4f;
			pstSnsRegsInfo->astI2cData[LINEAR_GAIN_MAGIC_1_ADDR].u32Data = 0x10;
			pstSnsRegsInfo->astI2cData[LINEAR_GAIN_MAGIC_2_ADDR].u32Data = 0x48;
			pstSnsRegsInfo->astI2cData[LINEAR_GAIN_MAGIC_3_ADDR].u32Data = 0x4c;
		} else {
			pstSnsRegsInfo->astI2cData[LINEAR_GAIN_MAGIC_0_ADDR].u32Data = 0x74;
			pstSnsRegsInfo->astI2cData[LINEAR_GAIN_MAGIC_1_ADDR].u32Data = 0x15;
			pstSnsRegsInfo->astI2cData[LINEAR_GAIN_MAGIC_2_ADDR].u32Data = 0x48;
			pstSnsRegsInfo->astI2cData[LINEAR_GAIN_MAGIC_3_ADDR].u32Data = 0x4c;
		}

		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_H_ADDR].u32Data = (info->regGain << 2) | 0x3;
		u32Again = info->regGainFineBase + (u32Again - info->idxBase) * info->regGainFineStep;
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_L_ADDR].u32Data = (u32Again & 0xFF);

		/* find Dgain register setting. */
		tbl_num = sizeof(DgainInfo)/sizeof(struct gain_tbl_info_s);
		for (i = tbl_num - 1; i >= 0; i--) {
			info = &DgainInfo[i];

			if (u32Dgain >= info->idxBase)
				break;
		}

		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_H_ADDR].u32Data = 0x0C | info->regGain;
		u32Dgain = info->regGainFineBase + (u32Dgain - info->idxBase) * info->regGainFineStep;
		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_L_ADDR].u32Data = (u32Dgain & 0xFF);

	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support WDR: %d\n", pstSnsState->enWDRMode);
		return CVI_FAILURE;
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
	//pstExpFuncs->pfn_cmos_get_inttime_max   = cmos_get_inttime_max;
	//pstExpFuncs->pfn_cmos_ae_fswdr_attr_set = cmos_ae_fswdr_attr_set;

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

	memcpy(pstBlc, &g_stIspBlcCalibratio, sizeof(ISP_CMOS_BLACK_LEVEL_S));
	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg)
{
	const SC035HGS_1L_SLAVE_MODE_S *pstMode = CVI_NULL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC035HGS_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstMode = &g_astSC035HGS_1L_SLAVE_mode[pstSnsState->u8ImgMode];

	pstIspCfg->frm_num = 1;
	memcpy(&pstIspCfg->img_size[0], &pstMode->astImg[0], sizeof(ISP_WDR_SIZE_S));

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_wdr_mode(VI_PIPE ViPipe, CVI_U8 u8Mode)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC035HGS_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstSnsState->bSyncInit = CVI_FALSE;

	switch (u8Mode) {
	case WDR_MODE_NONE:
		pstSnsState->u8ImgMode = SC035HGS_1L_SLAVE_MODE_640X480P120;
		pstSnsState->enWDRMode = WDR_MODE_NONE;
		pstSnsState->u32FLStd = g_astSC035HGS_1L_SLAVE_mode[pstSnsState->u8ImgMode].u32VtsDef;
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
	SC035HGS_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstSnsRegsInfo = &pstSnsSyncInfo->snsCfg;
	pstCfg0 = &pstSnsState->astSyncInfo[0];
	pstCfg1 = &pstSnsState->astSyncInfo[1];
	pstI2c_data = pstCfg0->snsCfg.astI2cData;

	if ((pstSnsState->bSyncInit == CVI_FALSE) || (pstSnsRegsInfo->bConfig == CVI_FALSE)) {
		pstCfg0->snsCfg.enSnsType = SNS_I2C_TYPE;
		pstCfg0->snsCfg.unComBus.s8I2cDev = g_aunSC035HGS_1L_SLAVE_BusInfo[ViPipe].s8I2cDev;
		pstCfg0->snsCfg.u8Cfg2ValidDelayMax = 1;
		pstCfg0->snsCfg.use_snsr_sram = CVI_TRUE;
		pstCfg0->snsCfg.u32RegNum = LINEAR_REGS_NUM;

		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			pstI2c_data[i].bUpdate = CVI_TRUE;
			pstI2c_data[i].u8DevAddr = sc035hgs_1L_slave_i2c_addr;
			pstI2c_data[i].u32AddrByteNum = sc035hgs_1L_slave_addr_byte;
			pstI2c_data[i].u32DataByteNum = sc035hgs_1L_slave_data_byte;
		}

		switch (pstSnsState->enWDRMode) {
		case WDR_MODE_NONE:
			//Linear Mode Regs
			pstI2c_data[LINEAR_EXP_H_ADDR].u32RegAddr = SC035HGS_1L_SLAVE_EXP_H_ADDR;
			pstI2c_data[LINEAR_EXP_L_ADDR].u32RegAddr = SC035HGS_1L_SLAVE_EXP_L_ADDR;
			pstI2c_data[LINEAR_AGAIN_H_ADDR].u32RegAddr = SC035HGS_1L_SLAVE_AGAIN_H_ADDR;
			pstI2c_data[LINEAR_AGAIN_H_ADDR].u8DelayFrmNum = 1;
			pstI2c_data[LINEAR_AGAIN_L_ADDR].u32RegAddr = SC035HGS_1L_SLAVE_AGAIN_L_ADDR;
			pstI2c_data[LINEAR_AGAIN_L_ADDR].u8DelayFrmNum = 1;
			pstI2c_data[LINEAR_DGAIN_H_ADDR].u32RegAddr = SC035HGS_1L_SLAVE_DGAIN_H_ADDR;
			pstI2c_data[LINEAR_DGAIN_H_ADDR].u8DelayFrmNum = 1;
			pstI2c_data[LINEAR_DGAIN_L_ADDR].u32RegAddr = SC035HGS_1L_SLAVE_DGAIN_L_ADDR;
			pstI2c_data[LINEAR_DGAIN_L_ADDR].u8DelayFrmNum = 1;
			pstI2c_data[LINEAR_VMAX_H_ADDR].u32RegAddr = SC035HGS_1L_SLAVE_VMAX_H_ADDR;
			pstI2c_data[LINEAR_VMAX_L_ADDR].u32RegAddr = SC035HGS_1L_SLAVE_VMAX_L_ADDR;
			pstI2c_data[LINEAR_GAIN_MAGIC_0_ADDR].bUpdate = CVI_FALSE;
			pstI2c_data[LINEAR_GAIN_MAGIC_0_ADDR].u32RegAddr = SC035HGS_1L_SLAVE_GAIN_MAGIC_0_ADDR;
			pstI2c_data[LINEAR_GAIN_MAGIC_1_ADDR].bUpdate = CVI_FALSE;
			pstI2c_data[LINEAR_GAIN_MAGIC_1_ADDR].u32RegAddr = SC035HGS_1L_SLAVE_GAIN_MAGIC_1_ADDR;
			pstI2c_data[LINEAR_GAIN_MAGIC_2_ADDR].bUpdate = CVI_FALSE;
			pstI2c_data[LINEAR_GAIN_MAGIC_2_ADDR].u32RegAddr = SC035HGS_1L_SLAVE_GAIN_MAGIC_2_ADDR;
			pstI2c_data[LINEAR_GAIN_MAGIC_3_ADDR].bUpdate = CVI_FALSE;
			pstI2c_data[LINEAR_GAIN_MAGIC_3_ADDR].u32RegAddr = SC035HGS_1L_SLAVE_GAIN_MAGIC_3_ADDR;
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

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_image_mode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
	CVI_U8 u8SensorImageMode = 0;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstSensorImageMode);
	SC035HGS_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8SensorImageMode = pstSnsState->u8ImgMode;
	pstSnsState->bSyncInit = CVI_FALSE;

	if (pstSensorImageMode->f32Fps <= 120) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (SC035HGS_1L_SLAVE_RES_IS_480P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height)) {
				u8SensorImageMode = SC035HGS_1L_SLAVE_MODE_640X480P120;
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

	SC035HGS_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);
	if (pstSnsState->bInit == CVI_TRUE && g_aeSc035hgs_slave_MirrorFip[ViPipe] != eSnsMirrorFlip) {
		sc035hgs_1L_slave_mirror_flip(ViPipe, eSnsMirrorFlip);
		g_aeSc035hgs_slave_MirrorFip[ViPipe] = eSnsMirrorFlip;
	}
}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	const SC035HGS_1L_SLAVE_MODE_S *pstMode = CVI_NULL;

	SC035HGS_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsState->bInit = CVI_FALSE;
	pstSnsState->bSyncInit = CVI_FALSE;
	pstSnsState->u8ImgMode = SC035HGS_1L_SLAVE_MODE_640X480P120;
	pstSnsState->enWDRMode = WDR_MODE_NONE;
	pstMode = &g_astSC035HGS_1L_SLAVE_mode[pstSnsState->u8ImgMode];
	pstSnsState->u32FLStd  = pstMode->u32VtsDef;
	pstSnsState->au32FL[0] = pstMode->u32VtsDef;
	pstSnsState->au32FL[1] = pstMode->u32VtsDef;

	memset(&pstSnsState->astSyncInfo[0], 0, sizeof(ISP_SNS_SYNC_INFO_S));
	memset(&pstSnsState->astSyncInfo[1], 0, sizeof(ISP_SNS_SYNC_INFO_S));
}

static CVI_S32 sensor_rx_attr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *pstRxAttr)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC035HGS_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pstRxAttr);

	memcpy(pstRxAttr, &sc035hgs_1L_slave_rx_attr, sizeof(*pstRxAttr));

	pstRxAttr->img_size.width = g_astSC035HGS_1L_SLAVE_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Width;
	pstRxAttr->img_size.height = g_astSC035HGS_1L_SLAVE_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Height;
	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 sensor_patch_rx_attr(RX_INIT_ATTR_S *pstRxInitAttr)
{
	SNS_COMBO_DEV_ATTR_S *pstRxAttr = &sc035hgs_1L_slave_rx_attr;
	int i;

	CMOS_CHECK_POINTER(pstRxInitAttr);

	if (pstRxInitAttr->stMclkAttr.bMclkEn)
		pstRxAttr->mclk.cam = pstRxInitAttr->stMclkAttr.u8Mclk;

	if (pstRxInitAttr->MipiDev >= VI_MAX_DEV_NUM)
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

	pstSensorExpFunc->pfn_cmos_sensor_init = sc035hgs_1L_slave_init;
	pstSensorExpFunc->pfn_cmos_sensor_exit = sc035hgs_1L_slave_exit;
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
	if (SC035HGS_1L_SLAVE_I2C_ADDR_IS_VALID(s32I2cAddr))
		sc035hgs_1L_slave_i2c_addr = s32I2cAddr;
}

static CVI_S32 sc035hgs_1L_slave_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
	g_aunSC035HGS_1L_SLAVE_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	SC035HGS_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

	if (pastSnsStateCtx == CVI_NULL) {
		pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
		if (pastSnsStateCtx == CVI_NULL) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
			return -ENOMEM;
		}
	}

	memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

	SC035HGS_1L_SLAVE_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

	return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	SC035HGS_1L_SLAVE_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
	SENSOR_FREE(pastSnsStateCtx);
	SC035HGS_1L_SLAVE_SENSOR_RESET_CTX(ViPipe);
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

	stSnsAttrInfo.eSensorId = SC035HGS_1L_SLAVE_ID;

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

	s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, SC035HGS_1L_SLAVE_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, SC035HGS_1L_SLAVE_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, SC035HGS_1L_SLAVE_ID);
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
	g_au16SC035HGS_1L_SLAVE_GainMode[ViPipe] = pstInitAttr->enGainMode;
	g_au16SC035HGS_1L_SLAVE_L2SMode[ViPipe] = pstInitAttr->enL2SMode;

	return CVI_SUCCESS;
}

ISP_SNS_OBJ_S stSnsSC035HGS_1L_slave_Obj = {
	.pfnRegisterCallback    = sensor_register_callback,
	.pfnUnRegisterCallback  = sensor_unregister_callback,
	.pfnStandby             = sc035hgs_1L_slave_standby,
	.pfnRestart             = sc035hgs_1L_slave_restart,
	.pfnMirrorFlip          = sensor_mirror_flip,
	.pfnWriteReg            = sc035hgs_1L_slave_write_register,
	.pfnReadReg             = sc035hgs_1L_slave_read_register,
	.pfnSetBusInfo          = sc035hgs_1L_slave_set_bus_info,
	.pfnSetInit             = sensor_set_init,
	.pfnPatchRxAttr		    = sensor_patch_rx_attr,
	.pfnPatchI2cAddr	    = sensor_patch_i2c_addr,
	.pfnGetRxAttr	    	= sensor_rx_attr,
	.pfnExpSensorCb		    = cmos_init_sensor_exp_function,
	.pfnExpAeCb		        = cmos_init_ae_exp_function,
	.pfnSnsProbe		    = sc035hgs_1L_slave_probe,
};

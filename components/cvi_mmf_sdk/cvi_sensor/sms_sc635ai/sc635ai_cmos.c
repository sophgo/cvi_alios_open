#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <cvi_type.h>
#include <cvi_comm_video.h>
#include "cvi_debug.h"
#include "cvi_comm_sns.h"
#include "cvi_sns_ctrl.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_isp.h"

#include "sc635ai_cmos_ex.h"
#include "sc635ai_cmos_param.h"

#define DIV_0_TO_1(a)   ((0 == (a)) ? 1 : (a))
#define DIV_0_TO_1_FLOAT(a) ((((a) < 1E-10) && ((a) > -1E-10)) ? 1 : (a))
#define SC635AI_ID 635
#define SC635AI_I2C_ADDR_1 0x30
#define SC635AI_I2C_ADDR_2 0x32
#define SC635AI_I2C_ADDR_IS_VALID(addr) ((addr) == SC635AI_I2C_ADDR_1 || (addr) == SC635AI_I2C_ADDR_2)

#define SENSOR_SC635AI_WIDTH 3200
#define SENSOR_SC635AI_HEIGHT 1800

#define SC635AI_EXPACCURACY                    (1)

/****************************************************************************
 * global variables                                                            *
 ****************************************************************************/

ISP_SNS_STATE_S *g_pastSC635AI[VI_MAX_PIPE_NUM] = {CVI_NULL};
SNS_COMBO_DEV_ATTR_S* g_pastSC635AIComboDevArray[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define SC635AI_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastSC635AI[dev])
#define SC635AI_SENSOR_SET_CTX(dev, pstCtx)   (g_pastSC635AI[dev] = pstCtx)
#define SC635AI_SENSOR_RESET_CTX(dev)         (g_pastSC635AI[dev] = CVI_NULL)
#define SC635AI_SENSOR_GET_COMBO(dev, pstCtx)   (pstCtx = g_pastSC635AIComboDevArray[dev])
#define SC635AI_SENSOR_SET_COMBO(dev, pstCtx)   (g_pastSC635AIComboDevArray[dev] = pstCtx)

ISP_SNS_COMMBUS_U g_aunSC635AI_BusInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cDev = 0},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

ISP_SNS_COMMADDR_U g_aunSC635AI_AddrInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cAddr = 0},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cAddr = -1}
};

CVI_U16 g_au16SC635AI_GainMode[VI_MAX_PIPE_NUM] = {0};
CVI_U16 g_au16SC635AI_L2SMode[VI_MAX_PIPE_NUM] = {0};

/****************************************************************************
 * local variables and functions                                             *
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
/*****SC635AI Lines Range*****/
#define SC635AI_FULL_LINES_MAX  (0xFFF0)

/*****SC635AI Register Address*****/
#define SC635AI_SHS1_0_ADDR		0x3E00 //bit[4:0] H
#define SC635AI_SHS1_1_ADDR		0x3E01 //bit[7:0] M
#define SC635AI_SHS1_2_ADDR		0x3E02 //bit[7:4] L

#define SC635AI_AGAIN_ADDR		0x3E08
#define SC635AI_A_FINEGAIN_ADDR	0x3E09
#define SC635AI_DGAIN_ADDR		0x3E06
#define SC635AI_D_FINEGAIN_ADDR	0x3E07

#define SC635AI_VMAX_ADDR		0x320E //(0x320e[7:0],0x320f)

#define SC635AI_GROUP_HOLD_ADDR	0x3812

#define SC635AI_RES_IS_1800P(w, h)      ((w) == 3200 && (h) == 1800)

static CVI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	SC635AI_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FlickerFreq = 50 * 256;
	pstAeSnsDft->u32FullLinesMax = SC635AI_FULL_LINES_MAX;
	pstAeSnsDft->u32HmaxTimes =
		(1000000) / (pstSnsState->u32FLStd * g_astSC635AI_mode[pstSnsState->u8ImgMode].f32MaxFps);

	pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
	pstAeSnsDft->stIntTimeAccu.f32Accuracy = SC635AI_EXPACCURACY;
	pstAeSnsDft->stIntTimeAccu.f32Offset = 0;

	pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stAgainAccu.f32Accuracy = 1;

	pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stDgainAccu.f32Accuracy = 1;

	pstAeSnsDft->u32ISPDgainShift = 8;
	pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
	pstAeSnsDft->u32MaxISPDgainTarget = 2 << pstAeSnsDft->u32ISPDgainShift;

	if (g_au32LinesPer500ms[ViPipe] == 0)
		pstAeSnsDft->u32LinesPer500ms =
			pstSnsState->u32FLStd * g_astSC635AI_mode[pstSnsState->u8ImgMode].f32MaxFps / 2;
	else
		pstAeSnsDft->u32LinesPer500ms = g_au32LinesPer500ms[ViPipe];
	pstAeSnsDft->u32SnsStableFrame = 0;

	switch (pstSnsState->enWDRMode) {
	case WDR_MODE_NONE:   /*linear mode*/
		pstAeSnsDft->f32Fps = g_astSC635AI_mode[pstSnsState->u8ImgMode].f32MaxFps;
		pstAeSnsDft->f32MinFps = g_astSC635AI_mode[pstSnsState->u8ImgMode].f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xd;
		pstAeSnsDft->au8HistThresh[1] = 0x28;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxAgain = g_astSC635AI_mode[pstSnsState->u8ImgMode].stAgain[0].u32Max;
		pstAeSnsDft->u32MinAgain = g_astSC635AI_mode[pstSnsState->u8ImgMode].stAgain[0].u32Min;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = g_astSC635AI_mode[pstSnsState->u8ImgMode].stDgain[0].u32Max;
		pstAeSnsDft->u32MinDgain = g_astSC635AI_mode[pstSnsState->u8ImgMode].stDgain[0].u32Min;
		pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
		pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

		pstAeSnsDft->u8AeCompensation = 40;
		pstAeSnsDft->u32InitAESpeed = 64;
		pstAeSnsDft->u32InitAETolerance = 5;
		pstAeSnsDft->u32AEResponseFrame = 4;
		pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe] ? g_au32InitExposure[ViPipe] : 76151;

		pstAeSnsDft->u32MaxIntTime = g_astSC635AI_mode[pstSnsState->u8ImgMode].stExp[0].u32Max;
		pstAeSnsDft->u32MinIntTime = g_astSC635AI_mode[pstSnsState->u8ImgMode].stExp[0].u32Min;
		pstAeSnsDft->u32MaxIntTimeTarget = 65535;
		pstAeSnsDft->u32MinIntTimeTarget = 1;
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support sensor mode: %d\n", pstSnsState->u8ImgMode);
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
	SC635AI_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32Vts = g_astSC635AI_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	f32MaxFps = g_astSC635AI_mode[pstSnsState->u8ImgMode].f32MaxFps;
	f32MinFps = g_astSC635AI_mode[pstSnsState->u8ImgMode].f32MinFps;

	switch (pstSnsState->u8ImgMode) {
	case SC635AI_MODE_1800P60:
		if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
			u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
			return CVI_FAILURE;
		}
		u32VMAX = (u32VMAX > SC635AI_FULL_LINES_MAX) ? SC635AI_FULL_LINES_MAX : u32VMAX;
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support sensor mode: %d\n", pstSnsState->u8ImgMode);
		return CVI_FAILURE;
	}

	pstSnsState->u32FLStd = u32VMAX;

	pstSnsRegsInfo->astI2cData[LINEAR_VMAX_0_ADDR].u32Data = ((u32VMAX & 0xFF00) >> 8);
	pstSnsRegsInfo->astI2cData[LINEAR_VMAX_1_ADDR].u32Data = (u32VMAX & 0xFF);

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

	SC635AI_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(u32IntTime);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	CVI_U32 u32TmpIntTime = u32IntTime[0];
	/* linear exposure reg range:
	 * min : 2
	 * max : (vts - 8)
	 * step : 1
	 */
	u32TmpIntTime = (u32TmpIntTime > ((pstSnsState->au32FL[0]) - 8)) ?
			((pstSnsState->au32FL[0]) - 8) : u32TmpIntTime;
	if (!u32TmpIntTime)
		u32TmpIntTime = 2;

	pstSnsRegsInfo->astI2cData[LINEAR_SHS1_0_ADDR].u32Data = ((u32TmpIntTime & 0x1F000) >> 12); //bit[16:12]
	pstSnsRegsInfo->astI2cData[LINEAR_SHS1_1_ADDR].u32Data = ((u32TmpIntTime & 0x0FF0) >> 4);  //bit[11:4]
	pstSnsRegsInfo->astI2cData[LINEAR_SHS1_2_ADDR].u32Data = ((u32TmpIntTime & 0x000F) << 4);  //bit[7:4]

	return CVI_SUCCESS;

}

struct gain_tbl_info_s {
	CVI_U32	gainMax;
	CVI_U16	idxBase;
	CVI_U8	regGain;
	CVI_U8	regGainFineBase;
	CVI_U8	regGainFineStep;
};

static struct gain_tbl_info_s AgainInfo[7] = {
	{
		.gainMax = 2016,
		.idxBase = 0,
		.regGain = 0x00,
		.regGainFineBase = 0x20,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 2688,
		.idxBase = 32,
		.regGain = 0x01,
		.regGainFineBase = 0x20,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 5362,
		.idxBase = 43,
		.regGain = 0x80,
		.regGainFineBase = 0x21,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 10725,
		.idxBase = 74,
		.regGain = 0x81,
		.regGainFineBase = 0x20,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 21450,
		.idxBase = 106,
		.regGain = 0x83,
		.regGainFineBase = 0x20,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 42900,
		.idxBase = 138,
		.regGain = 0x87,
		.regGainFineBase = 0x20,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 85800,
		.idxBase = 170,
		.regGain = 0x8f,
		.regGainFineBase = 0x20,
		.regGainFineStep = 1,
	},
};

static CVI_U32 Again_table[] = {
	1024, 1055, 1088, 1120, 1152, 1183, 1216, 1248, 1280, 1311,
	1344, 1376, 1408, 1439, 1472, 1504, 1536, 1567, 1600, 1632,
	1664, 1695, 1728, 1760, 1792, 1823, 1856, 1888, 1920, 1951,
	1984, 2016, 2048, 2112, 2176, 2240, 2304, 2368, 2432, 2496,
	2560, 2624, 2688, /*2723,*/ 2808, 2893, 2978, 3064, 3149, 3234,
	3319, 3404, 3489, 3574, 3659, 3745, 3830, 3915, 4000, 4085,
	4170, 4255, 4340, 4426, 4511, 4596, 4681, 4766, 4851, 4936,
	5021, 5107, 5192, 5277, 5362, 5447, 5617, 5788, 5958, 6128,
	6298, 6469, 6639, 6809, 6979, 7150, 7320, 7490, 7660, 7831,
	8001, 8171, 8341, 8512, 8682, 8852, 9022, 9193, 9363, 9533,
	9703, 9874, 10044, 10214, 10384, 10555, 10725, 10895, 11236, 11576,
	11917, 12257, 12598, 12938, 13279, 13619, 13960, 14300, 14641, 14981,
	15322, 15662, 16003, 16343, 16684, 17024, 17364, 17704, 18045, 18385,
	18726, 19066, 19407, 19747, 20088, 20428, 20769, 21109, 21450, 21790,
	22471, 23152, 23833, 24514, 25195, 25876, 26557, 27238, 27919, 28600,
	29281, 29962, 30643, 31324, 32005, 32686, 33367, 34048, 34728, 35409,
	36090, 36771, 37452, 38133, 38814, 39495, 40176, 40857, 41538, 42219,
	42900, 43581, 44943, 46305, 47667, 49029, 50391, 51752, 53114, 54476,
	55838, 57200, 58562, 59924, 61286, 62648, 64010, 65372, 66734, 68096,
	69457, 70819, 72181, 73543, 74905, 76267, 77629, 78991, 80353, 81715,
	83077, 84439, 85800
};

static struct gain_tbl_info_s DgainInfo[4] = {
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
	{
		.gainMax = 8064,
		.idxBase = 64,
		.regGain = 0x03,
		.regGainFineBase = 0x80,
		.regGainFineStep = 4,
	},
	{
		.gainMax = 16128,
		.idxBase = 96,
		.regGain = 0x07,
		.regGainFineBase = 0x80,
		.regGainFineStep = 4,
	},
};

static CVI_U32 Dgain_table[] = {
	1024, 1055, 1088, 1120, 1152, 1183, 1216, 1248, 1280, 1311,
	1344, 1376, 1408, 1439, 1472, 1504, 1536, 1567, 1600, 1632,
	1664, 1695, 1728, 1760, 1792, 1823, 1856, 1888, 1920, 1951,
	1984, 2016, 2048, 2112, 2176, 2240, 2304, 2368, 2432, 2496,
	2560, 2624, 2688, 2752, 2816, 2880, 2944, 3008, 3072, 3136,
	3200, 3264, 3328, 3392, 3456, 3520, 3584, 3648, 3712, 3776,
	3840, 3904, 3968, 4032, 4096, 4224, 4352, 4480, 4608, 4736,
	4864, 4992, 5120, 5248, 5376, 5504, 5632, 5760, 5888, 6016,
	6144, 6272, 6400, 6528, 6656, 6784, 6912, 7040, 7168, 7296,
	7424, 7552, 7680, 7808, 7936, 8064, 8192, 8448, 8704, 8960,
	9216, 9472, 9728, 9984, 10240, 10496, 10752, 11008, 11264, 11520,
	11776, 12032, 12288, 12544, 12800, 13056, 13312, 13568, 13824, 14080,
	14336, 14592, 14848, 15104, 15360, 15616, 15872, 16128
};

static CVI_S32 cmos_again_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32AgainLin, CVI_U32 *pu32AgainDb)
{
	int i;
	const int array_size = sizeof(Again_table)/sizeof(Again_table[0]);

	(void)ViPipe;

	CMOS_CHECK_POINTER(pu32AgainLin);
	CMOS_CHECK_POINTER(pu32AgainDb);

	if (*pu32AgainLin >= Again_table[array_size - 1]) {
		*pu32AgainLin = Again_table[array_size - 1];
		*pu32AgainDb = array_size - 1;
		return CVI_SUCCESS;
	}

	for (i = 1; i < array_size; i++) {
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
	const int array_size = sizeof(Dgain_table)/sizeof(Dgain_table[0]);

	(void)ViPipe;

	CMOS_CHECK_POINTER(pu32DgainLin);
	CMOS_CHECK_POINTER(pu32DgainDb);

	if (*pu32DgainLin >= Dgain_table[array_size - 1]) {
		*pu32DgainLin = Dgain_table[array_size - 1];
		*pu32DgainDb = array_size - 1;
		return CVI_SUCCESS;
	}

	for (i = 1; i < array_size; i++) {
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

	SC635AI_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pu32Again);
	CMOS_CHECK_POINTER(pu32Dgain);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	u32Again = pu32Again[0];
	u32Dgain = pu32Dgain[0];

	/* find Again register setting. */
	tbl_num = sizeof(AgainInfo)/sizeof(struct gain_tbl_info_s);
	for (i = tbl_num - 1; i >= 0; i--) {
		info = &AgainInfo[i];

		if (u32Again >= info->idxBase)
			break;
	}

	pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_ADDR].u32Data = (info->regGain & 0xFF);
	u32Again = info->regGainFineBase + (u32Again - info->idxBase) * info->regGainFineStep;
	pstSnsRegsInfo->astI2cData[LINEAR_A_FINEGAIN_ADDR].u32Data = (u32Again & 0xFF);

	/* find Dgain register setting. */
	tbl_num = sizeof(DgainInfo)/sizeof(struct gain_tbl_info_s);
	for (i = tbl_num - 1; i >= 0; i--) {
		info = &DgainInfo[i];

		if (u32Dgain >= info->idxBase)
			break;
	}

	pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_ADDR].u32Data = (info->regGain & 0xFF);
	u32Dgain = info->regGainFineBase + (u32Dgain - info->idxBase) * info->regGainFineStep;
	pstSnsRegsInfo->astI2cData[LINEAR_D_FINEGAIN_ADDR].u32Data = (u32Dgain & 0xFF);

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_inttime_max(VI_PIPE ViPipe, CVI_U16 u16ManRatioEnable, CVI_U32 *au32Ratio,
		CVI_U32 *au32IntTimeMax, CVI_U32 *au32IntTimeMin, CVI_U32 *pu32LFMaxIntTime)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	(void) u16ManRatioEnable;

	CMOS_CHECK_POINTER(au32Ratio);
	CMOS_CHECK_POINTER(au32IntTimeMax);
	CMOS_CHECK_POINTER(au32IntTimeMin);
	CMOS_CHECK_POINTER(pu32LFMaxIntTime);
	SC635AI_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

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
	const SC635AI_MODE_S *pstMode = CVI_NULL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC635AI_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstMode = &g_astSC635AI_mode[pstSnsState->u8ImgMode];

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

	SC635AI_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstSnsState->bSyncInit = CVI_FALSE;

	switch (u8Mode) {
	case WDR_MODE_NONE:
		pstSnsState->u8ImgMode = SC635AI_MODE_1800P60;
		pstSnsState->enWDRMode = WDR_MODE_NONE;
		pstSnsState->u32FLStd = g_astSC635AI_mode[pstSnsState->u8ImgMode].u32VtsDef;
		CVI_TRACE_SNS(CVI_DBG_INFO, "linear mode\n");
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
	SC635AI_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstSnsRegsInfo = &pstSnsSyncInfo->snsCfg;
	pstCfg0 = &pstSnsState->astSyncInfo[0];
	pstCfg1 = &pstSnsState->astSyncInfo[1];
	pstI2c_data = pstCfg0->snsCfg.astI2cData;

	if ((pstSnsState->bSyncInit == CVI_FALSE) || (pstSnsRegsInfo->bConfig == CVI_FALSE)) {
		pstCfg0->snsCfg.enSnsType = SNS_I2C_TYPE;
		pstCfg0->snsCfg.unComBus.s8I2cDev = g_aunSC635AI_BusInfo[ViPipe].s8I2cDev;
		pstCfg0->snsCfg.u8Cfg2ValidDelayMax = 0;
		pstCfg0->snsCfg.use_snsr_sram = CVI_TRUE;
		pstCfg0->snsCfg.u32RegNum = LINEAR_REGS_NUM;

		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			pstI2c_data[i].bUpdate = CVI_TRUE;
			pstI2c_data[i].u8DevAddr = g_aunSC635AI_AddrInfo[ViPipe].s8I2cAddr;
			pstI2c_data[i].u32AddrByteNum = sc635ai_addr_byte;
			pstI2c_data[i].u32DataByteNum = sc635ai_data_byte;
		}
		switch (pstSnsState->enWDRMode) {
		case WDR_MODE_NONE:
			//Linear Mode Regs
			pstI2c_data[LINEAR_HOLD_START].u32RegAddr     = SC635AI_GROUP_HOLD_ADDR;
			pstI2c_data[LINEAR_HOLD_START].u32Data = 0x00;
			pstI2c_data[LINEAR_SHS1_0_ADDR].u32RegAddr     = SC635AI_SHS1_0_ADDR;
			pstI2c_data[LINEAR_SHS1_1_ADDR].u32RegAddr     = SC635AI_SHS1_1_ADDR;
			pstI2c_data[LINEAR_SHS1_2_ADDR].u32RegAddr     = SC635AI_SHS1_2_ADDR;

			pstI2c_data[LINEAR_AGAIN_ADDR].u32RegAddr      = SC635AI_AGAIN_ADDR;
			pstI2c_data[LINEAR_A_FINEGAIN_ADDR].u32RegAddr = SC635AI_A_FINEGAIN_ADDR;
			pstI2c_data[LINEAR_DGAIN_ADDR].u32RegAddr      = SC635AI_DGAIN_ADDR;
			pstI2c_data[LINEAR_D_FINEGAIN_ADDR].u32RegAddr = SC635AI_D_FINEGAIN_ADDR;

			pstI2c_data[LINEAR_VMAX_0_ADDR].u32RegAddr     = SC635AI_VMAX_ADDR;
			pstI2c_data[LINEAR_VMAX_1_ADDR].u32RegAddr     = SC635AI_VMAX_ADDR + 1;
			pstI2c_data[LINEAR_HOLD_END].u32RegAddr     = SC635AI_GROUP_HOLD_ADDR;
			pstI2c_data[LINEAR_HOLD_END].u32Data = 0x30;
			break;
		default:
			CVI_TRACE_SNS(CVI_DBG_ERR, "NOT support this mode!\n");
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
		if (pstCfg0->snsCfg.need_update == CVI_TRUE) {
			if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
				//TODO:
			} else {
				pstI2c_data[LINEAR_HOLD_START].bUpdate = CVI_TRUE;
				pstI2c_data[LINEAR_HOLD_END].bUpdate = CVI_TRUE;
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
	SC635AI_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8SensorImageMode = pstSnsState->u8ImgMode;
	pstSnsState->bSyncInit = CVI_FALSE;

	if (pstSensorImageMode->f32Fps <= 60) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (SC635AI_RES_IS_1800P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height)) {
				if (pstSensorImageMode->u8LaneNum == 4) {
					u8SensorImageMode = SC635AI_MODE_1800P60;
				} else {
					CVI_TRACE_SNS(CVI_DBG_ERR, "Current config only support 4lane mode \n");
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
		/* Don't need to switch SensorImageMode */
		return CVI_FAILURE;
	}

	pstSnsState->u8ImgMode = u8SensorImageMode;

	return CVI_SUCCESS;
}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	const SC635AI_MODE_S *pstMode = CVI_NULL;

	SC635AI_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsState->bInit = CVI_FALSE;
	pstSnsState->bSyncInit = CVI_FALSE;
	pstSnsState->u8ImgMode = SC635AI_MODE_1800P60;
	pstSnsState->enWDRMode = WDR_MODE_NONE;
	pstMode = &g_astSC635AI_mode[pstSnsState->u8ImgMode];
	pstSnsState->u32FLStd  = pstMode->u32VtsDef;
	pstSnsState->au32FL[0] = pstMode->u32VtsDef;
	pstSnsState->au32FL[1] = pstMode->u32VtsDef;

	memset(&pstSnsState->astSyncInfo[0], 0, sizeof(ISP_SNS_SYNC_INFO_S));
	memset(&pstSnsState->astSyncInfo[1], 0, sizeof(ISP_SNS_SYNC_INFO_S));
}

static CVI_S32 sensor_rx_attr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *pstRxAttr)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	SNS_COMBO_DEV_ATTR_S *pstRxAttrSrc = CVI_NULL;

	SC635AI_SENSOR_GET_CTX(ViPipe, pstSnsState);
	SC635AI_SENSOR_GET_COMBO(ViPipe, pstRxAttrSrc);

	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pstRxAttr);
	CMOS_CHECK_POINTER(pstRxAttrSrc);

	memcpy(pstRxAttr, pstRxAttrSrc, sizeof(*pstRxAttr));

	pstRxAttr->img_size.start_x = g_astSC635AI_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.s32X;
	pstRxAttr->img_size.start_y = g_astSC635AI_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.s32Y;
	pstRxAttr->img_size.active_w = g_astSC635AI_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.u32Width;
	pstRxAttr->img_size.active_h = g_astSC635AI_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.u32Height;
	pstRxAttr->img_size.width = g_astSC635AI_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Width;
	pstRxAttr->img_size.height = g_astSC635AI_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Height;
	pstRxAttr->img_size.max_width = g_astSC635AI_mode[pstSnsState->u8ImgMode].astImg[0].stMaxSize.u32Width;
	pstRxAttr->img_size.max_height = g_astSC635AI_mode[pstSnsState->u8ImgMode].astImg[0].stMaxSize.u32Height;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;
	}
	pstRxAttrSrc = CVI_NULL;
	return CVI_SUCCESS;

}

static CVI_S32 sensor_patch_rx_attr(VI_PIPE ViPipe, RX_INIT_ATTR_S *pstRxInitAttr)
{
	int i;
	SNS_COMBO_DEV_ATTR_S* pstRxAttr = malloc(sizeof(SNS_COMBO_DEV_ATTR_S));

	if(!g_pastSC635AIComboDevArray[ViPipe]) {
		pstRxAttr = malloc(sizeof(SNS_COMBO_DEV_ATTR_S));
	} else {
		SC635AI_SENSOR_GET_COMBO(ViPipe, pstRxAttr);
	}
	memcpy(pstRxAttr, &sc635ai_rx_attr, sizeof(SNS_COMBO_DEV_ATTR_S));
	SC635AI_SENSOR_SET_COMBO(ViPipe, pstRxAttr);

	CMOS_CHECK_POINTER(pstRxInitAttr);

	if (pstRxInitAttr->stMclkAttr.bMclkEn)
		pstRxAttr->mclk.cam = pstRxInitAttr->stMclkAttr.u8Mclk;

	if (pstRxInitAttr->MipiDev >= VI_MAX_DEV_NUM)
		return CVI_SUCCESS;

	pstRxAttr->devno = pstRxInitAttr->MipiDev;
	pstRxAttr->cif_mode = pstRxInitAttr->MipiMode;

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
	pstRxAttr = CVI_NULL;
	return CVI_SUCCESS;
}

void sc635ai_exit(VI_PIPE ViPipe)
{
	if(g_pastSC635AIComboDevArray[ViPipe]) {
		free(g_pastSC635AIComboDevArray[ViPipe]);
		g_pastSC635AIComboDevArray[ViPipe] = CVI_NULL;
	}
	sc635ai_i2c_exit(ViPipe);
}

static CVI_S32 cmos_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
	CMOS_CHECK_POINTER(pstSensorExpFunc);

	memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

	pstSensorExpFunc->pfn_cmos_sensor_init = sc635ai_init;
	pstSensorExpFunc->pfn_cmos_sensor_exit = sc635ai_exit;
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
static CVI_VOID sensor_patch_i2c_addr(VI_PIPE ViPipe, CVI_S32 s32I2cAddr)
{
	if (SC635AI_I2C_ADDR_IS_VALID(s32I2cAddr))
		g_aunSC635AI_AddrInfo[ViPipe].s8I2cAddr = s32I2cAddr;
	else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C addr input error ,please check [0x%x]\n", s32I2cAddr);
		g_aunSC635AI_AddrInfo[ViPipe].s8I2cAddr = SC635AI_I2C_ADDR_1;
	}
}

static CVI_S32 sc635ai_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
	g_aunSC635AI_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	SC635AI_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

	if (pastSnsStateCtx == CVI_NULL) {
		pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
		if (pastSnsStateCtx == CVI_NULL) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
			return -ENOMEM;
		}
	}

	memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

	SC635AI_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

	return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	SC635AI_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
	SENSOR_FREE(pastSnsStateCtx);
	SC635AI_SENSOR_RESET_CTX(ViPipe);
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

	stSnsAttrInfo.eSensorId = SC635AI_ID;

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

	s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, SC635AI_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, SC635AI_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, SC635AI_ID);
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
	g_au16SC635AI_GainMode[ViPipe] = pstInitAttr->enGainMode;
	g_au16SC635AI_L2SMode[ViPipe] = pstInitAttr->enL2SMode;

	return CVI_SUCCESS;
}

ISP_SNS_OBJ_S stSnsSC635AI_Obj = {
	.pfnRegisterCallback    = sensor_register_callback,
	.pfnUnRegisterCallback  = sensor_unregister_callback,
	.pfnStandby             = sc635ai_standby,
	.pfnRestart             = sc635ai_restart,
	.pfnMirrorFlip          = sc635ai_mirror_flip,
	.pfnWriteReg            = sc635ai_write_register,
	.pfnReadReg             = sc635ai_read_register,
	.pfnSetBusInfo          = sc635ai_set_bus_info,
	.pfnSetInit             = sensor_set_init,
	.pfnPatchRxAttr		= sensor_patch_rx_attr,
	.pfnPatchI2cAddr	= sensor_patch_i2c_addr,
	.pfnGetRxAttr		= sensor_rx_attr,
	.pfnExpSensorCb		= cmos_init_sensor_exp_function,
	.pfnExpAeCb		= cmos_init_ae_exp_function,
	.pfnSnsProbe		= sc635ai_probe,
};


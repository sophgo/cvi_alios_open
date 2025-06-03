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

#include "sc1330_cmos_ex.h"
#include "sc1330_cmos_param.h"

#define DIV_0_TO_1(a)   ((0 == (a)) ? 1 : (a))
#define DIV_0_TO_1_FLOAT(a) ((((a) < 1E-10) && ((a) > -1E-10)) ? 1 : (a))
#define SC1330_ID 1330
#define SENSOR_SC1330_WIDTH 1280
#define SENSOR_SC1330_HEIGHT 960

#define SC1330_I2C_ADDR_1 0x30
#define SC1330_I2C_ADDR_2 0x32
#define SC1330_I2C_ADDR_IS_VALID(addr)      ((addr) == SC1330_I2C_ADDR_1 || (addr) == SC1330_I2C_ADDR_2)
/****************************************************************************
 * global variables                                                            *
 ****************************************************************************/

ISP_SNS_STATE_S *g_pastSC1330[VI_MAX_PIPE_NUM] = {CVI_NULL};
SNS_COMBO_DEV_ATTR_S* g_pastSC1330ComboDevArray[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define SC1330_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastSC1330[dev])
#define SC1330_SENSOR_SET_CTX(dev, pstCtx)   (g_pastSC1330[dev] = pstCtx)
#define SC1330_SENSOR_RESET_CTX(dev)         (g_pastSC1330[dev] = CVI_NULL)
#define SC1330_SENSOR_SET_COMBO(dev, pstCtx)   (g_pastSC1330ComboDevArray[dev] = pstCtx)
#define SC1330_SENSOR_GET_COMBO(dev, pstCtx)   (pstCtx = g_pastSC1330ComboDevArray[dev])

ISP_SNS_COMMBUS_U g_aunSC1330_BusInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cDev = 0},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

ISP_SNS_COMMADDR_U g_aunSC1330_AddrInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cAddr = 0},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cAddr = -1}
};

CVI_U16 g_au16SC1330_GainMode[VI_MAX_PIPE_NUM] = {0};
CVI_U16 g_au16SC1330_L2SMode[VI_MAX_PIPE_NUM] = {0};

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
/*****SC1330 Lines Range*****/
#define SC1330_FULL_LINES_MAX  (0xFFFF)

/*****SC1330 Register Address*****/
#define SC1330_SHS1_0_ADDR		0x3E00 //bit[3:0] H
#define SC1330_SHS1_1_ADDR		0x3E01 //bit[7:0] M
#define SC1330_SHS1_2_ADDR		0x3E02 //bit[7:4] L

#define SC1330_AGAIN_ADDR		0x3E08
#define SC1330_A_FINEGAIN_ADDR	0x3E09
#define SC1330_DGAIN_ADDR		0x3E06
#define SC1330_D_FINEGAIN_ADDR	0x3E07

#define SC1330_VMAX_ADDR		0x320E //(0x320e[7:0],0x320f)

#define SC1330_GROUP_HOLD_ADDR	0x3812

#define SC1330_RES_IS_960P(w, h)      ((w) <= 1280 && (h) <= 960)

static CVI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	SC1330_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FlickerFreq = 50 * 256;
	pstAeSnsDft->u32FullLinesMax = SC1330_FULL_LINES_MAX;
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
	pstAeSnsDft->u32SnsStableFrame = 0;

	switch (pstSnsState->enWDRMode) {
	case WDR_MODE_NONE:   /*linear mode*/
		pstAeSnsDft->f32Fps = g_astSC1330_mode[pstSnsState->u8ImgMode].f32MaxFps;
		pstAeSnsDft->f32MinFps = g_astSC1330_mode[pstSnsState->u8ImgMode].f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xd;
		pstAeSnsDft->au8HistThresh[1] = 0x28;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxAgain = g_astSC1330_mode[pstSnsState->u8ImgMode].stAgain[0].u32Max;
		pstAeSnsDft->u32MinAgain = g_astSC1330_mode[pstSnsState->u8ImgMode].stAgain[0].u32Min;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = g_astSC1330_mode[pstSnsState->u8ImgMode].stDgain[0].u32Max;
		pstAeSnsDft->u32MinDgain = g_astSC1330_mode[pstSnsState->u8ImgMode].stDgain[0].u32Min;
		pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
		pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

		pstAeSnsDft->u8AeCompensation = 40;
		pstAeSnsDft->u32InitAESpeed = 64;
		pstAeSnsDft->u32InitAETolerance = 5;
		pstAeSnsDft->u32AEResponseFrame = 4;
		pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe] ? g_au32InitExposure[ViPipe] : 76151;

		pstAeSnsDft->u32MaxIntTime = g_astSC1330_mode[pstSnsState->u8ImgMode].stExp[0].u32Max;
		pstAeSnsDft->u32MinIntTime = g_astSC1330_mode[pstSnsState->u8ImgMode].stExp[0].u32Min;
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
	SC1330_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32Vts = g_astSC1330_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	f32MaxFps = g_astSC1330_mode[pstSnsState->u8ImgMode].f32MaxFps;
	f32MinFps = g_astSC1330_mode[pstSnsState->u8ImgMode].f32MinFps;

	switch (pstSnsState->u8ImgMode) {
	case SC1330_MODE_960P30:
	case SC1330_MODE_960P30_MASTER:
	case SC1330_MODE_960P30_SLAVE:
	case SC1330_MODE_960P30_1L:
	case SC1330_MODE_960P30_1L_MASTER:
	case SC1330_MODE_960P30_1L_SLAVE:
		if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
			u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
			return CVI_FAILURE;
		}
		u32VMAX = (u32VMAX > SC1330_FULL_LINES_MAX) ? SC1330_FULL_LINES_MAX : u32VMAX;
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
	pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 4;
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

	SC1330_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(u32IntTime);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	CVI_U32 u32TmpIntTime = u32IntTime[0];
	/* linear exposure reg range:
	 * min : 1
	 * max : (vts - 4)
	 * step : 1
	 */
	u32TmpIntTime = (u32TmpIntTime > ((pstSnsState->au32FL[0]) - 4)) ?
			((pstSnsState->au32FL[0]) - 4) : u32TmpIntTime;
	if (!u32TmpIntTime)
		u32TmpIntTime = 1;

	pstSnsRegsInfo->astI2cData[LINEAR_SHS1_0_ADDR].u32Data = ((u32TmpIntTime & 0xF000) >> 12); //bit[15:12]
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

static struct gain_tbl_info_s AgainInfo[] = {
	{
		.gainMax = 2032,
		.idxBase = 0,
		.regGain = 0x03,
		.regGainFineBase = 0x40,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 4064,
		.idxBase = 74,
		.regGain = 0x07,
		.regGainFineBase = 0x40,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 5504,
		.idxBase = 128,
		.regGain = 0x0F,
		.regGainFineBase = 0x40,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 11050,
		.idxBase = 151,
		.regGain = 0x23,
		.regGainFineBase = 0x40,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 22100,
		.idxBase = 215,
		.regGain = 0x27,
		.regGainFineBase = 0x40,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 44200,
		.idxBase = 279,
		.regGain = 0x2F,
		.regGainFineBase = 0x40,
		.regGainFineStep = 1,
	},
	{
		.gainMax = 88400,
		.idxBase = 343,
		.regGain = 0x3f,
		.regGainFineBase = 0x40,
		.regGainFineStep = 1,
	},
};

static CVI_U32 Again_table[] = {
	1024, 1040, 1056, 1072, 1089, 1104, 1120, 1136, 1152, 1168,
	1184, 1200, 1217, 1232, 1248, 1264, 1280, 1296, 1312, 1328,
	1345, 1360, 1376, 1392, 1408, 1424, 1440, 1456, 1473, 1488,
	1504, 1520, 1536, 1552, 1568, 1584, 1601, 1616, 1632, 1648,
	1664, 1680, 1696, 1712, 1729, 1744, 1760, 1776, 1792, 1808,
	1824, 1840, 1857, 1872, 1888, 1904, 1920, 1936, 1952, 1968,
	1985, 2000, 2016, 2032, 2048, 2080, 2113, 2144, 2176, 2208,
	2241, 2272, 2304, 2336, 2369, 2400, 2432, 2464, 2497, 2528,
	2560, 2592, 2625, 2656, 2688, 2720, 2753, 2784, 2816, 2848,
	2881, 2912, 2944, 2976, 3009, 3040, 3072, 3104, 3137, 3168,
	3200, 3232, 3265, 3296, 3328, 3360, 3393, 3424, 3456, 3488,
	3521, 3552, 3584, 3616, 3649, 3680, 3712, 3744, 3777, 3808,
	3840, 3872, 3905, 3936, 3968, 4000, 4033, 4064, 4096, 4161,
	4224, 4289, 4352, 4417, 4480, 4545, 4608, 4673, 4736, 4801,
	4864, 4929, 4992, 5057, 5120, 5185, 5248, 5313, 5376, 5441,
	5504, 5569, 5656, 5743, 5830, 5917, 6004, 6091, 6178, 6265,
	6352, 6439, 6526, 6613, 6700, 6787, 6874, 6961, 7047, 7134,
	7221, 7308, 7395, 7482, 7569, 7656, 7743, 7831, 7918, 8005,
	8092, 8179, 8266, 8353, 8440, 8527, 8614, 8701, 8788, 8875,
	8962, 9049, 9136, 9223, 9310, 9397, 9484, 9571, 9658, 9745,
	9831, 9918, 10006, 10093, 10180, 10267, 10354, 10441, 10528, 10615,
	10702, 10789, 10876, 10963, 11050, 11137, 11311, 11485, 11659, 11833,
	12007, 12182, 12356, 12530, 12703, 12877, 13051, 13225, 13399, 13573,
	13747, 13921, 14095, 14269, 14444, 14618, 14792, 14966, 15140, 15314,
	15487, 15661, 15835, 16009, 16183, 16357, 16531, 16706, 16880, 17054,
	17228, 17402, 17576, 17750, 17924, 18098, 18271, 18445, 18619, 18793,
	18968, 19142, 19316, 19490, 19664, 19838, 20012, 20186, 20360, 20534,
	20708, 20882, 21055, 21230, 21404, 21578, 21752, 21926, 22100, 22274,
	22622, 22970, 23319, 23667, 24014, 24362, 24710, 25058, 25406, 25755,
	26103, 26451, 26798, 27146, 27494, 27843, 28191, 28539, 28887, 29235,
	29582, 29930, 30279, 30627, 30975, 31323, 31671, 32019, 32367, 32715,
	33063, 33411, 33759, 34107, 34456, 34804, 35151, 35499, 35847, 36195,
	36543, 36892, 37240, 37588, 37935, 38283, 38631, 38980, 39328, 39676,
	40024, 40372, 40719, 41068, 41416, 41764, 42112, 42460, 42808, 43156,
	43504, 43852, 44200, 44548, 45244, 45941, 46636, 47332, 48029, 48725,
	49420, 50117, 50813, 51509, 52205, 52901, 53597, 54294, 54989, 55685,
	56381, 57078, 57773, 58469, 59166, 59862, 60557, 61254, 61950, 62646,
	63342, 64038, 64734, 65431, 66126, 66822, 67518, 68215, 68910, 69606,
	70303, 70999, 71694, 72391, 73087, 73783, 74479, 75175, 75871, 76568,
	77263, 77959, 78655, 79352, 80047, 80743, 81440, 82136, 82831, 83528,
	84224, 84920, 85616, 86312, 87008, 87705, 88400
};


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
		.gainMax = 8129,
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
	{
		.gainMax = 32512,
		.idxBase = 256,
		.regGain = 0x0f,
		.regGainFineBase = 0x80,
		.regGainFineStep = 2,
	},
};

static CVI_U32 Dgain_table[] = {
	1024, 1040, 1056, 1072, 1089, 1104, 1120, 1136, 1152, 1168,
	1184, 1200, 1217, 1232, 1248, 1264, 1280, 1296, 1312, 1328,
	1345, 1360, 1376, 1392, 1408, 1424, 1440, 1456, 1473, 1488,
	1504, 1520, 1536, 1552, 1568, 1584, 1601, 1616, 1632, 1648,
	1664, 1680, 1696, 1712, 1729, 1744, 1760, 1776, 1792, 1808,
	1824, 1840, 1857, 1872, 1888, 1904, 1920, 1936, 1952, 1968,
	1985, 2000, 2016, 2032, 2048, 2080, 2113, 2144, 2176, 2208,
	2241, 2272, 2304, 2336, 2369, 2400, 2432, 2464, 2497, 2528,
	2560, 2592, 2625, 2656, 2688, 2720, 2753, 2784, 2816, 2848,
	2881, 2912, 2944, 2976, 3009, 3040, 3072, 3104, 3137, 3168,
	3200, 3232, 3265, 3296, 3328, 3360, 3393, 3424, 3456, 3488,
	3521, 3552, 3584, 3616, 3649, 3680, 3712, 3744, 3777, 3808,
	3840, 3872, 3905, 3936, 3968, 4000, 4033, 4064, 4096, 4161,
	4224, 4289, 4352, 4417, 4480, 4545, 4608, 4673, 4736, 4801,
	4864, 4929, 4992, 5057, 5120, 5185, 5248, 5313, 5376, 5441,
	5504, 5569, 5632, 5697, 5760, 5825, 5888, 5953, 6016, 6081,
	6144, 6209, 6272, 6337, 6400, 6465, 6528, 6593, 6656, 6721,
	6784, 6849, 6912, 6977, 7040, 7105, 7168, 7233, 7296, 7361,
	7424, 7489, 7552, 7617, 7680, 7745, 7808, 7873, 7936, 8001,
	8064, 8129, 8192, 8320, 8448, 8576, 8704, 8832, 8960, 9088,
	9216, 9344, 9472, 9600, 9728, 9856, 9984, 10112, 10240, 10368,
	10496, 10624, 10752, 10880, 11008, 11136, 11264, 11392, 11520, 11648,
	11776, 11904, 12032, 12160, 12288, 12416, 12544, 12672, 12800, 12928,
	13056, 13184, 13312, 13440, 13568, 13696, 13824, 13952, 14080, 14208,
	14336, 14464, 14592, 14720, 14848, 14976, 15104, 15232, 15360, 15488,
	15616, 15744, 15872, 16000, 16128, 16256, 16384, 16640, 16896, 17152,
	17408, 17664, 17920, 18176, 18432, 18688, 18944, 19200, 19456, 19712,
	19968, 20224, 20480, 20736, 20992, 21248, 21504, 21760, 22016, 22272,
	22528, 22784, 23040, 23296, 23552, 23808, 24064, 24320, 24576, 24832,
	25088, 25344, 25600, 25856, 26112, 26368, 26624, 26880, 27136, 27392,
	27648, 27904, 28160, 28416, 28672, 28928, 29184, 29440, 29696, 29952,
	30208, 30464, 30720, 30976, 31232, 31488, 31744, 32000, 32256, 32512
};

static CVI_U32 Again_tableSize = sizeof(Again_table) / sizeof(CVI_U32);
static CVI_U32 Dgain_tableSize = sizeof(Dgain_table) / sizeof(CVI_U32);

static CVI_S32 cmos_again_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32AgainLin, CVI_U32 *pu32AgainDb)
{
	CVI_U32 i;
	(void)ViPipe;

	CMOS_CHECK_POINTER(pu32AgainLin);
	CMOS_CHECK_POINTER(pu32AgainDb);

	if (*pu32AgainLin >= Again_table[Again_tableSize-1]) {
		*pu32AgainLin = Again_table[Again_tableSize-1];
		*pu32AgainDb = Again_tableSize-1;
		return CVI_SUCCESS;
	}

	for (i = 1; i < Again_tableSize; i++) {
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
	(void)ViPipe;
	CMOS_CHECK_POINTER(pu32DgainLin);
	CMOS_CHECK_POINTER(pu32DgainDb);

	if (*pu32DgainLin >= Dgain_table[Dgain_tableSize - 1]) {
		*pu32DgainLin = Dgain_table[Dgain_tableSize - 1];
		*pu32DgainDb = Dgain_tableSize - 1;
		return CVI_SUCCESS;
	}

	for (CVI_U32 i = 1; i < Dgain_tableSize; i++) {
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

	SC1330_SENSOR_GET_CTX(ViPipe, pstSnsState);
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
	SC1330_SENSOR_GET_CTX(ViPipe, pstSnsState);
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
	const SC1330_MODE_S *pstMode = CVI_NULL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC1330_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstMode = &g_astSC1330_mode[pstSnsState->u8ImgMode];

	if (pstSnsState->enWDRMode != WDR_MODE_NONE) {
		pstIspCfg->frm_num = 2;
		memcpy(&pstIspCfg->img_size[0], &pstMode->astImg[0], sizeof(ISP_WDR_SIZE_S));
		memcpy(&pstIspCfg->img_size[1], &pstMode->astImg[1], sizeof(ISP_WDR_SIZE_S));
	} else {
		pstIspCfg->frm_num = 1;
		memcpy(&pstIspCfg->img_size[0], &pstMode->astImg, sizeof(ISP_WDR_SIZE_S));
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_wdr_mode(VI_PIPE ViPipe, CVI_U8 u8Mode)
{
	CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport sensor[%d] mode: %d!\n", ViPipe, u8Mode);
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
	SC1330_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstSnsRegsInfo = &pstSnsSyncInfo->snsCfg;
	pstCfg0 = &pstSnsState->astSyncInfo[0];
	pstCfg1 = &pstSnsState->astSyncInfo[1];
	pstI2c_data = pstCfg0->snsCfg.astI2cData;

	if ((pstSnsState->bSyncInit == CVI_FALSE) || (pstSnsRegsInfo->bConfig == CVI_FALSE)) {
		pstCfg0->snsCfg.enSnsType = SNS_I2C_TYPE;
		pstCfg0->snsCfg.unComBus.s8I2cDev = g_aunSC1330_BusInfo[ViPipe].s8I2cDev;
		pstCfg0->snsCfg.u8Cfg2ValidDelayMax = 0;
		pstCfg0->snsCfg.use_snsr_sram = CVI_TRUE;
		pstCfg0->snsCfg.u32RegNum = LINEAR_REGS_NUM;

		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			pstI2c_data[i].bUpdate = CVI_TRUE;
			pstI2c_data[i].u8DevAddr = g_aunSC1330_AddrInfo[ViPipe].s8I2cAddr;
			pstI2c_data[i].u32AddrByteNum = sc1330_addr_byte;
			pstI2c_data[i].u32DataByteNum = sc1330_data_byte;
		}
		switch (pstSnsState->enWDRMode) {
		case WDR_MODE_NONE:
			//Linear Mode Regs
			pstI2c_data[LINEAR_HOLD_START].u32RegAddr     = SC1330_GROUP_HOLD_ADDR;
			pstI2c_data[LINEAR_HOLD_START].u32Data = 0x00;
			pstI2c_data[LINEAR_SHS1_0_ADDR].u32RegAddr     = SC1330_SHS1_0_ADDR;
			pstI2c_data[LINEAR_SHS1_1_ADDR].u32RegAddr     = SC1330_SHS1_1_ADDR;
			pstI2c_data[LINEAR_SHS1_2_ADDR].u32RegAddr     = SC1330_SHS1_2_ADDR;

			pstI2c_data[LINEAR_AGAIN_ADDR].u32RegAddr      = SC1330_AGAIN_ADDR;
			pstI2c_data[LINEAR_A_FINEGAIN_ADDR].u32RegAddr = SC1330_A_FINEGAIN_ADDR;
			pstI2c_data[LINEAR_DGAIN_ADDR].u32RegAddr      = SC1330_DGAIN_ADDR;
			pstI2c_data[LINEAR_D_FINEGAIN_ADDR].u32RegAddr = SC1330_D_FINEGAIN_ADDR;

			pstI2c_data[LINEAR_VMAX_0_ADDR].u32RegAddr     = SC1330_VMAX_ADDR;
			pstI2c_data[LINEAR_VMAX_1_ADDR].u32RegAddr     = SC1330_VMAX_ADDR + 1;
			pstI2c_data[LINEAR_HOLD_END].u32RegAddr     = SC1330_GROUP_HOLD_ADDR;
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
	SC1330_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8SensorImageMode = pstSnsState->u8ImgMode;
	pstSnsState->bSyncInit = CVI_FALSE;

	if (pstSensorImageMode->f32Fps <= 30) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (SC1330_RES_IS_960P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height)) {
				if (pstSensorImageMode->u8LaneNum == 2) {
					if (pstSensorImageMode->u8EnableMaster == 1) {
						u8SensorImageMode = SC1330_MODE_960P30_MASTER;
					} else if (pstSensorImageMode->u8EnableMaster == 0) {
						u8SensorImageMode = SC1330_MODE_960P30_SLAVE;
					} else {
						u8SensorImageMode = SC1330_MODE_960P30;
					}
				} else if (pstSensorImageMode->u8LaneNum == 1) {
					if (pstSensorImageMode->u8EnableMaster == 1) {
						u8SensorImageMode = SC1330_MODE_960P30_1L_MASTER;
					} else if (pstSensorImageMode->u8EnableMaster == 0) {
						u8SensorImageMode = SC1330_MODE_960P30_1L_SLAVE;
					} else {
						u8SensorImageMode = SC1330_MODE_960P30_1L;
					}
				}
			} else {
				goto ERROR;
			}
		} else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
			goto ERROR;
		}
	} else if (pstSensorImageMode->f32Fps <= 60) {
			goto ERROR;
	}

	if ((pstSnsState->bInit == CVI_TRUE) && (u8SensorImageMode == pstSnsState->u8ImgMode)) {
		/* Don't need to switch SensorImageMode */
		return CVI_FAILURE;
	}

	pstSnsState->u8ImgMode = u8SensorImageMode;

	return CVI_SUCCESS;

ERROR:
	CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
			pstSensorImageMode->u16Width,
			pstSensorImageMode->u16Height,
			pstSensorImageMode->f32Fps,
			pstSnsState->enWDRMode);
	return CVI_FAILURE;
}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	const SC1330_MODE_S *pstMode = CVI_NULL;

	SC1330_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsState->bInit = CVI_FALSE;
	pstSnsState->bSyncInit = CVI_FALSE;
	pstSnsState->u8ImgMode = SC1330_MODE_960P30;
	pstSnsState->enWDRMode = WDR_MODE_NONE;
	pstMode = &g_astSC1330_mode[pstSnsState->u8ImgMode];
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

	SC1330_SENSOR_GET_CTX(ViPipe, pstSnsState);
	SC1330_SENSOR_GET_COMBO(ViPipe, pstRxAttrSrc);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pstRxAttr);

	CMOS_CHECK_POINTER(pstRxAttrSrc);

	memcpy(pstRxAttr, pstRxAttrSrc, sizeof(*pstRxAttr));

	pstRxAttr->img_size.start_x = g_astSC1330_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.s32X;
	pstRxAttr->img_size.start_y = g_astSC1330_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.s32Y;
	pstRxAttr->img_size.active_w = g_astSC1330_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.u32Width;
	pstRxAttr->img_size.active_h = g_astSC1330_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.u32Height;
	pstRxAttr->img_size.width = g_astSC1330_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Width;
	pstRxAttr->img_size.height = g_astSC1330_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Height;
	pstRxAttr->img_size.max_width = g_astSC1330_mode[pstSnsState->u8ImgMode].astImg[0].stMaxSize.u32Width;
	pstRxAttr->img_size.max_height = g_astSC1330_mode[pstSnsState->u8ImgMode].astImg[0].stMaxSize.u32Height;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;
	}
	if (g_pastSC1330[ViPipe]->u8ImgMode == SC1330_MODE_960P30 ||
		g_pastSC1330[ViPipe]->u8ImgMode == SC1330_MODE_960P30_MASTER ||
		g_pastSC1330[ViPipe]->u8ImgMode == SC1330_MODE_960P30_SLAVE) {
			pstRxAttr->mipi_attr.raw_data_type = RAW_DATA_10BIT;
	} else if (g_pastSC1330[ViPipe]->u8ImgMode == SC1330_MODE_960P30_1L ||
		g_pastSC1330[ViPipe]->u8ImgMode == SC1330_MODE_960P30_1L_MASTER ||
		g_pastSC1330[ViPipe]->u8ImgMode == SC1330_MODE_960P30_1L_SLAVE) {
		pstRxAttr->mipi_attr.raw_data_type = RAW_DATA_12BIT;
	}
	pstRxAttrSrc = CVI_NULL;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_patch_rx_attr(VI_PIPE ViPipe, RX_INIT_ATTR_S *pstRxInitAttr)
{
	int i;
	SNS_COMBO_DEV_ATTR_S* pstRxAttr = CVI_NULL;

	if (!g_pastSC1330ComboDevArray[ViPipe]) {
		pstRxAttr = malloc(sizeof(SNS_COMBO_DEV_ATTR_S));
	} else {
		SC1330_SENSOR_GET_COMBO(ViPipe, pstRxAttr);
	}
	memcpy(pstRxAttr, &sc1330_rx_attr, sizeof(SNS_COMBO_DEV_ATTR_S));
	SC1330_SENSOR_SET_COMBO(ViPipe, pstRxAttr);

	CMOS_CHECK_POINTER(pstRxInitAttr);

	if (pstRxInitAttr->stMclkAttr.bMclkEn)
		pstRxAttr->mclk.cam = pstRxInitAttr->stMclkAttr.u8Mclk;

	if (pstRxInitAttr->hsettlen)
		pstRxAttr->mipi_attr.dphy.hs_settle = pstRxInitAttr->hsettle;

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

void sc1330_exit(VI_PIPE ViPipe)
{
	if (g_pastSC1330ComboDevArray[ViPipe]) {
		free(g_pastSC1330ComboDevArray[ViPipe]);
		g_pastSC1330ComboDevArray[ViPipe] = CVI_NULL;
	}
	sc1330_i2c_exit(ViPipe);
}

static CVI_S32 cmos_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
	CMOS_CHECK_POINTER(pstSensorExpFunc);

	memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

	pstSensorExpFunc->pfn_cmos_sensor_init          = sc1330_init;
	pstSensorExpFunc->pfn_cmos_sensor_exit          = sc1330_exit;
	pstSensorExpFunc->pfn_cmos_sensor_global_init   = sensor_global_init;
	pstSensorExpFunc->pfn_cmos_set_image_mode       = cmos_set_image_mode;
	pstSensorExpFunc->pfn_cmos_set_wdr_mode         = cmos_set_wdr_mode;

	pstSensorExpFunc->pfn_cmos_get_isp_default      = cmos_get_isp_default;
	pstSensorExpFunc->pfn_cmos_get_isp_black_level  = cmos_get_blc_default;
	pstSensorExpFunc->pfn_cmos_get_sns_reg_info     = cmos_get_sns_regs_info;

	return CVI_SUCCESS;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
static CVI_VOID sensor_patch_i2c_addr(VI_PIPE ViPipe, CVI_S32 s32I2cAddr)
{
	if (SC1330_I2C_ADDR_IS_VALID(s32I2cAddr))
		g_aunSC1330_AddrInfo[ViPipe].s8I2cAddr = s32I2cAddr;
	else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C addr input error ,please check [0x%x]\n", s32I2cAddr);
		g_aunSC1330_AddrInfo[ViPipe].s8I2cAddr = SC1330_I2C_ADDR_2;
	}
}

static CVI_S32 sc1336_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
	g_aunSC1330_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	SC1330_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

	if (pastSnsStateCtx == CVI_NULL) {
		pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
		if (pastSnsStateCtx == CVI_NULL) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
			return -ENOMEM;
		}
	}

	memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

	SC1330_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

	return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	SC1330_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
	SENSOR_FREE(pastSnsStateCtx);
	SC1330_SENSOR_RESET_CTX(ViPipe);
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

	stSnsAttrInfo.eSensorId = SC1330_ID;

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

	s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, SC1330_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, SC1330_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, SC1330_ID);
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
	g_au16SC1330_GainMode[ViPipe] = pstInitAttr->enGainMode;
	g_au16SC1330_L2SMode[ViPipe] = pstInitAttr->enL2SMode;

	return CVI_SUCCESS;
}

ISP_SNS_OBJ_S stSnsSC1330_Obj = {
	.pfnRegisterCallback    = sensor_register_callback,
	.pfnUnRegisterCallback  = sensor_unregister_callback,
	.pfnStandby             = sc1330_standby,
	.pfnRestart             = sc1330_restart,
	.pfnMirrorFlip          = sc1330_mirror_flip,
	.pfnWriteReg            = sc1330_write_register,
	.pfnReadReg             = sc1330_read_register,
	.pfnSetBusInfo          = sc1336_set_bus_info,
	.pfnSetInit             = sensor_set_init,
	.pfnPatchRxAttr		= sensor_patch_rx_attr,
	.pfnPatchI2cAddr	= sensor_patch_i2c_addr,
	.pfnGetRxAttr		= sensor_rx_attr,
	.pfnExpSensorCb		= cmos_init_sensor_exp_function,
	.pfnExpAeCb		= cmos_init_ae_exp_function,
	.pfnSnsProbe		= sc1330_probe,
};


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
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

#include "gc8613_cmos_ex.h"
#include "gc8613_cmos_param.h"

#define DIV_0_TO_1(a)   ((0 == (a)) ? 1 : (a))
#define DIV_0_TO_1_FLOAT(a) ((((a) < 1E-10) && ((a) > -1E-10)) ? 1 : (a))

#define GC8613_I2C_ADDR_IS_VALID(addr)      ((addr) == GC8613_I2C_ADDR_1 || (addr) == GC8613_I2C_ADDR_2)
/****************************************************************************
 * global variables                                                            *
 ****************************************************************************/

ISP_SNS_STATE_S *g_pastGc8613[VI_MAX_PIPE_NUM] = {CVI_NULL};
SNS_COMBO_DEV_ATTR_S* g_pastGc8613ComboDevArray[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define GC8613_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastGc8613[dev])
#define GC8613_SENSOR_SET_CTX(dev, pstCtx)   (g_pastGc8613[dev] = pstCtx)
#define GC8613_SENSOR_RESET_CTX(dev)         (g_pastGc8613[dev] = CVI_NULL)
#define GC8613_SENSOR_SET_COMBO(dev, pstCtx)   (g_pastGc8613ComboDevArray[dev] = pstCtx)
#define GC8613_SENSOR_GET_COMBO(dev, pstCtx)   (pstCtx = g_pastGc8613ComboDevArray[dev])

#define GC8613_SNNSOR_IS_1L() (gc8613_rx_attr.mipi_attr.lane_id[2] == -1 && gc8613_rx_attr.mipi_attr.lane_id[4] == -1)
#define GC8613_SNNSOR_IS_2L() (gc8613_rx_attr.mipi_attr.lane_id[3] == -1 && gc8613_rx_attr.mipi_attr.lane_id[4] == -1)
#define GC8613_SNNSOR_IS_4L() (gc8613_rx_attr.mipi_attr.lane_id[3] != -1 && gc8613_rx_attr.mipi_attr.lane_id[4] != -1)

ISP_SNS_COMMBUS_U g_aunGc8613_BusInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cDev = 0},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

ISP_SNS_COMMADDR_U g_aunGc8613_AddrInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cAddr = 0},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cAddr = -1}
};

CVI_U16 g_au16Gc8613_GainMode[VI_MAX_PIPE_NUM] = {0};
ISP_SNS_MIRRORFLIP_TYPE_E g_aeGc8613_MirrorFip[VI_MAX_PIPE_NUM] = {0};

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
/*****Gc8613 Lines Range*****/
#define GC8613_FULL_LINES_MAX  (0xFFFFF)
#define GC8613_VMAX_8M30_LINEAR	0x08ca

/*****Gc8613 Register Address*****/
#define GC8613_HOLD_ADDR		0x031d

#define GC8613_SHR0_L_ADDR		0x0203 //shutter time
#define GC8613_SHR0_H_ADDR		0x0202 //bit[13:8]
#define GC8613_SHR1_L_ADDR     	0x0201 // se shutter time
#define GC8613_SHR1_H_ADDR     	0x0200

#define GC8613_PRE_GAIN_L_ADDR  0x0065 //auto_pregain [5:0]	(dgain)
#define GC8613_PRE_GAIN_H_ADDR	0x0064 //auto_pregain [9:6]
#define GC8613_PRE_GAIN1_L_ADDR 0x007c //auto_pregain [5:0]	(dgain)
#define GC8613_PRE_GAIN1_H_ADDR	0x007b //auto_pregain [9:6]
#define GC8613_GAIN_L_ADDR		0x0614 //gain				(again)
#define GC8613_GAIN_H_ADDR		0x0615 //bit[11:8]
#define GC8613_GAIN_F_ADDR		0x0225 //bit[12]
#define GC8613_COL_GAIN_L_ADDR  0x00b9 //Col_gain [11:6]
#define GC8613_COL_GAIN_H_ADDR	0x00b8 //[5:0]

#define GC8613_GAIN_MAG1_ADDR	0x1467
#define GC8613_GAIN_MAG2_ADDR	0x1468
#define GC8613_GAIN_MAG3_ADDR	0x1447

#define GC8613_VMAX_L_ADDR		0x0341 //vmax
#define GC8613_VMAX_H_ADDR		0x0340 //bit[15:8]

#define GC8613_WINDOW_HEIGHT	0x0888 // 34a 34b

/*****Gc8613 Size*****/
#define SENSOR_GC8613_8M_WIDTH	3840
#define SENSOR_GC8613_8M_HEIGHT	2160
#define GC8613_RES_IS_8M(w, h)      ((w) == SENSOR_GC8613_8M_WIDTH && (h) == SENSOR_GC8613_8M_HEIGHT)
#define SENSOR_GC8613_2M_WIDTH	1920
#define SENSOR_GC8613_2M_HEIGHT	1080
#define GC8613_RES_IS_2M(w, h)      ((w) == SENSOR_GC8613_2M_WIDTH && (h) == SENSOR_GC8613_2M_HEIGHT)

static CVI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	GC8613_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FlickerFreq = 50 * 256;
	pstAeSnsDft->u32FullLinesMax = GC8613_FULL_LINES_MAX;
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * g_astGc8613_mode[pstSnsState->u8ImgMode].f32MaxFps);

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
		pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * g_astGc8613_mode[pstSnsState->u8ImgMode].f32MaxFps / 2;
	else
		pstAeSnsDft->u32LinesPer500ms = g_au32LinesPer500ms[ViPipe];
	pstAeSnsDft->u32SnsStableFrame = 8;

	switch (pstSnsState->enWDRMode) {
	default:
	case WDR_MODE_NONE:   /*linear mode*/
		pstAeSnsDft->f32Fps = g_astGc8613_mode[pstSnsState->u8ImgMode].f32MaxFps;
		pstAeSnsDft->f32MinFps = g_astGc8613_mode[pstSnsState->u8ImgMode].f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xd;
		pstAeSnsDft->au8HistThresh[1] = 0x28;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxAgain = g_astGc8613_mode[pstSnsState->u8ImgMode].stAgain[0].u32Max;
		pstAeSnsDft->u32MinAgain = g_astGc8613_mode[pstSnsState->u8ImgMode].stAgain[0].u32Min;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = g_astGc8613_mode[pstSnsState->u8ImgMode].stDgain[0].u32Max;
		pstAeSnsDft->u32MinDgain = g_astGc8613_mode[pstSnsState->u8ImgMode].stDgain[0].u32Min;
		pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
		pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

		pstAeSnsDft->u8AeCompensation = 40;
		pstAeSnsDft->u32InitAESpeed = 64;
		pstAeSnsDft->u32InitAETolerance = 5;
		pstAeSnsDft->u32AEResponseFrame = 4;
		pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe] ? g_au32InitExposure[ViPipe] : 76151;

		//shutter time [9 to (number of lines perframe -1)]
		pstAeSnsDft->u32MaxIntTime = g_astGc8613_mode[pstSnsState->u8ImgMode].stExp[0].u16Max;
		pstAeSnsDft->u32MinIntTime = g_astGc8613_mode[pstSnsState->u8ImgMode].stExp[0].u16Min;
		pstAeSnsDft->u32MaxIntTimeTarget = 65535;
		pstAeSnsDft->u32MinIntTimeTarget = 1;
		break;

	case WDR_MODE_2To1_LINE:
		pstAeSnsDft->f32Fps = g_astGc8613_mode[pstSnsState->u8ImgMode].f32MaxFps;
		pstAeSnsDft->f32MinFps = g_astGc8613_mode[pstSnsState->u8ImgMode].f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xC;
		pstAeSnsDft->au8HistThresh[1] = 0x18;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 1;
		pstAeSnsDft->u32MinIntTime = 1;

		pstAeSnsDft->u32MaxIntTimeTarget = 65535;
		pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;

		pstAeSnsDft->u32MaxAgain = g_astGc8613_mode[pstSnsState->u8ImgMode].stAgain[0].u32Max;
		pstAeSnsDft->u32MinAgain = g_astGc8613_mode[pstSnsState->u8ImgMode].stAgain[0].u32Min;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = g_astGc8613_mode[pstSnsState->u8ImgMode].stDgain[0].u32Max;
		pstAeSnsDft->u32MinDgain = g_astGc8613_mode[pstSnsState->u8ImgMode].stDgain[0].u32Min;
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
		}
		break;
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_fps_set(VI_PIPE ViPipe, CVI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	CVI_U32 u32VMAX = GC8613_VMAX_8M30_LINEAR;
	CVI_FLOAT f32MaxFps = 0;
	CVI_FLOAT f32MinFps = 0;
	CVI_U32 u32Vts = 0;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	GC8613_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32Vts = g_astGc8613_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	f32MaxFps = g_astGc8613_mode[pstSnsState->u8ImgMode].f32MaxFps;
	f32MinFps = g_astGc8613_mode[pstSnsState->u8ImgMode].f32MinFps;

	if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
		u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport Fps: %f\n", f32Fps);
		return CVI_FAILURE;
	}

	u32VMAX = (u32VMAX > GC8613_FULL_LINES_MAX) ? GC8613_FULL_LINES_MAX : u32VMAX;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_L].u32Data = (u32VMAX & 0xFF);
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_H].u32Data = ((u32VMAX & 0xFF00) >> 8);
	} else {
		pstSnsRegsInfo->astI2cData[WDR_VMAX_L].u32Data = (u32VMAX & 0xFF);
		pstSnsRegsInfo->astI2cData[WDR_VMAX_H].u32Data = ((u32VMAX & 0xFF00) >> 8);
	}

	pstSnsState->u32FLStd = u32VMAX;

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

	GC8613_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(u32IntTime);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) {
		/*
			short_intt < vb,
			short_intt + long_intt < vts (vts register is 0x0340 0x0341)
   			vb = vts - window_height - 16 - 20
				2314 - 2184 - 36 = 94
		*/

		CVI_U32 u32ShortIntTime = u32IntTime[0];
		CVI_U32 u32LongIntTime = u32IntTime[1];
		CVI_U16 u16Sexp, u16Lexp;
		CVI_U32 u32MaxRangeLExp, u32MaxRangeSExp;

		u32MaxRangeSExp = pstSnsState->au32FL[0] - GC8613_WINDOW_HEIGHT - 16 - 20 - 1;
		pstSnsState->au32WDRIntTime[0] = (u32ShortIntTime <= u32MaxRangeSExp) ?
			u32ShortIntTime : u32MaxRangeSExp;
		if (!pstSnsState->au32WDRIntTime[0])
			pstSnsState->au32WDRIntTime[0] = 1;
		u16Sexp = (CVI_U16)pstSnsState->au32WDRIntTime[0];

		u32MaxRangeLExp = pstSnsState->au32FL[0] - u16Sexp - 1;
		pstSnsState->au32WDRIntTime[1] = (u32LongIntTime <= u32MaxRangeLExp) ?
			u32LongIntTime : u32MaxRangeLExp;
		if (!pstSnsState->au32WDRIntTime[1])
			pstSnsState->au32WDRIntTime[1] = 1;
		u16Lexp = (CVI_U16)pstSnsState->au32WDRIntTime[1];

		pstSnsRegsInfo->astI2cData[WDR_SHR1_H].u32Data = ((u16Sexp & 0x3F00) >> 8); // 0x0200
		pstSnsRegsInfo->astI2cData[WDR_SHR1_L].u32Data = (u16Sexp & 0xFF);			// 0x0201
		pstSnsRegsInfo->astI2cData[WDR_SHR0_H].u32Data = ((u16Lexp & 0x3F00) >> 8); // 0x0202
		pstSnsRegsInfo->astI2cData[WDR_SHR0_L].u32Data = (u16Lexp & 0xFF);			// 0x0203

	} else {
		pstSnsRegsInfo->astI2cData[LINEAR_SHR_H].u32Data = ((u32IntTime[0] & 0x3F00) >> 8);
		pstSnsRegsInfo->astI2cData[LINEAR_SHR_L].u32Data = (u32IntTime[0] & 0xFF);
	}

	return CVI_SUCCESS;

}

static CVI_U32 regValTable[26][8] = {
 //  0614  0615	 0225  1467	 1468  00b8  00b9  1447
	{0x00, 0x00, 0x00, 0x07, 0x07, 0x01, 0x00, 0x77},
	{0x90, 0x02, 0x00, 0x07, 0x07, 0x01, 0x09, 0x77},
	{0x01, 0x00, 0x00, 0x08, 0x08, 0x01, 0x19, 0x77},
	{0x91, 0x02, 0x00, 0x08, 0x08, 0x01, 0x2A, 0x77},
	{0x02, 0x00, 0x00, 0x09, 0x09, 0x01, 0x3D, 0x77},
	{0x00, 0x00, 0x00, 0x07, 0x07, 0x02, 0x10, 0x75},
	{0x90, 0x02, 0x00, 0x07, 0x07, 0x02, 0x29, 0x75},
	{0x01, 0x00, 0x00, 0x08, 0x08, 0x03, 0x0B, 0x75},
	{0x91, 0x02, 0x00, 0x08, 0x08, 0x03, 0x2F, 0x75},
	{0x02, 0x00, 0x00, 0x08, 0x08, 0x04, 0x1C, 0x75},
	{0x92, 0x02, 0x00, 0x09, 0x09, 0x05, 0x11, 0x75},
	{0x03, 0x00, 0x00, 0x0a, 0x0a, 0x06, 0x20, 0x75},
	{0x93, 0x02, 0x00, 0x0b, 0x0b, 0x07, 0x25, 0x75},
	{0x00, 0x00, 0x01, 0x0c, 0x0c, 0x08, 0x1E, 0x75},
	{0x90, 0x02, 0x01, 0x0d, 0x0d, 0x09, 0x3B, 0x75},
	{0x01, 0x00, 0x01, 0x0d, 0x0d, 0x0B, 0x3B, 0x75},
	{0x91, 0x02, 0x01, 0x0e, 0x0e, 0x0E, 0x03, 0x75},
	{0x02, 0x00, 0x01, 0x0f, 0x0f, 0x10, 0x25, 0x75},
	{0x92, 0x02, 0x01, 0x10, 0x10, 0x13, 0x35, 0x75},
	{0x03, 0x00, 0x01, 0x11, 0x11, 0x17, 0x30, 0x75},
	{0x93, 0x02, 0x01, 0x13, 0x13, 0x1C, 0x06, 0x75},
	{0x04, 0x00, 0x01, 0x14, 0x14, 0x21, 0x07, 0x75},
	{0x94, 0x02, 0x01, 0x15, 0x15, 0x27, 0x38, 0x75},
	{0x05, 0x00, 0x01, 0x17, 0x17, 0x2F, 0x18, 0x75},
	{0x95, 0x02, 0x01, 0x19, 0x19, 0x38, 0x09, 0x75},
	{0x06, 0x00, 0x01, 0x1a, 0x1a, 0x41, 0x37, 0x75},

};

static CVI_U32 gain_table[27] = {
	1024, 1168, 1424, 1712, 2016, 2304, 2720, 3264, 3824, 4544, 5408,
	6656, 7776, 8688, 10160, 12208, 14384, 16976, 20320, 24336, 28768,
	33904, 40832, 48528, 57488, 67456, 0xffffffff,
};

static CVI_U32 wdr_regValTable[26][8] = {
  // 614    615	  225  1467	 1468	 b8	  b9   1447
	{0x00, 0x00, 0x00, 0x09, 0x09, 0x01, 0x00, 0x77},
	{0x90, 0x02, 0x00, 0x09, 0x09, 0x01, 0x0E, 0x77},
	{0x01, 0x00, 0x00, 0x0a, 0x0a, 0x01, 0x1A, 0x77},
	{0x91, 0x02, 0x00, 0x0a, 0x0a, 0x01, 0x2D, 0x77},
	{0x02, 0x00, 0x00, 0x0b, 0x0b, 0x02, 0x05, 0x77},
	{0x00, 0x00, 0x00, 0x09, 0x09, 0x02, 0x11, 0x75},
	{0x90, 0x02, 0x00, 0x09, 0x09, 0x02, 0x29, 0x75},
	{0x01, 0x00, 0x00, 0x0a, 0x0a, 0x03, 0x0D, 0x75},
	{0x91, 0x02, 0x00, 0x0a, 0x0a, 0x03, 0x33, 0x75},
	{0x02, 0x00, 0x00, 0x0b, 0x0b, 0x04, 0x1E, 0x75},
	{0x92, 0x02, 0x00, 0x0b, 0x0b, 0x05, 0x19, 0x75},
	{0x03, 0x00, 0x00, 0x0d, 0x0d, 0x06, 0x1D, 0x75},
	{0x93, 0x02, 0x00, 0x0d, 0x0d, 0x07, 0x2A, 0x75},
	{0x00, 0x00, 0x01, 0x10, 0x10, 0x08, 0x23, 0x75},
	{0x90, 0x02, 0x01, 0x10, 0x10, 0x0A, 0x04, 0x75},
	{0x01, 0x00, 0x01, 0x12, 0x12, 0x0C, 0x04, 0x75},
	{0x91, 0x02, 0x01, 0x12, 0x12, 0x0E, 0x03, 0x75},
	{0x02, 0x00, 0x01, 0x14, 0x14, 0x10, 0x3F, 0x75},
	{0x92, 0x02, 0x01, 0x14, 0x14, 0x13, 0x3D, 0x75},
	{0x03, 0x00, 0x01, 0x17, 0x17, 0x18, 0x02, 0x75},
	{0x93, 0x02, 0x01, 0x17, 0x17, 0x1C, 0x2C, 0x75},
	{0x04, 0x00, 0x01, 0x1a, 0x1a, 0x22, 0x06, 0x75},
	{0x94, 0x02, 0x01, 0x1b, 0x1b, 0x28, 0x01, 0x75},
	{0x05, 0x00, 0x01, 0x1e, 0x1e, 0x30, 0x1E, 0x75},
	{0x95, 0x02, 0x01, 0x1e, 0x1e, 0x39, 0x3F, 0x75},
	{0x06, 0x00, 0x01, 0x21, 0x21, 0x41, 0x37, 0x75},
};

static CVI_U32 wdr_gain_table[27] = {
	1024, 1248, 1456, 1744, 2128, 2320, 2720, 3280, 3904, 4592,
	5520, 6624, 7856, 8768, 10304, 12368, 14384, 17392, 20448, 24608,
	29392, 34912, 40992, 49648, 59376, 67456, 0xffffffff,
};

static CVI_S32 cmos_again_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32AgainLin, CVI_U32 *pu32AgainDb)
{
	int i, total;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	GC8613_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pu32AgainLin);
	CMOS_CHECK_POINTER(pu32AgainDb);

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {

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
		*pu32AgainDb = i;
		*pu32AgainLin = gain_table[i];
	} else {
		total = sizeof(wdr_gain_table) / sizeof(CVI_U32);
		if (*pu32AgainLin >= wdr_gain_table[total - 1]) {
			*pu32AgainLin = *pu32AgainDb = wdr_gain_table[total - 1];
			return CVI_SUCCESS;
		}

		for (i = 1; i < total; i++) {
			if (*pu32AgainLin < wdr_gain_table[i])
				break;
		}

		i--;
		*pu32AgainDb = i;
		*pu32AgainLin = wdr_gain_table[i];
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_dgain_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32DgainLin, CVI_U32 *pu32DgainDb)
{
	CVI_U32 pregain;

	(void) ViPipe;
	CMOS_CHECK_POINTER(pu32DgainLin);
	CMOS_CHECK_POINTER(pu32DgainDb);

	// find the pregain
	pregain = *pu32DgainLin * 64 / 1024;
	// set the Db as the AE algo gain, we need this to do gain update
	*pu32DgainDb = pregain;
	// set the Lin as the closest sensor gain for AE algo reference
	*pu32DgainLin = pregain * 16;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_gains_update(VI_PIPE ViPipe, CVI_U32 *pu32Again, CVI_U32 *pu32Dgain)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U32 u32Dgain;
	CVI_U8 i = 0;

	GC8613_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pu32Again);
	CMOS_CHECK_POINTER(pu32Dgain);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	/*
		Dgain (auto_pregain) : [9:0]
		Dgain max : 0x3FF = 1023
		Dgain min : 0x40  = 64

	*/
	i = pu32Again[0];
	u32Dgain = pu32Dgain[0];

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		/* linear mode */
		pstSnsRegsInfo->astI2cData[LINEAR_HOLD].u32Data 	  = 0x2d;
		pstSnsRegsInfo->astI2cData[LINEAR_GAIN_L].u32Data 	  = regValTable[i][0]; // 614
		pstSnsRegsInfo->astI2cData[LINEAR_GAIN_H].u32Data 	  = regValTable[i][1]; // 615
		pstSnsRegsInfo->astI2cData[LINEAR_REL].u32Data 		  = 0x28;
		pstSnsRegsInfo->astI2cData[LINEAR_GAIN_F].u32Data 	  = regValTable[i][2]; // 225
		pstSnsRegsInfo->astI2cData[LINEAR_MAG_1].u32Data 	  = regValTable[i][3]; // 1467
		pstSnsRegsInfo->astI2cData[LINEAR_MAG_2].u32Data 	  = regValTable[i][4]; // 1468
		pstSnsRegsInfo->astI2cData[LINEAR_COL_GAIN_H].u32Data = regValTable[i][5]; // b8
		pstSnsRegsInfo->astI2cData[LINEAR_COL_GAIN_L].u32Data = regValTable[i][6]; // b9
		pstSnsRegsInfo->astI2cData[LINEAR_MAG_3].u32Data 	  = regValTable[i][7]; // 1447
		pstSnsRegsInfo->astI2cData[LINEAR_PRE_GAIN_H].u32Data = (u32Dgain >> 6);   // 64
		pstSnsRegsInfo->astI2cData[LINEAR_PRE_GAIN_L].u32Data = (u32Dgain & 0x3F); // 65
	} else {
		/* DOL mode */
		pstSnsRegsInfo->astI2cData[WDR_HOLD].u32Data		= 0x2d;					 // 31d
		pstSnsRegsInfo->astI2cData[WDR_GAIN_L].u32Data		= wdr_regValTable[i][0]; // 614
		pstSnsRegsInfo->astI2cData[WDR_GAIN_H].u32Data		= wdr_regValTable[i][1]; // 615
		pstSnsRegsInfo->astI2cData[WDR_REL].u32Data			= 0x28;					 // 31d
		pstSnsRegsInfo->astI2cData[WDR_GAIN_F].u32Data		= wdr_regValTable[i][2]; // 225
		pstSnsRegsInfo->astI2cData[WDR_MAG_1].u32Data		= wdr_regValTable[i][3]; // 1467
		pstSnsRegsInfo->astI2cData[WDR_MAG_2].u32Data 		= wdr_regValTable[i][4]; // 1468
		pstSnsRegsInfo->astI2cData[WDR_COL_GAIN_H].u32Data	= wdr_regValTable[i][5]; // b8
		pstSnsRegsInfo->astI2cData[WDR_COL_GAIN_L].u32Data	= wdr_regValTable[i][6]; // b9
		pstSnsRegsInfo->astI2cData[WDR_MAG_3].u32Data		= wdr_regValTable[i][7]; // 1447
		pstSnsRegsInfo->astI2cData[WDR_PRE_GAIN_H].u32Data	= (u32Dgain >> 6);		 //64
		pstSnsRegsInfo->astI2cData[WDR_PRE_GAIN_L].u32Data	= (u32Dgain & 0x3F);	 //65
		pstSnsRegsInfo->astI2cData[WDR_PRE_GAIN1_H].u32Data	= (u32Dgain >> 6);		 //7b
		pstSnsRegsInfo->astI2cData[WDR_PRE_GAIN1_L].u32Data	= (u32Dgain & 0x3F);	 //7c
	}
	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_inttime_max(VI_PIPE ViPipe, CVI_U16 u16ManRatioEnable, CVI_U32 *au32Ratio,
		CVI_U32 *au32IntTimeMax, CVI_U32 *au32IntTimeMin, CVI_U32 *pu32LFMaxIntTime)
{
	CVI_U32 u32IntTimeMaxTmp = 0, u32IntTimeMaxTmp0 = 0;
	CVI_U32 u32RatioTmp = 0x40;
	CVI_U32 u32ShortTimeMinLimit = 1;
	CVI_U32 u32ShortTimeMaxLimit = 0;

	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(au32Ratio);
	CMOS_CHECK_POINTER(au32IntTimeMax);
	CMOS_CHECK_POINTER(au32IntTimeMin);
	CMOS_CHECK_POINTER(pu32LFMaxIntTime);
	GC8613_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	/*
		short_intt < vb
		short_intt + long_intt < vts (vts register is 0x0340/0x0341)
   		vb = vts - window_height - 16 - 20 (window_height register is 0x034a/0x034b)
	*/

	/*use old lexp calculate new sexp*/
	u32IntTimeMaxTmp0 = ((pstSnsState->au32FL[1] - pstSnsState->au32WDRIntTime[0]) * 0x40) /
			DIV_0_TO_1(au32Ratio[0]);
	/*expect new max sexp*/
	u32IntTimeMaxTmp  = ((pstSnsState->au32FL[0] - 1) * 0x40) / DIV_0_TO_1(au32Ratio[0] + 0x40);
	u32ShortTimeMaxLimit = pstSnsState->au32FL[0] - GC8613_WINDOW_HEIGHT - 16 - 20 - 1;
	/*real new max sexp*/
	u32IntTimeMaxTmp = (u32IntTimeMaxTmp > u32IntTimeMaxTmp0) ? u32IntTimeMaxTmp0 : u32IntTimeMaxTmp;
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

	memcpy(pstDef->stNoiseCalibration.CalibrationCoef,
		&g_stIspNoiseCalibratio, sizeof(ISP_CMOS_NOISE_CALIBRATION_S));

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
	const GC8613_MODE_S *pstMode = CVI_NULL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	GC8613_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstMode = &g_astGc8613_mode[pstSnsState->u8ImgMode];

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

	GC8613_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstSnsState->bSyncInit = CVI_FALSE;

	switch (u8Mode) {
	case WDR_MODE_NONE:
		if (pstSnsState->u8ImgMode == GC8613_MODE_8M30_WDR)
			pstSnsState->u8ImgMode = GC8613_MODE_8M30;
		else {
		}
		pstSnsState->enWDRMode = WDR_MODE_NONE;
		pstSnsState->u32FLStd = g_astGc8613_mode[pstSnsState->u8ImgMode].u32VtsDef;
		CVI_TRACE_SNS(CVI_DBG_INFO, "WDR_MODE_NONE\n");
		break;

	case WDR_MODE_2To1_LINE:
		if (pstSnsState->u8ImgMode == GC8613_MODE_8M30)
			pstSnsState->u8ImgMode = GC8613_MODE_8M30_WDR;
		else {
		}
		pstSnsState->enWDRMode = WDR_MODE_2To1_LINE;
		pstSnsState->u32FLStd = g_astGc8613_mode[pstSnsState->u8ImgMode].u32VtsDef * 2;
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
	GC8613_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstSnsRegsInfo = &pstSnsSyncInfo->snsCfg;
	pstCfg0 = &pstSnsState->astSyncInfo[0];
	pstCfg1 = &pstSnsState->astSyncInfo[1];
	pstI2c_data = pstCfg0->snsCfg.astI2cData;

	if ((pstSnsState->bSyncInit == CVI_FALSE) || (pstSnsRegsInfo->bConfig == CVI_FALSE)) {
		pstCfg0->snsCfg.enSnsType = SNS_I2C_TYPE;
		pstCfg0->snsCfg.unComBus.s8I2cDev = g_aunGc8613_BusInfo[ViPipe].s8I2cDev;
		pstCfg0->snsCfg.u8Cfg2ValidDelayMax = 2;
		pstCfg0->snsCfg.use_snsr_sram = CVI_TRUE;
		pstCfg0->snsCfg.u32RegNum = (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) ?
					WDR_REGS_NUM : LINEAR_REGS_NUM;

		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			pstI2c_data[i].bUpdate = CVI_TRUE;
			pstI2c_data[i].u8DevAddr = g_aunGc8613_AddrInfo[ViPipe].s8I2cAddr;
			pstI2c_data[i].u32AddrByteNum = gc8613_addr_byte;
			pstI2c_data[i].u32DataByteNum = gc8613_data_byte;
		}

		switch (pstSnsState->enWDRMode) {
		case WDR_MODE_2To1_LINE:
			pstI2c_data[WDR_HOLD].u32RegAddr = GC8613_HOLD_ADDR;
			pstI2c_data[WDR_REL].u32RegAddr  = GC8613_HOLD_ADDR;
			pstI2c_data[WDR_SHR0_L].u32RegAddr = GC8613_SHR0_L_ADDR;
			pstI2c_data[WDR_SHR0_H].u32RegAddr = GC8613_SHR0_H_ADDR;
			pstI2c_data[WDR_SHR1_L].u32RegAddr = GC8613_SHR1_L_ADDR;
			pstI2c_data[WDR_SHR1_H].u32RegAddr = GC8613_SHR1_H_ADDR;
			pstI2c_data[WDR_PRE_GAIN_L].u32RegAddr = GC8613_PRE_GAIN_L_ADDR;
			pstI2c_data[WDR_PRE_GAIN_H].u32RegAddr = GC8613_PRE_GAIN_H_ADDR;
			pstI2c_data[WDR_PRE_GAIN1_L].u32RegAddr = GC8613_PRE_GAIN1_L_ADDR;
			pstI2c_data[WDR_PRE_GAIN1_H].u32RegAddr = GC8613_PRE_GAIN1_H_ADDR;
			pstI2c_data[WDR_GAIN_F].u32RegAddr = GC8613_GAIN_F_ADDR;
			pstI2c_data[WDR_GAIN_L].u32RegAddr = GC8613_GAIN_L_ADDR;
			pstI2c_data[WDR_GAIN_H].u32RegAddr = GC8613_GAIN_H_ADDR;
			pstI2c_data[WDR_COL_GAIN_L].u32RegAddr = GC8613_COL_GAIN_L_ADDR;
			pstI2c_data[WDR_COL_GAIN_H].u32RegAddr = GC8613_COL_GAIN_H_ADDR;
			pstI2c_data[WDR_MAG_1].u32RegAddr = GC8613_GAIN_MAG1_ADDR;
			pstI2c_data[WDR_MAG_2].u32RegAddr = GC8613_GAIN_MAG2_ADDR;
			pstI2c_data[WDR_MAG_3].u32RegAddr = GC8613_GAIN_MAG3_ADDR;
			pstI2c_data[WDR_VMAX_L].u32RegAddr = GC8613_VMAX_L_ADDR;
			pstI2c_data[WDR_VMAX_H].u32RegAddr = GC8613_VMAX_H_ADDR;
			pstCfg0->ispCfg.u8DelayFrmNum = 0;
			break;
		default:
			pstI2c_data[LINEAR_HOLD].u32RegAddr = GC8613_HOLD_ADDR;
			pstI2c_data[LINEAR_REL].u32RegAddr = GC8613_HOLD_ADDR;
			pstI2c_data[LINEAR_SHR_L].u32RegAddr = GC8613_SHR0_L_ADDR;
			pstI2c_data[LINEAR_SHR_H].u32RegAddr = GC8613_SHR0_H_ADDR;
			pstI2c_data[LINEAR_PRE_GAIN_L].u32RegAddr = GC8613_PRE_GAIN_L_ADDR;
			pstI2c_data[LINEAR_PRE_GAIN_H].u32RegAddr = GC8613_PRE_GAIN_H_ADDR;
			pstI2c_data[LINEAR_GAIN_L].u32RegAddr = GC8613_GAIN_L_ADDR;
			pstI2c_data[LINEAR_GAIN_H].u32RegAddr = GC8613_GAIN_H_ADDR;
			pstI2c_data[LINEAR_GAIN_F].u32RegAddr = GC8613_GAIN_F_ADDR;
			pstI2c_data[LINEAR_COL_GAIN_L].u32RegAddr = GC8613_COL_GAIN_L_ADDR;
			pstI2c_data[LINEAR_COL_GAIN_H].u32RegAddr = GC8613_COL_GAIN_H_ADDR;
			pstI2c_data[LINEAR_MAG_1].u32RegAddr = GC8613_GAIN_MAG1_ADDR;
			pstI2c_data[LINEAR_MAG_2].u32RegAddr = GC8613_GAIN_MAG2_ADDR;
			pstI2c_data[LINEAR_MAG_3].u32RegAddr = GC8613_GAIN_MAG3_ADDR;
			pstI2c_data[LINEAR_VMAX_L].u32RegAddr = GC8613_VMAX_L_ADDR;
			pstI2c_data[LINEAR_VMAX_H].u32RegAddr = GC8613_VMAX_H_ADDR;
			pstCfg0->ispCfg.u8DelayFrmNum = 0;
			break;
		}
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
				if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
					if (i <= LINEAR_SHR_L)
						shutterUpdate = 1;
					if ((i >= LINEAR_HOLD) && (i <= LINEAR_PRE_GAIN_L))
						gainsUpdate = 1;
					if ((i >= LINEAR_VMAX_H) && (i <= LINEAR_VMAX_L))
						vtsUpdate = 1;
				} else {
					if (i <= WDR_SHR0_L)
						shutterUpdate = 1;
					if ((i >= WDR_HOLD) && (i <= WDR_PRE_GAIN1_H))
						gainsUpdate = 1;
					if ((i >= WDR_VMAX_H) && (i <= WDR_VMAX_L))
						vtsUpdate = 1;
				}
				pstCfg0->snsCfg.astI2cData[i].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.need_update = CVI_TRUE;
			}
		}


		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (shutterUpdate) {
				pstCfg0->snsCfg.astI2cData[LINEAR_SHR_H].bUpdate 	  = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_SHR_L].bUpdate 	  = CVI_TRUE;
			}
			if (gainsUpdate) {
				pstCfg0->snsCfg.astI2cData[LINEAR_HOLD].bUpdate   	  = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_GAIN_L].bUpdate 	  = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_GAIN_H].bUpdate 	  = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_REL].bUpdate 		  = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_GAIN_F].bUpdate 	  = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_MAG_1].bUpdate 	  = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_MAG_2].bUpdate 	  = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_COL_GAIN_H].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_COL_GAIN_L].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_MAG_3].bUpdate 	  = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_PRE_GAIN_H].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_PRE_GAIN_L].bUpdate = CVI_TRUE;
			}
			if (vtsUpdate) {
				pstCfg0->snsCfg.astI2cData[LINEAR_VMAX_L].bUpdate  	  = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[LINEAR_VMAX_H].bUpdate  	  = CVI_TRUE;
			}
		} else {
			if (shutterUpdate) {
				pstCfg0->snsCfg.astI2cData[WDR_SHR1_H].bUpdate 	   	= CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_SHR1_L].bUpdate 	   	= CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_SHR0_H].bUpdate 	   	= CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_SHR0_L].bUpdate 	   	= CVI_TRUE;
			}
			if (gainsUpdate) {
				pstCfg0->snsCfg.astI2cData[WDR_HOLD].bUpdate   	   	= CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_GAIN_L].bUpdate 	   	= CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_GAIN_H].bUpdate 	   	= CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_REL].bUpdate 	   	= CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_GAIN_F].bUpdate 	   	= CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_MAG_1].bUpdate 	   	= CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_MAG_2].bUpdate 	   	= CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_COL_GAIN_H].bUpdate 	= CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_COL_GAIN_L].bUpdate 	= CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_MAG_3].bUpdate 	   	= CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_PRE_GAIN_H].bUpdate 	= CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_PRE_GAIN_L].bUpdate 	= CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_PRE_GAIN1_H].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_PRE_GAIN1_L].bUpdate = CVI_TRUE;
			}
			if (vtsUpdate) {
				pstCfg0->snsCfg.astI2cData[WDR_VMAX_L].bUpdate 	   	= CVI_TRUE;
				pstCfg0->snsCfg.astI2cData[WDR_VMAX_H].bUpdate 	   	= CVI_TRUE;
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
	GC8613_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8SensorImageMode = pstSnsState->u8ImgMode;
	pstSnsState->bSyncInit = CVI_FALSE;

	if (pstSensorImageMode->f32Fps <= 25) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (GC8613_RES_IS_8M(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = GC8613_MODE_8M25;
			else
				goto unsupport_mode;
		} else {
			goto unsupport_mode;
		}
	} else if (pstSensorImageMode->f32Fps <= 30) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (GC8613_RES_IS_8M(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = GC8613_MODE_8M30;
			else if (GC8613_RES_IS_2M(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = GC8613_MODE_2M30;
			else {
				goto unsupport_mode;
			}
		} else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
			if (GC8613_RES_IS_8M(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = GC8613_MODE_8M30_WDR;
			else {
				goto unsupport_mode;
			}
		} else {
			goto unsupport_mode;
		}
	} else if (pstSensorImageMode->f32Fps == 60) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (GC8613_RES_IS_8M(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = GC8613_MODE_8M60_30;
			else {
				goto unsupport_mode;
			}
		} else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
				goto unsupport_mode;
			}
	} else {
	}

	if ((pstSnsState->bInit == CVI_TRUE) && (u8SensorImageMode == pstSnsState->u8ImgMode)) {
		return CVI_FAILURE;
	}

	pstSnsState->u8ImgMode = u8SensorImageMode;

	return CVI_SUCCESS;

unsupport_mode:
	CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
		pstSensorImageMode->u16Width,
		pstSensorImageMode->u16Height,
		pstSensorImageMode->f32Fps,
		pstSnsState->enWDRMode);
	return CVI_FAILURE;
}

static CVI_VOID sensor_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	GC8613_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);
	if (pstSnsState->bInit == CVI_TRUE && g_aeGc8613_MirrorFip[ViPipe] != eSnsMirrorFlip) {
		gc8613_mirror_flip(ViPipe, eSnsMirrorFlip);
		g_aeGc8613_MirrorFip[ViPipe] = eSnsMirrorFlip;
	}
}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	GC8613_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsState->bInit = CVI_FALSE;
	pstSnsState->bSyncInit = CVI_FALSE;
	pstSnsState->u8ImgMode = GC8613_MODE_8M30;
	pstSnsState->enWDRMode = WDR_MODE_NONE;
	pstSnsState->u32FLStd  = g_astGc8613_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[0] = g_astGc8613_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[1] = g_astGc8613_mode[pstSnsState->u8ImgMode].u32VtsDef;

	memset(&pstSnsState->astSyncInfo[0], 0, sizeof(ISP_SNS_SYNC_INFO_S));
	memset(&pstSnsState->astSyncInfo[1], 0, sizeof(ISP_SNS_SYNC_INFO_S));
}

static CVI_S32 sensor_rx_attr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *pstRxAttr)
{
	CVI_U8 u8SensorImageMode = 0;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	SNS_COMBO_DEV_ATTR_S *pstRxAttrSrc = CVI_NULL;

	GC8613_SENSOR_GET_CTX(ViPipe, pstSnsState);
	GC8613_SENSOR_GET_COMBO(ViPipe, pstRxAttrSrc);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pstRxAttr);
	CMOS_CHECK_POINTER(pstRxAttrSrc);

	memcpy(pstRxAttr, pstRxAttrSrc, sizeof(*pstRxAttr));


	pstRxAttr->img_size.start_x = g_astGc8613_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.s32X;
	pstRxAttr->img_size.start_y = g_astGc8613_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.s32Y;
	pstRxAttr->img_size.active_w = g_astGc8613_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.u32Width;
	pstRxAttr->img_size.active_h = g_astGc8613_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.u32Height;
	pstRxAttr->img_size.width = g_astGc8613_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Width;
	pstRxAttr->img_size.height = g_astGc8613_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Height;
	pstRxAttr->img_size.max_width = g_astGc8613_mode[pstSnsState->u8ImgMode].astImg[0].stMaxSize.u32Width;
	pstRxAttr->img_size.max_height = g_astGc8613_mode[pstSnsState->u8ImgMode].astImg[0].stMaxSize.u32Height;

	u8SensorImageMode = pstSnsState->u8ImgMode;

	if (pstSnsState->enWDRMode != WDR_MODE_NONE) {
		pstRxAttr->mclk.freq = CAMPLL_FREQ_24M;
		pstRxAttr->mipi_attr.dphy.hs_settle = 14;
	} else {
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;
		if (u8SensorImageMode == GC8613_MODE_8M60_30)
			pstRxAttr->mclk.freq = CAMPLL_FREQ_24M;
	}

	return CVI_SUCCESS;
}

static CVI_S32 sensor_patch_rx_attr(VI_PIPE ViPipe, RX_INIT_ATTR_S *pstRxInitAttr)
{
	SNS_COMBO_DEV_ATTR_S* pstRxAttr = CVI_NULL;
	int i;

	if (!g_pastGc8613ComboDevArray[ViPipe]) {
		pstRxAttr = malloc(sizeof(SNS_COMBO_DEV_ATTR_S));
	} else {
		GC8613_SENSOR_GET_COMBO(ViPipe, pstRxAttr);
	}
	memcpy(pstRxAttr, &gc8613_rx_attr, sizeof(SNS_COMBO_DEV_ATTR_S));
	GC8613_SENSOR_SET_COMBO(ViPipe, pstRxAttr);

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

void gc8613_exit(VI_PIPE ViPipe)
{
	if (g_pastGc8613ComboDevArray[ViPipe]) {
		free(g_pastGc8613ComboDevArray[ViPipe]);
		g_pastGc8613ComboDevArray[ViPipe] = CVI_NULL;
	}
	gc8613_i2c_exit(ViPipe);
}

static CVI_S32 cmos_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
	CMOS_CHECK_POINTER(pstSensorExpFunc);

	memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

	pstSensorExpFunc->pfn_cmos_sensor_init = gc8613_init;
	pstSensorExpFunc->pfn_cmos_sensor_exit = gc8613_exit;
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
	if (GC8613_I2C_ADDR_IS_VALID(s32I2cAddr))
		g_aunGc8613_AddrInfo[ViPipe].s8I2cAddr = s32I2cAddr;
	else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C addr input error ,please check [0x%x]\n", s32I2cAddr);
		g_aunGc8613_AddrInfo[ViPipe].s8I2cAddr = GC8613_I2C_ADDR_2;
	}
}

static CVI_S32 gc8613_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
	g_aunGc8613_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	GC8613_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

	if (pastSnsStateCtx == CVI_NULL) {
		pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
		if (pastSnsStateCtx == CVI_NULL) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
			return -ENOMEM;
		}
	}

	memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

	GC8613_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

	return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	GC8613_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
	SENSOR_FREE(pastSnsStateCtx);
	GC8613_SENSOR_RESET_CTX(ViPipe);
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

	stSnsAttrInfo.eSensorId = GC8613_ID;

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

	s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, GC8613_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, GC8613_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, GC8613_ID);
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
	g_au16Gc8613_GainMode[ViPipe] = pstInitAttr->enGainMode;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_probe(VI_PIPE ViPipe)
{
	return gc8613_probe(ViPipe);
}

ISP_SNS_OBJ_S stSnsGc8613_Obj = {
	.pfnRegisterCallback    = sensor_register_callback,
	.pfnUnRegisterCallback  = sensor_unregister_callback,
	.pfnStandby             = gc8613_standby,
	.pfnRestart             = gc8613_restart,
	.pfnMirrorFlip          = sensor_mirror_flip,
	.pfnWriteReg            = gc8613_write_register,
	.pfnReadReg             = gc8613_read_register,
	.pfnSetBusInfo          = gc8613_set_bus_info,
	.pfnSetInit             = sensor_set_init,
	.pfnPatchRxAttr		= sensor_patch_rx_attr,
	.pfnPatchI2cAddr	= sensor_patch_i2c_addr,
	.pfnGetRxAttr		= sensor_rx_attr,
	.pfnExpSensorCb		= cmos_init_sensor_exp_function,
	.pfnExpAeCb		= cmos_init_ae_exp_function,
	.pfnSnsProbe		= sensor_probe,
};


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

#include "gc4663_cmos_ex.h"
#include "gc4663_cmos_param.h"

#include "cvi_comm_cif.h"


#define DIV_0_TO_1(a)   ((0 == (a)) ? 1 : (a))
#define DIV_0_TO_1_FLOAT(a) ((((a) < 1E-10) && ((a) > -1E-10)) ? 1 : (a))
#define GC4663_ID 4663
#define GC4663_I2C_ADDR_1 0x10
#define GC4663_I2C_ADDR_2 0x29
#define GC4663_I2C_ADDR_IS_VALID(addr)      ((addr) == GC4663_I2C_ADDR_1 || (addr) == GC4663_I2C_ADDR_2)

/****************************************************************************
 * global variables                                                         *
 ***************************************************************************/

ISP_SNS_STATE_S *g_pastGc4663[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define GC4663_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastGc4663[dev])
#define GC4663_SENSOR_SET_CTX(dev, pstCtx)   (g_pastGc4663[dev] = pstCtx)
#define GC4663_SENSOR_RESET_CTX(dev)         (g_pastGc4663[dev] = CVI_NULL)

ISP_SNS_COMMBUS_U g_aunGc4663_BusInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cDev = 3},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

ISP_SNS_MIRRORFLIP_TYPE_E g_aeGc4663_MirrorFip[VI_MAX_PIPE_NUM] = {0};

CVI_U16 g_au16Gc4663_GainMode[VI_MAX_PIPE_NUM] = {0};

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
/*****Gc4663 Lines Range*****/
#define GC4663_FULL_LINES_MAX  (0x3fff)

/*****GC4663 Register Address*****/
#define GC4663_EXP_H_ADDR			0x0202 /* Exposure_long[13:8], 6bit */
#define GC4663_EXP_L_ADDR			0x0203 /* Exposure_long[7:0],  8bit */
#define GC4663_EXP_SHORT_H_ADDR		0x0200 /* Exposure_short[13:8], 6bit (HDR) */
#define GC4663_EXP_SHORT_L_ADDR		0x0201 /* Exposure_short[7:0],  8bit (HDR) */

#define GC4663_DGAIN_H_ADDR			0x020e /* Auto_pregain[9:6], 4bit */
#define GC4663_DGAIN_L_ADDR			0x020f /* Auto_pregain[5:0], 6bit */

#define GC4663_AGAIN_L_ADDR			0x02b3  /* ANALOG_PGA_gain[7:0],  8bit */
#define GC4663_AGAIN_H_ADDR			0x02b4  /* ANALOG_PGA_gain[10:8], 3bit */
#define GC4663_COL_AGAIN_H_ADDR			0x02b8  /* col_gain[13:6], 8bit */
#define GC4663_COL_AGAIN_L_ADDR			0x02b9  /* col_gain[5:0],  6bit */
#define GC4663_AGAIN_MAG1_ADDR			0x0515
#define GC4663_AGAIN_MAG2_ADDR			0x0519
#define GC4663_AGAIN_MAG3_ADDR			0x02d9

/* Frame Length[13:0] */
#define GC4663_VTS_H_ADDR			0x0340 /* VTS[13:8] */
#define GC4663_VTS_L_ADDR			0x0341 /* VTS[7:8] */

#define GC4663_FLIP_MIRROR_ADDR			0x0101
#define GC4663_WINDOW_HEIGHT  1456
#define GC4663_VB_OFFSET      (GC4663_WINDOW_HEIGHT + 20 + 16)  /* vb = vts - 1492 */
#define GC4663_HDR_EXP_MARGIN 16  /* T1+T2 < vts - 16 */
#define GC4663_FULL_LINES_MAX (0x3fff)

#define GC4663_RES_IS_1440P(w, h)      ((w) <= 2560 && (h) <= 1440)

static CVI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	const GC4663_MODE_S *pstMode;

	CVI_U32 FpsMax;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	GC4663_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstMode = &g_astGc4663_mode[pstSnsState->u8ImgMode];
	FpsMax = g_astGc4663_mode[pstSnsState->u8ImgMode].f32MaxFps;

	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FlickerFreq = 50 * 256;
	pstAeSnsDft->u32FullLinesMax = GC4663_FULL_LINES_MAX;
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
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	GC4663_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32Vts = g_astGc4663_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	f32MaxFps = g_astGc4663_mode[pstSnsState->u8ImgMode].f32MaxFps;
	f32MinFps = g_astGc4663_mode[pstSnsState->u8ImgMode].f32MinFps;

	if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
		u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport Fps: %f\n", f32Fps);
		return CVI_FAILURE;
	}

	u32VMAX = (u32VMAX > GC4663_FULL_LINES_MAX) ? GC4663_FULL_LINES_MAX : u32VMAX;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstSnsRegsInfo->astI2cData[LINEAR_VTS_H].u32Data = ((u32VMAX >> 8) & 0x3F);
		pstSnsRegsInfo->astI2cData[LINEAR_VTS_L].u32Data = (u32VMAX & 0xFF);
	} else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
		pstSnsRegsInfo->astI2cData[WDR_VTS_H].u32Data = ((u32VMAX >> 8) & 0x3F);
		pstSnsRegsInfo->astI2cData[WDR_VTS_L].u32Data = (u32VMAX & 0xFF);
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
	pstSnsState->au32FL[1] = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * DIV_0_TO_1_FLOAT(f32Fps));

	return CVI_SUCCESS;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static CVI_S32 cmos_inttime_update(VI_PIPE ViPipe, CVI_U32 *u32IntTime)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U32 u32Long, u32Short;

	GC4663_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(u32IntTime);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		CVI_U32 u32TmpIntTime = u32IntTime[0];
		CVI_U32 u32MaxExp = (pstSnsState->au32FL[0] > 16) ? (pstSnsState->au32FL[0] - 16) : 1;

		/* linear exposure reg range:
		 * min : 1
		 * max : vts - 16
		 * step : 1
		 */
		u32TmpIntTime = (u32TmpIntTime > u32MaxExp) ? u32MaxExp : u32TmpIntTime;
		if (u32TmpIntTime < 1)
			u32TmpIntTime = 1;
		u32IntTime[0] = u32TmpIntTime;

		u32Long = u32IntTime[0] & 0x3FFF;
		pstSnsRegsInfo->astI2cData[LINEAR_EXP_H].u32Data = (u32Long >> 8) & 0x3F;
		pstSnsRegsInfo->astI2cData[LINEAR_EXP_L].u32Data = u32Long & 0xFF;
	} else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
		CVI_U32 u32ShortMin = 2;
		CVI_U32 u32VbShort;
		CVI_U32 u32ShortMax;
		CVI_U32 u32LongMin = 2;
		CVI_U32 u32LongMax;
		CVI_U32 u32FL;

		/* GC4663 HDR: vb = vts - 1456 - 20 - 16, expS <= vb, expS+exoL < vts - 16 */

		/* u32IntTime[0]=short, u32IntTime[1]=long */
		u32Short = u32IntTime[0] & 0x3FFF;
		u32Long = u32IntTime[1] & 0x3FFF;

		u32FL = pstSnsState->au32FL[0];

		/* short: clamp to [min, vb], vb = FL - 1492 */
		u32VbShort = (u32FL > GC4663_VB_OFFSET) ? (u32FL - GC4663_VB_OFFSET) : 0;
		u32ShortMax = u32VbShort;
		u32Short = (u32Short > u32ShortMax) ? u32ShortMax : u32Short;
		if (u32Short < u32ShortMin)
			u32Short = u32ShortMin;

		/* long: clamp to [min, FL - 16 - expS], expS+exoL < vts - 16 */
		u32LongMax = (u32FL > (GC4663_HDR_EXP_MARGIN + u32Short)) ?
			     (u32FL - GC4663_HDR_EXP_MARGIN - u32Short) : u32LongMin;
		u32Long = (u32Long > u32LongMax) ? u32LongMax : u32Long;
		if (u32Long < u32LongMin)
			u32Long = u32LongMin;

		/* write back clamped values for AE reference */
		u32IntTime[0] = u32Short;
		u32IntTime[1] = u32Long;
		pstSnsState->au32WDRIntTime[0] = u32Short;
		pstSnsState->au32WDRIntTime[1] = u32Long;

		pstSnsRegsInfo->astI2cData[WDR_EXP_LONG_H].u32Data = (u32Long >> 8) & 0x3F;
		pstSnsRegsInfo->astI2cData[WDR_EXP_LONG_L].u32Data = u32Long & 0xFF;
		pstSnsRegsInfo->astI2cData[WDR_EXP_SHORT_H].u32Data = (u32Short >> 8) & 0x3F;
		pstSnsRegsInfo->astI2cData[WDR_EXP_SHORT_L].u32Data = u32Short & 0xFF;
	}
	return CVI_SUCCESS;
}

/* GC4663 gain reg table: 0x2b3, 0x2b4, 0x2b8, 0x2b9, 0x515, 0x519, 0x2d9，共 21 档 */
static CVI_U32 regValTable[21][7] = {
	{0x00, 0x00, 0x01, 0x00, 0x30, 0x28, 0x66},
	{0x20, 0x00, 0x01, 0x0B, 0x30, 0x2a, 0x68},
	{0x01, 0x00, 0x01, 0x19, 0x30, 0x27, 0x65},
	{0x21, 0x00, 0x01, 0x2A, 0x30, 0x29, 0x67},
	{0x02, 0x00, 0x02, 0x00, 0x30, 0x27, 0x65},
	{0x22, 0x00, 0x02, 0x17, 0x30, 0x29, 0x67},
	{0x03, 0x00, 0x02, 0x33, 0x30, 0x28, 0x66},
	{0x23, 0x00, 0x03, 0x14, 0x30, 0x2a, 0x68},
	{0x04, 0x00, 0x04, 0x00, 0x30, 0x2a, 0x68},
	{0x24, 0x00, 0x04, 0x2F, 0x30, 0x2b, 0x69},
	{0x05, 0x00, 0x05, 0x26, 0x30, 0x2c, 0x6A},
	{0x25, 0x00, 0x06, 0x28, 0x30, 0x2e, 0x6C},
	{0x06, 0x00, 0x08, 0x00, 0x30, 0x2f, 0x6D},
	{0x26, 0x00, 0x09, 0x1E, 0x30, 0x31, 0x6F},
	{0x46, 0x00, 0x0B, 0x0C, 0x30, 0x34, 0x72},
	{0x66, 0x00, 0x0D, 0x11, 0x30, 0x37, 0x75},
	{0x0e, 0x00, 0x10, 0x00, 0x30, 0x3a, 0x78},
	{0x2e, 0x00, 0x12, 0x3D, 0x30, 0x3e, 0x7C},
	{0x4e, 0x00, 0x16, 0x19, 0x30, 0x41, 0x7F},
	{0x6e, 0x00, 0x1A, 0x22, 0x30, 0x45, 0x83},
	{0x1e, 0x00, 0x20, 0x00, 0x30, 0x49, 0x87},
};

/* GC4663 analog gain table ，with 22 entries, ending with a sentinel */
static CVI_U32 gain_table[22] = {
	1024,  1200,  1424,  1696,  2048,  2416,  2864,  3392,  4096,  4848,
	5728,  6784,  8192,  9696,  11456, 13584, 16384, 19408, 22928, 27168,
	32768, 65535
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

	GC4663_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pu32Again);
	CMOS_CHECK_POINTER(pu32Dgain);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	u32Again = pu32Again[0];
	if (u32Again < gain_table[20]) {
		for (i = 1; i < total; i++) {
			if (*pu32Again < gain_table[i])
				break;
		}
		i--;
		u32Dgain = u32Again * 64 / gain_table[i];
		u32Again = i;
	} else {
		u32Again = 20; /* max analog gain */
		u32Dgain = pu32Dgain[0] * 64 / 1024;
	}

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_L].u32Data = regValTable[u32Again][0];
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_H].u32Data = regValTable[u32Again][1];
		pstSnsRegsInfo->astI2cData[LINEAR_COL_AGAIN_H].u32Data = regValTable[u32Again][2];
		pstSnsRegsInfo->astI2cData[LINEAR_COL_AGAIN_L].u32Data = regValTable[u32Again][3];
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_MAG1].u32Data = regValTable[u32Again][4];
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_MAG2].u32Data = regValTable[u32Again][5];
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_MAG3].u32Data = regValTable[u32Again][6];
		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_H].u32Data = (u32Dgain >> 6) & 0x0F;
		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_L].u32Data = (u32Dgain & 0x3F) << 2;
	} else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
		pstSnsRegsInfo->astI2cData[WDR_AGAIN_L].u32Data = regValTable[u32Again][0];
		pstSnsRegsInfo->astI2cData[WDR_AGAIN_H].u32Data = regValTable[u32Again][1];
		pstSnsRegsInfo->astI2cData[WDR_COL_AGAIN_H].u32Data = regValTable[u32Again][2];
		pstSnsRegsInfo->astI2cData[WDR_COL_AGAIN_L].u32Data = regValTable[u32Again][3];
		pstSnsRegsInfo->astI2cData[WDR_AGAIN_MAG1].u32Data = regValTable[u32Again][4];
		pstSnsRegsInfo->astI2cData[WDR_AGAIN_MAG2].u32Data = regValTable[u32Again][5];
		pstSnsRegsInfo->astI2cData[WDR_AGAIN_MAG3].u32Data = regValTable[u32Again][6];
		pstSnsRegsInfo->astI2cData[WDR_DGAIN_H].u32Data = (u32Dgain >> 6) & 0x0F;
		pstSnsRegsInfo->astI2cData[WDR_DGAIN_L].u32Data = (u32Dgain & 0x3F) << 2;
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport WDRMode: %d\n", pstSnsState->enWDRMode);
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

/* GC4663 HDR limits: window_height=1456, vb = vts - 1456 - 20 - 16 = vts - 1492
 * short <= vb, T1+T2 < vts - 16 => short + long <= FL - 16
 * au32Ratio[0] = long*64/short
 */
static CVI_S32 cmos_get_inttime_max(VI_PIPE ViPipe, CVI_U16 u16ManRatioEnable, CVI_U32 *au32Ratio,
				    CVI_U32 *au32IntTimeMax, CVI_U32 *au32IntTimeMin, CVI_U32 *pu32LFMaxIntTime)
{
	CVI_U32 u32IntTimeMaxTmp = 0, u32IntTimeMaxTmp0 = 0, u32IntTimeMaxTmp1 = 0;
	CVI_U32 u32ShortTimeMinLimit = 2;
	CVI_U32 u32VbShort;
	CVI_U32 u32FL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	(void)u16ManRatioEnable;

	CMOS_CHECK_POINTER(au32Ratio);
	CMOS_CHECK_POINTER(au32IntTimeMax);
	CMOS_CHECK_POINTER(au32IntTimeMin);
	CMOS_CHECK_POINTER(pu32LFMaxIntTime);
	GC4663_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	if (pstSnsState->enWDRMode != WDR_MODE_2To1_LINE)
		return CVI_SUCCESS;

	u32FL = pstSnsState->au32FL[0];

	/* short <= vb = FL - 1492 */
	u32VbShort = (u32FL > GC4663_VB_OFFSET) ? (u32FL - GC4663_VB_OFFSET) : 0;
	/* short + long <= FL - 16 => short <= (FL - 16) * 64 / (64 + ratio) */
	if (u32FL > GC4663_HDR_EXP_MARGIN) {
		u32IntTimeMaxTmp0 = ((u32FL - GC4663_HDR_EXP_MARGIN) * 0x40) /
				    DIV_0_TO_1(au32Ratio[0] + 0x40);
		u32IntTimeMaxTmp = (u32VbShort < u32IntTimeMaxTmp0) ? u32VbShort : u32IntTimeMaxTmp0;
	} else {
		u32IntTimeMaxTmp = u32VbShort;
	}

	/* same from FL[1] if different (2To1 typically FL[0]==FL[1]) */
	if (pstSnsState->au32FL[1] > GC4663_HDR_EXP_MARGIN) {
		u32IntTimeMaxTmp1 = ((pstSnsState->au32FL[1] - GC4663_HDR_EXP_MARGIN) * 0x40) /
				    DIV_0_TO_1(au32Ratio[0] + 0x40);
		if (u32IntTimeMaxTmp1 < u32IntTimeMaxTmp)
			u32IntTimeMaxTmp = u32IntTimeMaxTmp1;
	}

	u32IntTimeMaxTmp = (!u32IntTimeMaxTmp) ? u32ShortTimeMinLimit : u32IntTimeMaxTmp;
	if (u32IntTimeMaxTmp < u32ShortTimeMinLimit)
		u32IntTimeMaxTmp = u32ShortTimeMinLimit;

	au32IntTimeMax[0] = u32IntTimeMaxTmp;
	au32IntTimeMax[1] = au32IntTimeMax[0] * au32Ratio[0] >> 6;
	au32IntTimeMin[0] = u32ShortTimeMinLimit;
	au32IntTimeMin[1] = au32IntTimeMin[0] * au32Ratio[0] >> 6;
	*pu32LFMaxIntTime = au32IntTimeMax[1];

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

	memcpy(pstDef->stNoiseCalibration.CalibrationCoef,
		&g_stIspNoiseCalibratio, sizeof(ISP_CMOS_NOISE_CALIBRATION_S));

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
	const GC4663_MODE_S *pstMode = CVI_NULL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	GC4663_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstMode = &g_astGc4663_mode[pstSnsState->u8ImgMode];

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstIspCfg->frm_num = 1;
		memcpy(&pstIspCfg->img_size[0], &pstMode->astImg[0], sizeof(ISP_WDR_SIZE_S));
	} else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
		pstIspCfg->frm_num = 2;
		memcpy(&pstIspCfg->img_size[0], &pstMode->astImg[0], sizeof(ISP_WDR_SIZE_S));
		memcpy(&pstIspCfg->img_size[1], &pstMode->astImg[1], sizeof(ISP_WDR_SIZE_S));
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport WDRMode: %d\n", pstSnsState->enWDRMode);
	}
	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_wdr_mode(VI_PIPE ViPipe, CVI_U8 u8Mode)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	GC4663_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstSnsState->bSyncInit = CVI_FALSE;

	switch (u8Mode) {
	case WDR_MODE_NONE:
		pstSnsState->enWDRMode = WDR_MODE_NONE;
		pstSnsState->u8ImgMode = GC4663_MODE_2560X1440P30;
		pstSnsState->u32FLStd = g_astGc4663_mode[pstSnsState->u8ImgMode].u32VtsDef;
		CVI_TRACE_SNS(CVI_DBG_INFO, "WDR_MODE_NONE\n");
		break;
	case WDR_MODE_2To1_LINE:
		pstSnsState->enWDRMode = WDR_MODE_2To1_LINE;
		pstSnsState->u8ImgMode = GC4663_MODE_2560X1440P30_WDR;
		pstSnsState->u32FLStd = g_astGc4663_mode[pstSnsState->u8ImgMode].u32VtsDef;
		CVI_TRACE_SNS(CVI_DBG_INFO, "WDR_MODE_2To1_LINE 1440p\n");
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support WDR mode: %d\n", u8Mode);
		return CVI_FAILURE;
	}

	pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
	pstSnsState->au32FL[1] = pstSnsState->u32FLStd;
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
	GC4663_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstSnsRegsInfo = &pstSnsSyncInfo->snsCfg;
	pstCfg0 = &pstSnsState->astSyncInfo[0];
	pstCfg1 = &pstSnsState->astSyncInfo[1];
	pstI2c_data = pstCfg0->snsCfg.astI2cData;

	if ((pstSnsState->bSyncInit == CVI_FALSE) || (pstSnsRegsInfo->bConfig == CVI_FALSE)) {
		pstCfg0->snsCfg.enSnsType = SNS_I2C_TYPE;
		pstCfg0->snsCfg.unComBus.s8I2cDev = g_aunGc4663_BusInfo[ViPipe].s8I2cDev;
		pstCfg0->snsCfg.u8Cfg2ValidDelayMax = 0;
		pstCfg0->snsCfg.use_snsr_sram = CVI_TRUE;
		pstCfg0->snsCfg.u32RegNum = (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) ?
					    WDR_REGS_NUM : LINEAR_REGS_NUM;

		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			pstI2c_data[i].bUpdate = CVI_TRUE;
			pstI2c_data[i].u8DevAddr = gc4663_i2c_addr;
			pstI2c_data[i].u32AddrByteNum = gc4663_addr_byte;
			pstI2c_data[i].u32DataByteNum = gc4663_data_byte;
		}

		switch (pstSnsState->enWDRMode) {
		case WDR_MODE_NONE:
			pstI2c_data[LINEAR_EXP_H].u32RegAddr = GC4663_EXP_H_ADDR;
			pstI2c_data[LINEAR_EXP_L].u32RegAddr = GC4663_EXP_L_ADDR;
			pstI2c_data[LINEAR_AGAIN_L].u32RegAddr = GC4663_AGAIN_L_ADDR;
			pstI2c_data[LINEAR_AGAIN_H].u32RegAddr = GC4663_AGAIN_H_ADDR;
			pstI2c_data[LINEAR_COL_AGAIN_H].u32RegAddr = GC4663_COL_AGAIN_H_ADDR;
			pstI2c_data[LINEAR_COL_AGAIN_L].u32RegAddr = GC4663_COL_AGAIN_L_ADDR;
			pstI2c_data[LINEAR_AGAIN_MAG1].u32RegAddr = GC4663_AGAIN_MAG1_ADDR;
			pstI2c_data[LINEAR_AGAIN_MAG2].u32RegAddr = GC4663_AGAIN_MAG2_ADDR;
			pstI2c_data[LINEAR_AGAIN_MAG3].u32RegAddr = GC4663_AGAIN_MAG3_ADDR;
			pstI2c_data[LINEAR_DGAIN_H].u32RegAddr = GC4663_DGAIN_H_ADDR;
			pstI2c_data[LINEAR_DGAIN_L].u32RegAddr = GC4663_DGAIN_L_ADDR;
			pstI2c_data[LINEAR_VTS_H].u32RegAddr = GC4663_VTS_H_ADDR;
			pstI2c_data[LINEAR_VTS_L].u32RegAddr = GC4663_VTS_L_ADDR;
			pstI2c_data[LINEAR_FLIP_MIRROR].u32RegAddr = GC4663_FLIP_MIRROR_ADDR;
			break;
		case WDR_MODE_2To1_LINE:
			pstI2c_data[WDR_EXP_LONG_H].u32RegAddr = GC4663_EXP_H_ADDR;
			pstI2c_data[WDR_EXP_LONG_L].u32RegAddr = GC4663_EXP_L_ADDR;
			pstI2c_data[WDR_EXP_SHORT_H].u32RegAddr = GC4663_EXP_SHORT_H_ADDR;
			pstI2c_data[WDR_EXP_SHORT_L].u32RegAddr = GC4663_EXP_SHORT_L_ADDR;
			pstI2c_data[WDR_AGAIN_L].u32RegAddr = GC4663_AGAIN_L_ADDR;
			pstI2c_data[WDR_AGAIN_H].u32RegAddr = GC4663_AGAIN_H_ADDR;
			pstI2c_data[WDR_COL_AGAIN_H].u32RegAddr = GC4663_COL_AGAIN_H_ADDR;
			pstI2c_data[WDR_COL_AGAIN_L].u32RegAddr = GC4663_COL_AGAIN_L_ADDR;
			pstI2c_data[WDR_AGAIN_MAG1].u32RegAddr = GC4663_AGAIN_MAG1_ADDR;
			pstI2c_data[WDR_AGAIN_MAG2].u32RegAddr = GC4663_AGAIN_MAG2_ADDR;
			pstI2c_data[WDR_AGAIN_MAG3].u32RegAddr = GC4663_AGAIN_MAG3_ADDR;
			pstI2c_data[WDR_DGAIN_H].u32RegAddr = GC4663_DGAIN_H_ADDR;
			pstI2c_data[WDR_DGAIN_L].u32RegAddr = GC4663_DGAIN_L_ADDR;
			pstI2c_data[WDR_VTS_H].u32RegAddr = GC4663_VTS_H_ADDR;
			pstI2c_data[WDR_VTS_L].u32RegAddr = GC4663_VTS_L_ADDR;
			pstI2c_data[WDR_FLIP_MIRROR].u32RegAddr = GC4663_FLIP_MIRROR_ADDR;
			break;
		default:
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support WDR: %d\n", pstSnsState->enWDRMode);
			return CVI_FAILURE;
		}

		pstSnsState->bSyncInit = CVI_TRUE;
		pstCfg0->snsCfg.need_update = CVI_TRUE;
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
		pstCfg0->ispCfg.need_update =
			(sensor_cmp_wdr_size(&pstCfg0->ispCfg, &pstCfg1->ispCfg) ? CVI_TRUE : CVI_FALSE);
	}

	pstSnsRegsInfo->bConfig = CVI_FALSE;
	memcpy(pstSnsSyncInfo, &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));
	memcpy(&pstSnsState->astSyncInfo[1], &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));
	pstSnsState->au32FL[1] = pstSnsState->au32FL[0];

	if (pstSnsState->enWDRMode == WDR_MODE_NONE)
		pstCfg0->snsCfg.astI2cData[LINEAR_FLIP_MIRROR].bDropFrm = CVI_FALSE;
	else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE)
		pstCfg0->snsCfg.astI2cData[WDR_FLIP_MIRROR].bDropFrm = CVI_FALSE;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_image_mode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
	CVI_U8 u8SensorImageMode = 0;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstSensorImageMode);
	GC4663_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8SensorImageMode = pstSnsState->u8ImgMode;
	pstSnsState->bSyncInit = CVI_FALSE;

	if (pstSensorImageMode->f32Fps <= 30) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (GC4663_RES_IS_1440P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = GC4663_MODE_2560X1440P30;
			else {
				CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
					      pstSensorImageMode->u16Width,
					      pstSensorImageMode->u16Height,
					      pstSensorImageMode->f32Fps,
					      pstSnsState->enWDRMode);
				return CVI_FAILURE;
			}
		} else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
			if (GC4663_RES_IS_1440P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = GC4663_MODE_2560X1440P30_WDR;
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
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U8 value;

	GC4663_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	/* 0x0101[1:0]: 00=Normal(Gr), 01=Mirror(R), 10=Flip(B), 11=Mirror+Flip(Gb) */
	if (pstSnsState->bInit == CVI_TRUE && g_aeGc4663_MirrorFip[ViPipe] != eSnsMirrorFlip) {
		switch (eSnsMirrorFlip) {
		case ISP_SNS_NORMAL:
			value = 0; /* 00 */
			break;
		case ISP_SNS_MIRROR:
			value = 1; /* 01 */
			break;
		case ISP_SNS_FLIP:
			value = 2; /* 10 */
			break;
		case ISP_SNS_MIRROR_FLIP:
			value = 3; /* 11 */
			break;
		default:
			return;
		}

		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			pstSnsRegsInfo->astI2cData[LINEAR_FLIP_MIRROR].u32Data = value;
			pstSnsRegsInfo->astI2cData[LINEAR_FLIP_MIRROR].bDropFrm = 1;
			pstSnsRegsInfo->astI2cData[LINEAR_FLIP_MIRROR].u8DropFrmNum = 2;
		} else {
			pstSnsRegsInfo->astI2cData[WDR_FLIP_MIRROR].u32Data = value;
			pstSnsRegsInfo->astI2cData[WDR_FLIP_MIRROR].bDropFrm = 1;
			pstSnsRegsInfo->astI2cData[WDR_FLIP_MIRROR].u8DropFrmNum = 2;
		}
		g_aeGc4663_MirrorFip[ViPipe] = eSnsMirrorFlip;
	}
}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	GC4663_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsState->bInit = CVI_FALSE;
	pstSnsState->bSyncInit = CVI_FALSE;
	pstSnsState->u8ImgMode = GC4663_MODE_2560X1440P30;
	pstSnsState->enWDRMode = WDR_MODE_NONE;
	pstSnsState->u32FLStd  = g_astGc4663_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[0] = g_astGc4663_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[1] = g_astGc4663_mode[pstSnsState->u8ImgMode].u32VtsDef;

	memset(&pstSnsState->astSyncInfo[0], 0, sizeof(ISP_SNS_SYNC_INFO_S));
	memset(&pstSnsState->astSyncInfo[1], 0, sizeof(ISP_SNS_SYNC_INFO_S));
}

static CVI_S32 sensor_rx_attr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *pstRxAttr)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	GC4663_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pstRxAttr);

	memcpy(pstRxAttr, &gc4663_rx_attr, sizeof(*pstRxAttr));

	pstRxAttr->img_size.width = g_astGc4663_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Width;
	pstRxAttr->img_size.height = g_astGc4663_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Height;
	if (pstSnsState->enWDRMode == WDR_MODE_NONE)
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;
	else if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE)
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_VC;

	return CVI_SUCCESS;

}

static CVI_S32 sensor_patch_rx_attr(RX_INIT_ATTR_S *pstRxInitAttr)
{
	SNS_COMBO_DEV_ATTR_S *pstRxAttr = &gc4663_rx_attr;
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

	pstSensorExpFunc->pfn_cmos_sensor_init = gc4663_init;
	pstSensorExpFunc->pfn_cmos_sensor_exit = gc4663_exit;
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
	if (GC4663_I2C_ADDR_IS_VALID(s32I2cAddr))
		gc4663_i2c_addr = s32I2cAddr;
}

static CVI_S32 gc4663_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
	g_aunGc4663_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	GC4663_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

	if (pastSnsStateCtx == CVI_NULL) {
		pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
		if (pastSnsStateCtx == CVI_NULL) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
			return -ENOMEM;
		}
	}

	memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

	GC4663_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

	return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	GC4663_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
	SENSOR_FREE(pastSnsStateCtx);
	GC4663_SENSOR_RESET_CTX(ViPipe);
	g_aeGc4663_MirrorFip[ViPipe] = ISP_SNS_NORMAL;
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

	stSnsAttrInfo.eSensorId = GC4663_ID;

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
	CVI_S32 s32Ret = CVI_SUCCESS;;

	CMOS_CHECK_POINTER(pstAeLib);
	CMOS_CHECK_POINTER(pstAwbLib);

	s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, GC4663_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, GC4663_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, GC4663_ID);
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
	g_au16Gc4663_GainMode[ViPipe] = pstInitAttr->enGainMode;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_probe(VI_PIPE ViPipe)
{
	return gc4663_probe(ViPipe);
}

ISP_SNS_OBJ_S stSnsGc4663_Obj = {
	.pfnRegisterCallback    = sensor_register_callback,
	.pfnUnRegisterCallback  = sensor_unregister_callback,
	.pfnStandby             = gc4663_standby,
	.pfnRestart             = gc4663_restart,
	.pfnWriteReg            = gc4663_write_register,
	.pfnReadReg             = gc4663_read_register,
	.pfnSetBusInfo          = gc4663_set_bus_info,
	.pfnSetInit             = sensor_set_init,
	.pfnMirrorFlip          = sensor_mirror_flip,
	.pfnPatchRxAttr         = sensor_patch_rx_attr,
	.pfnPatchI2cAddr        = sensor_patch_i2c_addr,
	.pfnGetRxAttr           = sensor_rx_attr,
	.pfnExpSensorCb         = cmos_init_sensor_exp_function,
	.pfnExpAeCb             = cmos_init_ae_exp_function,
	.pfnSnsProbe            = sensor_probe,
};


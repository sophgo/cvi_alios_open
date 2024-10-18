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

#include "ov9732_cmos_ex.h"
#include "ov9732_cmos_param.h"
#ifdef ARCH_CV182X
#include "cvi_vip_cif_uapi.h"
#else
#include "cif_uapi.h"
#endif

#define DIV_0_TO_1(a) ((0 == (a)) ? 1 : (a))
#define DIV_0_TO_1_FLOAT(a) ((((a) < 1E-10) && ((a) > -1E-10)) ? 1 : (a))
#define OV9732_ID 0x9732
#define OV9732_I2C_ADDR_1 0x36
#define OV9732_I2C_ADDR_IS_VALID(addr) \
        ((addr) == OV9732_I2C_ADDR_1)
/****************************************************************************
 * global variables                                                            *
 ****************************************************************************/

ISP_SNS_STATE_S *g_pastOv9732[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define OV9732_SENSOR_GET_CTX(dev, pstCtx) (pstCtx = g_pastOv9732[dev])
#define OV9732_SENSOR_SET_CTX(dev, pstCtx) (g_pastOv9732[dev] = pstCtx)
#define OV9732_SENSOR_RESET_CTX(dev) (g_pastOv9732[dev] = CVI_NULL)

ISP_SNS_COMMBUS_U g_aunOv9732_BusInfo[VI_MAX_PIPE_NUM] = {
    [0] = {.s8I2cDev = 2},
    [1 ... VI_MAX_PIPE_NUM - 1] = {.s8I2cDev = -1}};

CVI_U16 g_au16Ov9732_GainMode[VI_MAX_PIPE_NUM] = {0};
CVI_U16 g_au16Ov9732_UseHwSync[VI_MAX_PIPE_NUM] = {0};

ISP_SNS_MIRRORFLIP_TYPE_E g_aeOv9732_MirrorFip[VI_MAX_PIPE_NUM] = {0};

/****************************************************************************
 * local variables and functions                                                           *
 ****************************************************************************/
static CVI_U32 g_au32InitExposure[VI_MAX_PIPE_NUM] = {0};
static CVI_U32 g_au32LinesPer500ms[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16InitWBGain[VI_MAX_PIPE_NUM][3] = {{0}};
static CVI_U16 g_au16SampleRgain[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16SampleBgain[VI_MAX_PIPE_NUM] = {0};
static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg);
/*****Ov9732 Lines Range*****/
#define OV9732_FULL_LINES_MAX (0xFFFF)

/*****Ov9732 Register Address*****/
#define OV9732_EXP0_ADDR 0x3502
#define OV9732_EXP1_ADDR 0x3501
#define OV9732_EXP2_ADDR 0x3500
#define OV9732_AGAIN_ADDR 0x350B
#define OV9732_DGAIN0_ADDR 0x5180
#define OV9732_DGAIN1_ADDR 0x5181
#define OV9732_DGAIN2_ADDR 0x5182
#define OV9732_DGAIN3_ADDR 0x5183
#define OV9732_DGAIN4_ADDR 0x5184
#define OV9732_DGAIN5_ADDR 0x5185
#define OV9732_VTS0_ADDR 0x380E
#define OV9732_VTS1_ADDR 0x380F

#define OV9732_RES_IS_720P(w, h) ((w) == 1280 && (h) == 720)

static CVI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
        const OV9732_MODE_S *pstMode;
        ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

        CMOS_CHECK_POINTER(pstAeSnsDft);
        OV9732_SENSOR_GET_CTX(ViPipe, pstSnsState);
        CMOS_CHECK_POINTER(pstSnsState);

        pstMode = &g_astOv9732_mode[pstSnsState->u8ImgMode];
        pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
        pstAeSnsDft->u32FlickerFreq = 50 * 256;
        pstAeSnsDft->u32FullLinesMax = OV9732_FULL_LINES_MAX;
        pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * 30);

        pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
        pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
        pstAeSnsDft->stIntTimeAccu.f32Offset = 0;

        pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
        pstAeSnsDft->stAgainAccu.f32Accuracy = 1;

        pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_DB;
        pstAeSnsDft->stDgainAccu.f32Accuracy = 1;

        pstAeSnsDft->u32ISPDgainShift = 8;
        pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
        pstAeSnsDft->u32MaxISPDgainTarget = 2 << pstAeSnsDft->u32ISPDgainShift;

        if (g_au32LinesPer500ms[ViPipe] == 0)
                pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * 30 / 2;
        else
                pstAeSnsDft->u32LinesPer500ms = g_au32LinesPer500ms[ViPipe];
        pstAeSnsDft->u32SnsStableFrame = 0;
        /* OV sensor cannot update new setting before the old setting takes effect */
        pstAeSnsDft->u8AERunInterval = 1;
        switch (pstSnsState->enWDRMode)
        {
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
        OV9732_SENSOR_GET_CTX(ViPipe, pstSnsState);
        CMOS_CHECK_POINTER(pstSnsState);

        u32Vts = g_astOv9732_mode[pstSnsState->u8ImgMode].u32VtsDef;
        pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
        f32MaxFps = g_astOv9732_mode[pstSnsState->u8ImgMode].f32MaxFps;
        f32MinFps = g_astOv9732_mode[pstSnsState->u8ImgMode].f32MinFps;

        switch (pstSnsState->u8ImgMode)
        {
        case OV9732_MODE_1280X720P30:
                if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps))
                {
                        u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
                }
                else
                {
                        CVI_TRACE_SNS(CVI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
                        return CVI_FAILURE;
                }
                u32VMAX = (u32VMAX > OV9732_FULL_LINES_MAX) ? OV9732_FULL_LINES_MAX : u32VMAX;
                break;
        default:
                CVI_TRACE_SNS(CVI_DBG_ERR, "Not support sensor mode: %d\n", pstSnsState->u8ImgMode);
                return CVI_FAILURE;
        }

        pstSnsState->u32FLStd = u32VMAX;

        if (pstSnsState->enWDRMode == WDR_MODE_NONE)
        {
                pstSnsRegsInfo->astI2cData[LINEAR_VTS_0].u32Data = ((u32VMAX & 0xFF00) >> 8);
                pstSnsRegsInfo->astI2cData[LINEAR_VTS_1].u32Data = (u32VMAX & 0xFF);
        }
        else
        {
                CVI_TRACE_SNS(CVI_DBG_ERR, "Not support WDR: %d\n", pstSnsState->enWDRMode);
                return CVI_FAILURE;
        }

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

        OV9732_SENSOR_GET_CTX(ViPipe, pstSnsState);
        CMOS_CHECK_POINTER(pstSnsState);
        CMOS_CHECK_POINTER(u32IntTime);
        pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

        if (pstSnsState->enWDRMode == WDR_MODE_NONE)
        {
                /* linear exposure reg range:
                 * min : 4
                 * max : vts - 4
                 * step : 1
                 */
                CVI_U32 u32TmpIntTime = u32IntTime[0];
                CVI_U32 mimExp = 4;
                CVI_U32 maxExp = pstSnsState->au32FL[0] - 4;

                u32TmpIntTime = (u32TmpIntTime > maxExp) ? maxExp : u32TmpIntTime;
                u32TmpIntTime = (u32TmpIntTime < mimExp) ? mimExp : u32TmpIntTime;
                u32IntTime[0] = u32TmpIntTime;

                pstSnsRegsInfo->astI2cData[LINEAR_EXP_2].u32Data = ((u32TmpIntTime & 0xF000) >> 12);
                pstSnsRegsInfo->astI2cData[LINEAR_EXP_1].u32Data = ((u32TmpIntTime & 0xFF0) >> 4);
                pstSnsRegsInfo->astI2cData[LINEAR_EXP_0].u32Data = ((u32TmpIntTime & 0x0F) << 4);

        }
        else
        {
                CVI_TRACE_SNS(CVI_DBG_ERR, "Not support WDR: %d\n", pstSnsState->enWDRMode);
                return CVI_FAILURE;
        }

        return CVI_SUCCESS;
}

static CVI_U32 Again_value[] =
{
    0x010, 0x011, 0x012, 0x013, 0x014, 0x015, 0x016, 0x017, 0x018, 0x019, 0x01A, 0x01B, 0x01C, 0x01D, 0x01E, 0x01F,
    0x020, 0x022, 0x024, 0x026, 0x028, 0x02A, 0x02C, 0x02E, 0x030, 0x032, 0x034, 0x036, 0x038, 0x03A, 0x03C, 0x03E,
    0x040, 0x044, 0x048, 0x04C, 0x050, 0x054, 0x058, 0x05C, 0x060, 0x064, 0x068, 0x06C, 0x070, 0x074, 0x078, 0x07C,
    0x080, 0x088, 0x090, 0x098, 0x0A0, 0x0A8, 0x0B0, 0x0B8, 0x0C0, 0x0C8, 0x0D0, 0x0D8, 0x0E0, 0x0E8, 0x0F0, 0x0F8
};

static CVI_U32 Again_table[] =
{
    1024, 1088, 1152, 1216, 1280, 1344, 1408, 1472, 1536, 1600, 1664, 1728, 1792, 1856, 1920, 1984, 2048, 2176,
    2304, 2432, 2560, 2688, 2816, 2944, 3072, 3200, 3328, 3456, 3584, 3712, 3840, 3968, 4096, 4352, 4608, 4864,
    5120, 5376, 5632, 5888, 6144, 6400, 6656, 6912, 7168, 7424, 7680, 7936, 8192, 8704, 9216, 9728, 10240, 10752,
    11264, 11776, 12288, 12800, 13312, 13824, 14336, 14848, 15360, 15872
};

static const CVI_U32 again_table_size = ARRAY_SIZE(Again_table);

static CVI_U32 Dgain_value[] =
{
    0x100, 0x103, 0x106, 0x109, 0x10C, 0x10F, 0x112, 0x115, 0x118, 0x11B, 0x11E, 0x121, 0x124, 0x127, 0x12A, 0x12D,
    0x130, 0x133, 0x136, 0x139, 0x13C, 0x13F, 0x142, 0x145, 0x148, 0x14B, 0x14E, 0x151, 0x154, 0x157, 0x15A, 0x15D,
    0x160, 0x163, 0x166, 0x169, 0x16C, 0x16F, 0x172, 0x175, 0x178, 0x17B, 0x17E, 0x181, 0x184, 0x187, 0x18A, 0x18D,
    0x190, 0x193, 0x196, 0x199, 0x19C, 0x19F, 0x1A2, 0x1A5, 0x1A8, 0x1AB, 0x1AE, 0x1B1, 0x1B4, 0x1B7, 0x1BA, 0x1BD,
    0x1C0, 0x1C3, 0x1C6, 0x1C9, 0x1CC, 0x1CF, 0x1D2, 0x1D5, 0x1D8, 0x1DB, 0x1DE, 0x1E1, 0x1E4, 0x1E7, 0x1EA, 0x1ED,
    0x1F0, 0x1F3, 0x1F6, 0x1F9, 0x1FC, 0x1FF, 0x202, 0x205, 0x208, 0x20B, 0x20E, 0x211, 0x214, 0x217, 0x21A, 0x21D,
    0x220, 0x223, 0x226, 0x229, 0x22C, 0x22F, 0x232, 0x235, 0x238, 0x23B, 0x23E, 0x241, 0x244, 0x247, 0x24A, 0x24D,
    0x250, 0x253, 0x256, 0x259, 0x25C, 0x25F, 0x262, 0x265, 0x268, 0x26B, 0x26E, 0x271, 0x274, 0x277, 0x27A, 0x27D,
    0x280, 0x283, 0x286, 0x289, 0x28C, 0x28F, 0x292, 0x295, 0x298, 0x29B, 0x29E, 0x2A1, 0x2A4, 0x2A7, 0x2AA, 0x2AD,
    0x2B0, 0x2B3, 0x2B6, 0x2B9, 0x2BC, 0x2BF, 0x2C2, 0x2C5, 0x2C8, 0x2CB, 0x2CE, 0x2D1, 0x2D4, 0x2D7, 0x2DA, 0x2DD,
    0x2E0, 0x2E3, 0x2E6, 0x2E9, 0x2EC, 0x2EF, 0x2F2, 0x2F5, 0x2F8, 0x2FB, 0x2FE, 0x301, 0x304, 0x307, 0x30A, 0x30D,
    0x310, 0x313, 0x316, 0x319, 0x31C, 0x31F, 0x322, 0x325, 0x328, 0x32B, 0x32E, 0x331, 0x334, 0x337, 0x33A, 0x33D,
    0x340, 0x343, 0x346, 0x349, 0x34C, 0x34F, 0x352, 0x355, 0x358, 0x35B, 0x35E, 0x361, 0x364, 0x367, 0x36A, 0x36D,
    0x370, 0x373, 0x376, 0x379, 0x37C, 0x37F, 0x382, 0x385, 0x388, 0x38B, 0x38E, 0x391, 0x394, 0x397, 0x39A, 0x39D,
    0x3A0, 0x3A3, 0x3A6, 0x3A9, 0x3AC, 0x3AF, 0x3B2, 0x3B5, 0x3B8, 0x3BB, 0x3BE, 0x3C1, 0x3C4, 0x3C7, 0x3CA, 0x3CD,
    0x3D0, 0x3D3, 0x3D6, 0x3D9, 0x3DC, 0x3DF, 0x3E2, 0x3E5, 0x3E8, 0x3EB, 0x3EE, 0x3F1, 0x3F4, 0x3F7, 0x3FA, 0x3FD,
    0x3FF
};

static CVI_U32 Dgain_table[] =
{
    1024, 1036, 1048, 1060, 1072, 1084, 1096, 1108, 1120, 1132, 1144, 1156, 1168, 1180, 1192, 1204, 1216, 1228,
    1240, 1252, 1264, 1276, 1288, 1300, 1312, 1324, 1336, 1348, 1360, 1372, 1384, 1396, 1408, 1420, 1432, 1444,
    1468, 1480, 1492, 1504, 1516, 1528, 1540, 1552, 1564, 1576, 1588, 1600, 1612, 1624, 1636, 1648, 1660, 1672,
    1684, 1696, 1708, 1720, 1732, 1744, 1756, 1768, 1780, 1792, 1804, 1816, 1828, 1840, 1852, 1864, 1876, 1888,
    1900, 1912, 1924, 1936, 1948, 1960, 1972, 1984, 1996, 2008, 2020, 2032, 2044, 2056, 2068, 2080, 2092, 2104,
    2116, 2128, 2140, 2152, 2164, 2176, 2188, 2200, 2212, 2224, 2236, 2248, 2260, 2272, 2284, 2296, 2308, 2320,
    2332, 2344, 2356, 2368, 2380, 2392, 2404, 2416, 2428, 2440, 2452, 2464, 2476, 2488, 2500, 2512, 2524, 2536,
    2548, 2560, 2572, 2584, 2596, 2608, 2620, 2632, 2644, 2656, 2668, 2680, 2692, 2704, 2716, 2728, 2740, 2752,
};

static CVI_S32 cmos_again_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32AgainLin, CVI_U32 *pu32AgainDb)
{
        CVI_U32 i;

        (void)ViPipe;

        CMOS_CHECK_POINTER(pu32AgainLin);
        CMOS_CHECK_POINTER(pu32AgainDb);

        if (*pu32AgainLin >= Again_table[again_table_size - 1])
        {
                *pu32AgainLin = Again_table[again_table_size - 1];
                *pu32AgainDb = again_table_size - 1;
                return CVI_SUCCESS;
        }

        for (i = 1; i < again_table_size; i++)
        {
                if (*pu32AgainLin < Again_table[i])
                {
                        *pu32AgainLin = Again_table[i - 1];
                        *pu32AgainDb = i - 1;
                        break;
                }
        }
        return CVI_SUCCESS;
}


static const CVI_U32 dgain_table_size = ARRAY_SIZE(Dgain_table);

static CVI_S32 cmos_dgain_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32DgainLin, CVI_U32 *pu32DgainDb)
{
        CVI_U32 i;

        (void)ViPipe;

        CMOS_CHECK_POINTER(pu32DgainLin);
        CMOS_CHECK_POINTER(pu32DgainDb);

        if (*pu32DgainLin >= Dgain_table[dgain_table_size - 1])
        {
                *pu32DgainLin = Dgain_table[dgain_table_size - 1];
                *pu32DgainDb = dgain_table_size - 1;
                return CVI_SUCCESS;
        }

        for (i = 1; i < dgain_table_size; i++)
        {
                if (*pu32DgainLin < Dgain_table[i])
                {
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

        OV9732_SENSOR_GET_CTX(ViPipe, pstSnsState);
        CMOS_CHECK_POINTER(pstSnsState);
        CMOS_CHECK_POINTER(pu32Again);
        CMOS_CHECK_POINTER(pu32Dgain);
        pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
        u32Again = pu32Again[0];
        u32Dgain = pu32Dgain[0];

        if (pstSnsState->enWDRMode == WDR_MODE_NONE)
        {
                pstSnsRegsInfo->astI2cData[LINEAR_AGAIN].u32Data = Again_value[u32Again];
                pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_0].u32Data = (Dgain_value[u32Dgain] & 0xF00) >> 8;
                pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_1].u32Data = (Dgain_value[u32Dgain] & 0xFF);
                pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_2].u32Data = (Dgain_value[u32Dgain] & 0xF00) >> 8;
                pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_3].u32Data = (Dgain_value[u32Dgain] & 0xFF);
                pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_4].u32Data = (Dgain_value[u32Dgain] & 0xF00) >> 8;
                pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_5].u32Data = (Dgain_value[u32Dgain] & 0xFF);
        }

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

        return CVI_SUCCESS;
}

static CVI_S32 cmos_get_awb_default(VI_PIPE ViPipe, AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
        (void)ViPipe;

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
        (void)ViPipe;

        memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));

        return CVI_SUCCESS;
}

static CVI_S32 cmos_get_blc_default(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *pstBlc)
{
        (void)ViPipe;

        CMOS_CHECK_POINTER(pstBlc);

        memset(pstBlc, 0, sizeof(ISP_CMOS_BLACK_LEVEL_S));

        memcpy(pstBlc, &g_stIspBlcCalibratio, sizeof(ISP_CMOS_BLACK_LEVEL_S));

        return CVI_SUCCESS;
}

static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg)
{
        const OV9732_MODE_S *pstMode = CVI_NULL;
        ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

        OV9732_SENSOR_GET_CTX(ViPipe, pstSnsState);
        CMOS_CHECK_POINTER(pstSnsState);
        pstMode = &g_astOv9732_mode[pstSnsState->u8ImgMode];

        pstIspCfg->frm_num = 1;
        memcpy(&pstIspCfg->img_size[0], &pstMode->astImg[0], sizeof(ISP_WDR_SIZE_S));

        return CVI_SUCCESS;
}

static CVI_S32 cmos_set_wdr_mode(VI_PIPE ViPipe, CVI_U8 u8Mode)
{
        ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

        OV9732_SENSOR_GET_CTX(ViPipe, pstSnsState);
        CMOS_CHECK_POINTER(pstSnsState);

        pstSnsState->bSyncInit = CVI_FALSE;

        switch (u8Mode)
        {
        case WDR_MODE_NONE:
                pstSnsState->u8ImgMode = OV9732_MODE_1280X720P30;
                pstSnsState->enWDRMode = WDR_MODE_NONE;
                pstSnsState->u32FLStd = g_astOv9732_mode[pstSnsState->u8ImgMode].u32VtsDef;
                syslog(LOG_INFO, "linear mode\n");
                break;
        default:
                CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport sensor mode!\n");
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
        for (i = 0; i < 2; i++)
        {
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
        OV9732_SENSOR_GET_CTX(ViPipe, pstSnsState);
        CMOS_CHECK_POINTER(pstSnsState);
        pstSnsRegsInfo = &pstSnsSyncInfo->snsCfg;
        pstCfg0 = &pstSnsState->astSyncInfo[0];
        pstCfg1 = &pstSnsState->astSyncInfo[1];
        pstI2c_data = pstCfg0->snsCfg.astI2cData;

        if ((pstSnsState->bSyncInit == CVI_FALSE) || (pstSnsRegsInfo->bConfig == CVI_FALSE))
        {
                pstCfg0->snsCfg.enSnsType = SNS_I2C_TYPE;
                pstCfg0->snsCfg.unComBus.s8I2cDev = g_aunOv9732_BusInfo[ViPipe].s8I2cDev;
                pstCfg0->snsCfg.u8Cfg2ValidDelayMax = 0;
                pstCfg0->snsCfg.use_snsr_sram = CVI_TRUE;
                pstCfg0->snsCfg.u32RegNum = LINEAR_REGS_NUM;

                for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++)
                {
                        pstI2c_data[i].bUpdate = CVI_TRUE;
                        pstI2c_data[i].u8DevAddr = ov9732_i2c_addr;
                        pstI2c_data[i].u32AddrByteNum = ov9732_addr_byte;
                        pstI2c_data[i].u32DataByteNum = ov9732_data_byte;
                }

                switch (pstSnsState->enWDRMode)
                {
                default:
                        pstI2c_data[LINEAR_EXP_0].u32RegAddr = OV9732_EXP0_ADDR;
                        pstI2c_data[LINEAR_EXP_1].u32RegAddr = OV9732_EXP1_ADDR;
                        pstI2c_data[LINEAR_EXP_2].u32RegAddr = OV9732_EXP2_ADDR;
                        pstI2c_data[LINEAR_AGAIN].u32RegAddr = OV9732_AGAIN_ADDR;
                        pstI2c_data[LINEAR_DGAIN_0].u32RegAddr = OV9732_DGAIN0_ADDR;
                        pstI2c_data[LINEAR_DGAIN_1].u32RegAddr = OV9732_DGAIN1_ADDR;
                        pstI2c_data[LINEAR_DGAIN_2].u32RegAddr = OV9732_DGAIN2_ADDR;
                        pstI2c_data[LINEAR_DGAIN_3].u32RegAddr = OV9732_DGAIN3_ADDR;
                        pstI2c_data[LINEAR_DGAIN_4].u32RegAddr = OV9732_DGAIN4_ADDR;
                        pstI2c_data[LINEAR_DGAIN_5].u32RegAddr = OV9732_DGAIN5_ADDR;
                        pstI2c_data[LINEAR_VTS_0].u32RegAddr = OV9732_VTS0_ADDR;
                        pstI2c_data[LINEAR_VTS_1].u32RegAddr = OV9732_VTS1_ADDR;
                        break;
                }
                pstSnsState->bSyncInit = CVI_TRUE;
                pstCfg0->snsCfg.need_update = CVI_TRUE;
                /* recalcualte WDR size */
                cmos_get_wdr_size(ViPipe, &pstCfg0->ispCfg);
                pstCfg0->ispCfg.need_update = CVI_TRUE;
        }
        else
        {
                pstCfg0->snsCfg.need_update = CVI_FALSE;
                for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++)
                {
                        if (pstCfg0->snsCfg.astI2cData[i].u32Data == pstCfg1->snsCfg.astI2cData[i].u32Data)
                        {
                                pstCfg0->snsCfg.astI2cData[i].bUpdate = CVI_FALSE;
                        }
                        else
                        {
                                pstCfg0->snsCfg.astI2cData[i].bUpdate = CVI_TRUE;
                                pstCfg0->snsCfg.need_update = CVI_TRUE;
                        }
                }
                /* check update isp crop or not */
                pstCfg0->ispCfg.need_update = (sensor_cmp_wdr_size(&pstCfg0->ispCfg, &pstCfg1->ispCfg) ? CVI_TRUE : CVI_FALSE);
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
        OV9732_SENSOR_GET_CTX(ViPipe, pstSnsState);
        CMOS_CHECK_POINTER(pstSnsState);

        u8SensorImageMode = pstSnsState->u8ImgMode;
        pstSnsState->bSyncInit = CVI_FALSE;
        if (pstSensorImageMode->f32Fps <= 30)
        {
                if (pstSnsState->enWDRMode == WDR_MODE_NONE)
                {
                        if (OV9732_RES_IS_720P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
                                u8SensorImageMode = OV9732_MODE_1280X720P30;
                        else
                        {
                                CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
                                              pstSensorImageMode->u16Width,
                                              pstSensorImageMode->u16Height,
                                              pstSensorImageMode->f32Fps,
                                              pstSnsState->enWDRMode);
                                return CVI_FAILURE;
                        }
                }
                else
                {
                        CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
                                      pstSensorImageMode->u16Width,
                                      pstSensorImageMode->u16Height,
                                      pstSensorImageMode->f32Fps,
                                      pstSnsState->enWDRMode);
                        return CVI_FAILURE;
                }
        }

        if ((pstSnsState->bInit == CVI_TRUE) && (u8SensorImageMode == pstSnsState->u8ImgMode))
        {
                /* Don't need to switch SensorImageMode */
                return CVI_FAILURE;
        }
        pstSnsState->u8ImgMode = u8SensorImageMode;

        return CVI_SUCCESS;
}

static CVI_VOID sensor_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
        ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

        OV9732_SENSOR_GET_CTX(ViPipe, pstSnsState);
        CMOS_CHECK_POINTER_VOID(pstSnsState);
        if (pstSnsState->bInit == CVI_TRUE && g_aeOv9732_MirrorFip[ViPipe] != eSnsMirrorFlip)
        {
                ov9732_mirror_flip(ViPipe, eSnsMirrorFlip);
                g_aeOv9732_MirrorFip[ViPipe] = eSnsMirrorFlip;
        }
}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe)
{
        ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

        OV9732_SENSOR_GET_CTX(ViPipe, pstSnsState);
        CMOS_CHECK_POINTER_VOID(pstSnsState);

        pstSnsState->bInit = CVI_FALSE;
        pstSnsState->bSyncInit = CVI_FALSE;
        pstSnsState->u8ImgMode = OV9732_MODE_1280X720P30;
        pstSnsState->enWDRMode = WDR_MODE_NONE;
        pstSnsState->u32FLStd = g_astOv9732_mode[pstSnsState->u8ImgMode].u32VtsDef;
        pstSnsState->au32FL[0] = g_astOv9732_mode[pstSnsState->u8ImgMode].u32VtsDef;
        pstSnsState->au32FL[1] = g_astOv9732_mode[pstSnsState->u8ImgMode].u32VtsDef;

        memset(&pstSnsState->astSyncInfo[0], 0, sizeof(ISP_SNS_SYNC_INFO_S));
        memset(&pstSnsState->astSyncInfo[1], 0, sizeof(ISP_SNS_SYNC_INFO_S));
}

static CVI_S32 sensor_rx_attr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *pstRxAttr)
{
        ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

        OV9732_SENSOR_GET_CTX(ViPipe, pstSnsState);
        CMOS_CHECK_POINTER(pstSnsState);
        CMOS_CHECK_POINTER(pstRxAttr);

        memcpy(pstRxAttr, &ov9732_rx_attr, sizeof(*pstRxAttr));

        pstRxAttr->img_size.width = g_astOv9732_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Width;
        pstRxAttr->img_size.height = g_astOv9732_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Height;
        if (pstSnsState->enWDRMode == WDR_MODE_NONE)
                pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;

        return CVI_SUCCESS;
}

static CVI_S32 sensor_patch_rx_attr(RX_INIT_ATTR_S *pstRxInitAttr)
{
        SNS_COMBO_DEV_ATTR_S *pstRxAttr = &ov9732_rx_attr;
        int i;

        CMOS_CHECK_POINTER(pstRxInitAttr);

        if (pstRxInitAttr->stMclkAttr.bMclkEn)
                pstRxAttr->mclk.cam = pstRxInitAttr->stMclkAttr.u8Mclk;

        if (pstRxInitAttr->MipiDev >= VI_MAX_DEV_NUM)
                return CVI_SUCCESS;

        pstRxAttr->devno = pstRxInitAttr->MipiDev;

        if (pstRxAttr->input_mode == INPUT_MODE_MIPI)
        {
                struct mipi_dev_attr_s *attr = &pstRxAttr->mipi_attr;

                for (i = 0; i < MIPI_LANE_NUM + 1; i++)
                {
                        attr->lane_id[i] = pstRxInitAttr->as16LaneId[i];
                        attr->pn_swap[i] = pstRxInitAttr->as8PNSwap[i];
                }
        }
        else
        {
                struct lvds_dev_attr_s *attr = &pstRxAttr->lvds_attr;

                for (i = 0; i < MIPI_LANE_NUM + 1; i++)
                {
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

        pstSensorExpFunc->pfn_cmos_sensor_init = ov9732_init;
        pstSensorExpFunc->pfn_cmos_sensor_exit = ov9732_exit;
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
        if (OV9732_I2C_ADDR_IS_VALID(s32I2cAddr))
                ov9732_i2c_addr = s32I2cAddr;
}

static CVI_S32 ov9732_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
        g_aunOv9732_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

        return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
        ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

        OV9732_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

        if (pastSnsStateCtx == CVI_NULL)
        {
                pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
                if (pastSnsStateCtx == CVI_NULL)
                {
                        CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
                        return -ENOMEM;
                }
        }

        memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

        OV9732_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

        return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
        ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

        OV9732_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
        SENSOR_FREE(pastSnsStateCtx);
        OV9732_SENSOR_RESET_CTX(ViPipe);
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

        stSnsAttrInfo.eSensorId = OV9732_ID;

        s32Ret = cmos_init_sensor_exp_function(&stIspRegister.stSnsExp);
        s32Ret |= CVI_ISP_SensorRegCallBack(ViPipe, &stSnsAttrInfo, &stIspRegister);

        if (s32Ret != CVI_SUCCESS)
        {
                CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function failed!\n");
                return s32Ret;
        }

        s32Ret = cmos_init_ae_exp_function(&stAeRegister.stAeExp);
        s32Ret |= CVI_AE_SensorRegCallBack(ViPipe, pstAeLib, &stSnsAttrInfo, &stAeRegister);

        if (s32Ret != CVI_SUCCESS)
        {
                CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function to ae lib failed!\n");
                return s32Ret;
        }

        s32Ret = cmos_init_awb_exp_function(&stAwbRegister.stAwbExp);
        s32Ret |= CVI_AWB_SensorRegCallBack(ViPipe, pstAwbLib, &stSnsAttrInfo, &stAwbRegister);

        if (s32Ret != CVI_SUCCESS)
        {
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

        s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, OV9732_ID);
        if (s32Ret != CVI_SUCCESS)
        {
                CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
                return s32Ret;
        }

        s32Ret = CVI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, OV9732_ID);
        if (s32Ret != CVI_SUCCESS)
        {
                CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
                return s32Ret;
        }

        s32Ret = CVI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, OV9732_ID);
        if (s32Ret != CVI_SUCCESS)
        {
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
        g_au16Ov9732_GainMode[ViPipe] = pstInitAttr->enGainMode;
        g_au16Ov9732_UseHwSync[ViPipe] = pstInitAttr->u16UseHwSync;

        return CVI_SUCCESS;
}
static CVI_S32 sensor_probe(VI_PIPE ViPipe)
{
        return ov9732_probe(ViPipe);
}

ISP_SNS_OBJ_S stSnsOv9732_Obj = {
    .pfnRegisterCallback = sensor_register_callback,
    .pfnUnRegisterCallback = sensor_unregister_callback,
    .pfnStandby = ov9732_standby,
    .pfnRestart = ov9732_restart,
    .pfnMirrorFlip = sensor_mirror_flip,
    .pfnWriteReg = ov9732_write_register,
    .pfnReadReg = ov9732_read_register,
    .pfnSetBusInfo = ov9732_set_bus_info,
    .pfnSetInit = sensor_set_init,
    .pfnPatchRxAttr = sensor_patch_rx_attr,
    .pfnPatchI2cAddr = sensor_patch_i2c_addr,
    .pfnGetRxAttr = sensor_rx_attr,
    .pfnExpSensorCb = cmos_init_sensor_exp_function,
    .pfnExpAeCb = cmos_init_ae_exp_function,
    .pfnSnsProbe = sensor_probe,
};

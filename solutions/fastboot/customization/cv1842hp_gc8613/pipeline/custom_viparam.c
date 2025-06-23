/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_viparam.c
 * Description:
 *   ....
 */
#include "custom_param.h"

PARAM_CLASSDEFINE(PARAM_SNS_CFG_S,SENSORCFG,CTX,Sensor)[] = {
    {
        .s32Framerate = 25,
        .enSnsType = GCORE_GC8613_MIPI_8M_25FPS_10BIT,
        .MipiDev = 0,
        .s32BusId = 3,
        .s8I2cDev = 3,
        .s32I2cAddr = 49,
        .as16LaneId = {2, 0, 1, 3, 4},
        .as8PNSwap = {0, 0, 0, 0, 0},
        .bMclkEn = 1,
        .u8Mclk = RX_MAC_CLK_900M,
        .u8MclkFreq = CAMPLL_FREQ_27M,
        .u8MclkCam = 0,
        .u8Orien = 0,
        .bHwSync = 0,
        .u32Rst_pol = 1,
        .u32Rst_pin = 8,
        .u32Rst_port_idx = 2,
        .u8Rotation = 0,
    },
};

PARAM_CLASSDEFINE(PARAM_ISP_CFG_S,ISPCFG,CTX,ISP)[] = {
    {
        .bMonoSet = {0},
        .bUseSingleBin = 0,
        .stPQBinDes[0] =
        {
            .pIspBinData = NULL,
            .u32IspBinDataLen = 0,
        },
        .s8ByPassNum = 5,
    },
};

PARAM_CLASSDEFINE(PARAM_CHN_CFG_S,CHNCFG,CTX,CHN)[] = {
    {
        .s32ChnId = 0,
        .enWDRMode = WDR_MODE_NONE,
        .bYuvBypassPath = 0,
        .f32Fps = -1,
        .u32Width = 3840,
        .u32Height = 2160,
        .enPixFormat = PIXEL_FORMAT_NV12,
        .enDynamicRange = DYNAMIC_RANGE_SDR8,
        .enVideoFormat = VIDEO_FORMAT_LINEAR,
        .enCompressMode = COMPRESS_MODE_TILE,
    },
};

PARAM_CLASSDEFINE(PARAM_PIPE_CFG_S,PIPECFG,CTX,PIPE)[] = {
    {
        .pipe = {0, -1, -1, -1, -1, -1},
    },
};

PARAM_VI_CFG_S g_stViCtx = {
    .u32WorkSnsCnt = 1,
    .pstSensorCfg = PARAM_CLASS(SENSORCFG,CTX,Sensor),
    .pstPipeInfo = PARAM_CLASS(PIPECFG,CTX,PIPE),
    .pstIspCfg = PARAM_CLASS(ISPCFG,CTX,ISP),
    .pstChnInfo = PARAM_CLASS(CHNCFG,CTX,CHN),
};

PARAM_VI_CFG_S * PARAM_GET_VI_CFG(void) {
    return &g_stViCtx;
}

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
        .s32Framerate = 15,
        .enSnsType = GCORE_GC4653_MIPI_4M_30FPS_10BIT,
        .MipiDev = 0,
        .s32BusId = 2,
        .s8I2cDev = 2,
        .s32I2cAddr = 41,
        .as16LaneId = {3, 2, 4, -1, -1},
        .as8PNSwap = {0, 0, 0, 0, 0},
        .bMclkEn = 0,
        .u8Mclk = RX_MAC_CLK_200M,
        .u8MclkCam = 1,
        .u8MclkFreq = CAMPLL_FREQ_27M,
        .u8Orien = 0,
        .bHwSync = 0,
        .u32Rst_port_idx = 2,
        .u32Rst_pin = 13,
        .u32Rst_pol = 1,
        .u8Rotation = 0,
    },
};

PARAM_CLASSDEFINE(PARAM_CHN_CFG_S,CHNCFG,CTX,CHN)[] = {
    {
        .s32ChnId = 0,
        .enWDRMode = WDR_MODE_NONE,
        .f32Fps = 15,
        .u32Width = 2560,
        .u32Height = 1440,
        .enPixFormat = PIXEL_FORMAT_NV21,
        .enDynamicRange = DYNAMIC_RANGE_SDR8,
        .enVideoFormat = VIDEO_FORMAT_LINEAR,
        .enCompressMode = COMPRESS_MODE_TILE,
    },
};

PARAM_CLASSDEFINE(PARAM_PIPE_CFG_S,PIPECFG,CTX,PIPE)[] = {
    {
        .pipe[0] = 0,
        .pipe[1] = -1,
        .pipe[2] = -1,
        .pipe[3] = -1,
        .pipe[4] = -1,
        .pipe[5] = -1,
    },
};

PARAM_CLASSDEFINE(PARAM_ISP_CFG_S,ISPCFG,CTX,ISP)[] = {
    {
        .s8FastConvergeAvailableNode = 5,
        .as16firstFrLuma = {62, 77, 173, 343, 72},
        .as16targetBv = {89, 194, 479, 533, 721},
    },
};

PARAM_CLASSDEFINE(PARAM_DEV_CFG_S,DEVCFG,CTX,DEV)[] = {
    {
    },
};

PARAM_VI_CFG_S g_stViCtx = {
    .u32WorkSnsCnt = 1,
    .pstSensorCfg = PARAM_CLASS(SENSORCFG,CTX,Sensor),
    .pstChnInfo = PARAM_CLASS(CHNCFG,CTX,CHN),
    .pstPipeInfo = PARAM_CLASS(PIPECFG,CTX,PIPE),
    .bFastConverge = 0,
    .pstIspCfg = PARAM_CLASS(ISPCFG,CTX,ISP),
    .pstDevInfo = PARAM_CLASS(DEVCFG,CTX,DEV),
};

PARAM_VI_CFG_S * PARAM_GET_VI_CFG(void) {
    return &g_stViCtx;
}








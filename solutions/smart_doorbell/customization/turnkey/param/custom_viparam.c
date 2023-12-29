/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_viparam.c
 * Description:
 *   ....
 */
#include "custom_param.h"

extern unsigned char rgb_color_mode_param[];
extern unsigned int rgb_color_len;
extern unsigned char rgb_mono_mode_param[];
extern unsigned int rgb_mono_len;
extern void * g_ViDmaBuf;
extern unsigned int g_ViDmaBufSize;

PARAM_CLASSDEFINE(PARAM_SNS_CFG_S,SENSORCFG,CTX,Sensor)[] = {
    {
        .s32Framerate = 30,
        .enSnsType = CONFIG_SNS0_TYPE,
        .s32I2cAddr = 0x37,
        .s8I2cDev = 2,
        .u32Rst_port_idx = 2,//GPIOC_12
        .u32Rst_pin = 12,
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,
        .u8Rotation = ROTATION_90,
        .bHwSync = CVI_FALSE,
    },
};

PARAM_CLASSDEFINE(PARAM_ISP_CFG_S,ISPCFG,CTX,ISP)[] = {
    {
        .bMonoSet = {0, 0},
        .bUseSingleBin = 0,
        .astPQBinDes[0] =
        {
            .pIspBinData = rgb_color_mode_param,
            .binID = CVI_BIN_ID_ISP0,
        },
        .astPQBinDes[1] =
        {
            .pIspBinData = rgb_mono_mode_param,
            .binID = CVI_BIN_ID_ISP1,
        },
    },
};

PARAM_CLASSDEFINE(PARAM_DEV_CFG_S,VIDEVCFG,CTX,VI)[] = {
    {
        .pViDmaBuf = NULL,
        .u32ViDmaBufSize = 0,
    }
};

PARAM_VI_CFG_S g_stViCtx = {
    .u32WorkSnsCnt = 1,
    .u32IspSceneNum = 2,
    .bFastConverge = 0,
    .pstSensorCfg = PARAM_CLASS(SENSORCFG,CTX,Sensor),
    .pstIspCfg = PARAM_CLASS(ISPCFG,CTX,ISP),
    .pstDevInfo = PARAM_CLASS(VIDEVCFG,CTX,VI)
};

PARAM_VI_CFG_S * PARAM_GET_VI_CFG(void) {
    if(g_ViDmaBuf == NULL) {
        g_ViDmaBuf = (void *)malloc(g_ViDmaBufSize);
    }
    g_stViCtx.pstDevInfo[0].pViDmaBuf = g_ViDmaBuf;
    g_stViCtx.pstDevInfo[0].u32ViDmaBufSize = g_ViDmaBufSize;
    g_stViCtx.pstIspCfg[0].astPQBinDes[0].u32IspBinDataLen = rgb_color_len;
    g_stViCtx.pstIspCfg[0].astPQBinDes[1].u32IspBinDataLen = rgb_mono_len;
    return &g_stViCtx;
}

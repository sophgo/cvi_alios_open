/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_viparam.c
 * Description:
 *   ....
 */
#include "ir_custom_param.h"

// extern unsigned char cvi_isp_default_param[];
// extern unsigned int default_length;
void * g_ViDmaBuf = NULL;
unsigned int g_ViDmaBufSize = 13 * 1024 * 1024;

PARAM_CLASSDEFINE(PARAM_SNS_CFG_S,SENSORCFG,CTX,Sensor)[] = {
    {
        .enSnsType = CONFIG_SNS0_TYPE,
        .s32I2cAddr = 0X10,
        .s8I2cDev = 2,
        .u32Rst_port_idx = 2,//GPIOC_12
        .u32Rst_pin = 12,
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,
        .u8Rotation = ROTATION_0,
        .bHwSync = CVI_TRUE,
    },
    {
        .enSnsType = CONFIG_SNS1_TYPE,
        .s32I2cAddr = 0X37,
        .s8I2cDev = 4,
        .u32Rst_port_idx = 2,//GPIOC_12
        .u32Rst_pin = 12,
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,
        .u8Rotation = ROTATION_0,
        .bHwSync = CVI_TRUE,
        .u8DisableRst = CVI_TRUE,
    }
};

PARAM_CLASSDEFINE(PARAM_ISP_CFG_S,ISPCFG,CTX,ISP)[] = {
    {
        .bMonoSet = {1, 1},
        .bUseSingleBin = 1,
        // .stPQBinDes =
        // {
        //     .pIspBinData = cvi_isp_default_param,
        //     .u32IspBinDataLen = 0,
        // },
    }
};

PARAM_CLASSDEFINE(PARAM_DEV_CFG_S,VIDEVCFG,CTX,VI)[] = {
    {
        .pViDmaBuf = NULL,
        .u32ViDmaBufSize = 0,
    }
};

PARAM_VI_CFG_S g_stFcViCtx = {
    .u32WorkSnsCnt = 2,
    .u32IspSceneNum = 1,
    .bFastConverge = 1,
    .pstSensorCfg = PARAM_CLASS(SENSORCFG,CTX,Sensor),
    .pstIspCfg = PARAM_CLASS(ISPCFG,CTX,ISP),
    .pstDevInfo = PARAM_CLASS(VIDEVCFG,CTX,VI)
};

PARAM_VI_CFG_S * IR_PARAM_GET_VI_CFG(void)
{
    if(g_ViDmaBuf == NULL) {
        g_ViDmaBuf = (void *)malloc(g_ViDmaBufSize);
    }
    g_stFcViCtx.pstDevInfo->pViDmaBuf = g_ViDmaBuf;
    g_stFcViCtx.pstDevInfo[0].u32ViDmaBufSize = g_ViDmaBufSize;
    // g_stFcViCtx.pstIspCfg[0].stPQBinDes.u32IspBinDataLen = default_length;//非常量无法直接初始化
    return &g_stFcViCtx;
}

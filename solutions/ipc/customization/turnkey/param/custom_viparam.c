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
        .enSnsType = CONFIG_SNS0_TYPE,
        .s32I2cAddr = -1,
        .s8I2cDev = 3,
        .u32Rst_port_idx = 0,//GPIOA_2
        .u32Rst_pin = 2,
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,
        /* config mipi attr */
        .bSetDevAttrMipi = 1,
        .as16LaneId[0] = 2,
        .as16LaneId[1] = 3,
        .as16LaneId[2] = 1,
        .as16LaneId[3] = 4,
        .as16LaneId[4] = 0,
        .as8PNSwap[0] = 1,
        .as8PNSwap[1] = 1,
        .as8PNSwap[2] = 1,
        .as8PNSwap[3] = 1,
        .as8PNSwap[4] = 1,
        /* config dev attr(reference sensor driver) */
        .bSetDevAttr = 1,
        .u8MclkCam = 0,
        .s16MacClk = RX_MAC_CLK_200M,
        .u8MclkFreq = CAMPLL_FREQ_37P125M,
    }
};

PARAM_CLASSDEFINE(PARAM_ISP_CFG_S,ISPCFG,CTX,ISP)[] = {
    {
        .bMonoSet = {0},
        .bUseSingleBin = 0,
    },
};

PARAM_CLASSDEFINE(PARAM_DEV_CFG_S,VIDEVCFG,CTX,VI)[] = {
    {
        .pViDmaBuf = NULL,
        .u32ViDmaBufSize = 0,
    #if CONFIG_SENSOR_DUAL_SWITCH
        .isMux = true,
        .u8AttachDev = 0,
        .switchGpioIdx = -1,
        .switchGpioPin = -1,
        .switchGPioPol = -1,
    #endif
    },
    {
    #if CONFIG_SENSOR_DUAL_SWITCH
        .isMux = true,
        .u8AttachDev = 1,
        .switchGpioIdx = -1,
        .switchGpioPin = -1,
        .switchGPioPol = -1,
    #endif
    }
};

PARAM_VI_CFG_S g_stViCtx = {
    .u32WorkSnsCnt = 1,
    .pstSensorCfg = PARAM_CLASS(SENSORCFG,CTX,Sensor),
    .pstIspCfg = PARAM_CLASS(ISPCFG,CTX,ISP),
    .pstDevInfo = PARAM_CLASS(VIDEVCFG,CTX,VI)
};

PARAM_VI_CFG_S * PARAM_GET_VI_CFG(void) {
    return &g_stViCtx;
}








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
        .as16LaneId[0] = 1,
        .as16LaneId[1] = 2,
        .as16LaneId[2] = 0,
        .as16LaneId[3] = -1,
        .as16LaneId[4] = -1,
        .as8PNSwap[0] = 1,
        .as8PNSwap[1] = 1,
        .as8PNSwap[2] = 1,
        .as8PNSwap[3] = 0,
        .as8PNSwap[4] = 0,
        /* config dev attr(reference sensor driver) */
        .bSetDevAttr = 1,
        .u8MclkCam = 0,
        .s16MacClk = RX_MAC_CLK_200M,
        .u8MclkFreq = CAMPLL_FREQ_27M,
    }
};

PARAM_CLASSDEFINE(PARAM_ISP_CFG_S,ISPCFG,CTX,ISP)[] = {
    {
        .bMonoSet = {0},
        .bUseSingleBin = 0,
        .stPQBinDes =
        {
            .pIspBinData = NULL,
            .u32IspBinDataLen = 0,
        },
    },
};

PARAM_VI_CFG_S g_stViCtx = {
    .u32WorkSnsCnt = 1,
    .pstSensorCfg = PARAM_CLASS(SENSORCFG,CTX,Sensor),
    .pstIspCfg = PARAM_CLASS(ISPCFG,CTX,ISP),
};

PARAM_VI_CFG_S * PARAM_GET_VI_CFG(void) {
    return &g_stViCtx;
}








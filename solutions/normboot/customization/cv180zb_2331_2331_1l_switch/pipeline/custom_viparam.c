/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_viparam.c
 * Description:
 *   ....
 */
#include "custom_param.h"

PARAM_CLASSDEFINE(PARAM_SNS_CFG_S,SENSORCFG,CTX,Sensor)[] = {
#if 0
    {
#if defined (CONFIG_CHIP_cv1811c) || defined (CONFIG_CHIP_cv1801c) || defined (CONFIG_CHIP_cv1812cp)
        .enSnsType = CONFIG_SNS0_TYPE,
        .s32I2cAddr = -1,
        .s8I2cDev = 2,
        .u32Rst_port_idx = 2,//GPIOC_13
        .u32Rst_pin = 13,
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,
#endif
#if defined (CONFIG_CHIP_cv1811h) || defined (CONFIG_CHIP_cv1812h) || defined (CONFIG_CHIP_cv1811ha) || defined (CONFIG_CHIP_cv1812ha) || defined(CONFIG_CHIP_cv1813h)
        .enSnsType = CONFIG_SNS0_TYPE,
        .s32I2cAddr = -1,
        .s8I2cDev = 3,
        .u32Rst_port_idx = 0,//GPIOA_2
        .u32Rst_pin = 2,
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,
#endif
#if defined (CONFIG_CHIP_cv1801b) || defined (CONFIG_CHIP_cv180zb)
        .enSnsType = CONFIG_SNS0_TYPE,
        .s32I2cAddr = -1,
        .s8I2cDev = 1,
        .u32Rst_port_idx = 2,//GPIOC_8
        .u32Rst_pin = 8,
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,
#endif
    }
#else
    {
        .s32Framerate = 30,
        .enSnsType = SMS_SC2331_1L_SLAVE_MIPI_2M_30FPS_10BIT,
        .MipiDev = 0,
        .s32BusId = 1,
        .s8I2cDev = 1,
        .s32I2cAddr = 48,
        .as16LaneId = {3, 2, -1, -1, -1},
        .as8PNSwap = {0, 0, 0, 0, 0},
        .bMclkEn = 1,
        .u8Mclk = RX_MAC_CLK_200M,
        .u8MclkFreq = CAMPLL_FREQ_27M,
        .u8MclkCam = 0,
        .u8Orien = 0,
        .bHwSync = 0,
        .u32Rst_pol = 1,
        .u32Rst_pin = 8,
        .u32Rst_port_idx = 2,
        .u8Rotation = 0,
    },
    {
        .s32Framerate = 30,
        .enSnsType = SMS_SC2331_1L_SLAVE1_MIPI_2M_30FPS_10BIT,
        .MipiDev = 0,
        .s32BusId = 1,
        .s8I2cDev = 1,
        .s32I2cAddr = 50,
        .as16LaneId = {3, 2, -1, -1, -1},
        .as8PNSwap = {0, 0, 0, 0, 0},
        .bMclkEn = 1,
        .u8Mclk = RX_MAC_CLK_200M,
        .u8MclkFreq = CAMPLL_FREQ_27M,
        .u8MclkCam = 0,
        .u8Orien = 0,
        .bHwSync = 0,
        .u32Rst_pol = 1,
        .u32Rst_pin = 8,
        .u32Rst_port_idx = 2,
        .u8Rotation = 0,
    }
#endif
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
        .u32Width = 1920,
        .u32Height = 1080,
        .enPixFormat = PIXEL_FORMAT_NV21,
        .enDynamicRange = DYNAMIC_RANGE_SDR8,
        .enVideoFormat = VIDEO_FORMAT_LINEAR,
        .enCompressMode = COMPRESS_MODE_TILE,
    },
    {
        .s32ChnId = 1,
        .enWDRMode = WDR_MODE_NONE,
        .bYuvBypassPath = 0,
        .f32Fps = -1,
        .u32Width = 1920,
        .u32Height = 1080,
        .enPixFormat = PIXEL_FORMAT_NV21,
        .enDynamicRange = DYNAMIC_RANGE_SDR8,
        .enVideoFormat = VIDEO_FORMAT_LINEAR,
        .enCompressMode = COMPRESS_MODE_TILE,
    }
};

PARAM_CLASSDEFINE(PARAM_PIPE_CFG_S,PIPECFG,CTX,PIPE)[] = {
    {
        .pipe = {0, -1, -1, -1, -1, -1},
    },
    {
        .pipe = {3, -1, -1, -1, -1, -1},
    }
};

PARAM_VI_CFG_S g_stViCtx = {
    .u32WorkSnsCnt = 2,
    .pstSensorCfg = PARAM_CLASS(SENSORCFG,CTX,Sensor),
    .pstPipeInfo = PARAM_CLASS(PIPECFG,CTX,PIPE),
    .pstIspCfg = PARAM_CLASS(ISPCFG,CTX,ISP),
    .pstChnInfo = PARAM_CLASS(CHNCFG,CTX,CHN),
};

PARAM_VI_CFG_S * PARAM_GET_VI_CFG(void) {
    return &g_stViCtx;
}

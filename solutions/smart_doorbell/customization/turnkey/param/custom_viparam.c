/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_viparam.c
 * Description:
 *   ....
 */
#include "custom_param.h"

extern uint8_t isp_param_rgb[];
extern uint32_t isp_param_rgb_len;
extern uint8_t isp_param_ir_face[];
extern uint32_t isp_param_ir_face_len;

PARAM_CLASSDEFINE(PARAM_SNS_CFG_S, SENSORCFG, CTX, Sensor)
[] = {
    {
        .enSnsType       = CONFIG_SNS0_TYPE,
        .s32I2cAddr      = 0x3d,
        .s8I2cDev        = 1,
        .bSetDevAttrMipi = 1,
        .as16LaneId      = {3, 4, -1, -1, -1},
        .as8PNSwap       = {0, 0, 0, 0, 0},
        .bSetDevAttr     = 1,
        .u8MclkCam       = 0,
        .u8MclkFreq      = CAMPLL_FREQ_24M,
        .s16MacClk       = RX_MAC_CLK_200M,
#if CONFIG_SENSOR_DUAL_SWITCH
        .bDualSwitch = CVI_TRUE,
#endif
    },
    {
        .enSnsType       = CONFIG_SNS1_TYPE,
        .s32I2cAddr      = 0x3c,
        .s8I2cDev        = 1,
        .bSetDevAttrMipi = 1,
        .as16LaneId      = {3, 4, -1, -1, -1},
        .as8PNSwap       = {0, 0, 0, 0, 0},
        .bSetDevAttr     = 1,
        .u8MclkCam       = 0,
        .u8MclkFreq      = CAMPLL_FREQ_24M,
        .s16MacClk       = RX_MAC_CLK_200M,
#if CONFIG_SENSOR_DUAL_SWITCH
        .bDualSwitch = CVI_TRUE,
#endif
    },
};

PARAM_CLASSDEFINE(PARAM_ISP_CFG_S, ISPCFG, CTX, ISP)
[] = {
    {
        .bMonoSet      = {0, 0},
        .bUseSingleBin = 0,
        .astPQBinDes[0] =
            {
                .pIspBinData = isp_param_rgb,
                .binID       = CVI_BIN_ID_ISP0,
            },
        .astPQBinDes[1] =
            {
                .pIspBinData = isp_param_ir_face,
                .binID       = CVI_BIN_ID_ISP2,
            },
    },
};

static PARAM_DEV_CFG_S VIDEVCFG_CTX_VI[] = {
    {
        .pViDmaBuf       = NULL,
        .u32ViDmaBufSize = 0,
        .stViLdcCfg =
            {
                /* Developer should configure other params when enable vi ldc */
                .bLdcEn = false,
            },
#if CONFIG_SENSOR_DUAL_SWITCH
        .isMux         = true,
        .u8AttachDev   = 0,
        .switchGpioIdx = -1,
        .switchGpioPin = -1,
        .switchGPioPol = -1,
        .dstFrm        = 5,
        .isFrmCtrl     = true,
#endif
    },
    {
        .stViLdcCfg =
            {
                /* Developer should configure other params when enable vi ldc */
                .bLdcEn = false,
            },
#if CONFIG_SENSOR_DUAL_SWITCH
        .isMux         = true,
        .u8AttachDev   = 1,
        .switchGpioIdx = -1,
        .switchGpioPin = -1,
        .switchGPioPol = -1,
        .dstFrm        = 1,
        .isFrmCtrl     = true,
#endif
    },
};

PARAM_VI_CFG_S g_stViCtx = {
    .u32WorkSnsCnt = 2,
    .pstSensorCfg  = PARAM_CLASS(SENSORCFG, CTX, Sensor),
    .pstIspCfg     = PARAM_CLASS(ISPCFG, CTX, ISP),
    .pstDevInfo    = PARAM_CLASS(VIDEVCFG, CTX, VI),
    .bFastConverge = true,
};

PARAM_VI_CFG_S* PARAM_GET_VI_CFG(void)
{
    g_stViCtx.pstIspCfg[0].astPQBinDes[0].u32IspBinDataLen = isp_param_rgb_len;
    g_stViCtx.pstIspCfg[0].astPQBinDes[1].u32IspBinDataLen = isp_param_ir_face_len;
    return &g_stViCtx;
}
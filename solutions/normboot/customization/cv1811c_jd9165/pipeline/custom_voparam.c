/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_vocfg.c
 * Description:
 *   ....
 */
#include "custom_param.h"
#include "cvi_buffer.h"
#include "cvi_mipi_tx.h"
#include "board_config.h"

PARAM_CLASSDEFINE(VO_CHN_ATTR_S,VOCHN,VO,CHN)[] = {
    {
        .stRect = {0, 0, PANEL_WIDTH, PANEL_HEIGHT},
    }
};

PARAM_CLASSDEFINE(PARAM_VODEV_S,VOCFG,CTX,VO)[] = {
    {
        .stVoConfig = {
            .VoDev  = 0,
            .u32DisBufLen  = 3,
            .stVoPubAttr.enIntfType  = VO_INTF_MIPI,
            .stVoPubAttr.enIntfSync  = VO_OUTPUT_1024x600_60,
            .stVoPubAttr.u32BgColor = COLOR_10_RGB_BLACK,
            .stDispRect = {0, 0, PANEL_WIDTH, PANEL_HEIGHT},
            .stImageSize = {PANEL_WIDTH, PANEL_HEIGHT},
            .enPixFormat = PIXEL_FORMAT_NV21,
        },
        .stLayerAttr = {
            .stDispRect = {0, 0, PANEL_WIDTH, PANEL_HEIGHT},
            .u32DispFrmRt = 60,
            .stImageSize = {PANEL_WIDTH, PANEL_HEIGHT},
            .enPixFormat = PIXEL_FORMAT_NV21,
        },
        .u8ChnCnt = 1,
        .pstVOChnAttr = PARAM_CLASS(VOCHN,VO,CHN),
        .stSrcChn = {
            .enModId = CVI_ID_VPSS,
            .s32ChnId = 1,
            .s32DevId = 0,
        },
        .stDestChn = {
            .enModId = CVI_ID_VO,
            .s32ChnId = 0,
            .s32DevId = 0,
        },
        .u8ChnRotation = ROTATION_0,
    }
};

PARAM_VO_CFG_S g_stVoCtx = {
#if (CONFIG_PANEL_HX8394 == 1 || CONFIG_PANEL_ILI9488 == 1 \
    || CONFIG_PANEL_JD9165 == 1)
    .u8VoCnt = 1,
#else
    .u8VoCnt = 0,
#endif
    .pstVoCfg = PARAM_CLASS(VOCFG,CTX,VO),
};

PARAM_VO_CFG_S * PARAM_GET_VO_CFG(void) {
    return &g_stVoCtx;
}

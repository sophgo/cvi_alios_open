/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_vpsscfg.c
 * Description:
 *   ....
 */
#include "custom_param.h"
#include "board_config.h"
#include "cvi_buffer.h"

PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP0,CHN)[] = {
    {
        .u8Rotation = ROTATION_0,
        .stVpssChnAttr = {
            .u32Width = 1200,
            .u32Height = 1600,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate.s32SrcFrameRate = -1,
            .stFrameRate.s32DstFrameRate = -1,
            .bFlip = CVI_FALSE,
            .bMirror = CVI_FALSE,
            .u32Depth  = 0,
            .stAspectRatio.enMode        = ASPECT_RATIO_AUTO,
            .stAspectRatio.bEnableBgColor = CVI_TRUE,
            //.stAspectRatio.u32BgColor    = COLOR_RGB_BLACK,
            .stNormalize.bEnable         = CVI_FALSE,
        }
    },
    {
        .u8Rotation = ROTATION_0,
        .stVpssChnAttr = {
            .u32Width = PANEL_WIDTH,
            .u32Height = PANEL_HEIGHT,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate.s32SrcFrameRate = -1,
            .stFrameRate.s32DstFrameRate = -1,
            .bFlip = CVI_FALSE,
            .bMirror = CVI_FALSE,
            .u32Depth  = 0,
            .stAspectRatio.enMode        = ASPECT_RATIO_MANUAL,
            .stAspectRatio.bEnableBgColor = CVI_TRUE,
            .stAspectRatio.stVideoRect.s32X = 0,
            .stAspectRatio.stVideoRect.s32Y = 0,
            .stAspectRatio.stVideoRect.u32Width = PANEL_WIDTH,
            .stAspectRatio.stVideoRect.u32Height = PANEL_HEIGHT,
            //.stAspectRatio.u32BgColor    = COLOR_RGB_BLACK,
            .stNormalize.bEnable         = CVI_FALSE,
        }
    },
    {
        .u8Rotation = ROTATION_0,
        .stVpssChnAttr = {
            .u32Width = 480,
            .u32Height = 720,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate.s32SrcFrameRate = -1,
            .stFrameRate.s32DstFrameRate = -1,
            .bFlip = CVI_FALSE,
            .bMirror = CVI_FALSE,
            .u32Depth  = 0,
            .stAspectRatio.enMode        = ASPECT_RATIO_AUTO,
            .stAspectRatio.bEnableBgColor = CVI_TRUE,
            //.stAspectRatio.u32BgColor    = COLOR_RGB_BLACK,
            .stNormalize.bEnable         = CVI_FALSE,
        }
    },
};


PARAM_CLASSDEFINE(PARAM_VPSS_GRP_CFG_S,GRPCFG,CTX,GRP)[] = {
    {
        .VpssGrp = 0,
        .u8ChnCnt = 3,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP0,CHN),
        .u8ViRotation = 0,
        .s32BindVidev = -1,
        .stVpssGrpAttr = {
            .u8VpssDev = 1,
            .u32MaxW = 1200,
            .u32MaxH = 1600,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate.s32SrcFrameRate = -1,
            .stFrameRate.s32DstFrameRate = -1,
        },
        .bBindMode = CVI_TRUE,
        .astChn[0] = {
            .enModId = CVI_ID_VI,
            .s32DevId = 0,
            .s32ChnId = 0,
        },
        .astChn[1] = {
            .enModId = CVI_ID_VPSS,
            .s32DevId = 0,
            .s32ChnId = 0,
        }
    },
};

PARAM_VPSS_CFG_S  g_stVpssCtx = {
    .u8GrpCnt = 1,
    .pstVpssGrpCfg = PARAM_CLASS(GRPCFG,CTX,GRP),
};

PARAM_VPSS_CFG_S * PARAM_GET_VPSS_CFG(void) {
    return &g_stVpssCtx;
}

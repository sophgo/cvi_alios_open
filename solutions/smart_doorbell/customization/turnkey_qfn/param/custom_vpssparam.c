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
            .u32Width = 1920,
            .u32Height = 1080,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .bFlip = CVI_FALSE,
            .bMirror = CVI_FALSE,
            .u32Depth  = 0,
            .stAspectRatio= {
                .enMode = ASPECT_RATIO_AUTO,
                .bEnableBgColor = CVI_TRUE,
                .u32BgColor = COLOR_RGB_BLACK,
            },
            .stNormalize = {
                .bEnable = CVI_FALSE,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = CVI_FALSE,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Height = -1,
                .u32Width = -1,
            },
        },
    },
};

PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP1,CHN)[] = {
    {
        .u8Rotation = ROTATION_0,
        .stVpssChnAttr = {
            .u32Width = 1920,
            .u32Height = 1080,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .bFlip = CVI_FALSE,
            .bMirror = CVI_FALSE,
            .u32Depth = 0,
            .stAspectRatio= {
                .enMode = ASPECT_RATIO_AUTO,
                .bEnableBgColor = CVI_TRUE,
                .u32BgColor     = COLOR_RGB_BLACK,
            },
            .stNormalize = {
                .bEnable = CVI_FALSE,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = CVI_FALSE,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Height = -1,
                .u32Width = -1,
            },
        },
    },
};

PARAM_CLASSDEFINE(PARAM_VPSS_GRP_CFG_S,GRPCFG,CTX,GRP)[] = {
    {
        .VpssGrp = 0,
        .u8ChnCnt = 1,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP0,CHN),
        .u8ViRotation = 0,
        .s32BindVidev = 0,
        .stVpssGrpAttr = {
            .u8VpssDev = 1,
            .u32MaxW = -1,
            .u32MaxH = -1,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
        },
        .bBindMode = CVI_FALSE,
        .astChn[0] = {
            .enModId = CVI_ID_VI,
            .s32DevId = 0,
            .s32ChnId = 0,
        },
        .astChn[1] = {
            .enModId = CVI_ID_VPSS,
            .s32DevId = 0,
            .s32ChnId = 0,
        },
        .stVpssGrpCropInfo = {
            .bEnable = CVI_FALSE,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Height = -1,
                .u32Width = -1,
            },
        },
    },
    {
        .VpssGrp = 1,
        .u8ChnCnt = 1,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP1,CHN),
        .u8ViRotation = 0,
        .s32BindVidev = 1,
        .stVpssGrpAttr = {
            .u8VpssDev = 1,
            .u32MaxW = -1,
            .u32MaxH = -1,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
        },
        .bBindMode = CVI_FALSE,
        .astChn[0] = {
            .enModId = CVI_ID_VI,
            .s32DevId = 0,
            .s32ChnId = 1,
        },
        .astChn[1] = {
            .enModId = CVI_ID_VPSS,
            .s32DevId = 1,
            .s32ChnId = 0,
        },
        .stVpssGrpCropInfo = {
            .bEnable = CVI_FALSE,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Height = -1,
                .u32Width = -1,
            },
        },
    },
};


PARAM_VPSS_CFG_S  g_stVpssCtx = {
    .u8GrpCnt = 2,
    .pstVpssGrpCfg = PARAM_CLASS(GRPCFG,CTX,GRP),
};

PARAM_VPSS_CFG_S * PARAM_GET_VPSS_CFG(void) {
    return &g_stVpssCtx;
}

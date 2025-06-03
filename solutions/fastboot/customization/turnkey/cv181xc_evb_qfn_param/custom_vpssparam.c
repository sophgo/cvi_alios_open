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
        .abChnEnable = 1,
        .stVpssChnAttr = {
            .u32Width = 2560,
            .u32Height = 1440,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate.s32SrcFrameRate = -1,
            .stFrameRate.s32DstFrameRate = -1,
            .u32Depth = 0,
            .bMirror = 0,
            .bFlip = 0,
            .stAspectRatio = {
                .enMode = ASPECT_RATIO_NONE,
                .stVideoRect.s32X = 0,
                .stVideoRect.s32Y = 0,
                .stVideoRect.u32Width = 0,
                .stVideoRect.u32Height = 0,
                .bEnableBgColor = 1,
                .bEnableBgColor = 0,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = 0,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .u8VpssAttachEnable = 0,
        .u8VpssAttachId = 0,
        .u8Rotation = 0,
    },
    {
        .abChnEnable = 1,
        .stVpssChnAttr = {
            .u32Width = 720,
            .u32Height = 576,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate.s32SrcFrameRate = -1,
            .stFrameRate.s32DstFrameRate = -1,
            .u32Depth = 0,
            .bMirror = 0,
            .bFlip = 0,
            .stAspectRatio = {
                .enMode = ASPECT_RATIO_NONE,
                .stVideoRect.s32X = 0,
                .stVideoRect.s32Y = 0,
                .stVideoRect.u32Width = 0,
                .stVideoRect.u32Height = 0,
                .bEnableBgColor = 1,
                .bEnableBgColor = 0,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = 0,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .u8VpssAttachEnable = 1,
        .u8VpssAttachId = 0,
        .u8Rotation = 0,
    },
    {
        .abChnEnable = 1,
        .stVpssChnAttr = {
            .u32Width = 960,
            .u32Height = 540,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate.s32SrcFrameRate = -1,
            .stFrameRate.s32DstFrameRate = -1,
            .u32Depth = 0,
            .bMirror = 0,
            .bFlip = 0,
            .stAspectRatio = {
                .enMode = ASPECT_RATIO_NONE,
                .stVideoRect.s32X = 0,
                .stVideoRect.s32Y = 0,
                .stVideoRect.u32Width = 0,
                .stVideoRect.u32Height = 0,
                .bEnableBgColor = 1,
                .bEnableBgColor = 0,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = 0,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .u8VpssAttachEnable = 1,
        .u8VpssAttachId = 1,
        .u8Rotation = 0,
    },
};

// double sensor reserved
// PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP1,CHN)[] = {

// };

PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP2,CHN)[] = {
    {
        .abChnEnable = 1,
        .stVpssChnAttr = {
            .u32Width = 384,
            .u32Height = 256,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_RGB_888_PLANAR,
            .stFrameRate.s32SrcFrameRate = 15,
            .stFrameRate.s32DstFrameRate = 5,
            .u32Depth = 0,
            .bMirror = 0,
            .bFlip = 0,
            .stAspectRatio = {
                .enMode = ASPECT_RATIO_NONE,
                .stVideoRect.s32X = 0,
                .stVideoRect.s32Y = 0,
                .stVideoRect.u32Width = 0,
                .stVideoRect.u32Height = 0,
                .bEnableBgColor = 1,
                .bEnableBgColor = 0,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = 0,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .u8VpssAttachEnable = 1,
        .u8VpssAttachId = 2,
        .u8Rotation = 0,
    },
};

PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP3,CHN)[] = {
    {
        .abChnEnable = 1,
        .stVpssChnAttr = {
            .u32Width = 640,
            .u32Height = 360,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_YUV_400,
            .stFrameRate.s32SrcFrameRate = 15,
            .stFrameRate.s32DstFrameRate = 5,
            .u32Depth = 0,
            .bMirror = 0,
            .bFlip = 0,
            .stAspectRatio = {
                .enMode = ASPECT_RATIO_NONE,
                .stVideoRect.s32X = 0,
                .stVideoRect.s32Y = 0,
                .stVideoRect.u32Width = 0,
                .stVideoRect.u32Height = 0,
                .bEnableBgColor = 1,
                .bEnableBgColor = 0,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = 0,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .u8VpssAttachEnable = 1,
        .u8VpssAttachId = 3,
        .u8Rotation = 0,
    },
};

PARAM_CLASSDEFINE(PARAM_VPSS_GRP_CFG_S,GRPCFG,CTX,GRP)[] = {
    {
        .bEnable = 1,
        .VpssGrp = 0,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP0,CHN),
        .stVpssGrpAttr = {
            .u8VpssDev = 1,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate.s32SrcFrameRate = -1,
            .stFrameRate.s32DstFrameRate = -1,
            .u32MaxW = 2560,
            .u32MaxH = 1440,
        },
        .u8ChnCnt = 3,
        .stVpssGrpCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = 0,
            .stCropRect.s32X = 0,
            .stCropRect.s32Y = 0,
            .stCropRect.u32Width = 0,
            .stCropRect.u32Height = 0,
        },
        .bBindMode = 1,
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
        .s32BindVidev = 0,
        .u8ViRotation = 0,
    },
    {
        .bEnable = 0,
    },
    {
        .bEnable = 1,
        .VpssGrp = 2,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP2,CHN),
        .stVpssGrpAttr = {
            .u8VpssDev = 0,
            .enPixelFormat = PIXEL_FORMAT_RGB_888_PLANAR,
            .stFrameRate.s32SrcFrameRate = -1,
            .stFrameRate.s32DstFrameRate = -1,
            .u32MaxW = 960,
            .u32MaxH = 540,
        },
        .u8ChnCnt = 1,
        .stVpssGrpCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = 0,
            .stCropRect.s32X = 0,
            .stCropRect.s32Y = 0,
            .stCropRect.u32Width = 0,
            .stCropRect.u32Height = 0,
        },
        .bBindMode = 1,
        .astChn[0] = {
            .enModId = CVI_ID_VPSS,
            .s32DevId = 0,
            .s32ChnId = 2,
        },
        .astChn[1] = {
            .enModId = CVI_ID_VPSS,
            .s32DevId = 2,
            .s32ChnId = 0,
        },
        .s32BindVidev = -1,
        .u8ViRotation = 0,
    },
    {
        .bEnable = 1,
        .VpssGrp = 3,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP3,CHN),
        .stVpssGrpAttr = {
            .u8VpssDev = 0,
            .enPixelFormat = PIXEL_FORMAT_YUV_400,
            .stFrameRate.s32SrcFrameRate = -1,
            .stFrameRate.s32DstFrameRate = -1,
            .u32MaxW = 960,
            .u32MaxH = 540,
        },
        .u8ChnCnt = 1,
        .stVpssGrpCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = 0,
            .stCropRect.s32X = 0,
            .stCropRect.s32Y = 0,
            .stCropRect.u32Width = 0,
            .stCropRect.u32Height = 0,
        },
        .bBindMode = 1,
        .astChn[0] = {
            .enModId = CVI_ID_VPSS,
            .s32DevId = 0,
            .s32ChnId = 2,
        },
        .astChn[1] = {
            .enModId = CVI_ID_VPSS,
            .s32DevId = 3,
            .s32ChnId = 0,
        },
        .s32BindVidev = -1,
        .u8ViRotation = 0,
    },
};

PARAM_VPSS_CFG_S  g_stVpssCtx = {
    .u8GrpCnt = 4,
    .pstVpssGrpCfg = PARAM_CLASS(GRPCFG,CTX,GRP),
};

PARAM_VPSS_CFG_S * PARAM_GET_VPSS_CFG(void) {
    return &g_stVpssCtx;
}

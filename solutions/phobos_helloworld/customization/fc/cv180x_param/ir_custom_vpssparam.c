#include <board_config.h>
#include "ir_custom_param.h"

//IR Pipeline



PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP0,CHN)[] = {
    {
        .u8Rotation = ROTATION_90,
        .stVpssChnAttr = {
            .u32Width = 640,
            .u32Height = 480,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate.s32SrcFrameRate = -1,
            .stFrameRate.s32DstFrameRate = -1,
            .bFlip = CVI_FALSE,
            .bMirror = CVI_FALSE,
            .u32Depth  = 0,
            .stAspectRatio.enMode        = ASPECT_RATIO_AUTO,
            .stAspectRatio.bEnableBgColor = CVI_TRUE,
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
            .stAspectRatio.stVideoRect.s32X = 0,
            .stAspectRatio.stVideoRect.s32Y = 0,
            .stAspectRatio.stVideoRect.u32Width = PANEL_WIDTH,
            .stAspectRatio.stVideoRect.u32Height = PANEL_HEIGHT,
            .stAspectRatio.bEnableBgColor = CVI_TRUE,
            //.stAspectRatio.u32BgColor    = COLOR_RGB_BLACK,
            .stNormalize.bEnable         = CVI_FALSE,
        }
    },
};

PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP1,CHN)[] = {
    {
        .u8Rotation = ROTATION_90,
        .stVpssChnAttr = {
            .u32Width = 640,
            .u32Height = 480,
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
        .u8ChnCnt = 2,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP0,CHN),
        .u8ViRotation = 0,
        .s32BindVidev = 0,
        .stVpssGrpAttr = {
            .u8VpssDev = 1,
            .u32MaxW = -1,
            .u32MaxH = -1,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate.s32SrcFrameRate = -1,
            .stFrameRate.s32DstFrameRate = -1,
        },
        .bBindMode = CVI_FALSE,
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
            .stFrameRate.s32SrcFrameRate = -1,
            .stFrameRate.s32DstFrameRate = -1,
        },
        .bBindMode = CVI_FALSE,
    },
};

PARAM_VPSS_CFG_S  g_stFcVpssCtx = {
    .u8GrpCnt = 2,
    .pstVpssGrpCfg = PARAM_CLASS(GRPCFG,CTX,GRP),
};

PARAM_VPSS_CFG_S * IR_PARAM_GET_VPSS_CFG(void)
{
    return &g_stFcVpssCtx;
}







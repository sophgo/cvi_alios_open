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
            .u32Width = 1920,
            .u32Height = 1080,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .u32Depth = 0,
            .bMirror = 0,
            .bFlip = 0,
            .stAspectRatio = {
                .enMode = ASPECT_RATIO_NONE,
                .stVideoRect = {
                    .s32X = 0,
                    .s32Y = 0,
                    .u32Width = 0,
                    .u32Height = 0,
                },
                .bEnableBgColor = 0,
                .u32BgColor = 0,
                
            },
            .stNormalize = {
                .bEnable = 0,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .u8VpssAttachEnable = 0,
        .u8VpssAttachId = 0,
        .u8Rotation = ROTATION_0,
    },
    {
        .abChnEnable = 1,
        .stVpssChnAttr = {
            .u32Width = 640,
            .u32Height = 360,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .u32Depth = 0,
            .bMirror = 0,
            .bFlip = 0,
            .stAspectRatio = {
                .enMode = ASPECT_RATIO_NONE,
                .stVideoRect = {
                    .s32X = 0,
                    .s32Y = 0,
                    .u32Width = 0,
                    .u32Height = 0,
                },
                .bEnableBgColor = 0,
                .u32BgColor = 0,
                
            },
            .stNormalize = {
                .bEnable = 0,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .u8VpssAttachEnable = 1,
        .u8VpssAttachId = 0,
        .u8Rotation = ROTATION_0,
    }
};

PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP1,CHN)[] = {
    {
        .abChnEnable = 1,
        .stVpssChnAttr = {
            .u32Width = 1920,
            .u32Height = 1080,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .u32Depth = 0,
            .bMirror = 0,
            .bFlip = 0,
            .stAspectRatio = {
                .enMode = ASPECT_RATIO_NONE,
                .stVideoRect = {
                    .s32X = 0,
                    .s32Y = 0,
                    .u32Width = 0,
                    .u32Height = 0,
                },
                .bEnableBgColor = 0,
                .u32BgColor = 0,
                
            },
            .stNormalize = {
                .bEnable = 0,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .u8VpssAttachEnable = 0,
        .u8VpssAttachId = 0,
        .u8Rotation = ROTATION_0,
    },
    {
        .abChnEnable = 1,
        .stVpssChnAttr = {
            .u32Width = 640,
            .u32Height = 360,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .u32Depth = 0,
            .bMirror = 0,
            .bFlip = 0,
            .stAspectRatio = {
                .enMode = ASPECT_RATIO_NONE,
                .stVideoRect = {
                    .s32X = 0,
                    .s32Y = 0,
                    .u32Width = 0,
                    .u32Height = 0,
                },
                .bEnableBgColor = 0,
                .u32BgColor = 0,
                
            },
            .stNormalize = {
                .bEnable = 0,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .u8VpssAttachEnable = 1,
        .u8VpssAttachId = 1,
        .u8Rotation = ROTATION_0,
    }
};

PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP3,CHN)[] = {
    {
        //grp3 chn0
        .abChnEnable = 1,
        .stVpssChnAttr = {
            .u32Width = 1920,
            .u32Height = 1080,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .u32Depth = 0,
            .bMirror = 0,
            .bFlip = 0,
            .stAspectRatio = {
                .enMode = ASPECT_RATIO_NONE,
                .stVideoRect = {
                    .s32X = 0,
                    .s32Y = 0,
                    .u32Width = 0,
                    .u32Height = 0,
                },
                .bEnableBgColor = 0,
                .u32BgColor = 0,
                
            },
            .stNormalize = {
                .bEnable = 0,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .u8VpssAttachEnable = 0,
        .u8VpssAttachId = 0,
        .u8Rotation = ROTATION_0,
    },
    {
        //grp3 chn1
        .abChnEnable = 0,
        .stVpssChnAttr = {
            .u32Width = 640,
            .u32Height = 360,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .u32Depth = 0,
            .bMirror = 0,
            .bFlip = 0,
            .stAspectRatio = {
                .enMode = ASPECT_RATIO_NONE,
                .stVideoRect = {
                    .s32X = 0,
                    .s32Y = 0,
                    .u32Width = 0,
                    .u32Height = 0,
                },
                .bEnableBgColor = 0,
                .u32BgColor = 0,
                
            },
            .stNormalize = {
                .bEnable = 0,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .u8VpssAttachEnable = 1,
        .u8VpssAttachId = 2,
        .u8Rotation = ROTATION_0,
    },
};

PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP4,CHN)[] = {
    {
        //grp4 chn0
        .abChnEnable = 1,
        .stVpssChnAttr = {
            .u32Width = 384,
            .u32Height = 256,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_RGB_888_PLANAR,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .u32Depth = 0,
            .bMirror = 0,
            .bFlip = 0,
            .stAspectRatio = {
                .enMode = ASPECT_RATIO_NONE,
                .stVideoRect = {
                    .s32X = 0,
                    .s32Y = 0,
                    .u32Width = 0,
                    .u32Height = 0,
                },
                .bEnableBgColor = 0,
                .u32BgColor = 0,
                
            },
            .stNormalize = {
                .bEnable = 0,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .u8VpssAttachEnable = 1,
        .u8VpssAttachId = 3,
        .u8Rotation = ROTATION_0,
    },
};

PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP5,CHN)[] = {
    {
        //grp5 chn0
        .abChnEnable = 1,
        .stVpssChnAttr = {
            .u32Width = 640,
            .u32Height = 360,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_YUV_400,
            .stFrameRate = {
                .s32SrcFrameRate = 15,
                .s32DstFrameRate = 5,
            },
            .u32Depth = 0,
            .bMirror = 0,
            .bFlip = 0,
            .stAspectRatio = {
                .enMode = ASPECT_RATIO_NONE,
                .stVideoRect = {
                    .s32X = 0,
                    .s32Y = 0,
                    .u32Width = 0,
                    .u32Height = 0,
                },
                .bEnableBgColor = 0,
                .u32BgColor = 0,
                
            },
            .stNormalize = {
                .bEnable = 0,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .u8VpssAttachEnable = 1,
        .u8VpssAttachId = 4,
        .u8Rotation = ROTATION_0,
    },
};

PARAM_CLASSDEFINE(PARAM_VPSS_GRP_CFG_S,GRPCFG,CTX,GRP)[] = {
    {
        .bEnable = 1,
        .stVpssGrpAttr = {
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .u8VpssDev = 1,
            .u32MaxW = 1920,
            .u32MaxH = 1080,
        },
        .VpssGrp = 0,
        .u8ChnCnt = 2,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP0,CHN),
        .stVpssGrpCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .bBindMode = 0,
        .astChn[0].enModId = CVI_ID_VI,
        .astChn[0].s32DevId = 0,
        .astChn[0].s32ChnId = 0,
        .astChn[1].enModId = CVI_ID_VPSS,
        .astChn[1].s32DevId = 0,
        .astChn[1].s32ChnId = 0,
        .s32BindVidev = 0,
        .u8ViRotation = 0,
    },
    {
        .bEnable = 1,
        .stVpssGrpAttr = {
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .u8VpssDev = 1,
            .u32MaxW = 1920,
            .u32MaxH = 1080,
        },
        .VpssGrp = 1,
        .u8ChnCnt = 2,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP1,CHN),
        .stVpssGrpCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .bBindMode = 0,
        .astChn[0].enModId = CVI_ID_VI,
        .astChn[0].s32DevId = 0,
        .astChn[0].s32ChnId = 0,
        .astChn[1].enModId = CVI_ID_VPSS,
        .astChn[1].s32DevId = 0,
        .astChn[1].s32ChnId = 0,
        .s32BindVidev = 2,
        .u8ViRotation = 0,
    },
    {
        .bEnable = 0,
    },
    {
        .bEnable = 1,
        .stVpssGrpAttr = {
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .u8VpssDev = 1,
            .u32MaxW = 1920,
            .u32MaxH = 1080,
        },
        .VpssGrp = 3,
        .u8ChnCnt = 2,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP3,CHN),
        .stVpssGrpCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .bBindMode = 0,
        .astChn[0].enModId = CVI_ID_VI,
        .astChn[0].s32DevId = 0,
        .astChn[0].s32ChnId = 0,
        .astChn[0].enModId = CVI_ID_VPSS,
        .astChn[0].s32DevId = 0,
        .astChn[0].s32ChnId = 0,
        .s32BindVidev = 1,
        .u8ViRotation = 0,
    },
    {
        .bEnable = 1,
        .stVpssGrpAttr = {
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .u8VpssDev = 0,
            .u32MaxW = 640,
            .u32MaxH = 360,
        },
        .VpssGrp = 4,
        .u8ChnCnt = 1,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP4,CHN),
        .stVpssGrpCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .bBindMode = 1,
        .astChn[0].enModId = CVI_ID_VPSS,
        .astChn[0].s32DevId = 1,
        .astChn[0].s32ChnId = 1,
        .astChn[1].enModId = CVI_ID_VPSS,
        .astChn[1].s32DevId = 4,
        .astChn[1].s32ChnId = 0,
        .s32BindVidev = -1,
        .u8ViRotation = 0,
    },
    {
        .bEnable = 1,
        .stVpssGrpAttr = {
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .u8VpssDev = 0,
            .u32MaxW = 640,
            .u32MaxH = 360,
        },
        .VpssGrp = 5,
        .u8ChnCnt = 1,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP5,CHN),
        .stVpssGrpCropInfo = {
            .bEnable = 0,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Width = 0,
                .u32Height = 0,
            },
        },
        .bBindMode = 1,
        .astChn[0].enModId = CVI_ID_VPSS,
        .astChn[0].s32DevId = 1,
        .astChn[0].s32ChnId = 1,
        .astChn[1].enModId = CVI_ID_VPSS,
        .astChn[1].s32DevId = 5,
        .astChn[1].s32ChnId = 0,
        .s32BindVidev = -1,
        .u8ViRotation = 0,
    }
};

PARAM_VPSS_CFG_S  g_stVpssCtx = {
    .u8GrpCnt = 6,
    .pstVpssGrpCfg = PARAM_CLASS(GRPCFG,CTX,GRP),
};

PARAM_VPSS_CFG_S * PARAM_GET_VPSS_CFG(void) {
    return &g_stVpssCtx;
}

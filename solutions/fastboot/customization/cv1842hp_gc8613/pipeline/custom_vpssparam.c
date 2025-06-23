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
    { //grp0 chn0
        .abChnEnable = 1,
        .stVpssChnAttr = {
            .u32Width = 3840,
            .u32Height = 2160,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV12,
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
                .bEnableBgColor = 1,
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
	//grp0 chn1
    {
        .abChnEnable = 1,
        .stVpssChnAttr = {
            .u32Width = 768,
            .u32Height = 572,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV12,
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
                    .u32Width = 768,
                    .u32Height = 572,
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
	//grp0 chn2
	{
		.abChnEnable = 1,
		.stVpssChnAttr = {
			.u32Width = 960,
			.u32Height = 540,
			.enVideoFormat = VIDEO_FORMAT_LINEAR,
			.enPixelFormat = PIXEL_FORMAT_NV12,
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
					.u32Width = 960,
					.u32Height = 540,
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
	}
};

PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP2,CHN)[] = {
    {
        //grp2 chn0
        .abChnEnable = 1,
        .stVpssChnAttr = {
            .u32Width = 640,
            .u32Height = 384,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_RGB_888_PLANAR,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .u32Depth = 1,
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
                .bEnableBgColor = 1,
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
};

PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP3,CHN)[] = {
    {
        //grp3 chn0
        .abChnEnable = 1,
        .stVpssChnAttr = {
            .u32Width = 768,
            .u32Height = 432,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_RGB_888_PLANAR,
            .stFrameRate = {
                .s32SrcFrameRate = 25,
                .s32DstFrameRate = 5,
            },
            .u32Depth = 1,
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
                .bEnableBgColor = 1,
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
};

PARAM_CLASSDEFINE(PARAM_VPSS_GRP_CFG_S,GRPCFG,CTX,GRP)[] = {
    { //grp0
        .bEnable = 1,
        .stVpssGrpAttr = {
            .enPixelFormat = PIXEL_FORMAT_NV12,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .u8VpssDev = 1,
            .u32MaxW = 3840,
            .u32MaxH = 2160,
        },
        .VpssGrp = 0,
        .u8ChnCnt = 3,
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
    { //grp2
        .bEnable = 1,
        .stVpssGrpAttr = {
            .enPixelFormat = PIXEL_FORMAT_NV12,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .u8VpssDev = 0,
            .u32MaxW = 960,
            .u32MaxH = 540,
        },
        .VpssGrp = 2,
        .u8ChnCnt = 1,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP2,CHN),
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
        .astChn[0].s32DevId = 0,
        .astChn[0].s32ChnId = 2,
        .astChn[1].enModId = CVI_ID_VPSS,
        .astChn[1].s32DevId = 2,
        .astChn[1].s32ChnId = 0,
        .s32BindVidev = -1,
        .u8ViRotation = 0,
    },
    { //grp3
        .bEnable = 1,
        .stVpssGrpAttr = {
            .enPixelFormat = PIXEL_FORMAT_NV12,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .u8VpssDev = 0,
            .u32MaxW = 960,
            .u32MaxH = 540,
        },
        .VpssGrp = 3,
        .u8ChnCnt = 1,
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
        .bBindMode = 1,
        .astChn[0].enModId = CVI_ID_VPSS,
        .astChn[0].s32DevId = 0,
        .astChn[0].s32ChnId = 2,
        .astChn[1].enModId = CVI_ID_VPSS,
        .astChn[1].s32DevId = 3,
        .astChn[1].s32ChnId = 0,
        .s32BindVidev = -1,
        .u8ViRotation = 0,
    },
};

PARAM_VPSS_CFG_S  g_stVpssCtx = {
    .stVPSSMode.enMode = VPSS_MODE_DUAL,
    .stVPSSMode.aenInput[0] = VPSS_INPUT_MEM,
    .stVPSSMode.aenInput[1] = VPSS_INPUT_ISP,
    .u8GrpCnt = 3,
    .pstVpssGrpCfg = PARAM_CLASS(GRPCFG,CTX,GRP),
};

PARAM_VPSS_CFG_S * PARAM_GET_VPSS_CFG(void) {
    return &g_stVpssCtx;
}
